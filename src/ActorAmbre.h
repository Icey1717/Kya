#ifndef ACTOR_AMBRE_H
#define ACTOR_AMBRE_H

#include "Types.h"
#include "Actor.h"

class CActorAmbre : public CActor {
public:
	CActorAmbre(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_AMBRE_H