#include "stdafx.h"
#include "Engine.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(780);
#endif

	try
	{
		std::unique_ptr<Engine> engine = std::make_unique<Engine>(hInstance);
		if (!engine->Initialize())
			return 0;
		if (!engine->Run())
			engine->Destroy();
		
		return 0;
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}