#ifndef EDLIST_H
#define EDLIST_H

#include "Types.h"
#include "edMem.h"

typedef int(edListFunc)(void*, void*);
typedef void(edListApply)(void*);

struct ShortField
{
	short type;
	ushort flags;
};

union NodeHeaderUnion
{
	byte byteFlags[4];
	short field_0x0[2];
	int count;
	void* randoPtr;
	ShortField typeField;
	int mode;
	edListFunc* pListFunc;
};

struct edNODE
{
	NodeHeaderUnion header;
	edNODE* pPrev;
	edNODE* pNext;
	void* pData;
};

struct edNODE_MANAGER
{
	char name[4];
	edNODE* pNodeHead;
	int totalCount;
	int linkCount;
};

struct edLIST : public edNODE
{
	undefined4 field_0x10;
	int nodeCount;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
#ifdef EDITOR_BUILD
	char name[128];
#endif
};

edNODE* edlistClear(edLIST* pList, edNODE* pNode);
bool edlistNewObj(edLIST* pList, void* pObj);
edNODE* edlistInsertNode(edLIST* pList, edNODE* pNode);
bool edListSetMode(edLIST* pList, int mode, edListFunc* pFunc);
edLIST* edListNew(int entryCount, int tableCount, EHeap heapID);
bool edListDel(edLIST* pList);
edNODE_MANAGER* edListCreateNodesTable(int count, EHeap heapID);
void edListDeleteNodesTables(edNODE_MANAGER* pNodeManager);
void* edListInitNodesTable(edNODE_MANAGER* pNodeManager, int count);
bool edListLink(edLIST* pList, edNODE_MANAGER* pManager, int mode, edListFunc* pFunc);
void edListClear(edLIST* pList);
bool edListAddNode(edLIST* pList, void* pNode);
void edListApplyFunc(edLIST* pList, edListApply* pFunc);

#ifdef EDITOR_BUILD
void edListSetName(edLIST* pList, char* name);
#endif

#endif // EDLIST_H
