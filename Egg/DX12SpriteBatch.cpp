#include "DX12SpriteBatch.h"
#include "Utility.h"
#include <algorithm>
#include "DX12ResourceViews.h"

namespace Egg::Graphics::DX12 {

	const DirectX::XMMATRIX SpriteBatch::MatrixIdentity = DirectX::XMMatrixIdentity();
	const DirectX::XMFLOAT2 SpriteBatch::Float2Zero(0, 0);

	uint64_t SpriteBatch::indexBuffer{ 0 };

void SpriteBatch::CreateIndexBuffer(Egg::Module::IGraphicsModule * graphics)
{
	if(indexBuffer != 0) {
		return;
	}

	std::vector<short> indices;

	indices.reserve(MaxBatchSize * IndicesPerSprite);

	for(size_t j = 0; j < MaxBatchSize * VerticesPerSprite; j += VerticesPerSprite)
	{
		short i = static_cast<short>(j);

		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);

		indices.push_back(i + 1);
		indices.push_back(i + 3);
		indices.push_back(i + 2);
	}
	size_t ibufferSize = sizeof(short) * MaxBatchSize * IndicesPerSprite;

	indexBuffer = graphics->resources->CreateIndexBuffer(ibufferSize, DXGI_FORMAT_R16_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
	
	UploadBatch upload;
	upload.Upload(indexBuffer, indices.data(), ibufferSize);
	upload.ResourceBarrier(indexBuffer, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);

	graphics->frame->SyncUpload(upload);
}

SpriteBatch::SpriteBatch(Egg::Module::IGraphicsModule * graphics)
		: mSpriteQueueCount{ 0 },
		mSpriteQueueArraySize{ 0 },
		mInBeginEndPair{ false },
		mSortMode{ SpriteSortMode::SpriteSortMode_Deferred },
		mTransformMatrix{ MatrixIdentity },
		mVertexPageSize{ 6 * 4 * MaxBatchSize * VerticesPerSprite },
		mSpriteCount{ 0 }
	{
	resourceContext = graphics->resources;
	renderContext = graphics->renderer;

		vertexBuffer = graphics->resources->CreateVertexBuffer(sizeof(PCT_Vertex) * MaxBatchSize * VerticesPerSprite, sizeof(PCT_Vertex), ResourceType::PERMANENT_UPLOAD, ResourceState::ANY_READ);
		vertexData = std::make_unique<PCT_Vertex[]>(MaxBatchSize * VerticesPerSprite);

		auto ilBuilder = graphics->CreateInputLayoutBuilder();
		ilBuilder->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		ilBuilder->AddInputElement("COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT);
		ilBuilder->AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		InputLayoutRef inputLayout = ilBuilder->Build();

		auto shaderBuilder = graphics->CreateShaderBuilder();
		ShaderBytecodeRef vs = shaderBuilder->LoadBytecode(L"SpriteFont_Vertex.cso");
		ShaderBytecodeRef ps = shaderBuilder->LoadBytecode(L"SpriteFont_Pixel.cso");

		auto rootSigBuilder = graphics->CreateRootSignatureBuilder();
		rootSignature = rootSigBuilder->BuildFromShader(vs);

		BlendDesc blendState;
		RenderTargetBlendDesc rt0Blend;
		rt0Blend.blendEnable = true;
		rt0Blend.logicOpEnable = false;
		rt0Blend.srcBlend = BlendMode::SRC_ALPHA;
		rt0Blend.destBlend = BlendMode::INV_SRC_ALPHA;
		rt0Blend.blendOp = BlendOp::ADD;
		rt0Blend.srcBlendAlpha = BlendMode::ONE;
		rt0Blend.destBlendAlpha = BlendMode::INV_SRC_ALPHA;
		rt0Blend.blendOpAlpha = BlendOp::ADD;
		rt0Blend.logicOp = LogicOp::NOOP;
		rt0Blend.renderTargetWriteMask = 0x0F; // all
		
		blendState.alphaToCoverageEnabled = false;
		blendState.independentAlphaEnabled = false;
		blendState.rtBlend[0] = rt0Blend;

		RasterizerDesc rasterizerState;
		rasterizerState.fillMode = FillMode::SOLID;
		rasterizerState.cullMode = CullMode::NONE;
		rasterizerState.frontCounterClockwise = false;
		rasterizerState.depthBias = 0;
		rasterizerState.depthBiasClamp = 0.0f;
		rasterizerState.slopeScaledDepthBias = 0.0f;
		rasterizerState.depthClipEnable = true;
		rasterizerState.multisampleEnable = true;
		rasterizerState.antialiasedLineEnable = false;
		rasterizerState.forcedSampleCount = 0;
		rasterizerState.conservativeRaster = false;

		DepthStencilDesc depthStencilDesc;
		depthStencilDesc.depthEnable = false;
		depthStencilDesc.stencilEnable = false;
		depthStencilDesc.depthWriteMaskZero = true;

		auto psoBuilder = graphics->CreateGPipelineStateBuilder();
		psoBuilder->SetInputLayout(inputLayout);
		psoBuilder->SetRootSignature(rootSignature);
		psoBuilder->SetVertexShader(vs);
		psoBuilder->SetPixelShader(ps);
		psoBuilder->SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT);
		psoBuilder->SetRenderTargetFormats({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder->SetPrimitiveTopologyType(PrimitiveTopologyType::TRIANGLE);
		psoBuilder->SetBlendState(blendState);
		psoBuilder->SetRasterizerState(rasterizerState);
		psoBuilder->SetDepthStencilState(depthStencilDesc);

		pipelineState = psoBuilder->Build();

		CreateIndexBuffer(graphics);
	}

	void XM_CALLCONV SpriteBatch::Begin(SpriteSortMode sortMode, DirectX::FXMMATRIX transformMatrix)
	{
		if(mInBeginEndPair)
			throw std::exception("Cannot nest Begin calls on a single SpriteBatch");

		mSortMode = sortMode;
		mTransformMatrix = transformMatrix;
		mSpriteCount = 0;

		if(sortMode == SpriteSortMode_Immediate)
		{
			PrepareForRendering();
		}

		mInBeginEndPair = true;
	}

	void SpriteBatch::End()
	{
		if(!mInBeginEndPair)
			throw std::exception("Begin must be called before End");

		if(mSortMode != SpriteSortMode_Immediate)
		{
			PrepareForRendering();
			FlushBatch();
		}

		mInBeginEndPair = false;
	}

	void SpriteBatch::PrepareForRendering()
	{
		renderContext->SetRootSignature(rootSignature);
		renderContext->SetPipelineState(pipelineState);
		renderContext->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
		renderContext->SetIndexBuffer(indexBuffer);

	}

	// Sorts the array of queued sprites.
	void SpriteBatch::SortSprites()
	{
		// Fill the mSortedSprites vector.
		if(mSortedSprites.size() < mSpriteQueueCount)
		{
			GrowSortedSprites();
		}

		switch(mSortMode)
		{
		case SpriteSortMode_Texture:
			// Sort by texture.
			std::sort(mSortedSprites.begin(),
					  mSortedSprites.begin() + static_cast<int>(mSpriteQueueCount),
					  [](SpriteInfo const * x, SpriteInfo const * y) -> bool
			{
				return x->texture.get() < y->texture.get();
			});
			break;

		case SpriteSortMode_BackToFront:
			// Sort back to front.
			std::sort(mSortedSprites.begin(),
					  mSortedSprites.begin() + static_cast<int>(mSpriteQueueCount),
					  [](SpriteInfo const * x, SpriteInfo const * y) -> bool
			{
				return x->originRotationDepth.w > y->originRotationDepth.w;
			});
			break;

		case SpriteSortMode_FrontToBack:
			// Sort front to back.
			std::sort(mSortedSprites.begin(),
					  mSortedSprites.begin() + static_cast<int>(mSpriteQueueCount),
					  [](SpriteInfo const * x, SpriteInfo const * y) -> bool
			{
				return x->originRotationDepth.w < y->originRotationDepth.w;
			});
			break;

		default:
			break;
		}
	}

	// Sends queued sprites to the graphics device.
	void SpriteBatch::FlushBatch()
	{
		if(!mSpriteQueueCount)
			return;

		SortSprites();

		// Walk through the sorted sprite list, looking for adjacent entries that share a texture.
		ResourceViewsRef batchTexture;
		DirectX::XMVECTOR batchTextureSize = {};
		size_t batchStart = 0;

		for(size_t pos = 0; pos < mSpriteQueueCount; pos++)
		{
			ResourceViewsRef texture = mSortedSprites[pos]->texture;

			ASSERT(texture != nullptr, "Texture is not set");

			DirectX::XMVECTOR textureSize = mSortedSprites[pos]->textureSize;

			// Flush whenever the texture changes.
			if(texture != batchTexture)
			{
				if(pos > batchStart)
				{
					RenderBatch(batchTexture, batchTextureSize, &mSortedSprites[batchStart], pos - batchStart);
				}

				batchTexture = texture;
				batchTextureSize = textureSize;
				batchStart = pos;
			}
		}

		// Flush the final batch.
		RenderBatch(batchTexture, batchTextureSize, &mSortedSprites[batchStart], mSpriteQueueCount - batchStart);

		for(size_t i = 0; i < mSpriteQueueCount; ++i) {
			mSpriteQueue[i].texture.reset();
		}
		// Reset the queue.
		mSpriteQueueCount = 0;

		// When sorting is disabled, we persist mSortedSprites data from one batch to the next, to avoid
		// uneccessary work in GrowSortedSprites. But we never reuse these when sorting, because re-sorting
		// previously sorted items gives unstable ordering if some sprites have identical sort keys.
		if(mSortMode != SpriteSortMode_Deferred)
		{
			mSortedSprites.clear();
		}
	}

	void SpriteBatch::RenderBatch(ResourceViewsRef texture, DirectX::XMVECTOR textureSize, SpriteInfo const * const * sprites, size_t count)
	{
		renderContext->SetShaderResources(0, texture);

		// Convert to vector format.
		DirectX::XMVECTOR inverseTextureSize = DirectX::XMVectorReciprocal(textureSize);

		while(count > 0)
		{
			// How many sprites do we want to draw?
			size_t batchSize = count;

			// How many sprites does the D3D vertex buffer have room for?
			size_t remainingSpace = MaxBatchSize - mSpriteCount;

			if(batchSize > remainingSpace)
			{
				if(remainingSpace < MinBatchSize)
				{
					// If we are out of room, or about to submit an excessively small batch, wrap back to the start of the vertex buffer.
					mSpriteCount = 0;

					batchSize = std::min(count, MaxBatchSize);
				} else
				{
					// Take however many sprites fit in what's left of the vertex buffer.
					batchSize = remainingSpace;
				}
			}

			size_t vertexOffset = (UINT64(mSpriteCount)) * VerticesPerSprite;

			PCT_Vertex * vertices = vertexData.get() + vertexOffset;

			// Generate sprite vertex data.
			for(size_t i = 0; i < batchSize; i++)
			{
				assert(i < count);
				_Analysis_assume_(i < count);
				RenderSprite(sprites[i], vertices, textureSize, inverseTextureSize);

				vertices += VerticesPerSprite;
			}

			DirectX::XMFLOAT2 texSize;
			DirectX::XMStoreFloat2(&texSize, textureSize);

			
			DirectX::XMFLOAT4X4 texTransform = DirectX::XMFLOAT4X4(-1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			DirectX::XMMATRIX transformMat = DirectX::XMLoadFloat4x4(&texTransform);

			transformMat = DirectX::XMMatrixMultiply(transformMat,  mTransformMatrix);

			size_t spriteVertexTotalSize = sizeof(PCT_Vertex) * VerticesPerSprite;
			resourceContext->CopyConstants(vertexBuffer, vertexData.get() + vertexOffset, batchSize * spriteVertexTotalSize, vertexOffset * sizeof(PCT_Vertex));

			// Ok lads, the time has come for us draw ourselves some sprites!
			UINT indexCount = static_cast<UINT>(batchSize * IndicesPerSprite);

			DirectX::XMStoreFloat4x4A(&cbuffer.transform, transformMat);
			renderContext->SetConstants(1, cbuffer);
			renderContext->SetVertexBuffer(vertexBuffer, static_cast<uint32_t>(vertexOffset));
			renderContext->DrawIndexed(indexCount);

			// Advance the buffer position.
			mSpriteCount += batchSize;

			sprites += batchSize;
			count -= batchSize;
		}
	}

	void XM_CALLCONV SpriteBatch::RenderSprite(SpriteInfo const * sprite, PCT_Vertex * vertices, DirectX::FXMVECTOR textureSize, DirectX::FXMVECTOR inverseTextureSize)
	{
		// Load sprite parameters into SIMD registers.
		DirectX::XMVECTOR source = XMLoadFloat4A(&sprite->source);
		DirectX::XMVECTOR destination = XMLoadFloat4A(&sprite->destination);
		DirectX::XMVECTOR color = XMLoadFloat4A(&sprite->color);
		DirectX::XMVECTOR originRotationDepth = XMLoadFloat4A(&sprite->originRotationDepth);

		float rotation = sprite->originRotationDepth.z;
		unsigned int flags = sprite->flags;

		// Extract the source and destination sizes into separate vectors.
		DirectX::XMVECTOR sourceSize = DirectX::XMVectorSwizzle<2, 3, 2, 3>(source);
		DirectX::XMVECTOR destinationSize = DirectX::XMVectorSwizzle<2, 3, 2, 3>(destination);

		// Scale the origin offset by source size, taking care to avoid overflow if the source region is zero.
		DirectX::XMVECTOR isZeroMask = DirectX::XMVectorEqual(sourceSize, DirectX::XMVectorZero());
		DirectX::XMVECTOR nonZeroSourceSize = DirectX::XMVectorSelect(sourceSize, DirectX::g_XMEpsilon, isZeroMask);

		DirectX::XMVECTOR origin = DirectX::XMVectorDivide(originRotationDepth, nonZeroSourceSize);

		// Convert the source region from texels to mod-1 texture coordinate format.
		if(flags & SpriteInfo::SourceInTexels)
		{
			source = DirectX::XMVectorMultiply(source, inverseTextureSize);
			sourceSize = DirectX::XMVectorMultiply(sourceSize, inverseTextureSize);
		} else
		{
			origin = DirectX::XMVectorMultiply(origin, inverseTextureSize);
		}

		// If the destination size is relative to the source region, convert it to pixels.
		if(!(flags & SpriteInfo::DestSizeInPixels))
		{
			destinationSize = DirectX::XMVectorMultiply(destinationSize, textureSize);
		}

		// Compute a 2x2 rotation matrix.
		DirectX::XMVECTOR rotationMatrix1;
		DirectX::XMVECTOR rotationMatrix2;

		if(rotation != 0)
		{
			float sin, cos;

			DirectX::XMScalarSinCos(&sin, &cos, rotation);

			DirectX::XMVECTOR sinV = DirectX::XMLoadFloat(&sin);
			DirectX::XMVECTOR cosV = DirectX::XMLoadFloat(&cos);

			rotationMatrix1 = DirectX::XMVectorMergeXY(cosV, sinV);
			rotationMatrix2 = DirectX::XMVectorMergeXY(DirectX::XMVectorNegate(sinV), cosV);
		} else
		{
			rotationMatrix1 = DirectX::g_XMIdentityR0;
			rotationMatrix2 = DirectX::g_XMIdentityR1;
		}

		// The four corner vertices are computed by transforming these unit-square positions.
		static DirectX::XMVECTORF32 cornerOffsets[VerticesPerSprite] =
		{
			{ { { 0, 0, 0, 0 } } },
			{ { { 1, 0, 0, 0 } } },
			{ { { 0, 1, 0, 0 } } },
			{ { { 1, 1, 0, 0 } } },
		};

		// Tricksy alert! Texture coordinates are computed from the same cornerOffsets
		// table as vertex positions, but if the sprite is mirrored, this table
		// must be indexed in a different order. This is done as follows:
		//
		//    position = cornerOffsets[i]
		//    texcoord = cornerOffsets[i ^ SpriteEffects]

		static_assert(SpriteEffects_FlipHorizontally == 1 &&
					  SpriteEffects_FlipVertically == 2, "If you change these enum values, the mirroring implementation must be updated to match");

		const unsigned int mirrorBits = flags & 3u;

		// Generate the four output vertices.
		for(size_t i = 0; i < VerticesPerSprite; i++)
		{
			// Calculate position.
			DirectX::XMVECTOR cornerOffset = DirectX::XMVectorMultiply(XMVectorSubtract(cornerOffsets[i], origin), destinationSize);

			// Apply 2x2 rotation matrix.
			DirectX::XMVECTOR position1 = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorSplatX(cornerOffset), rotationMatrix1, destination);
			DirectX::XMVECTOR position2 = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorSplatY(cornerOffset), rotationMatrix2, position1);

			// Set z = depth.
			DirectX::XMVECTOR position = DirectX::XMVectorPermute<0, 1, 7, 6>(position2, originRotationDepth);

			// Write position as a Float4, even though VertexPositionColor::position is an XMFLOAT3.
			// This is faster, and harmless as we are just clobbering the first element of the
			// following color field, which will immediately be overwritten with its correct value.
			DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4 *>(&vertices[i].position), position);

			// Write the color.
			DirectX::XMStoreFloat4(&vertices[i].color, color);

			// Compute and write the texture coordinate.
			DirectX::XMVECTOR textureCoordinate = DirectX::XMVectorMultiplyAdd(cornerOffsets[static_cast<unsigned int>(i) ^ mirrorBits], sourceSize, source);

			DirectX::XMStoreFloat2(&vertices[i].textureCoordinate, textureCoordinate);
		}
	}

