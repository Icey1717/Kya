#ifndef ACTOR_ADD_ON_PURCHASE_H
#define ACTOR_ADD_ON_PURCHASE_H

#include "Types.h"
#include "Actor.h"

class CActorAddOnPurchase : public CActor {
public:
	CActorAddOnPurchase(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_ADD_ON_PURCHASE_H