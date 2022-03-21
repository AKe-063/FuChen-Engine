#pragma once
#include "MeshDescribe.h"

class FActor
{
public:
	FActor() = default;
	FActor(const ActorInfo actorIn);
	FActor(const std::string& name, std::vector<FMeshInfoStruct>);
	~FActor();

	ActorInfo& GetActorInfo();

protected:
	ActorInfo actor;

private:

};