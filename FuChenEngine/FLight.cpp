#include "stdafx.h"
#include "FLight.h"

FLight::FLight()
{

}

FLight::~FLight()
{

}

FLightDesc* FLight::GetFlightDesc()
{
	return &fLightDesc;
}

void FLight::UpdateLightTrans()
{
 	float radius = 2500.0f;
	int speed = 5;
	if (flag)
	{
		rota += speed;
		if (rota >= 2000)
		{
			flag = !flag;
		}
		fLightDesc.lightPos = glm::vec3(sqrt(4000000 - pow(rota, 2)), rota, 2000.0f);
	}
	else
	{
		rota -= speed;
		if (rota <= -2000)
		{
			flag = !flag;
		}
		fLightDesc.lightPos = glm::vec3(-sqrt(4000000 - pow(rota, 2)), rota, 2000.0f);
	}
	//fLightDesc.lightPos = glm::vec3(-2000.0f, 2000.0f, 2000.0f);
	fLightDesc.targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
	fLightDesc.lightDir = glm::normalize(fLightDesc.targetPos - fLightDesc.lightPos);
	fLightDesc.lightView = glm::lookAtLH(fLightDesc.lightPos, fLightDesc.targetPos, fLightDesc.lightUp);

	glm::vec3 sphereCenterLS = MathHelper::Vector3TransformCoord(fLightDesc.targetPos, fLightDesc.lightView);

	float l = sphereCenterLS.x - radius;
	float b = sphereCenterLS.y - radius;
	float n = sphereCenterLS.z - radius;
	float r = sphereCenterLS.x + radius;
	float t = sphereCenterLS.y + radius;
	float f = sphereCenterLS.z + radius;

	fLightDesc.lightProj = glm::orthoLH_ZO(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	glm::mat4 T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	glm::mat4 S = T * fLightDesc.lightProj * fLightDesc.lightView;
	fLightDesc.shadowTransform = S;
}
