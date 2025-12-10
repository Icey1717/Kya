#include "ActorMoney.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "CameraViewManager.h"
#include "TimeController.h"
#include "LevelScheduler.h"
#include "ActorBonusServices.h"

void CActorMoney::Create(ByteCode* pByteCode)
{
	CActorMovable::Create(pByteCode);

	this->moneyValue = pByteCode->GetU32();
	(this->soundRef).index = pByteCode->GetS32();

	this->field_0x1d4 = pByteCode->GetS32();
	CActor::SV_InstallMaterialId(this->field_0x1d4);

	this->field_0x280 = CreateActorSound(1);

	this->field_0x288 = this->subObjA->boundingSphere;

	return;
}

void CActorMoney::Init()
{
	KyaUpdateObjA* pKVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	edF32MATRIX4* peVar5;
	edF32MATRIX4* peVar6;
	float fVar7;

	CActor::Init();

	this->soundRef.Init();

	this->lightConfig.lightAmbient = gF32Vector4Zero;
	this->lightConfig.lightDirections = gF32Matrix4Unit;
	this->lightConfig.lightColorMatrix = gF32Matrix4Unit;
	
	this->lightConfig.config.pLightAmbient = &this->lightConfig.lightAmbient;
	this->lightConfig.config.pLightDirections = &this->lightConfig.lightDirections;
	this->lightConfig.config.pLightColorMatrix = &this->lightConfig.lightColorMatrix;

	pKVar1 = this->subObjA;
	fVar7 = pKVar1->field_0x1c;
	if (pKVar1->visibilityDistance < fVar7) {
		pKVar1->visibilityDistance = fVar7 + 5.0f;
	}

	this->subObjA->field_0x20 = 50000.0f;

	return;
}

void CActorMoney::Reset()
{
	CActor::Reset();

	if (this->curBehaviourId == MONEY_BEHAVIOUR_ADD_ON) {
		CheckpointReset();
	}

	return;
}

void CActorMoney::CheckpointReset()
{
	CActorMovable::CheckpointReset();

	if (GetBehaviour(this->curBehaviourId) != (CBehaviour*)0x0) {
		static_cast<CBehaviourMoneyFlock*>(GetBehaviour(this->curBehaviourId))->CheckpointReset();
	}

	return;
}

void CActorMoney::SaveContext(void* pData, uint mode, uint maxSize)
{
	static_cast<CBehaviourMoneyFlock*>(GetBehaviour(this->curBehaviourId))->SaveContext(pData, mode, maxSize);

	return;
}

void CActorMoney::LoadContext(void* pData, uint mode, uint maxSize)
{
	static_cast<CBehaviourMoneyFlock*>(GetBehaviour(this->curBehaviourId))->LoadContext(pData, mode, maxSize);

	return;
}

CBehaviour* CActorMoney::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == MONEY_BEHAVIOUR_ADD_ON) {
		pBehaviour = new CBehaviourMoneyAddOn;
	}
	else {
		if (behaviourType == MONEY_BEHAVIOUR_FLOCK) {
			pBehaviour = new CBehaviourMoneyFlock;
		}
		else {
			pBehaviour = (CBehaviour*)0x0;
		}
	}
	return pBehaviour;
}

StateConfig CActorMoney::_gStateCfg_MNY[3] = {
	StateConfig(-1, 0),
	StateConfig(-1, 0),
	StateConfig(-1, 0)
};

StateConfig* CActorMoney::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 3);
		pStateConfig = _gStateCfg_MNY + state + -5;
	}

	return pStateConfig;
}

void CActorMoney::ChangeVisibleState(int bVisible)
{
	CBehaviourMoneyFlock* pFlock;
	int iVar2;

	if (bVisible == 0) {
		this->flags = this->flags & 0xffffbfff;

		if (this->curBehaviourId == MONEY_BEHAVIOUR_FLOCK) {
			pFlock = static_cast<CBehaviourMoneyFlock*>(GetBehaviour(this->curBehaviourId));
			iVar2 = 0;
			if (0 < pFlock->nbMoneyInstances) {
				do {
					pFlock->aMoneyInstances[iVar2].SetVisible(0);
					iVar2 = iVar2 + 1;
				} while (iVar2 < pFlock->nbMoneyInstances);
			}
		}
	}
	else {
		this->flags = this->flags | 0x4000;
	}

	return;
}

