#include "Fx.h"
#include "MemoryStream.h"
#include <string.h>

#include "FxComposite.h"
#include "FxParticle.h"
#include "FxSound.h"
#include "FxRandom.h"
#include "FxLod.h"
#include "FxPath.h"
#include "FxGroup.h"
#include "MathOps.h"
#include "CameraViewManager.h"

CFxManager::CFxManager()
{
	CFxManager* local_a0_lo_24;

	this->count_0x4 = 0;
	this->aFx = (CFx**)0x0;
	this->orderedCountArray = (uint*)0x0;
	this->totalEffectCount_0x2c = 0;
	this->pSortData = (s_fx_sort_data*)0x0;
	this->field_0x6c = 2;

	memset(this->aEffectCategory, 0, sizeof(this->aEffectCategory));
	memset(this->effectCountByType, 0, sizeof(this->effectCountByType));

	this->aParticleClassSizes[FX_TYPE_COMPOSITE]	= sizeof(CFxComposite);
	this->aParticleClassSizes[FX_TYPE_PATH]			= sizeof(CFxPath);
	this->aParticleClassSizes[FX_TYPE_GROUP]		= sizeof(CFxGroup);
	this->aParticleClassSizes[FX_TYPE_PARTICLE]		= sizeof(CFxNewParticle);
	this->aParticleClassSizes[FX_TYPE_SOUND]		= sizeof(CFxSound);
	this->aParticleClassSizes[FX_TYPE_RANDOM]		= 0; // ??
	this->aParticleClassSizes[FX_TYPE_LOD]			= 0; // ??

	return;
}

void CFxManager::Game_Init()
{
	//EffectsManagerProps* pEVar1;
	//EffectsManagerObjSetupParams* pSetupParams;
	//
	//pEVar1 = edParticlesGetSystemConfig();
	//pEVar1->effectObjCount = 1;
	//edPartSystemInit();
	//pSetupParams = edParticlesGetConfig();
	//pSetupParams->field_0x0 = 1000;
	//pSetupParams->field_0x4 = 0x14;
	//pSetupParams->field_0xc = 0x14;
	//pSetupParams->field_0x8 = 0x14;
	//pSetupParams->field_0x14 = 0x1e;
	//pSetupParams->field_0x10 = 0x1e;
	//pSetupParams->field_0x1c = 1;
	//pSetupParams->field_0x18 = 1;
	//pSetupParams->field_0x24 = 0x14;
	//pSetupParams->field_0x20 = 0x14;
	//edPartInit();
	return;
}

void CFxManager::Game_Term()
{
	//edPartSystemTerm();
	return;
}

void CFxManager::Level_Init()
{
	return;
}

void CFxManager::Level_Term()
{
	IMPLEMENTATION_GUARD();
}

void CFxManager::Level_AddAll(struct ByteCode* pByteCode)
{
	uint uVar1;
	undefined* puVar2;
	uint outCount;
	int iVar3;
	int outType;

	pByteCode->GetChunk();

	CFxParticleManager* pFxParticleManager = static_cast<CFxParticleManager*>(this->aEffectCategory[3]);
	pFxParticleManager->ReadPrtResChunk(pByteCode);

	pByteCode->GetChunk();

	this->count_0x4 = pByteCode->GetU32();

	if (this->count_0x4 != 0) {
		this->aFx = new CFx*[this->count_0x4];
		this->orderedCountArray = (uint*)edMemAlloc(TO_HEAP(H_MAIN), this->count_0x4 * sizeof(uint));
		uVar1 = 0;
		if (this->count_0x4 != 0) {
			do {
				outCount = AddFxClass(pByteCode, this->aFx + uVar1, &outType);
				orderedCountArray[uVar1] = outCount;
				this->effectCountByType[outType] = orderedCountArray[uVar1];
				this->totalEffectCount_0x2c = this->totalEffectCount_0x2c + orderedCountArray[uVar1];
				uVar1 = uVar1 + 1;
			} while (uVar1 < this->count_0x4);
		}

		iVar3 = 0;
		uVar1 = 0;
		do {
			if (this->aEffectCategory[uVar1] == (CFxPoolManagerFather*)0x0) {
				outCount = this->effectCountByType[uVar1];
			}
			else {
				outCount = this->aEffectCategory[uVar1]->GetNbPool();
			}

			iVar3 = iVar3 + outCount;
			uVar1 = uVar1 + 1;
		} while (uVar1 < 7);

		if (iVar3 != 0) {
			this->pSortData = reinterpret_cast<s_fx_sort_data*>(edMemAlloc(TO_HEAP(H_MAIN), iVar3 * sizeof(s_fx_sort_data)));
		}
	}

	return;
}

void CFxManager::Level_ClearAll()
{
	return;
}

