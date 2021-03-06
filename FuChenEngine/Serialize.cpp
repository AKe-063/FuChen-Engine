#include "stdafx.h"
#include "Serialize.h"

Serialize::Serialize()
{
	
}

AssetInfo Serialize::DeserializeAssetInfo(const std::string& name)
{
	std::string str = "../FuChenEngine/ExportFile/" + name;
	std::string filePath = str.erase(str.length() - 1) + ".dat";
	std::ifstream fin(filePath, std::ios::binary);
	AssetInfo assetInfo;
	MeshesLODInfo meshesLODInfo;
	int num;

	if (!fin.is_open())
		return assetInfo;

	fin.read((char*)&num, sizeof(int32_t));
	assetInfo.name.resize(num);
	fin.read((char*)assetInfo.name.data(), sizeof(char) * num);
	fin.read((char*)&assetInfo.numLOD, sizeof(int32_t));
	for (int i = 0; i < assetInfo.numLOD; i++)
	{
		fin.read((char*)&meshesLODInfo.numVertices, sizeof(int32_t));
		fin.read((char*)&meshesLODInfo.numTriangles, sizeof(int32_t));
		fin.read((char*)&meshesLODInfo.numIndices, sizeof(int32_t));

		fin.read((char*)&num, sizeof(int32_t));
		meshesLODInfo.vertices.resize(num);
		fin.read((char*)meshesLODInfo.vertices.data(), sizeof(FVector) * num);

		fin.read((char*)&num, sizeof(int32_t));
		meshesLODInfo.indices.resize(num);
		fin.read((char*)meshesLODInfo.indices.data(), sizeof(int32_t) * num);

		fin.read((char*)&meshesLODInfo.numTexcoords, sizeof(int32_t));

		fin.read((char*)&num, sizeof(int32_t));
		meshesLODInfo.normals.resize(num);
		fin.read((char*)meshesLODInfo.normals.data(), sizeof(FVector4) * num);

		fin.read((char*)&num, sizeof(int32_t));
		meshesLODInfo.tangentX.resize(num);
		fin.read((char*)meshesLODInfo.tangentX.data(), sizeof(FVector4) * num);

		fin.read((char*)&num, sizeof(int32_t));
		meshesLODInfo.tangentY.resize(num);
		fin.read((char*)meshesLODInfo.tangentY.data(), sizeof(FVector4) * num);

		fin.read((char*)&num, sizeof(int32_t));
		meshesLODInfo.verticeUVs.resize(num);
		fin.read((char*)meshesLODInfo.verticeUVs.data(), sizeof(FVector2D) * num);

		assetInfo.loDs.push_back(meshesLODInfo);
	}
	fin.close();
	return assetInfo;
}

ActorInfo Serialize::DeserializeActorInfo(const std::string& name)
{
	std::string str = "../FuChenEngine/ExportFile/" + name;
	std::string filePath = str + ".dat";
	std::ifstream fin(filePath, std::ios::binary);
	assert(fin.is_open());
	ActorInfo actorInfo;
	FMeshInfoStruct meshInfoStruct;
	int num;

	fin.read((char*)&actorInfo.staticMeshesNum, sizeof(int32_t));

	for (int i = 0; i < actorInfo.staticMeshesNum; i++)
	{
		fin.read((char*)&num, sizeof(int32_t));
		meshInfoStruct.name.resize(num);
		fin.read((char*)meshInfoStruct.name.data(), sizeof(char) * num);
		fin.read((char*)&meshInfoStruct.vertices, sizeof(int32_t));
		fin.read((char*)&meshInfoStruct.texcoords, sizeof(int32_t));
		fin.read((char*)&meshInfoStruct.transform, sizeof(FTransform));

		actorInfo.staticMeshes.push_back(meshInfoStruct);
	}

	fin.read((char*)&num, sizeof(int32_t));
	actorInfo.actorName.resize(num);
	fin.read((char*)actorInfo.actorName.data(), sizeof(char) * num);

	fin.close();
	return actorInfo;
}

FLight Serialize::DeserialzeLightInfo(const std::string& name)
{
	std::string str = "../FuChenEngine/ExportFile/" + name;
	std::string filePath = str + ".dat";
	std::ifstream fin(filePath, std::ios::binary);
	assert(fin.is_open());
	FLight lightInfo;
	int num;

	fin.read((char*)&num, sizeof(int32_t));
	lightInfo.GetFlightDesc()->name.resize(num);
	fin.read((char*)lightInfo.GetFlightDesc()->name.data(), sizeof(char) * num);
	FVector temp;
	fin.read((char*)&temp, sizeof(FVector));
	fin.read((char*)&temp, sizeof(FVector));
	lightInfo.GetFlightDesc()->lightDir = glm::vec3(temp.x, temp.y, temp.z);
	fin.read((char*)&lightInfo.GetFlightDesc()->lightDensity, sizeof(float));

	fin.close();

	return lightInfo;
}

