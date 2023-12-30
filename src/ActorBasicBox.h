#ifndef ACTOR_BASIC_BOX_H
#define ACTOR_BASIC_BOX_H

#include "Types.h"
#include "Actor.h"

class CActorBasicBox : public CActor {
public:
	CActorBasicBox(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_BASIC_BOX_H