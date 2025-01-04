#include "ActorBlazer.h"
#include "MemoryStream.h"

void CActorBlazer::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
