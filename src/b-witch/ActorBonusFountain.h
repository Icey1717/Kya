#ifndef ACTOR_BONUS_FOUNTAIN_H
#define ACTOR_BONUS_FOUNTAIN_H

#include "Types.h"
#include "Actor.h"
#include "ActorBonusServices.h"

class CActorBonusFountain : public CActor
{
public:
	CActorBonusFountain(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_BONUS_FOUNTAIN_H