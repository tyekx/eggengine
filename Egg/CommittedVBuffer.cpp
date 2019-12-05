#include "CommittedVBuffer.h"

namespace Egg::Graphics::Resource::Committed {

	void VBuffer::SetStride(UINT strideInBytes) {
		stride = strideInBytes;
	}

	void VBuffer::AddLODLevel(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, const void * srcData, UINT64 sizeInBytes, UINT strideInBytes) {
		SetStride(strideInBytes);

		LODLevelResources lodRes;
		lodRes.resourceDesc = desc;
		DX_API("Failed to create dimension buffer in default heap for committed vertex buffer")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&lodRes.resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(lodRes.resource.GetAddressOf()));

		lodLevels.push_back(std::move(lodRes));
	}

	void VBuffer::CreateResources(ID3D12Device * device) {

	}

	void VBuffer::ReleaseResources() {
		lodLevels.clear();
	}

	void VBuffer::UploadResources(IResourceUploader * uploader) {
		for(auto & i : lodLevels) {
			uploader->Upload(i.resourceDesc, i.resource.Get(), nullptr, 0);
			uploader->Transition(i.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}

	void VBuffer::ReleaseUploadResources() {

	}

}
