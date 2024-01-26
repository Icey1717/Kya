#include "ActorNativ.h"
#include "MemoryStream.h"

void CActorNativ::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

CBehaviour* CActorNativ::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	switch (behaviourType) {
	case 0x4:
		pBehaviour = &behaviourExorcism;
		break;
	case 0x6:
		pBehaviour = &behaviourTakeAndPut;
		break;
	default:
		assert(behaviourType < 3);
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
		break;
	}
	return pBehaviour;
}
