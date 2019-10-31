#pragma once

#include "Common.h"
#include "ConstantBufferTypes.h"

namespace Egg {
	class Material {

		void InitCbAssoc() {
			if(cbAssoc == nullptr) {
				cbAssoc = std::make_unique<int[]>(16);
				memset(cbAssoc.get(), -1, 16 * sizeof(int));
			}
		}

		void DeepCopy(int * otherCbAssoc) {
			for(int i = 0; i < 16; ++i) {
				cbAssoc[i] = otherCbAssoc[i];
			}
		}
	public:
		ID3D12PipelineState * pso;
		ID3D12RootSignature * rootSignature;
		D3D12_GPU_DESCRIPTOR_HANDLE texturesDescriptor;
		UINT texturesRootSigSlot;
		D3D12_GPU_VIRTUAL_ADDRESS perMeshCbAddr;
		std::unique_ptr<int[]> cbAssoc;

		~Material() = default;
		Material() noexcept : pso{ nullptr }, rootSignature{ nullptr }, texturesDescriptor{ }, texturesRootSigSlot{ 0 }, perMeshCbAddr{ }, cbAssoc{ nullptr } {

		}

		Material(Material && m) noexcept : Material{} {
			std::swap(pso, m.pso);
			std::swap(rootSignature, m.rootSignature);
			std::swap(texturesDescriptor, m.texturesDescriptor);
			std::swap(texturesRootSigSlot, m.texturesRootSigSlot);
			std::swap(perMeshCbAddr, m.perMeshCbAddr);
			std::swap(cbAssoc, m.cbAssoc);
		}

		Material & operator=(const Material & m) {
			pso = m.pso;
			rootSignature = m.rootSignature;
			texturesDescriptor = m.texturesDescriptor;
			texturesRootSigSlot = m.texturesRootSigSlot;
			perMeshCbAddr = m.perMeshCbAddr;
			InitCbAssoc();
			DeepCopy(m.cbAssoc.get());
			return *this;
		}

		Material & operator=(Material && m) noexcept {
			std::swap(pso, m.pso);
			std::swap(rootSignature, m.rootSignature);
			std::swap(texturesDescriptor, m.texturesDescriptor);
			std::swap(texturesRootSigSlot, m.texturesRootSigSlot);
			std::swap(perMeshCbAddr, m.perMeshCbAddr);
			std::swap(cbAssoc, m.cbAssoc);
			return *this;
		}

		void SetAssoc(int typeId, int rootSigParamId) {
			InitCbAssoc();
			cbAssoc[typeId] = rootSigParamId;
		}

		inline void SetPipelineState(ID3D12GraphicsCommandList * gcl) {
			gcl->SetGraphicsRootSignature(rootSignature);
			gcl->SetPipelineState(pso);
			if(texturesDescriptor.ptr != 0) {
				gcl->SetGraphicsRootDescriptorTable(texturesRootSigSlot, texturesDescriptor);
			}
			if(cbAssoc != nullptr) {
				int cbufferid = cbAssoc[PerMeshCb::id];
				if(cbufferid != -1) {
					gcl->SetGraphicsRootConstantBufferView((UINT)cbufferid, perMeshCbAddr);
				}
			}
		}
	};

}
