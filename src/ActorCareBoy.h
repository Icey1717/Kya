#ifndef ACTOR_CARE_BOY_H
#define ACTOR_CARE_BOY_H

#include "Types.h"
#include "Actor.h"

class CActorCareBoy : public CActor {
public:
	CActorCareBoy(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_CARE_BOY_H