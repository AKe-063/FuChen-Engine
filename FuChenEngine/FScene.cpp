#include "stdafx.h"
#include "FScene.h"
#include "Serialize.h"

FScene::FScene()
{
	mCamera = std::make_shared<Camera>();
	mCamera->SetControlCamera();
	std::unique_ptr<Serialize> Ar = std::make_unique<Serialize>();
	std::vector<std::string> names = Ar->GetNames();
	for (std::string name : names)
	{
		actors.insert({ name, Ar->DeserializeActorInfo(name) });
	}
}

FScene::FScene(const std::unordered_map<std::string, FActor>& actors)
{
	this->actors = actors;
}

FScene::~FScene()
{

}

Camera* FScene::GetCamera()
{
	return mCamera.get();
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

void FScene::Update()
{
	GetCamera()->UpdateViewMatrix();
	GetCamera()->SetView();
}

