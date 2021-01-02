#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <cstdlib>

#include <iostream>
#include <fstream>

#define S_HOOKFUNC_RELATIVE 0x2
#define S_HOOKFUNC_RET 0x3
#define S_WIPE 0x1
#define S_RESTORE 0x0
#define S_VTABLEHOOK 0x4
#define S_VTABLERESTORE_HOOK 0x5

using namespace std;

typedef struct sInlineHookData
{
public:
	void* hookedRegion;
	void* functionHook;
	DWORD jumpBackAddy = 0;


	sInlineHookData(void* hookedRegion, void* functionHook, unsigned int hookLength, unsigned int hookFlags)
	{
		this->hookedRegion = hookedRegion; this->functionHook = functionHook; this->hookLength = hookLength; this->hookFlags = hookFlags;

		if (functionHook != 0)
		{
			this->jumpBackAddy = ((DWORD)hookedRegion + hookLength);
		};
	};

	unsigned int hookLength;
	unsigned int hookFlags;
};
typedef struct sVTableHookData
{
public:

	void* classBase;
	int functionIndex;
	void* hookedFunction;

	void* originalFunction;


	sVTableHookData(void* pClassBase, int pFunctionIndex, void* pHookedFunction)
	{

		classBase = pClassBase; functionIndex = pFunctionIndex; hookedFunction = pHookedFunction;
	};
};



namespace memory
{
	void DumpToFile(const char* path, void* base, unsigned int size)
	{

		std::ofstream outStream(path, std::ios::binary);

		outStream.write((char*)base, size);

		outStream.close();

		return;
	};

	DWORD FindDMAAddy(DWORD baseAddress, DWORD* offsets, unsigned int size)
	{
		DWORD DMAAddy = baseAddress;

		for (int i = 0; i <= size - 1; i++)
		{

			DMAAddy = *(DWORD*)(DMAAddy + offsets[i]);
			if (DMAAddy == NULL)
			{
				return 0;
			};
		};

		return DMAAddy;
	};
	DWORD __cdecl GetModuleBase(DWORD processID, const char* moduleName)
	{
		DWORD baseAddress = NULL;

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);

		if (snapshot != INVALID_HANDLE_VALUE)
		{

			MODULEENTRY32 moduleEntry;
			moduleEntry.dwSize = sizeof(MODULEENTRY32);

			if (Module32First(snapshot, &moduleEntry))
			{
				//-- Do while so it'll check the Module32First before continuing the Module32Next loop.
				do
				{
					if (!strcmp(moduleName, moduleEntry.szModule))
					{
						return (DWORD)moduleEntry.modBaseAddr;
					};
				} while (Module32Next(snapshot, &moduleEntry));
			};
		};

		return 0;
	};
	int __cdecl sInlineHook(sInlineHookData instructionHookData)
	{
		if (instructionHookData.hookLength < 6)
		{
			return 0;
		};

		DWORD oldMemProtection;
		VirtualProtect(instructionHookData.hookedRegion, instructionHookData.hookLength, PAGE_EXECUTE_READWRITE, &oldMemProtection);
		if (instructionHookData.hookFlags == S_HOOKFUNC_RELATIVE)
		{
			DWORD relativeAddress = (((DWORD)instructionHookData.functionHook - (DWORD)instructionHookData.hookedRegion) - 0x5);
			std::memset(instructionHookData.hookedRegion, '\x90', instructionHookData.hookLength);

			*(unsigned char*)instructionHookData.hookedRegion = ('\xE9');
			*(DWORD*)((DWORD)instructionHookData.hookedRegion + 0x1) = (relativeAddress);
		}
		else if (instructionHookData.hookFlags == S_HOOKFUNC_RET)
		{
			std::memset(instructionHookData.hookedRegion, '\x90', instructionHookData.hookLength);

			*(unsigned char*)instructionHookData.hookedRegion = ('\x68');
			*(DWORD*)((DWORD)instructionHookData.hookedRegion + 0x1) = (DWORD)(instructionHookData.functionHook);

			*(unsigned char*)((DWORD)instructionHookData.hookedRegion + 0x5) = ('\xC3');
		}
		else if (instructionHookData.hookFlags == S_RESTORE)
		{
			std::memset(instructionHookData.hookedRegion, '\x90', instructionHookData.hookLength);

			std::memcpy(instructionHookData.hookedRegion, instructionHookData.functionHook, instructionHookData.hookLength);
		}
		else if (instructionHookData.hookFlags == S_WIPE)
		{
			std::memset(instructionHookData.hookedRegion, '\x90', instructionHookData.hookLength);
		};
		VirtualProtect(instructionHookData.hookedRegion, instructionHookData.hookLength, oldMemProtection, &oldMemProtection);

		return 1;
	};
	int sVTABLEHOOK(sVTableHookData& sVTableHookData)
	{

		DWORD* VTABLEBase = (DWORD*)(sVTableHookData.classBase);
		DWORD functionAddress = (*(DWORD*)(VTABLEBase)+0x4 * sVTableHookData.functionIndex);

		DWORD oldProtectionFlags;
		VirtualProtect((DWORD*)functionAddress, 0x4, PAGE_EXECUTE_READWRITE, &oldProtectionFlags);

		sVTableHookData.originalFunction = (void*)*(DWORD*)(functionAddress);

		*(DWORD*)functionAddress = (DWORD)(sVTableHookData.hookedFunction);

		VirtualProtect((DWORD*)functionAddress, 0x4, oldProtectionFlags, &oldProtectionFlags);

		return 1;
	};
};

namespace maths
{

	typedef struct vector3 {

		float x, y, z;
	};

	float __cdecl getVector3Distance(vector3 src, vector3 dst)
	{

		return std::sqrtf(((src.x - dst.x) * (src.x - dst.x)) + ((src.y - dst.y) * (src.y - dst.y)) + ((src.z - dst.z) * (src.z - dst.z)));
	};

	void __cdecl calcAngle(vector3 src, vector3 dst, float* pitch, float* yaw)
	{

		*yaw = -std::atan2f(dst.x - src.x, dst.y - src.y) / 3.1415927f * 180.0f + 180.0f;
		*pitch = std::asinf((dst.z - src.z) / getVector3Distance(src, dst)) * 180.0f / 3.1415927f;

		return;
	};
};

namespace info
{
	int GetCpuInfo(char* info)
	{

		for (int _countNumber = 0x80000002; _countNumber <= 0x80000004; _countNumber++)
		{
			static int _count = 0;
			_count++;

			int buffer[4];
			__asm
			{
				xor eax, eax;
				xor ebx, ebx;
				xor ecx, ecx;
				xor edx, edx;

				mov eax, _countNumber;
				cpuid;

				mov[buffer + 0x0], eax;
				mov[buffer + 0x4], ebx;
				mov[buffer + 0x8], ecx;
				mov[buffer + 0xC], edx;
			};

			if (_count == 1)
			{

				std::memcpy(info + 0x0, buffer, 0x10);
			}
			else if (_count == 2)
			{
				std::memcpy(info + 0x10, buffer, 0x10);
			}
			else if (_count == 3)
			{
				std::memcpy(info + 0x20, buffer, 0x10);
			};
		};

		return 1;
	};
};