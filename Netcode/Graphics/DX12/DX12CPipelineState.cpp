#include "DX12CPipelineState.h"
#include "DX12Helpers.h"
#include "DX12RootSignature.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	void CPipelineStateDesc::FillNativeDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC & cpsd)
	{
		cpsd.CachedPSO.pCachedBlob = nullptr;
		cpsd.CachedPSO.CachedBlobSizeInBytes = 0;
		cpsd.CS = GetNativeBytecode(CS.get());
		cpsd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		cpsd.NodeMask = 0;
		cpsd.pRootSignature = std::dynamic_pointer_cast<DX12::RootSignatureImpl>(rootSignature)->GetNativeRootSignature();
	}

	bool CPipelineStateDesc::operator==(const CPipelineStateDesc & rhs) const
	{
		return rootSignature == rhs.rootSignature && CS == rhs.CS;
	}

	CPipelineState::CPipelineState(ID3D12Device * device, CPipelineStateDesc && cpsoDesc) : pso{ nullptr }, psoDesc { std::move(cpsoDesc) }
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC cpsd;
		psoDesc.FillNativeDesc(cpsd);

		device->CreateComputePipelineState(&cpsd, IID_PPV_ARGS(pso.GetAddressOf()));
	}

	Ptr<ID3D12PipelineState> CPipelineState::GetNativePipelineState() const
	{
		return pso.Get();
	}

	const CPipelineStateDesc & CPipelineState::GetDesc() const
	{
		return psoDesc;
	}

	void * CPipelineState::GetImplDetail() const
	{
		return GetNativePipelineState();
	}

}
