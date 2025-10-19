#include "edList.h"
#include <string.h>

#define ED_LIST_LOG(level, format, ...) MY_LOG_CATEGORY("edList", level, format, ##__VA_ARGS__)

edNODE* edlistClear(edLIST* pList, edNODE* pNode)
{
	edNODE* peVar2;
	edNODE_MANAGER* pvVar1;

	peVar2 = pNode;
	if ((edLIST*)pNode != pList) {
		peVar2 = pNode->pPrev;
		pvVar1 = (edNODE_MANAGER*)pList->pData;
		if (pNode->header.typeField.flags == 0) {
			peVar2->pNext = pNode->pNext;
			pNode->pNext->pPrev = peVar2;
			pNode->header.typeField.type = 0;
			pNode->pPrev = pvVar1->pNodeHead;
			pvVar1->pNodeHead = pNode;
			pvVar1->linkCount = pvVar1->linkCount + -1;
			pList->nodeCount = pList->nodeCount + -1;
		}
		else {
			pNode->header.typeField.type = 1;
			pNode->pData = (void*)0x0;
		}
		for (; ((edLIST*)peVar2 != pList && (*(short*)&peVar2->header == 1)); peVar2 = peVar2->pPrev) {
		}
	}
	return peVar2;
}

bool edlistNewObj(edLIST* pList, void* pObj)
{
	bool bVar1;
	edNODE* peVar2;
	edNODE_MANAGER* pvVar2;

	pvVar2 = (edNODE_MANAGER*)pList->pData;
	bVar1 = pvVar2->totalCount != pvVar2->linkCount;
	if (bVar1) {
		peVar2 = pvVar2->pNodeHead;
		peVar2->header.typeField.type = 2;
		peVar2->pData = pObj;
	}
	return bVar1;
}

edNODE* edlistInsertNode(edLIST* pList, edNODE* pNode)
{
	edNODE* pNewNode;
	edNODE_MANAGER* pNodeManager;

	pNodeManager = (edNODE_MANAGER*)pList->pData;
	pNodeManager->linkCount = pNodeManager->linkCount + 1;
	pList->nodeCount = pList->nodeCount + 1;
	pNewNode = pNodeManager->pNodeHead;
	pNodeManager->pNodeHead = pNewNode->pPrev;
	pNewNode->header.typeField.type = 0;
	pNewNode->pNext = pNode->pNext;
	pNewNode->pPrev = pNode;
	pNode->pNext->pPrev = pNewNode;
	pNode->pNext = pNewNode;
	return pNewNode;
}

bool edListSetMode(edLIST* pList, int mode, edListFunc* pFunc)
{
	if (mode == 2) {
		pList->header.pListFunc = pFunc;
	}
	else {
		pList->header.mode = mode;
	}
	return true;
}

edLIST* edListNew(int listCount, int nodeCount, EHeap heapID)
{
	bool bVar1;
	edLIST* peVar2;
	edNODE_MANAGER* pNodeManager;

	if (nodeCount == 0) {
		peVar2 = (edLIST*)edMemAlloc(heapID, listCount * sizeof(edLIST));
	}
	else {
		peVar2 = (edLIST*)edMemAlloc(heapID, nodeCount * sizeof(edNODE) + listCount * sizeof(edLIST) + sizeof(edNODE_MANAGER));
		pNodeManager = (edNODE_MANAGER*)(peVar2 + listCount);
		edListInitNodesTable(pNodeManager, nodeCount);
		while (bVar1 = listCount != 0, listCount = listCount + -1, bVar1) {
			edListLink(peVar2 + listCount, pNodeManager, 0, 0);
		}
	}
	if (peVar2 == (edLIST*)0x0) {
		peVar2 = (edLIST*)0x0;
	}

	ED_LIST_LOG(LogLevel::Info, "Created new list: list count: %d, node count: %d, heapID: %d => %p", listCount, nodeCount, heapID, (void*)peVar2);

	return peVar2;
}

// Should be in: D:/Projects/EdenLib/edSys/sources/EdList.cpp
bool edListDel(edLIST* pList)
{
	edMemFree(pList);
	return true;
}

edNODE_MANAGER* edListCreateNodesTable(int count, EHeap heapID)
{
	edNODE_MANAGER* pNodeManager;

	pNodeManager = (edNODE_MANAGER*)edMemAlloc(heapID, count * sizeof(edNODE) + sizeof(edNODE_MANAGER));
	if (pNodeManager == (edNODE_MANAGER*)0x0) {
		pNodeManager = (edNODE_MANAGER*)0x0;
	}
	else {
		edListInitNodesTable(pNodeManager, count);
	}
	return pNodeManager;
}

