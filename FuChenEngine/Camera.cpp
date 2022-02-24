#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{

}

Camera::~Camera()
{

}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);
}

void Camera::SetPosition(const XMVECTOR& position)
{
	mPosition = position;
}

void Camera::SetView()
{
	if (mViewNeedInit)
	{
		InitialView();
		mViewNeedInit = false;
	}
}

void Camera::SetProj(const XMFLOAT4X4& proj)
{
	mProj = proj;
}

void Camera::Pitch(const float& angle)
{
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	mViewDirty = true;
}

void Camera::Yaw(const float& angle)
{
	//XMMATRIX R = XMMatrixRotationY(angle);
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mUp), angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	//XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));

	mViewDirty = true;
}

void Camera::Roll(const float& angle)
{
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mLook),angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));

	mViewDirty = true;
}

void Camera::UpdateViewMatrix()
{
	if (mViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&mRight);
		XMVECTOR U = XMLoadFloat3(&mUp);
		XMVECTOR L = XMLoadFloat3(&mLook);
		XMVECTOR P = mPosition;

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
		XMStoreFloat3(&mLook, L);

		mView(0, 0) = mRight.x;
		mView(1, 0) = mRight.y;
		mView(2, 0) = mRight.z;
		mView(3, 0) = x;

		mView(0, 1) = mUp.x;
		mView(1, 1) = mUp.y;
		mView(2, 1) = mUp.z;
		mView(3, 1) = y;

		mView(0, 2) = mLook.x;
		mView(1, 2) = mLook.y;
		mView(2, 2) = mLook.z;
		mView(3, 2) = z;

		mView(0, 3) = 0.0f;
		mView(1, 3) = 0.0f;
		mView(2, 3) = 0.0f;
		mView(3, 3) = 1.0f;

		mViewDirty = false;
	}
}

void Camera::InitialView()
{
// 	float x = mRadius * sinf(mPhi) * cosf(mTheta);
// 	float z = mRadius * sinf(mPhi) * sinf(mTheta);
// 	float y = mRadius * cosf(mPhi);
	XMVECTOR position = XMVectorSet( 0.0f,0.0f,-350.f,1.0f );
	//XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
	SetPosition(position);

	XMVECTOR up = XMLoadFloat3(&mUp);
	XMStoreFloat4x4(&mView, XMMatrixLookAtLH(mPosition, mTarget, up));
	
	XMFLOAT3 RUL[3];
	 for (int i = 0; i < 3; i++)
	 {
		 RUL[i] = XMFLOAT3(mView(0, i), mView(1, i), mView(2, i));
	 }

	 mRight = RUL[0];
	 mUp = RUL[1];
	 mLook = RUL[2];
}

void Camera::Walk(const float& d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&mLook);
	XMVECTOR p = mPosition;
	XMFLOAT3 position;
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, l, p));
	mPosition = XMLoadFloat3(&position);

	mViewDirty = true;
}

void Camera::Strafe(const float& d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = mPosition;
	XMFLOAT3 position;
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, r, p));
	mPosition = XMLoadFloat3(&position);

	mViewDirty = true;
}

XMMATRIX Camera::GetProj()
{
	return XMLoadFloat4x4(&mProj);
}

XMMATRIX Camera::GetView()
{
	return XMLoadFloat4x4(&mView);
}

DirectX::XMFLOAT4X4 Camera::GetProj4x4()
{
	return mProj;
}

DirectX::XMFLOAT4X4 Camera::GetView4x4()
{
	return mView;
}