void CFxManager::Level_Manage()
{
	CFx* pCVar1;
	uint* puVar2;
	uint uVar3;
	CFx** pFxEnd;

	puVar2 = this->orderedCountArray + this->count_0x4;
	pFxEnd = this->aFx + this->count_0x4;

	if (this->aFx < pFxEnd) {
		do {
			pFxEnd = pFxEnd + -1;
			pCVar1 = *pFxEnd;
			puVar2 = puVar2 + -1;
			if (pCVar1 != (CFx*)0x0) {
				pCVar1->ManageAll(*puVar2, pCVar1);
			}
		} while (this->aFx < pFxEnd);
	}

	uVar3 = 0;
	do {
		if (this->aEffectCategory[uVar3] != (CFxPoolManagerFather*)0x0) {
			this->aEffectCategory[uVar3]->Manage();
		}
		uVar3 = uVar3 + 1;
	} while (uVar3 < 7);

	return;
}

void CFxManager::Level_ManagePaused()
{
	return;
}

template<typename T>
void _SortDecreasing(T* pData, uint count)
{
	CFx* pCVar1;
	int iVar2;
	T* psVar3;
	T* psVar4;
	uint uVar5;
	uint uVar6;
	float fVar7;

	uVar6 = 0;
	psVar4 = pData;
	if (count != 1) {
		do {
			uVar5 = uVar6 + 1;
			if (uVar5 < count) {
				psVar3 = pData + uVar5;
				do {
					if (psVar4->field_0x4 < psVar3->field_0x4) {
						*psVar4 = *psVar3;
					}

					uVar5 = uVar5 + 1;
					psVar3 = psVar3 + 1;
				} while (uVar5 < count);
			}
			uVar6 = uVar6 + 1;
			psVar4 = psVar4 + 1;
		} while (uVar6 < count - 1);
	}

	return;
}

void CFxManager::Level_Draw()
{
	uint uVar1;
	int classSize;
	edF32MATRIX4* peVar3;
	CCameraManager* pCameraManager;
	s_fx_sort_data* psVar4;
	uint uVar5;
	float fVar6;
	int local_a0;
	CFxManager* local_90;
	uint local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32MATRIX4 eStack80;
	CFx* local_8;
	uint local_4;

	pCameraManager = CScene::ptable.g_CameraManager_0045167c;

	if ((GameFlags & 0x200) == 0) {
		local_4 = 0;
		local_80 = 0;
		local_a0 = 0;
		peVar3 = &(CScene::ptable.g_CameraManager_0045167c)->transformationMatrix;
		do {
			uVar1 = this->effectCountByType[local_80];
			if (uVar1 != 0) {
				local_8 = this->aFx[local_a0];
				if (local_8 == (CFx*)0x0) {
					this->aEffectCategory[local_80]->Play(&local_4, this->pSortData, pCameraManager);
				}
				else {
					classSize = this->aParticleClassSizes[local_80];
					uVar5 = 0;
					if (uVar1 != 0) {
						do {
							if ((local_8->flags & 2) != 0) {
								this->pSortData[local_4].field_0x8 = 0;
								this->pSortData[local_4].pFx = local_8;

								edF32Matrix4SetIdentityHard(&eStack80);
								edF32Matrix4TranslateHard(&eStack80, &eStack80, &local_8->vector_0x10);
								edF32Matrix4MulF32Matrix4Hard(&eStack80, &eStack80, &pCameraManager->worldToCamera_0x3d0);
								local_60.x = eStack80.ac;
								local_60.y = eStack80.bc;
								local_60.z = eStack80.cc;
								local_70 = local_8->vector_0x10;
								edF32Vector4SubHard(&local_70, &local_70, &peVar3->rowT);

								this->pSortData[local_4].field_0x4 = edF32Vector4DotProductHard(&local_60, &local_70);

								local_4 = local_4 + 1;
							}

							uVar5 = uVar5 + 1;
							local_8 = reinterpret_cast<CFx*>(reinterpret_cast<char*>(local_8) + classSize);
						} while (uVar5 < uVar1);
					}
				}

				local_a0 = local_a0 + 1;
			}

			local_80 = local_80 + 1;
		} while (local_80 < 7);

		if (local_4 != 0) {
			_SortDecreasing<s_fx_sort_data>(this->pSortData, local_4);
		}

		psVar4 = this->pSortData;
		uVar1 = local_4;
		while (local_4 = uVar1 - 1, uVar1 != 0) {
			if (psVar4->field_0x8 == 0) {
				psVar4->pFx->Draw();
			}
			else {
				IMPLEMENTATION_GUARD();
				// Delete?
				//psVar4->pFx->field_0x8();
			}

			psVar4 = psVar4 + 1;
			uVar1 = local_4;
		}
	}

	return;
}

void CFxManager::Level_Reset()
{
	IMPLEMENTATION_GUARD();
}