// Should be in: D:/Projects/EdenLib/edSys/sources/EdList.cpp
void edListDeleteNodesTables(edNODE_MANAGER* pNodeManager)
{
	edMemFree(pNodeManager);
	return;
}

const char sz_NodeTitle_00432b68[4] = { 'N', 'O', 'D', 'E' };

void* edListInitNodesTable(edNODE_MANAGER* pNodeManager, int count)
{
	void* pvVar1;
	edNODE* pNode;
	int iVar2;

	pNode = (edNODE*)(pNodeManager + 1);
	*(uint*)pNodeManager->name = *(uint*)sz_NodeTitle_00432b68;
	pNodeManager->totalCount = count;
	pNodeManager->linkCount = 0;
	pNodeManager->pNodeHead = pNode;
	pvVar1 = memset(pNode, 0, count * sizeof(edNODE));
	for (iVar2 = count + -1; iVar2 != 0; iVar2 = iVar2 + -1) {
		pNode->pPrev = pNode + 1;
		pNode = pNode + 1;
	}
	return pvVar1;
}

bool edListLink(edLIST* pList, edNODE_MANAGER* pManager, int mode, edListFunc* pFunc)
{
	bool bVar1;

	pList->pNext = (edNODE*)pList;
	pList->pPrev = (edNODE*)pList;
	pList->pData = pManager;
	pList->field_0x10 = 0;
	pList->nodeCount = 0;
	bVar1 = edListSetMode(pList, mode, pFunc);
	return bVar1;
}

// Should be in: D:/Projects/EdenLib/edSys/sources/EdList.cpp
void edListClear(edLIST* pList)
{
	edNODE* pNode;
	edNODE_MANAGER* pManager;

	if (pList->field_0x10 == 0) {
		if ((edLIST*)pList->pPrev != pList) {
			pManager = (edNODE_MANAGER*)pList->pData;
			(pList->pNext)->pPrev = pManager->pNodeHead;
			pManager->pNodeHead = pList->pPrev;
			pManager->linkCount = pManager->linkCount - pList->nodeCount;
			pList->nodeCount = 0;
			pList->pPrev = (edNODE*)pList;
			pList->pNext = (edNODE*)pList;
		}
	}
	else {
		for (pNode = pList->pPrev; (edLIST*)pNode != pList; pNode = edlistClear(pList, pNode)) {
		}
	}
	return;
}

bool edListAddNode(edLIST* pList, void* pNode)
{
	int iVar1;
	edLIST* pNode_00;
	bool bVar2;
	edNODE* peVar3;
	long lVar4;

	bVar2 = edlistNewObj(pList, pNode);
	if (bVar2 == false) {
		bVar2 = false;
	}
	else {
		iVar1 = pList->header.mode;
		if (iVar1 == 1) {
			peVar3 = edlistInsertNode(pList, (edNODE*)pList);
			pList->pNext = peVar3;
		}
		else {
			if (iVar1 == 0) {
				peVar3 = edlistInsertNode(pList, pList->pPrev);
				pList->pPrev = peVar3;
			}
			else {
				pNode_00 = (edLIST*)pList->pPrev;
				while (((pNode_00 != pList && (pNode_00->header.typeField.type != 1)) &&
					(lVar4 = pList->header.pListFunc(pNode, pNode_00->pData), -1 < lVar4))) {
					pNode_00 = (edLIST*)pNode_00->pPrev;
				}
				edlistInsertNode(pList, (edNODE*)pNode_00);
			}
		}
		bVar2 = true;
	}
	return bVar2;
}

void edListApplyFunc(edLIST* pList, edListApply* pFunc)
{
	edLIST* peVar1;

	for (peVar1 = (edLIST*)pList->pPrev; peVar1 != pList; peVar1 = (edLIST*)peVar1->pPrev) {
		if ((peVar1->header.typeField.type & 1) == 0) {
			(*pFunc)(peVar1->pData);
		}
	}
	return;
}

#ifdef EDITOR_BUILD
void edListSetName(edLIST* pList, char* name)
{
	ED_LIST_LOG(LogLevel::Info, "Set list name: %p => %s", (void*)pList, name);
	memcpy(pList->name, name, sizeof(pList->name));
}
#endif