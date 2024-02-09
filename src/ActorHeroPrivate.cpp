#include "ActorHeroPrivate.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "LevelScheduleManager.h"
#include "CameraViewManager.h"
#include "InputManager.h"

CActorHeroPrivate::CActorHeroPrivate()
{
	int* piVar1;
	undefined4* puVar2;
	
	CActor();
	//this->base.base.pVTable = &CActorMovable::_vt;
	//this->base.dynamic.field_0x54 = 0.0;
	//this->base.base.pVTable = &CActorAutonomous::_vt;
	//this->dynamicExt.base.rotationQuat.x = 1.0;
	//this->dynamicExt.base.rotationQuat.y = 0.0;
	//this->dynamicExt.base.rotationQuat.z = 0.0;
	//CLifeInterface::CLifeInterface(&this->lifeInterface);
	//piVar1 = &(this->character).field_0x550;
	//this->base.base.pVTable = &CActorFighter::_vt;
	//do {
	//	*piVar1 = 0;
	//	piVar1[1] = 0;
	//	piVar1 = piVar1 + 3;
	//} while (piVar1 != (int*)&(this->character).field_0x568);
	//CPathFinderClient::CPathFinderClient((long)(int)&(this->character).field_0x570);
	//*(undefined4*)&(this->character).field_0x638 = 0;
	//CSlaveGroundSampler::CSlaveGroundSampler((long)&this->field_0x9e0);
	//this->base.base.pVTable = &CActorHero::_vt;
	//CMagicInterface::CMagicInterface((long)&this->field_0xab8);
	//CMoneyInterface::CMoneyInterface((long)&this->field_0xad8);
	//CInventoryInterface::CInventoryInterface((long)&this->field_0xadc);
	//FUN_001dbc60((long)&this->field_0xc84);
	//FUN_00324320((long)&this->field_0xcd0);
	//this->base.base.pVTable = (ActorVTable*)&_vt;
	//CLifeInterface::CLifeInterface(&this->field_0xee4);
	//(this->fxTrailA).field_0x4 = (int)&CObject::_vt;
	//(this->fxTrailA).field_0x8 = 0xffffffff;
	//(this->fxTrailA).field_0x4 = (int)&CFxTail::_vt;
	//(this->fxTrailA).field_0x1c = 0.0;
	//(this->fxTrailB).field_0x4 = (int)&CObject::_vt;
	//(this->fxTrailB).field_0x8 = 0xffffffff;
	//(this->fxTrailB).field_0x4 = (int)&CFxTail::_vt;
	//(this->fxTrailB).field_0x1c = 0.0;
	//*(undefined4*)&this->field_0x13c0 = 0;
	//this->field_0x13c4 = 0;
	//*(undefined4*)&this->field_0x15d4 = 0;
	//this->field_0x15d8 = 0;
	//CHero_InputRotationAnalyser::CHero_InputRotationAnalyser
	//((CHero_InputRotationAnalyser*)&(this->field_0x15dc).field_0x4);
	//puVar2 = (undefined4*)&this->field_0x1880;
	//do {
	//	*puVar2 = 0;
	//	puVar2[1] = 0;
	//	puVar2 = puVar2 + 3;
	//} while (puVar2 != (undefined4*)&this->field_0x18b0);
	//*(undefined4*)&this->field_0x18b4 = 0;
	//*(undefined4*)&this->field_0x18b8 = 0;
	//*(undefined4*)&this->field_0x18c0 = 0;
	//*(undefined4*)&this->field_0x18c4 = 0;
	//*(undefined4*)&this->field_0x18cc = 0;
	//*(undefined4*)&this->field_0x18d0 = 0;
	//this->field_0x1994 = 0.0;
	//(this->field_0x1998).x = 0.0;
	//(this->field_0x1998).y = 0.2;
	//(this->field_0x1998).z = 0.2;
	//(this->field_0x1998).w = 2.0;
	//this->field_0x19a8 = 0.5;
	//*(undefined4*)&this->field_0x19ac = 0x40400000;
	//*(undefined*)&this->pAnimKeyData_0x19b4 = 0;
	//this->pAnimKeyData_0x19b8 = (edAnmAnim*)0x0;
	//*(undefined4*)&this->field_0x19b8 = 0;
	//this->field_0x1a60 = (undefined*)0x0;
	//Particle::Constructor_00401590((long)&(this->playerSubStruct_64_0x1bf0).field_0x4);
	

	//*(undefined4*)&this->field_0x1c60 = 0;
	//*(undefined4*)&this->field_0x1c64 = 0;
	//(this->cameraPan_0x1c70).textureIndex = (int)&StaticMeshComponent::VTable_0043a7a0;
	//(this->cameraPan_0x1c70).pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	//*(undefined4*)&(this->cameraPan_0x1c70).field_0x58 = 0;
	//*(undefined4*)&(this->cameraPan_0x1c70).field_0xc = 0xffffffff;
	//(this->cameraPan_0x1c70).meshIndex = -1;
	//(this->cameraPan_0x1c70).textureIndex = (int)&StaticMeshComponent::VTable_0043a6f0;
	//(this->cameraPan_0x1c70).textureIndex = (int)&StaticMeshComponent::VTable_0043a6d0;
	//*(undefined4*)&this->field_0x1d30 = 0;
	//*(undefined4*)&this->field_0x1d34 = 0;
	//*(undefined4*)&this->field_0x1d3c = 0;
	//*(undefined4*)&this->field_0x1d40 = 0;
	//(this->cameraPan_0x1d50).textureIndex = (int)&StaticMeshComponent::VTable_0043a7a0;
	//(this->cameraPan_0x1d50).pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	//*(undefined4*)&(this->cameraPan_0x1d50).field_0x58 = 0;
	//*(undefined4*)&(this->cameraPan_0x1d50).field_0xc = 0xffffffff;
	//(this->cameraPan_0x1d50).meshIndex = -1;
	//(this->cameraPan_0x1d50).textureIndex = (int)&StaticMeshComponent::VTable_0043a6f0;
	//(this->cameraPan_0x1d50).textureIndex = (int)&StaticMeshComponent::VTable_0043a6d0;
	
	//*(undefined4*)&this->field_0x1e20 = 0;
	//*(undefined4*)&this->field_0x1e24 = 0;
	//(this->cameraPan_0x1e30).textureIndex = (int)&StaticMeshComponent::VTable_0043a7a0;
	//(this->cameraPan_0x1e30).pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	//*(undefined4*)&(this->cameraPan_0x1e30).field_0x58 = 0;
	//*(undefined4*)&(this->cameraPan_0x1e30).field_0xc = 0xffffffff;
	//(this->cameraPan_0x1e30).meshIndex = -1;
	//(this->cameraPan_0x1e30).textureIndex = (int)&StaticMeshComponent::VTable_0043a6f0;
	//(this->cameraPan_0x1e30).textureIndex = (int)&StaticMeshComponent::VTable_0043a6d0;
	//*(undefined4*)&this->field_0x1ef0 = 0;
	//*(undefined4*)&this->field_0x1ef4 = 0;
	//*(undefined4*)&this->field_0x1efc = 0;
	//*(undefined4*)&this->field_0x1f00 = 0;
	//(this->cameraPan_0x1f10).textureIndex = (int)&StaticMeshComponent::VTable_0043a7a0;
	//(this->cameraPan_0x1f10).pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	//*(undefined4*)&(this->cameraPan_0x1f10).field_0x58 = 0;
	//*(undefined4*)&(this->cameraPan_0x1f10).field_0xc = 0xffffffff;
	//(this->cameraPan_0x1f10).meshIndex = -1;
	//(this->cameraPan_0x1f10).textureIndex = (int)&StaticMeshComponent::VTable_0043a6f0;
	//(this->cameraPan_0x1f10).textureIndex = (int)&StaticMeshComponent::VTable_0043a6d0;
	
	//*(undefined4*)&this->field_0x1fe0 = 0;
	//*(undefined4*)&this->field_0x1fe4 = 0;
	//(this->cameraPan_0x1ff0).textureIndex = (int)&StaticMeshComponent::VTable_0043a7a0;
	//(this->cameraPan_0x1ff0).pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	//*(undefined4*)&(this->cameraPan_0x1ff0).field_0x58 = 0;
	//*(undefined4*)&(this->cameraPan_0x1ff0).field_0xc = 0xffffffff;
	//(this->cameraPan_0x1ff0).meshIndex = -1;
	//(this->cameraPan_0x1ff0).textureIndex = (int)&StaticMeshComponent::VTable_0043a6f0;
	//(this->cameraPan_0x1ff0).textureIndex = (int)&StaticMeshComponent::VTable_0043a6d0;
	//*(undefined4*)&this->field_0x20b0 = 0;
	//*(undefined4*)&this->field_0x20b4 = 0;
	//*(undefined4*)&this->field_0x20bc = 0;
	//*(undefined4*)&this->field_0x20c0 = 0;
	//(this->cameraPan_0x20d0).textureIndex = (int)&StaticMeshComponent::VTable_0043a7a0;
	//(this->cameraPan_0x20d0).pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	//*(undefined4*)&(this->cameraPan_0x20d0).field_0x58 = 0;
	//*(undefined4*)&(this->cameraPan_0x20d0).field_0xc = 0xffffffff;
	//(this->cameraPan_0x20d0).meshIndex = -1;
	//(this->cameraPan_0x20d0).textureIndex = (int)&StaticMeshComponent::VTable_0043a6f0;
	//(this->cameraPan_0x20d0).textureIndex = (int)&StaticMeshComponent::VTable_0043a6d0;
	return;
}

