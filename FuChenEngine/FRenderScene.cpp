#include "stdafx.h"
#include "FRenderScene.h"

FRenderScene::FRenderScene()
{

}

FRenderScene::~FRenderScene()
{
	
}

void FRenderScene::AddPrimitive(const std::string& name, std::shared_ptr<FPrimitive> primitive)
{
	fPrimitives.insert({name, primitive});
}

void FRenderScene::AddTextureResource(std::shared_ptr<FRenderTexPrimitive> tex)
{
	fTextures.insert({ tex->GetTex()->Name, tex });
}

bool FRenderScene::ContainTexture(const std::string& name)
{
	return fTextures.find(name) == fTextures.end() ? false : true;
}

std::shared_ptr<FRenderTexPrimitive> FRenderScene::GetTexByName(const std::string& name)
{
	return fTextures[name];
}

FPrimitive& FRenderScene::GetPrimitive(const std::string& name)
{
	return *fPrimitives[name];
}

std::unordered_map<std::string, std::shared_ptr<FPrimitive>>& FRenderScene::GetAllPrimitives()
{
	return fPrimitives;
}

int FRenderScene::GetNumPrimitive()
{
	return (int)fPrimitives.size();
}