
#include "jellydll.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{	
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		while(1) MessageBox(NULL, TEXT("There\'s A Rat In My GPU What I'm A Gonna Do?"), TEXT("<<Persistent Evil>>"), MB_TOPMOST | MB_ICONINFORMATION);
	}
	return TRUE;
}
