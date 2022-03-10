#pragma once
#include "MeshDescribe.h"

class Serialize
{
public:
	Serialize();

	AssetInfo DeserializeAssetInfo(const std::string& name);
	ActorInfo DeserializeActorInfo(const std::string& name);

protected:

};