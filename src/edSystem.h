#ifndef _EDSYSTEM_H
#define _EDSYSTEM_H

#include "Types.h"

struct edHeap;

typedef void EdSysFunc(int, int, char*);

enum edSysHandlerType
{
	ESHT_LoadFile = 4,
};

struct edSysHandlersNodeTable {
	struct edSysHandlersPoolEntry* pPoolStart;
	struct edSysHandlersPoolEntry* pNextFreePoolEntry;
	int currentPoolEntries;
	int maxPoolEntries;
};

struct edSysHandlersPoolEntry {
	struct edSysHandlersPoolEntry* pNextEntry;
	EdSysFunc* pFunc;
	undefined4 field_0x8;
	undefined4 field_0xc;
};

extern edSysHandlersNodeTable g_SysHandlersNodeTable_00489170;;
extern edSysHandlersPoolEntry edSysHandlersPoolEntry_ARRAY_00469b84[2];

namespace edSystem
{
	extern edHeap* edSystemDataPtr_0042df0;
};

namespace edCSysHandlerPool
{
	bool initialize(edSysHandlersNodeTable* pNode, int count);
}

bool edSystemInit(int argc, char** argv);
bool edSysHandlersAdd(edSysHandlersNodeTable* pNode, edSysHandlersPoolEntry** param_2, int param_3, edSysHandlerType type, EdSysFunc* pHandlerFunc, long param_6, long param_7);
bool edSysHandlersCall(int typeA, edSysHandlersPoolEntry** pPool, int eventMax, int eventID, void* param_5);

void FormatFilePath(char* dst, char* src, ...);

extern int g_isNTSC;

#endif //_EDSYSTEM_H
