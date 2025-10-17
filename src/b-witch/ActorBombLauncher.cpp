#include "ActorBombLauncher.h"
#include "MemoryStream.h"

void CActorBombLauncher::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
