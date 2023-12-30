#include "ActorBridge.h"
#include "MemoryStream.h"

void CActorBridge::Create(ByteCode* pByteCode)
{
	int* piVar1;
	CWayPointManager* pCVar2;
	int iVar3;
	//ManagerC_24* pMVar4;
	uint uVar5;
	float fVar6;

	CActor::Create(pByteCode);
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	//this->field_0x168 = piVar1;
	//pCVar2 = CScene::ptable.g_CWayPointManager_0045169c;
	iVar3 = pByteCode->GetS32();
	//if (iVar3 == -1) {
	//	pMVar4 = (ManagerC_24*)0x0;
	//}
	//else {
	//	pMVar4 = pCVar2->field_0x4 + iVar3;
	//}
	//this->field_0x160 = pMVar4;
	iVar3 = pByteCode->GetS32();
	if (iVar3 == -1) {
		//pMVar4 = (ManagerC_24*)0x0;
	}
	else {
		//pMVar4 = pCVar2->field_0x4 + iVar3;
	}
	//this->field_0x164 = pMVar4;
	fVar6 = pByteCode->GetF32();
	//this->field_0x330 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x334 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x33c = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x340 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x338 = fVar6;
	uVar5 = pByteCode->GetU32();
	//this->field_0x344 = uVar5;
	uVar5 = pByteCode->GetU32();
	//this->field_0x358 = uVar5;
	iVar3 = pByteCode->GetS32();
	//this->field_0x348 = iVar3;
	fVar6 = pByteCode->GetF32();
	//this->field_0x34c = fVar6;
	//this->field_0x2e0 = 0;
	//this->field_0x350 = 0;
	return;
}
