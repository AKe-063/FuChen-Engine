#include "stdafx.h"
#include "FMaterial.h"

FMaterial::FMaterial()
{
	
}

FMaterial::~FMaterial()
{
}

std::string FMaterial::GetPSO()
{
	return fPSO;
}

std::wstring FMaterial::GetShader()
{
	return mShaderName;
}

void FMaterial::SetShader(std::wstring shaderName)
{
	mShaderName = shaderName;
}

void FMaterial::SetPSO(std::string pso)
{
	fPSO = pso;
}
