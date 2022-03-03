#include "Pch.h"

HMODULE g_hModule = NULL;

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,	// handle to DLL module 
    DWORD fdwReason,	// reason for calling function 
    LPVOID				// reserved 
   )
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hModule = hinstDLL;
		break;
	case DLL_PROCESS_DETACH:
		break;
	default:
		return TRUE;
	}
	return TRUE;
}
 
