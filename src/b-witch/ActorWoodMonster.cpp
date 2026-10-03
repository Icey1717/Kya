#include "ActorWoodMonster.h"
#include "MemoryStream.h"
#include "ActorHero.h"
#include "CameraViewManager.h"
#include "WayPoint.h"

StateConfig CActorWoodMonster::_gStateCfg_WOODMONSTER[8] =
{
	StateConfig(0, 0),
	StateConfig(6, 0),
	StateConfig(6, 0),
	StateConfig(7, 0),
	StateConfig(8, 0),
	StateConfig(8, 0),
	StateConfig(7, 0),
	StateConfig(8, 0)
};

void CActorWoodMonster::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->field_0x160.x = pByteCode->GetF32();
	this->field_0x160.y = pByteCode->GetF32();
	this->field_0x160.z = pByteCode->GetF32();
	this->field_0x160.w = pByteCode->GetF32();

	return;
}

void CActorWoodMonster::Init()
{
	CActor::Init();

	this->field_0x170 = (CActorAutonomous*)0x0;

	return;
}

void CActorWoodMonster::Reset()
{
	CActor::Reset();

	this->field_0x170 = (CActorAutonomous*)0x0;

	return;
}

CBehaviour* CActorWoodMonster::BuildBehaviour(int behaviourType)
{
	if (behaviourType == WOODMONSTER_BEHAVIOUR_GET) {
		return &this->behaviourGet;
	}

	return CActor::BuildBehaviour(behaviourType);
}

StateConfig* CActorWoodMonster::GetStateCfg(int state)
{
	if (state < 5) {
		return CActor::GetStateCfg(state);
	}

	assert((state - 5) < 8);
	return _gStateCfg_WOODMONSTER + state + -5;
}

int CActorWoodMonster::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return CActor::InterpretMessage(pSender, msg, pMsgParam);
}

void CActorWoodMonster::StateWoodMonsterStand(CBehaviourWoodMonsterGet* pBehaviour)
{
	CActorHero* pHero = CActorHero::_gThis;
	CActorConeInfluence coneInfluence;

	coneInfluence.field_0x8 = this->field_0x160.x;
	coneInfluence.field_0x0 = this->field_0x160.z;
	coneInfluence.field_0x4 = this->field_0x160.z * 1.5f;
	coneInfluence.field_0xc = this->field_0x160.w;
	coneInfluence.field_0x10 = this->field_0x160.w * 0.5f;
	coneInfluence.field_0x14 = 2;
	coneInfluence.field_0x20.x = 0.0f;
	coneInfluence.field_0x20.y = 1.0f;
	coneInfluence.field_0x20.z = 0.0f;
	coneInfluence.field_0x20.w = 0.0f;

	float distance = SV_AttractActorInAConeAboveMe(pHero, &coneInfluence);
	if (distance < this->field_0x160.y) {
		if (-1 < pBehaviour->field_0x10) {
			CCameraManager::_gThis->PushCamera(pBehaviour->field_0x10, 0);
		}

		this->field_0x170 = pHero;
		SetState(WOODMONSTER_STATE_ATTRACT, -1);
	}

	return;
}

void CActorWoodMonster::StateWoodMonsterAttract()
{
	CActorConeInfluence coneInfluence;
	_msg_enter_shop shopParams;

	coneInfluence.field_0x8 = this->field_0x160.x;
	coneInfluence.field_0x0 = this->field_0x160.z;
	coneInfluence.field_0x4 = this->field_0x160.z * 1.5f;
	coneInfluence.field_0xc = this->field_0x160.w;
	coneInfluence.field_0x10 = this->field_0x160.w * 0.5f;
	coneInfluence.field_0x14 = 2;
	coneInfluence.field_0x20.x = 0.0f;
	coneInfluence.field_0x20.y = 1.0f;
	coneInfluence.field_0x20.z = 0.0f;
	coneInfluence.field_0x20.w = 0.0f;
	SV_AttractActorInAConeAboveMe(CActorHero::_gThis, &coneInfluence);

	if (0.4f < this->timeInAir) {
		DoMessage(this->field_0x170, (ACTOR_MESSAGE)0x61, (MSG_PARAM)0);
		shopParams.field_0x0 = 1;
		shopParams.field_0x4 = 0;
		shopParams.field_0x8 = 0;
		// The original payload contains only the first three fields.
		shopParams.field_0xc = 0;
		DoMessage(this->field_0x170, MESSAGE_ENTER_SHOP, &shopParams);
		SetState(WOODMONSTER_STATE_GET, -1);
	}

	return;
}

