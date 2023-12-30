#ifndef ACTOR_TRAP_H
#define ACTOR_TRAP_H

#include "Types.h"
#include "Actor.h"

class CActorTrap : public CActor {
public:
	CActorTrap(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_TRAP_H