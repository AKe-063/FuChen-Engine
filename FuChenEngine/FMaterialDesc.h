#pragma once
#include "FTexture.h"
#include "MeshDescribe.h"

struct FMaterialDesc
{
	std::vector<FTexture> fTextures;
	FTransform texTransform;
};