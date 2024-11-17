#include "ActorAmortosCmd.h"
#include "MemoryStream.h"

void CActorAmortosCmd::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
