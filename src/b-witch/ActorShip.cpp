#include "ActorShip.h"
#include "ActorHero.h"
#include "ActorHunter.h"
#include "ActorCareBoy.h"
#include "ActorDCA.h"
#include "MemoryStream.h"
#include "FrontendDisp.h"
#include "EventManager.h"
#include "MathOps.h"
#include "PathManager.h"

void CActorShip::Create(ByteCode* pByteCode)
{
	S_ACTOR_STREAM_REF* pSVar1;
	S_NTF_TARGET_STREAM_REF* pSVar2;
	S_STREAM_EVENT_CAMERA* pSVar3;
	int iVar4;
	int* piVar5;
	int iVar6;
	uint uVar10;
	int iVar11;
	float fVar12;
	
	CActor::Create(pByteCode);

	this->careBoyStreamRef = S_ACTOR_STREAM_REF::Create(pByteCode);
	
	this->fxArrayManager.Create(pByteCode);

	this->jawCareBoyStreamRef.index = pByteCode->GetS32();
	this->airCannonStreamRef.index = pByteCode->GetS32();
	this->hunterStreamRef.index = pByteCode->GetS32();

	this->field_0x214 = pByteCode->GetF32();
	this->field_0x210 = pByteCode->GetF32();
	this->field_0x218 = pByteCode->GetF32();
	this->field_0x21c = pByteCode->GetF32();
	this->field_0x220 = pByteCode->GetF32();
	this->field_0x224 = pByteCode->GetF32() * 0.01745329f;
	this->field_0x230 = pByteCode->GetF32();
	this->kimTouchRange = pByteCode->GetF32();
	this->field_0x22c = pByteCode->GetF32();
	this->field_0x25c = this->field_0x224 / this->field_0x21c;
	this->field_0x260 = this->field_0x224 / this->field_0x220;
	(this->fxArrayManager).field_0x18 = pByteCode->GetS32();
	(this->fxArrayManager).nb = 0;
	(this->fxArrayManager).flags = 0;
	this->field_0x234 = pByteCode->GetS32();
	this->field_0x238 = pByteCode->GetS32();
	this->field_0x1ac.index = pByteCode->GetS32();

	this->field_0x18c[0] = pByteCode->GetU32();
	this->field_0x18c[1] = pByteCode->GetU32();
	this->field_0x18c[2] = pByteCode->GetU32();
	this->field_0x18c[3] = pByteCode->GetU32();

	this->field_0x19c = pByteCode->GetU32();
	this->field_0x1a0 = pByteCode->GetU32();
	this->field_0x1a4 = pByteCode->GetU32();
	this->field_0x1a8 = pByteCode->GetU32();
	
	this->vision.Create(this, pByteCode);

	field_0x23c.Create(pByteCode);

	return;
}

void CActorShip::Init()
{
	CActor* this_00;
	int* piVar1;
	CNewFx* pCVar2;
	bool bVar3;
	S_NTF_TARGET_STREAM_REF* pSVar4;
	ACTIVE_PATHS_CONTAINER* puVar6;
	int iVar5;
	S_ACTOR_STREAM_REF* pActorStreamRef;
	int iVar7;
	int iVar8;
	float fVar9;
	float fVar10;
	float fVar11;

	this->field_0x23c.Init();

	CActor::Init();

	this->careBoyStreamRef->Init();

	this->jawCareBoyStreamRef.Init();
	this->airCannonStreamRef.Init();
	this->hunterStreamRef.Init();
	this->field_0x1ac.Init();

	static_cast<CActorCareBoy*>(this->careBoyStreamRef->aEntries[0].Get())->LinkToParent(this, this->field_0x18c[0]);
	static_cast<CActorCareBoy*>(this->careBoyStreamRef->aEntries[1].Get())->LinkToParent(this, this->field_0x18c[1]);
	static_cast<CActorCareBoy*>(this->careBoyStreamRef->aEntries[2].Get())->LinkToParent(this, this->field_0x18c[2]);
	static_cast<CActorCareBoy*>(this->careBoyStreamRef->aEntries[3].Get())->LinkToParent(this, this->field_0x18c[3]);

	this->hunterStreamRef.Get()->LinkToActor(this, this->field_0x19c, 0);
	this->airCannonStreamRef.Get()->LinkToActor(this, this->field_0x1a8, 0);

	this->pAnimationController->RegisterBone(this->field_0x1a0);

	CActorHunter* pActorHunter = static_cast<CActorHunter*>(this->hunterStreamRef.Get());
	pActorHunter->pActorShip = this;

	this->field_0x258 = this->vision.visionRange_0x34;
	this->field_0x308 = 0;
	this->field_0x305 = 0;

	ClearLocalData();

	return;
}

void CActorShip::Manage()
{
	this->jawCareBoyStreamRef.Get()->UpdatePosition(&this->currentLocation, true);
	CActor::Manage();

	return;
}

void CActorShip::Reset()
{
	CActor::Reset();

	if (this->curBehaviourId == 2) {
		SetState(SHIP_FLY_STATE_PHASE_1, -1);
	}

	this->field_0x23c.Reset(this);

	ClearLocalData();

	return;
}

