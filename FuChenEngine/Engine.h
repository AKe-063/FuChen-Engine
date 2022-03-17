#pragma once
#include "MeshDescribe.h"
#include "FAssetManager.h"
#include "FScene.h"
#include "ForwardRenderer.h"
#include "Win32Window.h"
#include "FWin32Input.h"

using namespace glm;

class Engine : public FSingleton<Engine>
{
public:
	Engine();
	virtual ~Engine();

	int Run();
	bool Initialize();
	void Destroy();
	void Update();

	//Get
	ForwardRenderer* GetRenderer();
	FScene* GetFScene();
	FAssetManager* GetFAssetManager();
	Window* GetWindow();
	GameTimer* GetTimer();
	bool GetmEnginePaused();
	bool GetmEngineFinished();

protected:

private:
	bool InitWindow();
	Window* CreateAWindow();
	FInputBase* CreateInput();

	std::unique_ptr<FAssetManager> fAssetManager;
	std::unique_ptr<FScene> fScene;
	std::unique_ptr<ForwardRenderer> fRenderer;
	std::unique_ptr<FInputBase> fInput;
	std::unique_ptr<Window> mWindow;

	// Used to keep track of the delta-time?and game time (?.4).
	std::unique_ptr<GameTimer> mTimer;

	bool      mEnginePaused = false;  // is the application paused?
	bool	  mEngineFinished = false;
// 	bool      mMinimized = false;  // is the application minimized?
// 	bool      mMaximized = false;  // is the application maximized?
// 	bool      mResizing = false;   // are the resize bars being dragged?
// 	bool      mFullscreenState = false;// fullscreen enabled
};
