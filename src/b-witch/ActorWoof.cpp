#include "ActorWoof.h"
#include "ActorHero.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "EventManager.h"
#include "WayPoint.h"

CActorWoof::CActorWoof()
{
	this->field_0x4c4 = 0.0f;

	return;
}

void CActorWoof::Create(ByteCode* pByteCode)
{
	int* piVar1;
	uint uVar2;
	int iVar3;
	float fVar4;

	CActorAutonomous::Create(pByteCode);

	this->field_0x420 = pByteCode->GetU32();
	this->field_0x424 = pByteCode->GetU32();
	this->field_0x428 = pByteCode->GetU32();

	this->field_0x390 = pByteCode->GetF32();
	this->field_0x394 = pByteCode->GetF32();

	this->field_0x398 = pByteCode->GetU32();
	this->field_0x39c = pByteCode->GetU32();

	this->pathFollowReader.Create(pByteCode);
	this->field_0x41c = pByteCode->GetS32();

	this->field_0x3a0 = S_ACTOR_STREAM_REF::Create(pByteCode);

	this->field_0x418 = pByteCode->GetF32();
	this->field_0x414 = pByteCode->GetU32();
	this->field_0x410 = pByteCode->GetU32();
	this->field_0x3a4.index = pByteCode->GetS32();

	this->vision.Create(this, pByteCode);

	return;
}

void CActorWoof::Init()
{
	S_ACTOR_STREAM_REF* pStreamRef;
	int iVar1;

	this->pathFollowReader.Init();

	ClearLocalData();

	CActorAutonomous::Init();

	this->field_0x3a4.Init();

	this->field_0x3a0->Init();

	this->field_0xf0 = 10.0f;
	this->pAnimationController->RegisterBone(this->field_0x428);

	return;
}

void CActorWoof::Reset()
{
	ClearLocalData();
	CActorAutonomous::Reset();

	return;
}

void CActorWoof::CheckpointReset()
{
	CLifeInterface* pCVar1;
	float fVar2;

	CActorAutonomous::CheckpointReset();

	if (0.0f < GetLifeInterface()->GetValue()) {
		CActor::PreReset();
		Reset();
	}

	return;
}
struct S_SAVE_CLASS_WOOF
{
	uint field_0x0;
};

void CActorWoof::SaveContext(void* pData, uint mode, uint maxSize)
{
	float fVar2;

	S_SAVE_CLASS_WOOF* pSaveData = reinterpret_cast<S_SAVE_CLASS_WOOF*>(pData);

	if (mode == 1) {
		fVar2 = GetLifeInterface()->GetValue();
		pSaveData->field_0x0 = static_cast<uint>(0.0f < fVar2);
	}

	return;
}

void CActorWoof::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_WOOF* pSaveData = reinterpret_cast<S_SAVE_CLASS_WOOF*>(pData);

	if ((mode == 1) && (pSaveData->field_0x0 == 0)) {
		LifeAnnihilate();

		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;

		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;

		SetBehaviour(WOOF_BEHAVIOUR_GUARD, 0xb, -1);
	}

	return;
}

CBehaviour* CActorWoof::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == WOOF_BEHAVIOUR_VERTICAL_JUMP) {
		pBehaviour = &this->behaviourWoofVerticalJump;
	}
	else {
		if (behaviourType == WOOF_BEHAVIOUR_GUARD) {
			pBehaviour = &this->behaviourWoofGuard;
		}
		else {
			pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

StateConfig CActorWoof::_gStateCfg_WOF[38] = {
	StateConfig(0x0C, 0x2104),
StateConfig(0x1C, 0x0904), StateConfig(0x1D, 0x0904), StateConfig(0x0C, 0x0904), StateConfig(0x0C, 0x0104),
StateConfig(0x21, 0x1101), StateConfig(0x1E, 0x1040), StateConfig(0x1F, 0x1040), StateConfig(0x20, 0x1140),
StateConfig(0x15, 0x11C4), StateConfig(0x1D, 0x1104), StateConfig(0x0C, 0x5104), StateConfig(0x0D, 0x09C4),
StateConfig(0x0D, 0x09C4), StateConfig(0x0D, 0x0944), StateConfig(0x0D, 0x0104), StateConfig(0x0E, 0x0104),
StateConfig(0x11, 0x01C4), StateConfig(0x0F, 0x1000), StateConfig(0x0F, 0x1000), StateConfig(0x1A, 0x10C0),
StateConfig(0x1B, 0x10C0), StateConfig(0x0E, 0x0104), StateConfig(0x12, 0x51C0), StateConfig(0x13, 0x50C0),
StateConfig(0x14, 0x50C0), StateConfig(0x15, 0x51C0), StateConfig(0x22, 0x00C4), StateConfig(0x16, 0x51C0),
StateConfig(0x17, 0x50C0), StateConfig(0x18, 0x50C0), StateConfig(0x19, 0x51C0), StateConfig(0x17, 0xD0C0),
StateConfig(0x13, 0x90C0), StateConfig(0x14, 0x90C0), StateConfig(0x15, 0x91C0), StateConfig(0x22, 0x91C0),
StateConfig(0x0E, 0xD1C0), };

StateConfig* CActorWoof::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_WOF + state + -6;
	}

	return pStateConfig;
}

bool CActorWoof::Can_0x9c()
{
	bool uVar1;

	if ((((this->actorFieldS & 0x60) == 0) || (this->actorState != 8)) || (uVar1 = true, this->field_0x430 == (CActorHero*)0x0)) {
		uVar1 = CActor::Can_0x9c();
	}

	return uVar1;
}

CVision* CActorWoof::GetVision()
{
	return &this->vision;
}

int CActorWoof::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;
	CLifeInterface* pCVar4;
	int uVar5;
	edF32VECTOR4* peVar6;
	float fVar7;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	if (msg == MESSAGE_IMPULSE) {
		BounceParams* pBounceParams = reinterpret_cast<BounceParams*>(pMsgParam);

		edF32Vector4ScaleHard(pBounceParams->field_0x10, &eStack32, &pBounceParams->field_0x0);
		edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack64, &eStack32);
		peVar6 = this->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(peVar6, peVar6, &eStack64);
		fVar7 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
		this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar7;
		this->lastGroundY = this->currentLocation.y;
		SetState(WOOF_STATE_FALL, -1);
		uVar5 = 1;
	}
	else {
		if (msg == MESSAGE_ENTER_TRAMPO) {
			this->field_0x42c = pSender;
			this->field_0x564 = reinterpret_cast<uint>(pMsgParam);
			SetState(0x19, -1);
			uVar5 = 1;
		}
		else {
			if (msg == 3) {
				if (pMsgParam == reinterpret_cast<void*>(1)) {
					SetBehaviour(WOOF_BEHAVIOUR_GUARD, 0xb, -1);
					uVar5 = 1;
				}
				else {
					SetBehaviour(WOOF_BEHAVIOUR_GUARD, 0xb, -1);
					uVar5 = 1;
				}
			}
			else {
				if (msg == 2) {
					_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
					if (pHitParam->projectileType == 10) {
						if ((GetStateFlags(this->actorState) & 0x8000) == 0) {
							LifeDecrease(pHitParam->damage);

							pCVar4 = GetLifeInterface();
							fVar7 = pCVar4->GetValue();
							if (fVar7 <= 0.0f) {
								edF32Vector4ScaleHard(-pHitParam->field_0x30, &local_10, &pHitParam->field_0x20);
								local_10.y = 0.0f;
								fVar7 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
								if (0.001f < fVar7) {
									this->rotationQuat = local_10;
								}

								this->dynamic.speed = 0.0f;
								this->dynamicExt.normalizedTranslation.x = 0.0f;
								this->dynamicExt.normalizedTranslation.y = 0.0f;
								this->dynamicExt.normalizedTranslation.z = 0.0f;
								this->dynamicExt.normalizedTranslation.w = 0.0f;
								this->dynamicExt.field_0x6c = 0.0f;
								UpdatePosition(&this->currentLocation, false);
								SetBehaviour(WOOF_BEHAVIOUR_GUARD, 0xb, -1);
							}
							else {
								edF32Vector4ScaleHard(pHitParam->field_0x30, &local_10, &pHitParam->field_0x20);
								edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack48, &local_10);
								peVar6 = this->dynamicExt.aImpulseVelocities;
								edF32Vector4AddHard(peVar6, peVar6, &eStack48);
								fVar7 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
								this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar7;
								local_10.y = 0.0f;
								fVar7 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
								if (0.001f < fVar7) {
									this->rotationQuat = local_10;
								}

								this->field_0x530 = 1;
								this->field_0x430 = CActorHero::_gThis;
								this->dynamic.speed = 0.0f;
								SetBehaviour(WOOF_BEHAVIOUR_GUARD, 0x1a, -1);
							}

							return 1;
						}
					}

					uVar5 = 0;
				}
				else {
					if (msg == 0x4d) {
						uVar5 = 1;
						if (pMsgParam == reinterpret_cast<void*>(0x8)) {
							uVar5 = this->field_0x428;
						}
					}
					else {
						uVar5 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
					}
				}
			}
		}
	}

	return uVar5;
}

