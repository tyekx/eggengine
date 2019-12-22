#pragma once

#include "DX12Common.h"
#include "DX12ResourceUploadBatch.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <functional>
#include <memory>
#include "Vertex.h"
#include "DX12CbufferAllocator.h"

//#include "RenderTargetState.h"



namespace Egg::Graphics::DX12
{
   __declspec(align(256)) struct SpriteCbuffer {
       DirectX::XMFLOAT4X4A transform;
    };

    enum SpriteSortMode
    {
        SpriteSortMode_Deferred,
        SpriteSortMode_Immediate,
        SpriteSortMode_Texture,
        SpriteSortMode_BackToFront,
        SpriteSortMode_FrontToBack,
    };

    enum SpriteEffects : uint32_t
    {
        SpriteEffects_None = 0,
        SpriteEffects_FlipHorizontally = 1,
        SpriteEffects_FlipVertically = 2,
        SpriteEffects_FlipBoth = SpriteEffects_FlipHorizontally | SpriteEffects_FlipVertically,
    };

    class SpriteBatchPipelineStateDescription
    {
    public:
        explicit SpriteBatchPipelineStateDescription(
          //  const RenderTargetState & renderTarget,
            _In_opt_ const D3D12_BLEND_DESC * blend = nullptr,
            _In_opt_ const D3D12_DEPTH_STENCIL_DESC * depthStencil = nullptr,
            _In_opt_ const D3D12_RASTERIZER_DESC * rasterizer = nullptr,
            _In_opt_ const D3D12_GPU_DESCRIPTOR_HANDLE * isamplerDescriptor = nullptr)
            :
            blendDesc(blend ? *blend : s_DefaultBlendDesc),
            depthStencilDesc(depthStencil ? *depthStencil : s_DefaultDepthStencilDesc),
            rasterizerDesc(rasterizer ? *rasterizer : s_DefaultRasterizerDesc),
            //renderTargetState(renderTarget),
            samplerDescriptor{},
            customRootSignature(nullptr),
            customVertexShader{},
            customPixelShader{}
        {
            if(isamplerDescriptor)
                this->samplerDescriptor = *isamplerDescriptor;
        }

        D3D12_BLEND_DESC            blendDesc;
        D3D12_DEPTH_STENCIL_DESC    depthStencilDesc;
        D3D12_RASTERIZER_DESC       rasterizerDesc;
        //RenderTargetState           renderTargetState;
        D3D12_GPU_DESCRIPTOR_HANDLE samplerDescriptor;
        ID3D12RootSignature * customRootSignature;
        D3D12_SHADER_BYTECODE       customVertexShader;
        D3D12_SHADER_BYTECODE       customPixelShader;

    private:
        static const D3D12_BLEND_DESC           s_DefaultBlendDesc;
        static const D3D12_RASTERIZER_DESC      s_DefaultRasterizerDesc;
        static const D3D12_DEPTH_STENCIL_DESC   s_DefaultDepthStencilDesc;
    };

    class SpriteBatch
    {


    public:
        __declspec(align(16)) struct SpriteInfo
        {
            DirectX::XMFLOAT4A source;
            DirectX::XMFLOAT4A destination;
            DirectX::XMFLOAT4A color;
            DirectX::XMFLOAT4A originRotationDepth;
            D3D12_GPU_DESCRIPTOR_HANDLE texture;
            DirectX::XMVECTOR textureSize;
            unsigned int flags;

            // Combine values from the public SpriteEffects enum with these internal-only flags.
            static const unsigned int SourceInTexels = 4;
            static const unsigned int DestSizeInPixels = 8;

            static_assert((SpriteEffects_FlipBoth & (SourceInTexels | DestSizeInPixels)) == 0, "Flag bits must not overlap");
        };

        DXGI_MODE_ROTATION mRotation;

        bool mSetViewport;
        D3D12_VIEWPORT mViewPort;

        std::unique_ptr<SpriteInfo[]> mSpriteQueue;

        size_t mSpriteQueueCount;
        size_t mSpriteQueueArraySize;

        // To avoid needlessly copying around bulky SpriteInfo structures, we leave that
    // actual data alone and just sort this array of pointers instead. But we want contiguous
    // memory for cache efficiency, so these pointers are just shortcuts into the single
    // mSpriteQueue array, and we take care to keep them in order when sorting is disabled.
        std::vector<SpriteInfo const *> mSortedSprites;


        // Mode settings from the last Begin call.
        bool mInBeginEndPair;

        SpriteSortMode mSortMode;
        com_ptr<ID3D12PipelineState> mPSO;
        com_ptr<ID3D12RootSignature> mRootSignature;
        D3D12_GPU_DESCRIPTOR_HANDLE mSampler;
        DirectX::XMMATRIX mTransformMatrix;
        com_ptr<ID3D12GraphicsCommandList> mCommandList;

        // Batched data
        
        com_ptr<ID3D12Resource> vertexBuffer;
        void * mappedVertexBuffer;

        D3D12_GPU_VIRTUAL_ADDRESS cbufferAddr;
        SpriteCbuffer * cbuffer;

