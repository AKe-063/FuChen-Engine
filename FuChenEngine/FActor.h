#pragma once
#include "MeshDescribe.h"

class FActor
{
public:
	FActor(const ActorInfo actorIn);
	FActor(const std::string& name, std::vector<FMeshInfoStruct>);
	~FActor();

protected:
	ActorInfo actor;

private:

};