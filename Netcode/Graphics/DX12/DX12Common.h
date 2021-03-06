#pragma once

#include <NetcodeFoundation/Memory.h>
#include "DX12Decl.h"

namespace Netcode::Graphics::DX12 {

	template<typename T>
	using BuilderAllocator = Memory::StdAllocatorAdapter<T, Memory::ObjectAllocator>;

	template<typename T>
	using BuilderContainer = std::vector<T, BuilderAllocator<T>>;

	void DebugDestruction(IUnknown * pUnknown);

	const char * RootSignatureVersionToString(D3D_ROOT_SIGNATURE_VERSION version);

	/*
	Converts the shader model enum to string
	*/
	const char * ShaderModelToString(D3D_SHADER_MODEL shaderModel);
	
	/*
	Converts the feature level enum to its readable format
	*/
	const char * FeatureLevelToString(D3D_FEATURE_LEVEL ftlvl);

	const char * CommandListTypeToString(D3D12_COMMAND_LIST_TYPE type);

	const char * GetBlobDataAsString(com_ptr<ID3DBlob> blob);

	/*
	Prints a Blob's content as string to the Visual Studio Output window, if the blob is null, a message will still appear
	*/
	void DebugPrintBlob(com_ptr<ID3DBlob> blob);

	std::wstring RootSigDebugName(const std::wstring & fileReference);

	std::wstring IndexedDebugName(const char * prefix, uint32_t idx);

	void DebugDestructionCallback(void * pData);
}

namespace Netcode::Detail {

	class HResultTester {
	public:
		const char * message;
		const char * file;
		const int line;
		va_list args;

		HResultTester(const char * msg, const char * file, int line, ...);

		void operator<<(HRESULT hr);
	};

}

#define DX_API(msg, ...) Netcode::Detail::HResultTester(msg, __FILE__, __LINE__, __VA_ARGS__) <<
