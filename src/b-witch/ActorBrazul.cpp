#include "ActorBrazul.h"
#include "MemoryStream.h"

void CActorBrazul::Create(ByteCode* pByteCode)
{
	CActorFighter::Create(pByteCode);
	SkipToNextActorNoBase(pByteCode);
}
