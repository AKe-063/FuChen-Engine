#include "stdafx.h"
//#include "Engine.h"
#include "GameInstance.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(185);
#endif

	try
	{
		std::unique_ptr<GameInstance> gameIns = std::make_unique<GameInstance>();
		if (!gameIns->Init())
			return 0;
		gameIns->Run();
		gameIns->Destroy();
		
		return 0;
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}