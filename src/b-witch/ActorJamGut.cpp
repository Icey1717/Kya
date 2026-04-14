#include "ActorJamgut.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "LightManager.h"
#include "EventManager.h"
#include "ActorFactory.h"
#include "ActorMovingPlatform.h"
#include "ActorHero.h"
#include "ActorHero_Private.h"
#include "TimeController.h"
#include "LevelScheduler.h"
#include "Cheat.h"
#include "WayPoint.h"
#include "CollisionRay.h"
#include "InputManager.h"

void CActorJamGut::Create(ByteCode* pByteCode)
{
	uint nbToCreate;
	int curIndex;
	S_JMG_CHECKPOINT* pCurCheckpoint;
	S_JMG_PATH* pCurPath;

	CActorAutonomous::Create(pByteCode);

	this->field_0x400 = pByteCode->GetF32();
	this->field_0x404 = pByteCode->GetF32();
	this->field_0x408 = pByteCode->GetF32();
	this->field_0x40c = pByteCode->GetF32();
	this->field_0x410 = pByteCode->GetF32();
	this->field_0x414 = pByteCode->GetF32();
	if (2.28f <= CScene::_pinstance->field_0x1c) {
		this->field_0x41c = pByteCode->GetF32();
	}
	else {
		this->field_0x41c = 50.0f;
	}

	this->field_0x4a4 = pByteCode->GetF32();

	this->field_0x374 = pByteCode->GetU32();
	this->flags_0x358 = pByteCode->GetU32();

	this->nbCheckpoints = pByteCode->GetS32();
	this->aCheckpoints = (S_JMG_CHECKPOINT*)0x0;
	nbToCreate = this->nbCheckpoints;
	if (nbToCreate != 0) {
		this->aCheckpoints = new S_JMG_CHECKPOINT[nbToCreate];
		pCurCheckpoint = this->aCheckpoints;
		curIndex = 0;
		if (0 < this->nbCheckpoints) {
			do {
				(pCurCheckpoint->zoneRef).index = pByteCode->GetS32();
				(pCurCheckpoint->waypointRef).index = pByteCode->GetS32();
				curIndex = curIndex + 1;
				pCurCheckpoint = pCurCheckpoint + 1;
			} while (curIndex < this->nbCheckpoints);
		}
	}

	this->nbPaths = pByteCode->GetS32();
	this->aPaths = (S_JMG_PATH*)0x0;
	nbToCreate = this->nbPaths;
	if (nbToCreate != 0) {
		this->aPaths = new S_JMG_PATH[nbToCreate];
		pCurPath = this->aPaths;
		curIndex = 0;
		if (0 < this->nbPaths) {
			do {
				pCurPath->zoneRef.index = pByteCode->GetS32();
				pCurPath->pathPlane.pathFollowReader.Create(pByteCode);
				pCurPath->actorRef.index = pByteCode->GetS32();
				pCurPath->field_0x28 = pByteCode->GetF32();
				pCurPath->field_0x2c = pByteCode->GetF32();
				pCurPath->field_0x30 = pByteCode->GetF32();
				curIndex = curIndex + 1;
				pCurPath = pCurPath + 1;
			} while (curIndex < this->nbPaths);
		}
	}

	this->field_0x368.type = pByteCode->GetS32();

	return;
}

void CActorJamGut::Init()
{
	int curIndex;
	S_JMG_CHECKPOINT* pCurCheckpoint;
	S_JMG_PATH* pCurPath;

	ClearLocalData();

	this->field_0x5d0 = this->currentLocation;
	this->field_0x5e0 = this->rotationEuler.xyz;
	this->field_0x5b0 = this->field_0x5d0;
	this->field_0x5c0 = this->field_0x5e0;
	this->field_0x5cc = this->field_0x5ec;

	this->field_0x5f0 = this->currentLocation;
	this->field_0x600 = this->rotationEuler.xyz;

	this->field_0x620 = 0;
	this->field_0x618 = -1;

	pCurCheckpoint = this->aCheckpoints;
	curIndex = 0;
	if (0 < this->nbCheckpoints) {
		do {
			pCurCheckpoint->zoneRef.Init();
			pCurCheckpoint->waypointRef.Init();
			curIndex = curIndex + 1;
			pCurCheckpoint = pCurCheckpoint + 1;
		} while (curIndex < this->nbCheckpoints);
	}

	pCurPath = this->aPaths;
	curIndex = 0;
	if (0 < this->nbPaths) {
		do {
			pCurPath->zoneRef.Init();
			pCurPath->pathPlane.Init();
			pCurPath->actorRef.Init();
			pCurPath->pathFollowReader.Create(this->field_0x404, (pCurPath->pathPlane).pathFollowReader.pPathFollow, 1);

			curIndex = curIndex + 1;
			pCurPath = pCurPath + 1;
		} while (curIndex < this->nbPaths);
	}

	CActorAutonomous::Init();

	return;
}

void CActorJamGut::Manage()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pCVar2;
	float local_8;
	float local_4;

	ManagePaths();

	CActor::Manage();

	ManageCheckpoints();

	if (this->field_0x4fc <= Timer::GetTimer()->scaledTotalTime) {
		pHero = this->pRider;
		if ((pHero != (CActorHero*)0x0) && (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
			pCVar2 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
			pCVar2->SetInvincible(2.0f, 0);
		}

		this->field_0x4fc = Timer::GetTimer()->scaledTotalTime;
		this->field_0x500 = 0;
	}
	else {
		this->field_0x500 = this->field_0x500 + 1U & 7;
	}

	if (gShrinkJamgutCheatSeq_00449828 == 0) {
		local_8 = this->scale.x;
		if (local_8 != (this->pCinData->scale).x) {
			SV_UpdateValue((this->pCinData)->scale.x, 0.5f, &local_8);
			SetScaleVector(local_8, local_8, local_8);
		}
	}
	else {
		local_4 = this->scale.x;
		SV_UpdateValue(0.4f, 0.5f, &local_4);
		SetScaleVector(local_4, local_4, local_4);
	}

	return;
}

void CActorJamGut::ComputeLighting()
{
	CScene::ptable.g_LightManager_004516b0->ComputeLighting(this->lightingFloat_0xe0, this, this->lightingFlags, &(this->lightConfig).config);

	if (this->field_0x500 == 1) {
		(this->lightConfig).lightAmbient.x = 255.0f;
		(this->lightConfig).lightAmbient.y = 255.0f;
		(this->lightConfig).lightAmbient.z = 255.0f;
	}

	return;
}

void CActorJamGut::Reset()
{
	edF32MATRIX4 rot;

	CActorAutonomous::Reset();

	ClearLocalData();

	if (this->field_0x620 == 0) {
		this->field_0x5f0 = this->baseLocation;
		this->field_0x600 = this->pCinData->rotationEuler;
		this->field_0x60c = 0;
		this->field_0x618 = -1;
	}
	else {
		edF32Matrix4FromEulerSoft(&rot, &this->field_0x600, "XYZ");
		this->rotationEuler.x = 0.0f;
		this->rotationEuler.y = GetAngleYFromVector(&rot.rowZ);
		this->rotationEuler.z = 0.0f;

		SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
		UpdatePosition(&this->field_0x5f0, true);
		this->field_0x620 = 0;
	}

	return;
}

void CActorJamGut::CheckpointReset()
{
	edF32MATRIX4 rot;

	CActorAutonomous::CheckpointReset();

	SetBehaviour(-1, -1, -1);

	this->pAnimationController->Reset(this);
	this->pCollisionData->Reset();

	SetBehaviour(this->subObjA->defaultBehaviourId, -1, -1);

	ResetActorSound();

	if (this->pTiedActor != (CActor*)0x0) {
		TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
	}

	if (CActorHero::_gThis->field_0xcc0 == 0) {
		edF32Matrix4FromEulerSoft(&rot, &this->field_0x5e0, "XYZ");
		UpdatePosition(&this->field_0x5d0, true);
	}
	else {
		edF32Matrix4FromEulerSoft(&rot, &this->field_0x5c0, "XYZ");
		UpdatePosition(&this->field_0x5b0, true);

		this->field_0x5d0 = this->field_0x5b0;
		this->field_0x5e0 = this->field_0x5c0;

		this->field_0x5ec = this->field_0x5cc;
	}

	this->rotationEuler.x = 0.0f;
	this->rotationEuler.y = GetAngleYFromVector(&rot.rowZ);
	this->rotationEuler.z = 0.0f;

	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);

	this->dynamic.Reset(this);
	this->dynamicExt.ClearLocalData();

	ClearLocalData();

	this->field_0x620 = 0;

	LifeRestore();

	return;
}

struct S_JMG_SAVE_CONTEXT
{
	edF32VECTOR3 position_1;
	edF32VECTOR3 rotation_1;
	edF32VECTOR3 position_2;
	edF32VECTOR3 rotation_2;
};

void CActorJamGut::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_JMG_SAVE_CONTEXT* pSaveContext = reinterpret_cast<S_JMG_SAVE_CONTEXT*>(pData);

	pSaveContext->position_1 = this->field_0x5d0.xyz;
	pSaveContext->rotation_1 = this->field_0x5e0;
	pSaveContext->position_2 = this->field_0x5b0.xyz;
	pSaveContext->rotation_2 = this->field_0x5c0;

	return;
}

void CActorJamGut::LoadContext(void* pData, uint mode, uint maxSize)
{
	if (((mode == 2) || (mode == 1)) && (CLevelScheduler::gThis->bShouldLoad != 0)) {
		S_JMG_SAVE_CONTEXT* pLoadContext = reinterpret_cast<S_JMG_SAVE_CONTEXT*>(pData);
		this->field_0x5d0.xyz = pLoadContext->position_1;
		this->field_0x5d0.w = 1.0f;
		this->field_0x5e0 = pLoadContext->rotation_1;
		this->field_0x5ec = 0;

		if (1 < mode) {
			this->field_0x5b0.xyz = pLoadContext->position_2;
			this->field_0x5b0.w = 1.0f;
			this->field_0x5c0 = pLoadContext->rotation_2;
			this->field_0x5cc = 0;
		}
	}
}

StateConfigRider CActorJamGut::gStateCfg_JMG[21] = {
	StateConfigRider(0x0C, 0x0100, -1),         // [0]
	StateConfigRider(0x00, 0x1100, 0x011E),     // [1]
	StateConfigRider(0x0D, 0x1102, 0x011F),     // [2]
	StateConfigRider(0x0E, 0xA100, 0x0120),     // [3]
	StateConfigRider(0x0E, 0xA100, 0x0122),     // [4]
	StateConfigRider(0x0F, 0x0100, 0x0124),     // [5]
	StateConfigRider(0x10, 0x2000, 0x0125),     // [6]
	StateConfigRider(0x12, 0x2100, 0x0126),     // [7]
	StateConfigRider(0x13, 0x2100, 0x0127),     // [8]
	StateConfigRider(0x11, 0x2000, 0x0128),     // [9]
	StateConfigRider(0x00, 0x0000, 0x0129),     // [10]
	StateConfigRider(0x15, 0x0800, 0x012A),     // [11]
	StateConfigRider(0x16, 0x0800, 0x012B),     // [12]
	StateConfigRider(0x17, 0x0101, 0x012C),     // [13]
	StateConfigRider(0x18, 0x0101, 0x012D),     // [14]
	StateConfigRider(0x19, 0x0001, 0x012E),     // [15]
	StateConfigRider(0x1E, 0x0001, 0x0124),     // [16]
	StateConfigRider(0x1C, 0x0000, 0x011E),     // [17]
	StateConfigRider(0x1D, 0x0000, 0x011E),     // [18]
	StateConfigRider(0x1E, 0x4000, -1),         // [19]
	StateConfigRider(0x0C, 0x4000, -1),         // [20]
}; 

StateConfig* CActorJamGut::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		pStateConfig = gStateCfg_JMG + state + -6;
	}

	return pStateConfig;
}

