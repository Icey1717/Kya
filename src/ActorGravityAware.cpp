#include "ActorGravityAware.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "CinematicManager.h"

void CActorGravityAware::Create(ByteCode* pByteCode)
{
	CBehaviour* pCVar1;
	CActorSound* pCVar2;
	CCollision* pCollision;

	CActorAutonomous::Create(pByteCode);

	pCollision = this->pCollisionData;
	if (pCollision != (CCollision*)0x0) {
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x1000180;
	}

	this->flags = this->flags | 0x1000;

	pCVar1 = GetBehaviour(GRAVITY_AWARE_BEHAVIOUR_BELL);
	if ((pCVar1 == (CBehaviour*)0x0) && (pCVar1 = GetBehaviour(GRAVITY_AWARE_BEHAVIOUR_FALL), pCVar1 == (CBehaviour*)0x0)) {
		this->pActorSound = (CActorSound*)0x0;
	}
	else {
		this->pActorSound = CreateActorSound(1);
	}

	return;
}

void CActorGravityAware::Init()
{
	CActorAutonomous::Init();

	this->field_0x350 = 0.0f;
	this->field_0x354 = 0;

	return;
}

void CActorGravityAware::Reset()
{
	CActorAutonomous::Reset();

	this->field_0x350 = 0.0f;
	this->field_0x354 = 0;

	return;
}

void CActorGravityAware::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActorGravityAware::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorGravityAware::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == GRAVITY_AWARE_BEHAVIOUR_BELL) {
		pBehaviour = new CBehaviourGravityAwareBell;
	}
	else {
		if (behaviourType == GRAVITY_AWARE_BEHAVIOUR_FALL) {
			pBehaviour = &this->fallBehaviour;
		}
		else {
			pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

StateConfig CActorGravityAware::_gStateCfg_GRV[4] = {
	StateConfig(0x0, 0x0),
	StateConfig(0xc, 0x800),
	StateConfig(0xc, 0x800),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorGravityAware::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 6);
		pStateConfig = _gStateCfg_GRV + state + -6;
	}

	return pStateConfig;
}

int CActorGravityAware::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;

	if (msg == 0xd) {
		IMPLEMENTATION_GUARD(
		if (*(int*)((int)pMsgParam + 4) == 0) {
			SetBehaviour(this->subObjA->defaultBehaviourId, -1, -1);
		}
		else {
			SetBehaviour(GRAVITY_AWARE_BEHAVIOUR_FALL, -1, -1);
		}

		if (*(int*)((int)pMsgParam + 4) == 0) {
			this->field_0x354 = this->field_0x354 & 0xfe;
		}
		else {
			this->field_0x354 = this->field_0x354 | 1;
		}

		/* WARNING: Load size is inaccurate */
		CActor::UpdatePosition((CActor*)this, *pMsgParam, true);
		iVar1 = 1;)
	}
	else {
		if (((msg == 0xe) || (msg == 0x10)) || (msg == 0xf)) {
			SetBehaviour(GRAVITY_AWARE_BEHAVIOUR_FALL, -1, -1);
			iVar1 = 1;
		}
		else {
			iVar1 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
		}
	}

	return iVar1;
}

int CActorGravityAware::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	int iVar1;

	if ((*param_5 == 1) && ((uint)param_3 == 2)) {
		if (param_5[1] == 0) {
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;

			EvaluateDisplayState();

			this->field_0x354 = this->field_0x354 | 2;
		}
		iVar1 = 1;
	}
	else {
		iVar1 = CActor::InterpretEvent(pEventMessage, param_3, param_4, param_5);
	}

	return iVar1;
}

void CBehaviourGravityAwareFall::Create(ByteCode* pByteCode)
{
	this->soundRef.index = pByteCode->GetS32();

	return;
}

void CBehaviourGravityAwareFall::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorGravityAware*>(pOwner);
	this->soundRef.Init();

	return;
}

void CBehaviourGravityAwareFall::Manage()
{
	int iVar1;
	CSound* pSound;
	CCollision* pCollision;
	CActorGravityAware* pGravityAware;

	pGravityAware = this->pOwner;
	iVar1 = pGravityAware->actorState;
	if (iVar1 == 9) {
		pGravityAware->ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
		pCollision = pGravityAware->pCollisionData;
		if ((pCollision == (CCollision*)0x0) || ((pCollision->flags_0x4 & 2) == 0)) {
			pGravityAware->field_0x350 = 0.0f;
		}
		else {
			pGravityAware->field_0x350 = pGravityAware->field_0x350 + GetTimer()->cutsceneDeltaTime;
		}

		if (0.1f <= pGravityAware->field_0x350) {
			pSound = this->soundRef.Get();
			if (pSound != (CSound*)0x0) {
				IMPLEMENTATION_GUARD_AUDIO(
				pGravityAware->pActorSound->SoundStart(pGravityAware, 0, pSound, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);)
			}
			
			pGravityAware->SetState(6, -1);
			pGravityAware->flags = pGravityAware->flags & 0xfffbffff;
		}
	}
	else {
		if ((iVar1 == 6) &&
			((pGravityAware->pTiedActor == (CActor*)0x0 || ((pGravityAware->flags & 0x40000) == 0)))) {
			pGravityAware->flags = pGravityAware->flags | 0x200000;
			pGravityAware->flags = pGravityAware->flags & 0xfffbffff;
		}
	}

	return;
}

void CBehaviourGravityAwareFall::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CCollision* pCollision;

	if (newState == -1) {
		this->pOwner->SetState(9, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	pCollision = this->pOwner->pCollisionData;
	if (pCollision != (CCollision*)0x0) {
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x40;
	}

	return;
}

void CBehaviourGravityAwareFall::End(int newBehaviourId)
{
	CCollision* pCollision;

	pCollision = this->pOwner->pCollisionData;
	if (pCollision != (CCollision*)0x0) {
		pCollision->flags_0x0 = pCollision->flags_0x0 & ~0x40;
	}

	return;
}

void CBehaviourGravityAwareBell::Create(ByteCode* pByteCode)
{

}

void CBehaviourGravityAwareBell::Init(CActor* pOwner)
{

}

void CBehaviourGravityAwareBell::Manage()
{

}

void CBehaviourGravityAwareBell::Begin(CActor* pOwner, int newState, int newAnimationType)
{

}

void CBehaviourGravityAwareBell::End(int newBehaviourId)
{

}

void CBehaviourGravityAwareBell::InitState(int newState)
{

}

void CBehaviourGravityAwareBell::TermState(int oldState, int newState)
{

}

int CBehaviourGravityAwareBell::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	StateConfig* pSVar2;
	uint uVar3;
	CActorGravityAware* pGravityAware;

	if (msg == 0xe) {
		pGravityAware = this->pOwner;
		iVar1 = pGravityAware->actorState;
		if (iVar1 == -1) {
			uVar3 = 0;
		}
		else {
			pSVar2 = pGravityAware->GetStateCfg(iVar1);
			uVar3 = pSVar2->flags_0x4 & 0x800;
		}

		if ((pGravityAware->GetStateFlags(pGravityAware->actorState) & 0x800) == 0) {
			pGravityAware->SetState(7, -1);
		}
		else {
			pGravityAware->SetState(8, -1);
		}

		iVar1 = 1;
	}
	else {
		if (msg == 0x10) {
			pGravityAware->SetState(8, -1);
			iVar1 = 1;
		}
		else {
			if (msg == 0xf) {
				pGravityAware->SetState(7, -1);
				iVar1 = 1;
			}
			else {
				iVar1 = 0;
			}
		}
	}

	return iVar1;
}
