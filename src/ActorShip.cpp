#include "ActorShip.h"
#include "MemoryStream.h"

void CActorShip::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
