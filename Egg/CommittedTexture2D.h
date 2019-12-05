#pragma once

#include "Resource.h"
#include <DirectXTex/DirectXTex.h>

namespace Egg::Graphics::Resource::Committed {


	/*
	To initialize an instance of Texture2D call:
	- Default constructor
	- CreateResources(3 arguments)
	*/
	class Texture2D : public ITexture {
		com_ptr<ID3D12Resource> resource;
		D3D12_RESOURCE_DESC resourceDesc;
		DirectX::ScratchImage scratchImage;
		bool isUploaded;
	public:

		Texture2D() noexcept;

		Texture2D & operator=(Texture2D t) noexcept;

		void SetDesc(const D3D12_RESOURCE_DESC & resDesc) noexcept;

		void CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & resDesc, DirectX::ScratchImage && sImage);

		virtual void CreateResources(ID3D12Device * device) override;

		virtual void ReleaseResources() override;

		virtual void UploadResources(IResourceUploader * uploader) override;

		virtual void ReleaseUploadResources() override;

		virtual const D3D12_RESOURCE_DESC & GetDesc() const noexcept;

		D3D12_SHADER_RESOURCE_VIEW_DESC GetSRV() const override;

		virtual ID3D12Resource * GetResource() const override;
	};

}
