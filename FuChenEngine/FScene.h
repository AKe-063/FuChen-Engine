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
	void AddNewActor(const std::string& name, const FActor& newActor);
	void DelAActor(const std::string& name);
	std::unordered_map<std::string, FActor>& GetAllActor();

	void Update();

protected:
	std::unordered_map<std::string, FActor> actors;

private:
	std::unique_ptr<Camera> mCamera;
	std::vector<std::string> names;
};
