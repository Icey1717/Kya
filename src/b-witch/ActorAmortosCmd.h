#ifndef ACTOR_AMORTOS_CMD_H
#define ACTOR_AMORTOS_CMD_H

#include "Types.h"
#include "Actor.h"

class CActorAmortosCmd : public CActor {
public:
	CActorAmortosCmd(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_AMORTOS_CMD_H