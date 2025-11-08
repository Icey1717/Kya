#ifndef ACTOR_SHOOT_H
#define ACTOR_SHOOT_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"

class CActorShoot : public CActorAutonomous
{
public:
	CActorShoot(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_SHOOT_H