#ifndef ACTOR_ARAIGNOS_H
#define ACTOR_ARAIGNOS_H

#include "Types.h"
#include "Actor.h"

class CActorAraignos : public CActorMovable {
public:
	CActorAraignos(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_ARAIGNOS_H