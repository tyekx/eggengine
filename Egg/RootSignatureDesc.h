#pragma once

#include "ShaderCodeCollection.h"
#include "Common.h"

namespace Egg::Graphics::Internal {

	class RootSignatureDesc {
		std::vector<D3D12_ROOT_PARAMETER> rootParams;
		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
		D3D12_DESCRIPTOR_RANGE descRange;
		D3D12_ROOT_SIGNATURE_DESC rootSigDesc;

		bool GenerateSamplerState(const ShaderBindpoint & bp, D3D12_STATIC_SAMPLER_DESC & dst) {
			if(bp.name == "linearWrapSampler") {
				ZeroMemory(&dst, sizeof(D3D12_STATIC_SAMPLER_DESC));
				dst.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				dst.MaxLOD = 4.0f;
				dst.MinLOD = 0.0f;
				dst.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				dst.RegisterSpace = bp.space;
				dst.ShaderRegister = bp.uid;
				dst.ShaderVisibility = bp.visibility;
				return true;
			}
			return false;
		}
	public:

		RootSignatureDesc() : rootParams{}, staticSamplers{}, rootSigDesc{} { }

		RootSignatureDesc(const RootSignatureDesc & rs) : rootParams{ rs.rootParams }, staticSamplers{ rs.staticSamplers }, rootSigDesc{ rs.rootSigDesc } {

		}

		RootSignatureDesc(RootSignatureDesc && rs) noexcept : RootSignatureDesc{} {
			std::swap(rootParams, rs.rootParams);
			std::swap(staticSamplers, rs.staticSamplers);
			rootSigDesc = rs.rootSigDesc;
		}

		RootSignatureDesc & operator=(RootSignatureDesc rs) noexcept {
			std::swap(rootParams, rs.rootParams);
			std::swap(staticSamplers, rs.staticSamplers);
			rootSigDesc = rs.rootSigDesc;
			return *this;
		}

		bool operator==(const RootSignatureDesc & rs) const {
			if(rootSigDesc.NumParameters != rs.rootSigDesc.NumParameters ||
			   rootSigDesc.NumStaticSamplers != rs.rootSigDesc.NumStaticSamplers ||
			   rootSigDesc.Flags != rs.rootSigDesc.Flags) {
				return false;
			}

			for(UINT i = 0; i < rootSigDesc.NumParameters; ++i) {



				if(memcmp(rootSigDesc.pParameters + i, rs.rootSigDesc.pParameters + i, sizeof(D3D12_ROOT_PARAMETER)) != 0) {
					return false;
				}
			}

			for(UINT i = 0; i < rootSigDesc.NumStaticSamplers; ++i) {
				if(memcmp(rootSigDesc.pStaticSamplers + i, rs.rootSigDesc.pStaticSamplers + i, sizeof(D3D12_STATIC_SAMPLER_DESC)) != 0) {
					return false;
				}
			}

			return true;
		}

		inline bool operator!=(const RootSignatureDesc & rs) const {
			return !operator==(rs);
		}

		RootSignatureDesc(const ShaderBindpointCollection & bindPoints) {

			// constant buffers first
			for(const auto & i : bindPoints) {
				if(i.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER) {
					D3D12_ROOT_PARAMETER cbufferParam;
					cbufferParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
					cbufferParam.Descriptor.RegisterSpace = i.space;
					cbufferParam.Descriptor.ShaderRegister = i.uid;
					cbufferParam.ShaderVisibility = i.visibility;

					rootParams.push_back(cbufferParam);
				}
			}

			/*
			The textures are anticipated in a single descriptor heap
			*/
			int numTextures = 0;
			int minUid = 999999;
			D3D12_SHADER_VISIBILITY visibilty = D3D12_SHADER_VISIBILITY_ALL;
			for(const auto & i : bindPoints) {
				if(i.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE) {
					int uidIntVal = static_cast<int>(i.uid);

					if(numTextures == 0) {
						visibilty = i.visibility;
					}

					if(i.visibility != visibilty) {
						visibilty = D3D12_SHADER_VISIBILITY_ALL;
					}

					if(minUid > uidIntVal) {
						minUid = uidIntVal;
					}

					++numTextures;
				}
			}

			descRange.BaseShaderRegister = 0;
			descRange.NumDescriptors = numTextures;
			descRange.RegisterSpace = 0;
			descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER texturesParam;
			texturesParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			texturesParam.ShaderVisibility = visibilty;
			texturesParam.DescriptorTable.NumDescriptorRanges = 1;
			texturesParam.DescriptorTable.pDescriptorRanges = &descRange;

			if(numTextures > 0) {
				rootParams.push_back(texturesParam);
			}

			/*
			static samplers last
			*/
			for(const auto & i : bindPoints) {
				if(i.type == D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER) {
					D3D12_STATIC_SAMPLER_DESC sd;

					bool generationResult = GenerateSamplerState(i, sd);

					ASSERT(generationResult, "Failed to generate static sampler desc");

					staticSamplers.push_back(sd);
				}
			}

			rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			rootSigDesc.NumParameters = rootParams.size();
			rootSigDesc.pParameters = &(rootParams.at(0));
			rootSigDesc.NumStaticSamplers = staticSamplers.size();
			rootSigDesc.pStaticSamplers = (staticSamplers.size() > 0) ? &(staticSamplers.at(0)) : nullptr;
		}

		const D3D12_ROOT_SIGNATURE_DESC & GetDesc() const {
			return rootSigDesc;
		}
	};

}