int CActorWoof::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	int result;

	if (*param_5 == 1) {
		SetBehaviour(WOOF_BEHAVIOUR_GUARD, 0xb, -1);
		result = 1;
	}
	else {
		result = InterpretEvent(pEventMessage, static_cast<uint>(param_3), param_4, param_5);
	}

	return result;
}

void CActorWoof::StoreCollisionSphere()
{
	edF32VECTOR4 scaleToStore;
	edF32VECTOR4 positionToStore;

	positionToStore.y = 0.36f;
	positionToStore.z = 0.2f;
	positionToStore.x = 0.0f;
	positionToStore.w = 1.0f;

	scaleToStore.w = 0.0f;
	scaleToStore.x = 0.39f;
	scaleToStore.y = 0.35f;
	scaleToStore.z = 0.56f;

	ChangeCollisionSphere(0.0f, &scaleToStore, &positionToStore);
	CActorAutonomous::StoreCollisionSphere();

	return;
}

CPathFinderClient* CActorWoof::GetPathfinderClient()
{
	return &this->pathFinderClient;
}

CPathFinderClient* CActorWoof::GetPathfinderClientAlt()
{
	return &this->pathFinderClient;
}

void CActorWoof::ClearLocalData()
{
	this->vision.location = this->currentLocation;
	this->vision.rotationQuat = this->rotationQuat;

	this->field_0x430 = (CActorHero*)0x0;
	this->field_0x42c = (CActor*)0x0;
	this->field_0x520 = this->rotationQuat;
	this->field_0x570 = this->baseLocation;
	this->lastGroundY = this->baseLocation.y;
	this->field_0x530 = 0;
	this->field_0x554 = 0;
	this->field_0x558 = 0;
	this->field_0x55c = 0;
	this->field_0x560 = this->field_0x410;
	this->field_0x5a0.Reset();

	if ((this->pathFollowReader).pPathFollow != (CPathFollow*)0x0) {
		this->pathFollowReader.Reset();
	}
	this->field_0x564 = 0;

	return;
}

void CActorWoof::UpdateVision()
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	local_10 = this->rotationQuat;

	if (this->field_0x430 != (CActorHero*)0x0) {
		if ((GetStateFlags(this->actorState) & 0x800) != 0) {
			edF32Vector4SubHard(&eStack32, &this->field_0x430->currentLocation, &this->currentLocation);
			edF32Vector4NormalizeHard(&eStack32, &eStack32);
			local_10.y = eStack32.y;
		}
	}

	if (local_10.y <= -0.26f) {
		local_10.y = -0.26f;
	}

	if (0.26f <= local_10.y) {
		local_10.y = 0.26f;
	}

	SV_GetBoneWorldPosition(this->field_0x428, &local_30);
	local_30.y = local_30.y - 0.3f;
	GetVision()->location = local_30;
	edF32Vector4NormalizeHard(&local_10, &local_10);
	GetVision()->rotationQuat = local_10;

	return;
}

bool CActorWoof::DetectWithVision(edF32VECTOR4* param_2, edF32VECTOR4* param_3, bool param_4)
{
	bool bVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	if ((this->field_0x430 == CActorHero::_gThis) && (bVar1 = CActorHero::_gThis->TestState_IsInCheatMode(), bVar1 != false)) {
		bVar1 = false;
	}
	else {
		if (param_4 == false) {
			SV_GetBoneWorldPosition(this->field_0x420, &local_10);
		}
		else {
			SV_GetBoneWorldPosition(this->field_0x424, &local_10);
		}

		this->field_0x460.rotationQuat = *param_3;
		this->field_0x460.location = local_10;

		bVar1 = this->field_0x460.SV_PointIsInVision(param_2);
		if (bVar1 == false) {
			if (param_4 == false) {
				this->field_0x430->SV_GetBoneWorldPosition(0x4c81be8d, &eStack32);
				this->field_0x430->SV_GetBoneWorldPosition(0x4c81be90, &eStack48);
				this->field_0x430->SV_GetBoneWorldPosition(0x45544554, &eStack64);

				if (((this->field_0x430->pCollisionData)->flags_0x4 & 2) == 0) {
					bVar1 = this->field_0x460.SV_PointIsInVision(&eStack48);
					if (bVar1 != false) {
						return true;
					}

					bVar1 = this->field_0x460.SV_PointIsInVision(&eStack32);
					if (bVar1 != false) {
						return true;
					}
				}

				bVar1 = this->field_0x460.SV_PointIsInVision(&eStack64);
				if (bVar1 != false) {
					return true;
				}
			}

			if ((((this->field_0x430->pCollisionData)->flags_0x4 & 2) == 0) &&
				(bVar1 = this->field_0x460.SV_PointIsInVision(&this->field_0x430->currentLocation), bVar1 != false)) {
				bVar1 = true;
			}
			else {
				bVar1 = false;
			}
		}
		else {
			bVar1 = true;
		}
	}

	return bVar1;
}

int CActorWoof::CheckArea()
{
	CActorMovable* pCVar1;
	float fVar2;
	float fVar3;
	CEventManager* pCVar4;
	ed_zone_3d* pZone;
	ed_zone_3d* pZone_00;
	int iVar5;
	int iVar6;
	StateConfig* pSVar7;
	uint uVar8;
	CVision* this_00;
	CActor* pCVar9;
	int iVar10;
	float fVar11;
	float fVar12;

	pCVar4 = CScene::ptable.g_EventManager_006f5080;
	pZone_00 = (ed_zone_3d*)0x0;

	uVar8 = this->field_0x398;
	if (uVar8 == 0xffffffff) {
		this->field_0x430 = (CActorHero*)0x0;
		iVar5 = 2;
	}
	else {
		pZone = (ed_zone_3d*)0x0;
		if (uVar8 != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, uVar8);
		}

		uVar8 = this->field_0x39c;
		if ((uVar8 != 0xffffffff) && (pZone_00 = (ed_zone_3d*)0x0, uVar8 != 0xffffffff)) {
			pZone_00 = edEventGetChunkZone(pCVar4->activeChunkId, uVar8);
		}

		pCVar1 = CActorHero::_gThis;
		if (((this->field_0x560 & 2) == 0) && ((this->field_0x560 & 1) == 0)) {
			if (pZone_00 == (ed_zone_3d*)0x0) {
				iVar5 = 2;
			}
			else {
				iVar5 = edEventComputeZoneAgainstVertex(pCVar4->activeChunkId, pZone_00, &CActorHero::_gThis->currentLocation, 0);
			}

			if (pZone == (ed_zone_3d*)0x0) {
				iVar6 = 2;
			}
			else {
				iVar6 = edEventComputeZoneAgainstVertex(pCVar4->activeChunkId, pZone, &pCVar1->currentLocation, 0);
			}

			this->field_0x430 = (CActorHero*)0x0;
			fVar11 = 3.402823e+38f;

			if (((pCVar1->GetStateFlags(pCVar1->actorState) & 1) == 0) && (iVar6 != 2)) {
				if (this->field_0x530 == 0) {
					if (iVar5 == 1) {
						fVar11 = pCVar1->currentLocation.x - this->currentLocation.x;
						fVar12 = pCVar1->currentLocation.y - this->currentLocation.y;
						fVar2 = pCVar1->currentLocation.z - this->currentLocation.z;
						fVar11 = sqrtf(fVar11 * fVar11 + fVar12 * fVar12 + fVar2 * fVar2);
						this->field_0x430 = pCVar1;
					}
					else {
						if ((this->field_0x410 & 8) != 0) {
							pCVar9 = GetVision()->ScanForTarget(pCVar1, 1);
							if (pCVar9 != (CActor*)0x0) {
								fVar11 = pCVar1->currentLocation.x - this->currentLocation.x;
								fVar12 = pCVar1->currentLocation.y - this->currentLocation.y;
								fVar2 = pCVar1->currentLocation.z - this->currentLocation.z;
								this->field_0x430 = pCVar1;
								fVar11 = sqrtf(fVar11 * fVar11 + fVar12 * fVar12 + fVar2 * fVar2);
							}
						}
					}
				}
				else {
					if (iVar6 == 1) {
						fVar11 = pCVar1->currentLocation.x - this->currentLocation.x;
						fVar12 = pCVar1->currentLocation.y - this->currentLocation.y;
						fVar2 = pCVar1->currentLocation.z - this->currentLocation.z;
						fVar11 = sqrtf(fVar11 * fVar11 + fVar12 * fVar12 + fVar2 * fVar2);
						this->field_0x430 = pCVar1;
					}
				}
			}
			iVar5 = 0;
			while (true) {
				if (this->field_0x3a0 == (S_ACTOR_STREAM_REF*)0x0) {
					iVar6 = 0;
				}
				else {
					iVar6 = this->field_0x3a0->entryCount;
				}

				if (iVar6 <= iVar5) break;

				if (this->field_0x3a0->aEntries[iVar5].Get() != (CActor*)0x0) {
					pCVar1 = static_cast<CActorMovable*>(this->field_0x3a0->aEntries[iVar5].Get());
					fVar12 = pCVar1->currentLocation.x - this->currentLocation.x;
					fVar2 = pCVar1->currentLocation.y - this->currentLocation.y;
					fVar3 = pCVar1->currentLocation.z - this->currentLocation.z;
					fVar12 = sqrtf(fVar12 * fVar12 + fVar2 * fVar2 + fVar3 * fVar3);
					iVar6 = edEventComputeZoneAgainstVertex(pCVar4->activeChunkId, pZone_00, &pCVar1->currentLocation, 0);
					iVar10 = edEventComputeZoneAgainstVertex(pCVar4->activeChunkId, pZone, &pCVar1->currentLocation, 0);
					if (((pCVar1->flags & 4) != 0) && (fVar12 < fVar11)) {
						if (this->field_0x530 == 0) {
							if (iVar6 == 1) {
								this->field_0x430 = pCVar1;
								fVar11 = fVar12;
							}
						}
						else {
							if (iVar10 == 1) {
								this->field_0x430 = pCVar1;
								fVar11 = fVar12;
							}
						}
					}
				}

				iVar5 = iVar5 + 1;
			}

			if (this->field_0x430 == (CActorHero*)0x0) {
				this->field_0x530 = 0;
				iVar5 = 2;
			}
			else {
				iVar5 = 1;
				this->field_0x530 = 1;
			}
		}
		else {
			this->field_0x430 = (CActorHero*)0x0;
			iVar5 = 2;
		}
	}

	return iVar5;
}

