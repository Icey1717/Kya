#include "ActorEgg.h"
#include "MemoryStream.h"
#include "ActorHero.h"
#include "MathOps.h"
#include "EventManager.h"

void CActorEgg::Create(ByteCode* pByteCode)
{
	CActorMovable::Create(pByteCode);

	this->field_0x1d8 = pByteCode->GetF32();
	this->pathPlaneArray.Create(pByteCode);
	this->field_0x1dc = pByteCode->GetU32();
	this->field_0x1e0 = pByteCode->GetU32();

	return;
}

void CActorEgg::Init()
{
	this->pathPlaneArray.Init();
	this->field_0x200 = 3.0f;
	this->field_0x204 = 0;
	this->pathPlaneArray.Reset();
	CActorMovable::Init();

	return;
}

void CActorEgg::Reset()
{
	this->pathPlaneArray.Reset();
	this->field_0x200 = 3.0f;
	this->field_0x204 = 0;
	this->pathPlaneArray.Reset();
	CActorMovable::Reset();

	return;
}

struct S_SAVE_CLASS_EGG
{
	int field_0x0;
};

void CActorEgg::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_EGG* pSaveData = reinterpret_cast<S_SAVE_CLASS_EGG*>(pData);

	if (this->field_0x204 == 0) {
		pSaveData->field_0x0 = 0;
	}
	else {
		pSaveData->field_0x0 = 1;
	}

	return;
}

void CActorEgg::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_EGG* pSaveData = reinterpret_cast<S_SAVE_CLASS_EGG*>(pData);

	if (mode == 1) {
		if (pSaveData->field_0x0 == 1) {
			this->field_0x204 = 1;
		}
		else {
			if (pSaveData->field_0x0 == 0) {
				this->field_0x204 = 0;
			}
		}
	}

	if (this->curBehaviourId == EGG_BEHAVIOUR_MOVE) {
		if (this->field_0x204 == 0) {
			SetState(5, -1);
		}
		else {
			SetState(0xc, -1);
		}
	}

	return;
}

CBehaviour* CActorEgg::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == EGG_BEHAVIOUR_MOVE) {
		pBehaviour = &this->behaviourEggMove;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorEgg::_gStateCfg_EGG[8] =
{
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x7, 0x0),
	StateConfig(0x8, 0x0),
	StateConfig(0x8, 0x0),
};

StateConfig* CActorEgg::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_EGG + state + -5;
	}

	return pStateConfig;
}

int CActorEgg::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return CActorMovable::InterpretMessage(pSender, msg, pMsgParam);
}

void CActorEgg::FUN_00356820()
{
	float fVar1;
	float fVar2;
	bool bVar3;
	CPathPlane* pPathPlane;
	float fVar4;
	float fVar5;
	SV_MOV_PATH_PARAM movPathParam;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;
	fVar5 = this->field_0x1d8;
	pPathPlane = this->pathPlaneArray.GetCurPathPlane();
	fVar5 = SV_MOV_ComputeDistIdealPos(&pPathPlane->pathFollowReader, this->field_0x1f0.delay, fVar5);
	this->field_0x1f0.delay = fVar5;
	if ((this->field_0x1e0 & 1) != 0) {
		this->field_0x1f0.field_0x4 = 1;
	}

	pPathPlane = this->pathPlaneArray.GetCurPathPlane();
	SV_MOV_ComputeSpeedAccelerationToFleeActor(3.0f, pHero, pPathPlane, &this->field_0x1f0);
	movPathParam.field_0x10 = 0;
	movPathParam.field_0x8 = 2;
	movPathParam.field_0xc = 2;
	movPathParam.acceleration = this->field_0x1f0.acceleration;
	movPathParam.speed = this->field_0x1f0.speed;
	pPathPlane = this->pathPlaneArray.GetCurPathPlane();
	fVar4 = SV_MOV_ManageMovOnPath(&pPathPlane->pathFollowReader, &movPathParam);
	ManageDyn(4.0f, 0, (CActorsTable*)0x0);
	fVar5 = pHero->currentLocation.x - this->currentLocation.x;
	fVar1 = pHero->currentLocation.y - this->currentLocation.y;
	fVar2 = pHero->currentLocation.z - this->currentLocation.z;
	if (this->field_0x1f0.delay * 3.0f < sqrtf(fVar5 * fVar5 + fVar1 * fVar1 + fVar2 * fVar2)) {
		SetState(9, -1);
	}
	else {
		fVar5 = FUN_00120250(0.5f);
		if (fVar4 < fVar5) {
			bVar3 = this->pathPlaneArray.FUN_001bffd0();
			if (bVar3 != false) {
				pPathPlane = this->pathPlaneArray.GetCurPathPlane();
				bVar3 = pPathPlane->pathFollowReader.AtGoal((pPathPlane->pathFollowReader).splinePointIndex, (pPathPlane->pathFollowReader).field_0xc);
				if (bVar3 != false) {
					SetState(8, -1);
					return;
				}
			}

			this->pathPlaneArray.NextWayPoint();
		}
	}

	return;
}

