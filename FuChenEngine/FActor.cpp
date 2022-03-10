#include "stdafx.h"
#include "FActor.h"

FActor::FActor(const ActorInfo actorIn)
{
	actor = actorIn;
}

FActor::FActor(const std::string& name, std::vector<FMeshInfoStruct>)
{

}

FActor::~FActor()
{

}

ActorInfo FActor::GetActorInfo()
{
	return actor;
}

