#include "ActorHunter.h"
#include "MemoryStream.h"

void CActorHunter::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
