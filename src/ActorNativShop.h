#ifndef ACTOR_NATIV_SHOP_H
#define ACTOR_NATIV_SHOP_H

#include "Types.h"
#include "Actor.h"

class CActorNativShop : public CActor {
public:
	CActorNativShop(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_NATIV_SHOP_H