void CActorEgg::BehaviourEggMove_Manage()
{
	uint zoneId;
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar4;
	CActorHero* pHero;
	CEventManager* pCVar6;
	ed_zone_3d* pZone;
	int iVar7;
	CPathPlane* pCVar8;
	float fVar9;
	float fVar10;

	switch (this->actorState) {
	case 5:
		ManageDyn(4.0f, 0, (CActorsTable*)0x0);

		pCVar6 = CScene::ptable.g_EventManager_006f5080;
		pHero = CActorHero::_gThis;
		zoneId = this->field_0x1dc;

		if (zoneId == 0xffffffff) {
		LAB_00356ac8:
			bVar3 = false;
		}
		else {
			pZone = (ed_zone_3d*)0x0;
			if (zoneId != 0xffffffff) {
				pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, zoneId);
			}

			iVar7 = edEventComputeZoneAgainstVertex
			(pCVar6->activeChunkId, pZone,
				&pHero->currentLocation, 0);
			if (iVar7 != 1) goto LAB_00356ac8;
			bVar3 = true;
		}
		if (bVar3) {
			iVar7 = this->pathPlaneArray.GetNbPathPlane();
			if (iVar7 != 0) {
				pCVar8 = this->pathPlaneArray.GetCurPathPlane();
				pCVar8->pathFollowReader.NextWayPoint();
				SetState(6, -1);
			}
		}
		break;
	case 6:
		FUN_00356820();
		break;
	case 7:
		fVar10 = this->timeInAir;
		pCVar8 = this->pathPlaneArray.GetPathPlane(0);
		fVar9 = pCVar8->pathFollowReader.GetDelay();
		if (fVar9 < fVar10) {
			pCVar8 = this->pathPlaneArray.GetPathPlane(0);
			pCVar8->pathFollowReader.NextWayPoint();
			SetState(6, -1);
		}
		else {
			this->dynamic.speed = 0.0;
		}
		ManageDyn(4.0f, 0, (CActorsTable*)0x0);
		break;
	case 8:
		ManageDyn(4.0f, 0, (CActorsTable*)0x0);
		SetState(10, -1);
		break;
	case 9:
		ManageDyn(4.0f, 0, (CActorsTable*)0x0);
		fVar9 = pHero->currentLocation.x - this->currentLocation.x;
		fVar10 = pHero->currentLocation.y - this->currentLocation.y;
		fVar4 = pHero->currentLocation.z - this->currentLocation.z;
		if (sqrtf(fVar9 * fVar9 + fVar10 * fVar10 + fVar4 * fVar4) < this->field_0x1d8 * 3.0f) {
			SetState(6, -1);
		}
		break;
	case 10:
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0, (CActorsTable*)0x0);
		if (1.5f < this->timeInAir) {
			SetState(0xb, -1);
		}
		break;
	case 0xb:
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0, (CActorsTable*)0x0);
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->field_0x204 = 1;
			SetState(0xc, -1);
		}
		break;
	case 0xc:
		ManageDyn(4.0f, 0, (CActorsTable*)0x0);
	}
	return;
}

void CBehaviourEgg::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourEgg::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorEgg*>(pOwner);

	return;
}

int CBehaviourEgg::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourEgg::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourEggMove::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourEggMove::Manage()
{
	this->pOwner->BehaviourEggMove_Manage();

	return;
}

void CBehaviourEggMove::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourEgg::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		CActorEgg* pEgg = this->pOwner;

		if (pEgg->field_0x204 == 0) {
			pEgg->SetState(5, -1);
		}
		else {
			pEgg->SetState(0xc, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourEggMove::InitState(int newState)
{
	int iVar1;
	edF32VECTOR4* peVar2;
	CPathPlane* pPathPlane;
	float fVar3;
	float fVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CActorEgg* pEgg;
	CActorHero* pHero;

	pHero = CActorHero::_gThis;
	pEgg = this->pOwner;
	if (newState == 0xc) {
		iVar1 = pEgg->pathPlaneArray.GetNbPathPlane();
		pPathPlane = pEgg->pathPlaneArray.GetPathPlane(iVar1 + -1);
		peVar2 = pPathPlane->pathFollowReader.pPathFollow->GetGoal();
		pEgg->UpdatePosition(peVar2, true);
		pEgg->dynamic.speed = 0.0f;
	}
	else {
		if (newState == 6) {
			if (CActorHero::_gThis->dynamic.linearAcceleration == 0.0f) {
				pEgg->field_0x220 = 0.0f;
			}
			else {
				edF32Vector4NormalizeHard(&eStack32, &CActorHero::_gThis->dynamic.velocityDirectionEuler);
				pPathPlane = pEgg->pathPlaneArray.GetPathPlane(0);
				peVar2 = pPathPlane->pathFollowReader.GetWayPoint();
				edF32Vector4SubHard(&eStack16, peVar2, &pEgg->currentLocation);
				edF32Vector4NormalizeHard(&eStack16, &eStack16);
				fVar4 = pHero->dynamic.linearAcceleration;
				fVar3 = edF32Vector4DotProductHard(&eStack32, &eStack16);
				pEgg->field_0x220 = fVar3 * fVar4;
			}

			//pEgg->field_0x1f0 = pEgg->field_0x1d8;
			//pEgg->field_0x1fc = 0;
			//pEgg->field_0x1f8 = (void*)0x0;
			pPathPlane = pEgg->pathPlaneArray.GetCurPathPlane();
			pPathPlane->InitTargetPos(&pHero->currentLocation, &pPathPlane->outData);
		}
		else {
			if (newState == 5) {
				iVar1 = pEgg->pathPlaneArray.GetNbPathPlane();
				if (iVar1 != 0) {
					pPathPlane = pEgg->pathPlaneArray.GetPathPlane(0);
					peVar2 = pPathPlane->pathFollowReader.GetWayPoint();
					pEgg->UpdatePosition(peVar2, true);
				}
			}
		}
	}

	return;
}

void CBehaviourEggMove::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourEggMove::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