struct S_SAVE_CLASS_SHIP
{
	int field_0x0;
	edF32VECTOR3 position;
};

void CActorShip::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_SHIP* pSaveData = reinterpret_cast<S_SAVE_CLASS_SHIP*>(pData);

	pSaveData->field_0x0 = this->field_0x308;
	pSaveData->position = this->currentLocation.xyz;

	return;
}

void CActorShip::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_SHIP* pSaveData = reinterpret_cast<S_SAVE_CLASS_SHIP*>(pData);
	if ((mode == 1) && (this->field_0x308 = pSaveData->field_0x0, this->field_0x308 == 1)) {
		this->field_0x330 = pSaveData->position;
		this->currentLocation.xyz = this->field_0x330;
	}

	return;
}

CBehaviour* CActorShip::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == SHIP_BEHAVIOUR_FLY) {
		pBehaviour = &this->behaviourFly;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorShip::_gStateCfg_SHP[8] = {
	StateConfig(8, 0),
	StateConfig(8, 0),
	StateConfig(8, 0),
	StateConfig(8, 0),
	StateConfig(8, 0),
	StateConfig(8, 0),
	StateConfig(8, 0),
	StateConfig(8, 0)
};

StateConfig* CActorShip::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 8);
		pStateConfig = _gStateCfg_SHP + state + -5;
	}

	return pStateConfig;
}

int CActorShip::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActor* pCVar1;
	int* piVar2;
	CNewFx* pFx;
	CActor* pCVar4;
	bool bVar5;
	CFrontendDisplay* pCVar6;
	int result;
	S_NTF_TARGET_STREAM_REF* pSVar8;
	ACTIVE_PATHS_CONTAINER* pCurPathContainer;
	CFxHandle* pCurHandle;
	int iVar10;
	uint uVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	uint uStack4;

	pCVar6 = CScene::ptable.g_FrontendManager_00451680;
	if (msg == 3) {
		result = this->actorState;
		if ((result == SHIP_FLY_STATE_GOBING) || (result == 10)) {
			this->field_0x305 = 1;
		}
		else {
			CActorHunter* pHunter = static_cast<CActorHunter*>(this->hunterStreamRef.Get());
			pHunter->GetLifeInterfaceOther()->SetPriority(0);
			pCVar6->DeclareInterface(FRONTEND_INTERFACE_ENEMY_LIST, 0);
			this->field_0x23c.Switch(this);
			this->field_0x23c.PostSwitch(this);
			
			result = (this->fxArrayManager).nb;
			if (result != 0) {
				iVar10 = 0;
				if (0 < result) {
					do {
						pCurPathContainer = this->fxArrayManager.aActivePathContainers + iVar10;
						if (((this->fxArrayManager).flags & 1 << (pCurPathContainer->field_0x0 & 0x1f)) != 0) {
							uVar11 = pCurPathContainer->field_0x4;
							if (uVar11 != 0) {
								pCurHandle = pCurPathContainer->aActivePaths + uVar11 - 1;
								do {
									pCurHandle->Kill();
									pCurHandle->Reset();
									pCurHandle = pCurHandle + -1;
									uVar11 = uVar11 - 1;
								} while (uVar11 != 0);
							}
						}

						iVar10 = iVar10 + 1;
					} while (iVar10 < (this->fxArrayManager).nb);
				}

				(this->fxArrayManager).nb = 0;
				(this->fxArrayManager).flags = 0;
			}

			pFx = (this->field_0x264).pFx;
			if (((pFx == static_cast<CNewFx*>(0x0)) || (result = (this->field_0x264).id, result == 0)) ||
				(result != pFx->id)) {
				bVar5 = false;
			}
			else {
				bVar5 = false;
				if (((pFx->flags & 2) != 0) && ((pFx->flags & 8) == 0)) {
					bVar5 = true;
				}
			}

			if (bVar5) {
				this->field_0x264.Kill();
				this->field_0x264.Reset();
			}

			pFx = (this->field_0x26c).pFx;
			if (((pFx == static_cast<CNewFx*>(0x0)) || (result = (this->field_0x26c).id, result == 0)) ||
				(result != pFx->id)) {
				bVar5 = false;
			}
			else {
				bVar5 = false;
				if (((pFx->flags & 2) != 0) && ((pFx->flags & 8) == 0)) {
					bVar5 = true;
				}
			}

			if (bVar5) {
				this->field_0x26c.Kill();
				this->field_0x26c.Reset();
			}

			this->field_0x308 = 2;
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			pCVar4 = this->hunterStreamRef.Get();
			pCVar4->flags = pCVar4->flags & 0xfffffffd;
			pCVar4->flags = pCVar4->flags | 1;
			pCVar1 = (this->jawCareBoyStreamRef).Get();
			pCVar1->flags = pCVar1->flags & 0xfffffffd;
			pCVar1->flags = pCVar1->flags | 1;
		}

		result = 1;
	}
	else {
		if (msg == 0x10) {
			SetShipAnimations(8);
			result = 1;
		}
		else {
			if (msg == 0xf) {
				SetShipAnimations(0xc);
				result = 1;
			}
			else {
				if (msg == 0x5c) {
					this->field_0x308 = 1;
					this->vision.location = this->currentLocation;
					this->vision.rotationQuat = this->rotationQuat;
					pCVar1 = (this->jawCareBoyStreamRef).Get();
					pCVar1->rotationEuler.xyz = this->rotationEuler.xyz;
					pCVar1->UpdatePosition(&this->currentLocation, true);
					result = 1;
				}
				else {
					if (msg == 0x31) {
						this->nbCareBoys = this->nbCareBoys + -1;
						if ((pSender == this->careBoyStreamRef->aEntries[0].Get()) ||
							(pSender == this->careBoyStreamRef->aEntries[1].Get())) {
							result = 0xd;
							uStack4 = 8;
						}
						else {
							result = 0xe;
							uStack4 = 9;
						}

						DoMessage((this->jawCareBoyStreamRef).Get(), (ACTOR_MESSAGE)0x22, (void*)uStack4);

						pCVar6 = CScene::ptable.g_FrontendManager_00451680;
						if (this->nbCareBoys < 1) {
							CActorHunter* pHunter = static_cast<CActorHunter*>(this->hunterStreamRef.Get());
							pHunter->GetLifeInterfaceOther()->SetPriority(2);
							pCVar6->DeclareInterface(FRONTEND_INTERFACE_ENEMY_LIST, pHunter->GetLifeInterfaceOther());
							SetState(SHIP_FLY_STATE_FLAME_THROWER, result);
						}
						else {
							PlayAnim(result);
						}

						result = 0;
					}
					else {
						if (msg == 0x4c) {
							_msg_hit_param* pHitParam = static_cast<_msg_hit_param*>(pMsgParam);
							// BUG?
							if ((this->field_0x24c == (CActor*)0x0) || ((this->field_0x24c != (CActor*)0x0 &&
									(this->field_0x250 < *reinterpret_cast<float*>(&pMsgParam))))) {
								this->field_0x24c = pSender;
								this->field_0x250 = pHitParam->field_0x30;
							}

							result = 1;
						}
						else {
							if (msg == 0x27) {
								SetShipAnimations(0xb);
								result = 1;
							}
							else {
								result = CActor::InterpretMessage(pSender, msg, pMsgParam);
							}
						}
					}
				}
			}
		}
	}

	return result;
}