void CActorHeroPrivate::Create(ByteCode* pByteCode)
{
	char* pcVar1;
	uint uVar2;
	CCamera* pCVar3;
	//ed_zone_3d* peVar4;
	int* piVar5;
	S_CHECKPOINT* pSVar6;
	int* piVar7;
	int iVar8;
	undefined4 pOther;
	ulong uVar9;
	int iVar10;
	int iVar11;
	CActorHeroPrivate* pCVar12;
	float fVar13;

	if (CActorHero::_gThis == (CActorHero*)0x0) {
		CActorHero::_gThis = this;
	}

	this->field_0x1610 = 1;
	this->field_0x18dc = 1;
	this->field_0xee0 = 0;

	// HACK
	{
		CActorAutonomous::Create(pByteCode);

		char* pCurrent = pByteCode->currentSeekPos;

		while (true) {
			if (strncmp(pCurrent, "TSNI", 4) == 0) {
				break;
			}
			pCurrent++;
		}

		pByteCode->currentSeekPos = pCurrent;
	}

	//CActorFighter::Create(pByteCode);

	//this->field_0x1870 = 0;
	//this->field_0x1874 = this->character.field_0x444;
	//*(undefined4*)&this->field_0x1878 = *(undefined4*)&this->character.field_0x448;
	//this->field_0xff0 = 0;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->field_0x157c = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->animKey_0x1584 = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->animKey_0x1588 = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->animKey_0x158c = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->animKey_0x1590 = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->animKey_0x1594 = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->field_0x1594 = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//this->field_0x1598 = uVar2;
	//uVar2 = ByteCode::GetU32(pByteCode);
	//*(uint*)&this->field_0x159c = uVar2;
	//pCVar3 = (CCamera*)ByteCode::GetU32(pByteCode);
	//this->pKyaWindWallCamera_0x15ac = pCVar3;
	//piVar5 = (int*)pByteCode->currentSeekPos;
	//pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	//if (*piVar5 != 0) {
	//	pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 4;
	//}
	//*(int**)&this->field_0x14a4 = piVar5;
	//iVar11 = 0;
	//pCVar12 = this;
	//do {
	//	iVar10 = 0;
	//	do {
	//		iVar8 = (int)&(pCVar12->base).character.characterBase.base.base.pVTable + iVar10;
	//		*(undefined*)(iVar8 + 0xd34) = 0;
	//		iVar10 = iVar10 + 8;
	//		*(undefined*)(iVar8 + 0xd35) = 0;
	//		*(undefined*)(iVar8 + 0xd36) = 0;
	//		*(undefined*)(iVar8 + 0xd37) = 0;
	//		*(undefined*)(iVar8 + 0xd38) = 0;
	//		*(undefined*)(iVar8 + 0xd39) = 0;
	//		*(undefined*)(iVar8 + 0xd3a) = 0;
	//		*(undefined*)(iVar8 + 0xd3b) = 0;
	//	} while (iVar10 < 0x10);
	//	iVar11 = iVar11 + 1;
	//	pCVar12 = (CActorHeroPrivate*)&(pCVar12->base).character.characterBase.base.base.data.field_0xc;
	//} while (iVar11 < 0x10);
	//peVar4 = (ed_zone_3d*)ByteCode::GetS32(pByteCode);
	//this->pEventChunk24_0xd24 = peVar4;
	//*(undefined4*)&this->field_0xd24 = 0;
	//peVar4 = this->pEventChunk24_0xd24;
	//if (peVar4 != (ed_zone_3d*)0x0) {
	//	piVar5 = (int*)operator.new.array((long)((int)peVar4 * 0x28 + 0x10));
	//	piVar5 = __construct_new_array(piVar5, (ActorConstructorA*)&LAB_0034c690, FUN_0034b6b0, 0x28, (uint)peVar4);
	//	*(int**)&this->field_0xd24 = piVar5;
	//	piVar5 = *(int**)&this->field_0xd24;
	//	iVar11 = 0;
	//	if (0 < (int)this->pEventChunk24_0xd24) {
	//		do {
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			*piVar5 = iVar10;
	//			uVar9 = ByteCode::GetU64(pByteCode);
	//			*(ulong*)(piVar5 + 2) = uVar9;
	//			uVar2 = ByteCode::GetU32(pByteCode);
	//			piVar5[4] = uVar2;
	//			uVar2 = ByteCode::GetU32(pByteCode);
	//			piVar5[5] = uVar2;
	//			fVar13 = ByteCode::GetF32(pByteCode);
	//			piVar5[6] = (int)fVar13;
	//			if (2.24 <= CScene::_pinstance->field_0x1c) {
	//				piVar7 = (int*)pByteCode->currentSeekPos;
	//				pByteCode->currentSeekPos = (char*)(piVar7 + 1);
	//				if (*piVar7 != 0) {
	//					pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar7 * 0x1c;
	//				}
	//				piVar5[7] = (int)piVar7;
	//				pcVar1 = pByteCode->currentSeekPos;
	//				pByteCode->currentSeekPos = pcVar1 + 0x20;
	//				piVar5[8] = (int)pcVar1;
	//			}
	//			iVar11 = iVar11 + 1;
	//			piVar5 = piVar5 + 10;
	//		} while (iVar11 < (int)this->pEventChunk24_0xd24);
	//	}
	//}
	//pSVar6 = (S_CHECKPOINT*)ByteCode::GetS32(pByteCode);
	//this->pCheckpointManagerSubObjA_0xe38 = pSVar6;
	//this->field_0xe38 = 0;
	//pSVar6 = this->pCheckpointManagerSubObjA_0xe38;
	//if (pSVar6 != (S_CHECKPOINT*)0x0) {
	//	piVar5 = (int*)operator.new.array((long)((int)pSVar6 * 0x1c + 0x10));
	//	piVar5 = __construct_new_array(piVar5, (ActorConstructorA*)&LAB_0034c680, FUN_0034b660, 0x1c, (uint)pSVar6);
	//	this->field_0xe38 = (uint)piVar5;
	//	piVar5 = (int*)this->field_0xe38;
	//	iVar11 = 0;
	//	if (0 < (int)this->pCheckpointManagerSubObjA_0xe38) {
	//		do {
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			*piVar5 = iVar10;
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			piVar5[1] = iVar10;
	//			fVar13 = ByteCode::GetF32(pByteCode);
	//			piVar5[2] = (int)fVar13;
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			piVar5[3] = iVar10;
	//			fVar13 = ByteCode::GetF32(pByteCode);
	//			piVar5[4] = (int)fVar13;
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			piVar5[5] = iVar10;
	//			if (piVar5[5] < 1) {
	//				piVar5[5] = 1;
	//			}
	//			if (5 < piVar5[5]) {
	//				piVar5[5] = 5;
	//			}
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			piVar5[6] = iVar10;
	//			iVar11 = iVar11 + 1;
	//			piVar5 = piVar5 + 7;
	//		} while (iVar11 < (int)this->pCheckpointManagerSubObjA_0xe38);
	//	}
	//}
	//if (2.24 <= CScene::_pinstance->field_0x1c) {
	//	piVar5 = (int*)ByteCode::GetS32(pByteCode);
	//	this->field_0xe3c = piVar5;
	//}
	//else {
	//	this->field_0xe3c = (int*)0x0;
	//}
	//*(undefined4*)&this->field_0xe40 = 0;
	//piVar5 = this->field_0xe3c;
	//if (piVar5 != (int*)0x0) {
	//	piVar7 = (int*)operator.new.array((long)((int)piVar5 * 8 + 0x10));
	//	piVar5 = __construct_new_array(piVar7, (ActorConstructorA*)&LAB_0034c670, FUN_0034b610, 8, (uint)piVar5);
	//	*(int**)&this->field_0xe40 = piVar5;
	//	piVar5 = *(int**)&this->field_0xe40;
	//	iVar11 = 0;
	//	if (0 < (int)this->field_0xe3c) {
	//		do {
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			*piVar5 = iVar10;
	//			iVar10 = ByteCode::GetS32(pByteCode);
	//			piVar5[1] = iVar10;
	//			iVar11 = iVar11 + 1;
	//			piVar5 = piVar5 + 2;
	//		} while (iVar11 < (int)this->field_0xe3c);
	//	}
	//}
	//if (2.24 <= CScene::_pinstance->field_0x1c) {
	//	piVar5 = (int*)pByteCode->currentSeekPos;
	//	pByteCode->currentSeekPos = (char*)(piVar5 + 1);
	//	if (*piVar5 != 0) {
	//		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar5 * 4;
	//	}
	//	*(int**)&this->field_0xe48 = piVar5;
	//}
	//iVar11 = ByteCode::GetS32(pByteCode);
	//*(int*)&this->field_0x12e0 = iVar11;
	//iVar11 = *(int*)&this->field_0x12e0;
	//if (iVar11 != -1) {
	//	CActor::SV_InstallMaterialId(iVar11);
	//	CFxTail::Create(1.0, (CFxTail*)&this->fxTrailA.field_0x4, 0x10, 4, *(int*)&this->field_0x12e0);
	//	CFxTail::Create(1.0, (CFxTail*)&this->fxTrailB.field_0x4, 0x10, 4, *(int*)&this->field_0x12e0);
	//}
	//iVar11 = ByteCode::GetS32(pByteCode);
	//*(int*)&this->field_0x13c8 = iVar11;
	//fVar13 = ByteCode::GetF32(pByteCode);
	//this->field_0x1990 = fVar13;
	//this->bCanUseCheats = 0;
	//this->pCameraViewBase_0xaa8 = (CCamera*)0x0;
	//iVar11 = LevelScheduleManager::ScenVar_Get(0x11);
	//iVar10 = LevelScheduleManager::ScenVar_Get(0x12);
	//iVar8 = LevelScheduleManager::ScenVar_Get(0x14);
	//CLifeInterface::SetValueMax((float)iVar8, &this->character.characterBase.lifeInterface);
	//SetParam2_0x18ToParam1((float)(iVar11 * iVar10), &this->field_0xab8);
	//SetFloatParam1_0x1cToParam2(0.0, &this->field_0xab8);
	//(**(code**)(*(int*)&this->field_0xab8 + 0x20))((float)_gGameNfo.heroMagic, &this->field_0xab8);
	//iVar11 = ByteCode::GetS32(pByteCode);
	//if (iVar11 != 0) {
	//	pOther = CScene::GetManager(MO_Camera);
	//	iVar10 = 0;
	//	if (0 < iVar11 + -2) {
	//		do {
	//			/* Kim.act cam */
	//			AddCamera(pOther, CT_MainCamera, pByteCode, s_Kim.act_cam_004350c8);
	//			iVar10 = iVar10 + 1;
	//		} while (iVar10 < iVar11 + -2);
	//	}
	//	/* Kim.act WindWall */
	//	pCVar3 = AddCamera(pOther, CT_KyaWindWall, pByteCode, s_Kim.act_WindWall_004350e0);
	//	this->pJamgutCamera_0x15b8 = pCVar3;
	//	/* Kim.act Jamgut Cossack */
	//	pCVar3 = AddCamera(pOther, CT_KyaJamgut, pByteCode, s_Kim.act_Jamgut_Cossack_00435100);
	//	this->pIntViewCamera_0x15bc = pCVar3;
	//}
	//this->pBoomySnipeCamera_0xc94 = (CCamera*)0x0;
	//iVar11 = ByteCode::GetS32(pByteCode);
	//this->field_0x15dc.field_0x0 = iVar11;
	//iVar11 = ByteCode::GetS32(pByteCode);
	//uVar2 = 0;
	//pCVar12 = this;
	//do {
	//	FUN_00407690((int*)&(pCVar12->base).field_0x1880, iVar11);
	//	uVar2 = uVar2 + 1;
	//	pCVar12 = (CActorHeroPrivate*)&(pCVar12->base).character.characterBase.base.base.data.actorFieldS;
	//} while (uVar2 < 4);
	//*(undefined4*)&this->field_0x18b0 = 0;
	//FUN_004076f0((int*)&this->field_0x18b4, pByteCode);
	//FUN_004076f0((int*)&this->field_0x18c0, pByteCode);
	//FUN_004076f0((int*)&this->field_0x18cc, pByteCode);
	//FUN_00401470(&this->playerSubStruct_64_0x1bf0.field_0x4, 4);
	return;
}

