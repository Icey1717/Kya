#ifndef ACTOR_ELECTROLLA_H
#define ACTOR_ELECTROLLA_H

#include "Types.h"
#include "Actor.h"
#include "ActorManager.h"

class CActorElectrolla : public CActor {
public:
	CActorElectrolla(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

struct _criterion_near_params
{
	CActorsTable* pTable;
	float* aDistances;
	CActor* pActor;
	float nearestDistance;
	float field_0x10;
};

float GetActorsNearWithCriterion(float radius, CActor* pActor, CActorsTable* pTable, ColCallbackFuncPtr* pFunc);

#endif //ACTOR_ELECTROLLA_H