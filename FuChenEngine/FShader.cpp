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
