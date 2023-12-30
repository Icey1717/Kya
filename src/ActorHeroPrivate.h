#ifndef ACTOR_HERO_PRIVATE_H
#define ACTOR_HERO_PRIVATE_H

#include "Types.h"
#include "Actor.h"

class CActorHeroPrivate : public CActor {
public:
	CActorHeroPrivate(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_HERO_PRIVATE_H