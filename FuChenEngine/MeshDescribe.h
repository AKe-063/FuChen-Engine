#pragma once
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

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
	int numTexcoords = 0;
};

struct AssetInfo
{
	std::string name;
	int numLOD;
	std::vector<MeshesLODInfo> loDs;
};

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;

	void operator=(const FVector& vector)
	{
		Pos = XMFLOAT3(vector.x, vector.y, vector.z);
		Color = XMFLOAT4(Colors::LightGreen);
	}
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};