#ifndef _ANM_MANAGER_H
#define _ANM_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

struct AnimMatrixData {
	edF32MATRIX4 field_0x0;
	AnimMatrixData* pNext;
	AnimMatrixData* pPrev;
	uint key_0x48;
	short field_0x4c;
	short usedByCount;
};

struct edAnmManager {
	byte tempMatrixMemoryUsage[4];
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	edF32MATRIX4 tempMatrixBufferArray[4][128];
	char* pBufferStart_0x8010;
	char* pBufferEnd_0x8014; /* Created by retype action */
	char* pBufferSeekPos_0x8018; /* Created by retype action */

	void Initialize(char* pBufferStart, int size);
	void Reset();
	edF32MATRIX4* GetMatrixBuffer(int count);
};

extern edAnmManager TheAnimManager;

struct CAnimationManager : public CObjectManager {
public:
	virtual void Game_Init();
	virtual void Game_Term();
	virtual bool LevelLoading_Manage();
	virtual void Level_ClearAll();
	virtual void Level_Manage();
	virtual void Level_ManagePaused();

	AnimMatrixData* aAnimMatrixData;
	edF32MATRIX4* pBufferB_0x8;
	char* pAnimKeyEntryData;
	int count_0x10;
	int loadedAnimKeyDataCount;
	struct edAnmAnim** pAnimKeyTable;
};

#endif // _ANM_MANAGER_H