void CActorShip::ClearLocalData()
{
	int iVar5;
	bool bVar3;

	if (this->field_0x308 == 1) {
		this->currentLocation.xyz = this->field_0x330;
		UpdatePosition(&this->currentLocation, true);
	}

	this->vision.location = this->currentLocation;
	this->vision.rotationQuat = this->rotationQuat;

	CActor* this_00 = this->jawCareBoyStreamRef.Get();
	this_00->rotationEuler.xyz = this->rotationEuler.xyz;
	this_00->UpdatePosition(&this->currentLocation, true);

	this->field_0x24c = (CActor*)0x0;
	this->field_0x250 = 0.0f;

	if (this->careBoyStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
		iVar5 = 0;
	}
	else {
		iVar5 = this->careBoyStreamRef->entryCount;
	}

	this->nbCareBoys = iVar5;
	iVar5 = (this->fxArrayManager).nb;
	if (iVar5 != 0) {
		int iVar7 = 0;
		if (0 < iVar5) {
			do {
				ACTIVE_PATHS_CONTAINER* puVar6 = (this->fxArrayManager).aActivePathContainers + iVar7;
				if (((this->fxArrayManager).flags & 1 << (puVar6->field_0x0 & 0x1f)) != 0) {
					int iVar8 = puVar6->field_0x4;
					if (iVar8 != 0) {
						CFxHandle* pCurHandle = puVar6->aActivePaths + iVar8 + -1;
						do {
							pCurHandle->Kill();
							pCurHandle->Reset();
							pCurHandle = pCurHandle + -1;
							iVar8 = iVar8 + -1;
						} while (iVar8 != 0);
					}
				}

				iVar7 = iVar7 + 1;
			} while (iVar7 < (this->fxArrayManager).nb);
		}

		(this->fxArrayManager).nb = 0;
		(this->fxArrayManager).flags = 0;
	}

	CNewFx* pFx = (this->field_0x264).pFx;
	if (((pFx == (CNewFx*)0x0) || (iVar5 = (this->field_0x264).id, iVar5 == 0)) ||
		(iVar5 != pFx->id)) {
		bVar3 = false;
	}
	else {
		bVar3 = false;
		if (((pFx->flags & 2) != 0) && ((pFx->flags & 8) == 0)) {
			bVar3 = true;
		}
	}

	if (bVar3) {
		this->field_0x264.Kill();
		this->field_0x264.Reset();
	}

	pFx = (this->field_0x26c).pFx;
	if (((pFx == (CNewFx*)0x0) || (iVar5 = (this->field_0x26c).id, iVar5 == 0)) ||
		(iVar5 != pFx->id)) {
		bVar3 = false;
	}
	else {
		bVar3 = false;
		if (((pFx->flags & 2) != 0) && ((pFx->flags & 8) == 0)) {
			bVar3 = true;
		}
	}

	if (bVar3) {
		this->field_0x26c.Kill();
		this->field_0x26c.Reset();
	}

	return;
}

