#pragma once

using namespace glm;

class Camera
{
public:
	Camera();
	~Camera();

	void SetLens(float fovY, float aspect, float zn, float zf);
	void SetPosition(const vec4& position);
	void SetView();
	void SetProj(const mat4& proj);
	void Pitch(const float& angle);
	void Yaw(const float& angle); 
	void Roll(const float& angle);
	void UpdateViewMatrix();
	void InitialView();
	void Walk(const float& d);
	void Strafe(const float& d);
	void SetControlCamera();
	static Camera* GetControlCamera();

	vec4 GetPosition();
	mat4 GetProj();
	mat4 GetView();
	mat4x4 GetProj4x4();
	mat4x4 GetView4x4();

private:
	float mTheta = 1.5f * pi<float>();
	float mPhi = quarter_pi<float>();
	float mRadius = 500.0f;
	float mFovY;
	float mAspect;
	float mNearZ;
	float mFarZ;

	vec4 mPosition;
	vec4 mTarget = { 0.0f, 0.0f, 0.0f, 0.0f };
	vec3 mUp = { 0.0f, 1.0f, 0.0f };
	vec3 mRight = { 1.0f, 0.0f, 0.0f };
	vec3 mLook = { 0.0f, 0.0f, 1.0f };
	bool mViewDirty = false;
	bool mViewNeedInit = true;

	mat4 mView = MathHelper::Identity4x4();
	mat4 mProj = MathHelper::Identity4x4();

	static Camera* controlCamera;
};
