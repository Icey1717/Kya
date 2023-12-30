#include "ActorTrap.h"
#include "MemoryStream.h"

void CActorTrap::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