void CActorShip::SetShipAnimations(int state)
{
	int inAnimType;

	switch (state) {
	case 7:
		this->hunterStreamRef.Get()->LinkToActor(this, this->field_0x19c, 0);
		inAnimType = 0xc;
		this->field_0x30c = 7;
		break;
	case 8:
		this->field_0x30c = 8;
		inAnimType = 0xd;
		break;
	case 9:
		this->hunterStreamRef.Get()->UnlinkFromActor();
		inAnimType = 0xe;
		this->field_0x30c = 9;
		break;
	case 10:
		this->field_0x30c = 10;
		inAnimType = 0xf;
		break;
	case 0xb:
		this->field_0x30c = 0xb;
		inAnimType = 0x10;
		break;
	case 0xc:
		this->field_0x30c = 0xc;
		inAnimType = 0x11;
		break;
	default:
		this->field_0x30c = 8;
		inAnimType = 0xd;
	}

	this->hunterStreamRef.Get()->PlayAnim(inAnimType);
	PlayAnim(this->field_0x30c);

	return;
}

void CActorShip::BehaviourShipFLY_InitState(int newState)
{
	int prevState;
	float fVar2;
	float fVar3;
	float fVar4;
	float unaff_f20;
	edF32VECTOR4 local_30;
	undefined4 local_14;
	undefined4 local_10;
	undefined4 local_c;
	undefined4 local_8;
	undefined4 local_4;
	CActor* pActor;
	CNewFx* pFx;
	CFxManager* pFxManager;

	pFxManager = CScene::ptable.g_EffectsManager_004516b8;
	switch (newState) {
	case SHIP_FLY_STATE_PHASE_1:
		if (((this->vision).flags & 0x40000000) != 0) {
			(this->vision).flags = (this->vision).flags ^ 0x40000000;
		}
		break;
	case SHIP_FLY_STATE_FLAME_THROWER:
		prevState = this->hunterStreamRef.Get()->actorState;
		if ((prevState != SHIP_FLY_STATE_CLOSING) && (prevState != 10)) {
			CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x26c, this->field_0x238, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x26c.SetPosition(&this->currentLocation);
			this->field_0x26c.SetRotationEuler(&this->rotationEuler);
			this->field_0x26c.Start();
		}
		break;
	case SHIP_FLY_STATE_THROWING:
		(this->vision).visionRange_0x34 = 1.0f;
		if (((this->vision).flags & 0x40000000) == 0) {
			(this->vision).flags = (this->vision).flags ^ 0x40000000;
		}

		pFxManager->GetDynamicFx(&this->field_0x264, this->field_0x234, FX_MATERIAL_SELECTOR_NONE);
		SV_GetBoneWorldPosition(this->field_0x1a0, &local_30);

		this->field_0x264.SetPosition(&local_30);
		this->field_0x264.SetRotationEuler(&this->rotationEuler);
		this->field_0x264.Start();

		this->field_0x304 = 0;
		DoMessage(this->hunterStreamRef.Get(), (ACTOR_MESSAGE)0x3a, (MSG_PARAM)1);
		DoMessage(this->hunterStreamRef.Get(), (ACTOR_MESSAGE)0x36, (MSG_PARAM)1);
		break;
	case SHIP_FLY_STATE_CLOSING:
		DoMessage(this->hunterStreamRef.Get(), (ACTOR_MESSAGE)0x36, 0);
		break;
	case SHIP_FLY_STATE_GOBING:
		pActor = this->field_0x310;
		this->field_0x320 = pActor->currentLocation;
		break;
	case 10:
		SetShipAnimations(9);
		DoMessage(this->hunterStreamRef.Get(), (ACTOR_MESSAGE)0x36, (MSG_PARAM)1);
		DoMessage((this->jawCareBoyStreamRef).Get(), (ACTOR_MESSAGE)0x42, 0);
		static_cast<CActorHunter*>(this->hunterStreamRef.Get())->FUN_003ba4e0(180.0f);
	}
	return;
}

