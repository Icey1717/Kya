#ifndef ACTOR_PROJECTILE_H
#define ACTOR_PROJECTILE_H

#include "Types.h"
#include "Actor.h"

class CActorProjectile : public CActor {
public:
	CActorProjectile(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_PROJECTILE_H