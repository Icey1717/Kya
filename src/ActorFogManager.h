#ifndef ACTOR_FOG_MANAGER_H
#define ACTOR_FOG_MANAGER_H

#include "Types.h"
#include "Actor.h"
#include "SectorManager.h"

class CFogHierarchy : public CSectorHierarchy {
public:
	float field_0x20;
	float field_0x24;
};

class CActorFogManager : public CActor {
public:
	CActorFogManager(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	int field_0x16c;
	CFogHierarchy* field_0x170;
};

#endif //ACTOR_FOG_MANAGER_H