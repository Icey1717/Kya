#include "ActorPattern.h"
#include "MemoryStream.h"

void CActorPattern::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
