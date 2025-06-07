#include "ActorMoney.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "FileManager3D.h"
#include "CameraViewManager.h"
#include "TimeController.h"
#include "LevelScheduleManager.h"

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
	IMPLEMENTATION_GUARD();
}

void CActorMoney::CheckpointReset()
{
	IMPLEMENTATION_GUARD();
}

void CActorMoney::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActorMoney::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
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

		if (this->curBehaviourId == 2) {
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
					CShadowShared::Init(pCVar7, this->pOwner->objectId);
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
	IMPLEMENTATION_GUARD();
}

void CBehaviourMoneyFlock::Manage()
{
	CActorMoney* pMoney;
	ed_3d_hierarchy* peVar2;
	edF32MATRIX4* peVar3;
	Timer* pTVar4;
	int iVar5;
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
	iVar5 = this->nbMoneyInstances;
	iVar6 = 0;
	if (0 < iVar5) {
		do {
			if ((pCurrentInstance->flags & 1) != 0) {
				this->field_0x18 = this->field_0x18 + 1;
				fVar8 = pCurrentInstance->DistSquared(&peVar3->rowT);
				if (fVar8 < fVar7 * fVar7) {
					this->pOwner->GetBehaviour(this->pOwner->curBehaviourId);
					if ((pCurrentInstance->flags & 1) != 0) {
						iVar5 = pCurrentInstance->state;
						if ((iVar5 == 5) || (iVar5 == 4)) {
							pCurrentInstance->SetAlive(0);
						}
						else {
							if (iVar5 == 3) {
								pCurrentInstance->State_GotoKim();
							}
							else {
								if (iVar5 == 1) {
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
							iVar5 = pCurrentInstance->state;
							if (((iVar5 != 5) && (iVar5 != 4)) && ((iVar5 == 3 || (iVar5 == 1)))) {
								fVar8 = edF32Between_0_2Pi(pCurrentInstance->angleRotY + GetTimer()->cutsceneDeltaTime * pCurrentInstance->field_0x90);
								pCurrentInstance->angleRotY = fVar8;
								edF32Matrix4RotateZHard(pCurrentInstance->field_0x98, &pCurrentInstance->pHierarchy->transformA, &gF32Matrix4Unit);
								edF32Matrix4RotateYHard(pCurrentInstance->angleRotY, &pCurrentInstance->pHierarchy->transformA, &pCurrentInstance->pHierarchy->transformA);
								(pCurrentInstance->pHierarchy->transformA).rowT = pCurrentInstance->currentPosition;
							}

							pCurrentInstance->FUN_003982c0();
							
							pCurrentInstance->field_0x64 = pCurrentInstance->currentPosition.xyz;
							pCurrentInstance->field_0x5c = pCurrentInstance->field_0x5c + GetTimer()->cutsceneDeltaTime;
						}
					}

					pCurrentInstance->pHierarchy->pHierarchySetup->pLightData = &(this->pOwner->lightConfig).config;
				}
			}

			iVar5 = this->nbMoneyInstances;
			iVar6 = iVar6 + 1;
			pCurrentInstance = pCurrentInstance + 1;
		} while (iVar6 < iVar5);
	}

	this->instantFlares.Manage(this->aMoneyInstances, iVar5);

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
