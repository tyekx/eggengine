#include "DX12ResourcePool.h"
#include <Netcode/Common.h>
#include <DirectXTex.h>
#include <Netcode/Graphics/ResourceDesc.h>
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {
	ResourcePool::ResourcePool(Ref<HeapManager> heapMan) : heapManager { std::move(heapMan) } {

	}

	Ref<DX12::Resource> ResourcePool::CreateResource(const ResourceDesc & resource) {
		return heapManager->CreateResource(resource);
	}

	Ref<DX12::Resource> ResourcePool::CreateStructuredBuffer(size_t sizeInBytes, UINT strideInBytes, ResourceType type, ResourceStates initialState, ResourceFlags flags) {
		ResourceDesc rDesc;
		rDesc.type = type;
		rDesc.dimension = ResourceDimension::BUFFER;
		rDesc.height = 1;
		rDesc.mipLevels = 1;
		rDesc.sizeInBytes = rDesc.width = sizeInBytes;
		rDesc.strideInBytes = strideInBytes;
		rDesc.format = DXGI_FORMAT_UNKNOWN;
		rDesc.depth = 1;
		rDesc.flags = flags;
		rDesc.state = initialState;

		return CreateResource(rDesc);
	}

	Ref<DX12::Resource> ResourcePool::CreateTypedBuffer(size_t sizeInBytes, DXGI_FORMAT format, ResourceType type, ResourceStates initialState, ResourceFlags flags) {
		ResourceDesc rDesc;
		rDesc.type = type;
		rDesc.dimension = ResourceDimension::BUFFER;
		rDesc.height = 1;
		rDesc.mipLevels = 1;
		rDesc.sizeInBytes = rDesc.width = sizeInBytes;
		rDesc.strideInBytes = static_cast<UINT>(DirectX::BitsPerPixel(format)) / 8U;
		rDesc.format = format;
		rDesc.depth = 1;
		rDesc.flags = flags;
		rDesc.state = initialState;

		return CreateResource(rDesc);
	}

	Ref<DX12::Resource> ResourcePool::CreateDepthBuffer(ResourceType type, UINT width, UINT height, DXGI_FORMAT format, ResourceStates initialState, ResourceFlags optFlags) {
		ResourceDesc rDesc;
		rDesc.type = type;
		rDesc.dimension = ResourceDimension::TEXTURE2D;
		rDesc.height = height;
		rDesc.width = static_cast<UINT64>(width);
		rDesc.depth = 1;
		rDesc.mipLevels = 1;
		rDesc.strideInBytes = static_cast<UINT>(DirectX::BitsPerPixel(format)) / 8U;
		rDesc.sizeInBytes = 0;
		rDesc.format = format;
		rDesc.flags = optFlags;
		rDesc.state = initialState;

		bool isFormatValid = (format == DXGI_FORMAT_D16_UNORM ||
			format == DXGI_FORMAT_D32_FLOAT ||
			format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
			format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT);

		ASSERT(isFormatValid, "Depth buffer format is invalid");

		bool hasDepthFlag = (GetNativeFlags(optFlags) & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0;

		ASSERT(hasDepthFlag, "Depth buffer must have D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL flag set");

		bool isTypeValid = (type == ResourceType::PERMANENT_DEFAULT || type == ResourceType::TRANSIENT_DEFAULT);

		ASSERT(isTypeValid, "Depth buffer must be present in the default heap");

		return CreateResource(rDesc);
	}

}
