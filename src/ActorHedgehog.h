#ifndef ACTOR_HEDGEHOG_H
#define ACTOR_HEDGEHOG_H

#include "Types.h"
#include "Actor.h"

class CActorHedgehog : public CActor {
public:
	static StateConfig _gStateCfg_ABV[28];

	CActorHedgehog(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_HEDGEHOG_H