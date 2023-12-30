#include "ActorCompanion.h"
#include "MemoryStream.h"

CActorCompanion* CActorCompanion::_gThis = NULL;

void CActorCompanion::Create(ByteCode* pByteCode)
{
	CollisionData* pCVar1;
	int iVar2;
	int* piVar3;

	CActorCompanion::_gThis = this;
	CActorAutonomous::Create(pByteCode);
	//pCVar1 = this->field_0x2c;
	//if (pCVar1 != (CollisionData*)0x0) {
	//	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x2000;
	//	this->field_0x2c->flags_0x0 = this->field_0x2c->flags_0x0 & 0xffffefff;
	//}
	iVar2 = pByteCode->GetS32();
	//this->field_0x358 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0x35c = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0x360 = iVar2;
	//CActor::SV_InstallMaterialId(this->field_0x358);
	//CActor::SV_InstallMaterialId(this->field_0x35c);
	//CActor::SV_InstallMaterialId(this->field_0x360);
	return;
}
