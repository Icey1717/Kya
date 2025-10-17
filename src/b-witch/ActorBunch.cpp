#include "ActorBunch.h"
#include "MemoryStream.h"

void CActorBunch::Create(ByteCode* pByteCode)
{
	CActorFighter::Create(pByteCode);
	SkipToNextActorNoBase(pByteCode);
}