void CActorShip::BehaviourShipFLY_Manage()
{
	int* piVar1;
	CActorDCA* pDCA;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	CFrontendDisplay* pCVar5;
	CEventManager* pCVar6;
	CFxManager* pFxManager;
	bool bVar7;
	int iVar8;
	CLifeInterface* this_01;
	S_NTF_TARGET_STREAM_REF* pSVar9;
	ACTIVE_PATHS_CONTAINER* pAVar10;
	edF32VECTOR4* pSplinePoint;
	uint uVar12;
	int iVar13;
	ACTIVE_PATHS_CONTAINER* puVar14;
	CFxHandle* pCurHandle;
	int iVar16;
	uint uVar17;
	CActor* pCVar18;
	CPathFollow* pCVar19;
	float fVar20;
	float fVar21;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	undefined4 local_18;
	undefined4 local_14;
	float local_10;
	float local_c;
	float local_8;
	undefined4 local_4;
	CNewFx* pFx;
	CActorHunter* pHunter;
	S_ACTOR_STREAM_REF* pFireShotStreamRef;

	pCVar6 = CScene::ptable.g_EventManager_006f5080;
	pCVar5 = CScene::ptable.g_FrontendManager_00451680;
	switch (this->actorState) {
	case SHIP_FLY_STATE_PHASE_1:
		this->field_0x330 = this->currentLocation.xyz;
		if (this->field_0x24c != (CActor*)0x0) {
			DoMessage(this->field_0x24c, (ACTOR_MESSAGE)3, 0);
			this->field_0x24c = (CActor*)0x0;
		}
		break;
	case SHIP_FLY_STATE_FLAME_THROWER:
		local_8 = (((this->jawCareBoyStreamRef).Get())->rotationEuler).x;
		pDCA = static_cast<CActorDCA*>(this->airCannonStreamRef.Get());
		pFireShotStreamRef = pDCA->fireShot.pActorStreamRef;
		iVar13 = 0;
		if (pFireShotStreamRef != (S_ACTOR_STREAM_REF*)0x0) {
			iVar13 = pFireShotStreamRef->entryCount;
		}

		if (iVar13 != 0) {
			do {
				iVar13 = iVar13 + -1;
				pCVar18 = pFireShotStreamRef->aEntries[iVar13].Get();
				if (((pCVar18->actorState != SHIP_FLY_STATE_FLAME_THROWER) && (pCVar18->actorState != 0xe)) && (iVar8 = edEventComputeZoneAgainstVertex(pCVar6->activeChunkId, (this->field_0x1ac).Get(),
						&pCVar18->currentLocation, 0), iVar8 != 2)) goto LAB_003780f8;
			} while (iVar13 != 0);
		}

		pCVar18 = (CActor*)0x0;

	LAB_003780f8:
		if (pCVar18 != (CActor*)0x0) {
			this->field_0x310 = pCVar18;
			edF32Vector4SubHard(&eStack64, &this->currentLocation, &this->field_0x310->currentLocation);
			fVar20 = edF32Vector4GetDistHard(&eStack64);
			this->field_0x300 = fVar20 / 0.7f;
			this->field_0x310->SetState(0x12, -1);
			SetState(SHIP_FLY_STATE_GOBING, -1);
		}

		bVar7 = SV_UpdateValue(this->field_0x224 + this->rotationEuler.x, this->field_0x25c, &local_8);
		if (bVar7 == false) {
			(((this->jawCareBoyStreamRef).Get())->rotationEuler).x = local_8;
			pDCA = static_cast<CActorDCA*>(this->airCannonStreamRef.Get());
			pDCA->UpdatePosition(&pDCA->currentLocation, true);
		}
		else {
			iVar13 = (this->fxArrayManager).field_0x8;
			if (((iVar13 <= (this->fxArrayManager).nb) && (iVar13 < (this->fxArrayManager).nbInts)) &&
				(pAVar10 = (this->fxArrayManager).aActivePathContainers, ((this->fxArrayManager).flags & 1 << (pAVar10->field_0x0 & 0x1f)) != 0)) {
				iVar13 = pAVar10->field_0x4;
				if (iVar13 != 0) {
					pCurHandle = pAVar10->aActivePaths + iVar13 + -1;
					do {
						pCurHandle->Stop();
						iVar13 = iVar13 + -1;
						pCurHandle = pCurHandle + -1;
					} while (iVar13 != 0);
				}

				(this->fxArrayManager).flags = (this->fxArrayManager).flags &
					~(1 << (((this->fxArrayManager).aActivePathContainers)->field_0x0 & 0x1f));

				iVar13 = (this->fxArrayManager).field_0x8 + -1;

				if (0 < iVar13) {
					pAVar10 = (this->fxArrayManager).aActivePathContainers;
					memcpy(pAVar10, pAVar10 + 1, iVar13 * sizeof(ACTIVE_PATHS_CONTAINER));
				}

				(this->fxArrayManager).nb = (this->fxArrayManager).nb + -1;
			}

			SetState(SHIP_FLY_STATE_THROWING, -1);
		}

		ManageFlameThrower(0);
		break;
	case SHIP_FLY_STATE_THROWING:
		fVar20 = this->timeInAir;
		if (fVar20 < this->field_0x210) {
			(this->vision).visionRange_0x34 =
				(this->field_0x258 - 1.0f) * (fVar20 / this->field_0x210) + 1.0f;
		}

		ManageFlameThrower(1);

		if ((this->field_0x214 * 0.75f < fVar20) && (this->field_0x304 == 0)) {
			if ((this->fxArrayManager).nb < (this->fxArrayManager).nbInts) {
				do {
					iVar16 = rand();
					iVar13 = (this->fxArrayManager).nbInts;
					uVar17 = iVar16 % iVar13;
					if (iVar13 == 0) {
						trap(7);
					}

					uVar12 = 1 << (uVar17 & 0x1f);
					bVar7 = ((this->fxArrayManager).flags & uVar12) != 0;
				} while (bVar7);

				if (!bVar7) {
					(this->fxArrayManager).aActivePathContainers[(this->fxArrayManager).nb].field_0x0 = uVar17;

					pFxManager = CScene::ptable.g_EffectsManager_004516b8;
					iVar13 = (this->fxArrayManager).aInts[uVar17];
					pAVar10 = (this->fxArrayManager).aActivePathContainers + (this->fxArrayManager).nb;
					if (iVar13 == -1) {
						pCVar19 = static_cast<CPathFollow*>(0x0);
					}
					else {
						pCVar19 = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + iVar13;
					}

					iVar13 = pCVar19->splinePointCount;
					pAVar10->field_0x4 = iVar13;
					if (iVar13 != 0) {
						pCurHandle = pAVar10->aActivePaths + iVar13 + -1;
						do {
							iVar13 = iVar13 + -1;
							pFxManager->GetDynamicFx(pCurHandle, (this->fxArrayManager).field_0x18, FX_MATERIAL_SELECTOR_NONE);
							if (pCVar19->aSplinePoints == static_cast<edF32VECTOR4*>(0x0)) {
								pSplinePoint = &gF32Vertex4Zero;
							}
							else {
								pSplinePoint = pCVar19->aSplinePoints + iVar13;
							}

							pCurHandle->SetPosition(pSplinePoint);
							pCurHandle->Start();

							pCurHandle = pCurHandle + -1;
						} while (iVar13 != 0);
					}

					(this->fxArrayManager).flags = (this->fxArrayManager).flags | uVar12;
					(this->fxArrayManager).nb = (this->fxArrayManager).nb + 1;
				}
			}

			this->field_0x304 = 1;
		}

		if (this->field_0x214 < fVar20) {
			(this->vision).visionRange_0x34 = 1.0f;
			SetState(SHIP_FLY_STATE_CLOSING, -1);
		}
		break;
	case SHIP_FLY_STATE_CLOSING:
		if (this->field_0x218 <= this->timeInAir) {
			SetState(SHIP_FLY_STATE_FLAME_THROWER, -1);
		}
		local_c = (((this->jawCareBoyStreamRef).Get())->rotationEuler).x;
		SV_UpdateValue(this->rotationEuler.x, this->field_0x260, &local_c);
		(((this->jawCareBoyStreamRef).Get())->rotationEuler).x = local_c;
		(this->jawCareBoyStreamRef).Get()->UpdatePosition(&this->currentLocation, true);
		ManageFlameThrower(0);
		break;
	case SHIP_FLY_STATE_GOBING:
		local_10 = (((this->jawCareBoyStreamRef).Get())->rotationEuler).x;
		fVar20 = this->timeInAir;
		edF32Vector4SubHard(&eStack48, &this->currentLocation, &this->field_0x320);
		edF32Vector4ScaleHard(fVar20 / 0.7f, &eStack48, &eStack48);
		edF32Vector4AddHard(&eStack48, &eStack48, &this->field_0x320);
		this->field_0x310->UpdatePosition(&eStack48, true);
		bVar7 = SV_UpdateValue(this->rotationEuler.x, 0.7f, &local_10);
		if (bVar7 != false) {
			(this->field_0x310)->SetState(0xd, -1);
			SetState(10, -1);
		}

		(((this->jawCareBoyStreamRef).Get())->rotationEuler).x = local_10;
		(this->jawCareBoyStreamRef).Get()->UpdatePosition(&this->currentLocation, true);
		ManageFlameThrower(0);
		break;
	case 10:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if (this->field_0x305 == 0) {
				local_14 = 1;
				DoMessage(this->hunterStreamRef.Get(), (ACTOR_MESSAGE)0x44, (MSG_PARAM)1);
				SetState(SHIP_FLY_STATE_FLAME_THROWER, -1);
				SetShipAnimations(7);
			}
			else {
				this_01 = static_cast<CActorHunter*>(this->hunterStreamRef.Get())->GetLifeInterfaceOther();
				this_01->SetPriority(0);
				pCVar5->DeclareInterface(FRONTEND_INTERFACE_ENEMY_LIST, this_01);
				this->field_0x23c.Switch(this);
				this->field_0x23c.PostSwitch(this);
				
				iVar13 = (this->fxArrayManager).nb;
				if (iVar13 != 0) {
					iVar16 = 0;
					if (0 < iVar13) {
						do {
							puVar14 = this->fxArrayManager.aActivePathContainers + iVar16;
							if ((((this->fxArrayManager).flags & 1 << (puVar14->field_0x0 & 0x1f)) != 0) &&
								(uVar17 = puVar14->field_0x4, uVar17 != 0)) {
								CFxHandle* pCurHandle = puVar14->aActivePaths + uVar17 - 1;
								do {									
									uVar17 = uVar17 - 1;
									pCurHandle->Kill();
									pCurHandle->Reset();
									pCurHandle = pCurHandle + -1;
								} while (uVar17 != 0);
							}

							iVar16 = iVar16 + 1;
						} while (iVar16 < (this->fxArrayManager).nb);
					}

					(this->fxArrayManager).nb = 0;
					(this->fxArrayManager).flags = 0;
				}

				pFx = (this->field_0x264).pFx;
				if (((pFx == static_cast<CNewFx*>(0x0)) || (iVar13 = (this->field_0x264).id, iVar13 == 0)) ||
					(iVar13 != pFx->id)) {
					bVar7 = false;
				}
				else {
					bVar7 = false;
					if (((pFx->flags & 2) != 0) && ((pFx->flags & 8) == 0)) {
						bVar7 = true;
					}
				}

				if (bVar7) {
					this->field_0x264.Kill();
					this->field_0x264.Reset();
				}

				pFx = (this->field_0x26c).pFx;
				if (((pFx == static_cast<CNewFx*>(0x0)) || (iVar13 = (this->field_0x26c).id, iVar13 == 0)) ||
					(iVar13 != pFx->id)) {
					bVar7 = false;
				}
				else {
					bVar7 = false;
					if (((pFx->flags & 2) != 0) && ((pFx->flags & 8) == 0)) {
						bVar7 = true;
					}
				}

				if (bVar7) {
					this->field_0x26c.Kill();
					this->field_0x26c.Reset();
				}

				this->field_0x308 = 2;
				this->flags = this->flags & 0xfffffffd;
				this->flags = this->flags | 1;
				pHunter = static_cast<CActorHunter*>(this->hunterStreamRef.Get());
				pHunter->flags = pHunter->flags & 0xfffffffd;
				pHunter->flags = pHunter->flags | 1;

				pDCA = static_cast<CActorDCA*>((this->jawCareBoyStreamRef).Get());
				pDCA->flags = pDCA->flags & 0xfffffffd;
				pDCA->flags = pDCA->flags | 1;
			}
		}
		break;
	case 0xb:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(SHIP_FLY_STATE_FLAME_THROWER, -1);
			local_18 = 1;
			DoMessage(this->hunterStreamRef.Get(), (ACTOR_MESSAGE)0x44, (MSG_PARAM)1);
			SetShipAnimations(7);
		}
	}

	return;
}

