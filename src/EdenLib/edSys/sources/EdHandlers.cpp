#include "EdHandlers.h"
#include "EdSystem.h"

edCSysHandlerPool edSysHandlerMainPool;
edSysHandlerSoft edSoftHandlers;

bool edSysHandlersInit(int size)
{
	bool bVar1;

	bVar1 = edSysHandlerMainPool.initialize(size);
	if (bVar1 != false) {
		edSoftHandlers.add(0, _edSystemPrintf, ED_HANDLER_SOFT_PRINT, 0);
	}
	else {
		_edSystemPrintf(0, 0, "edSysHandlersInit: Unable to initialize handler system");
		edSystemBreak();
	}
	return bVar1 != false;
}

bool edSysHandlersTerm(void)
{
	bool bVar1;
	bool bVar2;

	bVar1 = edSoftHandlers.remove(0, _edSystemPrintf);

	if (bVar1 == 0) {
		_edSystemPrintf(0, 0, "edSysHandlersTerm: Unable to terminate handler system");
		edSystemBreak();
	}

	bVar2 = edSysHandlerMainPool.terminate();

	return (bVar1 & bVar2) != false;
}

edCSysHandlerNode::edCSysHandlerNode()
{
	this->pNext = (edCSysHandlerNode*)0x0;
	this->pFunc = (EdSysFunc*)0x0;
	this->field_0x8 = 0;
	this->field_0xc = 0;

	return;
}

// ~edCSysHandlerNode

edCSysHandlerPool::edCSysHandlerPool()
{
	this->currentPoolEntries = 0;
	this->aNodes = (edCSysHandlerNode*)0x0;
	this->pNextFreeNode = (edCSysHandlerNode*)0x0;
	this->nbNodes = 0;

	return;
}

// ~edCSysHandlerPool

bool edCSysHandlerPool::initialize(int count)
{
	bool bSuccess;

	if (this->aNodes == (edCSysHandlerNode*)0x0) {
		this->nbNodes = count;

		this->aNodes = new edCSysHandlerNode[count];
		if (this->aNodes == (edCSysHandlerNode*)0x0) {
			_edSystemPrintf(0, 0, "edCSysHandlerPool::initialize: Unable to allocate resource pool");
			edSystemBreak();
			terminate();
			bSuccess = false;
		}
		else {
			this->pNextFreeNode = this->aNodes;

			int curNodeIndex;
			for (curNodeIndex = 0; curNodeIndex < count + -1; curNodeIndex = curNodeIndex + 1) {
				this->pNextFreeNode[curNodeIndex].pNext = this->pNextFreeNode + curNodeIndex + 1;
			}

			bSuccess = true;

			this->pNextFreeNode[curNodeIndex].pNext = (edCSysHandlerNode*)0x0;
		}
	}
	else {
		_edSystemPrintf(0, 0, "edCSysHandlerPool::initialize: Initialization already done");
		edSystemBreak();
		bSuccess = false;
	}

	return bSuccess;
}

bool edCSysHandlerPool::terminate()
{
	if (this->currentPoolEntries != 0) {
		_edSystemPrintf(0, 0, "edCSysHandlerPool::terminate: Some handlers were not removed");
	}

	if (this->aNodes != (edCSysHandlerNode*)0x0) {
		delete[] this->aNodes;
		this->aNodes = (edCSysHandlerNode*)0x0;
		this->pNextFreeNode = (edCSysHandlerNode*)0x0;
		this->nbNodes = 0;
	}

	return true;
}

edCSysHandlerNode* edCSysHandlerPool::create_node()
{
	edCSysHandlerNode* pNewNode;

	if (this->nbNodes - this->currentPoolEntries < 1) {
		_edSystemPrintf(0, 0, "edCSysHandlerPool::create_node: Not enough room in node table");
		edSystemBreak();
		pNewNode = (edCSysHandlerNode*)0x0;
	}
	else {
		pNewNode = this->pNextFreeNode;
		this->pNextFreeNode = pNewNode->pNext;
		this->currentPoolEntries = this->currentPoolEntries + 1;
	}

	return pNewNode;
}