void CFxManager::Level_CheckpointReset()
{
	CFx* pFx;
	uint* pCount;
	uint uVar3;
	CFx** ppCurFx;

	pCount = this->orderedCountArray + this->count_0x4;
	ppCurFx = this->aFx + this->count_0x4;
	if (this->aFx < ppCurFx) {
		do {
			ppCurFx = ppCurFx + -1;
			pFx = *ppCurFx;
			pCount = pCount + -1;
			if (pFx != (CFx*)0x0) {
				pFx->CheckpointResetAll(*pCount, pFx);
			}
		} while (this->aFx < ppCurFx);
	}

	uVar3 = 0;
	do {
		if (this->aEffectCategory[uVar3] != (CFxPoolManagerFather*)0x0) {
			this->aEffectCategory[uVar3]->CheckpointReset();
		}
		uVar3 = uVar3 + 1;
	} while (uVar3 < 7);

	return;
}

void CFxManager::Level_PauseChange(bool bPaused)
{
	IMPLEMENTATION_GUARD();
}

void CFxManager::AddPool(ByteCode* pByteCode)
{
	uint uVar3;
	uint curIndex;

	this->aEffectCategory[FX_TYPE_COMPOSITE]	= new CFxCompositeManager;
	this->aEffectCategory[FX_TYPE_PARTICLE]		= new CFxParticleManager;
	this->aEffectCategory[FX_TYPE_SOUND]		= new CFxSoundManager;
	this->aEffectCategory[FX_TYPE_RANDOM]		= new CFxRandomManager;
	this->aEffectCategory[FX_TYPE_LOD]			= new CFxLodManager;

	curIndex = 0;
	do {
		uVar3 = pByteCode->GetU32();

		if (this->aEffectCategory[curIndex] != NULL) {
			this->aEffectCategory[curIndex]->SetupPool(pByteCode, uVar3);
		}

		curIndex = curIndex + 1;
	} while (curIndex < 7);

	return;
}

uint CFxManager::AddFxClass(ByteCode* pByteCode, CFx** pOutEffectObj, int* outClass)
{
	CFxPoolManagerFather* pCurFx;
	uint uVar2;
	int* piVar3;
	CFx* pNewFx;
	uint outCount;

	pByteCode->GetChunk();

	*outClass = pByteCode->GetU32();

	pCurFx = this->aEffectCategory[*outClass];
	if (pCurFx == (CFxPoolManagerFather*)0x0) {
		outCount = pByteCode->GetU32();
		*pOutEffectObj = (CFx*)0x0;
		if (outCount != 0) {
			if (*outClass == FX_TYPE_GROUP) {
				pNewFx = new CFxGroup[outCount];
			}
			else {
				if (*outClass == FX_TYPE_PATH) {
					pNewFx = new CFxPath[outCount];
				}
				else {
					pNewFx = (CFx*)0x0;
				}
			}

			*pOutEffectObj = pNewFx;

			pNewFx = *pOutEffectObj;
			if (pNewFx != (CFx*)0x0) {
				pNewFx->AddAll(pByteCode, pNewFx, outCount);
			}
		}
	}
	else {
		switch (*outClass) {
		case FX_TYPE_COMPOSITE:
		{
			outCount = pCurFx->InstallFxScenaricData(pByteCode);
		}
			break;
		case FX_TYPE_PARTICLE:
		{
			outCount = pCurFx->InstallFxScenaricData(pByteCode);
		}
			break;
		case FX_TYPE_SOUND:
		{
			outCount = pCurFx->InstallFxScenaricData(pByteCode);
		}
			break;
		case FX_TYPE_RANDOM:
		case FX_TYPE_LOD:
		{
			outCount = pCurFx->InstallFxScenaricData(pByteCode);
		}
		}

		*pOutEffectObj = (CFx*)0x0;
	}

	return outCount;
}

CNewFx::CNewFx()
{
	this->field_0x30 = gF32Vertex4Zero;
	this->field_0x50 = gF32Vertex4Zero;

	this->field_0x4c = 1.0f;
	this->field_0x48 = 1.0f;
	this->field_0x44 = 1.0f;
	this->field_0x40 = 1.0f;

	this->field_0x60 = gF32Vector4Zero;

	Func_0x30(1.0f);

	this->field_0x4 = 0;
	this->field_0x8 = 0;
	this->field_0x1c = (undefined4*)0x0;
	this->field_0x20 = 0;
	this->field_0x18 = 1;
	this->flags = 0;

	return;
}

void CNewFx::Func_0x30(float param_1)
{
	this->field_0x70 = param_1;
}

CFx::CFx()
{
	this->flags = 3;
	this->field_0x48 = 0;
	this->field_0x3c = 0;
	this->field_0x38 = 0;
	this->field_0x34 = 0;
	this->field_0x30 = 0;

	(this->vector_0x20).w = 1.0f;
	(this->vector_0x20).z = 1.0f;
	(this->vector_0x20).y = 1.0f;
	(this->vector_0x20).x = 1.0f;
}

void CFx::CheckpointResetAll(uint count, CFx* aFx)
{

}
