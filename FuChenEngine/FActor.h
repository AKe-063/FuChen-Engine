#pragma once
#include "MeshDescribe.h"
#include "FMesh.h"

class FActor
{
public:
	FActor() = default;
	FActor(const ActorInfo actorIn);
	FActor(const std::string& name, std::vector<FMeshInfoStruct>);
	~FActor();

	void AddMesh(FMesh mesh);

	ActorInfo& GetActorInfo();
	std::string GetMainTexName();
	std::string GetNormalTexName();
	FMesh& GetFMeshByName(std::string name);

private:
	ActorInfo actor;
	std::unordered_map<std::string, FMesh> fMesh;
	std::string mainTexName;
	std::string normalTexName;
};