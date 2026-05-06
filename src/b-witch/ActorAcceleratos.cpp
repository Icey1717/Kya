#include "ActorAcceleratos.h"
#include "MemoryStream.h"
#include "CollisionManager.h"

void CActorAcceleratos::Create(ByteCode* pByteCode)
{
	S_ACCELERATOS_CONFIG* pSVar1;
	CActorSound* pCVar2;
	CCollision* pCol;

	CActor::Create(pByteCode);

	pSVar1 = reinterpret_cast<S_ACCELERATOS_CONFIG*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pSVar1 + 1);
	this->pConfig = pSVar1;
	if (((this->pConfig->field_0xc).index == -1) || ((this->pConfig->field_0x10).index == -1)) {
		this->field_0x164 = (CActorSound*)0x0;
	}
	else {
		pCVar2 = CreateActorSound(2);
		this->field_0x164 = pCVar2;
	}

	pCol = this->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 | 0xa000;
	pCol = this->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 & 0xfffffffe;

	return;
}

void CActorAcceleratos::Init()
{
	S_ACCELERATOS_CONFIG* pSVar1;

	CActor::Init();

	pSVar1 = this->pConfig;

	pSVar1->field_0xc.Init();
	pSVar1->field_0x10.Init();

	this->field_0x168 = 0;
	this->field_0x16c = (CActor*)0x0;

	return;
}

void CActorAcceleratos::Reset()
{
	CActor::Reset();

	this->field_0x168 = 0;
	this->field_0x16c = (CActor*)0x0;

	return;
}

CBehaviour* CActorAcceleratos::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == ACCELERATOS_BEHAVIOUR_STAND) {
		pNewBehaviour = &this->behaviourStand;
	}
	else {
		pNewBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pNewBehaviour;
}

StateConfig CActorAcceleratos::_gStateCfg_ACC[1] =
{
	StateConfig(0, 0)
};

StateConfig* CActorAcceleratos::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 1);
		pStateConfig = _gStateCfg_ACC + state + -5;
	}

	return pStateConfig;
}

int CActorAcceleratos::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;

	if (msg == 0x1c) {
		if ((pSender->typeID == ACTOR_HERO_PRIVATE) && (this->field_0x16c == (CActor*)0x0)) {
			this->field_0x16c = pSender;
			result = 1;
			this->field_0x168 = 1;
		}
		else {
			result = 0;
		}
	}
	else {
		result = CActor::InterpretMessage(pSender, msg, static_cast<_msg_params_get_position*>(pMsgParam));
	}

	return result;
}

void CActorAcceleratos::BehaviourAcceleratosStand_Manage()
{
	int iVar1;
	edF32VECTOR4 local_40;
	_msg_params_boost boostParams;

	if (this->field_0x168 != 0) {
		boostParams.field_0x0 = this->rotationQuat;
		boostParams.field_0x10 = this->pConfig->field_0x0;
		boostParams.field_0x14 = this->pConfig->field_0x8;

		iVar1 = DoMessage(this->field_0x16c, MESSAGE_BOOST, &boostParams);
		if (iVar1 == 0) {
			if (this->field_0x164 != (CActorSound*)0x0) {
				this->field_0x164->SoundStart(this, 1, (this->pConfig->field_0x10).Get(), 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
			}
		}
		else {
			if (this->field_0x164 != (CActorSound*)0x0) {
				this->field_0x164->SoundStart(this, 0, (this->pConfig->field_0xc).Get(), 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
			}
		}

		this->field_0x168 = 0;
		this->timeInAir = 0.0f;
	}

	if (this->pConfig->field_0x4 < this->timeInAir) {
		this->field_0x16c = (CActor*)0x0;
	}

	if (this->pTiedActor != (CActor*)0x0) {
		local_40 = this->baseLocation;
		SV_UpdatePosition_Rel(&local_40, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	return;
}

void CBehaviourAcceleratos::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourAcceleratos::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorAcceleratos*>(pOwner);

	return;
}

void CBehaviourAcceleratosStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourAcceleratosStand::Manage()
{
	this->pOwner->BehaviourAcceleratosStand_Manage();

	return;
}

void CBehaviourAcceleratosStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourAcceleratos::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourAcceleratosStand::TermState(int oldState, int newState)
{
	return;
}