CBehaviour* CActorJamGut::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == JAMGUT_BEHAVIOUR_RIDDEN) {
		pBehaviour = &this->behaviourRidden;
	}
	else {
		if (behaviourType == JAMGUT_BEHAVIOUR_STAND) {
			pBehaviour = &this->behaviourStand;
		}
		else {
			pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

static uint _gBehaviourFlags_JMG[9] = {
	0x0,
	0xffffffff,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0xc,
	0x100,
};

uint CActorJamGut::GetBehaviourFlags(int state)
{
	assert(state >= 0 && state < 9);
	return _gBehaviourFlags_JMG[state];
}

void CActorJamGut::SetState(int newState, int animType)
{
	int iVar1;
	CActorHero* pHero;
	StateConfig* pSVar3;
	uint uVar4;
	CLifeInterface* pCVar5;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	float fVar6;

	if (((GetStateFlags(this->actorState) & 0x800) != 0) && (pHero = this->pRider, pHero != (CActorHero*)0x0)) {
		fVar6 = pHero->GetLifeInterface()->GetValue();
		if ((fVar6 <= 0.0f) || (0.0f < this->pRider->field_0x2e4)) {
			newState = ChooseStateDead(0xc, 4, 1);
		}
	}

	JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::SetState: {} -> {}", static_cast<int>(this->actorState), newState);
	CActorMovable::SetState(newState, animType);

	int riderState = -1;

	if (((animType == -1) && (riderState = -1, newState != AS_None)) && (riderState = -1, 5 < newState)) {
		riderState = GetStateRider(newState);
	}

	pHero = this->pRider;
	if (((pHero != (CActorHero*)0x0) &&
		(pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) && (riderState != -1)) {
		pHeroRideJamGut = (CBehaviourHeroRideJamGut*)pHero->GetBehaviour(pHero->curBehaviourId);
		pHeroRideJamGut->SetState(riderState, 0);
	}

	return;
}

bool CActorJamGut::CanPassThrough()
{
	return this->curBehaviourId == JAMGUT_BEHAVIOUR_RIDDEN;
}

void CActorJamGut::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	edAnmMacroBlendN macroBlendN = edAnmMacroBlendN(pAnimator->pAnimKeyTableEntry);
	char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
	AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

	if (newAnim == 0x14) {
		float fVar4 = this->field_0x380;
		if (0.0f <= fVar4) {
			pValue->field_0xc_array[2] = fVar4;
			pValue->field_0xc_array[1] = 1.0f - pValue->field_0xc_array[2];
			pValue->field_0xc_array[0] = 0.0f;
		}
		else {
			pValue->field_0xc_array[0] = fVar4;
			pValue->field_0xc_array[1] = 1.0f - pValue->field_0xc_array[0];
			pValue->field_0xc_array[2] = 0.0f;
		}
	}
	else if (newAnim == 0xe) {
		float fVar4 = this->field_0x378;
		uint uVar1 = this->flags_0x358 & 1;

		if ((fVar4 < 0.0f) || (uVar1 == 0)) {
			if (uVar1 == 0) {
				if (0.0f < fVar4) {
					this->field_0x378 = 0.0f;
				}
			}
			else {
				pValue->field_0xc_array[3] = 0.0f;
				pValue->field_0xc_array[4] = 0.0f;
				pValue->field_0xc_array[5] = 0.0f;
			}

			float r1 = this->field_0x380;
			if (0.0f <= r1) {
				CActor::SV_Blend4AnimationsWith2Ratios(r1, -this->field_0x378, &macroBlendN, 4, 5, 1, 2);
				pValue->field_0xc_array[0] = 0.0f;
				pValue->field_0xc_array[3] = 0.0f;
			}
			else {
				CActor::SV_Blend4AnimationsWith2Ratios(-r1, -this->field_0x378, &macroBlendN, 4, 3, 1, 0);
				pValue->field_0xc_array[2] = 0.0f;
				pValue->field_0xc_array[5] = 0.0f;
			}
		}
		else {
			float r1 = this->field_0x380;
			if (0.0f <= r1) {
				CActor::SV_Blend4AnimationsWith2Ratios(r1, fVar4, &macroBlendN, 4, 5, 7, 8);
				pValue->field_0xc_array[3] = 0.0f;
				pValue->field_0xc_array[6] = 0.0f;
			}
			else {
				CActor::SV_Blend4AnimationsWith2Ratios(-r1, fVar4, &macroBlendN, 4, 3, 7, 6);
				pValue->field_0xc_array[5] = 0.0f;
				pValue->field_0xc_array[6] = 0.0f;
			}
			pValue->field_0xc_array[0] = 0.0f;
			pValue->field_0xc_array[1] = 0.0f;
			pValue->field_0xc_array[2] = 0.0f;
		}
	}
	else {
		CActor::AnimEvaluate(layerId, pAnimator, newAnim);
	}

	return;
}

void CActorJamGut::CinematicMode_Leave(int behaviourId)
{
	if (this->field_0x35c == 0) {
		CActor::CinematicMode_Leave(behaviourId);
	}
	else {
		CActor::CinematicMode_Leave(JAMGUT_BEHAVIOUR_RIDDEN);
		SetState(7, -1);
		this->field_0x35c = 0;
	}

	return;
}

bool CActorJamGut::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	CActorAutonomous::CarriedByActor(pActor, m0);
	this->field_0x360 = 0;

	return true;
}

CPlayerInput* CActorJamGut::GetInputManager(int param_2, int param_3)
{
	CPlayerInput* pInputManager;
	CActorHero* pHero;

	pHero = this->pRider;
	if (pHero == (CActorHero*)0x0) {
		pInputManager = CActor::GetInputManager(param_2, param_3);
	}
	else {
		pInputManager = pHero->GetInputManager(param_2, param_3);
	}

	return pInputManager;
}

int CActorJamGut::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	StateConfig* pSVar1;
	uint uVar2;
	CActorWindState* pCVar3;
	int result;
	edF32VECTOR4* v0;
	float fVar5;
	edF32VECTOR4 eStack16;

	if (msg == MESSAGE_SPAWN) {
		this->flags = this->flags & 0xfffffffc;
		this->flags = this->flags & 0xffffff5f;

		EvaluateDisplayState();

		SetBehaviour(- 1, -1, -1);
		SetBehaviour(this->subObjA->defaultBehaviourId, -1, -1);

		_msg_spawn_params* pSpawnParams = reinterpret_cast<_msg_spawn_params*>(pMsgParam);

		this->rotationQuat = pSpawnParams->rotation;
		GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
		UpdatePosition(&pSpawnParams->position, true);
		result = 1;
		this->field_0x5d0 = this->currentLocation;
		this->field_0x5e0 = this->rotationEuler.xyz;

		this->field_0x5b0 = this->field_0x5d0;
		this->field_0x5c0 = this->field_0x5e0;
		this->field_0x5cc = this->field_0x5ec;
	}
	else {
		if (msg == 0x16) {
			IMPLEMENTATION_GUARD(
			result = this->actorState;
			if (result != 0x10) {
				if (result == -1) {
					uVar2 = 0;
				}
				else {
					pSVar1 = (*(this->pVTable)->GetStateCfg)((CActor*)this, result);
					uVar2 = pSVar1->flags_0x4;
				}
				if ((uVar2 & 1) == 0) {
					edF32Vector4ScaleHard
					(*(float*)((int)pMsgParam + 0x10), &eStack16, (edF32VECTOR4*)pMsgParam);
					v0 = this->dynamicExt.aImpulseVelocities + 2;
					edF32Vector4AddHard(v0, v0, &eStack16);
					fVar5 = edF32Vector4GetDistHard(v0);
					this->dynamicExt.aImpulseVelocityMagnitudes[2] = fVar5;
					pCVar3 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
					if (pCVar3 == (CActorWindState*)0x0) {
						fVar5 = 0.0;
					}
					else {
						pCVar3 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
						fVar5 = pCVar3->windImpulseSpeed;
					}
					if (fVar5 < 0.001) {
						this->dynamicExt.field_0x6c = this->dynamicExt.field_0x6c * 0.25;
					}
					return 1;
				}
			}
			result = 1;)
		}
		else {
			if (msg == 0x4d) {
				result = 1;
				if (pMsgParam == (void*)0x0) {
					result = this->field_0x374;
				}
			}
			else {
				if (msg == 0x14) {
					if ((this->flags & 0x800000) == 0) {
						result = 0;
					}
					else {
						this->pRider = (CActorHero*)pSender;
						result = 1;
						this->field_0x35c = 1;
					}
				}
				else {
					result = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}

	return result;
}

int CActorJamGut::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	uint uVar1;
	StateConfig* pConfig;
	uint uVar2;
	int result;

	uVar1 = param_5[1];
	if (*param_5 == EVENT_PRIM_KILL) {
		if ((GetStateFlags(this->actorState) & 1) == 0) {
			if ((param_3 == 2) && (this->actorState != 0x10)) {
				if (uVar1 == 2) {
					SetState(0x13, -1);
					return 1;
				}

				if (uVar1 == EVENT_PRIM_KILL_DROWN) {
					SetState(0x16, -1);
					return 1;
				}

				if (uVar1 == EVENT_PRIM_KILL_FALL) {
					SetState(0x15, -1);
					return 1;
				}
			}

			result = 1;
		}
		else {
			result = 0;
		}
	}
	else {
		result = CActor::InterpretEvent(pEventMessage, param_3, param_4, param_5);
	}

	return result;
}

void CActorJamGut::ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable)
{
	CActorAutonomous::ManageDyn(param_1, flags, pActorsTable);

	if ((GetStateFlags(this->actorState) & 0x100) != 0) {
		this->field_0x4f4 = this->currentLocation.y + 20.0f;
		this->field_0x450 = this->currentLocation;
	}

	return;
}

void CActorJamGut::SetRestartWaypoint(CWayPoint* pWayPoint, uint param_3)
{
	if (pWayPoint != (CWayPoint*)0x0) {
		this->field_0x5d0.xyz = pWayPoint->location;
		(this->field_0x5d0).w = 1.0f;
		this->field_0x5e0 = pWayPoint->rotation;
		this->field_0x5ec = 0;

		if (param_3 != 0) {
			this->field_0x5b0.xyz = pWayPoint->location;
			(this->field_0x5b0).w = 1.0f;
			this->field_0x5c0 = pWayPoint->rotation;
			this->field_0x5cc = 0;
		}
	}

	return;
}

CActorWindState* CActorJamGut::GetWindState()
{
	return &this->field_0x3b0;
}

float CActorJamGut::GetRunSpeed()
{
	return this->field_0x404;
}

void CActorJamGut::ClearLocalData()
{
	CBehaviourHeroRideJamGut* pRideJamGut;
	int iVar2;
	float fVar5;
	float fVar6;
	float fVar7;
	CActorHero* pHero;

	this->pRider = (CActorHero*)0x0;
	this->field_0x354 = (CActorHero*)0x0;

	this->field_0x444 = this->field_0x404;
	this->field_0x448 = this->field_0x410;

	this->field_0x440 = 0.0f;
	this->field_0x420 = 0;
	this->field_0x460 = 0.0f;
	this->field_0x4ec = 0;

	this->field_0x450 = this->currentLocation;

	if ((this->flags_0x358 & 1) == 0) {
		this->field_0x4f0 = 0.6f;
	}
	else {
		this->field_0x4f0 = 0.4f;
	}

	this->field_0x430 = this->rotationQuat;
	
	this->field_0x378 = -1.0f;
	this->field_0x37c = -1.0f;
	this->field_0x380 = 0.0f;
	this->field_0x384 = 0x1b;

	this->field_0x390.x = 0.0f;
	this->field_0x390.y = 1.0f;
	this->field_0x390.z = 0.0f;
	this->field_0x390.w = 0.0f;

	this->field_0x3a0 = 1;
	this->field_0x3a4 = 0.0f;
	this->field_0x3a8 = 0.0f;
	this->lightConfig.lightAmbient = gF32Vector4Zero;
	this->lightConfig.lightDirections = gF32Matrix4Unit;
	this->lightConfig.lightColorMatrix = gF32Matrix4Unit;

	(this->lightConfig).config.pLightAmbient = &this->lightConfig.lightAmbient;
	(this->lightConfig).config.pLightDirections = &(this->lightConfig).lightDirections;
	(this->lightConfig).config.pLightColorMatrix = &(this->lightConfig).lightColorMatrix;

	pHero = this->pRider;
	if ((pHero != (CActorHero*)0x0) && (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pRideJamGut = (CBehaviourHeroRideJamGut*)
			pHero->GetBehaviour(pHero->curBehaviourId);
		IMPLEMENTATION_GUARD(
		(*((pRideJamGut->base).pVTable)->IsInvincible)(2.0f, (CBehaviourRideJamGut*)pRideJamGut, 0);)
	}

	this->field_0x4fc = Timer::GetTimer()->scaledTotalTime;
	this->field_0x500 = 0;
	this->field_0xf0 = 10.0f;
	this->field_0x4f4 = this->currentLocation.y + 20.0f;
	//*(undefined4*)&this->field_0x4f8 = 0;
	this->dynamicExt.field_0xc = 1.3f;
	this->dynamicExt.field_0x1c = 0.9f;
	ResetStdSettings();
	this->activePathIndex = -1;
	this->field_0x64c = 0.0f;
	this->pathFollowIntensity = 1.0f;
	this->pathFollowCmdWalk = 0;
	this->pathFollowCmdRun = 1;
	this->pathTimeOnSegment = 0.0f;
	this->pathFollowDirection = this->rotationQuat;
	this->pTrackedPathActor = (CActor*)0x0;
	this->field_0x61c = 0;
	this->field_0x35c = 0;
	this->field_0x360 = 1;
	this->field_0x364 = 0;

	return;
}

void CActorJamGut::ManagePaths()
{
	CPathFollow* pCVar1;
	CEventManager* pEventManager;
	CBehaviourHeroRideJamGut* pCVar4;
	StateConfig* pSVar3;
	CActor* pPathActor;
	CBehaviourHeroRideJamGut* pCVar6;
	uint uVar7;
	CPathPlane* pPathPlane;
	int iVar10;
	S_JMG_PATH* pCurPath;
	int peVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	S_PATHREADER_POS_INFO pathreaderPosInfo;
	float local_8;
	float local_4;
	CActorHero* pHero;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pHero = this->pRider;
	if (pHero == (CActorHero*)0x0) {
		iVar10 = 0;
	}
	else {
		iVar10 = 0;
		if (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT) {
			pCVar4 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
			iVar10 = pCVar4->field_0x7c;
		}
	}

	if ((iVar10 == 0) && (pHero = this->pRider, pHero != (CActorHero*)0x0)) {
		if (((((pHero->GetStateFlags(pHero->actorState) & 0x100) != 0) && (pHero = this->pRider, ((pHero->pCollisionData)->flags_0x4 & COLLISION_GROUND_FLAG) != 0)) &&
			(pHero->pTiedActor == (CActor*)0x0)) &&
			(pPathActor = pHero->GetCollidingActor(), pPathActor == (CActor*)0x0)) {
			CBehaviourJamGutRidden* pRidden = static_cast<CBehaviourJamGutRidden*>(GetBehaviour(this->curBehaviourId));
			pRidden->field_0x8 = 1;
			JAMGUT_LOG(LogLevel::Info, "CActorJamGut::ManagePaths: hero lost footing mid-path, returning to stand");
			SetBehaviour(JAMGUT_BEHAVIOUR_STAND, -1, -1);
			this->activePathIndex = -1;
			this->pTrackedPathActor = (CActor*)0x0;
			return;
		}
	}

	pHero = this->pRider;
	iVar10 = 0;
	if ((pHero != (CActorHero*)0x0) && (iVar10 = 0x0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pCVar6 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		iVar10 = pCVar6->field_0x7c;
	}

	if ((iVar10 == 0) && (this->activePathIndex == -1)) {
		this->activePathIndex = -1;
		this->pTrackedPathActor = (CActor*)0x0;
	}
	else {
		if ((this->pRider == (CActorHero*)0x0) ||
			(((this->pRider->curBehaviourId != HERO_BEHAVIOUR_RIDE_JAMGUT || (iVar10 = this->actorState, iVar10 == JAMGUT_RIDE_STATE_RUN)) || (iVar10 == 9)))) {
			pCurPath = this->aPaths;

			for (iVar10 = 0; iVar10 < this->nbPaths; iVar10 = iVar10 + 1) {
				uVar7 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, (pCurPath->zoneRef).Get(), &this->currentLocation, 0);
				if ((uVar7 & 1) != 0) {
					pPathPlane = &pCurPath->pathPlane;

					if (this->activePathIndex != iVar10) {
						pPathPlane->InitTargetPos(&this->currentLocation, &(pCurPath->pathPlane).outData);
						float pathDist = ComputeDistanceToPath(&this->currentLocation, pPathPlane, (pCurPath->pathPlane).outData.field_0x0, &this->pathFollowDirection, 0, &local_4);
						this->field_0x64c = pathDist;

						fVar13 = edF32Vector4DotProductHard(&this->pathFollowDirection, &this->dynamic.horizontalVelocityDirectionEuler);
						if (fVar13 < 0.0f) {
							JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: path {} rejected, moving against path direction (dot={:.2f})", iVar10, fVar13);
							return;
						}

						if (pCurPath->field_0x2c < fabs(this->field_0x64c)) {
							JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: path {} rejected, lateral distance {:.2f} exceeds max {:.2f}", iVar10, fabs(this->field_0x64c), pCurPath->field_0x2c);
							return;
						}

						pathDist = pCurPath->field_0x28;
						if (fabs(this->field_0x64c) < pathDist) {
							if (0.0f < this->field_0x64c) {
								this->field_0x64c = pathDist;
							}
							else {
								this->field_0x64c = -pathDist;
							}
						}

						pathDist = pCurPath->field_0x30;
						if (pCurPath->field_0x28 < fabs(pathDist)) {
							if (0.0f < this->field_0x64c) {
								this->field_0x64c = pathDist;
							}
							else {
								this->field_0x64c = -pathDist;
							}
						}

						pPathActor = (pCurPath->actorRef).Get();
						if (pPathActor == (CActor*)0x0) {
							peVar12 = (pCurPath->pathPlane).outData.field_0x0;
							if (peVar12 == -1) {
								peVar12 = 0x0;
							}

							pathreaderPosInfo.prevSegment = pPathPlane->pathFollowReader.GetNextPlace(peVar12, 1);
							pathreaderPosInfo.currentSegment = peVar12;
							if (pathreaderPosInfo.prevSegment == -1) {
								pathreaderPosInfo.currentSegment = peVar12 - 1;
								pathreaderPosInfo.prevSegment = peVar12;
							}

							pathreaderPosInfo.segmentFraction = local_4;
							fVar13 = pCurPath->pathFollowReader.GetTimeOnSegment(&pathreaderPosInfo);
							this->pathTimeOnSegment = fVar13;
						}
						else {
							if (pPathActor->curBehaviourId != MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY) {
								return;
							}

							this->pTrackedPathActor = pPathActor;
							JAMGUT_LOG(LogLevel::Info, "CActorJamGut::ManagePaths: path {} tracking actor {:p}", iVar10, static_cast<void*>(pPathActor));
							this->aPaths[iVar10].pathPlane.InitTargetPos(&this->pTrackedPathActor->currentLocation, &this->pathPlaneOutData);
						}

						if (this->field_0x404 < this->dynamic.linearAcceleration) {
							JAMGUT_LOG(LogLevel::Info, "CActorJamGut::ManagePaths: path {} activation clamping speed: dynamic.speed={:.2f} field_0x444={:.2f} -> runSpeed={:.2f}",
								iVar10, this->dynamic.speed, this->field_0x444, this->field_0x404);
							this->dynamic.speed = this->field_0x404;
							this->field_0x444 = this->field_0x404;
						}

						this->activePathIndex = iVar10;
						JAMGUT_LOG(LogLevel::Info, "CActorJamGut::ManagePaths: path {} activated, timeOnSegment={:.2f}", iVar10, this->pathTimeOnSegment);
					}

					if (this->pTrackedPathActor == (CActor*)0x0) {
						pCurPath->pathFollowReader.ComputePosition(this->pathTimeOnSegment, &eStack48, (edF32VECTOR4*)0x0, (S_PATHREADER_POS_INFO*)0x0);
						this->pathTimeOnSegment = this->pathTimeOnSegment + GetTimer()->cutsceneDeltaTime;

						fVar13 = (pCurPath->pathFollowReader).field_0x4;
						if (fVar13 < this->pathTimeOnSegment) {
							fVar13 = fmodf(this->pathTimeOnSegment, fVar13);
							this->pathTimeOnSegment = fVar13;
						}
					}
					else {
						JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: tracked actor pos=({:.4f},{:.4f},{:.4f})",
							this->pTrackedPathActor->currentLocation.x,
							this->pTrackedPathActor->currentLocation.y,
							this->pTrackedPathActor->currentLocation.z);
						pPathPlane->ExternComputeTargetPosWithPlane(&this->pTrackedPathActor->currentLocation, &this->pathPlaneOutData);
						ComputeDistanceToPath(&this->pTrackedPathActor->currentLocation, pPathPlane, this->pathPlaneOutData.field_0x0, 0, &eStack48, &local_8);
						peVar12 = this->pathPlaneOutData.field_0x0;
						if (peVar12 == -1) {
							peVar12 = 0;
						}

						pathreaderPosInfo.prevSegment = pPathPlane->pathFollowReader.GetNextPlace(peVar12, 1);
						pathreaderPosInfo.currentSegment = peVar12;
						if (pathreaderPosInfo.prevSegment == -1) {
							pathreaderPosInfo.currentSegment = peVar12 - 1;
							pathreaderPosInfo.prevSegment = peVar12;
						}

						pathreaderPosInfo.segmentFraction = local_8;
						if (1.0f < local_8) {
							pathreaderPosInfo.segmentFraction = 1.0f;
						}

						fVar13 = pCurPath->pathFollowReader.GetTimeOnSegment(&pathreaderPosInfo);
						JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: tracked actor pathTimeOnSegment={:.4f} seg={} nextSeg={} t={:.4f}",
							fVar13, pathreaderPosInfo.currentSegment, pathreaderPosInfo.prevSegment, pathreaderPosInfo.segmentFraction);
						this->pathTimeOnSegment = fVar13;
					}

					pPathPlane->ExternComputeTargetPosWithPlane(&this->currentLocation, &(pCurPath->pathPlane).outData);
					ComputeDistanceToPath(&this->currentLocation, pPathPlane, (pCurPath->pathPlane).outData.field_0x0, &local_70, &eStack64, &local_4);
					peVar12 = (pCurPath->pathPlane).outData.field_0x0;
					if (peVar12 == -1) {
						peVar12 = 0;
					}

					pathreaderPosInfo.prevSegment = pPathPlane->pathFollowReader.GetNextPlace(peVar12, 1);
					pathreaderPosInfo.currentSegment = peVar12;
					if (pathreaderPosInfo.prevSegment == -1) {
						pathreaderPosInfo.currentSegment = peVar12 - 1;
						pathreaderPosInfo.prevSegment = peVar12;
					}

					pathreaderPosInfo.segmentFraction = local_4;
					if (1.0f < local_4) {
						pathreaderPosInfo.segmentFraction = 1.0f;
					}

					fVar13 = pCurPath->pathFollowReader.GetTimeOnSegment(&pathreaderPosInfo);
					pCVar1 = (pCurPath->pathFollowReader).pPathFollow;
					if (((pCVar1->mode == 0) && (pCVar1->type == 0)) &&
						(fabs(fVar13 - (pCurPath->pathFollowReader).field_0x4) < 0.1f)) {
						pHero = this->pRider;
						iVar10 = 0;
						if ((pHero != (CActorHero*)0x0) && (iVar10 = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
							pCVar6 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
							iVar10 = pCVar6->field_0x7c;
						}

						if (iVar10 == 0) {
							pCVar6 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
							pCVar6->field_0x8 = 1;
							JAMGUT_LOG(LogLevel::Info, "CActorJamGut::ManagePaths: path {} reached end, returning to stand behaviour", this->activePathIndex);
							SetBehaviour(JAMGUT_BEHAVIOUR_STAND, -1, -1);
						}
						else {
							JAMGUT_LOG(LogLevel::Info, "CActorJamGut::ManagePaths: path {} reached end, advancing rider to next state", this->activePathIndex);
							SetState(JAMGUT_RIDE_STATE_STAND, -1);

							pCVar6 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
							pCVar6->ResetCommands();
							pHero = this->pRider;
							if ((pHero != (CActorHero*)0x0) &&
								(pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
								pCVar6 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
								pCVar6->SetState(0x131, 1);
							}
						}

						this->activePathIndex = -1;
						this->pTrackedPathActor = (CActor*)0x0;

						return;
					}

					local_60.y = 0.0f;
					local_60.w = 0.0f;
					local_60.x = -local_70.z;
					local_60.z = local_70.x;

					edF32Vector4ScaleHard(this->field_0x64c, &local_60, &local_60);
					fVar14 = edFIntervalUnitSrcLERP(local_4, 2.0f, 6.0f);
					edF32Vector4ScaleHard(fVar14, &local_70, &local_70);
					edF32Vector4AddHard(&eStack64, &eStack64, &local_60);
					edF32Vector4AddHard(&eStack64, &eStack64, &local_70);
					edF32Vector4SubHard(&local_50, &eStack64, &this->currentLocation);
					local_50.y = 0.0f;
					edF32Vector4SafeNormalize1Hard(&local_50, &local_50);
					fVar16 = (pCurPath->pathFollowReader).field_0x4;
					fVar14 = this->pathTimeOnSegment;
					fVar15 = fVar16 * 0.25f;
					if ((fVar15 <= fVar14) || (fVar13 <= fVar16 - fVar15)) {
						if ((fVar13 < fVar15) && (fVar16 - fVar15 < fVar14)) {
							// JamGut near path start, target near path end: JamGut has wrapped ahead
							fVar13 = fVar13 + fVar16;
						}
					}
					else {
						// Target near start (just wrapped), JamGut near end: target is conceptually past end
						fVar14 = fVar14 + fVar16;
					}

					const float fJamGutTime = fVar13;
					const float fTargetTime = fVar14;
					if (fVar13 < fVar14) {
						this->pathFollowCmdRun = 1;
						this->pathFollowCmdWalk = 0;
						fVar13 = edFIntervalLERP(this->field_0x404 * (fVar14 - fVar13), 0.0f, 3.0f, 1.0f, 1.5f);
						JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: BEHIND target (jamgut={:.2f} target={:.2f}), intensity={:.2f} (boost)", fJamGutTime, fTargetTime, fVar13);
					}
					else {
						this->pathFollowCmdRun = 0;
						this->pathFollowCmdWalk = 1;
						fVar13 = edFIntervalLERP(this->field_0x404 * (fVar13 - fVar14), 0.0f, 3.0f, 1.0f, 0.5f);
						JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: AHEAD of target (jamgut={:.2f} target={:.2f}), intensity={:.2f} (slowdown)", fJamGutTime, fTargetTime, fVar13);
					}

					this->pathFollowDirection = local_50;
					this->pathFollowIntensity = fVar13;
					JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: targetSpeed={:.2f} (runSpeed={:.2f} * intensity={:.2f})",
						this->field_0x404 * fVar13, this->field_0x404, fVar13);

					return;
				}

				pCurPath = pCurPath + 1;
			}

			JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: no path zone active, clearing active path");
			this->activePathIndex = -1;
			this->pTrackedPathActor = (CActor*)0x0;
		}
		else {
			JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::ManagePaths: rider condition not met, clearing active path");
			this->activePathIndex = -1;
			this->pTrackedPathActor = (CActor*)0x0;
		}
	}

	return;
}

void CActorJamGut::ManageCheckpoints()
{
	ed_zone_3d* pZone;
	CWayPoint* pWayPoint;
	int iVar2;
	StateConfig* pSVar3;
	uint uVar4;
	int curIndex;
	S_JMG_CHECKPOINT* pCurCheckpoint;
	float fVar6;
	float fVar7;
	CEventManager* pEventManager;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pCurCheckpoint = this->aCheckpoints;
	curIndex = 0;
	if (0 < this->nbCheckpoints) {
		do {
			pZone = (pCurCheckpoint->zoneRef).Get();

			if ((((pZone != (ed_zone_3d*)0x0) && ((pCurCheckpoint->waypointRef).Get() != (CWayPoint*)0x0)) &&
				(edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &this->currentLocation, 0) != 2)) &&
				(this->field_0x618 != curIndex)) {
				if ((GetStateFlags(this->actorState) & 1) == 0) {
					uVar4 = GetStateFlags(this->actorState) & 0x100;
				}
				else {
					uVar4 = 0;
				}

				if (uVar4 != 0) {
					this->field_0x618 = curIndex;
					pWayPoint = (pCurCheckpoint->waypointRef).Get();
					JAMGUT_LOG(LogLevel::Info, "CActorJamGut::ManageCheckpoints: checkpoint {} activated, waypoint pos ({:.2f}, {:.2f}, {:.2f})",
						curIndex, pWayPoint->location.x, pWayPoint->location.y, pWayPoint->location.z);
					this->field_0x5f0.xyz = pWayPoint->location;
					this->field_0x5f0.w = 1.0f;

					this->field_0x600 = pWayPoint->rotation;
					this->field_0x60c = 0;
					this->field_0x61c = 0xb4;

					return;
				}
			}

			curIndex = curIndex + 1;
			pCurCheckpoint = pCurCheckpoint + 1;
		} while (curIndex < this->nbCheckpoints);
	}

	return;
}