void CActorWoof::BehaviourWoofGuard_InitState(int newState)
{
	CPathFinderClient* pClient;
	float fVar1;
	float fVar2;
	float fVar3;
	_vision_param local_e0;
	_vision_param local_c0;
	edF32VECTOR4 local_a0;
	_msg_hit_param msgHitParams;
	CCollision* pCol;

	if (newState == 0x2a) {
		this->dynamic.speed = 0.0f;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
	}
	else {
		if (newState == 0x2b) {
			this->pAnimationController->RegisterBone(this->field_0x420);
			this->field_0x430->pAnimationController->RegisterBone(0x45544554);
			this->field_0x430->pAnimationController->RegisterBone(0x4c81be8d);
			this->field_0x430->pAnimationController->RegisterBone(0x4c81be90);
			this->dynamic.speed = 0.0f;
			local_e0.field_0x0 = 30.0f;
			local_e0.field_0x4 = 1.3f;
			local_e0.field_0x8 = 0.5f;
			local_e0.field_0xc = 1.0f;
			local_e0.field_0x10 = 1;
			local_e0.pActor_0x14 = (CActor*)this;
			this->field_0x460.Create(&local_e0);
			(this->pCollisionData)->actorFieldA = (CActor*)this->field_0x430;
			(this->field_0x430->pCollisionData)->actorFieldA = (CActor*)this;
		}
		else {
			if (((newState == WOOF_STATE_ATTACK_HERO_4_4) || (newState == WOOF_STATE_ATTACK_HERO_3_4)) || (newState == WOOF_STATE_ATTACK_HERO_2_4)) {
				this->pAnimationController->RegisterBone(this->field_0x420);
				this->field_0x430->pAnimationController->RegisterBone(0x4c81be8d);
				this->field_0x430->pAnimationController->RegisterBone(0x4c81be90);
				this->field_0x430->pAnimationController->RegisterBone(0x45544554);
			}
			else {
				if (newState == 0x1d) {
					local_c0.field_0x0 = 30.0f;
					local_c0.field_0x4 = 1.0f;
					local_c0.field_0x8 = 0.80000001f;
					local_c0.field_0xc = 1.2f;
					local_c0.field_0x10 = 1;
					local_c0.pActor_0x14 = (CActor*)this;
					this->field_0x460.Create(&local_c0);
					this->pAnimationController->RegisterBone(this->field_0x420);
					this->field_0x430->pAnimationController->RegisterBone(0x4c81be8d);
					this->field_0x430->pAnimationController->RegisterBone(0x4c81be90);
					this->field_0x430->pAnimationController->RegisterBone(0x45544554);
					(this->field_0x460).flags = (this->field_0x460).flags ^ 0x40000000;
					this->field_0x430->pAnimationController->RegisterBone(0x544e8aa8);
				}
				else {
					if (newState == WOOF_STATE_PATH) {
						pClient = GetPathfinderClient();
						pClient->Init();
						pClient->ChangePathfindingId(this, this->field_0x41c, &this->currentLocation);
						this->field_0x410 = this->field_0x410 | 4;
					}
					else {
						if (newState == 0xc) {
							pCol = this->pCollisionData;
							local_a0.xyz = pCol->aCollisionContact[0].location.xyz * -1.0f;
							local_a0.y = 0.0f;
							edF32Vector4NormalizeHard(&local_a0, &local_a0);
							this->rotationQuat = local_a0;
						}
						else {
							if (newState != WOOF_STATE_FALL) {
								if (newState == WOOF_STATE_CHASE) {
									pClient = GetPathfinderClient();
									pClient->Init();
									pClient->ChangePathfindingId(this, this->field_0x41c, &this->currentLocation);
								}
								else {
									if (newState != WOOF_STATE_WAY_POINT) {
										if (newState == WOOF_STATE_COME_BACK) {
											pClient = GetPathfinderClient();
											pClient->Init();
											pClient->ChangePathfindingId(this, this->field_0x41c, &this->currentLocation);
											fVar3 = this->currentLocation.y;
											fVar1 = this->currentLocation.z;
											fVar2 = this->currentLocation.w;
											(this->field_0x570).x = this->currentLocation.x;
											(this->field_0x570).y = fVar3;
											(this->field_0x570).z = fVar1;
											(this->field_0x570).w = fVar2;
										}
										else {
											if (newState == WOOF_STATE_CHASE_INTRUDER_LOST) {
												pClient = GetPathfinderClient();
												pClient->Init();
												pClient->ChangePathfindingId(this, this->field_0x41c, &this->currentLocation);
												fVar3 = this->currentLocation.y;
												fVar1 = this->currentLocation.z;
												fVar2 = this->currentLocation.w;
												this->field_0x590 = this->currentLocation;
											}
											else {
												if (newState == 0xb) {
													LifeAnnihilate();
												}
												else {
													if (newState == 10) {
														this->dynamic.speed = 0.0f;
														this->dynamicExt.normalizedTranslation.x = 0.0f;
														this->dynamicExt.normalizedTranslation.y = 0.0f;
														this->dynamicExt.normalizedTranslation.z = 0.0f;
														this->dynamicExt.normalizedTranslation.w = 0.0f;
														this->dynamicExt.field_0x6c = 0.0f;
													}
													else {
														if (newState == WOOF_STATE_YAP) {
															this->field_0x558 = 0;
														}
														else {
															if (newState == WOOF_STATE_IDLE) {
																this->dynamic.speed = 0.0f;
																this->dynamicExt.normalizedTranslation.x = 0.0f;
																this->dynamicExt.normalizedTranslation.y = 0.0f;
																this->dynamicExt.normalizedTranslation.z = 0.0f;
																this->dynamicExt.normalizedTranslation.w = 0.0f;
																this->dynamicExt.field_0x6c = 0.0f;
															}
															else {
																if (newState == 0x11) {
																	pCol = this->pCollisionData;
																	pCol->flags_0x0 = pCol->flags_0x0 | 0x8000;
																	if ((this->field_0x410 & 1) != 0) {
																		this->field_0x560 = this->field_0x560 | 1;
																	}

																	this->dynamic.speed = 0.0f;
																	this->dynamicExt.normalizedTranslation.x = 0.0f;
																	this->dynamicExt.normalizedTranslation.y = 0.0f;
																	this->dynamicExt.normalizedTranslation.z = 0.0f;
																	this->dynamicExt.normalizedTranslation.w = 0.0f;
																	this->dynamicExt.field_0x6c = 0.0f;
																}
																else {
																	if ((newState == 0x27) || (newState == 0x28)) {
																		(this->pCollisionData)->actorFieldA = this->field_0x430;
																		(this->field_0x430->pCollisionData)->actorFieldA = this;
																	}
																	else {
																		if (newState == 0x26) {
																			msgHitParams.damage = 10.0f;
																			msgHitParams.projectileType = 0;
																			msgHitParams.flags = 0;
																			DoMessage(this->field_0x430, MESSAGE_KICKED, &msgHitParams);
																		}
																		else {
																			if (newState == 0x19) {
																				this->field_0x42c->pAnimationController->RegisterBone(this->field_0x564);
																				this->dynamicExt.normalizedTranslation.x = 0.0f;
																				this->dynamicExt.normalizedTranslation.y = 0.0f;
																				this->dynamicExt.normalizedTranslation.z = 0.0f;
																				this->dynamicExt.normalizedTranslation.w = 0.0f;
																				this->dynamicExt.field_0x6c = 0.0f;
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CActorWoof::BehaviourWoofGuard_Manage()
{
	byte bVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	CActor* pCVar4;
	CCollision* pCVar5;
	bool bVar6;
	Timer* pTVar7;
	uint uVar8;
	CVision* pVision;
	int iVar9;
	StateConfig* pSVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	edF32VECTOR4 eStack160;
	CActorMovParamsIn local_90;
	CActorMovParamsOut CStack112;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	undefined4 local_8;
	undefined4 local_4;

	UpdateVision();

	switch (this->actorState) {
	case WOOF_STATE_IDLE:
		SV_UpdateOrientation(4.0f, &this->field_0x520);
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		this->field_0x570 = this->baseLocation;
		if (((this->field_0x560 & 1) == 0) && (iVar9 = CheckArea(), iVar9 == 1)) {
			SetState(8, -1);
		}
		break;
	case WOOF_STATE_YAP:
		StateWoofYap();
		break;
	case 8:
		pCVar4 = this->field_0x430;
		local_20.x = pCVar4->currentLocation.x - this->currentLocation.x;
		local_20.z = pCVar4->currentLocation.z - this->currentLocation.z;
		local_20.w = pCVar4->currentLocation.w - this->currentLocation.w;
		local_20.y = 0.0f;
		edF32Vector4NormalizeHard(&local_20, &local_20);
		SV_UpdateOrientation(4.0f, &local_20);
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if ((this->field_0x560 & 1) == 0) {
				SetState(WOOF_STATE_CHASE_INTRUDER_LOST, -1);
			}
			else {
				SetState(WOOF_STATE_IDLE, -1);
			}
		}
		break;
	case 10:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		iVar9 = CheckArea();
		if (iVar9 == 1) {
			SetState(8, -1);
		}
		else {
			fVar11 = this->timeInAir;
			fVar13 = this->pathFollowReader.GetDelay();
			if (fVar13 < fVar11) {
				bVar6 = this->pathFollowReader.AtGoal((this->pathFollowReader).splinePointIndex, (this->pathFollowReader).field_0xc);
				if (bVar6 == false) {
					this->pathFollowReader.NextWayPoint();
					SetState(WOOF_STATE_PATH, -1);
				}
				else {
					SetState(WOOF_STATE_IDLE, -1);
				}
			}
		}
		break;
	case 0xb:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
		}
		break;
	case 0xc:
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(0xd, -1);
		}
		break;
	case 0xd:
		ManageDyn(4.0f, 0x40129, (CActorsTable*)0x0);
		if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
			if (5.0f < this->lastGroundY - this->currentLocation.y) {
				SetState(WOOF_STATE_FALL, -1);
			}
		}
		else {
			SetState(0xe, -1);
		}
		break;
	case 0xe:
		ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if (this->field_0x430 == (CActorHero*)0x0) {
				SetState(0x10, -1);
			}
			else {
				SetState(8, -1);
			}
		}
		break;
	case 0xf:
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(0x10, -1);
		}
		break;
	case 0x10:
		if (this->field_0x430 != (CActorHero*)0x0) {
			pCVar4 = this->field_0x430;
			local_30.x = pCVar4->currentLocation.x - this->currentLocation.x;
			local_30.z = pCVar4->currentLocation.z - this->currentLocation.z;
			local_30.w = pCVar4->currentLocation.w - this->currentLocation.w;
			local_30.y = 0.0f;
			edF32Vector4NormalizeHard(&local_30, &local_30);
			SV_UpdateOrientation(4.0f, &local_30);
		}
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if (this->field_0x430 == (CActorHero*)0x0) {
				SetState(WOOF_STATE_COME_BACK, -1);
			}
			else {
				SetState(WOOF_STATE_CHASE_INTRUDER_LOST, -1);
			}
		}
		break;
	case 0x11:
		IMPLEMENTATION_GUARD(
		StateWoofSleep(this);)
		break;
	case WOOF_STATE_WAY_POINT:
		IMPLEMENTATION_GUARD(
		StateWoofWayPoint(this);)
		break;
	case WOOF_STATE_CHASE:
		StateWoofChase();
		break;
	case WOOF_STATE_CHASE_INTRUDER_LOST:
		StateWoofChaseIntruderLost();
		break;
	case WOOF_STATE_COME_BACK:
		StateWoofComeBack();
		break;
	case WOOF_STATE_PATH:
		IMPLEMENTATION_GUARD(
		StateWoofPath(this);)
		break;
	case WOOF_STATE_SLIDE:
		StateWoofSlide();
		break;
	case WOOF_STATE_FALL:
		StateWoofFall();
		break;
	case 0x19:
		this->field_0x42c->SV_GetBoneWorldPosition(this->field_0x564, &local_40);
		SV_MOV_MoveCloserTo(0x40000000, &local_40);
		local_40.x = this->currentLocation.x;
		local_40.z = this->currentLocation.z;
		UpdatePosition(&local_40, true);
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
		break;
	case 0x1a:
		ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);
		pCVar5 = this->pCollisionData;
		if ((pCVar5->flags_0x4 & 1) == 0) {
		LAB_003340e8:
			bVar6 = false;
		}
		else {
			fVar13 = edF32Vector4DotProductHard(&pCVar5->aCollisionContact->location, &this->dynamic.rotationQuat);
			bVar6 = true;
			if (-0.3f <= fVar13) {
				fVar13 = edF32Vector4DotProductHard(&this->pCollisionData->aCollisionContact->location, &this->dynamic.velocityDirectionEuler);
				bVar6 = true;

				if (-0.3f <= fVar13) goto LAB_003340e8;
			}
		}
		if (bVar6) {
			SetState(0xc, -1);
		}
		else {
			bVar1 = (this->pCollisionData)->flags_0x4;
			if ((bVar1 & 4) == 0) {
				if ((bVar1 & 2) == 0) {
					if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
						SetState(0x1b, -1);
					}
				}
				else {
					SetState(WOOF_STATE_CHASE_INTRUDER_LOST, -1);
				}
			}
			else {
				SetState(0x1b, -1);
			}
		}
		break;
	case 0x1b:
		ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);
		pCVar5 = this->pCollisionData;
		fVar13 = this->dynamic.linearAcceleration;
		fVar11 = this->dynamic.velocityDirectionEuler.y;
		if ((pCVar5->flags_0x4 & 1) == 0) {
		LAB_003342a0:
			bVar6 = false;
		}
		else {
			fVar12 = edF32Vector4DotProductHard(&pCVar5->aCollisionContact->location, &this->dynamic.rotationQuat);
			bVar6 = true;
			if (-0.3f <= fVar12) {
				fVar12 = edF32Vector4DotProductHard(&this->pCollisionData->aCollisionContact->location, &this->dynamic.velocityDirectionEuler);
				bVar6 = true;

				if (-0.3f <= fVar12) goto LAB_003342a0;
			}
		}
		if (bVar6) {
			SetState(0xc, -1);
		}
		else {
			if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
				if (fVar13 * fVar11 < 0.0f) {
					if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
						this->lastGroundY = this->currentLocation.y;
						SetState(WOOF_STATE_FALL, -1);
					}
				}
			}
			else {
				SetState(WOOF_STATE_CHASE_INTRUDER_LOST, -1);
			}
		}
		break;
	case 0x1c:
		CStack112.flags = 0;
		local_90.pRotation = (edF32VECTOR4*)0x0;
		local_90.rotSpeed = 10.0f;
		local_90.speed = this->field_0x394;
		local_90.flags = 0x452;
		local_90.acceleration = 10.0f;
		SV_AUT_MoveTo(&CStack112, &local_90, &this->baseLocation);
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		if (CStack112.moveVelocity < 0.25f) {
			this->field_0x560 = this->field_0x410;
			SetState(WOOF_STATE_IDLE, -1);
		}
		break;
	case 0x1d:
		ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);
		this->field_0x430->SV_GetBoneWorldPosition(0x544e8aa8, &eStack160);
		bVar6 = DetectWithVision(&eStack160, &this->rotationQuat, 0);
		if (bVar6 != false) {
			DoMessage(this->field_0x430, (ACTOR_MESSAGE)3, (MSG_PARAM)3);
			SetState(0x27, -1);
		}
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			fVar13 = edF32Vector4DotProductHard(&this->dynamic.velocityDirectionEuler, &this->field_0x430->dynamic.velocityDirectionEuler);
			if (((fVar13 <= -1.0f) || (-0.9f <= fVar13)) || (bVar6 = true, this->field_0x430->dynamic.linearAcceleration <= 3.0f)) {
				bVar6 = false;
			}

			if (bVar6) {
				this->dynamic.speed = 4.0f;
			}
			else {
				this->dynamic.speed = this->dynamic.speed + 4.0f;
			}

			SetState(WOOF_STATE_ATTACK_HERO_2_4, -1);
		}
		break;
	case WOOF_STATE_ATTACK_HERO_2_4:
		StateWoofAttackHero_2_4();
		break;
	case WOOF_STATE_ATTACK_HERO_3_4:
		StateWoofAttackHero_3_4();
		break;
	case WOOF_STATE_ATTACK_HERO_4_4:
		ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if (GetVision()->SV_GetNearestActor(this->field_0x430->typeID, 1) == 0) {
				SetState(WOOF_STATE_SLIDE, -1);
			}
			else {
				SetState(WOOF_STATE_CHASE, -1);
			}
		}
		break;
	case 0x21:
		this->dynamic.speed = 0.0f;

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		iVar9 = DoMessage(this->field_0x430, (ACTOR_MESSAGE)3, 0);
		if (iVar9 != 0) {
			this->field_0x430 = (CActorHero*)0x0;
		}

		if (1.0f < this->timeInAir) {
			SetState(WOOF_STATE_COME_BACK, -1);
		}
		break;
	case 0x24:
		ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);
		if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
			if (5.0f < this->lastGroundY - this->currentLocation.y) {
				SetBehaviour(WOOF_BEHAVIOUR_GUARD, WOOF_STATE_FALL, -1);
			}
		}
		else {
			SetState(0x25, -1);
		}
		break;
	case 0x25:
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if ((uint)this->field_0x554 < 3) {
				SetBehaviour(4, 9, -1);
			}
			else {
				SetBehaviour(WOOF_BEHAVIOUR_GUARD, WOOF_STATE_YAP, -1);
			}
		}
		break;
	case 0x26:
		pCVar4 = this->field_0x430;
		local_50 = pCVar4->currentLocation;

		this->dynamic.speed = 0.0f;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;

		local_50.y = local_50.y - 1.4f;
		UpdatePosition(&local_50, true);
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (((pCVar4->GetStateFlags(pCVar4->actorState) & 1) == 0) && (this->timeInAir <= 0.2f)) {
			if (CActorHero::_gThis == this->field_0x430) {
				uVar8 = CActorHero::_gThis->FUN_00132f00(0xffffffff);
				bVar6 = true;
				if (uVar8 == 0) {
				LAB_00333d30:
					bVar6 = false;
				}
			}
			else {
				if (((this->field_0x430->pCollisionData)->flags_0x4 & 2) != 0) goto LAB_00333d30;

				bVar6 = true;
			}

			if (bVar6) break;
		}

		this->lastGroundY = this->currentLocation.y;
		SetState(0x24, -1);
		break;
	case 0x27:
		bVar6 = this->field_0x5a0.IsFinished();
		if (bVar6 == false) {
			pTVar7 = GetTimer();
			this->field_0x5a0.Integrate(pTVar7->cutsceneDeltaTime);
			this->dynamicExt.instanceIndex.y = (this->field_0x5a0).field_0x20;
			this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x8000;
			ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);
		}
		else {
			ManageDyn(4.0f, 0x40129, (CActorsTable*)0x0);
			SetState(0x28, -1);
		}
		break;
	case 0x28:
		if (10.0f < this->dynamic.speed) {
			this->dynamic.speed = 10.0f;
		}

		ManageDyn(4.0f, 0x140129, (CActorsTable*)0x0);

		if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
			pCVar4 = this->field_0x430;
			iVar9 = pCVar4->actorState;
			uVar8 = 0;
			if (iVar9 != -1) {
				pSVar10 = pCVar4->GetStateCfg(iVar9);
				uVar8 = pSVar10->flags_0x4;
			}
			if ((uVar8 & 1) != 0) break;
		}

		SetState(0x29, -1);
		break;
	case 0x29:
		SV_MOV_DecreaseSpeedIntensity(20.0f);
		ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(0x2a, -1);
		}
		break;
	case 0x2a:
		ManageDyn(4.0f, 0x1006023b, (CActorsTable*)0x0);

		pCVar4 = this->field_0x430;
		if (pCVar4 != (CActorHero*)0x0) {
			iVar9 = pCVar4->actorState;
			uVar8 = 0;
			if (iVar9 != -1) {
				pSVar10 = pCVar4->GetStateCfg(iVar9);
				uVar8 = pSVar10->flags_0x4;
			}

			if ((uVar8 & 1) == 0) {
				SetState(WOOF_STATE_COME_BACK, -1);
			}
		}
		break;
	case 0x2b:
		IMPLEMENTATION_GUARD(
		ActorFunc_003301c0();)
	}
	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
		iVar9 = this->actorState;
		uVar8 = 0;
		if (iVar9 != -1) {
			pSVar10 = GetStateCfg(iVar9);
			uVar8 = pSVar10->flags_0x4;
		}

		if (((uVar8 & 0x1000) == 0) && (bVar6 = true, 2.8f < this->distanceToGround)) goto LAB_003349b0;
	}
	bVar6 = false;