bool edCSysHandlerPool::remove_node(edCSysHandlerNode* pNode)
{
	bool bSuccess;

	if ((pNode == (edCSysHandlerNode*)0x0) || (((pNode < this->aNodes && (pNode < this->aNodes + this->nbNodes)) || (this->currentPoolEntries < 1)))) {
		_edSystemPrintf(0, 0, "edCSysHandlerPool::remove_node: Error");
		edSystemBreak();
		bSuccess = false;
	}
	else {
		bSuccess = true;
		pNode->pNext = this->pNextFreeNode;
		this->pNextFreeNode = pNode;
		this->currentPoolEntries = this->currentPoolEntries + -1;
	}

	return bSuccess;
}

bool edSysHandlersAdd(edCSysHandlerPool* pPool, edCSysHandlerNode** ppNode, int param_3, int type, EdSysFunc* pHandlerFunc, int param_6, int param_7)
{
	edCSysHandlerNode* peVar1;
	bool bSuccess;
	edCSysHandlerNode* pNewNode;
	edCSysHandlerNode** ppOffsetNode;
	edCSysHandlerNode* peVar4;

	if ((((pPool == (edCSysHandlerPool*)0x0) || (type < 0)) || (param_3 <= type)) || ((param_6 < 1 || (pHandlerFunc == (EdSysFunc*)0x0)))) {
		_edSystemPrintf(0, 0, "edSysHandlersAdd: Bad parameters");
		edSystemBreak();
		bSuccess = false;
	}
	else {
		pNewNode = pPool->create_node();

		if (pNewNode == (edCSysHandlerNode*)0x0) {
			_edSystemPrintf(0, 0, "edSysHandlersAdd: Unable to allocate memory");
			edSystemBreak();
			bSuccess = false;
		}
		else {
			ppOffsetNode = ppNode + type;
			peVar1 = *ppOffsetNode;
			pNewNode->pFunc = pHandlerFunc;
			pNewNode->field_0x8 = param_6 + -1;
			pNewNode->field_0xc = 0;

			if (peVar1 == (edCSysHandlerNode*)0x0) {
				*ppOffsetNode = pNewNode;
				pNewNode->pNext = (edCSysHandlerNode*)0x0;
			}
			else {
				if (param_7 == 0) {
					pNewNode->pNext = peVar1;
					*ppOffsetNode = pNewNode;
				}
				else {
					do {
						peVar4 = peVar1;
						peVar1 = peVar4->pNext;
					} while (peVar1 != (edCSysHandlerNode*)0x0);

					peVar4->pNext = pNewNode;
					pNewNode->pNext = (edCSysHandlerNode*)0x0;
				}
			}

			bSuccess = true;
		}
	}

	return bSuccess;
}

bool edSysHandlersRemove(edCSysHandlerPool* pPool, edCSysHandlerNode** ppNode, int param_3, int param_4, EdSysFunc* pFunc)
{
	edCSysHandlerNode* pNode;
	bool bSuccess;
	edCSysHandlerNode* peVar1;

	if ((param_4 < 0) || (param_3 <= param_4)) {
		_edSystemPrintf(0, 0, "edSysHandlersRemove: Invalid event index");
		edSystemBreak();
		bSuccess = false;
	}
	else {
		peVar1 = (edCSysHandlerNode*)0x0;

		for (pNode = ppNode[param_4]; pNode != (edCSysHandlerNode*)0x0; pNode = pNode->pNext) {
			if (pNode->pFunc == pFunc) {
				if (peVar1 == (edCSysHandlerNode*)0x0) {
					ppNode[param_4] = pNode->pNext;
				}
				else {
					peVar1->pNext = pNode->pNext;
				}

				pPool->remove_node(pNode);
				return true;
			}

			peVar1 = pNode;
		}

		bSuccess = true;
	}

	return bSuccess;
}

bool edSysHandlersCall(int typeA, edCSysHandlerNode** pPool, int eventMax, int eventID, void* param_5)
{
	bool bSuccess;
	edCSysHandlerNode* pEntry;

	if ((eventID < 0) || (eventMax <= eventID)) {
		_edSystemPrintf(0, 0, "edSysHandlersCall: Invalid event index");
		edSystemBreak();
		bSuccess = false;
	}
	else {
		for (pEntry = pPool[eventID]; bSuccess = true, pEntry != (edCSysHandlerNode*)0x0; pEntry = pEntry->pNext) {
			if (pEntry->field_0xc == 0) {
				(*pEntry->pFunc)(typeA, eventID, (char*)param_5);

				pEntry->field_0xc = pEntry->field_0x8;
			}
			else {
				pEntry->field_0xc = pEntry->field_0xc + -1;
			}
		}
	}

	return bSuccess;
}