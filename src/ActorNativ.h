#ifndef ACTOR_NATIV_H
#define ACTOR_NATIV_H

#include "Types.h"
#include "Actor.h"

class CActorNativ : public CActor {
public:
	CActorNativ(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_NATIV_H