int CActorMoney::GetType()
{
	uint moneyType;

	if (this->moneyValue < 2) {
		moneyType = 0;
	}
	else {
		if (this->moneyValue < 6) {
			moneyType = 1;
		}
		else {
			moneyType = 2;
		}
	}

	return moneyType;
}

CBehaviourMoneyFlock::CBehaviourMoneyFlock()
{
	this->nbMoneyInstances = 0;
	this->nbSharedShadows = 0;
	this->field_0x18 = 0;
	this->aMoneyInstances = (CMnyInstance*)0x0;
	this->aSharedShadows = (CShadowShared*)0x0;
}

void CBehaviourMoneyFlock::Create(ByteCode* pByteCode)
{
	this->pathFollow.index = pByteCode->GetS32();

	return;
}

void CBehaviourMoneyFlock::Init(CActor* pOwner)
{
	CPathFollow* pPathFollow;
	uint uVar2;
	CShadow* pCVar3;
	int iVar4;
	int* piVar5;
	CMnyInstance* pCVar6;
	CShadowShared* pCVar7;
	int iVar8;
	edF32VECTOR4* peVar9;
	int iVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	CActorMoney* pMoney;

	this->pathFollow.Init();

	pPathFollow = this->pathFollow.Get();

	if (pPathFollow == (CPathFollow*)0x0) {
		this->nbMoneyInstances = 1;
	}
	else {
		this->nbMoneyInstances = pPathFollow->splinePointCount;
	}

	this->field_0x18 = 0;
	iVar4 = this->nbMoneyInstances;
	if (iVar4 < 0) {
		iVar4 = iVar4 + 3;
	}

	this->instantFlares.Create(0.5, 2.0, (iVar4 >> 2) + 1);

	this->pOwner = static_cast<CActorMoney*>(pOwner);

	this->field_0x20 = ((float)rand() / 2.147484e+09f) * 6.283185f;
	this->field_0x24 = ((float)rand() / 2.147484e+09f) * 6.283185f;
	this->field_0x28 = ((float)rand() / 2.147484e+09f) * 6.283185f;

	this->aMoneyInstances = new CMnyInstance[this->nbMoneyInstances];

	if ((this->pOwner->pShadow != (CShadow*)0x0) && ((this->pathFollow).Get() != (CPathFollow*)0x0)) {
		IMPLEMENTATION_GUARD(
		this->nbSharedShadows = this->nbMoneyInstances + -1;
		uVar2 = this->nbSharedShadows;
		piVar5 = (int*)operator.new.array((long)(int)(uVar2 * 0x50 + 0x10));
		pCVar7 = (CShadowShared*)
			__construct_new_array(piVar5, CShadowShared::CShadowShared, CShadowShared::~CShadowShared, 0x50, uVar2);
		this->aSharedShadows = pCVar7;)
	}

	if (this->nbMoneyInstances == 1) {
		pMoney = this->pOwner;
		pCVar6 = this->aMoneyInstances;
		pCVar6->Init(pMoney, &pMoney->baseLocation, &pMoney->field_0x288, 0);

		pCVar6->angleRotY = ((float)rand() / 2.147484e+09f) * 6.283185f;
		pCVar6->field_0x98 = ((float)rand() / 2.147484e+09f) * 6.283185f;
		pCVar6->field_0x90 = ((float)rand() / 2.147484e+09f) * 4.712389f + 1.570796f;

		pCVar6->SetVisible(0);
	}
	else {
		pCVar6 = this->aMoneyInstances;
		iVar4 = 0;
		if (0 < this->nbMoneyInstances) {
			do {
				peVar9 = this->pathFollow.Get()->aSplinePoints;
				if (peVar9 == (edF32VECTOR4*)0x0) {
					peVar9 = &gF32Vertex4Zero;
				}
				else {
					peVar9 = peVar9 + iVar4;
				}

				pCVar6->Init(this->pOwner, peVar9, &this->pOwner->field_0x288, iVar4);

				pCVar6->angleRotY = ((float)rand() / 2.147484e+09f) * 6.283185f;
				pCVar6->field_0x98 = ((float)rand() / 2.147484e+09f) * 6.283185f;
				pCVar6->field_0x90 = ((float)rand() / 2.147484e+09f) * 4.712389f + 1.570796f;
				pCVar6->SetVisible(0);

				iVar4 = iVar4 + 1;
				pCVar6 = pCVar6 + 1;
			} while (iVar4 < this->nbMoneyInstances);
		}

		if (this->nbSharedShadows != 0) {
			IMPLEMENTATION_GUARD(
			pCVar7 = this->aSharedShadows;
			iVar4 = 0;
			if (0 < this->nbSharedShadows) {
				do {
					CShadowShared::Init(pCVar7, this->pOwner->sectorId);
					fVar13 = gF32Vector4UnitY.w;
					fVar12 = gF32Vector4UnitY.z;
					fVar11 = gF32Vector4UnitY.y;
					(pCVar7->field_0x20).x = gF32Vector4UnitY.x;
					(pCVar7->field_0x20).y = fVar11;
					(pCVar7->field_0x20).z = fVar12;
					(pCVar7->field_0x20).w = fVar13;
					pCVar7->field_0x48 = 0.35;
					pCVar7->field_0x30 = 0x80457766;
					peVar9 = ((this->pathFollow).pPathFollow)->aSplinePoints;
					if (peVar9 == (edF32VECTOR4*)0x0) {
						peVar9 = &gF32Vertex4Zero;
					}
					else {
						peVar9 = peVar9 + iVar4 + 1;
					}
					iVar4 = iVar4 + 1;
					fVar13 = peVar9->y;
					fVar11 = peVar9->z;
					fVar12 = peVar9->w;
					(pCVar7->field_0x10).x = peVar9->x;
					(pCVar7->field_0x10).y = fVar13;
					(pCVar7->field_0x10).z = fVar11;
					(pCVar7->field_0x10).w = fVar12;
					pCVar7 = pCVar7 + 1;
				} while (iVar4 < this->nbSharedShadows);
			})
		}
	}

	pCVar3 = this->pOwner->pShadow;
	if (pCVar3 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		(pCVar3->base).field_0x48 = 0.35;);
	}

	this->pOwner->UpdateBoundingSphere(this->aMoneyInstances, this->nbMoneyInstances);

	if (this->aSharedShadows != (CShadowShared*)0x0) {
		IMPLEMENTATION_GUARD(
		iVar4 = pOwner->GameDListPatch_Register(this->nbSharedShadows << 2, 0);
		this->field_0xc = iVar4;)
	}

	return;
}

