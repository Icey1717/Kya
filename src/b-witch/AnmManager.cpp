#include "AnmManager.h"
#include "MathOps.h"
#include "Actor.h"
#include "Animation.h"
#include "MemoryStream.h"

void gAnimation_Callback_Layer0(edAnmMacroAnimator* pAnmMacroAnimator, CActor* pActor, uint newAnim)
{
	uint uVar1;
	uint uVar2;
	int iVar3;

	iVar3 = 1;
	uVar1 = 1;
	do {
		uVar2 = uVar1;
		if ((pActor->pAnimationController->count_0x2c & uVar2) != 0) {
			iVar3 = iVar3 + -1;
		}
		uVar1 = uVar2 << 1;
	} while (iVar3 != 0);

	pActor->AnimEvaluate(uVar2 & 0x7fffffff, pAnmMacroAnimator, newAnim);

	return;
}

void gAnimation_Callback_Layer1(edAnmMacroAnimator* pAnmMacroAnimator, CActor* pActor, uint newAnim)
{
	uint uVar1;
	uint uVar2;
	int iVar3;

	iVar3 = 2;
	uVar1 = 1;
	do {
		uVar2 = uVar1;
		if ((pActor->pAnimationController->count_0x2c & uVar2) != 0) {
			iVar3 = iVar3 + -1;
		}
		uVar1 = uVar2 << 1;
	} while (iVar3 != 0);

	pActor->AnimEvaluate(uVar2 & 0x7fffffff, pAnmMacroAnimator, newAnim);

	return;
}

void gAnimation_Callback_Layer2(edAnmMacroAnimator* pAnmMacroAnimator, CActor* pActor, uint newAnim)
{
	uint uVar1;
	uint uVar2;
	int iVar3;

	iVar3 = 3;
	uVar1 = 1;
	do {
		uVar2 = uVar1;
		if ((pActor->pAnimationController->count_0x2c & uVar2) != 0) {
			iVar3 = iVar3 + -1;
		}
		uVar1 = uVar2 << 1;
	} while (iVar3 != 0);

	pActor->AnimEvaluate(uVar2 & 0x7fffffff, pAnmMacroAnimator, newAnim);

	return;
}

void gAnimation_Callback_Layer3(edAnmMacroAnimator* pAnmMacroAnimator, CActor* pActor, uint newAnim) { IMPLEMENTATION_GUARD() };

AnimationCallback CAnimationManager::_gLayersCallbacks[4] = { gAnimation_Callback_Layer0, gAnimation_Callback_Layer1, gAnimation_Callback_Layer2, gAnimation_Callback_Layer3 };

void edAnmManager::Initialize(char* pBufferStart, int size)
{
	MY_LOG("edAnmManager::Initialize start: %p size: %x", (uintptr_t)pBufferStart, size);
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

	ANIMATION_LOG(LogLevel::Verbose, "edAnmManager::GetMatrixBuffer count: 0x{:x}", count);
	ANIMATION_LOG(LogLevel::Verbose, "edAnmManager::GetMatrixBuffer seek: 0x{:x} end: 0x{:x}", (uintptr_t)this->pBufferSeekPos_0x8018, (uintptr_t)this->pBufferEnd_0x8014);

	peVar1 = (edF32MATRIX4*)this->pBufferSeekPos_0x8018;
	if ((edF32MATRIX4*)this->pBufferEnd_0x8014 < peVar1 + count) {
		peVar1 = (edF32MATRIX4*)0x0;
	}
	else {
		this->pBufferSeekPos_0x8018 = (char*)(peVar1 + count);
	}
	return peVar1;
}

edANM_WRTS* edAnmManager::AllocWRTSBuffer()
{
	int iVar1;

	iVar1 = 0;
	do {
		if (this->tempMatrixMemoryUsage[iVar1] == 0) {
			this->tempMatrixMemoryUsage[iVar1] = 1;
			return &this->tempMatrixBufferArray[iVar1];
		}
		iVar1 = iVar1 + 1;
	} while (iVar1 < 4);
	return (edANM_WRTS*)0x0;
}

void edAnmManager::FreeWRTSBuffer(edANM_WRTS* pBuffer)
{
	int iVar2;

	iVar2 = 0;
	while (true) {
		if ((this->tempMatrixMemoryUsage[iVar2] != 0) && (pBuffer == &this->tempMatrixBufferArray[iVar2])) break;
		iVar2 = iVar2 + 1;
		if (3 < iVar2) {
			return;
		}
	}
	this->tempMatrixMemoryUsage[iVar2] = 0;
	return;
}