void CActorShip::ManageFlameThrower(int param_2)
{
	bool bVar2;
	CActor* pCVar3;
	long lVar4;
	int iVar6;
	_msg_hit_param local_110;
	_msg_hit_param local_90;
	CActorDCA* pDCA;
	S_ACTOR_STREAM_REF* pFireShotActorStream;
	CActor* pLocalActor;

	pLocalActor = (CActor*)0x0;
	if ((param_2 != 0) && (pLocalActor = this->vision.ScanForTarget(CActorHero::_gThis, 1), pLocalActor != (CActor*)0x0)) {
		local_90.damage = this->field_0x230;
		local_90.projectileType = 0;
		local_90.flags = 0;
		DoMessage(CActorHero::_gThis, MESSAGE_KICKED, &local_90);
	}

	if ((pLocalActor == (CActor*)0x0) && (lVar4 = this->fxArrayManager.CheckForKimTouch(this->kimTouchRange), lVar4 != 0)) {
		local_110.damage = this->field_0x22c;
		local_110.projectileType = 0;
		local_110.flags = 0;
		DoMessage(CActorHero::_gThis, MESSAGE_KICKED, &local_110);
	}

	pDCA = static_cast<CActorDCA*>(this->airCannonStreamRef.Get());
	pFireShotActorStream = (pDCA->fireShot).pActorStreamRef;
	iVar6 = 0;
	if (pFireShotActorStream != (S_ACTOR_STREAM_REF*)0x0) {
		iVar6 = pFireShotActorStream->entryCount;
	}

	while (iVar6 != 0) {
		iVar6 = iVar6 + -1;
		pLocalActor = pFireShotActorStream->aEntries[iVar6].Get();
		if ((pLocalActor->actorState != SHIP_FLY_STATE_FLAME_THROWER) && (pLocalActor->actorState != 0xe)) {
			bVar2 = this->fxArrayManager.CheckForFireShotTouch(pLocalActor);
			if (bVar2 != false) {
				(pLocalActor)->SetState(0xd, -1);
			}

			if (((param_2 != 0) && (bVar2 == false)) &&
				(pCVar3 = this->vision.ScanForTarget(pLocalActor, 1), pCVar3 != (CActor*)0x0)) {
				(pLocalActor)->SetState(0xd, -1);
			}
		}
	}

	return;
}

