#ifndef ACTOR_WIND_H
#define ACTOR_WIND_H

#include "Types.h"
#include "Actor.h"
#include "SectorManager.h"

class CActorWind : public CActor {
public:
	CActorWind(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	CSectorHierarchy sectorObj;
};

#endif //ACTOR_WIND_H