edAnmManager TheAnimManager = {};

#define NUM_ANIM_MATRIX_DATA 0xc0

// Should be in: D:/Projects/b-witch/Animation.cpp
void CAnimationManager::Game_Init()
{
	BoneData* pAVar1;
	edF32MATRIX4* puVar2;

	this->pAnimKeyEntryData = (char*)0x0;
	this->levelAnimCount = 0;
	this->loadedAnimKeyDataCount = 0;
	this->pAnimKeyTable = (edANM_HDR**)0x0;
	pAVar1 = new BoneData[NUM_ANIM_MATRIX_DATA];
	this->aAnimMatrixData = pAVar1;
	puVar2 = new edF32MATRIX4[1360];
	this->pBufferB_0x8 = puVar2;
	TheAnimManager.Initialize((char*)this->pBufferB_0x8, 1360 * sizeof(edF32MATRIX4));
	return;
}

// Should be in: D:/Projects/b-witch/Animation.cpp
void CAnimationManager::Game_Term()
{
	if (this->pBufferB_0x8 != (edF32MATRIX4*)0x0) {
		delete this->pBufferB_0x8;
	}
	if (this->aAnimMatrixData != (BoneData*)0x0) {
		delete this->aAnimMatrixData;
	}
	return;
}

// Should be in: D:/Projects/b-witch/Animation.cpp
bool CAnimationManager::LevelLoading_Manage()
{
	Level_Manage();
	return false;
}

// Should be in: D:/Projects/b-witch/Animation.cpp
void CAnimationManager::Level_ClearAll()
{
	BoneData* m0;
	int iVar1;

	iVar1 = NUM_ANIM_MATRIX_DATA;
	m0 = this->aAnimMatrixData;
	do {
		m0->usedByCount = 0;
		m0->boneId = 0xffffffff;
		edF32Matrix4SetIdentityHard(&m0->matrix);
		iVar1 = iVar1 + -1;
		m0 = m0 + 1;
	} while (0 < iVar1);

	if (this->pAnimKeyTable != (edANM_HDR**)0x0) {
		delete this->pAnimKeyTable;
	}

	this->pAnimKeyEntryData = (char*)0x0;
	this->levelAnimCount = 0;
	this->loadedAnimKeyDataCount = 0;
	this->pAnimKeyTable = (edANM_HDR**)0x0;
	return;
}

// Should be in: D:/Projects/b-witch/Animation.cpp
void CAnimationManager::Level_Manage()
{
	TheAnimManager.Reset();
}

// Should be in: D:/Projects/b-witch/Animation.cpp
void CAnimationManager::Level_ManagePaused()
{
	Level_Manage();
}

edANM_HDR* edAnmAnim::LoadFromMem(char* pInFileData, int size)
{
	int* pFileEnd;
	AnimHeaderPacked* pFileData = (AnimHeaderPacked*)pInFileData;

	pFileEnd = &pFileData->hash;
	if (pFileData != (AnimHeaderPacked*)0x0) {
		for (; pFileData < (AnimHeaderPacked*)((ulong)pFileEnd + size);
			pFileData = (AnimHeaderPacked*)((ulong)&pFileData->field_0x8 + pFileData->offset)) {
			if (pFileData->hash == 0x41544144) {
				return (edANM_HDR*)&pFileData->field_0x8;
			}
			if ((pFileData->hash == 0x214d4e41) && (pFileData->field_0x8 != 1.1f)) {
				return (edANM_HDR*)0x0;
			}
		}
	}
	return (edANM_HDR*)0x0;
}

