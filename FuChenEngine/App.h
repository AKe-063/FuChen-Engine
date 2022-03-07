#pragma once

#include <crtdbg.h>
#include "Window.h"

using namespace glm;

class App
{
protected:
    App();
    App(const App& rhs) = delete;
    App& operator=(const App& rhs) = delete;
    virtual ~App();

public:

    //static App* GetApp();
    
	//float AspectRatio()const;

	virtual int Run() = 0;
 
    virtual bool Initialize() = 0;

protected:
	//virtual void OnResize() = 0; 
	virtual void Update(const GameTimer& gt)=0;
    //virtual void Draw(const GameTimer& gt)=0;

	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled
	float mTheta = 1.5f * pi<float>();
	float mPhi = quarter_pi<float>();
	float mRadius = 500.0f;

	// Used to keep track of the delta-time?and game time (?.4).
	/*GameTimer mTimer;*/
	
	// Derived class should set these in derived constructor to customize starting values.
	/*std::wstring mMainWndCaption = L"d3d App";*/
};

