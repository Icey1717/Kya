#include "ActorEgg.h"
#include "MemoryStream.h"

void CActorEgg::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