void edAnmMacroAnimator::Initialize(float param_1, edANM_HDR* pHdr, bool param_4, uint flags)
{
	edANM_HDR* peVar1;
	float fVar2;

	if ((flags & 0x80000000) == 0) {
		if (pHdr != (edANM_HDR*)0x0) {
			this->pAnimKeyTableEntry = pHdr;
			this->currentAnimDataFlags = this->pAnimKeyTableEntry->count_0x0;
		}

		if ((flags & 0x40000000) != 0) {
			this->currentAnimDataFlags = flags & 0xbfffffff;
		}

		this->flags = 0;
		this->field_0x8 = 0;
		this->keyStartTime_0x14 = 0.0f;
		this->keyDuration_0x18 = 0.0f;

		if (this->pAnimKeyTableEntry->field_0x4.asKey == 0) {
			this->field_0x8 = 0;
			peVar1 = this->pKeyDataArray[this->pAnimKeyTableEntry->keyIndex_0x8.asKey];
			if ((this->currentAnimDataFlags & 1) == 0) {
				fVar2 = peVar1->keyIndex_0x8.asTime;
			}
			else {
				fVar2 = peVar1->field_0x4.asTime;
			}
			this->keyStartTime_0x14 = fVar2;
			this->keyDuration_0x18 = peVar1->field_0x4.asTime - peVar1->keyIndex_0x8.asTime;
		}
		else {
			UpdateAnimParams();
		}

		if (this->keyStartTime_0x14 < 0.0f) {
			this->keyStartTime_0x14 = 0.0f;
		}

		if (this->keyDuration_0x18 < 0.0f) {
			this->keyDuration_0x18 = 0.0f;
		}

		if (param_4 == false) {
			this->time_0x10 = param_1;

			fVar2 = 0.0f;
			if (0.0f < this->keyStartTime_0x14) {
				fVar2 = param_1 / this->keyStartTime_0x14;
			}

			this->time_0xc = fVar2;
		}
		else {
			this->time_0xc = param_1;
			this->time_0x10 = param_1 * this->keyStartTime_0x14;
		}
	}
	else {
		if ((flags & 1) == 0) {
			fVar2 = pHdr->keyIndex_0x8.asTime;
		}
		else {
			fVar2 = pHdr->field_0x4.asTime;
		}

		this->keyStartTime_0x14 = fVar2;
		this->keyDuration_0x18 = pHdr->field_0x4.asTime - pHdr->keyIndex_0x8.asTime;
		this->currentAnimDataFlags = flags & 0xbfffffff | 2;
		this->field_0x8 = 0;
		this->flags = 0;

		if (param_4 == false) {
			this->time_0x10 = param_1;
			fVar2 = 0.0f;
			if (0.0f < this->keyStartTime_0x14) {
				fVar2 = param_1 / this->keyStartTime_0x14;
			}
			this->time_0xc = fVar2;
		}
		else {
			this->time_0xc = param_1;
			this->time_0x10 = param_1 * this->keyStartTime_0x14;
		}

		this->pAnimKeyTableEntry = pHdr;
	}

	return;
}

void edAnmMacroAnimator::AnimateDT(float time)
{
	float fVar1;

	ANIMATION_LOG(LogLevel::Verbose, "edAnmMacroAnimator::AnimateDT time: {}", time);

	fVar1 = 0.0f;
	if (0.0f < this->keyStartTime_0x14) {
		fVar1 = time / this->keyStartTime_0x14;
	}
	this->time_0xc = this->time_0xc + fVar1;

	fVar1 = this->time_0x10;
	this->time_0x10 = fVar1 + time;

	Animate();

	if (((this->flags & 0x40000000) != 0) && (fVar1 < this->keyStartTime_0x14 - this->keyDuration_0x18)) {
		this->flags = this->flags | 0x20000000;
	}
	return;
}

