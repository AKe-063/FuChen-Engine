#include "stdafx.h"
#include "FMaterial.h"

FMaterial::FMaterial()
{
	fPSO = std::make_shared<FPSO>();
}

FMaterial::~FMaterial()
{
}

void FMaterial::SetShader(std::wstring shaderName)
{
	mShaderName = shaderName;
}