void CBehaviourMoneyFlock::Term()
{
	int iVar2;

	iVar2 = 0;
	if (0 < this->nbMoneyInstances) {
		do {
			this->aMoneyInstances[iVar2].Term();
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbMoneyInstances);
	}

	if (this->aMoneyInstances != (CMnyInstance*)0x0) {
		delete[] this->aMoneyInstances;
	}

	if (this->aSharedShadows != (CShadowShared*)0x0) {
		delete[] this->aSharedShadows;
	}

	return;
}

void CBehaviourMoneyFlock::Manage()
{
	CActorMoney* pMoney;
	ed_3d_hierarchy* peVar2;
	edF32MATRIX4* peVar3;
	Timer* pTVar4;
	int curState;
	int iVar6;
	CMnyInstance* pCurrentInstance;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;

	pCurrentInstance = this->aMoneyInstances;

	peVar3 = &CCameraManager::_gThis->transformationMatrix;
	fVar7 = this->pOwner->subObjA->visibilityDistance;
	this->field_0x18 = 0;
	pMoney = this->pOwner;
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(pMoney->lightingFloat_0xe0, pMoney, pMoney->lightingFlags, &(pMoney->lightConfig).config);
	curState = this->nbMoneyInstances;
	iVar6 = 0;
	if (0 < curState) {
		do {
			if ((pCurrentInstance->flags & 1) != 0) {
				this->field_0x18 = this->field_0x18 + 1;
				fVar8 = pCurrentInstance->DistSquared(&peVar3->rowT);
				if (fVar8 < fVar7 * fVar7) {
					this->pOwner->GetBehaviour(this->pOwner->curBehaviourId);
					if ((pCurrentInstance->flags & 1) != 0) {
						curState = pCurrentInstance->state;
						if ((curState == 5) || (curState == 4)) {
							pCurrentInstance->SetAlive(0);
						}
						else {
							if (curState == CActInstance::STT_INS_GOTO_KIM) {
								pCurrentInstance->State_GotoKim();
							}
							else {
								if (curState == CActInstance::STT_INS_WAIT) {
									pCurrentInstance->State_Wait();
								}
							}
						}

						if ((pCurrentInstance->flags & 4) == 0) {
							pCurrentInstance->flags = pCurrentInstance->flags & 0xfffffffd;
						}
						else {
							pCurrentInstance->flags = pCurrentInstance->flags | 2;
						}

						if ((pCurrentInstance->flags & 1) != 0) {
							curState = pCurrentInstance->state;
							if (((curState != 5) && (curState != 4)) && ((curState == CActInstance::STT_INS_GOTO_KIM || (curState == CActInstance::STT_INS_WAIT)))) {
								fVar8 = edF32Between_0_2Pi(pCurrentInstance->angleRotY + GetTimer()->cutsceneDeltaTime * pCurrentInstance->field_0x90);
								pCurrentInstance->angleRotY = fVar8;
								edF32Matrix4RotateZHard(pCurrentInstance->field_0x98, &pCurrentInstance->pHierarchy->transformA, &gF32Matrix4Unit);
								edF32Matrix4RotateYHard(pCurrentInstance->angleRotY, &pCurrentInstance->pHierarchy->transformA, &pCurrentInstance->pHierarchy->transformA);
								(pCurrentInstance->pHierarchy->transformA).rowT = pCurrentInstance->currentPosition;
							}

							pCurrentInstance->UpdateVisibility();
							
							pCurrentInstance->field_0x64 = pCurrentInstance->currentPosition.xyz;
							pCurrentInstance->field_0x5c = pCurrentInstance->field_0x5c + GetTimer()->cutsceneDeltaTime;
						}
					}

					pCurrentInstance->pHierarchy->pHierarchySetup->pLightData = &(this->pOwner->lightConfig).config;
				}
			}

			curState = this->nbMoneyInstances;
			iVar6 = iVar6 + 1;
			pCurrentInstance = pCurrentInstance + 1;
		} while (iVar6 < curState);
	}

	this->instantFlares.Manage(this->aMoneyInstances, curState);

	if (this->field_0x18 == 0) {
		this->pOwner->SetState(5, -1);
		pMoney = this->pOwner;
		pMoney->flags = pMoney->flags & 0xfffffffd;
		pMoney->flags = pMoney->flags | 1;
		pMoney = this->pOwner;
		pMoney->flags = pMoney->flags & 0xffffff7f;
		pMoney->flags = pMoney->flags | 0x20;

		pMoney->EvaluateDisplayState();
	}

	return;
}

