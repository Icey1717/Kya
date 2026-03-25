#include "ActorCareBoy.h"
#include "MemoryStream.h"

CActorCareBoy::CActorCareBoy()
{
	this->field_0x1dc = 0;

	return;
}

void CActorCareBoy::Create(ByteCode* pByteCode)
{
	CActorMovable::Create(pByteCode);

	this->field_0x1dc = pByteCode->GetS32();
	this->field_0x1e0 = pByteCode->GetS32();
	
	return;
}

CBehaviour* CActorCareBoy::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == CARE_BOY_BEHAVIOUR_STEADY) {
		pBehaviour = &this->behaviourSteady;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorCareBoy::_gStateCfg_CRB[4] = {
	StateConfig(0x0, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorCareBoy::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActorMovable::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 4);
		pStateConfig = _gStateCfg_CRB + state + -5;
	}

	return pStateConfig;
}

int CActorCareBoy::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float unaff_f20;
	CNewFx* pNewFx;

	if (msg == 3) {
		if (this->actorState != 6) {
			CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x1e4, this->field_0x1e0, FX_MATERIAL_SELECTOR_NONE);
			this->field_0x1e4.SetPosition(&this->currentLocation);
			this->field_0x1e4.SetRotationEuler(&this->rotationEuler);
			this->field_0x1e4.Start();

			SetState(6, -1);

			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;

			DoMessage(this->pParent, MESSAGE_TRAP_CAUGHT, (MSG_PARAM)1);
		}
	}
	else {
		if (msg == 2) {
			_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
			if ((((pHitParam->projectileType == 10) && (this->field_0x1dc != 0)) && (0.0f < pHitParam->damage)) && (this->actorState != 6)) {
				DoMessage(this->pParent, (ACTOR_MESSAGE)0x4c, pMsgParam);
			}
		}
		else {
			if (msg == 0x42) {
				SetState(7, 7);
			}
			else {
				if (msg != 0x22) {
					iVar1 = CActorMovable::InterpretMessage(pSender, msg, pMsgParam);
					return iVar1;
				}

				SetState(7, reinterpret_cast<int>(pMsgParam));
			}
		}
	}

	return 0;
}

void CActorCareBoy::LinkToParent(CActor* pLinkedActor, uint key)
{
	CActor::LinkToActor(pLinkedActor, key, 1);

	this->pParent = pLinkedActor;

	return;
}

void CBehaviourCareBoy::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourCareBoy::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorCareBoy*>(pOwner);

	return;
}

int CBehaviourCareBoy::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourCareBoy::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourCareBoySteady::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourCareBoySteady::Manage()
{
	CActorCareBoy* pCareBoy;

	pCareBoy = this->pOwner;
	if (pCareBoy->actorState == 7) {
		if (pCareBoy->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			(static_cast<CActor*>(pCareBoy))->SetState(5, -1);
		}
	}

	return;
}

void CBehaviourCareBoySteady::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourCareBoy::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourCareBoySteady::InitState(int newState)
{
	return;
}

void CBehaviourCareBoySteady::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourCareBoySteady::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}