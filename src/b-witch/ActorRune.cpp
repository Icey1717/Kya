#include "ActorRune.h"
#include "MemoryStream.h"

void CActorRune::Create(ByteCode* pByteCode)
{
	int iVar1;
	float fVar2;

	CActor::Create(pByteCode);
	iVar1 = pByteCode->GetS32();
	//this->field_0x164 = iVar1;
	fVar2 = pByteCode->GetF32();
	//this->field_0x160 = fVar2;
	iVar1 = pByteCode->GetS32();
	//this->field_0x168 = iVar1;
	iVar1 = pByteCode->GetS32();
	//this->field_0x16c = iVar1;
	iVar1 = pByteCode->GetS32();
	//this->field_0x170 = iVar1;
	iVar1 = pByteCode->GetS32();
	//this->field_0x174 = iVar1;
	//if (this->field_0x170 != -1) {
	//	CActor::SV_InstallMaterialId(this->field_0x170);
	//}
	//if (this->field_0x174 != -1) {
	//	CActor::SV_InstallMaterialId(this->field_0x174);
	//}
	return;
}
