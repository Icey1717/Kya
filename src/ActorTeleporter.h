#ifndef ACTOR_TELEPORTER_H
#define ACTOR_TELEPORTER_H

#include "Types.h"
#include "Actor.h"

class CActorTeleporter : public CActor {
public:
	CActorTeleporter(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_TELEPORTER_H