void CBehaviourMoneyFlock::SectorChange(int oldSectorId, int newSectorId)
{
	CActorMoney* pMoney;
	int iVar2;

	if (oldSectorId != -1) {
		iVar2 = 0;
		if (0 < this->nbMoneyInstances) {
			do {
				this->aMoneyInstances[iVar2].SetVisible(0);
				iVar2 = iVar2 + 1;
			} while (iVar2 < this->nbMoneyInstances);
		}

		pMoney = this->pOwner;
		pMoney->flags = pMoney->flags & 0xffffff7f;
		pMoney->flags = pMoney->flags | 0x20;

		pMoney->EvaluateDisplayState();

		pMoney = this->pOwner;
		pMoney->flags = pMoney->flags & 0xfffffffd;
		pMoney->flags = pMoney->flags | 1;
	}

	if (newSectorId != -1) {
		this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
		pMoney = this->pOwner;
		pMoney->flags = pMoney->flags & 0xffffff5f;

		pMoney->EvaluateDisplayState();
	}

	return;
}

void CBehaviourMoneyFlock::Draw()
{
	IMPLEMENTATION_GUARD_SHADOW();
}

void CBehaviourMoneyFlock::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	int iVar1;
	CActorMoney* pMoney;

	this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
	pMoney = this->pOwner;
	pMoney->flags = pMoney->flags & 0xffffff5f;

	pMoney->EvaluateDisplayState();

	iVar1 = 0;
	if (0 < this->nbMoneyInstances) {
		do {
			this->aMoneyInstances[iVar1].Reset();
			this->aMoneyInstances[iVar1].SetAlive(1);

			iVar1 = iVar1 + 1;
		} while (iVar1 < this->nbMoneyInstances);
	}
	return;
}

