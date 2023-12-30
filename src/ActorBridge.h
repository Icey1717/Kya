#ifndef ACTOR_BRIDGE_H
#define ACTOR_BRIDGE_H

#include "Types.h"
#include "Actor.h"

class CActorBridge : public CActor {
public:
	CActorBridge(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_BRIDGE_H