#ifndef ACTOR_WEAPON_H
#define ACTOR_WEAPON_H

#include "Types.h"
#include "ActorMovable.h"

class CActorWeapon : public CActorMovable {
public:
	CActorWeapon(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_WEAPON_H