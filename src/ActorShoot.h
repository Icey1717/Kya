#ifndef ACTOR_SHOOT_H
#define ACTOR_SHOOT_H

#include "Types.h"
#include "Actor.h"

class CActorShoot : public CActorAutonomous {
public:
	CActorShoot(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_SHOOT_H