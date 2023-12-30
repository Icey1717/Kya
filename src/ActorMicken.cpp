#include "ActorMicken.h"
#include "MemoryStream.h"

void CActorMicken::Create(ByteCode* pByteCode)
{
	int iVar1;
	uint uVar2;
	CBehaviour* pCVar3;
	float fVar4;

	CActorAutonomous::Create(pByteCode);
	iVar1 = pByteCode->GetS32();
	//this->field_0x350 = iVar1;
	iVar1 = pByteCode->GetS32();
	//this->field_0x354 = iVar1;
	uVar2 = pByteCode->GetU32();
	//this->field_0x358 = uVar2;
	fVar4 = pByteCode->GetF32();
	//this->field_0x35c = fVar4;
	if (fVar4 < 0.0) {
		//this->field_0x35c = 1.0;
	}
	iVar1 = pByteCode->GetS32();
	//this->field_0x368 = iVar1;
	//this->field_0x3d0 = 0xffffffff;
	//pCVar3 = CActor::GetBehaviour((CActor*)this, 5);
	//if (pCVar3 != (CBehaviour*)0x0) {
	//	this->field_0x3d0 = 5;
	//}
	//pCVar3 = CActor::GetBehaviour((CActor*)this, 7);
	//if (pCVar3 != (CBehaviour*)0x0) {
	//	this->field_0x3d0 = 7;
	//}
	//this->field_0x3d4 = 0xffffffff;
	//pCVar3 = CActor::GetBehaviour((CActor*)this, 6);
	//if (pCVar3 != (CBehaviour*)0x0) {
	//	this->field_0x3d4 = 6;
	//}
	//pCVar3 = CActor::GetBehaviour((CActor*)this, 8);
	//if (pCVar3 != (CBehaviour*)0x0) {
	//	this->field_0x3d4 = 8;
	//}
	//this->field_0x3fc = (*(float*)&this->field_0x68 + *(float*)&this->field_0x60 + *(float*)&this->field_0x64) / 3.0;
	//this->field_0x1b4 = this->field_0x3fc * (float)this->field_0x1b4;
	//fVar4 = this->field_0x1d8 / this->field_0x3fc;
	//this->field_0x1d8 = fVar4;
	//if (0.95 < fVar4) {
	//	this->field_0x1d8 = 0.95;
	//}
	//fVar4 = this->field_0x1d4 / this->field_0x3fc;
	//this->field_0x1d4 = fVar4;
	//if (0.95 < fVar4) {
	//	this->field_0x1d4 = 0.95;
	//}
	//this->field_0x404 = 0;
	//this->field_0x408 = 0;
	//this->field_0x40c = 0;
	return;
}
