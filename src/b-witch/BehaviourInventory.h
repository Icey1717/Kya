#ifndef _BEHAVIOUR_INVENTORY_H
#define _BEHAVIOUR_INVENTORY_H

#include "Types.h"
#include "Actor.h"
#include "InventoryInfo.h"

struct ByteCode;

class CBehaviourInventory : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	int initialBehaviourId;
	CInventoryInfo inventoryInfo;
	CActor* pOwner;
};

#endif // !_BEHAVIOUR_INVENTORY_H