void CActorHeroPrivate::Init()
{
	//ushort uVar1;
	//int* piVar2;
	//CAnimation* pAnimationController;
	//edANM_HDR** ppeVar3;
	//FrontendManager* pFVar4;
	//CCameraManager* pCameraManager;
	//CCamera* pCVar5;
	//void* pvVar6;
	//undefined8 uVar7;
	//ed_zone_3d* peVar8;
	//int iVar9;
	//int iVar10;
	//edAnmAnim* peVar11;
	//int iVar12;
	//S_STREAM_REF<ed_zone_3d>* pSVar13;
	//float fVar14;
	//ByteCode BStack32;
	//undefined4 local_10;
	//uint local_c;
	//uint local_8;
	//uint local_4;
	//
	//*(undefined4*)&this->field_0xcbc = 0;
	//*(undefined4*)&this->field_0x1a48 = 0;
	//*(undefined4*)&this->field_0x1a4c = 0;
	//*(undefined4*)&this->field_0x1a50 = 0;
	//*(undefined4*)&this->field_0xcc0 = 0;
	this->field_0xee0 = 1;
	CActorFighter::Init();
	this->field_0xf54 = 0;
	ResetStdDefaultSettings();
	//FUN_0032bfc0((int)this);
	ResetBoomyDefaultSettings();
	//FUN_0013df70((int)this);
	//FUN_0014cfe0((int)this);
	//FUN_00369360();
	ClearLocalData();
	//this->field_0xe4c.y = *(float*)&this->baseLocation;
	//this->field_0xe4c.z = *(float*)((int)&this->baseLocation + 4);
	//this->field_0xe4c.w = *(float*)((int)&this->baseLocation + 8);
	//this->x_0xe60 = *(float*)((int)&this->baseLocation + 0xc);
	//this->y_0xe64 = *(float*)((int)this->pCinData + 0x18);
	//this->z_0xe68 = *(float*)((int)this->pCinData + 0x1c);
	//this->field_0xe68 = *(undefined4*)((int)this->pCinData + 0x20);
	//*(int*)&this->field_0xe70 =
	//	LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->currentLevelID].sectorStartIndex;
	//this->field_0xe7c.y = this->field_0xe4c.y;
	//this->field_0xe7c.z = this->field_0xe4c.z;
	//this->field_0xe7c.w = this->field_0xe4c.w;
	//this->x_0xe90 = this->x_0xe60;
	//this->y_0xe94 = this->y_0xe64;
	//this->z_0xe98 = this->z_0xe68;
	//this->field_0xe98 = this->field_0xe68;
	//this->levelDataField1C_0xea0 = this->levelDataField1C_0xe70;
	//*(undefined4*)&this->field_0xea0 = *(undefined4*)&this->field_0xe70;
	//CAnimation::RegisterBone
	//(this->pAnimationController, this->field_0x157c);
	//CAnimation::RegisterBone(this->pAnimationController, 0x45544554);
	//CAnimation::RegisterBone
	//(this->pAnimationController, this->animKey_0x1588);
	//pSVar13 = *(S_STREAM_REF<ed_zone_3d> **) & this->field_0x14a4;
	//for (peVar8 = pSVar13->pZone; peVar8 != (ed_zone_3d*)0x0; peVar8 = (ed_zone_3d*)((int)&peVar8[-1].field_0x20 + 3)) {
	//	pSVar13 = pSVar13 + 1;
	//	S_STREAM_REF<ed_zone_3d>::Init(pSVar13);
	//}
	//pSVar13 = *(S_STREAM_REF<ed_zone_3d> **) & this->field_0xd24;
	//iVar12 = 0;
	//if (0 < (int)this->pEventChunk24_0xd24) {
	//	do {
	//		S_STREAM_REF<ed_zone_3d>::Init(pSVar13);
	//		if (2.24 <= CScene::_pinstance->field_0x1c) {
	//			peVar8 = pSVar13[7].pZone;
	//			iVar10 = 0;
	//			if (0 < (int)peVar8->field_0x0[0]) {
	//				iVar9 = 0;
	//				do {
	//					S_STREAM_NTF_TARGET_BASE::Init((S_STREAM_NTF_TARGET_BASE*)((int)peVar8->field_0x0 + iVar9 + 4));
	//					peVar8 = pSVar13[7].pZone;
	//					iVar10 = iVar10 + 1;
	//					iVar9 = iVar9 + 0x1c;
	//				} while (iVar10 < (int)peVar8->field_0x0[0]);
	//			}
	//			S_STREAM_EVENT_CAMERA::Init((S_STREAM_EVENT_CAMERA*)pSVar13[8].pZone);
	//		}
	//		iVar12 = iVar12 + 1;
	//		pSVar13 = pSVar13 + 10;
	//	} while (iVar12 < (int)this->pEventChunk24_0xd24);
	//}
	//pSVar13 = (S_STREAM_REF<ed_zone_3d> *)this->field_0xe38;
	//iVar12 = 0;
	//if (0 < (int)this->pCheckpointManagerSubObjA_0xe38) {
	//	do {
	//		S_STREAM_REF<ed_zone_3d>::Init(pSVar13);
	//		iVar12 = iVar12 + 1;
	//		pSVar13 = pSVar13 + 7;
	//	} while (iVar12 < (int)this->pCheckpointManagerSubObjA_0xe38);
	//}
	//if (2.24 <= CScene::_pinstance->field_0x1c) {
	//	pSVar13 = *(S_STREAM_REF<ed_zone_3d> **) & this->field_0xe48;
	//	for (peVar8 = pSVar13->pZone; peVar8 != (ed_zone_3d*)0x0; peVar8 = (ed_zone_3d*)((int)&peVar8[-1].field_0x20 + 3))
	//	{
	//		pSVar13 = pSVar13 + 1;
	//		S_STREAM_REF<ed_zone_3d>::Init(pSVar13);
	//	}
	//}
	//pSVar13 = *(S_STREAM_REF<ed_zone_3d> **) & this->field_0xe40;
	//iVar12 = 0;
	//if (0 < (int)this->field_0xe3c) {
	//	do {
	//		S_STREAM_REF<ed_zone_3d>::Init(pSVar13);
	//		iVar12 = iVar12 + 1;
	//		pSVar13 = pSVar13 + 2;
	//	} while (iVar12 < (int)this->field_0xe3c);
	//}
	//iVar12 = (int)this->pShadow;
	//if (iVar12 != 0) {
	//	*(undefined4*)(iVar12 + 0x48) = 0x3ecccccd;
	//	*(undefined4*)((int)this->pShadow + 0x50) = 0x3ecccccd;
	//}
	//pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	//pCVar5 = CCameraManager::GetDefGameCamera(pCameraManager, 3);
	//this->pKyaJamgutCamera_0xaac = pCVar5;
	//pCVar5 = CCameraManager::GetDefGameCamera(pCameraManager, CT_KyaWindWall);
	//this->pCameraViewBase_0x15b0 = pCVar5;
	//pCVar5 = CCameraManager::GetDefGameCamera(pCameraManager, 6);
	//this->pWindWallCamera_0x15b4 = pCVar5;
	//pCVar5 = CCameraManager::GetDefGameCamera(pCameraManager, CT_KyaJamgut);
	//this->pCameraViewBase_0xab0 = pCVar5;
	//pCVar5 = CCameraManager::GetDefGameCamera(pCameraManager, 4);
	//this->pDeathCamera_0xab4 = pCVar5;
	//ByteCode::ByteCode(&BStack32);
	///* Boomy Snipe */
	//pCVar5 = AddCamera(pCameraManager, CT_SilverBoomy, &BStack32, s_Boomy_Snipe_004350a8);
	//*(CCamera**)&this->field_0xc94 = pCVar5;
	//(**(code**)(**(int**)&this->field_0xc94 + 0x30))();
	//(**(code**)(**(int**)&this->field_0xc94 + 0x18))();
	///* Death */
	//pCVar5 = AddCamera(pCameraManager, CT_Death, &BStack32, s_Death_004350b8);
	//this->field_0xab4 = (int)pCVar5;
	//(**(code**)(*(int*)this->field_0xab4 + 0x30))();
	//(**(code**)(*(int*)this->field_0xab4 + 0x18))();
	///* IntView */
	//*(undefined4*)(this->field_0xab4 + 8) = 0xffffff9c;
	//pCVar5 = AddCamera(pCameraManager, CT_IntView, &BStack32, s_IntView_004350c0);
	//*(CCamera**)&this->field_0x15bc = pCVar5;
	//piVar2 = *(int**)&this->field_0x15bc;
	//(**(code**)(*piVar2 + 0x30))(piVar2, CActorHero::_gThis);
	//(**(code**)(**(int**)&this->field_0x15bc + 0x18))();
	//*(undefined4*)&this->field_0x15c0 = 0;
	//CCameraManager::Func_001961e0(pCameraManager, this->pKyaJamgutCamera_0xaac);
	//pCVar5 = this->pKyaJamgutCamera_0xaac;
	//pCVar5[1].transformationMatrix.ca = (float)((uint)pCVar5[1].transformationMatrix.ca | 0x20000);
	//pCVar5 = this->pCameraViewBase_0xab0;
	//*(float*)&this->field_0x15c4 = pCVar5->fov;
	//fVar14 = (*((pCVar5->objBase).pVTable)->GetDistance)();
	//*(float*)&this->field_0x15c8 = fVar14;
	//*(float*)&this->field_0x15cc = (this->pKyaJamgutCamera_0xaac)->fov;
	//*(undefined4*)&this->field_0x15d0 = 0x40c00000;
	//ByteCode::~ByteCode(&BStack32, -1);
	//this->field_0xec = 10.3;
	//FUN_00136e60((int)this);
	//pAnimationController = this->pAnimationController;
	//ppeVar3 = ((pAnimationController->anmBinMetaAnimator).base.aAnimData)->pAnimManagerKeyData;
	//peVar11 = (edAnmAnim*)0x0;
	//iVar12 = CActor::GetIdMacroAnim((CActor*)this, 0x18d);
	//iVar12 = CAnimation::OnLoadFunc_0017f830(pAnimationController, (long)iVar12);
	//if (iVar12 != -1) {
	//	peVar11 = (edAnmAnim*)ppeVar3[iVar12];
	//}
	//this->pAnimKeyData_0x19b8 = peVar11;
	//*(undefined4*)&this->field_0x19b8 = 0;
	//*(undefined*)((int)&this->pAnimKeyData_0x19b4 + 1) = 1;
	//*(undefined*)((int)&this->pAnimKeyData_0x19b4 + 1) = 1;
	//this->field_0x1998.z = 0.2;
	//this->field_0x19a8 = this->field_0x19a8;
	//this->field_0x1998.w = 0.4;
	//*(undefined4*)&this->field_0x19ac = 0x40000000;
	//this->field_0x1998.y = 0.0;
	//uVar1 = ((pAnimationController->anmSkeleton).pTag)->boneCount;
	//pvVar6 = operator.new.array((long)(int)((uint)uVar1 * 0x30));
	//this->field_0xff0 = pvVar6;
	//CAnimation::SetField_0017f7f0(pAnimationController, (AnimData_30*)this->field_0xff0, (ulong)uVar1);
	//FUN_003cb7b0();
	//LevelScheduleManager::FUN_002db410(LevelScheduleManager::gThis, &this->field_0xadc);
	//ActorLoadBoomyStorm((Actor*)this);
	//if (*(int*)&this->field_0x12e0 != -1) {
	//	CAnimation::RegisterBone
	//	(this->pAnimationController, this->animKey_0x158c);
	//	CAnimation::RegisterBone
	//	(this->pAnimationController, this->animKey_0x1590);
	//	FUN_001d4a80(0.3333333, (int)&this->fxTrailA.field_0x4,
	//		this->objectId);
	//	CAnimation::RegisterBone
	//	(this->pAnimationController, this->animKey_0x1594);
	//	CAnimation::RegisterBone
	//	(this->pAnimationController, this->field_0x1594);
	//	FUN_001d4a80(0.3333333, (int)&this->fxTrailB.field_0x4,
	//		this->objectId);
	//}
	this->field_0xee0 = 0;
	//local_4 = LevelScheduleManager::ScenVar_Get(9);
	//CActor::DoMessage((CActor*)this, (CActor*)this, 0x62, local_4);
	//if (this->pBoomySnipeCamera_0xc94 != (CCamera*)0x0) {
	//	local_8 = LevelScheduleManager::ScenVar_Get(9);
	//	CActor::DoMessage((CActor*)this, (CActor*)this->pBoomySnipeCamera_0xc94, 0x62, local_8);
	//}
	//local_c = LevelScheduleManager::ScenVar_Get(10);
	//CActor::DoMessage((CActor*)this, (CActor*)this, 0x6b, local_c);
	//local_10 = 0;
	//CActor::DoMessage((CActor*)this, (CActor*)this, 0x79, 0);
	//*(undefined4*)&this->field_0xcbc = 0;
	//pFVar4 = CScene::ptable.g_FrontendManager_00451680;
	//uVar7 = (**(code**)((int)(this->character.characterBase.base.base.pVTable + 1) + 0x44))(this);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 0, uVar7);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 2, &this->field_0xab8);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 3, &this->field_0xad8);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 6, &this->field_0xadc);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 7, &this->field_0xc84);
	return;
}

