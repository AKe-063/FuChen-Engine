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
	vec4 Normal;

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
	mat4 WorldViewProj = MathHelper::Identity4x4();
	float time = 1.0f;
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