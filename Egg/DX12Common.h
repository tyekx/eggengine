#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <wrl.h>

template<typename T>
using com_ptr = Microsoft::WRL::ComPtr<T>;

namespace Egg::Graphics::DX12 {
	/*
	Converts the feature level enum to its readable format
	*/
	const char * FeatureLevelToString(D3D_FEATURE_LEVEL ftlvl);

	/*
	Queries the available video adapters from the OS
	*/
	void GetAdapters(IDXGIFactory6 * dxgiFactory, std::vector<com_ptr<IDXGIAdapter1>> & adapters);

	/*
	Prints a Blob's content as string to the Visual Studio Output window, if the blob is null, a message will still appear
	*/
	void DebugPrintBlob(com_ptr<ID3DBlob> blob);
}

namespace Egg::Internal {

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

#define DX_API(msg, ...) Egg::Internal::HResultTester(msg, __FILE__, __LINE__, __VA_ARGS__) <<