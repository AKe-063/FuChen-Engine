#pragma once
#include "FTexture.h"

class FMaterial
{
public:
	FMaterial();
	~FMaterial();

private:
	std::unordered_map<std::string, FTexture> fTexex;

};