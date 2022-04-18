#include "stdafx.h"
#include "FShader.h"

FShader::FShader()
{
}

FShader::FShader(std::wstring name, ShaderCompileResult compileResult, std::vector<INPUT_ELEMENT_DESC> inputLayout)
{
	this->name = name;
	this->compileResult = compileResult;
	this->inputLayout = inputLayout;
}

FShader::~FShader()
{
}

void FShader::SetShaderName(std::wstring name)
{
	this->name = name;
}

void FShader::SetShaderLayout(std::vector<INPUT_ELEMENT_DESC> layout)
{
	inputLayout = layout;
}

std::vector<INPUT_ELEMENT_DESC> FShader::GetLayout()
{
	return inputLayout;
}

FShaderManager::FShaderManager()
{
	std::vector<INPUT_ELEMENT_DESC> mInputLayout =
	{
		{ "POSITION", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TangentY", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32A32_FLOAT, 0, 12, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TangentX", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32A32_FLOAT, 0, 28, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32A32_FLOAT, 0, 44, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32_FLOAT, 0, 60, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	AddShader(L"..\\FuChenEngine\\Shaders\\color.hlsl");
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\color.hlsl"].SetShaderLayout(mInputLayout);
	AddShader(L"..\\FuChenEngine\\Shaders\\Shadows.hlsl");
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\Shadows.hlsl"].SetShaderLayout(mInputLayout);
	AddShader(L"..\\FuChenEngine\\Shaders\\bloomsetup.hlsl");
	AddShader(L"..\\FuChenEngine\\Shaders\\bloomdown.hlsl");
	AddShader(L"..\\FuChenEngine\\Shaders\\bloomup.hlsl");
	AddShader(L"..\\FuChenEngine\\Shaders\\bloomsunmergeps.hlsl");
	AddShader(L"..\\FuChenEngine\\Shaders\\tonemapps.hlsl");
	AddShader(L"..\\FuChenEngine\\Shaders\\cyberpunk.hlsl");
	AddShader(L"..\\FuChenEngine\\Shaders\\postprocessblend.hlsl");
	mInputLayout.clear();
	mInputLayout =
	{
		{ "POSITION", 0, INPUT_FORMAT::INPUT_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomsetup.hlsl"].SetShaderLayout(mInputLayout);
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomdown.hlsl"].SetShaderLayout(mInputLayout);
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomup.hlsl"].SetShaderLayout(mInputLayout);
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\bloomsunmergeps.hlsl"].SetShaderLayout(mInputLayout);
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\tonemapps.hlsl"].SetShaderLayout(mInputLayout);
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\cyberpunk.hlsl"].SetShaderLayout(mInputLayout);
	GetShaderMap()[L"..\\FuChenEngine\\Shaders\\postprocessblend.hlsl"].SetShaderLayout(mInputLayout);
}

FShaderManager::~FShaderManager()
{

}

void FShaderManager::AddShader(std::wstring name)
{
	shaderMap.insert({ name, FShader()});
}

std::unordered_map<std::wstring, FShader>& FShaderManager::GetShaderMap()
{
	return shaderMap;
}
