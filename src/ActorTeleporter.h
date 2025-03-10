#ifndef ACTOR_TELEPORTER_H
#define ACTOR_TELEPORTER_H

#include "Types.h"
#include "Actor.h"
#include "ScenaricCondition.h"

class CBehaviourTeleporterDefault : public CBehaviour
{

};

class CActorTeleporter : public CActor {
public:
	CActorTeleporter(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	void UpdateCurTeleporterState(int levelId, int param_3);

	edDList_material* GetMySubSectorMaterial(int levelId, int nbAreas);

	ConditionedOperationArray condOpArray;

	CBehaviourTeleporterDefault behaviourTeleporterDefault;

	undefined4 field_0x2a0;
};

#endif //ACTOR_TELEPORTER_H