void edAnmMacroAnimator::Animate()
{
	uint uVar1;
	edANM_HDR* peVar2;
	int* piVar3;
	float* pfVar4;
	int iVar5;
	int iVar6;
	float fVar7;
	float fVar8;

	this->flags = 0;
	uVar1 = this->currentAnimDataFlags;
	if ((uVar1 & 0x80000000) == 0) {
		peVar2 = this->pAnimKeyTableEntry;
		iVar5 = peVar2->field_0x4.asKey;

		ANIMATION_LOG(LogLevel::Verbose, "edAnmMacroAnimator::Animate mode: {}", iVar5);

		if (iVar5 == 2) {
			fVar8 = 0.0f;
			iVar5 = peVar2->keyIndex_0x8.asKey + -1;
			if (-1 < iVar5) {
				char* pData = reinterpret_cast<char*>(peVar2) + (iVar5 * 4) + (peVar2->keyIndex_0x8.asKey * 4);

				pfVar4 = reinterpret_cast<float*>(pData + 0xc);
				piVar3 = (int*)((ulong)peVar2 + iVar5 * 4 + 0xc);
				do {
					fVar7 = *pfVar4;
					fVar8 = fVar8 + fVar7;
					if (0.0001f < fVar7) {
						TheAnimStage.SetAnim((edANM_HDR*)this->pKeyDataArray[*piVar3]);
						TheAnimStage.SetTimeAsRatio(0.0f);
						TheAnimStage.AnimBlendToWRTS(fVar7);
					}

					iVar5 = iVar5 + -1;
					pfVar4 = pfVar4 + -1;
					piVar3 = piVar3 + -1;
				} while (-1 < iVar5);
			}

			if ((0.0f < fVar8) && (fVar8 < 1.0f)) {
				IMPLEMENTATION_GUARD(
				edAnmStage::BlendDefaultRTSWithDestWRTS(1.0, &TheAnimStage, 1);)
			}
			this->flags = this->flags | 0x80000000;
			this->flags = this->flags | 0x40000000;
			this->time_0x10 = TheAnimStage.animTime;
			this->time_0xc = 1.0f;
		}
		else {
			if (iVar5 == 4) {
				iVar5 = peVar2->keyIndex_0x8.asKey;
				if (iVar5 != 0) {
					iVar6 = iVar5 + -1;
					if (-1 < iVar6) {
						int* pAfterHeader = reinterpret_cast<int*>(peVar2 + 1);


						piVar3 = pAfterHeader + iVar6;
						pfVar4 = reinterpret_cast<float*>(pAfterHeader + iVar5 + iVar6);
						do {
							TheAnimStage.SetAnim(this->pKeyDataArray[*piVar3]);
							TheAnimStage.SetTimeAsRatio(*pfVar4);
							TheAnimStage.AnimBlendToWRTS(1.0f);
							iVar6 = iVar6 + -1;
							piVar3 = piVar3 + -1;
							pfVar4 = pfVar4 + -1;
						} while (-1 < iVar6);
					}
				}

				this->flags = this->flags | 0x80000000;
				this->flags = this->flags | 0x40000000;
				this->time_0xc = 1.0f;
				this->time_0x10 = 0.0f;
			}
			else {
				if (iVar5 == 1) {
					iVar5 = peVar2->keyIndex_0x8.asKey + -1;
					ANIMATION_LOG(LogLevel::Verbose, "edAnmMacroAnimator::Animate count B: {}", iVar5);

					if (-1 < iVar5) {
						int* pAfterHeader = reinterpret_cast<int*>(peVar2 + 1);

						piVar3 = reinterpret_cast<int*>(pAfterHeader + iVar5);
						pfVar4 = reinterpret_cast<float*>(piVar3 + peVar2->keyIndex_0x8.asKey);

						do {
							ANIMATION_LOG(LogLevel::Verbose, "edAnmMacroAnimator::Animate time: {}", *pfVar4);

							if (0.0001f < *pfVar4) {
								if ((this->currentAnimDataFlags & 1) == 0) {
									TheAnimStage.SetAnim(this->pKeyDataArray[*piVar3]);
								}
								else {
									TheAnimStage.SetAnimLoop(this->pKeyDataArray[*piVar3]);
								}

								TheAnimStage.SetTimeAsRatio(this->time_0xc);
								TheAnimStage.AnimBlendToWRTS(*pfVar4);
							}

							iVar5 = iVar5 + -1;
							pfVar4 = pfVar4 + -1;
							piVar3 = piVar3 + -1;
						} while (-1 < iVar5);
					}

					if (TheAnimStage.field_0x4c != false) {
						this->flags = this->flags | 0x80000000;
					}

					if (TheAnimStage.field_0x4d != false) {
						this->flags = this->flags | 0x40000000;
					}

					fVar8 = TheAnimStage.field_0x40;
					this->time_0xc = TheAnimStage.field_0x40;
					this->time_0x10 = fVar8 * this->keyStartTime_0x14;
				}
				else {
					if (iVar5 == 0) {
						if ((uVar1 & 1) == 0) {
							TheAnimStage.SetAnim(this->pKeyDataArray[peVar2->keyIndex_0x8.asKey]);
						}
						else {
							TheAnimStage.SetAnimLoop(this->pKeyDataArray[peVar2->keyIndex_0x8.asKey]);
						}
						TheAnimStage.SetTime(this->time_0x10);
						TheAnimStage.AnimToWRTS();
						this->time_0x10 = TheAnimStage.animTime;
						this->time_0xc = TheAnimStage.field_0x40;
						if (TheAnimStage.field_0x4c != false) {
							this->flags = this->flags | 0x80000000;
						}
						if (TheAnimStage.field_0x4d != false) {
							this->flags = this->flags | 0x40000000;
						}
					}
				}
			}
		}
	}
	else {
		if ((uVar1 & 1) == 0) {
			TheAnimStage.SetAnim(this->pAnimKeyTableEntry);
		}
		else {
			TheAnimStage.SetAnimLoop(this->pAnimKeyTableEntry);
		}

		TheAnimStage.SetTime(this->time_0x10);
		TheAnimStage.AnimToWRTS();
		this->time_0x10 = TheAnimStage.animTime;
		this->time_0xc = TheAnimStage.field_0x40;
		if (TheAnimStage.field_0x4c != false) {
			this->flags = this->flags | 0x80000000;
		}
		if (TheAnimStage.field_0x4d != false) {
			this->flags = this->flags | 0x40000000;
		}
	}
	return;
}