void CBehaviourShip::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShip::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorShip*>(pOwner);

	return;
}

int CBehaviourShip::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourShip::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourShipFLY::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourShipFLY::Manage()
{
	this->pOwner->BehaviourShipFLY_Manage();

	return;
}

void CBehaviourShipFLY::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourShip::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(SHIP_FLY_STATE_PHASE_1, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourShipFLY::InitState(int newState)
{
	this->pOwner->BehaviourShipFLY_InitState(newState);

	return;
}

void CBehaviourShipFLY::TermState(int oldState, int newState)
{
	int iVar1;
	bool bVar2;
	CNewFx* pNewFx;
	CActorShip* pShip;

	pShip = this->pOwner;
	if (oldState == SHIP_FLY_STATE_FLAME_THROWER) {
		pNewFx = (pShip->field_0x26c).pFx;
		if (((pNewFx == static_cast<CNewFx*>(0x0)) || (iVar1 = (pShip->field_0x26c).id, iVar1 == 0)) ||
			(iVar1 != pNewFx->id)) {
			bVar2 = false;
		}
		else {
			bVar2 = false;
			if (((pNewFx->flags & 2) != 0) && ((pNewFx->flags & 8) == 0)) {
				bVar2 = true;
			}
		}

		if (((bVar2) && (pNewFx != static_cast<CNewFx*>(0x0))) &&
			((iVar1 = (pShip->field_0x26c).id, iVar1 != 0 && (iVar1 == pNewFx->id)))) {
			pNewFx->Stop(-1.0f);
		}
	}

	return;
}

int CBehaviourShipFLY::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

CFxArrayManager::CFxArrayManager()
{
	int* pBase;
	ACTIVE_PATHS_CONTAINER* pAVar1;

	this->nbInts = 0;
	this->nb = 0;
	this->flags = 0;
	this->aInts = (int*)0x0;
	this->field_0x18 = 0;
	this->aActivePathContainers = new ACTIVE_PATHS_CONTAINER[2];
	this->field_0x8 = 2;

	return;
}

void CFxArrayManager::Create(ByteCode* pByteCode)
{
	this->nbInts = pByteCode->GetS32();

	int iVar4 = this->nbInts;
	if (iVar4 != 0) {
		this->aInts = new int[iVar4];
		iVar4 = 0;
		if (0 < this->nbInts) {
			do {
				this->aInts[iVar4] = pByteCode->GetS32();
				iVar4 = iVar4 + 1;
			} while (iVar4 < this->nbInts);
		}
	}

	return;
}

bool CFxArrayManager::CheckForKimTouch(float param_1)
{
	int uVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	CFxHandle* pCurHandle;
	edF32VECTOR4* pfVar7;
	int iVar8;
	int iVar9;
	int curNb;

	curNb = this->nb;
	bVar5 = false;

	if (curNb != 0) {
		while ((curNb != 0 && (!bVar5))) {
			curNb = curNb + -1;
			iVar9 = this->aActivePathContainers[curNb].field_0x4;

			while ((iVar9 != 0 && (!bVar5))) {
				iVar8 = iVar9 + -1;
				pCurHandle = this->aActivePathContainers[curNb].aActivePaths + iVar9 + -1;
				CNewFx* pFx = pCurHandle->pFx;
				if ((pFx == (CNewFx*)0x0) ||
					((uVar1 = pCurHandle->id, uVar1 == 0 || (uVar1 != pFx->id)))) {
					pfVar7 = (edF32VECTOR4*)0x0;
				}
				else {
					pfVar7 = &pFx->position;
				}

				iVar9 = iVar8;

				if ((pfVar7 != (edF32VECTOR4*)0x0) &&
					(fVar2 = pfVar7->x - CActorHero::_gThis->currentLocation.x,
						fVar3 = pfVar7->y - CActorHero::_gThis->currentLocation.y,
						fVar4 = pfVar7->z - CActorHero::_gThis->currentLocation.z,
						fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4 < param_1 * param_1)) {
					bVar5 = true;
				}
			}
		}
	}

	return bVar5;
}

bool CFxArrayManager::CheckForFireShotTouch(CActor* pActor)
{
	int uVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	CFxHandle* pCurHandle;
	edF32VECTOR4* pfVar7;
	int iVar8;
	int iVar9;
	int curNb;

	curNb = this->nb;
	bVar5 = false;

	if (curNb != 0) {
		while ((curNb != 0 && (!bVar5))) {
			curNb = curNb + -1;
			iVar9 = this->aActivePathContainers[curNb].field_0x4;

			while ((iVar9 != 0 && (!bVar5))) {
				iVar8 = iVar9 + -1;
				pCurHandle = this->aActivePathContainers[curNb].aActivePaths + iVar9 + -1;
				CNewFx* pFx = pCurHandle->pFx;
				if ((pFx == (CNewFx*)0x0) ||
					((uVar1 = pCurHandle->id, uVar1 == 0 || (uVar1 != pFx->id)))) {
					pfVar7 = (edF32VECTOR4*)0x0;
				}
				else {
					pfVar7 = &pFx->position;
				}

				iVar9 = iVar8;

				if ((pfVar7 != (edF32VECTOR4*)0x0) &&
					(fVar2 = pfVar7->x - pActor->currentLocation.x,
						fVar3 = pfVar7->y - pActor->currentLocation.y,
						fVar4 = pfVar7->z - pActor->currentLocation.z,
						fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4 < 0.25f)) {
					bVar5 = true;
				}
			}
		}
	}

	return bVar5;
}
