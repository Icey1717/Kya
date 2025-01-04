#include "ActorStiller.h"
#include "MemoryStream.h"

void CActorStiller::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
	return;
}
