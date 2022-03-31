#pragma once
#include "MeshDescribe.h"

class FRenderTexPrimitive
{
public:
	virtual ~FRenderTexPrimitive();
	virtual int GetSrvIndex() = 0;
	virtual Texture* GetTex() { return nullptr; };

	virtual void SetSrvIndex(const int index) = 0;
	virtual void SetTex(Texture tex) {};

};

class FPrimitive
{
public:
	virtual ~FPrimitive();

	virtual int GetMainRsvIndex() = 0;
	virtual int GetNormalRsvIndex() = 0;
	virtual int GetObjCBIndex() = 0;
	virtual void SetObjCBIndex(const int& index) = 0;
	virtual void SetMainRsvIndex(const int& index) = 0;
	virtual void SetNormalRsvIndex(const int& index) = 0;
	virtual MeshGeometry& GetMeshGeometryInfo() = 0;
	virtual ObjectConstants& GetObjConstantInfo() = 0;
};