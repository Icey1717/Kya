#include "ActorProjectile.h"
#include "MemoryStream.h"

void CActorProjectile::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
