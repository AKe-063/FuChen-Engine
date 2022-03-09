#pragma once
#include "FActor.h"
#include "Camera.h"

class FScene
{
public:
	FScene();
	FScene(const std::unordered_map<std::string, FActor>& actors);
	virtual ~FScene();

	Camera* GetCamera();

	void AddNewActor(const std::string& name, const FActor& newActor);
	void DelAActor(const std::string& name);
	std::unordered_map<std::string, FActor>& GetAllActor();

	void Update();

protected:
	std::unordered_map<std::string, FActor> actors;

private:
	std::shared_ptr<Camera> mCamera;
};
