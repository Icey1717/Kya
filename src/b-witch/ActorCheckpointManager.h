#ifndef ACTOR_CHECKPOINT_MANAGER_H
#define ACTOR_CHECKPOINT_MANAGER_H

#include "Types.h"
#include "Actor.h"

struct EventChunk_24 {};
class CWayPoint;
class CActorCheckpointManager;

class CBehaviourCheckpoint : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId) {}

	CActorCheckpointManager* pOwner;
};

struct S_CHECKPOINT {
	S_STREAM_REF<ed_zone_3d> pZone;
	S_STREAM_REF<CWayPoint> pWayPointA;
	int sectorId;
	S_STREAM_REF<CWayPoint> pWayPointB;
	uint flags;

	ActorAndWaypoint* aActorWaypoints;
	int actorWaypointsCount;
};

class CActorCheckpointManager : public CActor {
public:

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	void ManageCheckpoints();
	void ActivateCheckpoint(int index);

	CBehaviourCheckpoint behaviourCheckpoint;

	S_CHECKPOINT* aCheckpoints;
	int checkpointCount;

	int currentCheckpointIndex;
};

#endif //ACTOR_CHECKPOINT_MANAGER_H