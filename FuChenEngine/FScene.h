#pragma once
#include "FActor.h"

class FScene
{
public:
	FScene();
	FScene(const std::unordered_map<std::string, FActor>& actors);
	virtual ~FScene();

	void AddNewActor(const std::string& name, const FActor& newActor);
	void DelAActor(const std::string& name);
	std::unordered_map<std::string, FActor>& GetAllActor();

protected:
	std::unordered_map<std::string, FActor> actors;

private:

};
