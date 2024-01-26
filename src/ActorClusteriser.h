#ifndef ACTOR_CLUSTERISER_H
#define ACTOR_CLUSTERISER_H

#include "Types.h"
#include "Actor.h"

class CBehaviourClusteriserZones : public CBehaviour
{

};

class CActorClusteriser : public CActor 
{
public:
	CActorClusteriser(){
		IMPLEMENTATION_GUARD_LOG()
	}

	CBehaviourClusteriserZones behaviourClusteriserZones;

	virtual CBehaviour* BuildBehaviour(int behaviourType);
};

#endif //ACTOR_CLUSTERISER_H