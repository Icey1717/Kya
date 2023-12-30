#ifndef ACTOR_WOLFEN_H
#define ACTOR_WOLFEN_H

#include "Types.h"
#include "Actor.h"

class CActorWolfen : public CActor {
public:
	CActorWolfen(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_WOLFEN_H