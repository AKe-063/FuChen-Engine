#include "stdafx.h"
#include "FMesh.h"

FMesh::FMesh()
{
	material.SetShader(L"..\\FuChenEngine\\Shaders\\color.hlsl");
}

FMesh::~FMesh()
{

}

std::string FMesh::GetName()
{
	return name;
}

FMaterial FMesh::GetMaterial()
{
	return material;
}
