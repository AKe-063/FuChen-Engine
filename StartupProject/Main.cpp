/*#include "Common/d3dUtil.h"
#include "d3d9helper.h"
#include "wrl/client.h"
#include "Common/d3dApp.h"
#include "Common/GameTimer.h"
#include "Common/d3dUtil.h"*/
#include "stdafx.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

_Use_decl_annotations_
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		InitDirect3DApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}
