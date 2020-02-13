#include "DX12ShaderBuilder.h"

namespace Egg::Graphics::DX12 {

	void ShaderBuilder::SetState(BuilderState st) {
		if(st == state) {
			return;
		}

		ASSERT(state == BuilderState::CLEAR || st == BuilderState::CLEAR, "Failed to set state");

		state = st;
	}

	void ShaderBuilder::Clear() {
		variantDesc = ShaderVariantDesc();
		SetState(BuilderState::CLEAR);
	}

	ShaderBuilder::ShaderBuilder(DX12ShaderLibraryRef shaderLibrary) : shaderLibrary{ std::move(shaderLibrary) }, variantDesc{}, state{ BuilderState::CLEAR } { }

	void ShaderBuilder::SetShaderType(ShaderType shaderType) {
		SetState(BuilderState::VARIANT);
		variantDesc.shaderType = shaderType;
	}

	void ShaderBuilder::SetEntrypoint(const std::string & entryFunction) {
		SetState(BuilderState::VARIANT);
		variantDesc.entryFunctionName = entryFunction;
	}

	void ShaderBuilder::SetSource(const std::wstring & shaderPath) {
		SetState(BuilderState::VARIANT);
		variantDesc.sourceFile = shaderPath;
	}

	void ShaderBuilder::SetDefinitions(const std::map<std::string, std::string> & defines) {
		SetState(BuilderState::VARIANT);
		variantDesc.defines = defines;
	}

	ShaderBytecodeRef ShaderBuilder::LoadBytecode(const std::wstring & precompiledShaderPath) {
		SetState(BuilderState::PRECOMPILED);
		ShaderPath path{ precompiledShaderPath };
		auto shader = shaderLibrary->LoadShader(path.GetAbsolutePath());
		Clear();
		return shader;
	}

	ShaderBytecodeRef ShaderBuilder::Build() {
		ASSERT(state == BuilderState::VARIANT, "Failed to build shader: no inputs were given");

		if(state == BuilderState::VARIANT) {
			auto variant = shaderLibrary->GetShaderVariant(variantDesc);
			Clear();
			return variant;
		} else {
			return nullptr;
		}
	}

}

