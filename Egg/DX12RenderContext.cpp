#include "DX12RenderContext.h"
#include "DX12SpriteFont.h"

namespace Egg::Graphics::DX12 {
	void RenderContext::SetStencilReference(uint8_t stencilValue) {
		gcl->OMSetStencilRef(stencilValue);
	}

	void RenderContext::SetVertexBuffer(uint64_t handle)
	{
		const GResource & res = resources->GetNativeResource(handle);

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = res.address;
		vbv.SizeInBytes = static_cast<UINT>(res.desc.sizeInBytes);
		vbv.StrideInBytes = res.desc.strideInBytes;

		gcl->IASetVertexBuffers(0, 1, &vbv);
	}

	void RenderContext::SetVertexBuffer(uint64_t handle, uint32_t vertexOffset)
	{
		const GResource & res = resources->GetNativeResource(handle);

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.StrideInBytes = res.desc.strideInBytes;
		vbv.BufferLocation = res.address + vertexOffset * vbv.StrideInBytes;
		vbv.SizeInBytes = static_cast<UINT>(res.desc.sizeInBytes);

		gcl->IASetVertexBuffers(0, 1, &vbv);
	}

	void RenderContext::SetIndexBuffer(uint64_t handle)
	{
		const GResource & res = resources->GetNativeResource(handle);
		
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = res.address;
		ibv.Format = res.desc.format;
		ibv.SizeInBytes = static_cast<UINT>(res.desc.sizeInBytes);

		gcl->IASetIndexBuffer(&ibv);
	}

	void RenderContext::DrawIndexed(uint64_t indexCount)
	{
		gcl->DrawIndexedInstanced(static_cast<UINT>(indexCount), 1, 0, 0, 0);
	}

	void RenderContext::Draw(uint64_t vertexCount)
	{
		gcl->DrawInstanced(static_cast<UINT>(vertexCount), 1, 0, 0);
	}

	void RenderContext::SetRootSignature(RootSignatureRef rs) {
		gcl->SetGraphicsRootSignature(reinterpret_cast<ID3D12RootSignature *>(rs->GetImplDetail()));
	}

	void RenderContext::SetPipelineState(PipelineStateRef pso)
	{
		gcl->SetPipelineState(reinterpret_cast<ID3D12PipelineState*>(pso->GetImplDetail()));
	}