float CActorJamGut::ComputeDistanceToPath(edF32VECTOR4* param_2, CPathPlane* param_3, int param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, float* param_7)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* v2;
	int index;
	float fVar2;
	float t;
	float fVar3;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	int local_4;

	if (param_4 == -1) {
		param_4 = 0;
	}

	local_4 = param_3->pathFollowReader.GetNextPlace(param_4, 1);
	index = param_4;
	if (local_4 == -1) {
		index = param_4 + -1;
		local_4 = param_4;
	}

	peVar1 = param_3->pathFollowReader.GetWayPoint(local_4);
	v2 = param_3->pathFollowReader.GetWayPoint(index);

	edF32Vector4SubHard(&local_20, peVar1, v2);
	local_20.y = 0.0f;
	if (param_5 == (edF32VECTOR4*)0x0) {
		fVar2 = edF32Vector4GetDistHard(&local_20);
	}
	else {
		fVar2 = edF32Vector4SafeNormalize1Hard(param_5, &local_20);
	}

	local_40.x = -local_20.z;
	local_40.y = local_20.y;
	local_40.z = local_20.x;
	edF32Vector4NormalizeHard(&local_40, &local_40);
	peVar1 = param_3->pathFollowReader.GetWayPoint(index);
	edF32Vector4SubHard(&eStack48, param_2, peVar1);
	eStack48.y = 0.0f;
	t = edF32Vector4DotProductHard(&eStack48, &local_40);
	if ((param_6 != (edF32VECTOR4*)0x0) || (param_7 != (float*)0x0)) {
		edF32Vector4ScaleHard(t, &local_40, &local_40);
		edF32Vector4SubHard(&eStack48, &eStack48, &local_40);
		fVar3 = edF32Vector4DotProductHard(&eStack48, &local_20);
		if (fVar3 < 0.0f) {
			if (param_6 != (edF32VECTOR4*)0x0) {
				peVar1 = param_3->pathFollowReader.GetWayPoint(index);
				param_6->x = peVar1->x;
				peVar1 = param_3->pathFollowReader.GetWayPoint(index);
				param_6->y = peVar1->y;
				peVar1 = param_3->pathFollowReader.GetWayPoint(index);
				param_6->z = peVar1->z;
				peVar1 = param_3->pathFollowReader.GetWayPoint(index);
				param_6->w = peVar1->w;
			}

			if (param_7 != (float*)0x0) {
				*param_7 = 0.0f;
			}
		}
		else {
			if (param_7 != (float*)0x0) {
				fVar3 = edF32Vector4GetDistHard(&eStack48);
				*param_7 = fVar3 / fVar2;
			}

			if (param_6 != (edF32VECTOR4*)0x0) {
				if (1.0f < *param_7) {
					peVar1 = param_3->pathFollowReader.GetWayPoint(local_4);
					param_6->x = peVar1->x;
					peVar1 = param_3->pathFollowReader.GetWayPoint(local_4);
					param_6->y = peVar1->y;
					peVar1 = param_3->pathFollowReader.GetWayPoint(local_4);
					param_6->z = peVar1->z;
					peVar1 = param_3->pathFollowReader.GetWayPoint(local_4);
					param_6->w = peVar1->w;
				}
				else {
					peVar1 = param_3->pathFollowReader.GetWayPoint(index);
					edF32Vector4AddHard(param_6, &eStack48, peVar1);
				}
			}
		}
	}

	return t;
}

void CActorJamGut::ResetStdSettings()
{
	this->field_0x418 = 1.2f;
	this->field_0x4a0 = 20.0f;
	this->field_0x4a8 = 0.5f;
	this->field_0x4ac = 0.18f;
	this->field_0x4b0 = 0.1f;
	this->field_0x4b4 = 1.5f;

	SetJumpCfg(0.1f, this->field_0x404, this->field_0x4a8, this->field_0x4a0, this->field_0x4a4, 1, (edF32VECTOR4*)0x0);

	return;
}

void CActorJamGut::SetJumpCfg(float param_1, float param_2, float param_3, float param_4, float param_5,
	int param_7, edF32VECTOR4* param_8)
{
	if (0.0f <= param_1) {
		this->field_0x480 = param_1;
	}

	if (0.0f <= param_2) {
		this->field_0x484 = param_2;
	}

	if (0.0f <= param_3) {
		this->field_0x488 = param_3;
	}

	if (0.0f <= param_4) {
		this->field_0x48c = param_4;
	}

	if (0.0f <= param_5) {
		this->field_0x490 = param_5;
	}

	this->field_0x494 = param_7;
	if (param_8 == (edF32VECTOR4*)0x0) {
		(this->field_0x470).x = 0.0f;
		(this->field_0x470).y = 1.0f;
		(this->field_0x470).z = 0.0f;
		(this->field_0x470).w = 0.0f;
	}
	else {
		this->field_0x470 = *param_8;
	}

	return;
}

int CActorJamGut::GetHeroAction(float param_1, CActorHero* pHero)
{
	float fVar1;
	int heroAction;
	float fVar3;
	float fVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	if (GetBehaviour(JAMGUT_BEHAVIOUR_RIDDEN) == (CBehaviour*)0x0) {
		heroAction = 0;
	}
	else {
		if ((this->pRider == (CActorHero*)0x0) || (this->pRider->curBehaviourId != HERO_BEHAVIOUR_RIDE_JAMGUT)) {
			int curState = this->actorState;
			if ((curState == 0x17) || (curState == 0x18)) {
				heroAction = 0;
			}
			else {
				edF32Vector4SubHard(&eStack16, &this->currentLocation, &pHero->currentLocation);
				fVar1 = eStack16.y;
				eStack16.y = 0.0f;
				edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
				fVar3 = (pHero->currentLocation).x - this->currentLocation.x;
				fVar4 = (pHero->currentLocation).z - this->currentLocation.z;
				fVar3 = fVar3 * fVar3 + fVar4 * fVar4;
				if (((0.5f <= fVar1) || (fVar1 <= -3.0f)) || (param_1 <= fVar3)) {
					heroAction = 0;
				}
				else {
					fVar3 = edFIntervalLERP(fVar3, 0.4f, param_1, -1.0f, -1.0f);
					fVar4 = edF32Vector4DotProductHard(&eStack16, &pHero->rotationQuat);
					heroAction = 0;
					if ((fVar3 < fVar4) && ((SV_GetBoneDefaultWorldPosition(this->field_0x374, &eStack32),
							-0.1f <= pHero->dynamic.linearAcceleration * pHero->dynamic.velocityDirectionEuler.y ||
							(heroAction = 0, eStack32.y <= (pHero->currentLocation).y)))) {
						heroAction = 0xd;
					}
				}
			}
		}
		else {
			heroAction = 0;
		}
	}

	return heroAction;
}

