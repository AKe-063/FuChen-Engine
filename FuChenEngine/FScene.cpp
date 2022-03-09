#include "stdafx.h"
#include "FScene.h"
#include "Serialize.h"

FScene::FScene()
{
	mCamera = std::make_unique<Camera>();
	mCamera->SetControlCamera();
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

bool FScene::LoadAllActors(const std::string& filePath)
{
	std::ifstream fin(filePath, std::ios::binary);
	if (!fin.is_open())
		return false;

	std::string str;
	int num = 0, len;
	fin.read((char*)&num, sizeof(int32_t));
	for (int i = 0; i < num; i++)
	{
		fin.read((char*)&len, sizeof(int32_t));
		str.resize(len);
		fin.read((char*)str.data(), sizeof(char) * len);
		names.push_back(str);
	}

	fin.close();

	std::unique_ptr<Serialize> Ar = std::make_unique<Serialize>();
	for (std::string name : names)
	{
		actors.insert({ name, Ar->DeserializeActorInfo(name) });
	}
	return true;
}

std::vector<std::string> FScene::GetNames()
{
	return names;
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
// 	Camera::GetControlCamera()->UpdateViewMatrix();
// 	Camera::GetControlCamera()->SetView();
}

