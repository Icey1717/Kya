#include "ActorRope.h"
#include "MemoryStream.h"

void CActorRope::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

CBehaviour* CActorRope::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourRopeStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}
