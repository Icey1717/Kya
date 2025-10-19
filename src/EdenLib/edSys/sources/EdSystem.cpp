#include "EdSystem.h"
#include <stdio.h>
#include "edMem.h"
#include "EdHandlers.h"

#include <stdlib.h>
#include <string.h>
#include <cstdarg>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include "ps2/_edSystem.h"
#include <libdma.h>
#include <devvif0.h>
#include <devvif1.h>
#include <devvu0.h>
#include <devvu1.h>
#include <libgraph.h>
#endif

#ifdef PLATFORM_WIN
#include <string.h>
#endif

byte g_NetIsSetup_004893d0 = 0;
int g_isNTSC = 1;

void _edSystemPrintf(int type, int eventID, char* format)
{
	// forward to sceprintf
	scePrintf(format);
}

// Should be in: D:/Projects/EdenLib/edSys/sources/ps2/_edsystem.h
void edSystemBreak(void)
{
	trap(0);
	return;
}

bool _edSystemInit(int argc, char** argv)
{
	bool bSuccess;
	EFileLoadMode EVar2;
	uint uVar3;

	g_NetIsSetup_004893d0 = 0;

#ifdef PLATFORM_WIN
	edVarMemWorkStart = (S_MAIN_MEMORY_HEADER*)_aligned_malloc(sizeof(VirtualMemory), 0x10);
	memset(edVarMemWorkStart, 0x0, sizeof(VirtualMemory));
#endif

	scePrintf("--------------------------------------\n");
	scePrintf("Code Size:     %09d  (0x%08x)\n", 0, 0);
	scePrintf("Data Size:     %09d  (0x%08x)\n", 0, 0);
	scePrintf("MemWork Size:  %09d  (0x%08x)\n", 0, 0);
	scePrintf("Stack Size:    %09d  (0x%08x)\n", 0, 0);
	scePrintf("--------------------------------------\n");

	if (edSystem.system.startAddress == (char*)0x0) {
#ifdef PLATFORM_PS2
		_edSystemPS2SpecificInit();

		if (GetFileLoadMode_00424d9c() == FLM_Net) {
			_edSystemPS2SpecificAtMonInit(argc, argv);
		}
#endif

		char* pcVar3 = edVarMemStackStart + edVarMemStackSize;

		edSystem.system.startAddress = (char*)edVarMemWorkStart;
		edSystem.system.size = edVarMemWorkSize;

#ifdef PLATFORM_PS2
		if (0x2000000 < (ulong)pcVar3) {
			edSystem.heap3.size = 0x8000000 - (int)pcVar3;
			edSystem.heap3.startAddress = pcVar3;
		}

		bSuccess = edSystem.scratchpad.startAddress != (char*)0x0;
		if (bSuccess) {
			bSuccess = edVarMemWorkStart != (S_MAIN_MEMORY_HEADER*)0x0;
		}
#else
		bSuccess = true;
#endif
	}
	else {
		scePrintf("EdSystem Error : _edSystemInit called twice");
		bSuccess = false;
	}

	return bSuccess;
}

// Should be in: D:/Projects/EdenLib/edSys/sources/ps2/_edSystem.cpp
void _edSystemTerm()
{
	exit(0);
}