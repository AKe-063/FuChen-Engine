#include "stdafx.h"
#include "FActor.h"

FActor::FActor(const ActorInfo actorIn)
{
	actor = actorIn;
	mainTexName = "T_Chair_M";
	normalTexName = "T_Chair_N";
}

FActor::FActor(const std::string& name, std::vector<FMeshInfoStruct>)
{

}

FActor::~FActor()
{

}

ActorInfo& FActor::GetActorInfo()
{
	return actor;
}

std::string FActor::GetMainTexName()
{
	return mainTexName;
}

std::string FActor::GetNormalTexName()
{
	return normalTexName;
}

