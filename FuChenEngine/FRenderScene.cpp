#include "stdafx.h"
#include "FRenderScene.h"

FRenderScene::FRenderScene()
{

}

FRenderScene::~FRenderScene()
{
	
}

void FRenderScene::AddPrimitive(std::shared_ptr<FPrimitive> primitive)
{
	fPrimitives.push_back(primitive);
}

FPrimitive& FRenderScene::GetPrimitive(const int& index)
{
	return *fPrimitives[index];
}

int FRenderScene::GetNumPrimitive()
{
	return (int)fPrimitives.size();
}