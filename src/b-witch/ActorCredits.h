#ifndef ACTOR_CREDITS_H
#define ACTOR_CREDITS_H

#include "Types.h"
#include "Actor.h"

class CActorCredits : public CActor {
public:
	CActorCredits(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_CREDITS_H