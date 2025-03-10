#include "ActorTeleporter.h"
#include "MemoryStream.h"
#include "FileManager3D.h"
#include "LevelScheduleManager.h"

void CActorTeleporter::Create(ByteCode* pByteCode)
{
	int* piVar1;
	CCollision* pCVar2;
	C3DFileManager* p3DFileManager;
	int iVar3;
	uint uVar4;
	int iVar5;
	ParticleInfo* pPVar6;
	ulong uVar7;
	long lVar8;

	//p3DFileManager = CScene::ptable.g_C3DFileManager_00451664;
	CActor::Create(pByteCode);
	iVar3 = pByteCode->GetS32();
	//*(int*)((int)&this->field_0x164 + 4) = iVar3;
	uVar7 = pByteCode->GetU64();
	//*(ulong*)&this->field_0x160 = uVar7;
	uVar4 = pByteCode->GetU32();
	//this->field_0x16c = uVar4;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0xc;
	}
	//*(int**)&this->field_0x2bc = piVar1;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x170 = (uint)piVar1;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 8);
	//this->field_0x174 = piVar1;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x178 = (int)piVar1;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 8);
	//this->field_0x17c = piVar1;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x180 = (int)piVar1;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 8);
	//this->field_0x184 = piVar1;
	iVar3 = pByteCode->GetS32();
	//this->field_0x188 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x18c = iVar3;
	iVar3 = pByteCode->GetS32();
	iVar5 = pByteCode->GetS32();
	//*(int*)&this->field_0x1c8 = iVar5;
	//CActor::SV_InstallMaterialId(iVar3);
	//pPVar6 = 3DFileManager::GetParticleInfo_001a6d10(p3DFileManager, iVar3);
	//this->field_0x190 = (int)pPVar6;
	iVar3 = pByteCode->GetS32();
	//CActor::SV_InstallMaterialId(iVar3);
	//pPVar6 = 3DFileManager::GetParticleInfo_001a6d10(p3DFileManager, iVar3);
	//this->pParticleInfo_0x190 = pPVar6;
	iVar3 = pByteCode->GetS32();
	//this->field_0x1d4 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x1d8 = iVar3;
	iVar3 = pByteCode->GetS32();
	//CActor::SV_InstallMaterialId(iVar3);
	//pPVar6 = 3DFileManager::GetParticleInfo_001a6d10(p3DFileManager, iVar3);
	//this->field_0x1dc = (int)pPVar6;
	pPVar6 = (ParticleInfo*)pByteCode->GetS32();
	//this->pParticleInfo_0x1dc = pPVar6;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 8;
	}
	//this->field_0x2c0 = piVar1;
	this->condOpArray.Create(pByteCode);
	//if (*(int*)&this->field_0x1c8 != -1) {
	//	piVar1 = *(int**)&this->field_0x2bc;
	//	iVar3 = 0;
	//	if (piVar1 != (int*)0x0) {
	//		iVar3 = *piVar1;
	//	}
	//	if ((iVar3 != 0) && (this->field_0x190 == 0)) {
	//		if (piVar1 == (int*)0x0) {
	//			lVar8 = 0;
	//		}
	//		else {
	//			lVar8 = (long)*piVar1;
	//		}
	//		iVar3 = GetMaterialInfo_00360170(lVar8);
	//		this->field_0x1cc = iVar3;
	//		goto LAB_002ef408;
	//	}
	//}
	//this->field_0x1cc = 0;
LAB_002ef408:
	//this->field_0x1d0 = 0;
	//pCVar2 = (this->base).pCollisionData;
	//if (pCVar2 != (CCollision*)0x0) {
	//	pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffffc4;
	//	pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x40;
	//}
	//this->field_0x2ac = (uint*)0xffffffff;
	//this->field_0x2b0 = -1;
	return;
}

CBehaviour* CActorTeleporter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourTeleporterDefault;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

void CActorTeleporter::UpdateCurTeleporterState(int levelId, int param_3)
{
	IMPLEMENTATION_GUARD();
}

edDList_material* CActorTeleporter::GetMySubSectorMaterial(int levelId, int nbAreas)
{
	int* piVar1;
	bool bVar2;
	CLevelScheduler* pCVar3;
	edDList_material* pSubSectorMaterial;
	int iVar5;
	int iVar6;
	int iVar7;

	pCVar3 = CLevelScheduler::gThis;
	pSubSectorMaterial = (edDList_material*)0x0;

	IMPLEMENTATION_GUARD_LOG(
	bVar2 = levelId == 0 || levelId == INT_ARRAY_0048ed60[0];
	if (levelId != 0 && levelId != INT_ARRAY_0048ed60[0]) {
		bVar2 = INT_ARRAY_0048ed60[levelId] == 0;
	}

	if (bVar2) {
		pSubSectorMaterial = GetDestinationMaterial(0, 1);
	}
	else {
		iVar7 = 0;
		iVar6 = 0;
		while (true) {
			piVar1 = *(int**)&this->field_0x2bc;
			iVar5 = 0;
			if (piVar1 != (int*)0x0) {
				iVar5 = *piVar1;
			}
			if ((iVar5 <= iVar7) || (pSubSectorMaterial != (edDList_material*)0x0)) break;
			iVar5 = *(int*)((int)piVar1 + iVar6 + 4);
			if (iVar5 != 0x10) {
				bVar2 = levelId == iVar5;
				if (!bVar2) {
					bVar2 = iVar5 == INT_ARRAY_0048ed60[levelId];
				}
				if (!bVar2) {
					bVar2 = levelId == INT_ARRAY_0048ed60[iVar5];
				}
				if (bVar2) {
					pSubSectorMaterial = GetDestinationMaterial(iVar5, nbAreas);
				}
			}
			iVar6 = iVar6 + 0xc;
			iVar7 = iVar7 + 1;
		}

		if ((pSubSectorMaterial == (edDList_material*)0x0) && (levelId == pCVar3->currentLevelID)) {
			pSubSectorMaterial = GetDestinationMaterial(this, levelId, nbAreas);
		}
	})

	return pSubSectorMaterial;
}
