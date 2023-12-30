#include "ActorHeroPrivate.h"
#include "MemoryStream.h"

void CActorHeroPrivate::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}
