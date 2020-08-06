#pragma once

#include <cstdint>

namespace Microsoft::WRL {

	template<typename T>
	class ComPtr;

}

namespace Netcode {

	template<typename T>
	using com_ptr = Microsoft::WRL::ComPtr<T>;

}

enum D3D_ROOT_SIGNATURE_VERSION;
enum D3D_SHADER_MODEL;
enum D3D_FEATURE_LEVEL;
enum D3D_PRIMITIVE_TOPOLOGY;
enum D3D12_RESOURCE_DIMENSION;
enum D3D12_HEAP_TYPE;
enum D3D12_HEAP_FLAGS;
enum D3D12_RESOURCE_STATES;
enum D3D12_RESOURCE_FLAGS;
enum D3D12_DESCRIPTOR_HEAP_TYPE;
enum D3D12_COMMAND_LIST_TYPE;

struct IUnknown;
struct ID3D10Blob;
struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12Device5;
struct ID3D12CommandQueue;
struct ID3D12Heap;
struct ID3D12Fence;
struct ID3D12Fence1;
struct ID3D12Debug3;
struct ID3D12Resource;
struct ID3D12PipelineState;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;
struct ID3D12GraphicsCommandList3;
struct ID3D12CommandAllocator;

struct IDXGIOutput1;
struct IDXGIFactory5;
struct IDXGIAdapter3;
struct IDXGISwapChain4;

using ID3DBlob = ID3D10Blob;
using HRESULT = long;
using D3D12_GPU_VIRTUAL_ADDRESS = uint64_t;

struct D3D12_RESOURCE_DESC;
struct D3D12_INPUT_LAYOUT_DESC;
struct D3D12_INPUT_ELEMENT_DESC;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;
struct D3D12_RENDER_TARGET_BLEND_DESC;
struct D3D12_BLEND_DESC;
struct D3D12_SHADER_BYTECODE;
struct D3D12_DEPTH_STENCIL_DESC;
struct D3D12_RASTERIZER_DESC;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;
struct D3D12_UNORDERED_ACCESS_VIEW_DESC;
struct D3D12_RENDER_TARGET_VIEW_DESC;
struct D3D12_DEPTH_STENCIL_VIEW_DESC;
struct D3D12_RESOURCE_BARRIER;
struct D3D12_VIEWPORT;
struct D3D12_CLEAR_VALUE;
struct D3D12_RESOURCE_ALLOCATION_INFO;
struct D3D12_SO_DECLARATION_ENTRY;
struct D3D12_STREAM_OUTPUT_DESC;
struct D3D12_ROOT_SIGNATURE_DESC;
struct D3D12_DESCRIPTOR_RANGE;
struct D3D12_DESCRIPTOR_RANGE1;
struct D3D12_ROOT_PARAMETER;
struct D3D12_ROOT_PARAMETER1;
struct D3D12_STATIC_SAMPLER_DESC;
struct D3D12_ROOT_SIGNATURE_DESC1;
struct D3D12_VERSIONED_ROOT_SIGNATURE_DESC;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct D3D12_COMPUTE_PIPELINE_STATE_DESC;
