#include "ActorClusteriser.h"

CBehaviour* CActorClusteriser::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourClusteriserZones;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}
	return pBehaviour;
}
