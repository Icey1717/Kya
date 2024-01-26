#include "ActorFighter.h"

CBehaviour* CActorFighter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 6) {
		pNewBehaviour = new CBehaviourFighterSlave;
	}
	else {
		if (behaviourType == 5) {
			pNewBehaviour = new CBehaviourFighterRidden;
		}
		else {
			if (behaviourType == 4) {
				pNewBehaviour = new CBehaviourFighterProjected;
			}
			else {
				if (behaviourType == 3) {
					pNewBehaviour = new CBehaviourFighter;
				}
				else {
					pNewBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
				}
			}
		}
	}
	return pNewBehaviour;
}

bool CActorFighter::IsFightRelated(int param_2)
{
	bool bVar1;

	if ((param_2 - 3U < 3) || (param_2 == 6)) {
		bVar1 = true;
	}
	else {
		bVar1 = false;
	}
	return bVar1;
}

void CBehaviourFighter::Init(CActor* pOwner)
{
	this->pOwner = pOwner;
	this->field_0x8 = 3;
	return;
}
