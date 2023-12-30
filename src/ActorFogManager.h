#ifndef ACTOR_FOG_MANAGER_H
#define ACTOR_FOG_MANAGER_H

#include "Types.h"
#include "Actor.h"

struct CSectorHierarchy;

class CActorFogManager : public CActor {
public:
	CActorFogManager(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	int field_0x16c;
	CSectorHierarchy* field_0x170;
};

#endif //ACTOR_FOG_MANAGER_H