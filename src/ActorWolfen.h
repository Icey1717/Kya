#ifndef ACTOR_WOLFEN_H
#define ACTOR_WOLFEN_H

#include "Types.h"
#include "Actor.h"
#include "ActorFighter.h"

class CBehaviourWolfen : public CBehaviour
{
public:
};

class CBehaviourLost : public CBehaviourWolfen
{
public:
};

class CBehaviourExorcism : public CBehaviour
{
public:
};

class CBehaviourTrackStand : public CBehaviourWolfen
{
public:
};

class CBehaviourTrack : public CBehaviourTrackStand
{
public:
};

class CBehaviourWatchDog : public CBehaviourWolfen
{
public:
};

class CBehaviourSleep : public CBehaviourWatchDog
{
public:
};

class CBehaviourEscape : public CBehaviourWolfen
{
public:
};

class CBehaviourWolfenWeapon : public CBehaviourWolfen
{
public:
};

class CBehaviourSnipe : public CBehaviourWolfenWeapon
{
public:
};

class CBehaviourTrackWeaponStand : public CBehaviourWolfenWeapon
{
public:
};

class CBehaviourTrackWeapon : public CBehaviourTrackWeaponStand
{
public:
};

class CBehaviourTrackWeaponSnipe : public CBehaviourTrackWeaponStand
{
public:
};

class CBehaviourDCA : public CBehaviourWolfen
{
public:
};

class CBehaviourGuardArea : public CBehaviourWolfen
{
public:
};

class CBehaviourWolfenUnknown : public CBehaviourWolfen
{
public:
};

class CBehaviourAvoid : public CBehaviour
{
public:
};

class CActorWolfen : public CActorFighter 
{
public:
	CActorWolfen(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBehaviourLost behaviourLost;
	CBehaviourTrack behaviourTrack;
	CBehaviourExorcism behaviourExorcism;
};

#endif //ACTOR_WOLFEN_H