	void SpriteBatch::GrowSortedSprites()
	{
		size_t previousSize = mSortedSprites.size();

		mSortedSprites.resize(mSpriteQueueCount);

		for(size_t i = previousSize; i < mSpriteQueueCount; i++)
		{
			mSortedSprites[i] = &mSpriteQueue[i];
		}
	}

	// Dynamically expands the array used to store pending sprite information.
	void SpriteBatch::GrowSpriteQueue()
	{
		// Grow by a factor of 2.
		size_t newSize = std::max(InitialQueueSize, mSpriteQueueArraySize * 2);

		// Allocate the new array.
		std::unique_ptr<SpriteInfo[]> newArray(new SpriteInfo[newSize]);

		// Copy over any existing sprites.
		for(size_t i = 0; i < mSpriteQueueCount; i++)
		{
			newArray[i] = mSpriteQueue[i];
		}

		// Replace the previous array with the new one.
		mSpriteQueue = std::move(newArray);
		mSpriteQueueArraySize = newSize;

		// Clear any dangling SpriteInfo pointers left over from previous rendering.
		mSortedSprites.clear();
	}

	inline DirectX::XMVECTOR LoadRect(_In_ RECT const * rect)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadInt4(reinterpret_cast<uint32_t const *>(rect));

		v = DirectX::XMConvertVectorIntToFloat(v, 0);

