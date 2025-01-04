#include "ActorCredits.h"
#include "MemoryStream.h"

void CActorCredits::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
