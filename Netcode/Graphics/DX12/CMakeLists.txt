cmake_minimum_required(VERSION 3.8)

target_sources(Netcode
PUBLIC
	"d3dx12.h"
	"DX12Builders.h"
	"DX12CommandList.h"
	"DX12CommandListPool.h"
	"DX12Common.h"
	"DX12ConstantBufferPool.h"
	"DX12CPipelineState.h"
	"DX12CPipelineStateBuilder.h"
	"DX12CPipelineStateLibrary.h"
	"DX12DebugContext.h"
	"DX12Decl.h"
	"DX12DynamicDescriptorHeap.h"
	"DX12Fence.h"
	"DX12FrameGraph.h"
	"DX12FrameGraphBuilder.h"
	"DX12GPipelineState.h"
	"DX12GPipelineStateBuilder.h"
	"DX12GPipelineStateLibrary.h"
	"DX12GraphicsModule.h"
	"DX12GraphicsModuleForWin32.h"
	"DX12Heap.h"
	"DX12HeapManager.h"
	"DX12Helpers.h"
	"DX12Includes.h"
	"DX12InputLayout.h"
	"DX12InputLayoutBuilder.h"
	"DX12InputLayoutLibrary.h"
	"DX12Platform.h"
	"DX12RenderContext.h"
	"DX12Resource.h"
	"DX12ResourceContext.h"
	"DX12RenderPass.h"
	"DX12ResourceDesc.h"
	"DX12ResourcePool.h"
	"DX12ResourceViews.h"
	"DX12RootSignature.h"
	"DX12RootSignatureBuilder.h"
	"DX12RootSignatureLibrary.h"
	"DX12ShaderBuilder.h"
	"DX12ShaderCompiled.h"
	"DX12ShaderCompiler.h"
	"DX12ShaderLibrary.h"
	"DX12ShaderSourceLibrary.h"
	"DX12ShaderVariant.h"
	"DX12SpriteBatch.h"
	"DX12SpriteBatchBuilder.h"
	"DX12SpriteFont.h"
	"DX12SpriteFontBuilder.h"
	"DX12SpriteFontLibrary.h"
	"DX12StreamOutput.h"
	"DX12StreamOutputBuilder.h"
	"DX12StreamOutputLibrary.h"
	"DX12Texture.h"
	"DX12TextureBuilder.h"
	"DX12TextureLibrary.h"
	"DX12FrameGraphExecutor.h"
	"DX12UploadBatch.h"

PRIVATE
	"DX12CommandListPool.cpp"
	"DX12Common.cpp"
	"DX12ConstantBufferPool.cpp"
	"DX12CPipelineState.cpp"
	"DX12CPipelineStateBuilder.cpp"
	"DX12CPipelineStateLibrary.cpp"
	"DX12DebugContext.cpp"
	"DX12DynamicDescriptorHeap.cpp"
	"DX12Fence.cpp"
	"DX12FrameGraph.cpp"
	"DX12FrameGraphBuilder.cpp"
	"DX12GPipelineState.cpp"
	"DX12GPipelineStateBuilder.cpp"
	"DX12GPipelineStateLibrary.cpp"
	"DX12GraphicsModule.cpp"
	"DX12GraphicsModuleForWin32.cpp"
	"DX12Heap.cpp"
	"DX12HeapManager.cpp"
	"DX12Helpers.cpp"
	"DX12InputLayout.cpp"
	"DX12InputLayoutBuilder.cpp"
	"DX12InputLayoutLibrary.cpp"
	"DX12Platform.cpp"
	"DX12RenderContext.cpp"
	"DX12Resource.cpp"
	"DX12ResourceContext.cpp"
	"DX12RenderPass.cpp"
	"DX12ResourceDesc.cpp"
	"DX12ResourcePool.cpp"
	"DX12ResourceViews.cpp"
	"DX12RootSignature.cpp"
	"DX12RootSignatureBuilder.cpp"
	"DX12RootSignatureLibrary.cpp"
	"DX12ShaderBuilder.cpp"
	"DX12ShaderLibrary.cpp"
	"DX12ShaderSourceLibrary.cpp"
	"DX12ShaderCompiled.cpp"
	"DX12ShaderCompiler.cpp"
	"DX12ShaderVariant.cpp"
	"DX12SpriteBatch.cpp"
	"DX12SpriteBatchBuilder.cpp"
	"DX12SpriteFont.cpp"
	"DX12SpriteFontLibrary.cpp"
	"DX12SpriteFontBuilder.cpp"
	"DX12StreamOutput.cpp"
	"DX12StreamOutputBuilder.cpp"
	"DX12StreamOutputLibrary.cpp"
	"DX12Texture.cpp"
	"DX12TextureBuilder.cpp"
	"DX12TextureLibrary.cpp"
	"DX12FrameGraphExecutor.cpp"
	"DX12UploadBatch.cpp"
)
