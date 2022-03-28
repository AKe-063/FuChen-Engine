#include "stdafx.h"
#include "DXPrimitive.h"

DXPrimitive::DXPrimitive()
{
}

DXPrimitive::~DXPrimitive()
{
}

int DXPrimitive::GetIndex()
{
	return objCBIndex;
}

void DXPrimitive::SetIndex(const int& index)
{
	objCBIndex = index;
}

MeshGeometry& DXPrimitive::GetMeshGeometryInfo()
{
	return geo;
}

ObjectConstants& DXPrimitive::GetObjConstantInfo()
{
	return objConstant;
}
