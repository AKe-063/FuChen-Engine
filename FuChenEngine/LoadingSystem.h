#pragma once

class LoadingSystem
{
public:
	LoadingSystem();
	~LoadingSystem();

	bool InitFAssetManager(const std::string& assetsPath);
	bool LoadMap(const std::string& filePath);
	bool OnAddActorKeyDown();

private:

};