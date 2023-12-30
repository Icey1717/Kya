#include "ActorWeapon.h"
#include "MemoryStream.h"

void CActorWeapon::Create(ByteCode* pByteCode)
{
	float fVar1;

	CActorMovable::Create(pByteCode);
	//this->field_0x1d4 = 0;
	fVar1 = pByteCode->GetF32();
	//this->field_0x1d8 = fVar1;
	fVar1 = pByteCode->GetF32();
	//this->field_0x1dc = fVar1;
	//this->field_0x1d0 = 0;
	return;
}
