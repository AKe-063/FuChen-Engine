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
	//vec4 Normal;

	void operator=(const FVector& vector)
	{
		Pos = vec3(vector.x, vector.y, vector.z);
// 		float x = (float)(rand() / (float)RAND_MAX);
// 		float y = (float)(rand() / (float)RAND_MAX);
// 		float z = (float)(rand() / (float)RAND_MAX);
// 		Color = vec4(x,y,z,1.0f);
	}

	void operator=(const FVector4& vector)
	{
		//Normal = XMFLOAT4(vector.x, vector.y, vector.z, vector.w);
		Color = vec4(vector.x * 0.5f + 0.5f, vector.y * 0.5f + 0.5f, vector.z * 0.5f + 0.5f, 1.0f);
	}
};

struct ObjectConstants
{
	mat4 WorldViewProj = MathHelper::Identity4x4();
};

struct PassConstants
{
	glm::mat4x4 View = MathHelper::Identity4x4();
	glm::mat4x4 InvView = MathHelper::Identity4x4();
	glm::mat4x4 Proj = MathHelper::Identity4x4();
	glm::mat4x4 InvProj = MathHelper::Identity4x4();
	glm::mat4x4 ViewProj = MathHelper::Identity4x4();
	glm::mat4x4 InvViewProj = MathHelper::Identity4x4();
	glm::vec3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	glm::vec2 RenderTargetSize = { 0.0f, 0.0f };
	glm::vec2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	glm::vec4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};