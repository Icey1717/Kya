#include "ActorFx.h"
#include "MemoryStream.h"

void CActorFx::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