CBehaviour* CActorHeroPrivate::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 0xb) {
		pNewBehaviour = &this->behaviour_0x1fd0;
	}
	else {
		if (behaviourType == 10) {
			pNewBehaviour = &this->behaviour_0x1e10;
		}
		else {
			if (behaviourType == 9) {
				pNewBehaviour = &this->behaviour_0x1c50;
			}
			else {
				if (behaviourType == 8) {
					pNewBehaviour = new CBehaviourHeroRideJamGut;
				}
				else {
					if (behaviourType == 7) {
						pNewBehaviour = &this->behaviourHeroDefault;
					}
					else {
						pNewBehaviour = CActorFighter::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}
	return pNewBehaviour;
}

void CActorHeroPrivate::SetState(int newState, int animType)
{
	uint uVar1;
	int iVar2;
	AnimResult* piVar3;
	Timer* pTVar4;
	undefined8 uVar5;
	ulong uVar6;
	bool bFightRelated;
	float fVar8;
	EActorState currentState;

	/* For stand to jump the type or mode is 0x78
	   For begin jump the type or mode is 0x79
	   Initial anim is -1 */
	currentState = this->actorState;
	uVar1 = TestState_IsInHit(0xffffffff);
	if (uVar1 != 0) {
		IMPLEMENTATION_GUARD(
		uVar5 = (**(code**)((int)(this->base.base.pVTable + 1) + 0x40))(this);
		fVar8 = (float)(**(code**)(*(int*)uVar5 + 0x24))(uVar5);
		if ((fVar8 <= 0.0) || (0.0 < *(float*)&this->field_0x2e4)) {
			newState = ChooseStateDead(this, 0xc, 4, 1);
			animType = AT_None;
		})
	}

	if ((currentState != newState) &&
		(uVar6 = GetBehaviourFlags(this->curBehaviourId), (uVar6 & 0x200) != 0)) {
		GetStateHeroFlags(newState);
		//::EmptyFunction();
	}

	bFightRelated = this->IsFightRelated(this->curBehaviourId);

	if ((bFightRelated != false) && (newState != AS_None)) {
		IMPLEMENTATION_GUARD(
		animType = ActorStateFunc_00327470((Actor*)this, newState, animType);)
	}

	uVar1 = GetStateHeroFlags(newState);
	uVar1 = TestState_IsOnAToboggan(uVar1);
	if (((uVar1 != 0) && (iVar2 = LevelScheduleManager::ScenVar_Get(0x10), 0 < iVar2)) &&
		(newState != 0xef)) {
		piVar3 = GetStateCfg(newState);
		animType = piVar3->field_0x0 + 0xa;
	}

	CActorMovable::SetState(newState, animType);

	if (currentState != newState) {
		pTVar4 = GetTimer();
		this->time_0x1538 = pTVar4->scaledTotalTime;
		pTVar4 = GetTimer();
		this->time_0x153c = pTVar4->scaledTotalTime;

		uVar6 =GetBehaviourFlags(this->curBehaviourId);
		if ((uVar6 & 0x200) != 0) {
			//::EmptyFunction();
		}
	}

	uVar1 = GetStateHeroFlags(this->actorState);
	this->heroFlags = uVar1;
	return;
}

void CActorHeroPrivate::CinematicMode_Leave(int behaviourId)
{
	CBehaviour* pCVar1;
	undefined8 uVar2;
	CActor* pOtherActor;
	edF32VECTOR4 local_30;
	undefined4 local_18;
	CActorHeroPrivate* local_14;
	CActor* local_10;
	undefined4 local_8;
	undefined4 local_4;

	pOtherActor = (CActor*)0x0;
	if (behaviourId == 8) {
		IMPLEMENTATION_GUARD(
		CActor::GetBehaviour((CActor*)this, 8);
		LevelScheduleManager::ScenVar_Get(0xd);
		local_18 = 0x42c80000;
		local_10 = (CActor*)0x0;
		local_30.x = (float)this->pCollisionData;
		local_30.y = *(float*)&this->currentLocation;
		local_30.z = *(float*)((int)&this->currentLocation + 4);
		local_30.w = 10.0;
		local_14 = this;
		KyaVectorFunc(&(CScene::ptable.g_ActorManager_004516a4)->cluster, &local_30, &LAB_0033d920, &local_18);
		pOtherActor = local_10;
		local_4 = 0;
		uVar2 = CActor::DoMessage((CActor*)this, local_10, 0x4d, (ActorCompareStruct*)0x0);
		*(int*)&this->field_0x15a0 = (int)uVar2;
		local_8 = 0;
		CActor::DoMessage((CActor*)this, pOtherActor, 0x14, (ActorCompareStruct*)0x0);)
	}
	CActor::CinematicMode_Leave(behaviourId);
	if (behaviourId == 8) {
		IMPLEMENTATION_GUARD(
		pCVar1 = CActor::GetBehaviour((CActor*)this, (int)this->aComponents);
		(*(code*)pCVar1->pVTable[1].Init)(pCVar1, pOtherActor, *(undefined4*)&this->field_0x15a0, 7, 0x11d);
		(*((this->pVTable)->actorBase).SetState)((CActor*)this, 0x11e, -1);)
	}
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x19b1 = 1;
	this->field_0xffc = 0.0f;)
	return;
}

void CActorHeroPrivate::ResetStdDefaultSettings()
{
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1040 = 9.77f;
	this->field_0x1044 = 3.91f;
	this->field_0x104c = 2.5f;
	this->field_0x1050 = 5.4f;
	this->field_0x1054 = 1.0f;
	this->field_0x1058 = 15.0f;)
	this->field_0x105c = 50.0f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1060 = 4.0f;
	this->field_0x1150 = 17.0f;
	this->field_0x1154 = 1.7f;
	this->field_0x1158 = 0.2f;
	this->field_0x115c = 0.18f;
	this->field_0x1160 = 0.1f;
	this->field_0x1164 = 1.5f;
	this->field_0x1174 = 5.0f;
	this->field_0x1178 = 1.9f;
	this->field_0x117c = 1.4f;
	this->field_0x1180 = 0.4f;
	this->field_0x1184 = 0.1f;
	this->field_0x1188 = 0.0f;
	this->field_0x1430 = 1.396263f;
	this->field_0x1570 = 0.6f;
	this->field_0x1574 = 0.0f;)
	return;
}