void CActorJamGut::SetHitInvincibility(float duration, bool bAdd)
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;

	pHero = this->pRider;
	if ((pHero != (CActorHero*)0x0) && (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		pHeroRideJamGut->SetInvincible(duration, bAdd);
	}

	if (bAdd == false) {
		this->field_0x4fc = Timer::GetTimer()->scaledTotalTime;
		this->field_0x500 = 0;
	}
	else {
		this->field_0x4fc = duration + Timer::GetTimer()->scaledTotalTime;
	}

	return;
}

int CActorJamGut::AccomplishHit(CActorAutonomous* pActor, CActor* pSender, _msg_hit_param* pParams, edF32VECTOR4* param_5)
{
	CPlayerInput* pPlayerInput;
	CLifeInterface* pLifeInterface;
	int iVar4;
	float fVar5;
	float fVar6;

	iVar4 = -1;
	if (((pActor != (CActor*)0x0) && (this->field_0x4fc <= Timer::GetTimer()->scaledTotalTime)) && (pActor->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pPlayerInput = GetInputManager(0, 0);
		if (pPlayerInput != (CPlayerInput*)0x0) {
	CPlayerInput::FUN_001b66f0(1.0f, 0.0f, 0.1f, 0.0f, &pPlayerInput->field_0x1c, 0);
		}

		pActor->LifeDecrease(pParams->damage);
	}
	if (pActor != (CActor*)0x0) {
		fVar5 = pActor->GetLifeInterface()->GetValue();
		if (fVar5 == 0.0f) {
			iVar4 = pParams->projectileType;
			SetHitInvincibility(2.0f, 0);
			if (iVar4 == 5) {
				iVar4 = 0x14;
			}
			else {
				iVar4 = -1;
			}
		}
	}

	if (iVar4 == -1) {
		if (((pSender == (CActor*)0x0) || (pParams->projectileType == 9)) ||
			((pParams->projectileType == 2 ||
				(this->field_0x4fc <= Timer::GetTimer()->scaledTotalTime)))) {
			fVar6 = Timer::GetTimer()->scaledTotalTime;
			fVar5 = this->field_0x4fc;
			if (((pActor != (CActor*)0x0) && (0.0f < pParams->damage)) &&
				(this->field_0x4fc <= Timer::GetTimer()->scaledTotalTime)) {
				SetHitInvincibility(2.0f, 1);
			}
			if ((pParams->projectileType == 0) && (pParams->flags == 0)) {
				if (pActor != (CActor*)0x0) {
					fVar5 = pActor->GetLifeInterface()->GetValue();
					if (fVar5 == 0.0f) {
						SetHitInvincibility(2.0f, 0);
						SetState(0x13, -1);
					}
				}
			}
			else {
				iVar4 = ChooseStateHit(pSender, pParams, param_5, (fVar6 < fVar5) ^ 1);
				if (iVar4 == this->actorState) {
					fVar5 = pActor->GetLifeInterface()->GetValue();
					if (0.0f < fVar5) {
						return 1;
					}
				}

				SetState(iVar4, -1);
			}
		}
	}
	else {
		SetState(iVar4, -1);
	}

	return 1;
}

uint CActorJamGut::GetStateRider(int state)
{
	uint uVar1;

	uVar1 = 0xffffffff;

	if ((state != -1) && (5 < state)) {
		uVar1 = gStateCfg_JMG[state + -6].field_0x8;
	}

	return uVar1;
}

int CActorJamGut::ChooseStateDead(int param_2, int param_3, int param_4)
{
	int deadState;

	IMPLEMENTATION_GUARD(
	FUN_00366500(2.0f, 0);)
	if (param_4 == 0) {
		if (param_3 == 1) {
			deadState = 0x13;
		}
		else {
			deadState = -1;
			if (param_2 == 5) {
				deadState = 0x14;
			}
		}
	}
	else {
		deadState = 0x13;
	}

	return deadState;
}

int CActorJamGut::ChooseStateWind(float param_1, edF32VECTOR4* param_3)
{
	int iVar1;
	bool bVar2;

	iVar1 = this->actorState;
	if (iVar1 != 0x10) {
		if ((GetStateFlags(this->actorState) & 1) == 0) {
			if (GetWindState() == (CActorWindState*)0x0) {
				bVar2 = false;
			}
			else {
				iVar1 = GetWindState()->nbActiveWind;
				if (iVar1 == GetWindState()->nbWindWaypoint) {
					bVar2 = true;
				}
				else {
					if (GetWindState()->nbWindWaypoint == 0) {
						bVar2 = false;
					}
					else {
						bVar2 = true;
						if (0.17398384f <= fabs(param_3->y)) {
							bVar2 = false;
						}
					}
				}
			}

			if ((!bVar2) && (0.001f < param_1)) {
				return 0x17;
			}

			if (GetWindState() == (CActorWindState*)0x0) {
				bVar2 = false;
			}
			else {
				iVar1 = GetWindState()->nbActiveWind;
				if (iVar1 == GetWindState()->nbWindWaypoint) {
					bVar2 = true;
				}
				else {
					if (GetWindState()->nbWindWaypoint == 0) {
						bVar2 = false;
					}
					else {
						bVar2 = true;
						if (0.17398384f <= fabs(param_3->y)) {
							bVar2 = false;
						}
					}
				}
			}

			if ((!bVar2) || (bVar2 = true, param_1 <= 2.0f)) {
				bVar2 = false;
			}

			if (bVar2) {
				return 0x18;
			}

			return -1;
		}
	}

	return -1;
}

bool CActorJamGut::CheckHitAndDeath(CActorHero* pHero)
{
	bool bHit;
	uint uVar1;
	_msg_hit_param msgHitParam;
	edF32VECTOR4 local_10;
	CCollision* pCol;

	bHit = ColWithLava();
	if ((bHit == false) && (bHit = ColWithCactus(), bHit == false)) {
		bHit = false;
	}
	else {
		pCol = this->pCollisionData;
		bHit = ColWithCactus();
		if (bHit == false) {
			local_10.x = 0.0f;
			local_10.y = 0.0f;
			local_10.z = 0.0f;
			local_10.w = 0.0f;

			uVar1 = pCol->aCollisionContact[1].materialFlags & 0xf;
			if (uVar1 == 0) {
				uVar1 = CScene::_pinstance->defaultMaterialIndex;
			}

			if (uVar1 == 3) {
				edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)(pCol->aCollisionContact + 1));
			}

			uVar1 = pCol->aCollisionContact[0].materialFlags & 0xf;
			if (uVar1 == 0) {
				uVar1 = CScene::_pinstance->defaultMaterialIndex;
			}
			if (uVar1 == 3) {
				edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)pCol->aCollisionContact);
			}

			uVar1 = pCol->aCollisionContact[2].materialFlags & 0xf;
			if (uVar1 == 0) {
				uVar1 = CScene::_pinstance->defaultMaterialIndex;
			}

			if (uVar1 == 3) {
				edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)(pCol->aCollisionContact + 2));
			}

			edF32Vector4NormalizeHard(&local_10, &local_10);
			msgHitParam.damage = 2.0f;
		}
		else {
			local_10.x = 0.0f;
			local_10.y = 0.0f;
			local_10.z = 0.0f;
			local_10.w = 0.0f;

			uVar1 = pCol->aCollisionContact[1].materialFlags & 0xf;
			if (uVar1 == 0) {
				uVar1 = CScene::_pinstance->defaultMaterialIndex;
			}

			if (uVar1 == 7) {
				edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)(pCol->aCollisionContact + 1));
			}

			uVar1 = pCol->aCollisionContact[0].materialFlags & 0xf;
			if (uVar1 == 0) {
				uVar1 = CScene::_pinstance->defaultMaterialIndex;
			}

			if (uVar1 == 7) {
				edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)pCol->aCollisionContact);
			}

			uVar1 = pCol->aCollisionContact[2].materialFlags & 0xf;
			if (uVar1 == 0) {
				uVar1 = CScene::_pinstance->defaultMaterialIndex;
			}

			if (uVar1 == 7) {
				edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)(pCol->aCollisionContact + 2));
			}

			edF32Vector4NormalizeHard(&local_10, &local_10);
			if (pHero == (CActorHero*)0x0) {
				SetState(0x13, -1);
				return true;
			}

			pHero->field_0x2e4 = 10.0f;
			msgHitParam.damage = 0.0f;
		}

		msgHitParam.flags = 1;
		msgHitParam.projectileType = 0;
		AccomplishHit(pHero, (CActor*)0x0, &msgHitParam, &local_10);
		bHit = true;
	}

	return bHit;
}

int CActorJamGut::ChooseStateHit(CActor* pSender, _msg_hit_param* param_3, edF32VECTOR4* param_4, bool param_5)
{
	CCollision* pCol;
	bool bVar2;
	bool bVar3;
	int iVar4;
	edF32VECTOR4* v0;
	float fVar6;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	iVar4 = param_3->projectileType;
	if (iVar4 == 5) {
		iVar4 = 0x12;
	}
	else {
		bVar3 = true;
		bVar2 = true;

		if (iVar4 == 9) {
			edF32Vector4SubHard(&local_10, &this->currentLocation, &pSender->currentLocation);
			local_10.y = 0.0f;
			edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
			fVar6 = edF32Vector4DotProductHard(&this->dynamic.horizontalVelocityDirectionEuler, &local_10);
			bVar2 = fVar6 <= 0.0f;
			local_10.y = 1.0f;

			if (param_5 == false) {
				edF32Vector4ScaleHard(param_3->field_0x30, &local_10, &local_10);
			}
			else {
				edF32Vector4ScaleHard(param_3->field_0x30 * 2.0f, &local_10, &local_10);
			}
		}
		else {
			pCol = this->pCollisionData;
			if (pSender != (CActor*)0x0) {
				if ((iVar4 == 1) || (iVar4 == 2)) {
					if (iVar4 == 2) {
						param_3->field_0x30 = param_3->field_0x30 * 2.0f;
					}

					this->rotationQuat = param_3->field_0x20;
				}
				else {
					if (iVar4 != 10) {
						SV_SetOrientationToPosition2D(&pSender->currentLocation);
					}
				}
			}

			if ((param_4 == (edF32VECTOR4*)0x0) ||
				(CCollisionManager::_material_table[3].field_0x8 <= param_4->y)) {
				if (((iVar4 == 1) || (iVar4 == 2)) && (param_3->field_0x30 != 0.0f)) {
					edF32Vector4ScaleHard(-1.0f, &local_10, &this->rotationQuat);
					local_10.y = 1.0f;
					edF32Vector4ScaleHard(param_3->field_0x30, &local_10, &local_10);
				}
				else {
					if ((param_4 != (edF32VECTOR4*)0x0) && (param_4->y < 0.99984443f)) {
						edProjectVectorOnPlane(0.0f, &local_10, &CDynamicExt::gForceGravity, param_4, 1);
						local_10.y = 0.0f;
						edF32Vector4NormalizeHard(&local_10, &local_10);
						edF32Vector4ScaleHard(-1.0f, &local_10, &local_10);
						this->rotationQuat = local_10;
					}

					edF32Vector4ScaleHard(-1.0f, &local_10, &this->rotationQuat);
					local_10.y = 2.0f;
					edF32Vector4ScaleHard(200.0f, &local_10, &local_10);
				}
			}
			else {
				edProjectVectorOnPlane(0.0, &local_10, param_4, &gF32Vector4UnitY, 0);
				if ((0.001f < fabs(local_10.x)) || (0.001f < fabs(local_10.z))) {
					local_10.y = 0.0f;
					edF32Vector4NormalizeHard(&local_20, &local_10);
					edF32Vector4ScaleHard(-1.0f, &local_20, &local_20);
					this->rotationQuat = local_20;
				}

				if (-CCollisionManager::_material_table[3].field_0x8 < param_4->y) {
					local_10.y = 0.5f;
				}
				else {
					local_10.y = 0.2f;
					if ((pCol->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
						bVar3 = false;
					}
				}

				edF32Vector4ScaleHard(200.0f, &local_10, &local_10);
			}
		}
		if (bVar2) {
			this->dynamic.speed = 0.0f;
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;
		}

		if (bVar3) {
			edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack48, &local_10);
			v0 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(v0, v0, &eStack48);
			fVar6 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar6;
		}

		RestartCurAnim();
		iVar4 = 0x11;
	}

	return iVar4;
}

bool CActorJamGut::UpdateFunc(float inputFloat)
{
	edF32VECTOR4 eStack32;

	float puVar8 = edF32Vector4DotProductHard(&this->rotationQuat, &this->dynamic.horizontalVelocityDirectionEuler);
	this->field_0x44c = puVar8;
	if (1.0f < puVar8) {
		puVar8 = 1.0f;
	}
	else {
		if (puVar8 < -1.0f) {
			puVar8 = -1.0f;
		}
	}

	float fVar7 = acosf(puVar8);
	this->field_0x44c = fVar7;
	edF32Vector4ScaleHard(this->dynamic.horizontalLinearSpeed, &eStack32, &this->dynamic.horizontalVelocityDirectionEuler);
	fVar7 = edF32Vector4DotProductHard(&this->rotationQuat, &eStack32);
	float fVar8 = this->field_0x444;
	if (fVar7 < 1.0f) {
		this->field_0x440 = fVar8 - fVar7;
	}
	else {
		if (fVar8 == 0.0f) {
			this->field_0x440 = (-fVar8 * 0.5f) / fVar7;
		}
		else {
			this->field_0x440 = (fVar8 - fVar7) / fVar7;
		}
	}

	this->field_0x440 = this->field_0x440;
	int iVar3 = RiderCmdWalk();
	if (((iVar3 == 0) && (iVar3 = RiderCmdRun(), iVar3 == 0)) && (iVar3 = RiderCmdAttack(), iVar3 == 0)) {
		SV_MOV_UpdateSpeedIntensity(0.0f, inputFloat);
	}
	else {
		BuildHorizontalSpeedVector(this->field_0x444, 15.0f, this->field_0x448, 9.424778f, 2.094395f);
	}

	return ManageDynAndKillActors(0x1006023b);
}


void CActorJamGut::BehaviourJamGutStand_Manage(CBehaviourJamGutStand* pBehaviour)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;

	iVar1 = this->actorState;
	if ((((iVar1 != 0x17) && (iVar1 != 0x18)) && (iVar1 = ChooseStateWind(this->dynamicExt.aImpulseVelocityMagnitudes[2], this->dynamicExt.aImpulseVelocities + 2), iVar1 != -1)) && (iVar1 != this->actorState)) {
		SetState(iVar1, -1);
	}

	switch (this->actorState) {
	case JAMGUT_STAND_STATE_STAND:
		StateJamGutStand(0.0f, pBehaviour, 0, -1);
		break;
	case 9:
		IMPLEMENTATION_GUARD(
		StateJamGutWalk(pBehaviour, JAMGUT_STAND_STATE_STAND);)
		break;
	case JAMGUT_RIDE_STATE_RUN:
		StateJamGutRun(pBehaviour, JAMGUT_STAND_STATE_STAND);
		break;
	case JAMGUT_RIDE_STATE_JUMP:
		StateJamGutJump();
		break;
	case JAMGUT_RIDE_STATE_JUMP_AFTER:
		StateJamGutJumpAfter(pBehaviour, JAMGUT_STAND_STATE_STAND);
		break;
	case JAMGUT_RIDE_STATE_JUMP_AFTER_B:
		StateJamGutJumpAfter(pBehaviour, JAMGUT_STAND_STATE_STAND);
		break;
	case JAMGUT_RIDE_STATE_FALL:
		StateJamGutFall();
		break;
	case 0x11:
		IMPLEMENTATION_GUARD(
		StateJamGutHit();)
		break;
	case 0x12:
		IMPLEMENTATION_GUARD(
		StateJamGutDuration(-1.0f, pBehaviour, JAMGUT_STAND_STATE_STAND);)
		break;
	case 0x13:
		StateJamGutDead(-1.0f, pBehaviour, 0, 0);
		break;
	case 0x15:
		StateJamGutDead(2.0f, pBehaviour, 1, 0);
		break;
	case 0x16:
		StateJamGutDead(2.0f, pBehaviour, 0, 0);
		break;
	case 0x17:
		IMPLEMENTATION_GUARD(
		FUN_00373860();)
		break;
	case 0x18:
		IMPLEMENTATION_GUARD(
		StateJamGutWindSlide();)
		break;
	case JAMGUT_STAND_STATE_REJECT_HERO_A:
		StateJamGutStand(0.625f, pBehaviour, this->field_0x354, JAMGUT_STAND_STATE_REJECT_HERO_B);
		break;
	case JAMGUT_STAND_STATE_REJECT_HERO_B:
		StateJamGutStand(0.4f, pBehaviour, 0, JAMGUT_STAND_STATE_STAND);
	}

	if (((GetStateFlags(this->actorState) & 1) == 0) && (this->actorState != 0x10)) {
		CheckHitAndDeath(this->pRider);
	}

	return;
}


