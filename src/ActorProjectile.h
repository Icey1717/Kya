#ifndef ACTOR_PROJECTILE_H
#define ACTOR_PROJECTILE_H

#include "Types.h"
#include "Actor.h"

class CBehaviourProjectile : public CBehaviour
{

};

class CBehaviourProjectileStand : public CBehaviourProjectile
{

};

class CBehaviourInventory : public CBehaviour
{

};

class CBehaviourProjectileStraight : public CBehaviourProjectile
{

};

class CBehaviourInactive : public CBehaviourProjectile
{

};

class CBehaviourProjectilePortable : public CBehaviourProjectile
{

};

class CActorProjectile : public CActorAutonomous 
{
public:
	CActorProjectile(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBehaviourProjectileStand behaviourProjectileStand;
};

#endif //ACTOR_PROJECTILE_H