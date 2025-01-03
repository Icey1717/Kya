#include "ActorFx.h"
#include "MemoryStream.h"

void CActorFx::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

CBehaviour* CActorFx::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;
	
	if (behaviourType == 4) {
		pBehaviour = new CBhvUnknown;
	}
	else if (behaviourType == 3) {
		pBehaviour = &this->bhvPath;
	}
	else if (behaviourType == 2) {
		pBehaviour = &this->bhvSingle;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}
