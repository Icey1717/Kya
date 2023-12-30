#ifndef ACTOR_RUNE_H
#define ACTOR_RUNE_H

#include "Types.h"
#include "Actor.h"

class CActorRune : public CActor {
public:
	CActorRune(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_RUNE_H