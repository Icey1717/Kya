#ifndef ACTOR_SHOCKER_H
#define ACTOR_SHOCKER_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"

class CActorShocker : public CActorAutonomous
{
public:
	CActorShocker(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_SHOCKER_H