bool CBehaviourMoneyFlock::InitDlistPatchable(int)
{
	IMPLEMENTATION_GUARD();
}

// Should be in: D:/Projects/b-witch/ActorMoney.h
void CBehaviourMoneyFlock::CheckpointReset()
{
	return;
}

struct S_SAVE_CLASS_MONEY
{
	uint bits[8]; // supports up to 256 money instances
};

void CBehaviourMoneyFlock::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_MONEY* pSaveData = reinterpret_cast<S_SAVE_CLASS_MONEY*>(pData);

	// Clear out all bitfields
	std::memset(pSaveData->bits, 0, sizeof(pSaveData->bits));

	for (uint i = 0; i < nbMoneyInstances; ++i)
	{
		const CMnyInstance& inst = aMoneyInstances[i];

		if (inst.flags & 1) // active?
		{
			uint wordIndex = i / 32;
			uint bitIndex = i % 32;

			pSaveData->bits[wordIndex] |= (1u << bitIndex);
		}
	}

	return;
}

void CBehaviourMoneyFlock::LoadContext(void* pData, uint mode, uint maxSize)
{
	uint uVar2;
	CActorMoney* pMoney;

	S_SAVE_CLASS_MONEY* pSaveData = reinterpret_cast<S_SAVE_CLASS_MONEY*>(pData);

	if (mode == 1) {
		uVar2 = 0;

		if (this->nbMoneyInstances != 0) {
			do {
				this->aMoneyInstances[uVar2].SetAlive(1 << (uVar2 & 0x1f) & pSaveData->bits[uVar2 >> 5]);

				uVar2 = uVar2 + 1;
			} while (uVar2 < this->nbMoneyInstances);
		}

		if (pSaveData == (S_SAVE_CLASS_MONEY*)0x0) {
			pMoney = this->pOwner;
			pMoney->flags = pMoney->flags & 0xffffff7f;
			pMoney->flags = pMoney->flags | 0x20;
			pMoney->EvaluateDisplayState();
			pMoney = this->pOwner;
			pMoney->flags = pMoney->flags & 0xfffffffd;
			pMoney->flags = pMoney->flags | 1;
		}
	}

	return;
}

void CInstantFlares::Create(float, float, int)
{
	IMPLEMENTATION_GUARD_FX();
}

void CInstantFlares::Manage(CActInstance* pInstances, int nbInstances)
{
	IMPLEMENTATION_GUARD_FX();
}

void CMnyInstance::SetState(int newState)
{
	//SOUND_SPATIALIZATION_PARAM local_4;
	CActorMoney* pActor;

	CActInstance::SetState(newState);

	if (this->state == 4) {
		pActor = static_cast<CActorMoney*>(this->pOwner);

		if ((this->flags & 0x20) == 0) {
			CLevelScheduler::gThis->Money_TakeFromScenery(pActor->moneyValue);
		}
		else {
			CLevelScheduler::gThis->Money_TakeFromBank(pActor->moneyValue);
		}

		IMPLEMENTATION_GUARD_AUDIO(
		local_4 = (float*)&this->field_0x64;
		CActorSound::SoundStart
		((CActorSound*)pActor->field_0x280, (CActor*)pActor, 0, (pActor->soundRef).pSound, 1, 2, &local_4);)
	}

	return;
}

float CMnyInstance::GetAngleRotY()
{
	return this->angleRotY;
}

