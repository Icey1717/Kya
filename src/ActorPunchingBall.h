#ifndef ACTOR_PUNCHING_BALL_H
#define ACTOR_PUNCHING_BALL_H

#include "Types.h"
#include "Actor.h"

class CActorPunchingBall : public CActor {
public:
	CActorPunchingBall(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_PUNCHING_BALL_H