#include "ActorAmortosCmd.h"
#include "MemoryStream.h"
#include "ActorAmortos.h"
#include "ActorFactory.h"
#include "ActorManager.h"

void CActorAmortosCmd::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->aAmortos = S_ACTOR_STREAM_REF::Create(pByteCode);

	return;
}

void CActorAmortosCmd::Init()
{
	CActor::Init();

	this->aAmortos->Init();
	this->field_0x164 = (CActor*)0x0;

	return;
}

void CActorAmortosCmd::Reset()
{
	CActor::Reset();

	this->field_0x164 = (CActor*)0x0;

	return;
}

CBehaviour* CActorAmortosCmd::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == AMORTOS_CMD_BEHAVIOUR_STAND) {
		pNewBehaviour = &this->behaviourStand;
	}
	else {
		pNewBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pNewBehaviour;
}

StateConfig CActorAmortosCmd::_gStateCfg_AMC[2] =
{
	StateConfig(0, 0),
	StateConfig(0, 0)
};

StateConfig* CActorAmortosCmd::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < AMORTOS_CMD_STATE_STAND) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - AMORTOS_CMD_STATE_STAND) < 2);
		pStateConfig = _gStateCfg_AMC + state - AMORTOS_CMD_STATE_STAND;
	}

	return pStateConfig;
}

int CActorAmortosCmd::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	S_ACTOR_STREAM_REF* pActorRef;
	int entryCount;
	int result;
	int i;

	if (msg == 0x41) {
		this->field_0x164 = pSender;
		i = 0;
		while (true) {
			pActorRef = this->aAmortos;
			entryCount = 0;
			if (pActorRef != (S_ACTOR_STREAM_REF*)0x0) {
				entryCount = pActorRef->entryCount;
			}

			if (entryCount <= i) break;

			DoMessage(pActorRef->aEntries[i].Get(), (ACTOR_MESSAGE)0x42, 0);
			i = i + 1;
		}

		result = 1;
	}
	else {
		result = CActor::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

static bool AmortosCmdReswellCallback(CActor* pActor, void* pParams)
{
	S_ACTOR_STREAM_REF* pActorRef;
	bool result;
	int entryCount;
	int i;
	CActorAmortos* pAmortos = reinterpret_cast<CActorAmortos*>(pParams);

	if ((((CActorFactory::gClassProperties[pActor->typeID].flags & 0x200) == 0) &&
		(pActor->IsKindOfObject(2) != false)) && (pAmortos != pActor)) {
		pActorRef = pAmortos->field_0x18c;
		i = 0;
		while (true) {
			entryCount = 0;
			if (pActorRef != (S_ACTOR_STREAM_REF*)0x0) {
				entryCount = pActorRef->entryCount;
			}

			if (entryCount <= i) break;

			if (pActorRef->aEntries[i].Get() == pActor) {
				return false;
			}

			i = i + 1;
		}

		result = true;
	}
	else {
		result = false;
	}

	return result;
}

void CActorAmortosCmd::StateAmortosCmdStand()
{
	S_ACTOR_STREAM_REF* pActorRef;
	CActorAmortos* pAmortos;
	CActorManager* pActorManager;
	bool bCanReswell;
	int entryCount;
	int i;
	CActorsTable table;
	edF32VECTOR4 intersectSphere;

	pActorManager = CScene::ptable.g_ActorManager_004516a4;
	bCanReswell = true;
	i = 0;
	intersectSphere.w = 2.3f;
	while (true) {
		table.nbEntries = 0;
		pActorRef = this->aAmortos;
		entryCount = 0;
		if (pActorRef != (S_ACTOR_STREAM_REF*)0x0) {
			entryCount = pActorRef->entryCount;
		}

		if (entryCount <= i) break;

		pAmortos = static_cast<CActorAmortos*>(pActorRef->aEntries[i].Get());
		if (pAmortos->actorState == AMORTOS_STATE_WAIT_RESWELL) {
			intersectSphere.xyz = pAmortos->currentLocation.xyz;
			pActorManager->cluster.GetActorsIntersectingSphereWithCriterion(&table, &intersectSphere, AmortosCmdReswellCallback, pAmortos);
			if (table.nbEntries != 0) {
				bCanReswell = false;
			}
		}

		i = i + 1;
	}

	if (bCanReswell) {
		i = 0;
		while (true) {
			pActorRef = this->aAmortos;
			entryCount = 0;
			if (pActorRef != (S_ACTOR_STREAM_REF*)0x0) {
				entryCount = pActorRef->entryCount;
			}

			if (entryCount <= i) break;

			pAmortos = static_cast<CActorAmortos*>(pActorRef->aEntries[i].Get());
			if ((pAmortos->actorState == AMORTOS_STATE_WAIT_RESWELL) && (pAmortos->field_0x160 < pAmortos->timeInAir)) {
				DoMessage(pAmortos, (ACTOR_MESSAGE)0x43, 0);
			}

			i = i + 1;
		}
	}

	return;
}

void CBehaviourAmortosCmd::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourAmortosCmd::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorAmortosCmd*>(pOwner);

	return;
}

int CBehaviourAmortosCmd::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourAmortosCmd::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourAmortosCmdStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourAmortosCmdStand::Manage()
{
	int state;

	state = this->pOwner->actorState;
	if ((state != AMORTOS_CMD_STATE_IDLE) && (state == AMORTOS_CMD_STATE_STAND)) {
		this->pOwner->StateAmortosCmdStand();
	}

	return;
}

void CBehaviourAmortosCmdStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourAmortosCmd::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(AMORTOS_CMD_STATE_STAND, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourAmortosCmdStand::InitState(int newState)
{
	return;
}

void CBehaviourAmortosCmdStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourAmortosCmdStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
