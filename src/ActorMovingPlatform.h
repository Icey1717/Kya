#ifndef ACTOR_MOVING_PLATFORM_H
#define ACTOR_MOVING_PLATFORM_H

#include "Types.h"
#include "Actor.h"

struct S_STREAM_MPF_NO_FRICTION_ZONE {
	int field_0x0;
	char* field_0x4;
};

class CBehaviourPlatform : public CBehaviour
{

};

class CBehaviourPlatformStand : public CBehaviourPlatform
{
public:
	
};

class CBehaviourPlatformTrajectory : public CBehaviourPlatform
{
public:

};

class CBehaviourPlatformSlab : public CBehaviourPlatform
{
public:

};

class CBehaviourPlatformDestroyed : public CBehaviourPlatform
{
public:

};

class CBehaviourWeighingMachine : public CBehaviourPlatform
{
public:

};

class CBehaviourWeighingMachineSlave : public CBehaviourWeighingMachine
{
public:

};

class CBehaviourTeleportRandom : public CBehaviourWeighingMachine
{
public:

};

class CBehaviourSelector : public CBehaviour
{
public:

};

class CBehaviourSelectorSlave : public CBehaviourSelector
{
public:

};

class CBehaviourSelectorNew : public CBehaviourSelector
{
public:

};

class CActorMovingPlatform : public CActorMovable
{
public:
	CActorMovingPlatform(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	int noFrictionZoneCount;
	S_STREAM_MPF_NO_FRICTION_ZONE* aNoFrictionZones;
};

#endif //ACTOR_MOVING_PLATFORM_H