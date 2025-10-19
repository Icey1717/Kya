#include "ActorAddOnPurchase.h"
#include "MemoryStream.h"

void CActorAddOnPurchase::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);
	inventoryInfo.Create(pByteCode);
	inventoryInfo.Init(this);

	return;
}

void CActorAddOnPurchase::Reset()
{
	CActor::Reset();

	inventoryInfo.Reset();

	return;
}

CBehaviour* CActorAddOnPurchase::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == ADD_ON_PURCHASE_BEHAVIOUR_DEFAULT) {
		pBehaviour = &this->behaviourAddOnPurchase;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

// Should be in: D:/Projects/b-witch/ActorAddOnPurchase.h
CInventoryInfo* CActorAddOnPurchase::GetInventoryInfo()
{
	return &this->inventoryInfo;
}

int CActorAddOnPurchase::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;

	iVar1 = this->inventoryInfo.ProcessMessage(this, msg, pMsgParam);

	if (iVar1 == 0) {
		iVar1 = CActor::InterpretMessage(pSender, msg, (GetPositionMsgParams*)pMsgParam);
	}
	else {
		iVar1 = 1;
	}

	return iVar1;
}

void CBhvAddOnPurchase::Create(ByteCode* pByteCode)
{
	return;
}

// Should be in: D:/Projects/b-witch/ActorAddOnPurchase.h
void CBhvAddOnPurchase::Manage()
{
	return;
}

void CBhvAddOnPurchase::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	pOwner->SetState(0x4, -1);
}

// Should be in: D:/Projects/b-witch/ActorAddOnPurchase.h
void CBhvAddOnPurchase::End(int newBehaviourId)
{
	return;
}