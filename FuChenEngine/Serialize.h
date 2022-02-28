#pragma once
#include "MeshDescribe.h"

class Serialize
{
public:
	Serialize();

	AssetInfo DeserializeAssetInfo(const std::string& name);
	ActorInfo DeserializeActorInfo(const std::string& name);
	void GetAllActorNames();
	std::vector<std::string> GetNames();

protected:
	std::vector<std::string> names;
};