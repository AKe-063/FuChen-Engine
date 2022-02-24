#pragma once

using namespace DirectX;

class Camera
{
public:
	Camera();
	~Camera();

	void SetLens(float fovY, float aspect, float zn, float zf);
	void SetPosition(const XMVECTOR& position);
	void SetView();
	void SetProj(const XMFLOAT4X4& proj);
	void Pitch(const float& angle);
	void RotateY(const float& angle);
	void UpdateViewMatrix();
	void InitialView();
	void Walk(const float& d);
	void Strafe(const float& d);

	XMMATRIX GetProj();
	XMMATRIX GetView();
	XMFLOAT4X4 GetProj4x4();
	XMFLOAT4X4 GetView4x4();

private:
	float mFovY;
	float mAspect;
	float mNearZ;
	float mFarZ;

	XMVECTOR mPosition;
	XMVECTOR mTarget = XMVectorZero();
	XMFLOAT3 mUp = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 mRight = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 mLook = { 0.0f, 0.0f, 1.0f };
	bool mViewDirty = false;
	bool mViewNeedInit = true;

	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();
};