void CActorHeroPrivate::ResetBoomyDefaultSettings()
{
	uint* puVar1;
	ulong uVar2;
	IMPLEMENTATION_GUARD_LOG(

	CScene::GetManager(MO_Camera);
	(this->base).field_0x1ba0 = 0x41000000;
	*(undefined4*)&(this->base).field_0x1b98 = 0;
	*(undefined4*)&(this->base).field_0x1b9c = 0;
	(this->base).field_0x1b94 = 0;
	*(undefined**)&(this->base).field_0x1b90 = &DAT_bf800000;
	*(undefined4*)&(this->base).field_0x1b64 = 0;
	(this->base).field_0x1b68 = 0;
	(this->base).field_0x1b6c = 0;
	APlayer::StateHeroBoomySnipeBack2Stand_Term((CActorHero*)this, 0);
	*(undefined4*)&(this->base).field_0x1b78 = 0;
	(this->base).field_0x1b74 = 0;
	*(undefined**)&(this->base).field_0x1000 = &DAT_bf800000;
	*(undefined4*)&(this->base).field_0x1004 = 0x41200000;
	*(undefined4*)&(this->base).field_0x1008 = 0x40a00000;
	DisableLayer(this, 8);
	SetMagicMode(this, 0);
	uVar2 = FUN_0034cdc0();
	if (uVar2 == 0) {
		CAnimation::AddDisabledBone
		((this->base).character.characterBase.base.base.data.pAnimationController, (this->base).animKey_0x1584);
	}
	else {
		CAnimation::RemoveDisabledBone
		((this->base).character.characterBase.base.base.data.pAnimationController, (this->base).animKey_0x1584);
	}
	/* BOOMY_1 */
	(this->base).field_0x1ba4 = 0;
	puVar1 = CActorFighter::FindBlowByName((CActorHero*)this, s_BOOMY_1_00429998);
	*(uint**)&(this->base).field_0x1bb0 = puVar1;
	puVar1 = CActorFighter::FindBlowByName((CActorHero*)this, s_BOOMY_2_004299a0);
	*(uint**)&(this->base).field_0x1bb4 = puVar1;
	puVar1 = CActorFighter::FindBlowByName((CActorHero*)this, s_BOOMY_3_004299a8);
	(this->base).field_0x1bb8 = (undefined*)puVar1;)
	return;
}

