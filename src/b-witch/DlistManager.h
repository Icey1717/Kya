#ifndef DLIST_MANAGER_H
#define DLIST_MANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "Rendering/DisplayList.h"

#define IMPLEMENTATION_GUARD_DLIST_PATCH(x) assert(false);

struct DisplayList;

struct S_GLOBAL_DLIST_PATCH;

#define CALL_ELEMENT_SET_ACTIVE 1
#define CALL_ELEMENT_HIDE_VERTEX 2
#define CALL_ELEMENT_HIDE_SPRITE 3

class CGlobalDListPatch : public CGlobalDList
{
public:
	S_GLOBAL_DLIST_PATCH* aPatches[296];
	undefined field_0x4c0;
	undefined field_0x4c1;
	undefined field_0x4c2;
	undefined field_0x4c3;
	undefined field_0x4c4;
	undefined field_0x4c5;
	undefined field_0x4c6;
	undefined field_0x4c7;
	undefined field_0x4c8;
	undefined field_0x4c9;
	undefined field_0x4ca;
	undefined field_0x4cb;
	undefined field_0x4cc;
	undefined field_0x4cd;
	undefined field_0x4ce;
	undefined field_0x4cf;
	S_GLOBAL_DLIST_PATCH* pCurrentPatch;
	int nbTotalPatches;
	byte field_0x4d8;
	byte field_0x4d9;
	undefined field_0x4da;
	undefined field_0x4db;
	uint field_0x4dc;

	inline CGlobalDListPatch(uint index)
	{
		this->field_0x4dc = index;
		this->nbTotalPatches = 0;
		this->pCurrentPatch = 0;
		this->field_0x4d8 = 0;
		this->field_0x4d9 = 0;

		for (int i = 0; i < 296; i++) {
			aPatches[i] = NULL;
		}
	}

	~CGlobalDListPatch();
};

struct GlobalDlistEntry
{
	CGlobalDListPatch* pDlistPatch;
	int nbMatrices;
	int nbRegisteredPatches;
	int nbInstances;
};

struct CallFuncElement
{
	int patchId;
	int bActive;
	byte type;
	byte nbElements;
};

class CGlobalDListManager : public CObjectManager 
{
public:
	virtual void Level_Init();
	virtual void Level_Term();

	virtual void Level_ClearAll();
	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset() {};

	virtual void Level_CheckpointReset() {};
	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

#ifdef PLATFORM_WIN
	virtual char* ProfileGetName() { return "DList"; }
#endif

	virtual void Level_Create();

	void SectorChange(int newSectorId);
	void _ExecuteCallFunc();

	bool SetActive(int patchId, int index);
	CGlobalDListPatch* BeginCurrent(int patchId);
	void EndCurrent(int nbVertex, int param_2);

	bool _AddCallFuncElement(int patchId, int type, int bActive);

	int dlistCount;
	byte bBeganLevelInit;
	byte bCompletedLevelInit;
	int field_0x1c;
	GlobalDlistEntry* ppGlobalDlist;
	DisplayList* pDisplayList;

	int field_0x14;
	int field_0x18;
	uint field_0x24;

	CallFuncElement aActiveCallFuncElements[400];
	int nbActiveCallFuncElements;
	int field_0x29b0;
};

void GameDListPatch_HideSprite(int patchId, uint index);
void GameDListPatch_HideVertex(int patchId, int index);

#endif // DLIST_MANAGER_H
