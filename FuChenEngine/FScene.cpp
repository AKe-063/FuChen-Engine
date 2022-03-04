#include "stdafx.h"
#include "FScene.h"

FScene::FScene()
{

}

FScene::FScene(const std::unordered_map<std::string, FActor>& actors)
{
	this->actors = actors;
}

FScene::~FScene()
{

}

void FScene::AddNewActor(const std::string& name, const FActor& newActor)
{
	actors.insert(std::make_pair(name, newActor));
}

void FScene::DelAActor(const std::string& name)
{
	std::unordered_map<std::string, FActor>::iterator it = actors.find(name);
	if (it == actors.end())
	{
		OutputDebugString(L"Can't find actor!");
	}
	else
	{
		actors.erase(it);
	}
}

std::unordered_map<std::string, FActor>& FScene::GetAllActor()
{
	return actors;
}