void CBehaviourMoneyAddOn::Allocate(int nbNewInstances)
{
	uint count;
	CActorMoney* pOwner;
	int* pBase;
	CMnyInstance* pCVar1;
	int iVar2;
	int iVar3;
	int iVar4;

	this->nbMoneyInstances = nbNewInstances;
	this->field_0x18 = 0;

	count = this->nbMoneyInstances;
	if (count != 0) {
		this->aMoneyInstances = new CMnyInstance[count];
		iVar2 = this->nbMoneyInstances;
		pCVar1 = this->aMoneyInstances;
		iVar3 = 0;
		if (0 < iVar2) {
			do {
				pOwner = this->pOwner;
				pCVar1->Init(pOwner, &pOwner->baseLocation, &pOwner->field_0x288, -1);
				pCVar1->angleRotY = ((float)rand() / 2.147484e+09f) * 6.283185f;
				pCVar1->field_0x98 = ((float)rand() / 2.147484e+09f) * 6.283185f;
				pCVar1->field_0x90 = ((float)rand() / 2.147484e+09f) * 4.712389f + 1.570796f;
				pCVar1->SetVisible(0);
				iVar2 = this->nbMoneyInstances;
				iVar3 = iVar3 + 1;
				pCVar1 = pCVar1 + 1;
			} while (iVar3 < iVar2);
		}

		iVar3 = 0;
		if (0 < iVar2) {
			do {
				this->aMoneyInstances[iVar3].Reset();
				this->aMoneyInstances[iVar3].SetAlive(0);
				this->aMoneyInstances[iVar3].flags = this->aMoneyInstances[iVar3].flags | 0x20;
				iVar3 = iVar3 + 1;
				iVar2 = this->nbMoneyInstances;
			} while (iVar3 < iVar2);
		}

		if (iVar2 < 0) {
			iVar2 = iVar2 + 7;
		}

		this->instantFlares.Create(0.5f, 2.0f, (iVar2 >> 3) + 1);
	}

	return;
}

CMnyInstance** CBehaviourMoneyAddOn::Generate(edF32VECTOR4* pPosition, CAddOnGenerator_SubObj* pSubObj, int nbToSpawn, CMnyInstance** pInstance)
{
	CActorMoney* pMoney;
	int iVar3;
	CMnyInstance* pCurInstance;
	int byteOffset;
	int curInstanceIndex;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	curInstanceIndex = 0;

	if (this->field_0x18 == 0) {
		pMoney = this->pOwner;
		pMoney->SetState(5, -1);
		pMoney = this->pOwner;
		pMoney->flags = pMoney->flags | 2;
		pMoney->flags = pMoney->flags & 0xfffffffe;
		pMoney = this->pOwner;
		pMoney->flags = pMoney->flags | 0x80;
		pMoney->flags = pMoney->flags & 0xffffffdf;
		pMoney->EvaluateDisplayState();
	}

	byteOffset = 0;
	for (; (nbToSpawn != 0 && (curInstanceIndex < this->nbMoneyInstances)); curInstanceIndex = curInstanceIndex + 1) {
		pCurInstance = reinterpret_cast<CMnyInstance*>(reinterpret_cast<char*>(this->aMoneyInstances) + byteOffset);

		if ((pCurInstance->flags & 1) == 0) {
			nbToSpawn = nbToSpawn + -1;
			pCurInstance->SetPosition(pPosition);

			reinterpret_cast<CMnyInstance*>(reinterpret_cast<char*>(this->aMoneyInstances) + byteOffset)->SetAlive(1);
			reinterpret_cast<CMnyInstance*>(reinterpret_cast<char*>(this->aMoneyInstances) + byteOffset)->SetVisible(1);

			pCurInstance = reinterpret_cast<CMnyInstance*>(reinterpret_cast<char*>(this->aMoneyInstances) + byteOffset);
			pCurInstance->SetState(1);
			local_50.x = 0.0f;
			local_50.y = 0.0f;

			fVar6 = pSubObj->field_0x14;
			fVar5 = -fVar6;
			local_50.z = pSubObj->field_0x10 + fVar5 + (fVar6 - fVar5) * ((float)rand() / 2.147484e+09f);
			local_50.w = 0.0f;

			fVar6 = pSubObj->field_0x4;
			fVar5 = -fVar6;
			edF32Matrix4RotateXHard(pSubObj->field_0x0 + fVar5 + (fVar6 - fVar5) * ((float)rand() / 2.147484e+09f), &eStack64, &gF32Matrix4Unit);
			fVar6 = pSubObj->field_0xc;
			fVar5 = -fVar6;
			edF32Matrix4RotateYHard(pSubObj->field_0x8 + fVar5 + (fVar6 - fVar5) * ((float)rand() / 2.147484e+09f), &eStack64, &eStack64);
			edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &local_50);

			pCurInstance = reinterpret_cast<CMnyInstance*>(reinterpret_cast<char*>(this->aMoneyInstances) + byteOffset);
			pCurInstance->pathDelta = local_50;

			if (pInstance != (CMnyInstance**)0x0) {
				*pInstance = reinterpret_cast<CMnyInstance*>(reinterpret_cast<char*>(this->aMoneyInstances) + byteOffset);
				pInstance = pInstance + 1;
			}
		}

		byteOffset = byteOffset + sizeof(CMnyInstance);
	}

	return pInstance;
}

