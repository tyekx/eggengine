#pragma once

#include "Resource.h"

namespace Egg::Graphics::Resource::Committed {

	class VBuffer : public AVBuffer {

		struct LODLevelResources {
			com_ptr<ID3D12Resource> resource;
			D3D12_RESOURCE_DESC resourceDesc;
		};

		std::vector<LODLevelResources> lodLevels;
		UINT stride;

	public:
		void SetStride(UINT strideInBytes);

		void AddLODLevel(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, const void * srcData, UINT64 sizeInBytes, UINT strideInBytes);

		virtual void CreateResources(ID3D12Device * device)  override;

		virtual void ReleaseResources() override;

		virtual void UploadResources(IResourceUploader * uploader)  override;

		virtual void ReleaseUploadResources() override;
	};

}
