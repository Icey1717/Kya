#include "ActorMiniGame.h"
#include "MemoryStream.h"

void CActorMiniGame::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

void CActorMiniGame::FUN_003ace00()
{
	this->field_0x1d8 = this->field_0x1d8 - 1;
}
