#include "ActorHedgehog.h"
#include "MemoryStream.h"

void CActorHedgehog::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
