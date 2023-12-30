#ifndef ACTOR_D_C_A_H
#define ACTOR_D_C_A_H

#include "Types.h"
#include "Actor.h"

// A.K.A AIRCANON

class CActorDCA : public CActor {
public:
	CActorDCA(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_D_C_A_H