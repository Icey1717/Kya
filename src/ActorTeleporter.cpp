#include "ActorTeleporter.h"
#include "MemoryStream.h"

void CActorTeleporter::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
