#ifndef ACTOR_BLAZER_H
#define ACTOR_BLAZER_H

#include "Types.h"
#include "Actor.h"
#include "PathFollow.h"
#include "Vision.h"

class CActorBlazer : public CActor {
public:
	CActorBlazer(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CVision vision;
	CAddOnGenerator addOnGenerator;
	CPathFollowReader pathFollowReader;
};

#endif //ACTOR_BLAZER_H