#pragma once
#include "FPrimitive.h"

class FRenderScene
{
public:
	FRenderScene();
	~FRenderScene();

	void AddPrimitive(const std::string& name, std::shared_ptr<FPrimitive> primitive);
	void AddTextureResource(std::shared_ptr<FRenderTexPrimitive> tex);
	bool ContainTexture(const std::string& name);

	std::shared_ptr<FRenderTexPrimitive> GetTexByName(const std::string& name);
	FPrimitive& GetPrimitive(const std::string& name);
	std::unordered_map<std::string, std::shared_ptr<FPrimitive>>& GetAllPrimitives();
	int GetNumPrimitive();

private:
	std::unordered_map<std::string, std::shared_ptr<FPrimitive>> fPrimitives;
	std::unordered_map<std::string, std::shared_ptr<FRenderTexPrimitive>> fTextures;
};