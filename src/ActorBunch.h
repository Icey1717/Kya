#ifndef ACTOR_BUNCH_H
#define ACTOR_BUNCH_H

#include "Types.h"
#include "ActorFighter.h"

class CActorBunch : public CActorFighter {
public:
	CActorBunch(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_BUNCH_H