#include "BehaviourInventory.h"
#include "MemoryStream.h"
#include "ActorProjectile.h"
#include "ActorHero.h"

void CBehaviourInventory::Create(ByteCode* pByteCode)
{
	inventoryInfo.Create(pByteCode);
	this->initialBehaviourId = pByteCode->GetS32();

	return;
}

void CBehaviourInventory::Init(CActor* pOwner)
{
	inventoryInfo.Init(pOwner);
	this->pOwner = pOwner;
}

void CBehaviourInventory::Manage()
{
	CActor* pActor;

	if ((uint)(this->inventoryInfo).field_0x0 < 2) {
		pActor = this->pOwner;
		if (((pActor->typeID == PROJECTILE) && (this->initialBehaviourId == 3)) && ((this->inventoryInfo).purchaseId == 0x1a)) {
			CActorProjectile* pProjectile = static_cast<CActorProjectile*>(pActor);
			pProjectile->SetBehaviour(PROJECTILE_BEHAVIOUR_STAND, 0xf, -1);
			pProjectile->pSoccerActor = CActorHero::_gThis;
		}
		else {
			pActor->SetBehaviour(this->initialBehaviourId, -1, -1);
		}
	}

	return;
}

void CBehaviourInventory::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActor* pActor;
	CInventoryInfo* pInventoryInfo;

	pInventoryInfo = this->pOwner->GetInventoryInfo();
	pInventoryInfo->Reset();
	pActor = this->pOwner;
	pActor->flags = pActor->flags & 0xfffffffd;
	pActor->flags = pActor->flags | 1;
	pActor = this->pOwner;
	pActor->flags = pActor->flags & 0xffffff7f;
	pActor->flags = pActor->flags | 0x20;
	pActor->EvaluateDisplayState();

	return;
}

void CBehaviourInventory::End(int newBehaviourId)
{
	CActor* pActor;

	this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
	pActor = this->pOwner;
	pActor->flags = pActor->flags & 0xffffff5f;
	pActor->EvaluateDisplayState();
	return;
}

int CBehaviourInventory::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	CActor* pActor;

	if (((uint)(this->inventoryInfo).field_0x0 < 2) || (this->inventoryInfo.ProcessMessage(this->pOwner, msg, pMsgParam) == 0)) {
		iVar1 = 0;
	}
	else {
		if ((msg == 0x5f) && (pMsgParam == (void*)0x4)) {
			pActor = this->pOwner;
			pActor->flags = pActor->flags & 0xffffff5f;
			pActor->EvaluateDisplayState();
			this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
		}
		if ((msg == 0x5f) && (pMsgParam == (void*)0x5)) {
			this->pOwner->PlayAnim(5);
		}

		if ((msg == 0x5e) && (*reinterpret_cast<int*>(pMsgParam) == 1)) {
			iVar1 = this->pOwner->GetIdMacroAnim(5);
			IMPLEMENTATION_GUARD(
				*(int*)((int)pMsgParam + 8) = iVar1;);
		}

		iVar1 = 1;
	}

	return iVar1;
}

int CBehaviourInventory::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}