void CBehaviourMoneyAddOn::Create(ByteCode* pByteCode)
{
	this->field_0x18 = 0;
	this->nbMoneyInstances = 0;

	return;
}

void CBehaviourMoneyAddOn::Init(CActor* pOwner)
{
	edF32VECTOR4 local_10;

	this->pOwner = static_cast<CActorMoney*>(pOwner);

	this->field_0x20 = ((float)rand() / 2.147484e+09f) * 6.283185f;
	this->field_0x24 = ((float)rand() / 2.147484e+09f) * 6.283185f;
	this->field_0x28 = ((float)rand() / 2.147484e+09f) * 6.283185f;
	local_10.xyz = (this->pOwner->subObjA->boundingSphere).xyz;
	this->pOwner->SetLocalBoundingSphere(500.0f, &local_10);

	return;
}

void CBehaviourMoneyAddOn::Manage()
{
	CActorMoney* pCVar1;
	ed_3d_hierarchy* peVar2;
	edF32MATRIX4* peVar3;
	int iVar5;
	int iVar6;
	CMnyInstance* pInstanceIt;
	float visibilityDistance;
	float fVar8;
	float fVar9;
	float fVar10;

	pInstanceIt = this->aMoneyInstances;
	peVar3 = &CCameraManager::_gThis->transformationMatrix;
	visibilityDistance = this->pOwner->subObjA->visibilityDistance;
	this->field_0x18 = 0;
	pCVar1 = this->pOwner;
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(pCVar1->lightingFloat_0xe0, pCVar1, pCVar1->lightingFlags, &(pCVar1->lightConfig).config);
	iVar5 = this->nbMoneyInstances;
	iVar6 = 0;
	if (0 < iVar5) {
		do {
			if ((pInstanceIt->flags & 1) != 0) {
				this->field_0x18 = this->field_0x18 + 1;
				fVar8 = pInstanceIt->DistSquared(&peVar3->rowT);
				if (fVar8 < visibilityDistance * visibilityDistance) {
					pCVar1 = this->pOwner;
					pCVar1->GetBehaviour(pCVar1->curBehaviourId);

					if ((pInstanceIt->flags & 1) != 0) {
						iVar5 = pInstanceIt->state;
						if ((iVar5 == 5) || (iVar5 == 4)) {
							pInstanceIt->SetAlive(0);
						}
						else {
							if (iVar5 == CActInstance::STT_INS_GOTO_KIM) {
								pInstanceIt->State_GotoKim();
							}
							else {
								if (iVar5 == 1) {
									pInstanceIt->FUN_00397ba0();
								}
							}
						}

						if ((pInstanceIt->flags & 4) == 0) {
							pInstanceIt->flags = pInstanceIt->flags & 0xfffffffd;
						}
						else {
							pInstanceIt->flags = pInstanceIt->flags | 2;
						}

						if ((pInstanceIt->flags & 1) != 0) {
							iVar5 = pInstanceIt->state;

							if (((iVar5 != 5) && (iVar5 != 4)) && ((iVar5 == 3 || (iVar5 == 1)))) {
								fVar8 = edF32Between_0_2Pi(pInstanceIt->angleRotY + GetTimer()->cutsceneDeltaTime * pInstanceIt->field_0x90);
								pInstanceIt->angleRotY = fVar8;
								edF32Matrix4RotateZHard(pInstanceIt->field_0x98, &pInstanceIt->pHierarchy->transformA, &gF32Matrix4Unit);
								peVar2 = pInstanceIt->pHierarchy;
								edF32Matrix4RotateYHard(pInstanceIt->angleRotY, &peVar2->transformA, &peVar2->transformA);
								peVar2 = pInstanceIt->pHierarchy;
								(peVar2->transformA).rowT = pInstanceIt->currentPosition;
							}

							pInstanceIt->UpdateVisibility();
							fVar8 = pInstanceIt->currentPosition.y;
							fVar9 = pInstanceIt->currentPosition.z;
							pInstanceIt->field_0x64.x = pInstanceIt->currentPosition.x;
							pInstanceIt->field_0x64.y = fVar8;
							pInstanceIt->field_0x64.z = fVar9;
							pInstanceIt->field_0x5c = pInstanceIt->field_0x5c + GetTimer()->cutsceneDeltaTime;
						}
					}

					(pInstanceIt->pHierarchy)->pHierarchySetup->pLightData = &((this->pOwner)->lightConfig).config;
				}
			}

			iVar5 = this->nbMoneyInstances;
			iVar6 = iVar6 + 1;
			pInstanceIt = pInstanceIt + 1;
		} while (iVar6 < iVar5);
	}

	this->instantFlares.Manage(this->aMoneyInstances, iVar5);

	if (this->field_0x18 == 0) {
		pCVar1 = this->pOwner;
		pCVar1->SetState(5, -1);
		pCVar1 = this->pOwner;
		pCVar1->flags = pCVar1->flags & 0xfffffffd;
		pCVar1->flags = pCVar1->flags | 1;
		pCVar1 = this->pOwner;
		pCVar1->flags = pCVar1->flags & 0xffffff7f;
		pCVar1->flags = pCVar1->flags | 0x20;
		pCVar1->EvaluateDisplayState();
	}
	return;
}

