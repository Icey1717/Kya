#ifndef ACTOR_MOVING_PLATFORM_H
#define ACTOR_MOVING_PLATFORM_H

#include "Types.h"
#include "Actor.h"

struct S_STREAM_MPF_NO_FRICTION_ZONE {
	int field_0x0;
	char* field_0x4;
};

class CActorMovingPlatform : public CActorMovable {
public:
	CActorMovingPlatform(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	int noFrictionZoneCount;
	S_STREAM_MPF_NO_FRICTION_ZONE* aNoFrictionZones;
};

#endif //ACTOR_MOVING_PLATFORM_H