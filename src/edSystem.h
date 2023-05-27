#ifndef _EDSYSTEM_H
#define _EDSYSTEM_H

#include "Types.h"

struct edHeapEntry;

typedef void EdSysFunc(int, int, char*);

enum edSysHandlerType
{
	ESHT_Profile_1 = 0,
	ESHT_LoadFile = 4,
	ESHT_RenderScene = 6,
	ESHT_Sound = 7,
	ESHT_RenderUI = 10,
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

struct edSysHandlersNodeParent {
	edSysHandlersNodeTable* pNodeTable;
	edSysHandlersPoolEntry* pTypeArray_0x4[1];
};

extern edSysHandlersNodeTable g_SysHandlersNodeTable_00489170;;

namespace edSystem
{
	extern edHeapEntry* edSystemDataPtr_0042df0;
};

namespace edCSysHandlerPool
{
	bool initialize(edSysHandlersNodeTable* pNode, int count);
}

bool _edSystemInit(int argc, char** argv);
bool edSysHandlersAdd(edSysHandlersNodeTable* pNode, edSysHandlersPoolEntry** param_2, int param_3, edSysHandlerType type, EdSysFunc* pHandlerFunc, long param_6, long param_7);
bool edSysHandlersCall(int typeA, edSysHandlersPoolEntry** pPool, int eventMax, int eventID, void* param_5);

void FormatFilePath(char* dst, char* src, ...);

edHeapEntry* edmemGetMainHeader();

extern int g_isNTSC;

#endif //_EDSYSTEM_H