LAB_003349b0:
	if (bVar6) {
		this->lastGroundY = this->currentLocation.y;
		SetState(WOOF_STATE_FALL, -1);
	}

	return;
}

void CActorWoof::BehaviourWoofGuard_TermState(int oldState)
{
	float fVar1;
	float fVar2;
	float fVar3;
	CCollision* pCol;

	if (oldState == 0x2b) {
		this->pAnimationController->UnRegisterBone(this->field_0x420);
		this->field_0x430->pAnimationController->UnRegisterBone(0x45544554);
		this->field_0x430->pAnimationController->UnRegisterBone(0x4c81be8d);
		this->field_0x430->pAnimationController->UnRegisterBone(0x4c81be90);
		(this->pCollisionData)->actorFieldA = (CActor*)0x0;
		(this->field_0x430->pCollisionData)->actorFieldA = (CActor*)0x0;
	}
	else {
		if (oldState == 0x25) {
			this->pAnimationController->UnRegisterBone(this->field_0x424);
		}
		else {
			if (oldState == 0x24) {
				(this->pCollisionData)->actorFieldA = (CActor*)0x0;
				(this->field_0x430->pCollisionData)->actorFieldA = (CActor*)0x0;
				RestoreCollisionSphere(0.2f);
			}
			else {
				if (oldState != 0x26) {
					if (oldState == WOOF_STATE_ATTACK_HERO_4_4) {
						this->dynamic.speed = this->field_0x390;
						this->pAnimationController->UnRegisterBone(this->field_0x420);
						this->field_0x430->pAnimationController->UnRegisterBone(0x4c81be8d);
						this->field_0x430->pAnimationController->UnRegisterBone(0x4c81be90);
						this->field_0x430->pAnimationController->UnRegisterBone(0x45544554);
					}
					else {
						if (((oldState == WOOF_STATE_ATTACK_HERO_3_4) || (oldState == WOOF_STATE_ATTACK_HERO_2_4)) || (oldState == 0x1d)) {
							this->pAnimationController->UnRegisterBone(this->field_0x420);
							this->field_0x430->pAnimationController->UnRegisterBone(0x4c81be8d);
							this->field_0x430->pAnimationController->UnRegisterBone(0x4c81be90);
							this->field_0x430->pAnimationController->UnRegisterBone(0x45544554);
						}
						else {
							if (oldState == WOOF_STATE_CHASE_INTRUDER_LOST) {
								SV_AUT_PathfindingEnd();
							}
							else {
								if (oldState == WOOF_STATE_CHASE) {
									SV_AUT_PathfindingEnd();
								}
								else {
									if (oldState == WOOF_STATE_WAY_POINT) {
										pCol = this->pCollisionData;
										pCol->flags_0x0 = pCol->flags_0x0 | 1;
									}
									else {
										if (oldState == WOOF_STATE_COME_BACK) {
											SV_AUT_PathfindingEnd();
										}
										else {
											if (oldState == WOOF_STATE_PATH) {
												SV_AUT_PathfindingEnd();
												this->field_0x440 = this->currentLocation;
											}
											else {
												if (oldState != WOOF_STATE_FALL) {
													if (oldState == 0x11) {
														this->field_0x560 = this->field_0x560 & 0xfffffffd;
														pCol = this->pCollisionData;
														pCol->flags_0x0 = pCol->flags_0x0 & 0xffff7fff;
													}
													else {
														if (oldState == 0xd) {
															this->lastGroundY = this->currentLocation.y;
														}
														else {
															if (oldState == 0x19) {
																this->dynamic.speed = 0.0f;
																this->field_0x42c->pAnimationController->UnRegisterBone(this->field_0x564);
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return;
}

void CActorWoof::BehaviourWoofVerticalJump_InitState(int newState)
{
	CPathFinderClient* pPathFinderClient;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	_vision_param visionParam;

	if (newState == 0x23) {
		(this->pCollisionData)->actorFieldA = this->field_0x430;
		(this->field_0x430->pCollisionData)->actorFieldA = static_cast<CActor*>(this);
		local_30.y = 0.8f;
		local_30.w = 1.0f;
		local_40.y = 0.68f;
		local_30.x = 0.0f;
		local_30.z = 0.0f;
		local_40.x = 0.33f;
		local_40.z = 0.33f;
		local_40.w = 0.0f;
		this->ChangeCollisionSphere(0.0f, &local_40, &local_30);
	}
	else {
		if (newState == 0x22) {
			visionParam.field_0x4 = 0.55f;
			visionParam.field_0x0 = 10.0f;
			visionParam.field_0x8 = 0.6f;
			visionParam.field_0xc = 1.0f;
			visionParam.field_0x10 = 1;
			visionParam.pActor_0x14 = this;
			this->field_0x460.Create(&visionParam);
			(this->field_0x460).flags = (this->field_0x460).flags ^ 0x40000000;
			this->pAnimationController->RegisterBone(this->field_0x424);
		}
		else {
			if (newState == 9) {
				pPathFinderClient = this->GetPathfinderClient();
				pPathFinderClient->Init();
				pPathFinderClient->ChangePathfindingId(this, this->field_0x41c, &this->currentLocation);
				this->dynamic.speed = 0.0f;
			}
		}
	}

	return;
}

void CActorWoof::BehaviourWoofVerticalJump_Manage()
{
	IMPLEMENTATION_GUARD();
}

void CActorWoof::BehaviourWoofVerticalJump_TermState(int oldState)
{
	CActorWoof* pWoof;

	if (oldState == 0x25) {
		this->pAnimationController->UnRegisterBone(this->field_0x424);
	}
	else {
		if (oldState == 0x24) {
			(this->pCollisionData)->actorFieldA = (CActor*)0x0;
			(this->field_0x430->pCollisionData)->actorFieldA = (CActor*)0x0;
			this->RestoreCollisionSphere(0.2f);
		}
		else {
			if (oldState == 9) {
				this->SV_AUT_PathfindingEnd();
			}
		}
	}

	return;
}

void CActorWoof::StateWoofYap()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	int iVar4;
	CVision* pVision;
	CActor* pCVar5;
	CPathFinderClient* pCVar6;
	edF32VECTOR4 local_10;

	this->dynamic.speed = 0.0f;

	pCVar5 = this->field_0x430;
	if (pCVar5 != (CActorMovable*)0x0) {
		local_10.x = pCVar5->currentLocation.x - this->currentLocation.x;
		local_10.z = pCVar5->currentLocation.z - this->currentLocation.z;
		local_10.w = pCVar5->currentLocation.w - this->currentLocation.w;
		local_10.y = 0.0f;
		if ((local_10.x != 0.0f) && (local_10.z != 0.0f)) {
			edF32Vector4NormalizeHard(&local_10, &local_10);
			SV_UpdateOrientation(4.0f, &local_10);
		}

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		this->pAnimationController->GetAnimType_00242330(0);
	
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->field_0x558 = this->field_0x558 + 1;
		}

		if (this->field_0x558 == 3) {
			this->SetBehaviour(WOOF_BEHAVIOUR_VERTICAL_JUMP, -1, -1);
		}
		else {
			iVar4 = CheckArea();
			if (iVar4 == 2) {
				this->SetState(WOOF_STATE_COME_BACK, -1);
			}
			else {
				pVision = this->GetVision();
				pCVar5 = static_cast<CActorMovable*>(pVision->SV_GetNearestActor(this->field_0x430->typeID, 1));
				if (pCVar5 == this->field_0x430) {
					pCVar6 = this->GetPathfinderClient();
					if (pCVar6->id != -1) {
						pCVar5 = this->field_0x430;
						pCVar6 = this->GetPathfinderClient();

						bVar3 = pCVar6->IsValidPosition(&pCVar5->currentLocation);
						if (bVar3 == false) {
							return;
						}
					}

					this->SetState(8, -1);
				}
			}
		}
	}

	return;
}

void CActorWoof::StateWoofComeBack()
{
	float fVar1;
	float fVar2;
	edF32VECTOR4* pLocation;
	int iVar3;
	edF32VECTOR4 local_50;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	CWayPoint* pWayPoint;

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.rotSpeed = 10.0f;
	movParamsIn.speed = this->field_0x390;
	movParamsIn.flags = 0x452;
	movParamsIn.acceleration = 10.0f;

	if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
		if ((this->field_0x410 & 1) == 0) {
			SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &this->baseLocation);
		}
		else {
			pWayPoint = (this->field_0x3a4).Get();
			if (pWayPoint == (CWayPoint*)0x0) {
				ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
				SetState(0x1c, -1);

				return;
			}

			local_50.xyz = pWayPoint->location;
			local_50.w = 1.0f;
			SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &local_50);
		}
	}
	else {
		pLocation = this->pathFollowReader.GetWayPoint();
		SV_AUT_MoveTo(&movParamsOut, &movParamsIn, pLocation);
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	iVar3 = CheckArea();
	if (iVar3 == 1) {
		SetState(8, -1);
	}
	else {
		if ((movParamsOut.flags & 2) == 0) {
			if ((this->field_0x410 & 1) == 0) {
				if (movParamsOut.moveVelocity < 0.25f) {
					this->field_0x430 = (CActorMovable*)0x0;
					this->field_0x530 = 0;
					if ((this->field_0x560 & 2) == 0) {
						if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
							SetState(WOOF_STATE_IDLE, -1);
						}
						else {
							SetState(WOOF_STATE_PATH, -1);
						}
					}
					else {
						SetState(0x11, -1);
					}
				}
			}
			else {
				fVar1 = this->baseLocation.x - this->currentLocation.x;
				fVar2 = this->baseLocation.z - this->currentLocation.z;
				if (sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) < 0.25f) {
					this->field_0x560 = this->field_0x410;
					this->field_0x430 = (CActorMovable*)0x0;
					this->field_0x530 = 0;
					if ((this->field_0x560 & 2) == 0) {
						if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
							SetState(WOOF_STATE_IDLE, -1);
						}
						else {
							SetState(WOOF_STATE_PATH, -1);
						}
					}
					else {
						SetState(0x11, -1);
					}
				}
				else {
					if (movParamsOut.moveVelocity < 0.25f) {
						SetState(0x1c, -1);
					}
				}
			}
		}
		else {
			this->field_0x430 = (CActorMovable*)0x0;
			this->field_0x530 = 0;
			if ((this->field_0x560 & 2) == 0) {
				if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
					SetState(WOOF_STATE_IDLE, -1);
				}
				else {
					SetState(WOOF_STATE_PATH, -1);
				}
			}
			else {
				SetState(0x11, -1);
			}
		}
	}

	return;
}

void CActorWoof::StateWoofSlide()
{
	int iVar1;
	bool bVar2;
	CVision* pVision;
	CActor* pNearest;
	float fVar3;
	CCollision* pCol;

	SV_MOV_DecreaseSpeedIntensity(this->field_0x390 + 3.0f);
	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	pVision = GetVision();
	pNearest = pVision->SV_GetNearestActor(this->field_0x430->typeID, 1);
	if (pNearest != (CActor*)0x0) {
		if (this->field_0x430->typeID != 6) {
			SetState(0x13, -1);
			return;
		}

		iVar1 = this->field_0x430->actorState;
		if ((iVar1 != 0x117) && (iVar1 != 0x118)) {
			SetState(0x13, -1);
			return;
		}
	}

	pCol = this->pCollisionData;
	if ((pCol->flags_0x4 & 1) != 0) {
		fVar3 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.rotationQuat);
		bVar2 = true;

		if (fVar3 < -0.3f) goto LAB_00330148;

		fVar3 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.velocityDirectionEuler);
		bVar2 = true;

		if (fVar3 < -0.3f) goto LAB_00330148;
	}

	bVar2 = false;

LAB_00330148:
	if (bVar2) {
		SetState(0xc, -1);
	}
	else {
		if (this->dynamic.linearAcceleration < 0.1f) {
			SetState(8, -1);
		}
	}

	return;
}

void CActorWoof::StateWoofFall()
{
	bool bVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_10;
	CCollision* pCol;

	local_10.z = this->rotationQuat.z;
	local_10.w = this->rotationQuat.w;
	local_10.x = this->rotationQuat.x;
	local_10.y = 0.0f;
	edF32Vector4NormalizeHard(&local_10, &local_10);
	CActor::SV_UpdateOrientation(3.0f, &local_10);

	ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

	pCol = this->pCollisionData;
	fVar3 = this->lastGroundY - this->currentLocation.y;
	if ((pCol->flags_0x4 & 1) != 0) {
		fVar2 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.rotationQuat);
		bVar1 = true;
		if (fVar2 < -0.3f) goto LAB_003308d0;
		fVar2 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.velocityDirectionEuler);
		bVar1 = true;
		if (fVar2 < -0.3f) goto LAB_003308d0;
	}
	bVar1 = false;
LAB_003308d0:
	if (bVar1) {
		if (8.0f < fVar3) {
			SetState(0xb, -1);
		}
		else {
			SetState(0xc, -1);
		}
	}
	else {
		if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
			if (10.0f < this->timeInAir) {
				SetState(0xb, -1);
			}
		}
		else {
			if (8.0f < fVar3) {
				SetState(0xb, -1);
			}
			else {
				SetState(0xf, -1);
			}
		}
	}

	return;
}

void CActorWoof::StateWoofChase()
{
	CActorMovable* pCVar1;
	CActorMovable* pCVar2;
	bool bVar3;
	bool bVar4;
	CPathFinderClient* pCVar6;
	int iVar7;
	CVision* this_00;
	uint uVar8;
	edF32VECTOR4* peVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack128;
	CActorMovParamsIn movParamsIn;
	undefined4 local_50;
	undefined4 local_48;
	undefined4 local_44;
	CActorMovParamsOut movParamsOut;
	undefined4 local_4;

	movParamsOut.flags = 0;
	local_50 = 0;
	local_48 = 0;
	local_44 = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.rotSpeed = 10.0f;
	movParamsIn.speed = this->field_0x390;
	movParamsIn.flags = 0x452;
	movParamsIn.acceleration = 10.0f;
	this->field_0x430->SV_GetGroundPosition(&eStack128);
	pCVar1 = this->field_0x430;
	local_90 = (pCVar1->dynamic).rotationQuat;

	fVar14 = (this->field_0x430->dynamic).horizontalLinearSpeed * GetTimer()->cutsceneDeltaTime * 8.0f;
	fVar10 = 1.5f;
	if ((fVar14 <= 1.5f) && (fVar10 = fVar14, fVar14 < 0.0f)) {
		fVar10 = 0.0f;
	}

	edF32Vector4ScaleHard(fVar10, &local_90, &local_90);
	edF32Vector4AddHard(&this->field_0x540, &eStack128, &local_90);
	pCVar6 = GetPathfinderClient();
	if (pCVar6->id == -1) {
		SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x540);
		this->field_0x570 = this->field_0x540;
	}
	else {
		pCVar6 = GetPathfinderClient();
		bVar4 = pCVar6->IsValidPosition(&this->currentLocation);
		if (bVar4 == false) {
			SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x580);
			this->field_0x570 = this->field_0x580;
		}
		else {
			pCVar6 = GetPathfinderClient();
			bVar4 = pCVar6->HasPathTo(this, &this->field_0x540);
			if (bVar4 == false) {
				SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x570);
			}
			else {
				SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x540);
				this->field_0x570 = this->field_0x540;
			}

			this->field_0x580 = this->currentLocation;
		}
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	iVar7 = CheckArea();
	if (iVar7 == 2) {
		SetState(WOOF_STATE_COME_BACK, -1);
		return;
	}

	fVar10 = edF32Vector4DotProductHard(&this->dynamic.velocityDirectionEuler, &(this->field_0x430->dynamic).velocityDirectionEuler);
	if (((fVar10 <= -1.0f) || (-0.9f <= fVar10)) || (bVar4 = true, (this->field_0x430->dynamic).linearAcceleration <= 3.0f)) {
		bVar4 = false;
	}

	fVar10 = (this->field_0x570).x - this->currentLocation.x;
	fVar14 = (this->field_0x570).z - this->currentLocation.z;
	if (bVar4) {
		fVar13 = (this->field_0x540).x - this->currentLocation.x;
		fVar11 = (this->field_0x540).z - this->currentLocation.z;
		fVar13 = fVar13 * fVar13 + 0.0f + fVar11 * fVar11;
	}
	else {
		fVar13 = this->field_0x430->currentLocation.x - this->currentLocation.x;
		fVar11 = this->field_0x430->currentLocation.z - this->currentLocation.z;
		fVar13 = fVar13 * fVar13 + 0.0f + fVar11 * fVar11;
	}

	fVar13 = sqrtf(fVar13);
	CActor* pFound = GetVision()->SV_GetNearestActor(this->field_0x430->typeID, 1);
	if ((pFound != (CActor*)0x0) || (pCVar6 = GetPathfinderClient(), pCVar6->id != -1)) {
		fVar11 = this->field_0x430->currentLocation.y - this->currentLocation.y;
		if (fVar11 < 0.0f) {
			fVar11 = 0.0f;
		}

		fVar12 = 1.0f;
		if (fVar11 <= 1.0f) {
			fVar12 = fVar11;
		}

		fVar13 = fVar13 - fVar12;
		if ((3.0f <= fVar13) || (pFound == (CActor*)0x0)) {
			if (1.0f <= sqrtf(fVar10 * fVar10 + 0.0f + fVar14 * fVar14)) {
				return;
			}

			if (this->field_0x430 != CActorHero::_gThis) {
				return;
			}

			SetState(WOOF_STATE_YAP, -1);
			return;
		}

		pCVar2 = this->field_0x430;
		if (pCVar2 != CActorHero::_gThis) {
			if (1.5f <= fVar13) {
				return;
			}

			SetState(0x21, -1);
			return;
		}

		peVar9 = pCVar2->GetBottomPosition();
		fVar10 = (peVar9->y + pCVar2->currentLocation.y) / 2.0f - this->currentLocation.y;
		if (fVar10 <= 2.0f) {
			fVar14 = fVar10 + (this->field_0x430->dynamic).linearAcceleration * (this->field_0x430->dynamic).velocityDirectionEuler.y * GetTimer()->cutsceneDeltaTime * 5.0f;
			fVar10 = 2.0f;
			if (fVar14 < 2.0f) {
				fVar10 = fVar14;
			}
		}

		if (3.5f < fVar10) {
			return;
		}

		if (fVar10 <= -0.5f) {
			return;
		}

		if (CActorHero::_gThis == this->field_0x430) {
			uVar8 = CActorHero::_gThis->FUN_00132f00(0xffffffff);
			if (uVar8 != 0) {
				bVar3 = true;
				goto LAB_00331bf0;
			}
		}
		else {
			if ((this->field_0x430->pCollisionData->flags_0x4 & 2) == 0) {
				bVar3 = true;
				goto LAB_00331bf0;
			}
		}

		bVar3 = false;

	LAB_00331bf0:
		if (bVar3) {
			SetState(WOOF_STATE_SLIDE, -1);
			return;
		}

		this->field_0x430->SV_GetBoneWorldPosition(0x544e8aa8, &eStack160);

		eStack160.y = eStack160.y - this->currentLocation.y;
		if ((eStack160.y < 0.6f) && (-0.5f < eStack160.y)) {
			if (1.3f <= fVar13) {
				return;
			}

			(this->pCollisionData)->actorFieldA = this->field_0x430;
			(this->field_0x430->pCollisionData)->actorFieldA = this;
			DoMessage(this->field_0x430, (ACTOR_MESSAGE)3, (MSG_PARAM)3);
			SetState(0x2a, -1);
			return;
		}

		if (fVar10 < 0.0f) {
			fVar10 = 0.0f;
		}

		if (this->dynamic.speed < 7.0f) {
			this->dynamic.speed = 7.0f;
		}

		if (bVar4) {
			this->field_0x5a0.BuildFromSpeedDist(fVar13 + 17.0f, 4.0f, fVar10);
		}
		else {
			this->field_0x5a0.BuildFromSpeedDist(fVar13 + this->dynamic.speed, 4.0f, fVar10);
		}

		SetState(0x1d, -1);

		return;
	}

	pCVar2 = this->field_0x430;
	if (pCVar2 != CActorHero::_gThis) goto LAB_00331a50;

	if (CActorHero::_gThis == pCVar2) {
		uVar8 = CActorHero::_gThis->FUN_00132f00( 0xffffffff);
		if (uVar8 == 0) {
		LAB_00331a20:
			bVar4 = false;
		}
		else {
			bVar4 = true;
		}
	}
	else {
		if (((pCVar2->pCollisionData)->flags_0x4 & 2) != 0) goto LAB_00331a20;
		bVar4 = true;
	}

	if (bVar4) {
		SetState(0x14, -1);
		return;
	}

