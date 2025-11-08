#ifndef ACTOR_BOX_H
#define ACTOR_BOX_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"

class CActorBox : public CActorAutonomous
{
public:
	CActorBox(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_BOX_H