void CBehaviourMoneyAddOn::SectorChange(int oldSectorId, int newSectorId)
{
	if ((newSectorId != -1) && (this->pOwner->sectorId != -1)) {
		CheckpointReset();
	}

	return;

}

void CBehaviourMoneyAddOn::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorMoney* pCVar1;
	int curMoneyIndex;

	pCVar1 = this->pOwner;
	pCVar1->flags = pCVar1->flags & 0xfffffffd;
	pCVar1->flags = pCVar1->flags | 1;
	pCVar1 = this->pOwner;
	pCVar1->flags = pCVar1->flags & 0xffffff7f;
	pCVar1->flags = pCVar1->flags | 0x20;
	pCVar1->EvaluateDisplayState();

	curMoneyIndex = 0;
	if (0 < this->nbMoneyInstances) {
		do {
			this->aMoneyInstances[curMoneyIndex].Reset();
			this->aMoneyInstances[curMoneyIndex].SetAlive(0);
			curMoneyIndex = curMoneyIndex + 1;
		} while (curMoneyIndex < this->nbMoneyInstances);
	}

	return;
}

void CBehaviourMoneyAddOn::SaveContext(void* pData, uint mode, uint maxSize)
{
	return;
}

void CBehaviourMoneyAddOn::LoadContext(void* pData, uint mode, uint maxSize)
{
	return;
}

void CBehaviourMoneyAddOn::CheckpointReset()
{
	CActorMoney* pCVar1;
	int curMoneyIndex;

	curMoneyIndex = 0;
	if (0 < this->nbMoneyInstances) {
		do {
			this->aMoneyInstances[curMoneyIndex].SetAlive(0);
			curMoneyIndex = curMoneyIndex + 1;
		} while (curMoneyIndex < this->nbMoneyInstances);
	}

	this->field_0x18 = 0;
	pCVar1 = this->pOwner;
	pCVar1->flags = pCVar1->flags & 0xfffffffd;
	pCVar1->flags = pCVar1->flags | 1;
	pCVar1 = this->pOwner;
	pCVar1->flags = pCVar1->flags & 0xffffff7f;
	pCVar1->flags = pCVar1->flags | 0x20;
	pCVar1->EvaluateDisplayState();

	return;
}