void edAnmMacroAnimator::UpdateAnimParams()
{
	edANM_HDR* peVar1;
	int iVar2;
	float* pfVar3;
	int* piVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if (((this->currentAnimDataFlags & 4) != 0) && (this->pFunction != 0x0)) {
		this->pFunction(this, this->pActor, this->animationType);
	}

	if (((this->currentAnimDataFlags & 0x80000000) == 0) && (peVar1 = this->pAnimKeyTableEntry, peVar1->field_0x4.asKey == 1)) {
		iVar2 = peVar1->keyIndex_0x8.asKey;
		fVar8 = 0.0f;
		this->keyDuration_0x18 = 0.0f;
		iVar5 = iVar2 + -1;
		this->keyStartTime_0x14 = 0.0f;
		if (-1 < iVar5) {
			piVar4 = (int*)((char*)peVar1 + iVar5 * 4 + 0xc);
			pfVar3 = (float*)((char*)peVar1 + iVar5 * 4 + iVar2 * 4 + 0xc);
			do {
				peVar1 = this->pKeyDataArray[*piVar4];
				if ((this->currentAnimDataFlags & 1) == 0) {
					fVar6 = peVar1->keyIndex_0x8.asTime;
				}
				else {
					fVar6 = peVar1->field_0x4.asTime;
				}
				fVar7 = *pfVar3;
				iVar5 = iVar5 + -1;
				piVar4 = piVar4 + -1;
				pfVar3 = pfVar3 + -1;
				this->keyStartTime_0x14 = this->keyStartTime_0x14 + fVar6 * fVar7;
				fVar8 = fVar8 + fVar7;
				this->keyDuration_0x18 = this->keyDuration_0x18 + fVar7 * (peVar1->field_0x4.asTime - peVar1->keyIndex_0x8.asTime);
			} while (-1 < iVar5);
		}

		if (0.0f < fVar8) {
			this->keyStartTime_0x14 = this->keyStartTime_0x14 / fVar8;
			this->keyDuration_0x18 = this->keyDuration_0x18 / fVar8;
		}

		if (this->keyStartTime_0x14 < 0.0f) {
			this->keyStartTime_0x14 = 0.0f;
		}

		if (this->keyDuration_0x18 < 0.0f) {
			this->keyDuration_0x18 = 0.0f;
		}

		this->field_0x8 = 0;
	}
	return;
}

void CAnimationManager::Level_Create(ByteCode* pMemoryStream)
{
	int iVar1;
	edANM_HDR** ppeVar2;
	int iVar3;
	BoneData* m0;

	iVar1 = pMemoryStream->GetS32();
	this->levelAnimCount = iVar1;
	if (this->levelAnimCount < 1) {
		this->pAnimKeyTable = (edANM_HDR**)0x0;
	}
	else {
		ppeVar2 = new edANM_HDR*[levelAnimCount];
		this->pAnimKeyTable = ppeVar2;

		for (int i = 0; i < this->levelAnimCount; i++) {
			pAnimKeyTable[i] = NULL;
		}
	}
	pMemoryStream->GetS32();
	m0 = this->aAnimMatrixData;
	iVar1 = 0xc0;
	do {
		m0->usedByCount = 0;
		m0->boneId = 0xffffffff;
		edF32Matrix4SetIdentityHard(&m0->matrix);
		iVar1 = iVar1 + -1;
		m0 = m0 + 1;
	} while (0 < iVar1);
	return;
}

char* CAnimationManager::ProfileGetName()
{
	return "Anims  ";
}

// Should be in: D:/Projects/b-witch/Animation.h
void CAnimationManager::Level_Term()
{
	return;
}