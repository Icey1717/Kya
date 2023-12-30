#include "ActorBonusFountain.h"
#include "MemoryStream.h"

void CActorBonusFountain::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);
	this->addOnGenerator.Create(this, pByteCode);
	return;
}

