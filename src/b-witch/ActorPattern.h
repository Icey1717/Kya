#ifndef ACTOR_PATTERN_H
#define ACTOR_PATTERN_H

#include "Types.h"
#include "Actor.h"

class CActorPattern : public CActor {
public:
	CActorPattern(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_PATTERN_H