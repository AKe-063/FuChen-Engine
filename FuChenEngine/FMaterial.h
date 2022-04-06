#pragma once
#include "FTexture.h"
#include "FPSO.h"

class FMaterial
{
public:
	FMaterial();
	~FMaterial();

	void SetShader(std::wstring shaderName);
	void SetPSO();

private:
	std::unordered_map<std::string, FTexture> fTexex;
	std::shared_ptr<FPSO> fPSO = nullptr;
	std::wstring mShaderName;
};