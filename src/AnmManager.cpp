#include "AnmManager.h"
#include "MathOps.h"

void edAnmManager::Initialize(char* pBufferStart, int size)
{
	this->pBufferStart_0x8010 = pBufferStart;
	this->pBufferEnd_0x8014 = pBufferStart + size;
	this->pBufferSeekPos_0x8018 = this->pBufferStart_0x8010;
	this->tempMatrixMemoryUsage[0] = 0;
	this->tempMatrixMemoryUsage[1] = 0;
	this->tempMatrixMemoryUsage[2] = 0;
	this->tempMatrixMemoryUsage[3] = 0;
	this->pBufferSeekPos_0x8018 = this->pBufferStart_0x8010;
	return;
}

void edAnmManager::Reset()
{
	this->tempMatrixMemoryUsage[0] = 0;
	this->tempMatrixMemoryUsage[1] = 0;
	this->tempMatrixMemoryUsage[2] = 0;
	this->tempMatrixMemoryUsage[3] = 0;
	this->pBufferSeekPos_0x8018 = this->pBufferStart_0x8010;
	return;
}

edF32MATRIX4* edAnmManager::GetMatrixBuffer(int count)
{
	edF32MATRIX4* peVar1;

	peVar1 = (edF32MATRIX4*)this->pBufferSeekPos_0x8018;
	if ((edF32MATRIX4*)this->pBufferEnd_0x8014 < peVar1 + count) {
		peVar1 = (edF32MATRIX4*)0x0;
	}
	else {
		this->pBufferSeekPos_0x8018 = (char*)(peVar1 + count);
	}
	return peVar1;
}

edAnmManager TheAnimManager = {};

#define NUM_ANIM_MATRIX_DATA 0xc0

void CAnimationManager::Game_Init()
{
	AnimMatrixData* pAVar1;
	edF32MATRIX4* puVar2;

	this->pAnimKeyEntryData = (char*)0x0;
	this->count_0x10 = 0;
	this->loadedAnimKeyDataCount = 0;
	this->pAnimKeyTable = (edAnmAnim**)0x0;
	pAVar1 = new AnimMatrixData[NUM_ANIM_MATRIX_DATA];
	this->aAnimMatrixData = pAVar1;
	puVar2 = new edF32MATRIX4[1360];
	this->pBufferB_0x8 = puVar2;
	TheAnimManager.Initialize((char*)this->pBufferB_0x8, 1360 * sizeof(edF32MATRIX4));
	return;
}

void CAnimationManager::Game_Term()
{
	if (this->pBufferB_0x8 != (edF32MATRIX4*)0x0) {
		delete this->pBufferB_0x8;
	}
	if (this->aAnimMatrixData != (AnimMatrixData*)0x0) {
		delete this->aAnimMatrixData;
	}
	return;
}

bool CAnimationManager::LevelLoading_Manage()
{
	Level_Manage();
	return false;
}

void CAnimationManager::Level_ClearAll()
{
	AnimMatrixData* m0;
	int iVar1;

	iVar1 = NUM_ANIM_MATRIX_DATA;
	m0 = this->aAnimMatrixData;
	do {
		m0->usedByCount = 0;
		m0->key_0x48 = 0xffffffff;
		edF32Matrix4SetIdentityHard(&m0->field_0x0);
		iVar1 = iVar1 + -1;
		m0 = m0 + 1;
	} while (0 < iVar1);

	if (this->pAnimKeyTable != (edAnmAnim**)0x0) {
		delete this->pAnimKeyTable;
	}

	this->pAnimKeyEntryData = (char*)0x0;
	this->count_0x10 = 0;
	this->loadedAnimKeyDataCount = 0;
	this->pAnimKeyTable = (edAnmAnim**)0x0;
	return;
}

void CAnimationManager::Level_Manage()
{
	TheAnimManager.Reset();
}

void CAnimationManager::Level_ManagePaused()
{
	Level_Manage();
}
