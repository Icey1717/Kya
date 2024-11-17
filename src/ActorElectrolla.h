#ifndef ACTOR_ELECTROLLA_H
#define ACTOR_ELECTROLLA_H

#include "Types.h"
#include "Actor.h"

class CActorElectrolla : public CActor {
public:
	CActorElectrolla(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_ELECTROLLA_H