	void RenderContext::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		gcl->IASetPrimitiveTopology(GetNativePrimitiveTopology(topology));
	}

	void RenderContext::ClearUnorderedAccessViewUint(uint64_t handle, const DirectX::XMUINT4 & values)
	{
		const GResource & res = resources->GetNativeResource(handle);

		const auto [cpuDesc, gpuDesc] = descHeaps->CreateBufferUAV(res);

		gcl->ClearUnorderedAccessViewUint(gpuDesc, cpuDesc, res.resource, &values.x, 0, nullptr);
	}


	void RenderContext::ClearDepthOnly() {
		gcl->ClearDepthStencilView(currentlyBoundDepth, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void RenderContext::ClearStencilOnly() {
		gcl->ClearDepthStencilView(currentlyBoundDepth, D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}

	void RenderContext::ClearDepthStencil() {
		gcl->ClearDepthStencilView(currentlyBoundDepth, D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void RenderContext::ClearRenderTarget(uint8_t idx)
	{
		static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		gcl->ClearRenderTargetView(currentlyBoundRenderTargets[idx], clearColor, 0, nullptr);
	}

	void RenderContext::ClearRenderTarget(uint8_t idx, const float * clearColor)
	{
		gcl->ClearRenderTargetView(currentlyBoundRenderTargets[idx], clearColor, 0, nullptr);
	}

	void RenderContext::SetStreamOutput(uint64_t handle)
	{
		const GResource & res = resources->GetNativeResource(handle);

		D3D12_STREAM_OUTPUT_BUFFER_VIEW sobv;
		sobv.BufferLocation = res.address;
		sobv.BufferFilledSizeLocation = streamOutput_FilledSizeLocation;
		sobv.SizeInBytes = res.desc.sizeInBytes;
		gcl->SOSetTargets(0, 1, &sobv);
	}

	void RenderContext::SetStreamOutputFilledSize(uint64_t handle, uint64_t byteOffset)
	{
		const GResource & res = resources->GetNativeResource(handle);

		streamOutput_FilledSizeLocation = res.address + ((byteOffset + 3ull) & ~(3ull));
	}

	void RenderContext::BeginSpriteRendering()
	{
		spriteBatch->Begin();
	}

	void RenderContext::EndSpriteRendering()
	{
		spriteBatch->End();
	}

	void RenderContext::DrawString(SpriteFontRef spriteFont, const wchar_t * string, const DirectX::XMFLOAT2 & position)
	{
		DX12SpriteFontRef font = std::dynamic_pointer_cast<DX12SpriteFont>(spriteFont);
		font->DrawString(spriteBatch, string, position);
	}

	void RenderContext::ResetStreamOutput()
	{
		if(streamOutput_FilledSizeLocation > 0) {
			gcl->SOSetTargets(0, 0, nullptr);
			streamOutput_FilledSizeLocation = 0;
		}
	}

	void RenderContext::SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		D3D12_VIEWPORT vp;
		vp.TopLeftX = static_cast<float>(left);
		vp.Width = static_cast<float>(right);
		vp.TopLeftY = static_cast<float>(top);
		vp.Height = static_cast<float>(bottom);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		gcl->RSSetViewports(1, &vp);
	}

	void RenderContext::SetViewport(uint32_t width, uint32_t height)
	{
		SetViewport(0, width, 0, height);
	}

	void RenderContext::SetViewport()
	{
		gcl->RSSetViewports(1, &defaultViewport);
	}

	void RenderContext::SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		D3D12_RECT scissorRect;
		scissorRect.left = left;
		scissorRect.right = right;
		scissorRect.top = top;
		scissorRect.bottom = bottom;

		gcl->RSSetScissorRects(1, &scissorRect);
	}

	void RenderContext::SetScissorRect(uint32_t width, uint32_t height)
	{
		SetScissorRect(0, width, 0, height);
	}

	void RenderContext::SetScissorRect()
	{
		gcl->RSSetScissorRects(1, &defaultScissorRect);
	}

	void RenderContext::SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil)
	{
		uint32_t numDescriptors;
		if(renderTargets != nullptr) {
			DX12ResourceViewsRef rtvs = std::dynamic_pointer_cast<DX12ResourceViews>(renderTargets);
			numDescriptors = rtvs->GetNumDescriptors();
			for(uint32_t i = 0; i < numDescriptors; ++i) {
				currentlyBoundRenderTargets[i] = rtvs->GetCpuVisibleCpuHandle(i);
			}
		} else {
			currentlyBoundRenderTargets[0] = backbuffer;
			numDescriptors = 1;
		}

		if(depthStencil != nullptr) {
			DX12ResourceViewsRef dsv = std::dynamic_pointer_cast<DX12ResourceViews>(depthStencil);
			currentlyBoundDepth = dsv->GetCpuVisibleCpuHandle(0);
		} else {
			currentlyBoundDepth = backbufferDepth;
		}

		gcl->OMSetRenderTargets(numDescriptors, currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void RenderContext::SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil)
	{
		uint8_t k = 0;
		for(uint64_t rtHandle : handles) {
			ASSERT(rtHandle != 0, "SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil): handles array must not contain undefined (=0) handle");
			const GResource & res = resources->GetNativeResource(rtHandle);

			currentlyBoundRenderTargets[k++] = descHeaps->CreateRTV(res);
		}

		if(depthStencil == 0) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			const GResource & res = resources->GetNativeResource(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(res);
		}

		gcl->OMSetRenderTargets(static_cast<uint32_t>(handles.size()), currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void RenderContext::SetRenderTargets(uint64_t handle, uint64_t depthStencil)
	{
		if(handle == 0) {
			currentlyBoundRenderTargets[0] = backbuffer;
		} else {
			const GResource & res = resources->GetNativeResource(handle);
			currentlyBoundRenderTargets[0] = descHeaps->CreateRTV(res);
		}

		if(depthStencil == 0) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			const GResource & res = resources->GetNativeResource(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(res);
		}

		gcl->OMSetRenderTargets(1, currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}



	void RenderContext::SetShaderResources(int slot, std::initializer_list<uint64_t> shaderResourceHandles) {
		

		D3D12_GPU_DESCRIPTOR_HANDLE descriptor;
		descriptor.ptr = 0;
		
		for(uint64_t i : shaderResourceHandles) {
			const GResource & gres = resources->GetNativeResource(i);
			if(descriptor.ptr == 0) {
				descriptor = descHeaps->CreateSRV(gres);
			} else descHeaps->CreateSRV(gres);
		}

		gcl->SetGraphicsRootDescriptorTable(slot, descriptor);
	}

	void RenderContext::SetShaderResources(int slot, ResourceViewsRef resourceView)
	{
		DX12ResourceViewsRef srv = std::dynamic_pointer_cast<DX12ResourceViews>(resourceView);
		
		gcl->SetGraphicsRootDescriptorTable(slot, srv->GetGpuHandle(0));
	}

	void RenderContext::SetConstantBuffer(int slot, uint64_t cbufferHandle)
	{
		Log::Warn("SetConstantBuffer(int slot, uint64_t cbufferHandle) not implemneted");
	}

	void RenderContext::SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)
	{
		uint64_t handle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);
		cbuffers->CopyData(handle, srcData, srcDataSizeInBytes);

		gcl->SetGraphicsRootConstantBufferView(slot, cbuffers->GetNativeHandle(handle));
	}

	void RenderContext::ResourceBarrier(uint64_t handle, ResourceState before, ResourceState after)
	{
		ID3D12Resource * res = resources->GetNativeResource(handle).resource;

#if defined(EGG_DEBUG)
		const auto it = std::find_if(std::begin(barriers), std::end(barriers), [res](const D3D12_RESOURCE_BARRIER & barrier) ->bool {
			return (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) && res == barrier.Transition.pResource;
		});

		if(it != barriers.end()) {
			Log::Warn("2 resource barrier was found for the same resource, probably forgot to call FlushResourceBarriers()");
		}
#endif

		barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(res, GetNativeState(before), GetNativeState(after)));
	}

	void RenderContext::FlushResourceBarriers()
	{
		if(!barriers.empty()) {
			gcl->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
			barriers.clear();
		}
	}

	void RenderContext::BeginPass()
	{
		descHeaps->SetDescriptorHeaps(gcl);
	}

	void RenderContext::EndPass()
	{
		FlushResourceBarriers();
		ResetStreamOutput();
	}

}