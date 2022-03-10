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
		actorCount.push_back(str);
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

void FScene::AddNewActor(const std::string& name)
{
	if (actors.find(name) != actors.end())
	{
		std::string newName = name;
		int num = count(actorCount.begin(), actorCount.end(), "name");
		newName = newName.erase(newName.length() - 1) + (char*)num;
		FActor newActor(actors.find(name)->second);
		newActor.GetActorInfo().actorName = newName;
		actors.insert(std::make_pair(newName, newActor));
		names.push_back(newName);
		actorCount.push_back(name);
	}
	
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

