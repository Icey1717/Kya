#ifndef ACTOR_EXPLOSIVE_DISTRIBUTOR_H
#define ACTOR_EXPLOSIVE_DISTRIBUTOR_H

#include "Types.h"
#include "Actor.h"

class CActorExplosiveDistributor : public CActor {
public:
	CActorExplosiveDistributor(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_EXPLOSIVE_DISTRIBUTOR_H