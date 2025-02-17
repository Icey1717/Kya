#ifndef ACTOR_BONUS_FOUNTAIN_H
#define ACTOR_BONUS_FOUNTAIN_H

#include "Types.h"
#include "Actor.h"

class CActorBonusFountain : public CActor {
public:
	CActorBonusFountain(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_BONUS_FOUNTAIN_H