void CActorJamGut::BehaviourJamGutRidden_Manage(CBehaviourJamGutRidden* pBehaviour)
{
	CNewFx* pCVar1;
	CActorHero* pHero;
	bool bVar3;
	CBehaviourHeroRideJamGut* pCVar6;
	CBehaviourHeroRideJamGut* pCVar5;
	CBehaviourHeroRideJamGut* pCVar4;
	int iVar4;
	CBehaviourHeroRideJamGut* pCVar7;
	CBehaviourHeroRideJamGut* pCVar8;
	CBehaviourHeroRideJamGut* pCVar9;
	CBehaviourHeroRideJamGut* pCVar10;
	StateConfig* pSVar5;
	CBehaviourHeroRideJamGut* pCVar11;
	int iVar6;
	uint uVar7;

	iVar4 = RiderCmdAttack();

	if (((iVar4 != 0) && (this->field_0x364 == 0)) && ((this->field_0x368).type != 0xffffffff)) {
		if (this->field_0x368.IsValid()) {
			this->field_0x368.Kill();
			this->field_0x368.Reset();
		}

		SV_FX_Start(&this->field_0x368);

		this->field_0x368.Start();
	}

	iVar4 = RiderCmdAttack();

	this->field_0x364 = iVar4;
	pHero = this->pRider;
	if ((pHero != (CActorHero*)0x0) &&
		(pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pCVar4 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		pCVar4->field_0x10 = this->dynamic.field_0x10;
	}

	iVar4 = ChooseStateWind(this->dynamicExt.aImpulseVelocityMagnitudes[2], this->dynamicExt.aImpulseVelocities + 2);
	switch (this->actorState) {
	case JAMGUT_RIDE_STATE_STAND:
		StateJamGutRideStand(pBehaviour, 0, -1);
		break;
	case 8:
		StateJamGutRideStand(pBehaviour, 1, JAMGUT_RIDE_STATE_STAND);
		break;
	case 9:
		StateJamGutWalk(pBehaviour, JAMGUT_RIDE_STATE_STAND);
		break;
	case JAMGUT_RIDE_STATE_RUN:
		StateJamGutRun(pBehaviour, JAMGUT_RIDE_STATE_STAND);
		break;
	case JAMGUT_RIDE_STATE_JUMP_BEFORE:
		StateJamGutJumpBefore(pBehaviour);
		break;
	case JAMGUT_RIDE_STATE_JUMP:
		StateJamGutJump();
		break;
	case JAMGUT_RIDE_STATE_JUMP_AFTER:
		StateJamGutJumpAfter(pBehaviour, JAMGUT_RIDE_STATE_STAND);
		break;
	case JAMGUT_RIDE_STATE_JUMP_AFTER_B:
		StateJamGutJumpAfter(pBehaviour, JAMGUT_RIDE_STATE_STAND);
		break;
	case JAMGUT_RIDE_STATE_FALL:
		StateJamGutFall();
		break;
	case 0x10:
		IMPLEMENTATION_GUARD(
		StateJamGutCheatFly((long)(int)this);)
		break;
	case 0x11:
		StateJamGutHit();
		break;
	case 0x12:
		IMPLEMENTATION_GUARD(
		StateJamGutDuration(-1.0f, this, (CBehaviourJamGut*)pBehaviour, JAMGUT_RIDE_STATE_STAND);)
		break;
	case 0x13:
		IMPLEMENTATION_GUARD(
		StateJamGutDead(-1.0f, this, (CBehaviourJamGut*)pBehaviour, 0, 1);)
		break;
	case 0x14:
		IMPLEMENTATION_GUARD(
		StateJamGutDead(-1.0f, this, (CBehaviourJamGut*)pBehaviour, 0, 1);)
		break;
	case 0x15:
		StateJamGutDead(2.0f, pBehaviour, 1, 1);
		break;
	case 0x16:
		IMPLEMENTATION_GUARD(
		StateJamGutDead(2.0, this, (CBehaviourJamGut*)pBehaviour, 0, 1);)
	}

	pHero = this->pRider;
	iVar6 = 0;
	if ((pHero != (CActorHero*)0x0) &&
		(iVar6 = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pCVar7 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		iVar6 = pCVar7->aCommands[1];
	}

	if (iVar6 != 0) {
		if ((GetStateFlags(this->actorState) & 0x1000) == 0) {
			pHero = this->pRider;
			iVar6 = 0;
			if ((pHero != (CActorHero*)0x0) &&
				(iVar6 = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
				pCVar8 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
				iVar6 = pCVar8->field_0x7c;
			}

			if (iVar6 == 0) {
				this->activePathIndex = -1;
				this->pTrackedPathActor = (CActor*)0x0;
			}

			iVar6 = this->actorState;
			uVar7 = 0xffffffff;
			if ((iVar6 != -1) && (5 < iVar6)) {
				uVar7 = gStateCfg_JMG[iVar6 + -6].field_0x8;
			}

			pHero = this->pRider;
			if (((pHero != (CActorHero*)0x0) &&
				(pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) &&
				(uVar7 != 0xffffffff)) {
				pCVar9 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
				pCVar9->SetState(uVar7, 0);
			}
		}
	}

	pHero = this->pRider;
	iVar6 = 0;
	if ((pHero != (CActorHero*)0x0) &&
		(iVar6 = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pCVar10 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		iVar6 = pCVar10->aCommands[10];
	}

	if (iVar6 == 0) {
		if ((((GetStateFlags(this->actorState) & 1) == 0) && (this->actorState != 0x10)) &&
			(bVar3 = CheckHitAndDeath(this->pRider), bVar3 == false)) {
			pHero = this->pRider;
			iVar6 = 0;
			if ((pHero != (CActorHero*)0x0) &&
				(iVar6 = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
				pCVar11 = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
				iVar6 = pCVar11->aCommands[9];
			}

			if (iVar6 == 0) {
				if (iVar4 != -1) {
					SetBehaviour(3, iVar4, -1);
				}
			}
			else {
				SetState(0x15, -1);
			}
		}
	}
	else {
		SetState(0x10, -1);
	}

	return;
}



void CActorJamGut::StateJamGutStand(float minTime, CBehaviourJamGutStand* pBehaviour, CActor* param_4, int nextState)
{
	CCollision* pCol;
	bool bVar4;
	uint dynFlags;

	pCol = this->pCollisionData;

	if (param_4 != (CActor*)0x0) {
		CActor::SV_UpdateOrientationToPosition2D(10.0531f, &param_4->currentLocation);
		this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.2f, 0);
	}

	dynFlags = 0x100a023b;

	this->dynamic.rotationQuat = this->rotationQuat;
	this->dynamic.speed = 0.0f;
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	if (this->field_0x360 != 0) {
		dynFlags = 0x100a021b;
	}

	ManageDyn(4.0f, dynFlags, (CActorsTable*)0x0);

	if (nextState == -1) {
		if (this->field_0x360 != 0) {
			return;
		}

		if ((pCol->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
			if (this->timeInAir <= 0.1f) {
				return;
			}

			SetState(JAMGUT_RIDE_STATE_FALL, -1);
			return;
		}

		this->timeInAir = 0.0f;
		return;
	}

	if ((minTime <= 0.0f) || (GetTimer()->scaledTotalTime - this->field_0x3a4 <= minTime)) {
		if (0.0f < minTime) {
			return;
		}

		if (!this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			return;
		}
	}

	SetState(nextState, -1);

	return;
}

void CActorJamGut::StateJamGutRideStand(CBehaviourJamGutRidden* pBehaviour, int param_3, int nextState)
{
	CCollision* pCVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	int iVar5;
	edF32VECTOR4* peVar6;
	StateConfig* pSVar7;
	int iVar9;
	ulong uVar10;
	float puVar12;
	float puVar13;
	float fVar11;
	float fVar12;
	float in_f21;
	float unaff_f20;

	pCVar1 = this->pCollisionData;
	pCVar2 = this->pAnimationController;
	this->dynamic.rotationQuat = this->rotationQuat;

	SV_MOV_UpdateSpeedIntensity(0.0f, 50.0f);

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;

	this->dynamicExt.field_0x6c = 0.0f;

	SV_UpdateValue(0.0f, 100.0f, &this->field_0x444);
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if ((pCVar1->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
		if (0.1f < this->timeInAir) {
			SetState(JAMGUT_RIDE_STATE_FALL, -1);
			return;
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	iVar5 = RiderCmdAttack();
	if (iVar5 == 0) {
		iVar5 = RiderCmdJump();
		if (iVar5 == 0) {
			iVar5 = RiderCmdRecept();
			if (iVar5 == 0) {
				uVar10 = RiderCmdWalk();
				if (((uVar10 != 0) || (uVar10 = RiderCmdRun(), uVar10 != 0)) ||
					(this->field_0x3a0 == 0)) {
					uVar10 = RiderCmdWalk();
					if ((uVar10 != 0) || (uVar10 = RiderCmdRun(), uVar10 != 0)) {
						this->field_0x3a0 = 0;
						this->field_0x390 = *RiderGetForce();
					}

					puVar12 = edF32Vector4DotProductHard(&this->rotationQuat, &this->field_0x390);
					if (1.0f < puVar12) {
						puVar13 = 1.0f;
					}
					else {
						puVar13 = -1.0f;
						if (-1.0f <= puVar12) {
							puVar13 = puVar12;
						}
					}

					fVar11 = acosf(puVar13);
	fVar11 = fVar11 / 3.141593f;
					fVar12 = edFIntervalUnitSrcLERP(fVar11, 0.9f, 0.03f);
					if (this->rotationQuat.x * (this->field_0x390).z -
						(this->field_0x390).x * this->rotationQuat.z < 0.0f) {
						fVar11 = fVar11 * -1.0f;
					}

					UpdatePercentLeftRight(fVar11, fVar12);

	bVar4 = CActor::SV_UpdateOrientation2D(12.56637f, &this->field_0x390, 0);
					this->field_0x3a0 = (int)bVar4;

					if (this->field_0x3a0 == 0) {
						pSVar7 = GetStateCfg(9);
						PlayAnim(pSVar7->animId);
						RiderSetState(GetStateRider(9), 0);
					}
					else {
						pSVar7 = GetStateCfg(this->actorState);
						PlayAnim(pSVar7->animId);
						RiderSetState(GetStateRider(this->actorState), 0);
					}

					this->field_0x430 = this->rotationQuat;
				}

				uVar10 = RiderCmdWalk();
				if ((uVar10 == 0) && (uVar10 = RiderCmdRun(), uVar10 == 0)) {
					this->field_0x3a4 = GetTimer()->scaledTotalTime;
					this->field_0x3a8 = GetTimer()->scaledTotalTime;
					if ((this->field_0x3a0 == 0) || (iVar5 = RiderCmdNew(), iVar5 != 0)) {
						this->idleTimer = 0.0f;
					}
					else {				
						if (pCVar2->IsCurrentLayerAnimEndReached(0)) {
							iVar5 = rand();
							if (this->field_0x384 == 0x1b) {
								in_f21 = 0.8f;
								unaff_f20 = 1.2f;
							}

							iVar9 = rand();
							bVar4 = PlayWaitingAnimation
							(((float)iVar5 / 2.147484e+09f) * 2.4f + 0.9f,
								in_f21 + (unaff_f20 - in_f21) * ((float)iVar9 / 2.147484e+09f), this->field_0x384, 0, 1);

							if ((bVar4 != false) &&
								(this->field_0x384 = this->field_0x384 + 1, this->field_0x384 == 0x1c)) {
								this->field_0x384 = 0x1b;
							}

							if (this->currentAnimType == 0x1b) {
								RiderSetState(0x12f, 0);
							}
							else {
								RiderSetState(0x11e, 0);
							}
						}
					}
				}
				else {
					uVar10 = RiderCmdWalk();
					if (uVar10 == 0) {
						uVar10 = RiderCmdRun();
						if (uVar10 != 0) {
							this->field_0x3a4 = GetTimer()->scaledTotalTime;
							if ((0.1f < GetTimer()->scaledTotalTime - this->field_0x3a8) && (this->field_0x3a0 != 0)) {
								SetState(JAMGUT_RIDE_STATE_RUN, -1);
								return;
							}
						}
					}
					else {
						this->field_0x3a8 = GetTimer()->scaledTotalTime;
						if ((0.1f < GetTimer()->scaledTotalTime - this->field_0x3a4) && (this->field_0x3a0 != 0)) {
							SetState(9, -1);
							return;
						}
					}
				}

				if (param_3 != 0) {
					if (this->pAnimationController->IsLayerActive(0)) {
						SetState(nextState, -1);
					}
				}
			}
			else {
				SetState(8, -1);
			}
		}
		else {
			SetJumpCfg(0.1f, this->field_0x404, this->field_0x4a8, this->field_0x4a0, this->field_0x4a4, 1, (edF32VECTOR4*)0x0);
			SetState(JAMGUT_RIDE_STATE_JUMP_BEFORE, -1);
		}
	}
	else {
		SetState(JAMGUT_RIDE_STATE_RUN, -1);
	}

	return;
}

void CActorJamGut::StateJamGutWalk(CBehaviourJamGut* pBehaviour, int restState)
{
	CCollision* pCVar1;
	bool bVar2;
	bool bVar3;
	int iVar4;
	edF32VECTOR4* peVar5;
	CBehaviourHeroRideJamGut* pRideBehaviour;
	float fVar7;
	float puVar9;
	float fVar8;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;
	fVar7 = RiderGetIntensity();
	SV_UpdateValue(this->field_0x400 * fVar7, 40.0f, &this->field_0x444);
	SV_UpdateValue(this->field_0x40c, 40.0f, &this->field_0x448);
	puVar9 = edF32Vector4DotProductHard(&this->rotationQuat, &this->dynamic.horizontalVelocityDirectionEuler);
	this->field_0x44c = puVar9;
	if (1.0f < puVar9) {
		puVar9 = 1.0f;
	}
	else {
		if (puVar9 < -1.0f) {
			puVar9 = -1.0f;
		}
	}

	fVar7 = acosf(puVar9);
	this->field_0x44c = fVar7;
	edF32Vector4ScaleHard(this->dynamic.horizontalLinearSpeed, &eStack16, &this->dynamic.horizontalVelocityDirectionEuler);
	fVar7 = edF32Vector4DotProductHard(&this->rotationQuat, &eStack16);
	fVar8 = this->field_0x444;
	if (fVar7 < 1.0f) {
		this->field_0x440 = fVar8 - fVar7;
	}
	else {
		if (fVar8 == 0.0f) {
			this->field_0x440 = (-fVar8 * 0.5f) / fVar7;
		}
		else {
			this->field_0x440 = (fVar8 - fVar7) / fVar7;
		}
	}

	this->field_0x440 = this->field_0x440;
	iVar4 = RiderCmdWalk();

	if (((iVar4 == 0) && (iVar4 = RiderCmdRun(), iVar4 == 0)) &&
		(iVar4 = RiderCmdAttack(), iVar4 == 0)) {
		CActorMovable::SV_MOV_UpdateSpeedIntensity(0.0f, 50.0f);
	}
	else {
		BuildHorizontalSpeedVector(this->field_0x444, 15.0f, this->field_0x448, 9.424778f, 2.094395f);
	}

	bVar2 = ManageDynAndKillActors(0x1006023b);
	fVar7 = edFIntervalUnitDstLERP(this->dynamic.horizontalLinearSpeed, this->field_0x404, this->field_0x400);
	UpdatePercentWalkRun(-fVar7, 0.9f, 0.0f);
	iVar4 = RiderCmdWalk();
	if ((iVar4 == 0) && (iVar4 = RiderCmdRun(), iVar4 == 0)) {
		fVar7 = this->rotationQuat.x * this->dynamic.horizontalVelocityDirectionEuler.z -
			this->dynamic.horizontalVelocityDirectionEuler.x * this->rotationQuat.z;
	}
	else {
		peVar5 = RiderGetForce();
		fVar7 = peVar5->x * this->dynamic.horizontalVelocityDirectionEuler.z -
			this->dynamic.horizontalVelocityDirectionEuler.x * peVar5->z;
	}

	UpdatePercentLeftRight(-fVar7, 0.9f);
	fVar7 = edFIntervalLERP(this->field_0x440, 1.0f, 8.0f, 1.0f, 1.2f);
	SetAnimSpeed(fVar7);
	bVar3 = FUN_003763d0();
	if (bVar3 == false) {
		if ((pCVar1->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
			if (0.1 < this->timeInAir) {
				SetState(0xf, -1);
				return;
			}
		}
		else {
			this->timeInAir = 0.0f;
		}

		iVar4 = RiderCmdAttack();
		if (iVar4 == 0) {
			iVar4 = RiderCmdJump();
			if (iVar4 == 0) {
				bVar2 = RiderCmdStand();
				if ((bVar2 == false) && (iVar4 = RiderCmdRun(), iVar4 == 0)) {
					this->field_0x3a4 = GetTimer()->scaledTotalTime;
					this->field_0x3a8 = GetTimer()->scaledTotalTime;
				}
				else {
					bVar2 = RiderCmdStand();
					if (bVar2 == false) {
						this->field_0x3a8 = GetTimer()->scaledTotalTime;
						if (0.1f < GetTimer()->scaledTotalTime - this->field_0x3a4) {
							SetState(10, -1);
						}
					}
					else {
						this->field_0x3a4 = GetTimer()->scaledTotalTime;
						if ((0.1f < GetTimer()->scaledTotalTime - this->field_0x3a8) && (restState != -1)) {
							SetState(restState, -1);
						}
					}
				}
			}
			else {
				if (bVar2 == false) {
					SetJumpCfg(0.1f, this->field_0x400, this->field_0x4a8, this->field_0x4a0, this->field_0x4a4, 1, (edF32VECTOR4*)0x0);
				}
				else {
					SetJumpCfg(0.1f, this->field_0x400 * 0.5f, this->field_0x4a8 * 0.1f, this->field_0x4a0 * 0.1f, this->field_0x4a4 * 0.1f, 1, (edF32VECTOR4*)0x0);
				}
				SetState(0xb, -1);
			}
		}
		else {
			SetState(10, -1);
		}
	}
	else {
		SetState(7, -1);
		pRideBehaviour = (CBehaviourHeroRideJamGut*)this->pRider->GetBehaviour(this->pRider->curBehaviourId);
		pRideBehaviour->ResetCommands();
		RiderSetState(0x131, 1);
	}

	return;
}

void CActorJamGut::StateJamGutRun(CBehaviourJamGut* pBehaviour, int nextState)
{
	bool bVar1;
	bool cVar2;
	int iVar3;
	uint uVar5;
	edF32VECTOR4* peVar6;
	S_JMG_PATH* pPath;
	float fVar7;
	float puVar10;
	float fVar8;
	float puVar9;
	float fVar9;
	float puVar8;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CCollision* pCol;
	CEventManager* pEventManager;

	pCol = this->pCollisionData;
	if ((this->flags_0x358 & 1) == 0) {
		fVar7 = RiderGetIntensity();
		JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::StateJamGutRun: intensity={:.2f}, targetSpeed={:.2f} (field_0x444 was {:.2f})", fVar7, this->field_0x404 * fVar7, this->field_0x444);
		SV_UpdateValue(this->field_0x404 * fVar7, 20.0f, &this->field_0x444);
		SV_UpdateValue(this->field_0x410, 80.0f, &this->field_0x448);
	}
	else {
		iVar3 = RiderCmdAttack();
		if (iVar3 == 0) {
			fVar7 = RiderGetIntensity();
			JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::StateJamGutRun: [attack-flag] intensity={:.2f}, targetSpeed={:.2f} (field_0x444 was {:.2f})", fVar7, this->field_0x404 * fVar7, this->field_0x444);
			SV_UpdateValue(this->field_0x404 * fVar7, 20.0f, &this->field_0x444);
			this->field_0x3a8 = GetTimer()->scaledTotalTime;
		}
		else {
			fVar7 = edFIntervalLERP(GetTimer()->scaledTotalTime - this->field_0x3a8, 0.0f, 1.2f,
				0.0f, this->field_0x418);
			this->field_0x444 = this->field_0x444 + fVar7;
			if (this->field_0x408 < this->field_0x444) {
				this->field_0x444 = this->field_0x408;
			}
			this->dynamic.speed = fVar7 + this->dynamic.speed;
			if (this->field_0x408 < this->dynamic.speed) {
				this->dynamic.speed = this->field_0x408;
			}
		}

		fVar7 = edFIntervalLERP(this->field_0x444, this->field_0x404, this->field_0x408, this->field_0x410, this->field_0x414);
		this->field_0x448 = fVar7;
	}

	bVar1 = RiderCmdStand();
	if (((bVar1 == false) ||
		(iVar3 = RiderGetNew(), pEventManager = CScene::ptable.g_EventManager_006f5080, iVar3 != 0)) || (this->pRider == (CActorHero*)0x0)) {
		cVar2 = UpdateFunc(50.0f);
	}
	else {
		pPath = this->aPaths;
		bVar1 = false;
		iVar3 = 0;
		if (0 < this->nbPaths) {
			for (; iVar3 < this->nbPaths; iVar3 = iVar3 + 1) {
				uVar5 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, (pPath->zoneRef).Get(), &this->currentLocation, 0);
				if ((uVar5 & 1) != 0) {
					bVar1 = true;
				}

				pPath = pPath + 1;
			}
		}

		if (bVar1) {
			fVar7 = this->field_0x41c;
			JAMGUT_LOG(LogLevel::Verbose, "CActorJamGut::StateJamGutRun: in path zone, UpdateFunc turnRate={:.2f} (vs default 50.0)", fVar7);
			cVar2 = UpdateFunc(fVar7);
		}
		else {
			cVar2 = UpdateFunc(50.0f);
		}
	}

	if (this->field_0x404 < this->field_0x444) {
		fVar7 = edFIntervalUnitDstLERP
		(this->dynamic.horizontalLinearSpeed, this->field_0x404,
			this->field_0x408);
		if (((this->flags_0x358 & 1) == 0) || (iVar3 = RiderCmdAttack(), iVar3 == 0)) {
			this->field_0x3a4 = GetTimer()->scaledTotalTime;
			UpdatePercentWalkRun(fVar7, 0.9f, 0.9f);
		}
		else {
			if ((1.5f < GetTimer()->scaledTotalTime - this->field_0x3a4) || (this->field_0x420 != 0)) {
				this->field_0x420 = 1;
				UpdatePercentWalkRun(fVar7, 0.9f, 0.95f);
			}
			else {
				UpdatePercentWalkRun(fVar7, 0.9f, 1.0f);
			}
		}
	}
	else {
		this->field_0x3a4 = GetTimer()->scaledTotalTime;
		fVar7 = edFIntervalUnitDstLERP(this->dynamic.horizontalLinearSpeed, this->field_0x404, this->field_0x400);
		UpdatePercentWalkRun(-fVar7, 0.9f, 0.9f);
	}

	if (this->pTrackedPathActor == (CActor*)0x0) {
		bVar1 = false;
	}
	else {
		bVar1 = this->pTrackedPathActor->typeID != MOVING_PLATFORM;
	}

	if (!bVar1) {
		iVar3 = RiderCmdWalk();
		if ((iVar3 == 0) && (iVar3 = RiderCmdRun(), iVar3 == 0)) {
			fVar7 = this->rotationQuat.x *
				this->dynamic.horizontalVelocityDirectionEuler.z -
				this->dynamic.horizontalVelocityDirectionEuler.x *
				this->rotationQuat.z;
		}
		else {
			peVar6 = RiderGetForce();
			fVar7 = peVar6->x * this->dynamic.horizontalVelocityDirectionEuler.z -
				this->dynamic.horizontalVelocityDirectionEuler.x * peVar6->z;
		}

		UpdatePercentLeftRight(-fVar7, 0.9f);
	}

	fVar7 = edFIntervalLERP(this->field_0x440, 1.0f, 8.0f, 1.0f, 1.2f);
	SetAnimSpeed(fVar7);
	bVar1 = FUN_003763d0();
	if (bVar1 == false) {
		if ((pCol->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
			if (0.1f < this->timeInAir) {
				SetState(JAMGUT_RIDE_STATE_FALL, -1);
				return;
			}
		}
		else {
			this->timeInAir = 0.0f;
		}

		iVar3 = RiderCmdJump();
		if (iVar3 == 0) {
			bVar1 = RiderCmdStand();
			if ((((bVar1 != false) && (iVar3 = RiderCmdAttack(), iVar3 == 0)) &&
				(this->dynamic.horizontalLinearSpeed < 3.0f)) && (nextState != -1)) {
				SetState(nextState, -1);
			}
		}
		else {
			if (cVar2 == false) {
				SetJumpCfg(0.1f, this->field_0x404, this->field_0x4a8, this->field_0x4a0, this->field_0x4a4, 1, (edF32VECTOR4*)0x0);
			}
			else {
				SetJumpCfg(0.1f, this->field_0x404 * 0.5f, this->field_0x4a8 * 0.1f, this->field_0x4a0 * 0.1f, this->field_0x4a4 * 0.1f, 1, (edF32VECTOR4*)0x0);
			}

			SetState(JAMGUT_RIDE_STATE_JUMP_BEFORE, -1);
		}
	}
	else {
		SetState(9, -1);
	}

	return;
}

void CActorJamGut::StateJamGutFall()
{
	CCollision* pCVar1;
	float fVar2;

	pCVar1 = this->pCollisionData;
	fVar2 = RiderGetIntensity();
	SV_UpdateValue(this->field_0x484 * fVar2, 5.0f, &this->field_0x444);
	MoveInAir(this->field_0x480, this->field_0x444, this->field_0x488, 10.0f, 4.712389f);
	ManageDyn(4.0f, 0x140129, (CActorsTable*)0x0);

	if ((pCVar1->flags_0x4 & COLLISION_GROUND_FLAG) != 0) {
		if (2.0f < this->dynamic.horizontalLinearSpeed) {
			SetState(JAMGUT_RIDE_STATE_JUMP_AFTER_B, -1);
		}
		else {
			SetState(JAMGUT_RIDE_STATE_JUMP_AFTER, -1);
		}
	}

	return;
}

void CActorJamGut::StateJamGutHit()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	int iVar3;
	bool bVar4;
	CLifeInterface* pCVar5;
	StateConfig* pSVar6;
	uint uVar7;
	float fVar8;
	CCollision* pCol;
	CActorHero* pHero;

	pCol = this->pCollisionData;
	pCVar1 = this->pAnimationController;
	pHero = this->pRider;

	ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

	if ((pCol->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
		if (pCVar1->IsCurrentLayerAnimEndReached(0)) {
			if (pHero == (CActorHero*)0x0) {
				SetState(0xf, -1);
			}
			else {
				if ((pHero->GetStateFlags(pHero->actorState) & 1) == 0) {
					fVar8 = pHero->GetLifeInterface()->GetValue();
					if (0.0f < fVar8 - pHero->field_0x2e4) {
						SetState(0xf, -1);
					}
				}
			}
		}
	}
	else {
		bVar4 = ColWithLava();
		if (bVar4 == false) {
			SetState(0xd, -1);
		}
		else {
			if (pHero != (CActorHero*)0x0) {
				fVar8 = pHero->GetLifeInterface()->GetValue();
				if (fVar8 - pHero->field_0x2e4 <= 0.0f) {
					SetState(0x13, -1);
				}
			}
		}

		this->timeInAir = 0.0f;
	}

	return;
}

void CActorJamGut::StateJamGutJumpBefore(CBehaviourJamGut*)
{
	bool bVar1;
	int iVar2;
	float fVar4;
	undefined* puVar5;
	float fVar6;
	edF32VECTOR4 eStack16;
	CCollision* pCol;

	pCol = this->pCollisionData;
	fVar4 = RiderGetIntensity();
	SV_UpdateValue(this->field_0x484 * fVar4, 20.0f, &this->field_0x444);
	bVar1 = UpdateFunc(50.0f);
	if (bVar1 != false) {
		SetJumpCfg(0.1f, this->field_0x484 * 0.5f, this->field_0x4a8 * 0.1f, this->field_0x4a0 * 0.1f, this->field_0x4a4 * 0.1f, 1, (edF32VECTOR4*)0x0);
	}

	if ((pCol->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
		if (0.1f < this->timeInAir) {
			SetState(JAMGUT_RIDE_STATE_FALL, -1);
			return;
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	if (0.05f < GetTimer()->scaledTotalTime - this->field_0x3a4) {
		SetState(JAMGUT_RIDE_STATE_JUMP, -1);
	}

	return;
}

void CActorJamGut::StateJamGutJumpInit()
{
	if (this->field_0x4ec == 0) {
		this->field_0x4c4.BuildFromSpeedDist(this->field_0x48c, 0.0f, this->field_0x490);
	}

	this->field_0x4ec = 0;
	this->field_0x4bc = 1.0f;
	this->field_0x4c0 = 1.0f;
	this->field_0x4b8 = 0;

	return;
}

void CActorJamGut::StateJamGutJump()
{
	bool bVar1;
	int iVar2;
	float fVar4;
	CCollision* pCol;

	pCol = this->pCollisionData;
	iVar2 = RiderCmdJump();
	if (iVar2 != 0) {
		this->field_0x4b8 = 1;
		this->field_0x494 = 0;
	}

	if (((this->field_0x494 != 0) && (3.0f < (this->field_0x4c4).field_0x20)) &&
		(iVar2 = RiderCmd_00367e40(), iVar2 == 0)) {
		this->field_0x4c0 = 2.0f;
	}

	if ((0.0f < this->field_0x490) &&
		(bVar1 = this->field_0x4c4.IsFinished(), bVar1 == false)) {
		fVar4 = GetTimer()->cutsceneDeltaTime;
		this->field_0x4c4.Integrate(this->field_0x4c0 * this->field_0x4bc * GetTimer()->cutsceneDeltaTime, this->field_0x4c0 * fVar4);
		this->dynamicExt.instanceIndex.y = (this->field_0x4c4).field_0x20;
		this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x8000;
		this->field_0x3a4 = GetTimer()->scaledTotalTime;
	}

	fVar4 = RiderGetIntensity();
	SV_UpdateValue(this->field_0x484 * fVar4, 20.0f, &this->field_0x444);

	MoveInAir(this->field_0x480, this->field_0x444, this->field_0x488, 10.0f, 4.712389f);
	ManageDynAndKillActors(0x140129);

	if ((pCol->flags_0x4 & COLLISION_CEILING_FLAG) != 0) {
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
		this->field_0x490 = 0.0f;
	}

	if ((pCol->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
		if (this->field_0x4f0 + (this->field_0x4c4).field_0x4 < this->timeInAir) {
			SetState(JAMGUT_RIDE_STATE_FALL, -1);
		}
	}
	else {
		if (0.0f < this->dynamic.velocityDirectionEuler.y) {
			this->field_0x4b8 = 0;
		}

		if (2.0f < this->dynamic.horizontalLinearSpeed) {
			SetState(JAMGUT_RIDE_STATE_JUMP_AFTER_B, -1);
		}
		else {
			SetState(JAMGUT_RIDE_STATE_JUMP_AFTER, -1);
		}
	}

	return;
}


void CActorJamGut::StateJamGutJumpAfter(CBehaviourJamGut* pBehaviour, int nextState)
{
	CCollision* pCVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	int iVar5;
	float fVar6;
	undefined* puVar7;
	float fVar8;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;
	pCVar2 = this->pAnimationController;
	fVar6 = RiderGetIntensity();
	SV_UpdateValue(this->field_0x484 * fVar6, 80.0f, &this->field_0x444);

	bVar4 = UpdateFunc(200.0f);
	if ((pCVar1->flags_0x4 & COLLISION_GROUND_FLAG) == 0) {
		if (0.1 < this->timeInAir) {
			SetState(JAMGUT_RIDE_STATE_FALL, -1);
			return;
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	iVar5 = RiderCmdJump();
	if (iVar5 == 0) {
		if (pCVar2->IsCurrentLayerAnimEndReached(0)) {
			iVar5 = RiderCmdRun();
			if (iVar5 == 0) {
				iVar5 = RiderCmdWalk();
				if (iVar5 == 0) {
					SetState(nextState, -1);
				}
				else {
					SetState(9, -1);
				}
			}
			else {
				SetState(JAMGUT_RIDE_STATE_RUN, -1);
			}
		}
	}
	else {
		if (bVar4 == false) {
	SetJumpCfg(0.1f, this->field_0x484, this->field_0x4a8, this->field_0x4a0, this->field_0x4a4, 1, (edF32VECTOR4*)0x0);
		}
		else {
	SetJumpCfg(0.1f, this->field_0x484 * 0.5f, this->field_0x4a8 * 0.1f, this->field_0x4a0 * 0.1f, this->field_0x4a4 * 0.1f, 1, (edF32VECTOR4*)0x0);
		}

		SetState(JAMGUT_RIDE_STATE_JUMP, -1);
	}

	return;
}

void CActorJamGut::StateJamGutStandInit()
{
	this->field_0x3a0 = 1;
	UpdatePercentWalkRun(-1.0f, 0.0f, 0.0f);
	UpdatePercentLeftRight(0.0f, 0.0f);

	return;
}

void CActorJamGut::StateJamGutDead(float param_1, CBehaviourJamGut* pBehaviour, int param_4, int param_5)
{
	if (param_4 == 0) {
		this->dynamic.speed = 0.0f;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);
	}

	if (param_1 < 0.0f) {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) goto LAB_00372290;
	}

	if (param_1 <= 0.0f) {
		return;
	}
	if (this->timeInAir <= param_1) {
		return;
	}

LAB_00372290:
	if (param_5 == 0) {
		this->field_0x620 = 1;
		PreReset();
		Reset();
	}
	else {
		if (this->pRider->IsKindOfObject(8) != false) {
			this->pRider->ProcessDeath();
		}
	}

	return;
}

void CActorJamGut::MoveInAir(float param_1, float param_2, float param_3, float param_4, float param_5)
{
	int iVar1;
	float fVar2;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	iVar1 = RiderCmdWalk();
	if ((iVar1 == 0) && (iVar1 = RiderCmdRun(), iVar1 == 0)) {
		SV_MOV_UpdateSpeedIntensity(0.0f, param_4);
	}
	else {
		edF32Vector4ScaleHard(this->dynamic.speed, &local_10, &this->dynamic.rotationQuat);
		FUN_00367a90(&eStack32);
		edF32Vector4ScaleHard(param_3, &eStack32, &eStack32);
		edF32Vector4AddHard(&local_10, &local_10, &eStack32);
		fVar2 = edF32Vector4NormalizeHard(&local_10, &local_10);
		if ((fVar2 <= param_2) && (param_2 = param_1, param_1 <= fVar2)) {
			param_2 = fVar2;
		}

		this->dynamic.speed = param_2;

		if (0.001f < param_2) {
			this->dynamic.rotationQuat = local_10;
		}

		SV_UpdateOrientation2D(param_5, &this->dynamic.rotationQuat, 0);
	}

	return;
}

bool CActorJamGut::ManageDynAndKillActors(uint dynFlags)
{
	CActor* pReceiver;
	CActor** pEntry;
	int iVar1;
	bool bVar2;
	CActorsTable actorsTable;

	actorsTable.nbEntries = 0;
	bVar2 = false;
	ManageDyn(4.0f, dynFlags, &actorsTable);

	iVar1 = 0;
	if (0 < actorsTable.nbEntries) {
		pEntry = actorsTable.aEntries;
		bVar2 = false;
		do {
			pReceiver = *pEntry;

			if ((this->pRider != (CActorHero*)0x0) && (((1.0f < this->dynamic.speed || (1.0f < this->dynamic.linearAcceleration)) &&
					((CActorFactory::gClassProperties[pReceiver->typeID].flags & 0x10000) != 0)))) {
				DoMessage(pReceiver, (ACTOR_MESSAGE)3, 0);
			}

			CActorMovingPlatform* pMovingPlatform = static_cast<CActorMovingPlatform*>(pReceiver);
			if ((pReceiver->typeID == MOVING_PLATFORM) && ((pMovingPlatform->pProperties->flags_0x24 & 0x800000) != 0)) {
				bVar2 = true;
			}

			iVar1 = iVar1 + 1;
			pEntry = pEntry + 1;
		} while (iVar1 < actorsTable.nbEntries);
	}

	return bVar2;
}

bool CActorJamGut::FUN_003763d0()
{
	bool bVar1;
	int iVar2;
	uint uVar3;
	uint uVar5;
	float fVar6;
	CCollisionRay CStack144;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 local_60;
	CCollisionRay CStack80;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	_ray_info_out rayInfo;
	CCollision* pCol;

	iVar2 = RiderGetNew();
	if ((iVar2 == 0) || (this->activePathIndex != -1)) {
		this->field_0x460 = 0.0f;
		bVar1 = false;
	}
	else {
		pCol = this->pCollisionData;
		if (((pCol->flags_0x4 & COLLISION_WALL_FLAG) != 0) && (fVar6 = edF32Vector4DotProductHard(&pCol->aCollisionContact[0].location, &this->rotationQuat), fVar6 < -0.5f)) {
			fVar6 = this->field_0x460 + GetTimer()->cutsceneDeltaTime;
			this->field_0x460 = fVar6;
			if (0.05f < fVar6) {
				return true;
			}
		}

		local_20.x = this->currentLocation.x;
		local_20.z = this->currentLocation.z;
		local_20.y = this->currentLocation.y + 0.1f;
		local_20.w = this->currentLocation.w;

		CCollisionRay CStack80 = CCollisionRay(6.0f, &local_20, &this->rotationQuat);
		fVar6 = CStack80.Intersect(3, this, (CActor*)0x0, 0x40000048, &eStack48, &rayInfo);
		uVar5 = rayInfo.hitMaterialFlags & 0xf;
		if (uVar5 == 0) {
			uVar5 = CScene::_pinstance->defaultMaterialIndex;
		}

		if ((fVar6 == 1e+30f) ||
			((((CCollisionManager::_material_table[uVar5].field_0x8 <= fabs(eStack48.y) &&
				(uVar3 = CCollisionManager::IsASomethingGround(uVar5, &eStack48), uVar3 == 0)) && (uVar5 != 3)) &&
				((uVar5 != 7 && (uVar5 != 5)))))) {
			local_60.y = -1.0f;
			local_60.x = 0.0f;
			local_60.z = 0.0f;
			local_60.w = 0.0f;
			edF32Vector4ScaleHard(6.0f, &eStack112, &this->rotationQuat);
			edF32Vector4AddHard(&local_20, &this->currentLocation, &eStack112);
			CCollisionRay CStack144 = CCollisionRay(this->field_0x4a4, &local_20, &local_60);
			fVar6 = CStack144.Intersect(3, this, (CActor*)0x0, 0x40000048, (edF32VECTOR4*)0x0,
				&rayInfo);

			rayInfo.hitMaterialFlags = rayInfo.hitMaterialFlags & 0xf;

			if (rayInfo.hitMaterialFlags == 0) {
				rayInfo.hitMaterialFlags = CScene::_pinstance->defaultMaterialIndex;
			}

			if (((fVar6 == 1e+30f) || (rayInfo.hitMaterialFlags == 3)) ||
				((rayInfo.hitMaterialFlags == 7 || (rayInfo.hitMaterialFlags == 5)))) {
				fVar6 = this->field_0x460 + GetTimer()->cutsceneDeltaTime;
				this->field_0x460 = fVar6;
				if (0.05 < fVar6) {
					return true;
				}
			}
			else {
				this->field_0x460 = 0.0f;
			}
			bVar1 = false;
		}
		else {
			fVar6 = this->field_0x460 + GetTimer()->cutsceneDeltaTime;
			this->field_0x460 = fVar6;
			if (0.05f < fVar6) {
				bVar1 = true;
			}
			else {
				bVar1 = false;
			}
		}
	}

	return bVar1;
}

void CActorJamGut::UpdatePercentWalkRun(float param_1, float param_2, float param_3)
{
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	CActorHero* pHero;

	if (param_2 == 0.0f) {
		this->field_0x378 = param_1;
	}
	else {
		SV_UpdatePercent(param_1, param_2,  &this->field_0x378);
	}

	if (param_3 == 0.0f) {
		this->field_0x37c = param_1;
	}
	else {
		if (param_3 != 1.0f) {
			SV_UpdatePercent(param_1, param_3, &this->field_0x37c);
		}
	}

	pHero = this->pRider;
	if ((pHero != (CActorHero*)0x0) && (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		pHeroRideJamGut->field_0x6c = this->field_0x37c;
	}

	return;
}

void CActorJamGut::SetAnimSpeed(float animSpeed)
{
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	CActorHero* pHero;

	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(animSpeed, 0);

	pHero = this->pRider;
	if ((pHero != (CActorHero*)0x0) &&
		(pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		pHeroRideJamGut->SetSpeedAnim(animSpeed);
	}

	return;
}

int CActorJamGut::RiderCmdAttack()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	bResult = 0;
	if ((this->flags_0x358 & 1) != 0) {
		pHero = this->pRider;
		bResult = 0;
		if ((pHero != (CActorHero*)0x0) &&
			(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
			pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
			bResult = pHeroRideJamGut->aCommands[JAMGUT_CMD_ATTACK];
		}
	}

	return bResult;
}

int CActorJamGut::RiderCmdJump()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	pHero = this->pRider;
	bResult = 0;
	if ((pHero != (CActorHero*)0x0) &&
		(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		bResult = pHeroRideJamGut->aCommands[JAMGUT_CMD_JUMP];
	}

	return bResult;
}

int CActorJamGut::RiderCmdRecept()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	pHero = this->pRider;
	bResult = 0;
	if ((pHero != (CActorHero*)0x0) &&
		(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		bResult = pHeroRideJamGut->aCommands[JAMGUT_CMD_RECEPT];
	}

	return bResult;
}

int CActorJamGut::RiderCmdStand()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	if (this->activePathIndex == -1) {
		pHero = this->pRider;
		bResult = 0;
		if ((pHero != (CActorHero*)0x0) &&
			(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
			pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
			bResult = pHeroRideJamGut->field_0x68 == 2;
		}
	}
	else {
		bResult = this->pathFollowCmdWalk;
	}

	return bResult;
}

int CActorJamGut::RiderCmdWalk()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	if (this->activePathIndex == -1) {
		pHero = this->pRider;
		bResult = 0;
		if ((pHero != (CActorHero*)0x0) &&
			(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
			pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
			bResult = pHeroRideJamGut->field_0x68 == 3;
		}
	}
	else {
		bResult = this->pathFollowCmdWalk;
	}

	return bResult;
}

int CActorJamGut::RiderCmdRun()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	if (this->activePathIndex == -1) {
		pHero = this->pRider;
		bResult = 0;
		if ((pHero != (CActorHero*)0x0) &&
			(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
			pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
			bResult = pHeroRideJamGut->field_0x68 == 4;
		}
	}
	else {
		bResult = this->pathFollowCmdRun;
	}

	return bResult;
}

int CActorJamGut::RiderCmdNew()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	pHero = this->pRider;
	bResult = 0;
	if ((pHero != (CActorHero*)0x0) &&
		(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		bResult = pHeroRideJamGut->field_0x7c;
	}

	return bResult;
}

int CActorJamGut::RiderCmd_00367e40()
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	pHero = this->pRider;
	bResult = 0;
	if ((pHero != (CActorHero*)0x0) &&
		(bResult = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		bResult = pHeroRideJamGut->aCommands[6];
	}

	return bResult;
}

edF32VECTOR4* CActorJamGut::RiderGetForce()
{
	int iVar1;
	edF32VECTOR4* pForce;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	CActorHero* pHero;

	if (this->activePathIndex == -1) {
		pHero = this->pRider;
		if (pHero == (CActorHero*)0x0) {
			pForce = &gF32Vector4Zero;
		}
		else {
			if ((pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
				pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
				iVar1 = pHeroRideJamGut->field_0x68;
				if ((iVar1 == 3) || (iVar1 == 4)) {
					pForce = &pHeroRideJamGut->inputAnalogDir;
				}
				else {
					pForce = &this->rotationQuat;
				}
			}
			else {
				pForce = &gF32Vector4Zero;
			}
		}
	}
	else {
		pForce = &this->pathFollowDirection;
	}

	return pForce;
}

float CActorJamGut::RiderGetIntensity()
{
	int iVar1;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	float intensity;
	CActorHero* pHero;

	if (this->activePathIndex == -1) {
		pHero = this->pRider;
		if (pHero == (CActorHero*)0x0) {
			intensity = 0.0f;
		}
		else {
			if (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT) {
				pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
				iVar1 = pHeroRideJamGut->field_0x68;
				if ((iVar1 == 3) || (iVar1 == 4)) {
					intensity = pHeroRideJamGut->inputMagnitude;
				}
				else {
					intensity = 1.0f;
				}
			}
			else {
				intensity = 0.0f;
			}
		}
	}
	else {
		intensity = this->pathFollowIntensity;
	}

	return intensity;
}

int CActorJamGut::RiderGetNew()
{
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int iVar1;
	CActorHero* pHero;

	pHero = this->pRider;
	iVar1 = 0;
	if ((pHero != (CActorHero*)0x0) && (iVar1 = 0, pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		iVar1 = pHeroRideJamGut->field_0x7c;
	}

	return iVar1;
}

void CActorJamGut::RiderSetState(int state, int anim)
{
	CActorHero* pHero;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	int bResult;

	pHero = this->pRider;
	if ((pHero != (CActorHero*)0x0) &&
		(pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT) && (state != -1)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		pHeroRideJamGut->SetState(state, anim);
	}

	return;
}

void CActorJamGut::FUN_00367a90(edF32VECTOR4* param_2)
{
	int iVar1;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	CActorHero* pHero;

	if (this->activePathIndex == -1) {
		pHero = this->pRider;

		if ((pHero != (CActorHero*)0x0) && (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
			pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
			iVar1 = pHeroRideJamGut->field_0x68;
			if ((iVar1 == 3) || (iVar1 == 4)) {
				edF32Vector4ScaleHard(pHeroRideJamGut->inputMagnitude, param_2, &pHeroRideJamGut->inputAnalogDir);
			}
			else {
				*param_2 = this->rotationQuat;
			}
		}
	}
	else {
		edF32Vector4ScaleHard(this->pathFollowIntensity, param_2, &this->pathFollowDirection);
	}

	return;
}

void CActorJamGut::BuildHorizontalSpeedVector(float param_1, float param_2, float param_3, float param_4, float param_5)
{
	edF32VECTOR4* v0;
	float puVar2;
	float puVar3;
	float fVar1;
	float fVar2;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	FUN_00367a90(&eStack32);
	v0 = RiderGetForce();
	puVar2 = edF32Vector4DotProductHard(v0, &this->dynamic.velocityDirectionEuler);
	if (1.0f < puVar2) {
		puVar3 = 1.0f;
	}
	else {
		puVar3 = -1.0f;
		if (-1.0f <= puVar2) {
			puVar3 = puVar2;
		}
	}

	fVar1 = acosf(puVar3);
	fVar2 = edFIntervalLERP(fVar1, 0.0f, param_5, 0.1f, 2.0f);
	edF32Vector4ScaleHard(param_3 * fVar2, &eStack32, &eStack32);
	fVar1 = edFIntervalLERP(fVar1, 0.0f, param_5, 1.0f, 0.5f);
	fVar1 = param_1 * fVar1;
	edF32Vector4ScaleHard(this->dynamic.speed, &local_10, &this->dynamic.rotationQuat);
	edF32Vector4AddHard(&local_10, &local_10, &eStack32);
	fVar2 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);
	if (0.001f < fVar2) {
		this->dynamic.rotationQuat = local_10;
	}

	if (fVar2 <= fVar1) {
		fVar1 = fVar2;
	}

	SV_MOV_UpdateSpeedIntensity(fVar1, param_2);
	SV_UpdateOrientation2D(param_4, &local_10, 0);

	return;
}

void CActorJamGut::UpdatePercentLeftRight(float param_1, float param_2)
{
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	CActorHero* pHero;

	if (param_2 == 0.0f) {
		this->field_0x380 = param_1;
	}
	else {
		SV_UpdatePercent(param_1, param_2, &this->field_0x380);
	}

	pHero = this->pRider;
	if ((pHero != (CActorHero*)0x0) &&
		(pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = static_cast<CBehaviourHeroRideJamGut*>(pHero->GetBehaviour(pHero->curBehaviourId));
		pHeroRideJamGut->field_0x70 = this->field_0x380;
	}

	return;
}

bool CActorJamGut::FUN_00376710()
{
	int iVar1;
	CActor* pCVar2;
	bool bVar3;
	StateConfig* pSVar4;
	uint uVar5;

	if ((GetStateFlags(this->actorState) & 1) == 0) {
		bVar3 = ColWithAToboggan();
		if ((bVar3 == false) && ((this->dynamic.flags & 2) == 0)) {
			pCVar2 = this->pTiedActor;
			if ((pCVar2 == (CActor*)0x0) || (pCVar2->typeID != 0x1b)) {
				bVar3 = true;
			}
			else {
				bVar3 = false;
			}
		}
		else {
			bVar3 = false;
		}
	}
	else {
		bVar3 = false;
	}

	return bVar3;
}

void CBehaviourJamGut::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourJamGut::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorJamGut*>(pOwner);

	return;
}

int CBehaviourJamGut::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourJamGutStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourJamGutStand::Manage()
{
	this->pOwner->BehaviourJamGutStand_Manage(this);

	return;
}

void CBehaviourJamGutStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorJamGut* pJamGut;
	int prevState;

	CBehaviourJamGut::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		pJamGut = this->pOwner;

		prevState = pJamGut->prevActorState;

		if (((pJamGut->GetStateFlags(prevState)) & 0x2000) == 0) {
			this->pOwner->SetState(6, -1);
		}
		else {
			this->pOwner->SetState(prevState, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourJamGutStand::InitState(int newState)
{
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CActorJamGut* pJamGut;

	pJamGut = this->pOwner;

	pJamGut->field_0x3a4 = GetTimer()->scaledTotalTime;
	pJamGut->field_0x3a8 = GetTimer()->scaledTotalTime;

	if (newState == 6) {
		pJamGut->field_0x360 = 1;
	}
	else {
		if (newState == 0x13) {
			local_10.w = 0.0f;
			local_10.x = 2.2f;
			local_10.z = 2.2f;
			local_10.y = 0.8f;

			local_20.x = 0.0f;
			local_20.z = 0.0f;
			local_20.y = 0.78f;
			local_20.w = 1.0f;

			pJamGut->ChangeCollisionSphere(0.4f, &local_10, &local_20);
		}
	}

	return;
}

void CBehaviourJamGutStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourJamGutStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	StateConfig* pSVar1;
	uint uVar2;
	int result;
	edF32VECTOR4* v0;
	float fVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CActorJamGut* pJamGut;

	if (msg == 0x8b) {
		this->pOwner->SetState(0x15, -1);
		result = 1;
	}
	else {
		result = 1;

		if (msg == 2) {
			pJamGut = this->pOwner;

			if (((pJamGut->GetStateFlags(pJamGut->actorState) & 1) == 0) && (this->pOwner->actorState != 0x10)) {
				_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
				uVar2 = pHitParam->projectileType;
				if ((uVar2 < 3) || (uVar2 == 5)) {
					result = this->pOwner->AccomplishHit(0, pSender, pHitParam, (edF32VECTOR4*)0x0);
				}
				else {
					if ((uVar2 == 10) || (uVar2 == 9)) {
						if (uVar2 == 9) {
							edF32Vector4ScaleHard(200.0f, &eStack16, &gF32Vector4UnitY);
						}
						else {
							edF32Vector4ScaleHard(pHitParam->field_0x30 * 0.5f, &eStack16, &gF32Vector4UnitY);
						}

						pJamGut = this->pOwner;
						edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack32, &eStack16);
						v0 = pJamGut->dynamicExt.aImpulseVelocities;
						edF32Vector4AddHard(v0, v0, &eStack32);
						pJamGut->dynamicExt.aImpulseVelocityMagnitudes[0] = edF32Vector4GetDistHard(pJamGut->dynamicExt.aImpulseVelocities);
						this->pOwner->SetState(0x11, -1);
						result = 1;
					}
					else {
						result = 0;
					}
				}
			}
			else {
				result = 0;
			}
		}
		else {
			if (msg != 1) {
				if (msg == 0x14) {
					this->pOwner->pRider = static_cast<CActorHero*>(pSender);
					this->pOwner->SetBehaviour(JAMGUT_BEHAVIOUR_RIDDEN, -1, -1);
					result = 1;
				}
				else {
					if (msg == 0x12) {
						if ((int)pMsgParam == 1) {
							pJamGut = this->pOwner;
							if ((pJamGut->GetStateFlags(pJamGut->actorState) & 0x4000) == 0) {
								this->pOwner->SetState(JAMGUT_STAND_STATE_REJECT_HERO_A, -1);
								this->pOwner->field_0x354 = static_cast<CActorHero*>(pSender);
							}
							result = 1;
						}
						else {
							result = this->pOwner->GetHeroAction(1.44f, static_cast<CActorHero*>(pSender));
						}
					}
					else {
						result = 0;
					}
				}
			}
		}
	}

	return result;
}

void CBehaviourJamGutRidden::Create(ByteCode* pByteCode)
{
	bool bVar1;
	CAudioManager* pCVar2;
	uint uVar3;
	CMusic* pCVar4;
	float fVar5;

	pCVar2 = CScene::ptable.g_AudioManager_00451698;
	uVar3 = pByteCode->GetU32();

	IMPLEMENTATION_GUARD_AUDIO(
	if ((uVar3 == 0xffffffff) || (bVar1 = (uint)pCVar2->nbMusic <= uVar3, bVar1)) {
		pCVar4 = (CMusic*)0x0;
	}
	else {
		if (bVar1) {
			uVar3 = 0;
		}
		pCVar4 = pCVar2->aMusic + uVar3;
	}
	this->field_0x10 = pCVar4;)

	this->field_0x14 = pByteCode->GetF32();
	this->field_0x18 = pByteCode->GetF32();
	this->field_0x1c = pByteCode->GetF32();
	this->field_0xc = -1;

	return;
}

void CBehaviourJamGutRidden::Manage()
{
	this->pOwner->BehaviourJamGutRidden_Manage(this);

	return;
}

void CBehaviourJamGutRidden::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	int iVar1;
	CMusicManager* pMusicManager;
	bool bVar2;
	int** ppiVar3;
	edF32VECTOR4 colSphereB;
	edF32VECTOR4 colSphereA;
	CActorJamGut* pJamGut;

	this->pOwner = static_cast<CActorJamGut*>(pOwner);

	colSphereA.y = 1.5f;
	colSphereB.y = 1.48f;
	colSphereB.w = 1.0f;
	colSphereA.x = 0.8f;
	colSphereA.z = 0.8f;
	colSphereA.w = 0.0f;
	colSphereB.x = 0.0f;
	colSphereB.z = 0.0f;

	this->pOwner->ChangeCollisionSphere(0.2f, &colSphereA, &colSphereB);

	if (newState == -1) {
		pJamGut = this->pOwner;
		iVar1 = pJamGut->prevActorState;
		if ((iVar1 == 6) || (iVar1 == -1)) {
			pJamGut->SetState(8, -1);
		}
		else {
			if (iVar1 == JAMGUT_RIDE_STATE_JUMP) {
				pJamGut->field_0x4ec = 1;
			}

			pJamGut = this->pOwner;
			pJamGut->SetState(pJamGut->prevActorState, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->field_0x8 = 0;

	if (this->field_0x10 != (CMusic*)0x0) {
		IMPLEMENTATION_GUARD_AUDIO(
		pMusicManager = (CMusicManager*)(CScene::ptable.g_AudioManager_00451698)->field_0x38;
		bVar2 = CMusicManager::FUN_00186bf0(pMusicManager, (long)this->field_0xc, (int)this->field_0x10);
		if ((bVar2 == false) ||
			(bVar2 = CMusicManager::FUN_00186b90(pMusicManager, (long)this->field_0xc), bVar2 == false)) {
			ppiVar3 = CMusicManager::Start
			(this->field_0x14, 1.0, this->field_0x18, 0.0, pMusicManager, this->field_0x10,
				(int*)0x19);
			this->field_0xc = (int)ppiVar3;
		}
		else {
			pMusicManager->CancelStop(this->field_0x18, this->field_0xc);
		})
	}

	return;
}

void CBehaviourJamGutRidden::End(int newBehaviourId)
{
	int iVar1;
	bool bVar2;
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	CNewFx* pFx;
	CActorHero* pHero;
	CActorJamGut* pJamGut;

	if (this->field_0x8 == 0) {
		this->pOwner->DoMessage(this->pOwner->pRider, (ACTOR_MESSAGE)0x44, 0);
	}
	else {
		this->field_0x8 = 0;
	}

	this->pOwner->pRider = (CActorHero*)0x0;

	pJamGut = this->pOwner;
	pHero = pJamGut->pRider;
	if ((pHero != (CActorHero*)0x0) && (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
		pHeroRideJamGut = (CBehaviourHeroRideJamGut*)pHero->GetBehaviour(pHero->curBehaviourId);
		pHeroRideJamGut->SetInvincible(2.0f, 0);
	}

	pJamGut->field_0x4fc = Timer::GetTimer()->scaledTotalTime;
	pJamGut->field_0x500 = 0;

	this->pOwner->RestoreCollisionSphere(0.2f);

	if (this->field_0x10 != (CMusic*)0x0) {
		IMPLEMENTATION_GUARD_AUDIO(
		(CScene::ptable.g_AudioManager_00451698)->field_0x38->MusicStop(this->field_0x1c, 0, this->field_0xc);
		this->field_0xc = -1;)
	}

	pJamGut = this->pOwner;
	if (pJamGut->field_0x368.type != -1) {
		if (pJamGut->field_0x368.IsValid()) {
			pJamGut->field_0x368.Kill();
			pJamGut->field_0x368.Reset();
		}
	}

	return;
}

void CBehaviourJamGutRidden::InitState(int newState)
{
	int iVar1;
	StateConfig* pSVar3;
	uint uVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CCollision* pCol;
	CActorJamGut* pJamGut;

	pJamGut = this->pOwner;
	pCol = pJamGut->pCollisionData;
	pJamGut->field_0x3a4 = GetTimer()->scaledTotalTime;
	pJamGut->field_0x3a8 = GetTimer()->scaledTotalTime;

	if ((pJamGut->GetStateFlags(pJamGut->actorState) & 0x100) == 0){
		pCol->flags_0x0 = pCol->flags_0x0 & 0xffffffcf;
	}
	else {
		pCol->flags_0x0 = pCol->flags_0x0 | 0x30;
	}

	if (newState == 0x13) {
		eStack16.w = 0.0f;
		eStack16.x = 2.2f;
		eStack16.z = 2.2f;
		eStack16.y = 0.8f;
		eStack32.x = 0.0f;
		eStack32.z = 0.0f;
		eStack32.y = 0.78f;
		eStack32.w = 1.0f;
		pJamGut->ChangeCollisionSphere(0.4f, &eStack16, &eStack32);
	}
	else {
		if (newState == 0x10) {
			IMPLEMENTATION_GUARD(
			FUN_003732c0((int)pJamGut);)
		}
		else {
			if (newState == JAMGUT_RIDE_STATE_JUMP) {
				pJamGut->StateJamGutJumpInit();
			}
			else {
				if (newState == JAMGUT_BEHAVIOUR_RIDDEN) {
					pJamGut->StateJamGutStandInit();
				}
				else {
					if (newState == 7) {
						pJamGut->StateJamGutStandInit();
					}
				}
			}
		}
	}

	return;
}

void CBehaviourJamGutRidden::TermState(int oldState, int newState)
{
	CBehaviourHeroRideJamGut* pHeroRideJamGut;
	CActorHero* pHero;
	CActorJamGut* pJamGut;

	pJamGut = this->pOwner;
	if (oldState == 0x13) {
		pJamGut->RestoreCollisionSphere(0.0f);
	}
	else {
		if ((oldState == JAMGUT_RIDE_STATE_RUN) || (oldState == 9)) {
			pJamGut->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
			pHero = pJamGut->pRider;
			if ((pHero != (CActorHero*)0x0) && (pHero->curBehaviourId == HERO_BEHAVIOUR_RIDE_JAMGUT)) {
				pHeroRideJamGut = (CBehaviourHeroRideJamGut*)pHero->GetBehaviour(pHero->curBehaviourId);
				pHeroRideJamGut->SetSpeedAnim(1.0f);
			}
		}
		else {
			if (oldState == 0x10) {
				IMPLEMENTATION_GUARD(
				pJamGut->FUN_00373040();)
			}
		}
	}

	return;
}

int CBehaviourJamGutRidden::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActor* pCVar1;
	bool bVar2;
	uint uVar3;
	StateConfig* pSVar5;
	int iVar6;
	CActorJamGut* pJamGut;

	if (msg == 3) {
		pJamGut = this->pOwner;
		if (((pJamGut->GetStateFlags(pJamGut->actorState) & 1) == 0) && (pJamGut = this->pOwner, pJamGut->actorState != 0x10)) {
			iVar6 = pJamGut->ChooseStateDead(0, (int)pMsgParam, 0);
			pJamGut->SetState(iVar6, -1);

			return 1;
		}

		return 0;
	}

	if (msg == MESSAGE_KICKED) {
		pJamGut = this->pOwner;

		if (((pJamGut->GetStateFlags(pJamGut->actorState) & 1) == 0) || (pJamGut = this->pOwner, pJamGut->actorState == 0x10)) {
			return 0;
		}

		_msg_hit_param* pMsgHitParam = (_msg_hit_param*)pMsgParam;

		uVar3 = pMsgHitParam->projectileType;
		if ((((uVar3 < 3) || (uVar3 == 5)) || (uVar3 == 10)) || (uVar3 == 9)) {
			uVar3 = pJamGut->AccomplishHit(pJamGut->pRider, pSender, pMsgHitParam, (edF32VECTOR4*)0x0);
			return uVar3;
		}
	}
	else {
		if (msg == 1) {
			pJamGut = this->pOwner;
			return Timer::GetTimer()->scaledTotalTime < pJamGut->field_0x4fc ^ 1;
		}

		if (msg == 0x14) {
			pCVar1 = this->pOwner->pTrackedPathActor;
			if (pCVar1 == (CActor*)0x0) {
				bVar2 = false;
			}
			else {
				bVar2 = pCVar1->typeID != MOVING_PLATFORM;
			}

			if (bVar2) {
				return 1;
			}

			return 0;
		}

		if (msg == 0x12) {
			pCVar1 = this->pOwner->pTrackedPathActor;
			if (pCVar1 == (CActor*)0x0) {
				bVar2 = false;
			}
			else {
				bVar2 = pCVar1->typeID != MOVING_PLATFORM;
			}

			if (bVar2) {
				uVar3 = this->pOwner->GetHeroAction(2.25f, (CActorHero*)pSender);
				return uVar3;
			}
		}
		else {
			if (msg == 0x44) {
				pJamGut = this->pOwner;
				if (pJamGut->activePathIndex == -1) {
					this->field_0x8 = 1;
					this->pOwner->SetBehaviour(3, -1, -1);
				}
				else {
					pJamGut->pTrackedPathActor = (CActor*)pJamGut->pRider;
					pJamGut->aPaths[pJamGut->activePathIndex].pathPlane.InitTargetPos(&pJamGut->pTrackedPathActor->currentLocation, &pJamGut->pathPlaneOutData);
				}

				return 1;
			}
		}
	}

	return 0;
}
