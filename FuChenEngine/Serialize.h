#pragma once
#include "MeshDescribe.h"
#include "FLight.h"

class Serialize
{
public:
	Serialize();

	AssetInfo DeserializeAssetInfo(const std::string& name);
	ActorInfo DeserializeActorInfo(const std::string& name);
	FLight DeserialzeLightInfo(const std::string& name);
protected:

};