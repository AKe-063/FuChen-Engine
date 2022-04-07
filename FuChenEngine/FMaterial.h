#pragma once
#include "FTexture.h"
#include "FPSO.h"

class FMaterial
{
public:
	FMaterial();
	~FMaterial();

	std::string GetPSO();

	void SetShader(std::wstring shaderName);
	void SetPSO(std::string pso);

private:
	std::unordered_map<std::string, FTexture> fTexex;
	std::string fPSO;
	std::wstring mShaderName;
};