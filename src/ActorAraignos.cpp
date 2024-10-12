#include "ActorAraignos.h"
#include "MemoryStream.h"

void CActorAraignos::Create(ByteCode* pByteCode)
{
	CAudioManager* pGVar1;
	CPathManager* pCVar2;
	int iVar3;
	uint uVar4;
	int* piVar5;
	long lVar6;
	float fVar7;

	pCVar2 = CScene::ptable.g_PathManager_004516a0;
	CActorMovable::Create(pByteCode);
	//this->field_0x1d0 = 0;
	iVar3 = pByteCode->GetS32();
	if (iVar3 == -1) {
		lVar6 = 0;
	}
	else {
		//lVar6 = (long)(int)(pCVar2->field_0x4 + iVar3);
	}
	fVar7 = pByteCode->GetF32();
	//this->field_0x1f8 = fVar7;
	fVar7 = pByteCode->GetF32();
	//this->field_0x1fc = fVar7;
	fVar7 = pByteCode->GetF32();
	//this->field_0x200 = fVar7;
	//pGVar1 = CScene::ptable.g_GlobalSoundPtr_00451698;
	uVar4 = pByteCode->GetU32();
	//uVar4 = FUN_001845c0((int)pGVar1, uVar4);
	//this->field_0x240 = uVar4;
	uVar4 = pByteCode->GetU32();
	//uVar4 = FUN_001845c0((int)pGVar1, uVar4);
	//this->field_0x238 = uVar4;
	uVar4 = pByteCode->GetU32();
	//uVar4 = FUN_001845c0((int)pGVar1, uVar4);
	//this->field_0x23c = uVar4;
	//this->pathFollowAbsolute.Create(this->field_0x1f8 * 1.5, lVar6, 0);
	//if (((this->field_0x240 == 0) && (this->field_0x238 == 0)) && (this->field_0x23c == 0)) {
	//	this->field_0x244 = (int*)0x0;
	//}
	//else {
	//	piVar5 = CActor::CreateActorSound(this, 2);
	//	this->field_0x244 = piVar5;
	//}
	addOnGenerator.Create(this, pByteCode);
	return;
}
