#ifndef ACTOR_SHOCKER_H
#define ACTOR_SHOCKER_H

#include "Types.h"
#include "Actor.h"

class CActorShocker : public CActorAutonomous {
public:
	CActorShocker(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_SHOCKER_H