#ifndef DLIST_MANAGER_H
#define DLIST_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct DisplayListInternal;

class CGlobalDList {
public:
	DisplayListInternal* pDisplayListInternal;
	int field_0x8;
	int field_0xc;
	int field_0x10;
	int bEnabled;
	byte field_0x18;
	byte field_0x19;
	byte field_0x1a;
	byte field_0x1b;
	int field_0x1c;

	inline CGlobalDList() 
	{
		this->field_0x8 = -1;
		this->field_0xc = -1;
		this->field_0x10 = -1;
		this->pDisplayListInternal = (DisplayListInternal*)0x0;
		this->bEnabled = 0;
	}
	virtual void Init();
	virtual void Term() {}
};

struct S_EYES_BRIGHT_SHADOW;

class CGlobalDListPatch : public CGlobalDList {
public:
	S_EYES_BRIGHT_SHADOW* pBrightEye[296];
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
	undefined4 field_0x4d0;
	int brightEyeCount;
	byte field_0x4d8;
	byte field_0x4d9;
	undefined field_0x4da;
	undefined field_0x4db;
	uint field_0x4dc;

	inline CGlobalDListPatch(uint index)
	{
		this->field_0x4dc = index;
		this->brightEyeCount = 0;
		this->field_0x4d0 = 0;
		this->field_0x4d8 = 0;
		this->field_0x4d9 = 0;

		for (int i = 0; i < 296; i++) {
			pBrightEye[i] = NULL;
		}
	}
};

struct GlobalDlistEntry
{
	CGlobalDListPatch* pDlistPatch;
	int field_0x4;
	int field_0x8;
	int field_0xc;
};

class CGlobalDListManager : public CObjectManager 
{
public:
	virtual void Level_Init();
	virtual void Level_Term() {};

	virtual void Level_ClearAll();
	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset() {};

	virtual void Level_CheckpointReset() {};
	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	virtual void Level_Create();

	void SectorChange(int newSectorId);
	void _ExecuteCallFunc();

	bool SetActive(int param_2, int param_3);

	int dlistCount;
	byte bBeganLevelInit;
	byte bCompletedLevelInit;
	int field_0x1c;
	GlobalDlistEntry* ppGlobalDlist;
	DisplayListInternal* pDisplayList;

	int field_0x14;
	int field_0x18;
	uint field_0x24;

	int field_0x12e8;
	int field_0x29b0;
};

#endif // DLIST_MANAGER_H
