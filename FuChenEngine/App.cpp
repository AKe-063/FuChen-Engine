#include "stdafx.h"
#include "App.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace glm;

// App* App::mApp = nullptr;
// App* App::GetApp()
// {
//     return mApp;
// }

App::App()
{
//     // Only one D3DApp can be constructed.
//     assert(mApp == nullptr);
//     mApp = this;
}

App::~App()
{
	
}

//float App::AspectRatio()const
//{
//	return static_cast<float>(mWindow->GetWidth() / mWindow->GetHeight());
//}