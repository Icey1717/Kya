#ifndef ACTOR_EGG_H
#define ACTOR_EGG_H

#include "Types.h"
#include "Actor.h"

class CActorEgg : public CActor {
public:
	CActorEgg(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_EGG_H