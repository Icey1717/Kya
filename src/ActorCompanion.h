#ifndef ACTOR_COMPANION_H
#define ACTOR_COMPANION_H

#include "Types.h"
#include "ActorAutonomous.h"

class CActorCompanion : public CActorAutonomous {
public:
	CActorCompanion(){
		IMPLEMENTATION_GUARD_LOG()
	}

	static CActorCompanion* _gThis;

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_COMPANION_H