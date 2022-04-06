#pragma once
#include "FPrimitive.h"

class DXRenderTexPrimitive : public FRenderTexPrimitive
{
public:
	virtual ~DXRenderTexPrimitive();
	virtual int GetSrvIndex()override;
	virtual void SetSrvIndex(const int index)override;

	virtual Texture* GetTex()override;
	virtual void SetTex(Texture tex)override;

private:
	int srvIndex;
	Texture tex;
};

class DXPrimitive : public FPrimitive
{
public:
	DXPrimitive();
	virtual ~DXPrimitive();

	virtual void SetObjCBIndex(const int& index)override;
	virtual void SetMainRsvIndex(const int& index)override;
	virtual void SetNormalRsvIndex(const int& index)override;
	virtual void SetMaterial(FMaterial fMaterial)override;

	virtual int GetMainRsvIndex()override;
	virtual int GetNormalRsvIndex()override;
	virtual int GetObjCBIndex()override;
	virtual MeshGeometry& GetMeshGeometryInfo()override;
	virtual ObjectConstants& GetObjConstantInfo()override;

private:
	MeshGeometry geo;
	int objCBIndex = -1;
	int cbvHandle = -1;
	int mainRsvHandle = -1;
	int normalRsvHandle = -1;
	ObjectConstants objConstant;
	std::shared_ptr<FMaterial> material = nullptr;
};