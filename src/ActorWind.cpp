#include "ActorWind.h"
#include "MemoryStream.h"

void CActorWind::Create(ByteCode* pByteCode)
{
	byte bVar1;
	int* piVar2;
	char* pcVar3;
	bool bVar4;
	GlobalSound_00451698* pGVar5;
	uint uVar6;
	int iVar7;
	uint uVar8;
	float fVar9;

	CActor::Create(pByteCode);
	uVar6 = pByteCode->GetU32();
	//this->field_0x160 = uVar6;
	fVar9 = pByteCode->GetF32();
	//this->field_0x164 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x168 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x16c = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x174 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x180 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x184 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x188 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x18c = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x178 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x17c = fVar9;
	uVar6 = pByteCode->GetU32();
	//this->field_0x190 = uVar6;
	//if (this->field_0x168 <= 0.0) {
	//	this->field_0x160 = this->field_0x160 | 1;
	//	this->field_0x168 = 0.001;
	//}
	//bVar4 = true;
	//if ((((this->field_0x160 & 0x20) == 0) && (bVar4 = false, (this->field_0x160 & 0x40) == 0)) &&
	//	(bVar4 = true, g_FloatSineCurve_00472260[1820] <= ABS((this->actorBase).rotationQuat.y))) {
	//	bVar4 = false;
	//}
	//if (bVar4) {
	//	this->field_0x160 = this->field_0x160 | 1;
	//}
	//else {
	//	FUN_00119a80(0.01745329, 0.01745329, 0.01745329, (int)this);
	//}
	pGVar5 = CScene::ptable.g_GlobalSoundPtr_00451698;
	uVar6 = pByteCode->GetU32();
	iVar7 = pByteCode->GetS32();
	//this->field_0x1b8 = iVar7;
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	//this->field_0x1bc = piVar2;
	iVar7 = pByteCode->GetS32();
	//this->field_0x1c0 = iVar7;
	//CActor::SV_InstallMaterialId(this->field_0x1c0);
	iVar7 = pByteCode->GetS32();
	//this->field_0x1c4 = iVar7;
	uVar8 = pByteCode->GetU32();
	//this->field_0x1c8 = uVar8;
	//if (*(byte*)&this->field_0x1c8 < 0x80) {
	//	*(byte*)&this->field_0x1c8 = *(byte*)&this->field_0x1c8 << 1;
	//}
	//else {
	//	*(undefined*)&this->field_0x1c8 = 0xff;
	//}
	//bVar1 = *(byte*)((int)&this->field_0x1c8 + 1);
	//if (bVar1 < 0x80) {
	//	*(byte*)((int)&this->field_0x1c8 + 1) = bVar1 << 1;
	//}
	//else {
	//	*(undefined*)((int)&this->field_0x1c8 + 1) = 0xff;
	//}
	//bVar1 = *(byte*)((int)&this->field_0x1c8 + 2);
	//if (bVar1 < 0x80) {
	//	*(byte*)((int)&this->field_0x1c8 + 2) = bVar1 << 1;
	//}
	//else {
	//	*(undefined*)((int)&this->field_0x1c8 + 2) = 0xff;
	//}
	iVar7 = pByteCode->GetS32();
	//this->field_0x1cc = iVar7;
	this->sectorObj.Create(pByteCode);
	if (2.2 <= CScene::_pinstance->field_0x1c) {
		piVar2 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar2 + 1);
		if (*piVar2 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
		}
		//this->field_0x1d0 = piVar2;
		pcVar3 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar3 + 0x20;
		//this->field_0x1d4 = pcVar3;
		piVar2 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar2 + 1);
		if (*piVar2 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
		}
		//this->field_0x1d8 = piVar2;
		pcVar3 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar3 + 0x20;
		//this->field_0x1dc = pcVar3;
		piVar2 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(piVar2 + 1);
		if (*piVar2 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
		}
		//this->field_0x1e0 = piVar2;
		pcVar3 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar3 + 0x20;
		//this->field_0x1e4 = pcVar3;
	}
	//this->field_0x170 = 0x40800000;
	fVar9 = pByteCode->GetF32();
	//this->field_0x194 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x198 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x19c = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x1a0 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x1a4 = fVar9;
	fVar9 = (float)pByteCode->GetS32();
	//this->field_0x1a8 = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x1ac = fVar9;
	fVar9 = pByteCode->GetF32();
	//this->field_0x1b0 = fVar9;
	//if (100.0 < this->field_0x16c) {
	//	this->field_0x16c = 100.0;
	//}
	//else {
	//	if (this->field_0x16c < 0.0) {
	//		this->field_0x16c = 0.0;
	//	}
	//}
	//this->field_0x1fc = this->field_0x16c + ABS(this->field_0x194);
	//this->field_0x200 = this->field_0x16c - ABS(this->field_0x198);
	//if (this->field_0x200 < 0.0) {
	//	this->field_0x200 = 0.0;
	//}
	//iVar7 = FUN_00184590((int)pGVar5, (long)(int)uVar6);
	//this->field_0x1b4 = iVar7;
	//this->field_0x224 = 0;
	//this->count_0x220 = 0;
	//this->field_0x1e8 = 0;
	//this->field_0x1ec = 0;
	return;
}
