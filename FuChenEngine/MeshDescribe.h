#pragma once
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace glm;

struct FVector4
{
public:
	float x;
	float y;
	float z;
	float w;
};

struct FVector
{
public:
	float x;
	float y;
	float z;

	FVector operator+(const FVector& vector)
	{
		FVector addResult;
		addResult.x = vector.x + x;
		addResult.y = vector.y + y;
		addResult.z = vector.z + z;
		return addResult;
	}

	FVector operator*(const FVector& vector)
	{
		FVector addResult;
		addResult.x = vector.x * x;
		addResult.y = vector.y * y;
		addResult.z = vector.z * z;
		return addResult;
	}
};

struct FVector2D
{
public:
	float x;
	float y;
};

class FTransform
{
public:
	FVector4 Rotation;
	FVector Translation;
	FVector Scale3D;
};

struct MeshesLODInfo
{
	int numVertices = 0;
	int numTriangles = 0;
	int numIndices = 0;
	std::vector<FVector> vertices;
	std::vector<int32_t> indices;
	int numTexcoords ;
	std::vector<FVector4> normals;
	std::vector<FVector2D> verticeUVs;
};

struct FMeshInfoStruct
{
	std::string name;
	int vertices;
	int texcoords;
	FTransform transform;
};

struct AssetInfo
{
	std::string name;
	int numLOD;
	std::vector<MeshesLODInfo> loDs;
};

struct ActorInfo
{
	int staticMeshesNum;
	std::vector<FMeshInfoStruct> staticMeshes;
	std::string actorName;
};

struct Vertex
{
	vec3 Pos;
	vec4 Color;
	vec4 Normal;
	vec2 mUVs;

	void operator=(const FVector& vector)
	{
		Pos = vec3(vector.x, vector.y, vector.z);
	}

	void SetNormal(const FVector4& vector)
	{
		Normal = vec4(vector.x, vector.y, vector.z, vector.w);
	}

	void SetColor(const FVector4& vector)
	{
		Color = vec4(vector.x, vector.y, vector.z, vector.w);
	}
};

struct ObjectConstants
{
// 	mat4 lightProj = MathHelper::Identity4x4();
// 	mat4 lightVP = MathHelper::Identity4x4();
// 	mat4 lightOrthoVP = MathHelper::Identity4x4();
	mat4 Roatation = MathHelper::Identity4x4();
	mat4 World = MathHelper::Identity4x4();
// 	mat4 View = MathHelper::Identity4x4();
// 	mat4 Proj = MathHelper::Identity4x4();
/*	mat4 ViewProj = MathHelper::Identity4x4();*/
	float time = 1.0f;
};

struct PassConstants
{
	glm::mat4x4 InvViewProj = MathHelper::Identity4x4();
};

struct LightConstants 
{
	mat4 lightProj = MathHelper::Identity4x4();
	mat4 lightVP = MathHelper::Identity4x4();
	mat4 lightOrthoVP = MathHelper::Identity4x4();
};

struct BoundingSphere 
{
	glm::vec3 center;
	float radius;
};