#include "ActorBombLauncher.h"
#include "MemoryStream.h"

void CBehaviourBombLauncher::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourBombLauncher::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorBombLauncher*>(pOwner);

	return;
}

int CBehaviourBombLauncher::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourBombLauncher::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourBombLauncherStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourBombLauncherStand::Manage()
{
	this->pOwner->fireShot.ManageShots();

	return;
}

void CBehaviourBombLauncherStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourBombLauncher::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourBombLauncherStand::InitState(int newState)
{
	return;
}

void CBehaviourBombLauncherStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourBombLauncherStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CActorBombLauncher::Create(ByteCode* pByteCode)
{
	float fVar1;

	CActor::Create(pByteCode);

	this->field_0x168 = pByteCode->GetF32();
	this->fireShot.Create(pByteCode);

	return;
}

void CActorBombLauncher::Init()
{
	CActor::Init();

	this->fireShot.Init();
	this->fireShot.Reset();

	return;
}

void CActorBombLauncher::Reset()
{
	CActor::Reset();

	this->fireShot.Reset();

	return;
}

CBehaviour* CActorBombLauncher::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorBombLauncher::gStateCfg_BLA[2] = {
	StateConfig(0x0, 0x0),
	StateConfig(0x6, 0x0)
};

StateConfig* CActorBombLauncher::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 2);
		pStateConfig = gStateCfg_BLA + state + -5;
	}

	return pStateConfig;
}

int CActorBombLauncher::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	int uVar2;

	if (msg == 0xf) {
		bVar1 = this->fireShot.Project(this->field_0x168, &this->currentLocation, &this->rotationQuat, (CActor*)0x0);
		uVar2 = bVar1 != false;
	}
	else {
		uVar2 = CActor::InterpretMessage(pSender, msg, (_msg_params_get_position*)pMsgParam);
	}

	return uVar2;
}
