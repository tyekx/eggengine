#include "DX12Texture.h"
#include <Netcode/Graphics/ResourceEnums.h>

namespace Netcode::Graphics::DX12 {

	TextureImpl::TextureImpl(DirectX::ScratchImage && tData) : textureData{ std::move(tData) } { }

	ResourceDimension TextureImpl::GetDimension() const {
		switch(textureData.GetMetadata().dimension) {
			case DirectX::TEX_DIMENSION_TEXTURE1D:
				return ResourceDimension::TEXTURE1D;
			case DirectX::TEX_DIMENSION_TEXTURE2D:
				return ResourceDimension::TEXTURE2D;
			case DirectX::TEX_DIMENSION_TEXTURE3D:
				return ResourceDimension::TEXTURE3D;
			default:
				return ResourceDimension::UNKNOWN;
		}
	}

	uint16_t TextureImpl::GetMipLevelCount() const {
		return static_cast<uint16_t>(textureData.GetMetadata().mipLevels);
	}

	const Image * TextureImpl::GetImage(uint16_t mipIndex, uint16_t arrayIndex, uint32_t slice) {
		return textureData.GetImage(mipIndex, arrayIndex, slice);
	}

	const Image * TextureImpl::GetImages() {
		return textureData.GetImages();
	}

	uint16_t TextureImpl::GetImageCount() {
		return static_cast<uint16_t>(textureData.GetMetadata().arraySize);
	}

}

