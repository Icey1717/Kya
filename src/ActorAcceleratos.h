#ifndef ACTOR_ACCELERATOS_H
#define ACTOR_ACCELERATOS_H

#include "Types.h"
#include "Actor.h"

class CActorAcceleratos : public CActor {
public:
	CActorAcceleratos(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_ACCELERATOS_H