#ifndef ACTOR_AMORTOS_H
#define ACTOR_AMORTOS_H

#include "Types.h"
#include "Actor.h"

class CActorAmortos : public CActor {
public:
	CActorAmortos(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_AMORTOS_H