LAB_00331a50:
	SetState(WOOF_STATE_SLIDE, -1);

	return;
}

void CActorWoof::StateWoofChaseIntruderLost()
{
	bool bVar1;
	CPathFinderClient* pPathFinderClient;
	int iVar2;
	CVision* pVision;
	edF32VECTOR4* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_50;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.speed = 0.0f;
	movParamsIn.flags = 0x50;
	this->field_0x430->SV_GetGroundPosition(&local_50);
	movParamsIn.rotSpeed = 10.0f;
	movParamsIn.speed = this->field_0x390;
	movParamsIn.flags = movParamsIn.flags | 0x402;
	movParamsIn.acceleration = 10.0f;
	pPathFinderClient = GetPathfinderClient();
	if (pPathFinderClient->id == -1) {
		SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &local_50);
		this->field_0x570 = local_50;
	}
	else {
		pPathFinderClient = GetPathfinderClient();
		bVar1 = pPathFinderClient->IsValidPosition(&this->currentLocation);
		if (bVar1 == false) {
			SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x580);
			this->field_0x570 = this->field_0x580;
		}
		else {
			pPathFinderClient = GetPathfinderClient();
			bVar1 = pPathFinderClient->HasPathTo(this, &local_50);
			if (bVar1 == false) {
				SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x570);
			}
			else {
				SV_AUT_MoveTo(&movParamsOut, &movParamsIn, &local_50);
				this->field_0x570 = local_50;
			}

			this->field_0x580 = this->currentLocation;
		}
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	iVar2 = CheckArea();
	if (iVar2 == 2) {
		SetState(WOOF_STATE_COME_BACK, -1);
	}
	else {
		fVar4 = (this->field_0x570).x - this->currentLocation.x;
		fVar5 = (this->field_0x570).z - this->currentLocation.z;
		fVar4 = sqrtf(fVar4 * fVar4 + 0.0f + fVar5 * fVar5);
		if (GetVision()->SV_GetNearestActor(this->field_0x430->typeID, 1) == (CActor*)0x0) {
			if (fVar4 < 0.5f) {
				SetBehaviour(WOOF_BEHAVIOUR_VERTICAL_JUMP, -1, -1);
			}
			else {
				if (1.0f < this->timeInAir) {
					peVar3 = &this->currentLocation;
					fVar5 = (this->field_0x590).x - peVar3->x;
					fVar6 = (this->field_0x590).z - this->currentLocation.z;
					if (sqrtf(fVar5 * fVar5 + 0.0f + fVar6 * fVar6) < 1.0f) {
						if (1.0f < fVar4) {
							SetState(WOOF_STATE_YAP, -1);
						}
						else {
							SetBehaviour(WOOF_BEHAVIOUR_VERTICAL_JUMP, -1, -1);
						}
					}
					else {
						this->field_0x590 = *peVar3;
						this->timeInAir = 0.0f;
					}
				}
			}
		}
		else {
			SetState(WOOF_STATE_CHASE, -1);
		}
	}

	return;
}

