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

void FActor::AddMesh(FMesh mesh)
{
	fMesh.insert({ mesh.GetName(),mesh });
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

FMesh FActor::GetFMeshByName(std::string name)
{
	return fMesh[name];
}

