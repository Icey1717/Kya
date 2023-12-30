#include "ActorSwitch.h"
#include "MemoryStream.h"

void CActorSwitch::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
