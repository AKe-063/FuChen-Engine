#pragma once
#include "FActor.h"
#include "Camera.h"

class FScene : public FSingleton<FScene>
{
public:
	FScene();
	FScene(const std::unordered_map<std::string, FActor>& actors);
	virtual ~FScene();

	Camera* GetCamera();

	bool LoadAllActors(const std::string& filePath);
	std::vector<std::string> GetNames();
	void AddNewActor(const std::string& name);
	void DelAActor(const std::string& name);
	std::unordered_map<std::string, FActor>& GetAllActor();
	std::vector<std::string> GetDirtyActor();
	void EraseDirtyActorByIndex(const int& index);

	void Update();

private:
	std::unique_ptr<Camera> mCamera;
	std::vector<std::string> names;
	std::vector<std::string> actorCount;
	std::unordered_map<std::string, FActor> actors;
	std::vector<std::string> dirtyActor;
};
