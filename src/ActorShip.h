#ifndef ACTOR_SHIP_H
#define ACTOR_SHIP_H

#include "Types.h"
#include "Actor.h"

class CActorShip : public CActor {
public:
	CActorShip(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_SHIP_H