#ifndef ACTOR_ADD_ON_PURCHASE_H
#define ACTOR_ADD_ON_PURCHASE_H

#include "Types.h"
#include "Actor.h"
#include "InventoryInfo.h"

#define ADD_ON_PURCHASE_BEHAVIOUR_DEFAULT 0x2

class CBhvAddOnPurchase : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
};

class CActorAddOnPurchase : public CActor
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual CInventoryInfo* GetInventoryInfo();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CInventoryInfo inventoryInfo;
	CBhvAddOnPurchase behaviourAddOnPurchase;
};

#endif //ACTOR_ADD_ON_PURCHASE_H