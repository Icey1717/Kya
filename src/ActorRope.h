#ifndef ACTOR_ROPE_H
#define ACTOR_ROPE_H

#include "Types.h"
#include "Actor.h"

class CActorRope : public CActor {
public:
	CActorRope(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_ROPE_H