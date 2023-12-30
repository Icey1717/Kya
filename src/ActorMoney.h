#ifndef ACTOR_MONEY_H
#define ACTOR_MONEY_H

#include "Types.h"
#include "Actor.h"

class CActorMoney : public CActorMovable {
public:
	CActorMoney(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MONEY_H