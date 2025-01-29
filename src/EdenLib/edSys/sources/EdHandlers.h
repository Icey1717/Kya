#ifndef ED_HANDLERS_H
#define ED_HANDLERS_H

#include "Types.h"

typedef void EdSysFunc(int, int, char*);

struct edCSysHandlerNode {
	edCSysHandlerNode();

	edCSysHandlerNode* pNext;
	EdSysFunc* pFunc;
	undefined4 field_0x8;
	undefined4 field_0xc;
};

struct edCSysHandlerPool {
	edCSysHandlerPool();
	bool initialize(int count);
	bool terminate();
	edCSysHandlerNode* create_node();
	bool remove_node(edCSysHandlerNode* pNode);

	struct edCSysHandlerNode* aNodes;
	struct edCSysHandlerNode* pNextFreeNode;
	int currentPoolEntries;
	int nbNodes;
};

template<int nbEvents, edCSysHandlerPool* pNodeTable, int identifier>
struct edCSysHandlerSystem
{
	edCSysHandlerSystem()
		: nodeParent(pNodeTable)
		, maxEventID(nbEvents)
		, mainIdentifier(identifier)
	{

	}

	inline void add(int type, EdSysFunc* pHandlerFunc, int param_6, int param_7)
	{
		edSysHandlersAdd(nodeParent, entries, maxEventID, type, pHandlerFunc, param_6, param_7);
	}

	inline bool remove(int type, EdSysFunc* pHandlerFunc)
	{
		return edSysHandlersRemove(nodeParent, entries, maxEventID, type, pHandlerFunc);
	}

	struct edCSysHandlerPool* nodeParent;
	struct edCSysHandlerNode* entries[nbEvents];
	int maxEventID;
	int mainIdentifier;
};

bool edSysHandlersInit(int size);
bool edSysHandlersTerm(void);
bool edSysHandlersAdd(edCSysHandlerPool* pPool, edCSysHandlerNode** ppNode, int param_3, int type, EdSysFunc* pHandlerFunc, int param_6, int param_7);
bool edSysHandlersRemove(edCSysHandlerPool* pPool, edCSysHandlerNode** ppNode, int param_3, int param_4, EdSysFunc* pFunc);
bool edSysHandlersCall(int typeA, edCSysHandlerNode** pPool, int eventMax, int eventID, void* param_5);

extern edCSysHandlerPool edSysHandlerMainPool;
extern edCSysHandlerSystem<9, &edSysHandlerMainPool, 3> edSoftHandlers;

#endif // !ED_HANDLERS_H
