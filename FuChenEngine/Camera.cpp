#include "stdafx.h"
#include "Camera.h"

using namespace glm;

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

	mat4 P = perspectiveFovLH(mFovY, 800.0f, 600.0f, mNearZ, mFarZ);
	mProj = P;
}

void Camera::SetPosition(const vec4& position)
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

void Camera::SetProj(const mat4& proj)
{
	mProj = proj;
}

void Camera::Pitch(const float& angle)
{
	mUp = rotate(mUp, angle, mRight);
	mLook = rotate(mLook, angle, mRight);

	mViewDirty = true;
}

void Camera::Yaw(const float& angle)
{
	mRight = rotate(mRight, angle, mUp);
	mLook = rotate(mLook, angle, mUp);

	mViewDirty = true;
}

void Camera::Roll(const float& angle)
{
	mRight = rotate(mRight, angle, mLook);
	mUp = rotate(mUp, angle, mLook);

	mViewDirty = true;
}

void Camera::UpdateViewMatrix()
{
	if (mViewDirty)
	{
		vec4 R = vec4(mRight, 0);
		vec4 U = vec4(mUp, 0);
		vec4 L = vec4(mLook, 0);
		vec4 P = mPosition;

		// Keep camera's axes orthogonal to each other and of unit length.
		L = normalize(L);
		U = vec4(normalize(cross(vec3(L), vec3(R))),0);

		// U, L already ortho-normal, so no need to normalize cross product.
		R = vec4(cross(vec3(U), vec3(L)),0);

		// Fill in the view matrix entries.
		float x = -dot(P, R);
		float y = -dot(P, U);
		float z = -dot(P, L);

		mRight = R;
		mUp = U;
		mLook = L;

		mView[0][0] = mRight.x;
		mView[1][0] = mRight.y;
		mView[2][0] = mRight.z;
		mView[3][0] = x;

		mView[0][1] = mUp.x;
		mView[1][1] = mUp.y;
		mView[2][1] = mUp.z;
		mView[3][1] = y;

		mView[0][2] = mLook.x;
		mView[1][2] = mLook.y;
		mView[2][2] = mLook.z;
		mView[3][2] = z;

		mView[0][3] = 0.0f;
		mView[1][3] = 0.0f;
		mView[2][3] = 0.0f;
		mView[3][3] = 1.0f;

		mViewDirty = false;
	}
}

void Camera::InitialView()
{
	float mTheta = 1.5f * pi<float>();
	float mPhi = quarter_pi<float>();
	float mRadius = 5000.0f;
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);
	vec4 position(-x, -y, -z, 1.0f );
	SetPosition(position);

	vec4 up = vec4(mUp,0);
	mView = lookAtLH(vec3(mPosition), vec3(mTarget), vec3(up));
	
	vec3 RUL[3];
	 for (int i = 0; i < 3; i++)
	 {
		 RUL[i] = vec3(mView[0][i], mView[1][i], mView[2][i]);
	 }

	 mRight = RUL[0];
	 mUp = RUL[1];
	 mLook = RUL[2];
}

void Camera::Walk(const float& d)
{
	vec4 s = vec4(d,d,d,d);
	vec4 l = vec4(mLook,0);
	vec4 p = mPosition;
	vec3 position;
	position = s * l + p;
	mPosition = vec4(position,0);

	mViewDirty = true;
}

void Camera::Strafe(const float& d)
{
	vec4 s = vec4(d, d, d, d);
	vec4 r = vec4(mRight, 0);
	vec4 p = mPosition;
	vec3 position;
	position = s * r + p;
	mPosition = vec4(position, 0);

	mViewDirty = true;
}

mat4 Camera::GetProj()
{
	return mProj;
}

mat4 Camera::GetView()
{
	return mView;
}

mat4x4 Camera::GetProj4x4()
{
	return mProj;
}

mat4x4 Camera::GetView4x4()
{
	return mView;
}