void CActorWoodMonster::BehaviourWoodMonsterGet_Manage(CBehaviourWoodMonsterGet* pBehaviour)
{
	struct S_WOODMONSTER_RELEASE_PARAMS
	{
		edF32VECTOR4* pPosition;
		edF32VECTOR3* pDestination;
		edF32VECTOR3* pRotation;
		float field_0xc;
	};

	edF32VECTOR4 position;
	S_WOODMONSTER_RELEASE_PARAMS releaseParams;

	switch (this->actorState) {
	case WOODMONSTER_STATE_STAND:
		StateWoodMonsterStand(pBehaviour);
		break;
	case WOODMONSTER_STATE_ATTRACT:
		StateWoodMonsterAttract();
		break;
	case WOODMONSTER_STATE_GET:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if (pBehaviour->field_0x8 < this->field_0x170->GetLifeInterface()->GetValue()) {
				// The PS2 call at 0x32350c retains the damage amount in f12.
				this->field_0x170->LifeDecrease(pBehaviour->field_0x8);
			}
			else {
				DoMessage(this->field_0x170, (ACTOR_MESSAGE)3, (MSG_PARAM)2);
				position = this->currentLocation;
				position.y = position.y + 1.0f;
				this->field_0x170->UpdatePosition(&position, true);
				this->field_0x170->flags = this->field_0x170->flags & 0xffffff7f;
				this->field_0x170->flags = this->field_0x170->flags | 0x20;
				this->field_0x170->EvaluateDisplayState();

				if (-1 < pBehaviour->field_0x10) {
					CCameraManager::_gThis->PopCamera(pBehaviour->field_0x10);
				}
			}

			SetState(WOODMONSTER_STATE_WAIT, -1);
		}
		break;
	case WOODMONSTER_STATE_WAIT:
		if ((2.0f < this->timeInAir) && ((this->field_0x170->GetStateFlags(this->field_0x170->actorState) & 1) == 0)) {
			SetState(WOODMONSTER_STATE_RELEASE, -1);
		}
		break;
	case WOODMONSTER_STATE_RELEASE:
		if (0.35f < this->timeInAir) {
			releaseParams.pPosition = &this->currentLocation;
			releaseParams.pDestination = &pBehaviour->wayPointRef.Get()->location;
			releaseParams.pRotation = &pBehaviour->wayPointRef.Get()->rotation;
			releaseParams.field_0xc = 3.0f;
			DoMessage(this->field_0x170, (ACTOR_MESSAGE)0x33, &releaseParams);

			if (-1 < pBehaviour->field_0x10) {
				CCameraManager::_gThis->PopCamera(pBehaviour->field_0x10);
			}

			SetState(WOODMONSTER_STATE_RELEASE_END, -1);
		}
		break;
	case WOODMONSTER_STATE_RELEASE_END:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(WOODMONSTER_STATE_STAND, -1);
		}
		break;
	case WOODMONSTER_STATE_WAIT_HERO_ANIM:
		if (this->field_0x170->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(WOODMONSTER_STATE_STAND, -1);
		}
		break;
	}

	if (this->pTiedActor != (CActor*)0x0) {
		position = this->baseLocation;
		SV_UpdatePosition_Rel(&position, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	return;
}

void CBehaviourWoodMonster::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourWoodMonster::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorWoodMonster*>(pOwner);
}

int CBehaviourWoodMonster::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourWoodMonster::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourWoodMonsterGet::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();
	this->wayPointRef.index = pByteCode->GetS32();
	this->field_0x10 = pByteCode->GetS32();

	return;
}

void CBehaviourWoodMonsterGet::Init(CActor* pOwner)
{
	this->wayPointRef.Init();

	return;
}

void CBehaviourWoodMonsterGet::Manage()
{
	this->pOwner->BehaviourWoodMonsterGet_Manage(this);

	return;
}

void CBehaviourWoodMonsterGet::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourWoodMonster::Begin(pOwner, newState, newAnimationType);
	if (newState == -1) {
		this->pOwner->SetState(WOODMONSTER_STATE_STAND, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourWoodMonsterGet::InitState(int newState)
{
	return;
}

void CBehaviourWoodMonsterGet::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourWoodMonsterGet::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
