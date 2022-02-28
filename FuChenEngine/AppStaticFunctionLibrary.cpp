#include "stdafx.h"
#include "AppStaticFunctionLibrary.h"

App* AppStaticFunctionLibrary::GetApp()
{
#if _Win32App
	std::unique_ptr<HINSTANCE> hInstance = std::make_unique<HINSTANCE>();
	return new Win32App(*hInstance);
#endif
}
