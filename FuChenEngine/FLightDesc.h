#pragma once

struct FLightDesc 
{
	std::string name;
	glm::vec3 lightDir;
	glm::vec3 lightPos;
	glm::mat4 shadowTransform;
	float lightDensity;
	glm::vec3 targetPos;
	glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 lightView;
	glm::mat4 lightProj;
	float lightNearZ;
	float lightFarZ;
};