void CActorWoof::StateWoofAttackHero_2_4()
{
	CCollision* pCol;
	bool bVar2;
	float fVar4;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	undefined4 local_4;

	bVar2 = this->field_0x5a0.IsFinished();
	if ((bVar2 != false) || (((this->pCollisionData)->flags_0x4 & 4) != 0)) {
		local_20.z = this->dynamic.rotationQuat.z;
		local_20.w = this->dynamic.rotationQuat.w;
		local_20.x = this->dynamic.rotationQuat.x;
		local_20.y = 0.0f;
		edF32Vector4NormalizeHard(&local_20, &local_20);
		this->dynamic.rotationQuat = local_20;
		this->lastGroundY = this->currentLocation.y;
		ManageDyn(4.0f, 0x40129, (CActorsTable*)0x0);
		SetState(WOOF_STATE_ATTACK_HERO_3_4, -1);

		return;
	}

	this->field_0x5a0.Integrate(GetTimer()->cutsceneDeltaTime);
	this->dynamicExt.instanceIndex.y = this->field_0x5a0.field_0x20;
	this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x8000;
	ManageDyn(4.0f, 0x40129, (CActorsTable*)0x0);
	pCol = this->pCollisionData;
	if ((pCol->flags_0x4 & 1) != 0) {
		fVar4 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.rotationQuat);
		bVar2 = true;

		if (fVar4 < -0.3f) goto LAB_00330740;

		fVar4 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.velocityDirectionEuler);
		bVar2 = true;

		if (fVar4 < -0.3f) goto LAB_00330740;
	}

	bVar2 = false;

