#ifndef ACTOR_STILLER_H
#define ACTOR_STILLER_H

#include "Types.h"
#include "Actor.h"
#include "PathFollow.h"
#include "Vision.h"

class CActorStiller : public CActor {
public:
	CActorStiller(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_STILLER_H