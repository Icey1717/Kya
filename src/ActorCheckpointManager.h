#ifndef ACTOR_CHECKPOINT_MANAGER_H
#define ACTOR_CHECKPOINT_MANAGER_H

#include "Types.h"
#include "Actor.h"

struct EventChunk_24 {};
class CWayPoint;

struct S_CHECKPOINT {
	ActorAndWaypoint* aActorWaypoints;
	int actorWaypointsCount;
};

class CActorCheckpointManager : public CActor {
public:
	CActorCheckpointManager(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	S_CHECKPOINT* aCheckpoints;
	int checkpointCount;
};

#endif //ACTOR_CHECKPOINT_MANAGER_H