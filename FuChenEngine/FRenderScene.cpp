#include "stdafx.h"
#include "FRenderScene.h"

FRenderScene::FRenderScene()
{

}

FRenderScene::~FRenderScene()
{
	
}

void FRenderScene::AddPrimitive(FPrimitive* primitive)
{
	std::shared_ptr<FPrimitive> pri(primitive);
	fPrimitives.push_back(std::move(pri));
}

FPrimitive& FRenderScene::GetPrimitive(const int& index)
{
	return *fPrimitives[index];
}

int FRenderScene::GetNumPrimitive()
{
	return fPrimitives.size();
}
