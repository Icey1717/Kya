#ifndef ACTOR_MONEY_H
#define ACTOR_MONEY_H

#include "Types.h"
#include "ActorMovable.h"

class CActorMoney : public CActorMovable {
public:
	CActorMoney(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
};

#endif //ACTOR_MONEY_H