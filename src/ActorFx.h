#ifndef ACTOR_FX_H
#define ACTOR_FX_H

#include "Types.h"
#include "Actor.h"

class CActorFx : public CActor {
public:
	CActorFx(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_FX_H