int CActorHeroPrivate::StateEvaluate()
{
	//InputManager* pIVar1;
	int iVar2;
	bool bVar3;
	int iVar4;
	uint uVar5;
	//AnimResult* pAVar6;
	long lVar7;
	undefined8 uVar8;
	edF32VECTOR4* peVar9;
	float fVar10;
	edF32VECTOR4 local_10;

	if ((this->field_0xee0 == 0) && (this->prevBehaviourId != 1)) {
		IMPLEMENTATION_GUARD(
		fVar10 = this->dynamicExt.base.field_0x10.z;
		lVar7 = (**(code**)((int)(this->base.base.pVTable + 1) + 8))();
		peVar9 = (edF32VECTOR4*)0x0;
		if (lVar7 != 0) {
			iVar4 = (**(code**)((int)(this->base.base.pVTable + 1) + 8))(this);
			peVar9 = *(edF32VECTOR4**)(iVar4 + 0x2c);
		}
		iVar4 = GetPossibleWind(fVar10, this, (edF32VECTOR4*)&this->dynamicExt.base.field_0x40
			, peVar9);
		if (iVar4 == -1) {
			if ((*(byte*)((int)this->pCollisionData + 4) & 2) == 0) {
				bVar3 = InClimbZone((CActorHero*)this, &this->currentLocation);
				*(bool*)&this->field_0x1454 = bVar3;
				iVar4 = 0xa9;
				if (*(char*)&this->field_0x1454 == '\0') {
					iVar4 = ChooseStateFall((CActorHero*)this, 0);
				}
			}
			else {
				iVar4 = this->prevActorState;
				if (iVar4 == -1) {
					uVar5 = 0;
				}
				else {
					pAVar6 = (*(this->base.base.pVTable)->GetStateCfg)((CActor*)this, iVar4);
					uVar5 = pAVar6->flags_0x4 & 0x100;
				}
				if (uVar5 == 0) {
					iVar4 = ActorFunc_0014bdf0((float)this->base.dynamic.field_0x44 *
						this->base.dynamic.field_0x20.y, (CActorHero*)this);
					if (iVar4 == -1) {
						iVar4 = 0x73;
					}
				}
				else {
					pIVar1 = this->pPlayerInput;
					if ((pIVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar10 = 0.0;
					}
					else {
						fVar10 = pIVar1->buttonArray[5].floatFieldB;
					}
					iVar4 = 0x85;
					if (fVar10 == 0.0) {
						if ((pIVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar10 = 0.0;
						}
						else {
							fVar10 = pIVar1->field_0x5fc;
						}
						if (0.3 < fVar10) {
							if ((pIVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
								fVar10 = 0.0;
							}
							else {
								fVar10 = pIVar1->buttonArray[5].floatFieldB;
							}
							iVar4 = 0x86;
							if (fVar10 == 0.0) {
								if ((pIVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar10 = 0.0;
								}
								else {
									fVar10 = pIVar1->field_0x5fc;
								}
								iVar4 = 0x77;
								if (0.9 <= fVar10) {
									iVar4 = 0x76;
								}
							}
						}
						else {
							iVar4 = 0x73;
						}
					}
				}
			}
		}
		(*(this->base.base.pVTable)->SetState)((CActor*)this, iVar4, -1);
		uVar8 = (**(code**)((int)(this->base.base.pVTable + 1) + 0x40))(this);
		fVar10 = (float)(**(code**)(*(int*)uVar8 + 0x24))(uVar8);
		bVar3 = fVar10 - *(float*)&this->field_0x2e4 <= 0.0;
		if (!bVar3) {
			iVar2 = this->actorState;
			if (iVar2 == -1) {
				uVar5 = 0;
			}
			else {
				pAVar6 = (*(this->base.base.pVTable)->GetStateCfg)((CActor*)this, iVar2);
				uVar5 = pAVar6->flags_0x4 & 1;
			}
			bVar3 = uVar5 != 0;
		}
		if (bVar3) {
			iVar4 = ActorStateFunc_0013f0c0((Actor*)this, 0xc, 4, 1);
			(*(this->base.base.pVTable)->SetState)((CActor*)this, iVar4, 0xffffffff);
		})
	}
	else {
		iVar4 = 0x73;
		SetState(0x73, -1);
		if ((this->prevBehaviourId == 1) &&
			((fVar10 = this->distanceToGround, 0.07f < fVar10 && (fVar10 < 0.3f))
				)) {

			local_10.x = this->currentLocation.x;
			local_10.z = this->currentLocation.z;
			local_10.w = this->currentLocation.w;
			local_10.y = this->currentLocation.y - this->distanceToGround;

			UpdatePosition(&local_10, true);
		}
	}
	return iVar4;
}

void CActorHeroPrivate::ClearLocalData()
{
	//float fVar1;
	//uint* puVar2;
	//float fVar3;
	//float fVar4;
	Timer* pTVar5;
	//CCameraManager* iVar4;
	CPlayerInput* pIVar6;
	//int iVar7;
	//int iVar8;
	//float* pfVar9;
	//int iVar10;
	//edF32MATRIX4* peVar11;
	//CActorHeroPrivate* pCVar12;
	//undefined4 uVar13;
	//undefined4 uVar14;
	//undefined4 uVar15;
	//
	//this->field_0x102c.x = *(float*)((int)&this->rotationEuler + 4);
	this->heroFlags = 0;
	//*(undefined4*)&this->field_0x1020 = 1;
	//this->field_0x1420 = 0;
	//*(undefined4*)&this->field_0x1424 = 0;
	//*(undefined4*)&this->field_0x1428 = 1;
	//*(undefined4*)&this->field_0x142c = 1;
	//*(undefined4*)&this->field_0x1024 = 0x3f800000;
	//this->field_0x1010 = 0;
	this->field_0x1014 = 1;
	this->field_0xff4 = 0;
	this->effort = 0.0f;
	//this->field_0xff8 = 0xe6;
	//*(undefined4*)&this->field_0x100c = 0;
	//*(undefined4*)&this->field_0x1a54 = 0;
	this->field_0x1610 = 0;
	//*(undefined4*)&this->field_0x1614 = 0;
	//pTVar5 = Timer::GetTimer();
	//*(float*)&this->field_0x155c = pTVar5->scaledTotalTime;
	//*(undefined4*)&this->field_0x1560 = 0;
	//*(undefined4*)&this->field_0x1568 = 0;
	//ActorTimeFunc_00325c40(20.0, (Actor*)this, 0);
	//*(undefined4*)&this->field_0x1168 = 1;
	//uVar15 = *(undefined4*)((int)&this->currentLocation + 4);
	//uVar13 = *(undefined4*)((int)&this->currentLocation + 8);
	//uVar14 = *(undefined4*)((int)&this->currentLocation + 0xc);
	//*(undefined4*)&this->field_0xf00 =
	//	*(undefined4*)&this->currentLocation;
	//*(undefined4*)&this->field_0xf04 = uVar15;
	//*(undefined4*)&this->field_0xf08 = uVar13;
	//*(undefined4*)&this->field_0xf0c = uVar14;
	//this->field_0x102c.y = 0.0;
	//this->field_0x102c.z = 1.0;
	//this->field_0x102c.w = 0.0;
	//this->field_0x103c = 0.0;
	//*(undefined4*)&this->field_0x10a0 = 0;
	//*(undefined4*)&this->field_0x10a4 = 0x3f800000;
	//*(undefined4*)&this->field_0x10a8 = 0;
	//*(undefined4*)&this->field_0x10ac = 0;
	//*(undefined4*)&this->field_0x10b0 = 0;
	//*(undefined4*)&this->field_0x10b4 = 0x3f800000;
	//*(undefined4*)&this->field_0x10b8 = 0;
	//*(undefined4*)&this->field_0x10bc = 0;
	//*(undefined4*)&this->field_0x1090 = 0;
	//*(undefined4*)&this->field_0x1094 = 0;
	//this->field_0x1064 = 0.0;
	//*(undefined*)&this->field_0x1454 = 0;
	//*(undefined*)&this->field_0x1455 = 0;
	//this->pCheckpointManagerSubObjA_0x14a4 = (S_CHECKPOINT*)0x0;
	//*(undefined4*)&this->field_0x14f4 = *(undefined4*)&this->field_0x14cc;
	//*(undefined4*)&this->field_0x14a8 = 0;
	//*(undefined4*)&this->field_0x1500 = 0;
	//*(undefined4*)&this->field_0x1504 = 0;
	//*(undefined4*)&this->field_0x1508 = 0;
	//*(undefined4*)&this->field_0x150c = 0;
	//*(undefined**)&this->field_0xa90 = &DAT_bf800000;
	//*(undefined4*)&this->field_0xa94 = 0;
	//*(undefined4*)&this->field_0xa98 = 0;
	//this->field_0xa9c = 0;
	//*(undefined**)&this->field_0x1460 = &DAT_bf800000;
	//*(undefined4*)&this->field_0x1464 = 0;
	//*(undefined4*)&this->field_0x1468 = 0;
	//*(undefined4*)&this->field_0x146c = 0;
	//*(undefined**)&this->field_0x1470 = &DAT_bf800000;
	//*(undefined4*)&this->field_0x1474 = 0;
	//*(undefined4*)&this->field_0x1478 = 0;
	//*(undefined4*)&this->field_0x147c = 0;
	//*(undefined4*)&this->field_0x1480 = 0;
	//*(undefined**)&this->field_0x1484 = &DAT_bf800000;
	//*(undefined4*)&this->field_0x1488 = 0;
	//*(undefined4*)&this->field_0x148c = 0;
	//*(undefined4*)&this->field_0x1490 = 0;
	//*(undefined4*)&this->field_0x1494 = 0;
	//*(undefined4*)&this->field_0x1498 = 0;
	//*(undefined4*)&this->field_0x149c = 0;
	//*(undefined**)&this->field_0x13f0 = &DAT_bf800000;
	//*(undefined4*)&this->field_0x13f4 = 0;
	//*(undefined4*)&this->field_0x13f8 = 0;
	//*(undefined4*)&this->field_0x13fc = 0;
	//*(undefined4*)&this->field_0x1400 = 0;
	//*(undefined4*)&this->field_0x1404 = 0;
	//*(undefined4*)&this->field_0x1408 = 0x3f800000;
	//*(undefined4*)&this->field_0x140c = 0;
	//if (this == (CActorHeroPrivate*)CActorHero::_gThis) {
	//	iVar4 = (CCameraManager*)CScene::GetManager(MO_Camera);
	//	CCameraStack::Reset(&iVar4->cameraStack);
	//}
	//*(undefined4*)&this->field_0xf14 = 0;
	//*(undefined4*)&this->field_0xf18 = 0;
	//*(undefined4*)&this->field_0xf1c = 0;
	//this->field_0xf20 = (undefined*)0x0;
	this->field_0xf24 = (char*)0x0;
	//*(undefined4*)&this->field_0xf28 = 0;
	//this->field_0xf50 = (undefined*)0x0;
	//*(undefined4*)&this->field_0xf58 = 0;
	//*(undefined4*)&this->field_0x1490 = 0;
	//*(undefined4*)&this->field_0x1494 = 0;
	//*(undefined4*)&this->field_0x1498 = 0;
	//*(undefined4*)&this->field_0x149c = 0;
	//*(undefined4*)&this->field_0xf5c = 0;
	//*(undefined4*)&this->field_0x15a0 = 1;
	//this->field_0x1048 = 0;
	this->time_0x1538 = 0.0f;
	this->time_0x153c = 0.0f;
	//*(undefined4*)&this->field_0x118c = 0x3f800000;
	//*(undefined4*)&this->field_0x1450 = 0xa7;
	pTVar5 = GetTimer();
	this->time_0x1538 = pTVar5->scaledTotalTime;
	pTVar5 = GetTimer();
	this->time_0x153c = pTVar5->scaledTotalTime;
	//pTVar5 = GetTimer();
	//*(float*)&this->field_0x1540 = pTVar5->scaledTotalTime;
	//pTVar5 = GetTimer();
	//*(float*)&this->field_0x1548 = pTVar5->scaledTotalTime;
	//*(undefined4*)&this->field_0x154c = 0;
	//this->field_0x1554 = 0.0;
	//*(undefined4*)&this->field_0x1550 = 0;
	//this->field_0x1544 = 0.0;
	//this->field_0x1558 = 0.0;
	//*(undefined4*)&this->field_0x1018 = 0;
	this->field_0xa80 = 0.0;
	this->field_0xa84 = 0.0;
	//*(undefined4*)&this->field_0xa88 = 0;
	//CVectorDyn::Reset((undefined4*)&this->field_0xf70);
	//SetJumpCfg(0.1, *(float*)&this->field_0x1050, *(float*)&this->field_0x1158, this->field_0x1150,
	//	this->field_0x1154, (Actor*)this, 1, (edF32VECTOR4*)0x0);
	//*(undefined4*)&this->field_0xf10 = 0;
	//fVar4 = gF32Vector4Zero.w;
	//fVar3 = gF32Vector4Zero.z;
	//fVar1 = gF32Vector4Zero.y;
	//peVar11 = &gF32Matrix4Unit;
	//pfVar9 = &this->field_0x18fc.ab;
	//iVar8 = 8;
	//this->field_0x18ec.y = gF32Vector4Zero.x;
	//this->field_0x18ec.z = fVar1;
	//this->field_0x18ec.w = fVar3;
	//this->field_0x18fc.aa = fVar4;
	//do {
	//	iVar8 = iVar8 + -1;
	//	fVar1 = peVar11->ab;
	//	*pfVar9 = peVar11->aa;
	//	peVar11 = (edF32MATRIX4*)&peVar11->ac;
	//	pfVar9[1] = fVar1;
	//	pfVar9 = pfVar9 + 2;
	//} while (0 < iVar8);
	//pfVar9 = &this->field_0x193c.ab;
	//peVar11 = &gF32Matrix4Unit;
	//iVar8 = 8;
	//do {
	//	iVar8 = iVar8 + -1;
	//	fVar1 = peVar11->ab;
	//	*pfVar9 = peVar11->aa;
	//	peVar11 = (edF32MATRIX4*)&peVar11->ac;
	//	pfVar9[1] = fVar1;
	//	pfVar9 = pfVar9 + 2;
	//} while (0 < iVar8);
	//this->field_0x197c.pLightDirections = (edF32MATRIX4*)&this->field_0x18ec.y;
	//this->field_0x197c.pLightColorMatrix = (edF32MATRIX4*)&this->field_0x18fc.ab;
	//this->field_0x1988 = (float)&this->field_0x193c.ab;
	//CActor::SetAlpha((CActor*)this, 0x80);
	this->animKey_0x157c = this->currentLocation.y + 20.0f;
	//*(undefined4*)&this->field_0x11f0 =
	//	*(undefined4*)((int)&this->currentLocation + 4);
	//_ResetHeroFight(this);
	//iVar8 = (int)this->pCollisionData;
	//if (iVar8 != 0) {
	//	*(uint*)iVar8 = *(uint*)iVar8 | 0x10000;
	//	iVar8 = (int)this->pCollisionData;
	//	*(uint*)iVar8 = *(uint*)iVar8 | 0x81003;
	//	iVar8 = (int)this->pCollisionData;
	//	*(uint*)iVar8 = *(uint*)iVar8 | 0x4000;
	//	iVar8 = *(int*)&this->field_0xf54;
	//	if (iVar8 != 0) {
	//		puVar2 = *(uint**)(iVar8 + 0x2c);
	//		*puVar2 = *puVar2 & 0xffffdfff;
	//	}
	//}

	this->field_0xf54 = 0;

	if (this == (CActorHeroPrivate*)CActorHero::_gThis) {
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
		this->flags = this->flags | 0x80;
		this->flags = this->flags & 0xffffffdf;
		EvaluateDisplayState();
		this->flags = this->flags | 0x800;
	}

	//*(undefined4*)&this->field_0xd28 = 0;
	//*(undefined4*)&this->field_0xd30 = 0;
	//this->mode_0xd30 = 0;
	this->field_0x18dc = 1;
	if (this == CActorHero::_gThis) {
		pIVar6 = GetPlayerInput(0);
		this->field_0x1610 = 0;
		if (pIVar6 == (CPlayerInput*)0x0) {
			this->field_0x18dc = 1;
		}
		else {
			this->pPlayerInput = pIVar6;
			this->field_0x18dc = 0;
		}
	}
	else {
		pIVar6 = GetPlayerInput(1);
		this->field_0x1610 = 0;
		if (pIVar6 == (CPlayerInput*)0x0) {
			this->field_0x18dc = 1;
		}
		else {
			this->pPlayerInput = pIVar6;
			this->field_0x18dc = 0;
		}
	}

	//this->boomyThrowState = BTS_None;
	//this->field_0x19fc = 0;
	//*(undefined4*)&this->field_0x1a00 = 0;
	//this->field_0x1a10 = 0;
	//*(undefined4*)&this->field_0x1a14 = 0;
	//*(undefined4*)&this->field_0x1a18 = 0;
	//iVar8 = 0;
	//*(undefined4*)&this->field_0x1a20 = 0;
	//*(undefined4*)&this->field_0x1a24 = 0;
	//*(undefined4*)&this->field_0x1a28 = 0;
	//*(undefined4*)&this->field_0x1a2c = 0;
	//*(undefined4*)&this->field_0x1a40 = 0;
	//*(undefined4*)&this->field_0x1a44 = 0;
	this->flags = this->flags & 0xfff7ffff;
	//*(undefined4*)&this->field_0x10fc = 0;
	//*(undefined4*)&this->field_0x1100 = 0;
	//*(undefined4*)&this->field_0x110c = 0;
	//*(undefined4*)&this->field_0x1110 = 0;
	//*(undefined4*)&this->field_0x1114 = 0;
	//*(undefined4*)&this->field_0x13cc = 0;
	//this->bUnknownBool = 1;
	//this->field_0x1c38 = -NAN;
	//this->stateMachine_0x1c40.pVTable = (CBehaviourVtable*)0x0;
	//this->field_0xe44 = (int*)0x0;
	//pCVar12 = this;
	//do {
	//	iVar10 = 0;
	//	do {
	//		iVar7 = (int)&(pCVar12->base).character.characterBase.base.base.pVTable + iVar10;
	//		if (1 < *(byte*)(iVar7 + 0xd34)) {
	//			*(undefined*)(iVar7 + 0xd34) = 0;
	//		}
	//		iVar10 = iVar10 + 1;
	//	} while (iVar10 < 0x10);
	//	iVar8 = iVar8 + 1;
	//	pCVar12 = (CActorHeroPrivate*)&(pCVar12->base).character.characterBase.base.base.data.field_0xc;
	//} while (iVar8 < 0x10);
	this->effort = 0.0;
	return;
}

void CActorHeroPrivate::BehaviourHero_InitState(int newState)
{
	switch (newState) {
	case 0x73:
		StateHeroStandInit(1);
		break;
	default:
		assert(false);
		break;
	}
}

void CActorHeroPrivate::BehaviourHero_TermState(int oldState, int newState)
{
	switch (newState) {
	case AS_None:
		StateHeroStandInit(1);
		break;
	default:
		assert(false);
		break;
	}
}

void CActorHeroPrivate::BehaviourHero_Manage()
{
	switch (this->actorState) {
	case 0x73:
		StateHeroStand(1);
		break;
	default:
		assert(false);
		break;
	}
}

void CActorHeroPrivate::StateHeroStandInit(int param_2)
{
	this->field_0x1014 = 1;
	this->field_0xff4 = 0;
	if (param_2 != 0) {
		if (5.0f < this->effort) {
			PlayAnim(0x240);
		}
		else {
			this->effort = 0.0f;
		}
	}
	return;
}

void CActorHeroPrivate::StateHeroStand(int param_2)
{
	char* pcVar1;
	int iVar2;
	CPlayerInput* pCVar3;
	int iVar4;
	uint uVar5;
	edF32VECTOR4* peVar6;
	//AnimationType* pAVar7;
	Timer* pTVar8;
	int iVar9;
	bool bSuccess;
	long lVar10;
	ulong uVar11;
	undefined4 uVar12;
	//AnimationType regularAnimType;
	float fVar13;
	float fVar14;
	undefined* in_f21;
	undefined* unaff_f20;

	pcVar1 = this->field_0xf24;
	iVar9 = (int)this->pCollisionData;
	iVar2 = (int)this->pAnimationController;
	if (pcVar1 != (char*)0x0) {
		IMPLEMENTATION_GUARD(
		param_2 = 0;
		CActor::SV_UpdateOrientationToPosition2D(this->field_0x1040 * 0.5, (CActor*)this, pcVar1 + 0x30);
		this->field_0x1014 = 1;
		lVar10 = (*(code*)(this->character.characterBase.base.base.pVTable)->IsLookingAt)(this);
		if ((lVar10 != 0) && (gPlayerInput.buttonArray[4].floatFieldB != 0.0)) {
			fVar14 = gPlayerInput.field_0x5f4;
			if (gPlayerInput.field_0x5fc * 0.7853982 < ABS(gPlayerInput.field_0x5f8)) {
				fVar14 = 0.0;
			}
			fVar13 = gPlayerInput.field_0x5f8;
			if (gPlayerInput.field_0x5fc * 0.7853982 < ABS(fVar14)) {
				fVar13 = 0.0;
			}
			fVar13 = edFIntervalDotSrcLERP(fVar13, 0.5235988, -0.4363323);
			(**(code**)((int)(this->character.characterBase.base.base.pVTable + 1) + 0x24))
				(fVar13 - 0.1745329, -(fVar14 * 50.0 * 0.01745329), 0x41490fdb, this);
		})
	}
	if (param_2 != 0) {
		iVar4 = this->currentAnimType;
		if (iVar4 == 0xf0) {
			IMPLEMENTATION_GUARD(
			SV_UpdateValue(0.0f, 5.0f, &this->effort);)
			if (this->effort == 0.0f) {
				IMPLEMENTATION_GUARD(
				iVar4 = *(int*)(iVar2 + 4);
				if (*(int*)(iVar4 + 0x10) == *(int*)(iVar2 + 0x30)) {
					bSuccess = false;
					if (*(int*)(iVar4 + 8) != 0) {
						bSuccess = (*(uint*)(iVar4 + 0xcc) & 2) != 0;
					}
				}
				else {
					bSuccess = false;
				}
				if (bSuccess) {
					CActor::PlayAnim((CActor*)this, New_Name);
				})
			}
			param_2 = 0;
		}
		else {
			if (iVar4 == 0xf1) {
				IMPLEMENTATION_GUARD(
				iVar4 = *(int*)(iVar2 + 4);
				if (*(int*)(iVar4 + 0x10) == *(int*)(iVar2 + 0x30)) {
					bSuccess = false;
					if (*(int*)(iVar4 + 8) != 0) {
						bSuccess = (*(uint*)(iVar4 + 0xcc) & 2) != 0;
					}
				}
				else {
					bSuccess = false;
				}
				if (bSuccess) {
					pAVar7 = (AnimationType*)
						(this->GetStateCfg)
						((CActor*)this, this->actorState);
					CActor::PlayAnim((CActor*)this, *pAVar7);
				}
				param_2 = 0;)
			}
		}
	}

	this->dynamic.rotationQuat = this->rotationQuat;

	SV_MOV_UpdateSpeedIntensity(0.0f, this->field_0x105c);

	this->dynamicExt.field_0x50.x = 0.0f;
	this->dynamicExt.field_0x50.y = 0.0f;
	this->dynamicExt.field_0x50.z = 0.0f;
	this->dynamicExt.field_0x50.w = 0.0f;

	this->dynamicExt.field_0x6c = 0.0f;

	fVar14 = ManageDyn(4.0f, 0x1002023b, 0);

	iVar4 = 0; // DetectGripablePrecipice(this);
	if (iVar4 == 0) {
		IMPLEMENTATION_GUARD_LOG(
		if ((*(byte*)(iVar9 + 4) & 2) == 0) {
			fVar14 = this->field_0x1184;
			if (fVar14 < this->timeA) {
				if ((*(byte*)((int)this->pCollisionData + 4) & 2) == 0) {
					uVar12 = 0x7e;
					if (*(int*)&this->field_0x142c != 0) {
						if (this->distanceToGround < 10.3) {
							uVar12 = 0x7e;
							*(undefined4*)&this->field_0x1020 = 1;
						}
						else {
							uVar12 = 0xf0;
							this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
						}
					}
				}
				else {
					uVar12 = 0x7a;
					*(undefined4*)&this->field_0x1020 = 1;
				}
				this->SetState(uVar12, 0xffffffff);
				return;
			}
		}
		else {
			this->timeA = 0.0f;
		})

		IMPLEMENTATION_GUARD_LOG(
		if (this->field_0x1574 == 0.0f) {
			pCVar3 = this->pPlayerInput;
			if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				/* No input manager so jump key can't be pressed */
				uVar5 = 0;
			}
			else {
				/* Check with the input manager if the jump key is pressed */
				uVar5 = (uint)pCVar3->pressedBitfield & 0x10;
			}
			if (uVar5 == 0) {
				uVar11 = CanEnterToboggan();
				if (uVar11 == 0) {
					if ((this->dynamic.flags & 2) == 0) {
						pCVar3 = this->pPlayerInput;
						if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar14 = 0.0f;
						}
						else {
							fVar14 = pCVar3->buttonArray[5].floatFieldB;
						}
						if (fVar14 == 0.0f) {
							pCVar3 = this->pPlayerInput;
							if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
								fVar14 = 0.0f;
							}
							else {
								fVar14 = pCVar3->field_0x5fc;
							}
							if ((0.3 < fVar14) || (this->field_0x1014 == 0)) {
								pCVar3 = this->pPlayerInput;
								if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar14 = 0.0;
								}
								else {
									fVar14 = pCVar3->field_0x5fc;
								}
								if (0.3 < fVar14) {
									/* Moving */
									this->field_0x1014 = 0;
									pCVar3 = this->pPlayerInput;
									if ((pCVar3 == (CPlayerInput*)0x0) ||
										(peVar6 = &pCVar3->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->field_0x102c.y = peVar6->x;
									pCVar3 = this->pPlayerInput;
									if ((pCVar3 == (CPlayerInput*)0x0) ||
										(peVar6 = &pCVar3->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->field_0x102c.z = peVar6->y;
									pCVar3 = this->pPlayerInput;
									if ((pCVar3 == (CPlayerInput*)0x0) ||
										(peVar6 = &pCVar3->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->field_0x102c.w = peVar6->z;
									pCVar3 = this->pPlayerInput;
									if ((pCVar3 == (CPlayerInput*)0x0) ||
										(peVar6 = &pCVar3->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->field_0x103c = peVar6->w;
									if (0.0 <= this->field_0x102c.y *
										*(float*)((int)&this->rotationQuat + 8) -
										*(float*)&this->rotationQuat *
										this->field_0x102c.w) {
										CActor::PlayAnim((CActor*)this, New_Name_(231));
									}
									else {
										CActor::PlayAnim((CActor*)this, New_Name_(230));
									}
									if (0xea < this->field_0xff8) {
										this->field_0xff8 = 0xe6;
									}
								}
								/* Moving */
								bSuccess = CActor::SV_UpdateOrientation2D
								(18.84, (Actor*)this, (edF32VECTOR4*)&this->field_0x102c.y, 0);
								this->field_0x1014 = (int)bSuccess;
								if (this->field_0x1014 != 0) {
									pAVar7 = (AnimationType*)
										(this->GetStateCfg)
										((CActor*)this, this->actorState);
									regularAnimType = *pAVar7;
									CActor::PlayAnim((CActor*)this, regularAnimType);
									this->prevAnimType = regularAnimType;
								}
							}
							if (*(int*)&this->field_0x1a4c == 0) {
								pCVar3 = this->pPlayerInput;
								if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar14 = 0.0;
								}
								else {
									fVar14 = pCVar3->field_0x5fc;
								}
								if (0.3 < fVar14) {
									pCVar3 = this->pPlayerInput;
									if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
										fVar14 = 0.0;
									}
									else {
										fVar14 = pCVar3->field_0x5fc;
									}
									if (fVar14 < 0.9) {
										ActorFunc_0011ab90(0.0, 0.9, (Actor*)this, (float*)&this->field_0x1048);
										pTVar8 = GetTimer();
										this->time_0x153c = pTVar8->scaledTotalTime;
										pTVar8 = GetTimer();
										if ((0.1 < pTVar8->scaledTotalTime - this->time_0x1538) && (this->field_0x1014 != 0))
										{
											(this->SetState)
												((CActor*)this, 0x77, 0xffffffff);
										}
									}
									else {
										pCVar3 = this->pPlayerInput;
										if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
											fVar14 = 0.0;
										}
										else {
											fVar14 = pCVar3->field_0x5fc;
										}
										if (0.9 < fVar14) {
											ActorFunc_0011ab90(1.0, 0.9, (Actor*)this, (float*)&this->field_0x1048);
											pTVar8 = GetTimer();
											if ((0.1 < pTVar8->scaledTotalTime - this->time_0x153c) && (this->field_0x1014 != 0)
												) {
												this->SetState(0x76, 0xf3);
											}
										}
										else {
											pTVar8 = GetTimer();
											this->time_0x153c = pTVar8->scaledTotalTime;
										}
									}
								}
								else {
									pTVar8 = GetTimer();
									this->time_0x153c = pTVar8->scaledTotalTime;
									this->field_0x1048 = 0;
									if (((param_2 == 0) || (this->field_0x1014 == 0)) || (*(int*)&this->field_0x1b70 != 0))
									{
										this->idleTimer = 0.0;
									}
									else {
										iVar9 = *(int*)(iVar2 + 4);
										if (*(int*)(iVar9 + 0x10) == *(int*)(iVar2 + 0x30)) {
											bSuccess = false;
											if (*(int*)(iVar9 + 8) != 0) {
												bSuccess = (*(uint*)(iVar9 + 0xcc) & 2) != 0;
											}
										}
										else {
											bSuccess = false;
										}
										if (bSuccess) {
											/* Idle animation trigger */
											regularAnimType = AT_None;
											if (this->field_0xff4 == 0) {
												fVar14 = 4.0;
											}
											else {
												iVar9 = rand();
												fVar14 = ((float)iVar9 / 2.147484e+09) * 2.4 + 0.9;
											}
											switch (this->field_0xff8) {
											case 0xe6:
												in_f21 = (undefined*)0x3f800000;
												unaff_f20 = (undefined*)0x3fe66666;
												break;
											case 0xe7:
												in_f21 = &DAT_3f4ccccd;
												unaff_f20 = &DAT_3f99999a;
												break;
											case 0xe8:
												in_f21 = (undefined*)0x3f800000;
												unaff_f20 = (undefined*)0x40000000;
												break;
											case 0xe9:
												in_f21 = &DAT_3f4ccccd;
												unaff_f20 = &DAT_3f99999a;
												break;
											case 0xea:
												in_f21 = &DAT_3f4ccccd;
												unaff_f20 = &DAT_3f99999a;
												regularAnimType = New_Name_(199);
												break;
											case 0xeb:
												in_f21 = &DAT_3f4ccccd;
												unaff_f20 = &DAT_3f99999a;
												break;
											case 0xec:
												in_f21 = &DAT_3f4ccccd;
												unaff_f20 = &DAT_3f99999a;
												break;
											case 0xed:
												in_f21 = (undefined*)0x3f800000;
												unaff_f20 = (undefined*)0x3fe66666;
												break;
											case 0xee:
												in_f21 = &DAT_3f4ccccd;
												unaff_f20 = &DAT_3f99999a;
												break;
											case 0xef:
												in_f21 = &DAT_3f4ccccd;
												regularAnimType = AT_Stand;
												unaff_f20 = &DAT_3f99999a;
											}
											/* Start new animation */
											iVar9 = rand();
											bSuccess = CActor::PlayWaitingAnimation
											(fVar14, (float)in_f21 +
												((float)unaff_f20 - (float)in_f21) * ((float)iVar9 / 2.147484e+09),
												(CActor*)this, this->field_0xff8, regularAnimType, 1);
											if (bSuccess != false) {
												/* Increment the special idle so we get a new one next time */
												this->field_0xff4 = 1;
												this->field_0xff8 = this->field_0xff8 + 1;
												if (this->field_0xff8 == 0xf0) {
													this->field_0xff8 = 0xe6;
												}
											}
										}
									}
								}
							}
						}
						else {
							this->SetState(0x83, 0xffffffff);
						}
					}
					else {
						this->SetState(0xe4, 0xffffffff);
					}
				}
				else {
					this->SetState(0xe8, 0xffffffff);
				}
			}
			else {
				IMPLEMENTATION_GUARD(
				/* Jumping */
				iVar9 = (int)this->pCollisionData;
				fVar13 = *(float*)&this->field_0x1050;
				if ((this->dynamic.flags & 2) != 0) {
					FUN_00211550(iVar9 + 0xd0);
					fVar13 = fVar14;
					FUN_00211590(iVar9 + 0xd0);
					fVar13 = edFIntervalLERP(*(float*)(iVar9 + 0xd4), fVar13, fVar14, 1.0, 0.3);
					fVar13 = *(float*)&this->field_0x1134 * fVar13;
				}
				/* Jumping */
				SetJumpCfg(0.1, fVar13, *(float*)&this->field_0x1158, this->field_0x1150, this->field_0x1154,
					(Actor*)this, 1, (edF32VECTOR4*)0x0);
				/* Jump animation */
				this->SetState(0x78, 0xffffffff);)
			}
		})
	}
	else {
		this->SetState(200, 0xffffffff);
	}
	return;
}

float CActorHeroPrivate::ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable)
{
	Timer* pTVar4;
	AnimResult* pAVar5;
	uint uVar6;
	int iVar7;
	float fVar8;
	float fVar9;
	CActorsTable local_110;
	int pCollision;
	bool bVar2;
	uint* puVar1;

	if (this->field_0xf54 == 0) {
		CActorAutonomous::ManageDyn(param_1, flags, pActorsTable);
	}
	else {
		local_110.entryCount = 0;
		bVar2 = false;
		if (pActorsTable == (CActorsTable*)0x0) {
			pActorsTable = &local_110;
		}
		CActorAutonomous::ManageDyn(param_1, flags, pActorsTable);
		iVar7 = 0;
		if (0 < pActorsTable->entryCount) {
			do {
				IMPLEMENTATION_GUARD()
				if (pActorsTable->aEntries[iVar7] == this->field_0xf54) {
					bVar2 = true;
					break;
				}
				iVar7 = iVar7 + 1;
			} while (iVar7 < pActorsTable->entryCount);
		}
		if (!bVar2) {
			IMPLEMENTATION_GUARD(
			puVar1 = *(uint**)(this->field_0xf54 + 0x2c);
			*puVar1 = *puVar1 & 0xffffdfff;
			iVar7 = (int)this->base.data.pCollisionData;
			*(uint*)iVar7 = *(uint*)iVar7 | 0x4000;
			this->field_0xf54 = 0;)
		}
	}
	pTVar4 = GetTimer();
	fVar8 = powf(0.9f, pTVar4->cutsceneDeltaTime * 50.0f);
	fVar9 = 1.0f;
	if ((fVar8 <= 1.0f) && (fVar9 = fVar8, fVar8 < 0.0f)) {
		fVar9 = 0.0f;
	}
	fVar8 = 1.0f - fVar9;
	this->field_0xa80 = fVar8 * this->dynamic.field_0x44 + fVar9 * this->field_0xa80;
	this->field_0xa84 = fVar8 * this->dynamic.field_0x40 + fVar9 * this->field_0xa84;
	fVar9 = fVar9 * this->field_0xa88 + fVar8 * this->dynamic.field_0x44 * this->dynamic.currentLocation.y;
	this->field_0xa88 = fVar9;

	iVar7 = this->actorState;
	if (iVar7 == -1) {
		uVar6 = 0;
	}
	else {
		pAVar5 = this->GetStateCfg(iVar7);
		uVar6 = pAVar5->flags_0x4 & 0x100;
	}

	if (uVar6 != 0) {
		fVar9 = this->currentLocation.y + 20.0f;
		this->animKey_0x157c = fVar9;
	}
	return fVar9;
}

void CActorHeroPrivate::StoreCollisionSphere()
{
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.x = 0.4f;
	local_10.y = 0.8f;
	local_10.z = 0.4f;
	local_10.w = 0.0f;
	local_20.x = 0.0f;
	local_20.z = 0.0f;
	local_20.y = 0.78f;
	local_20.w = 1.0f;
	ChangeCollisionSphere(0.0f, &local_10, &local_20);
	CActorAutonomous::StoreCollisionSphere();
	return;
}

void CBehaviourHeroDefault::Manage()
{
	this->pHero->BehaviourHero_Manage();
}

void CBehaviourHeroDefault::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorHeroPrivate* pCVar1;
	float fVar2;
	float in_f0;
	float fVar3;
	edF32VECTOR4 local_10;

	this->pHero = reinterpret_cast<CActorHeroPrivate*>(pOwner);
	pCVar1 = this->pHero;
	if ((pCVar1->flags & 0x1000) == 0) {
		pCVar1->rotationEuler.z = 0.0f;
		pCVar1->rotationEuler.x = 0.0f;

		local_10 = pCVar1->rotationQuat;
		local_10.y = 0.0f;
		edF32Vector4SafeNormalize1Hard(&local_10, &local_10);

		pCVar1->rotationQuat = local_10;
		in_f0 = local_10.w;
	}
	if (newState == -1) {
		this->pHero->StateEvaluate();
	}
	else {
		if (((newState - 0x78U < 2) || (newState - 0x7bU < 2)) || (newState == 0x11d)) {
			IMPLEMENTATION_GUARD(
			if (newState == 0x11d) {
				pCVar1 = this->pHero;
				fVar2 = *(float*)&(pCVar1->base).field_0xf54;
				fVar3 = (pCVar1->base).field_0x1154;
				if (fVar2 != 0.0) {
					in_f0 = 4.0;
					fVar3 = 4.0 - (*(float*)((int)&pCVar1->currentLocation + 4) -
						*(float*)((int)fVar2 + 0x34));
				}
				(**(GetAnimForState**)((int)((pCVar1->base).character.characterBase.base.base.pVTable + 1) + 0x54))
					((CActor*)pCVar1, (int)pOwner);
				ActorFloatFunc_00147a70
				(0.1, in_f0, *(float*)&(pCVar1->base).field_0x1158, (pCVar1->base).field_0x1150, fVar3, (Actor*)pCVar1, 1,
					(edF32VECTOR4*)0x0);
			}
			else {
				pCVar1 = this->pHero;
				(**(GetAnimForState**)((int)((pCVar1->base).character.characterBase.base.base.pVTable + 1) + 0x54))
					((CActor*)pCVar1, (int)pOwner);
				ActorFunc_001479b0(in_f0, pCVar1);
			})
		}
		this->pHero->SetState(newState, newAnimationType);
	}
	return;
}

void CBehaviourHeroDefault::End(int newBehaviourId)
{
	this->pHero->ResetBoomyDefaultSettings();
	this->pHero = (CActorHeroPrivate*)0x0;
	return;
}

void CBehaviourHeroDefault::InitState(int newState)
{
	this->pHero->BehaviourHero_InitState(newState);
	return;
}

void CBehaviourHeroDefault::TermState(int oldState, int newState)
{
	this->pHero->BehaviourHero_TermState(oldState, newState);
	return;
}
