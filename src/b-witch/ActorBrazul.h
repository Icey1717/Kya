#ifndef ACTOR_BRAZUL_H
#define ACTOR_BRAZUL_H

#include "Types.h"
#include "ActorFighter.h"

class CActorBrazul : public CActorFighter {
public:
	CActorBrazul(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_BRAZUL_H