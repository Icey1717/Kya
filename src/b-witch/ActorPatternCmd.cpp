#include "ActorPatternCmd.h"
#include "MemoryStream.h"

void CActorPatternCmd::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
