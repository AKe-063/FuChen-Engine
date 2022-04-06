#include "stdafx.h"
#include "DXPrimitive.h"

DXPrimitive::DXPrimitive()
{
	material = std::make_shared<FMaterial>();
}

DXPrimitive::~DXPrimitive()
{
}

int DXPrimitive::GetObjCBIndex()
{
	return objCBIndex;
}

void DXPrimitive::SetObjCBIndex(const int& index)
{
	objCBIndex = index;
}

void DXPrimitive::SetMainRsvIndex(const int& index)
{
	mainRsvHandle = index;
}

void DXPrimitive::SetNormalRsvIndex(const int& index)
{
	normalRsvHandle = index;
}

void DXPrimitive::SetMaterial(FMaterial fMaterial)
{
	*material = fMaterial;
}

int DXPrimitive::GetMainRsvIndex()
{
	return mainRsvHandle;
}

int DXPrimitive::GetNormalRsvIndex()
{
	return normalRsvHandle;
}

MeshGeometry& DXPrimitive::GetMeshGeometryInfo()
{
	return geo;
}

ObjectConstants& DXPrimitive::GetObjConstantInfo()
{
	return objConstant;
}

DXRenderTexPrimitive::~DXRenderTexPrimitive()
{

}

int DXRenderTexPrimitive::GetSrvIndex()
{
	return srvIndex;
}

void DXRenderTexPrimitive::SetSrvIndex(const int index)
{
	srvIndex = index;
}

Texture* DXRenderTexPrimitive::GetTex()
{
	return &tex;
}

void DXRenderTexPrimitive::SetTex(Texture tex)
{
	this->tex = tex;
}