LAB_00330740:
	if (bVar2) {
		SetState(0xc, -1);
	}
	else {
		this->field_0x430->SV_GetBoneWorldPosition(0x544e8aa8, &eStack48);
		bVar2 = DetectWithVision(&eStack48, &this->rotationQuat, false);
		if (bVar2 != false) {
			local_4 = 3;
			DoMessage((CActor*)this->field_0x430, (ACTOR_MESSAGE)3, (MSG_PARAM)3);
			SetState(0x27, -1);
		}
	}

	return;
}

void CActorWoof::StateWoofAttackHero_3_4()
{
	byte bVar1;
	bool bVar2;
	float fVar3;
	edF32VECTOR4 eStack32;
	undefined4 local_4;
	CCollision* pCol;

	ManageDyn(4.0f, 0x40129, (CActorsTable*)0x0);

	pCol = this->pCollisionData;
	bVar1 = pCol->flags_0x4;
	if ((bVar1 & 2) != 0) {
		SetState(WOOF_STATE_ATTACK_HERO_4_4, -1);
		return;
	}

	if (5.0f < this->lastGroundY - this->currentLocation.y) {
		SetState(WOOF_STATE_FALL, -1);
		return;
	}

	if ((bVar1 & 1) != 0) {
		fVar3 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.rotationQuat);
		bVar2 = true;

		if (fVar3 < -0.3f) goto LAB_00330510;

		fVar3 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->dynamic.velocityDirectionEuler);
		bVar2 = true;

		if (fVar3 < -0.3f) goto LAB_00330510;
	}

	bVar2 = false;

