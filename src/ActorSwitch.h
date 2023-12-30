#ifndef ACTOR_SWITCH_H
#define ACTOR_SWITCH_H

#include "Types.h"
#include "Actor.h"

class CActorSwitch : public CActor {
public:
	CActorSwitch(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_SWITCH_H