        size_t mVertexPageSize;
        size_t mSpriteCount;


        static const size_t MaxBatchSize = 2048;
        static const size_t MinBatchSize = 128;
        static const size_t InitialQueueSize = 64;
        static const size_t VerticesPerSprite = 4;
        static const size_t IndicesPerSprite = 6;

        static const D3D12_SHADER_BYTECODE s_DefaultVertexShaderByteCodeStatic;
        static const D3D12_SHADER_BYTECODE s_DefaultPixelShaderByteCodeStatic;

        enum RootParameterIndex
        {
            TextureSRV,
            ConstantBuffer,
            TextureSampler,
            RootParameterCount
        };

        // Only one of these helpers is allocated per D3D device, even if there are multiple SpriteBatch instances.
        struct DeviceResources
        {
            DeviceResources(_In_ ID3D12Device * device, Resource::IResourceUploader * upload);

            com_ptr<ID3D12Resource> indexBuffer;
            D3D12_INDEX_BUFFER_VIEW indexBufferView;
            com_ptr<ID3D12RootSignature> rootSignatureStatic;
            com_ptr<ID3D12RootSignature> rootSignatureHeap;
            ID3D12Device * mDevice;

        private:
            void CreateIndexBuffer(_In_ ID3D12Device * device, Resource::IResourceUploader * upload);
            void CreateRootSignatures(_In_ ID3D12Device * device);

            static std::vector<short> CreateIndexValues();
        };

        std::unique_ptr<DeviceResources> mDeviceResources;

        SpriteBatch(_In_ ID3D12Device * device, Resource::IResourceUploader * upload, const SpriteBatchPipelineStateDescription & psoDesc, CbufferAllocator* cbufferAlloc, _In_opt_ const D3D12_VIEWPORT * viewport = nullptr);
        SpriteBatch(SpriteBatch && moveFrom) = default;
        SpriteBatch & operator= (SpriteBatch && moveFrom) = default;

        SpriteBatch(SpriteBatch const &) = delete;
        SpriteBatch & operator= (SpriteBatch const &) = delete;

        virtual ~SpriteBatch() = default;

        // Begin/End a batch of sprite drawing operations.
        void XM_CALLCONV Begin(
            _In_ ID3D12GraphicsCommandList * commandList,
            SpriteSortMode sortMode = SpriteSortMode_Deferred,
            DirectX::FXMMATRIX transformMatrix = MatrixIdentity);
        void __cdecl End();


        // Draw overloads specifying position, origin and scale as XMFLOAT2.
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::XMFLOAT2 const & position, DirectX::FXMVECTOR color = DirectX::Colors::White);
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::XMFLOAT2 const & position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::XMFLOAT2 const & position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color, float rotation, DirectX::XMFLOAT2 const & origin, DirectX::XMFLOAT2 const & scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);

        // Draw overloads specifying position, origin and scale via the first two components of an XMVECTOR.
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, DirectX::FXMVECTOR color = DirectX::Colors::White);
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::FXMVECTOR origin = DirectX::g_XMZero, float scale = 1, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR position, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color, float rotation, DirectX::FXMVECTOR origin, DirectX::GXMVECTOR scale, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);

        // Draw overloads specifying position as a RECT.
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, RECT const & destinationRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White);
        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, RECT const & destinationRectangle, _In_opt_ RECT const * sourceRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White, float rotation = 0, DirectX::XMFLOAT2 const & origin = Float2Zero, SpriteEffects effects = SpriteEffects_None, float layerDepth = 0);

        // Rotation mode to be applied to the sprite transformation
        void __cdecl SetRotation(DXGI_MODE_ROTATION mode);
        DXGI_MODE_ROTATION __cdecl GetRotation() const;

        // Set viewport for sprite transformation
        void __cdecl SetViewport(const D3D12_VIEWPORT & viewPort);

    private:
        // Implementation helper methods.
        void GrowSpriteQueue();
        void PrepareForRendering();
        void FlushBatch();
        void SortSprites();
        void GrowSortedSprites();

        void RenderBatch(D3D12_GPU_DESCRIPTOR_HANDLE texture, DirectX::XMVECTOR textureSize, _In_reads_(count) SpriteInfo const * const * sprites, size_t count);

        static void XM_CALLCONV RenderSprite(_In_ SpriteInfo const * sprite,
                                             _Out_writes_(VerticesPerSprite) PCT_Vertex * vertices,
                                             DirectX::FXMVECTOR textureSize,
                                             DirectX::FXMVECTOR inverseTextureSize);

        DirectX::XMMATRIX GetViewportTransform(_In_ DXGI_MODE_ROTATION rotation);

        void XM_CALLCONV Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSRV, DirectX::XMUINT2 const & textureSize, DirectX::FXMVECTOR destination, RECT const * sourceRectangle, DirectX::FXMVECTOR color, DirectX::FXMVECTOR originRotationDepth, unsigned int flags);

        static const DirectX::XMMATRIX MatrixIdentity;
        static const DirectX::XMFLOAT2 Float2Zero;
    };
}