LAB_00330510:
	if (bVar2) {
		SetState(0xc, -1);
	}
	else {
		this->field_0x430->SV_GetBoneWorldPosition(0x544e8aa8, &eStack32);
		bVar2 = DetectWithVision(&eStack32, &this->rotationQuat, false);
		if (bVar2 != false) {
			local_4 = 3;
			DoMessage(this->field_0x430, (ACTOR_MESSAGE)3, (MSG_PARAM)3);
			SetState(0x28, -1);
		}
	}

	return;
}

void CBehaviourWoof::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourWoof::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorWoof*>(pOwner);

	return;
}

int CBehaviourWoof::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	int iVar1;
	CLifeInterface* pCVar2;
	StateConfig* pSVar3;
	uint uVar4;
	float fVar5;
	CActorHero* pHero;
	CActorWoof* pWoof;

	if (msg == 0x1c) {
		pCVar2 = this->pOwner->GetLifeInterface();
		fVar5 = pCVar2->GetValue();
		if (((0.0f < fVar5) && (this->pOwner->actorState == 0xc)) && (pSender->typeID == ACTOR_HERO_PRIVATE)) {
			this->pOwner->field_0x430 = static_cast<CActorHero*>(pSender);
			this->pOwner->SetBehaviour(WOOF_BEHAVIOUR_GUARD, 8, -1);
			return 1;
		}
	}
	else {
		if (msg == 0x10) {
			pCVar2 = this->pOwner->GetLifeInterface();
			fVar5 = pCVar2->GetValue();
			if ((0.0f < fVar5) && ((this->pOwner->field_0x560 & 1) == 0)) {
				this->pOwner->field_0x530 = 0;
				this->pOwner->field_0x430 = (CActorHero*)0x0;
				this->pOwner->field_0x560 = this->pOwner->field_0x560 | 1;
				this->pOwner->SetBehaviour(WOOF_BEHAVIOUR_GUARD, WOOF_STATE_IDLE, -1);
				return 1;
			}
		}
		else {
			if (msg == 0xf) {
				pCVar2 = this->pOwner->GetLifeInterface();
				fVar5 = pCVar2->GetValue();
				if (0.0f < fVar5) {
					pWoof = this->pOwner;
					iVar1 = pWoof->actorState;
					if (iVar1 == -1) {
						uVar4 = 0;
					}
					else {
						pSVar3 = pWoof->GetStateCfg(iVar1);
						uVar4 = pSVar3->flags_0x4 & 0x2000;
					}
					if ((uVar4 != 0) && ((this->pOwner->field_0x560 & 1) != 0)) {
						this->pOwner->field_0x530 = 1;
						this->pOwner->field_0x430 = CActorHero::_gThis;
						this->pOwner->field_0x560 = this->pOwner->field_0x560 & 0xfffffffc;
						this->pOwner->SetBehaviour(WOOF_BEHAVIOUR_GUARD, WOOF_STATE_CHASE_INTRUDER_LOST, -1);
						return 1;
					}
				}
			}
			else {
				if (msg != 2) {
					return 0;
				}

				_msg_hit_param* pHitParam = static_cast<_msg_hit_param*>(pMsgParam);

				if (pHitParam->projectileType == 5) {
					pCVar2 = this->pOwner->GetLifeInterface();
					fVar5 = pCVar2->GetValue();
					if (0.0f < fVar5) {
						return 1;
					}
				}
				else {
					if (pHitParam->projectileType == 4) {
						pCVar2 = this->pOwner->GetLifeInterface();
						fVar5 = pCVar2->GetValue();
						pHero = CActorHero::_gThis;
						if ((0.0f < fVar5) && (this->pOwner->actorState != 8)) {
							this->pOwner->field_0x530 = 1;
							this->pOwner->field_0x430 = pHero;
							this->pOwner->field_0x560 = this->pOwner->field_0x560 & 0xfffffffd;
							this->pOwner->SetBehaviour(WOOF_BEHAVIOUR_GUARD, 8, -1);
							return 1;
						}
					}
				}
			}
		}
	}

	return 0;
}

int CBehaviourWoof::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourWoofGuard::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourWoofGuard::Manage()
{
	this->pOwner->BehaviourWoofGuard_Manage();

	return;
}

void CBehaviourWoofGuard::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorWoof* pWoof;

	CBehaviourWoof::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		pWoof = this->pOwner;
		pWoof->field_0x430 = (CActorHero*)0x0;
		pWoof->field_0x530 = 0;

		if ((pWoof->field_0x560 & 2) == 0) {
			if ((pWoof->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
				pWoof->SetState(WOOF_STATE_IDLE, -1);
			}
			else {
				pWoof->SetState(WOOF_STATE_PATH, -1);
			}
		}
		else {
			pWoof->SetState(0x11, -1);
		}
	}
	else {
		pWoof = this->pOwner;
		pWoof->SetState(newState, -1);
	}

	return;
}

void CBehaviourWoofGuard::End(int newBehaviourId)
{
	return;
}

void CBehaviourWoofGuard::InitState(int newState)
{
	this->pOwner->BehaviourWoofGuard_InitState(newState);

	return;
}

void CBehaviourWoofGuard::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourWoofGuard_TermState(oldState);

	return;
}

int CBehaviourWoofGuard::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return CBehaviourWoof::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourWoofVerticalJump::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourWoofVerticalJump::Manage()
{
	this->pOwner->BehaviourWoofVerticalJump_Manage();

	return;
}

void CBehaviourWoofVerticalJump::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CActorWoof* pWoof;

	this->pOwner = static_cast<CActorWoof*>(pOwner);
	if (newState == -1) {
		pWoof = this->pOwner;
		pWoof->SetState(9, -1);
	}
	else {
		pWoof = this->pOwner;
		pWoof->SetState(newState, -1);
	}

	this->pOwner->field_0x554 = 0;

	return;
}

void CBehaviourWoofVerticalJump::End(int newBehaviourId)
{
	return;
}

void CBehaviourWoofVerticalJump::InitState(int newState)
{
	this->pOwner->BehaviourWoofVerticalJump_InitState(newState);

	return;
}

void CBehaviourWoofVerticalJump::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourWoofVerticalJump_TermState(oldState);

	return;
}

int CBehaviourWoofVerticalJump::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourWoof::InterpretMessage(pSender, msg, pMsgParam);
}
