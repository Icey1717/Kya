#ifndef ACTOR_PATTERN_CMD_H
#define ACTOR_PATTERN_CMD_H

#include "Types.h"
#include "Actor.h"

class CActorPatternCmd : public CActor {
public:
	CActorPatternCmd(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_PATTERN_CMD_H