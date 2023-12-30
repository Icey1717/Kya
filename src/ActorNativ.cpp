#include "ActorNativ.h"
#include "MemoryStream.h"

void CActorNativ::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
