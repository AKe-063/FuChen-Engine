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
	virtual int Run() = 0;
 
    virtual bool Initialize() = 0;

protected:
	virtual void Update(const GameTimer& gt)=0;

	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled
};