		// Convert right/bottom to width/height.
		v = DirectX::XMVectorSubtract(v, DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::g_XMZero, v));

		return v;
	}

	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::FXMVECTOR destination,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   DirectX::FXMVECTOR originRotationDepth,
									   unsigned int flags)
	{
		if(!mInBeginEndPair)
			throw std::exception("Begin must be called before Draw");


		// Get a pointer to the output sprite.
		if(mSpriteQueueCount >= mSpriteQueueArraySize)
		{
			GrowSpriteQueue();
		}

		SpriteInfo * sprite = &mSpriteQueue[mSpriteQueueCount];

		DirectX::XMVECTOR dest = destination;

		if(sourceRectangle)
		{
			// User specified an explicit source region.
			DirectX::XMVECTOR source = LoadRect(sourceRectangle);

			XMStoreFloat4A(&sprite->source, source);

			// If the destination size is relative to the source region, convert it to pixels.
			if(!(flags & SpriteInfo::DestSizeInPixels))
			{
				dest = DirectX::XMVectorPermute<0, 1, 6, 7>(dest, DirectX::XMVectorMultiply(dest, source)); // dest.zw *= source.zw
			}

			flags |= SpriteInfo::SourceInTexels | SpriteInfo::DestSizeInPixels;
		} else
		{
			// No explicit source region, so use the entire texture.
			static const DirectX::XMVECTORF32 wholeTexture = { { {0, 0, 1, 1} } };

			XMStoreFloat4A(&sprite->source, wholeTexture);
		}

		// Convert texture size
		DirectX::XMVECTOR textureSizeV = XMLoadUInt2(&textureSize);

		// Store sprite parameters.
		XMStoreFloat4A(&sprite->destination, dest);
		XMStoreFloat4A(&sprite->color, color);
		XMStoreFloat4A(&sprite->originRotationDepth, originRotationDepth);

		sprite->texture = texture;
		sprite->textureSize = textureSizeV;
		sprite->flags = flags;

		if(mSortMode == SpriteSortMode_Immediate)
		{
			// If we are in immediate mode, draw this sprite straight away.
			RenderBatch(sprite->texture, textureSizeV, &sprite, 1);
			sprite->texture.reset();
		} else {
			// Queue this sprite for later sorting and batched rendering.
			mSpriteQueueCount++;
		}
	}


	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::XMFLOAT2 const & position,
									   DirectX::FXMVECTOR color)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::XMLoadFloat2(&position), DirectX::g_XMOne); // x, y, 1, 1

		Draw(texture, textureSize, destination, nullptr, color, DirectX::g_XMZero, 0);
	}

	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::XMFLOAT2 const & position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::XMFLOAT2 const & origin,
									   float scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 4>(DirectX::XMLoadFloat2(&position), DirectX::XMLoadFloat(&scale)); // x, y, scale, scale

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorSet(origin.x, origin.y, rotation, layerDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::XMFLOAT2 const & position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::XMFLOAT2 const & origin,
									   DirectX::XMFLOAT2 const & scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(DirectX::XMLoadFloat2(&position), DirectX::XMLoadFloat2(&scale)); // x, y, scale.x, scale.y

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorSet(origin.x, origin.y, rotation, layerDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(position, DirectX::g_XMOne); // x, y, 1, 1

		Draw(texture, textureSize, destination, nullptr, color, DirectX::g_XMZero, 0);
	}


	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::FXMVECTOR position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::FXMVECTOR origin,
									   float scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 4>(position, DirectX::XMLoadFloat(&scale)); // x, y, scale, scale

		DirectX::XMVECTOR rotationDepth = DirectX::XMVectorMergeXY(DirectX::XMVectorReplicate(rotation), DirectX::XMVectorReplicate(layerDepth));

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorPermute<0, 1, 4, 5>(origin, rotationDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   DirectX::FXMVECTOR position,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::FXMVECTOR origin,
									   DirectX::GXMVECTOR scale,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = DirectX::XMVectorPermute<0, 1, 4, 5>(position, scale); // x, y, scale.x, scale.y

		DirectX::XMVECTOR rotationDepth = DirectX::XMVectorMergeXY(DirectX::XMVectorReplicate(rotation), DirectX::XMVectorReplicate(layerDepth));

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorPermute<0, 1, 4, 5>(origin, rotationDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects));
	}


	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   RECT const & destinationRectangle,
									   DirectX::FXMVECTOR color)
	{
		DirectX::XMVECTOR destination = LoadRect(&destinationRectangle); // x, y, w, h

		Draw(texture, textureSize, destination, nullptr, color, DirectX::g_XMZero, SpriteInfo::DestSizeInPixels);
	}


	void XM_CALLCONV SpriteBatch::Draw(ResourceViewsRef texture,
									   DirectX::XMUINT2 const & textureSize,
									   RECT const & destinationRectangle,
									   RECT const * sourceRectangle,
									   DirectX::FXMVECTOR color,
									   float rotation,
									   DirectX::XMFLOAT2 const & origin,
									   SpriteEffects effects,
									   float layerDepth)
	{
		DirectX::XMVECTOR destination = LoadRect(&destinationRectangle); // x, y, w, h

		DirectX::XMVECTOR originRotationDepth = DirectX::XMVectorSet(origin.x, origin.y, rotation, layerDepth);

		Draw(texture, textureSize, destination, sourceRectangle, color, originRotationDepth, static_cast<unsigned int>(effects) | SpriteInfo::DestSizeInPixels);
	}

}
