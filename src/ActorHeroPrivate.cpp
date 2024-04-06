#include "ActorHeroPrivate.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "LevelScheduleManager.h"
#include "CameraViewManager.h"
#include "InputManager.h"
#include "ActorBoomy.h"

#include <string.h>
#include <math.h>
#include "CameraGame.h"

CActorHeroPrivate::CActorHeroPrivate()
{
	int* piVar1;
	undefined4* puVar2;
	
	CActor();
	//this->base.base.pVTable = &CActorMovable::_vt;
	this->dynamic.weightB = 0.0f;
	//this->base.base.pVTable = &CActorAutonomous::_vt;
	this->dynamicExt.gravityScale = 1.0f;
	this->dynamicExt.field_0x4 = 0.0f;
	this->dynamicExt.field_0x8 = 0.0f;
	this->lifeInterface = CLifeInterface();
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
	this->field_0xee4 = CLifeInterface();
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
	int piVar5;
	S_CHECKPOINT* pSVar6;
	int* piVar7;
	int iVar8;
	CCameraManager* pOther;
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
		CActorFighter::Create(pByteCode);

		static bool bInitedFirst = true;

		if (bInitedFirst) {
			char* pCurrent = pByteCode->currentSeekPos;

			while (true) {
				if (strncmp(pCurrent, "TSNI", 4) == 0) {
					break;
				}
				pCurrent++;
			}

			pByteCode->currentSeekPos = pCurrent;

			this->pActorBoomy = (CActorBoomy*)0x0;
			return;
		}

		bInitedFirst = true;
	}

	//CActorFighter::Create(pByteCode);

	//pByteCode->currentSeekPos += 0xdd78;
	pByteCode->currentSeekPos += 0x22E0;

	//this->field_0x1870 = 0;
	//this->field_0x1874 = this->field_0x444;
	//*(undefined4*)&this->field_0x1878 = *(undefined4*)&this->field_0x448;
	//this->field_0xff0 = 0;
	uVar2 = pByteCode->GetU32();
	//this->field_0x157c = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->animKey_0x1584 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->animKey_0x1588 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->animKey_0x158c = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->animKey_0x1590 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->animKey_0x1594 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->field_0x1594 = uVar2;
	uVar2 = pByteCode->GetU32();
	//this->field_0x1598 = uVar2;
	uVar2 = pByteCode->GetU32();
	//*(uint*)&this->field_0x159c = uVar2;
	/*pCVar3 = (CCamera*)*/pByteCode->GetU32();
	//this->pKyaWindWallCamera_0x15ac = pCVar3;
	int* pSeekpiVar5 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSeekpiVar5 + 1);
	if (*pSeekpiVar5 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *pSeekpiVar5 * 4;
	}
	//*(int**)&this->field_0x14a4 = pSeekpiVar5;
	//iVar11 = 0;
	//pCVar12 = this;
	//do {
	//	iVar10 = 0;
	//	do {
	//		iVar8 = (int)&(pCVar12->base).base.pVTable + iVar10;
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
	//	pCVar12 = (CActorHeroPrivate*)&(pCVar12->base).base.data.field_0xc;
	//} while (iVar11 < 0x10);
	iVar11 = pByteCode->GetS32();
	//this->field_0xd20 = iVar11;
	//this->pEventChunk24_0xd24 = (ed_zone_3d*)0x0;
	//iVar11 = this->field_0xd20;
	if (iVar11 != 0) {
		//piVar5 = (int*)operator.new.array((long)((int)peVar4 * 0x28 + 0x10));
		//piVar5 = __construct_new_array(piVar5, (ActorConstructorA*)&LAB_0034c690, FUN_0034b6b0, 0x28, (uint)peVar4);
		//*(int**)&this->field_0xd24 = piVar5;
		//piVar5 = *(int**)&this->field_0xd24;
		int count = iVar11;
		iVar11 = 0;
		if (0 < count) {
			do {
				iVar10 = pByteCode->GetS32();
				//*piVar5 = iVar10;
				uVar9 = pByteCode->GetU64();
				//*(ulong*)(piVar5 + 2) = uVar9;
				uVar2 = pByteCode->GetU32();
				//piVar5[4] = uVar2;
				uVar2 = pByteCode->GetU32();
				//piVar5[5] = uVar2;
				fVar13 = pByteCode->GetF32();
				//piVar5[6] = (int)fVar13;
				if (2.24f <= CScene::_pinstance->field_0x1c) {
					IMPLEMENTATION_GUARD();
				//	piVar7 = (int*)pByteCode->currentSeekPos;
				//	pByteCode->currentSeekPos = (char*)(piVar7 + 1);
				//	if (*piVar7 != 0) {
				//		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar7 * 0x1c;
				//	}
				//	piVar5[7] = (int)piVar7;
				//	pcVar1 = pByteCode->currentSeekPos;
				//	pByteCode->currentSeekPos = pcVar1 + 0x20;
				//	piVar5[8] = (int)pcVar1;
				}
				iVar11 = iVar11 + 1;
				piVar5 = piVar5 + 10;
			} while (iVar11 < count);
		}
	}

	int count = pByteCode->GetS32();
	//this->pCheckpointManagerSubObjA_0xe38 = pSVar6;
	//this->field_0xe38 = 0;
	//pSVar6 = this->pCheckpointManagerSubObjA_0xe38;
	if (count != 0x0) {
	//	piVar5 = (int*)operator.new.array((long)((int)pSVar6 * 0x1c + 0x10));
	//	piVar5 = __construct_new_array(piVar5, (ActorConstructorA*)&LAB_0034c680, FUN_0034b660, 0x1c, (uint)pSVar6);
	//	this->field_0xe38 = (uint)piVar5;
	//	piVar5 = (int*)this->field_0xe38;
		iVar11 = 0;
		if (0 < count) {
			do {
				iVar10 = pByteCode->GetS32();
				//*piVar5 = iVar10;
				iVar10 = pByteCode->GetS32();
				//piVar5[1] = iVar10;
				fVar13 = pByteCode->GetF32();
				//piVar5[2] = (int)fVar13;
				iVar10 = pByteCode->GetS32();
				//piVar5[3] = iVar10;
				fVar13 = pByteCode->GetF32();
				//piVar5[4] = (int)fVar13;
				iVar10 = pByteCode->GetS32();
				//piVar5[5] = iVar10;
				//if (piVar5[5] < 1) {
				//	piVar5[5] = 1;
				//}
				//if (5 < piVar5[5]) {
				//	piVar5[5] = 5;
				//}
				iVar10 = pByteCode->GetS32();
				//piVar5[6] = iVar10;
				iVar11 = iVar11 + 1;
				piVar5 = piVar5 + 7;
			} while (iVar11 < count);
		}
	}

	if (2.24f <= CScene::_pinstance->field_0x1c) {
		this->field_0xe3c = pByteCode->GetS32();
	}
	else {
		this->field_0xe3c = 0;
	}

	//*(undefined4*)&this->field_0xe40 = 0;
	piVar5 = this->field_0xe3c;
	if (piVar5 != 0x0) {
		//piVar7 = (int*)operator.new.array((long)((int)piVar5 * 8 + 0x10));
		//piVar5 = __construct_new_array(piVar7, (ActorConstructorA*)&LAB_0034c670, FUN_0034b610, 8, (uint)piVar5);
		//*(int**)&this->field_0xe40 = piVar5;
		//piVar5 = *(int**)&this->field_0xe40;
		iVar11 = 0;
		if (0 < this->field_0xe3c) {
			do {
				iVar10 = pByteCode->GetS32();
				//*piVar5 = iVar10;
				iVar10 = pByteCode->GetS32();
				//piVar5[1] = iVar10;
				iVar11 = iVar11 + 1;
				piVar5 = piVar5 + 2;
			} while (iVar11 < this->field_0xe3c);
		}
	}
	if (2.24f <= CScene::_pinstance->field_0x1c) {
		pSeekpiVar5 = (int*)pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = (char*)(pSeekpiVar5 + 1);
		if (*pSeekpiVar5 != 0) {
			pByteCode->currentSeekPos = pByteCode->currentSeekPos + *pSeekpiVar5 * 4;
		}
		//this->field_0xe48 = piVar5;
	}

	iVar11 = pByteCode->GetS32();
	//*(int*)&this->field_0x12e0 = iVar11;
	//iVar11 = *(int*)&this->field_0x12e0;
	//if (iVar11 != -1) {
	//	CActor::SV_InstallMaterialId(iVar11);
	//	CFxTail::Create(1.0, (CFxTail*)&this->fxTrailA.field_0x4, 0x10, 4, *(int*)&this->field_0x12e0);
	//	CFxTail::Create(1.0, (CFxTail*)&this->fxTrailB.field_0x4, 0x10, 4, *(int*)&this->field_0x12e0);
	//}
	iVar11 = pByteCode->GetS32();
	//*(int*)&this->field_0x13c8 = iVar11;
	fVar13 = pByteCode->GetF32();
	//this->field_0x1990 = fVar13;
	//this->bCanUseCheats = 0;
	//this->pCameraViewBase_0xaa8 = (CCamera*)0x0;
	//iVar11 = LevelScheduleManager::ScenVar_Get(0x11);
	//iVar10 = LevelScheduleManager::ScenVar_Get(0x12);
	//iVar8 = LevelScheduleManager::ScenVar_Get(0x14);
	this->lifeInterface.SetValueMax((float)iVar8);
	//SetParam2_0x18ToParam1((float)(iVar11 * iVar10), &this->field_0xab8);
	//SetFloatParam1_0x1cToParam2(0.0, &this->field_0xab8);
	//(**(code**)(*(int*)&this->field_0xab8 + 0x20))((float)_gGameNfo.heroMagic, &this->field_0xab8);
	iVar11 = pByteCode->GetS32();
	if (iVar11 != 0) {
		pOther = (CCameraManager*)CScene::GetManager(MO_Camera);
		iVar10 = 0;
		if (0 < iVar11 + -2) {
			do {
				/* Kim.act cam */
				pOther->AddCamera(CT_MainCamera, pByteCode, "Kim.act cam");
				iVar10 = iVar10 + 1;
			} while (iVar10 < iVar11 + -2);
		}

		/* Kim.act WindWall */
		pCVar3 = pOther->AddCamera(CT_KyaWindWall, pByteCode, "Kim.act WindWall");
		//this->pJamgutCamera_0x15b8 = pCVar3;
		/* Kim.act Jamgut Cossack */
		pCVar3 = pOther->AddCamera(CT_KyaJamgut, pByteCode, "Kim.act Jamgut Cossack");
		//this->pIntViewCamera_0x15bc = pCVar3;
	}

	this->pActorBoomy = (CActorBoomy*)0x0;
	iVar11 = pByteCode->GetS32();
	//this->field_0x15dc.field_0x0 = iVar11;
	iVar11 = pByteCode->GetS32();

	// Hack p2
	char* pCurrent = pByteCode->currentSeekPos;

	while (true) {
		if (strncmp(pCurrent, "TSNI", 4) == 0) {
			break;
		}
		pCurrent++;
	}

	pByteCode->currentSeekPos = pCurrent;

	//uVar2 = 0;
	//pCVar12 = this;
	//do {
	//	FUN_00407690((int*)&(pCVar12->base).field_0x1880, iVar11);
	//	uVar2 = uVar2 + 1;
	//	pCVar12 = (CActorHeroPrivate*)&(pCVar12->base).base.data.actorFieldS;
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
	CCameraManager* pCameraManager;
	CCamera* pCVar5;
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
	undefined4 local_10;
	uint local_c;
	uint local_8;
	uint local_4;
	
	this->field_0xcbc = 0;
	//*(undefined4*)&this->field_0x1a48 = 0;
	this->field_0x1a4c = 0;
	this->field_0x1a50 = 0;
	//*(undefined4*)&this->field_0xcc0 = 0;
	this->field_0xee0 = 1;
	CActorFighter::Init();
	this->field_0xf54 = 0;
	ResetStdDefaultSettings();
	ResetSlideDefaultSettings();
	ResetBoomyDefaultSettings();
	ResetGripClimbDefaultSettings();
	ResetWindDefaultSettings();
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
	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	pCVar5 = pCameraManager->GetDefGameCamera(CT_Main);
	this->pMainCamera = pCVar5;
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
	pCameraManager->SetMainCamera(this->pMainCamera);
	//pCVar5 = this->pMainCamera;
	//pCVar5[1].transformationMatrix.ca = (float)((uint)pCVar5[1].transformationMatrix.ca | 0x20000);
	//pCVar5 = this->pCameraViewBase_0xab0;
	//*(float*)&this->field_0x15c4 = pCVar5->fov;
	//fVar14 = (*((pCVar5->objBase).pVTable)->GetDistance)();
	//*(float*)&this->field_0x15c8 = fVar14;
	//*(float*)&this->field_0x15cc = (this->pMainCamera)->fov;
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
	local_4 = LevelScheduleManager::ScenVar_Get(9);
	DoMessage(this, (ACTOR_MESSAGE)0x62, (MSG_PARAM)local_4);
	if (this->pActorBoomy != (CActorBoomy*)0x0) {
		local_8 = LevelScheduleManager::ScenVar_Get(9);
		DoMessage(this->pActorBoomy, (ACTOR_MESSAGE)0x62, (MSG_PARAM)local_8);
	}
	local_c = LevelScheduleManager::ScenVar_Get(10);
	DoMessage(this, (ACTOR_MESSAGE)0x6b, (MSG_PARAM)local_c);
	local_10 = 0;
	DoMessage(this, (ACTOR_MESSAGE)0x79, (MSG_PARAM)0);
	this->field_0xcbc = 0;
	//pFVar4 = CScene::ptable.g_FrontendManager_00451680;
	//uVar7 = (**(code**)((int)(this->pVTable + 1) + 0x44))(this);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 0, uVar7);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 2, &this->field_0xab8);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 3, &this->field_0xad8);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 6, &this->field_0xadc);
	//(*(code*)pFVar4->pManagerFunctionData[1].field_0x0)(pFVar4, 7, &this->field_0xc84);
	return;
}

void CActorHeroPrivate::Manage()
{
	CActor::Manage();
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
		animType = piVar3->animId + 0xa;
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
		GetBehaviour(8);
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
		pCVar1 = GetBehaviour((int)this->aComponents);
		(*(code*)pCVar1->pVTable[1].Init)(pCVar1, pOtherActor, *(undefined4*)&this->field_0x15a0, 7, 0x11d);
		(*((this->pVTable)->actorBase).SetState)((CActor*)this, 0x11e, -1);)
	}
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x19b1 = 1;
	this->field_0xffc = 0.0f;)
	return;
}

int CActorHeroPrivate::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	ed_3d_hierarchy_node* peVar1;
	edColPRIM_OBJECT* peVar2;
	CAnimation* pAnimationController;
	CActor* pReceiver;
	undefined* puVar3;
	float* pfVar4;
	undefined4* puVar5;
	undefined8 uVar6;
	LevelScheduleManager* pLVar7;
	//FrontendManager* pFVar8;
	bool bVar9;
	uint uVar10;
	CLifeInterface* pCVar11;
	AnimResult* pAVar12;
	int iVar13;
	int iVar14;
	Timer* pTVar15;
	CBehaviour* pCVar16;
	int* piVar17;
	CPlayerInput* pCVar18;
	int pAVar19;
	undefined4 uVar21;
	int uVar20;
	long lVar22;
	edAnmLayer* peVar23;
	edF32VECTOR4* peVar24;
	float fVar25;
	float fVar26;
	undefined4 uVar27;
	float fVar28;
	edF32VECTOR4 aeStack256[2];
	edF32VECTOR4 eStack224;
	//CCollisionRay CStack208;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	undefined4 local_8;
	undefined4 local_4;

	pLVar7 = LevelScheduleManager::gThis;
	if (msg == 0x8c) {
		IMPLEMENTATION_GUARD(
		iVar13 = LevelScheduleManager::ScenVar_Get(0x14);
		(*(code*)(this->lifeInterface.pVtable)->SetValue)
			((float)iVar13, &this->character.characterBase.lifeInterface);
		fVar25 = (float)(*(code*)(this->lifeInterface.pVtable)->GetValue)
			(&this->lifeInterface);
		fVar26 = (float)(**(code**)(*(int*)&(this->base).field_0xab8 + 0x24))(&(this->base).field_0xab8);
		fVar28 = (float)(**(code**)(*(int*)&(this->base).field_0xad8 + 0x24))(&(this->base).field_0xad8);
		LevelScheduleManager::FUN_002db590(fVar25, pLVar7, (int)fVar26, (int)fVar28);
		puVar3 = (CScene::ptable.g_FrontendManager_00451680)->pHealthBar;
		uVar21 = (*(this->pVTable)->GetLifeInterfaceOther)((CActor*)this);
		fVar25 = (float)(**(code**)(*(int*)uVar21 + 0x24))(uVar21);
		pCVar11 = (*(this->pVTable)->GetLifeInterfaceOther)((CActor*)this);
		fVar26 = CLifeInterface::GetValueMax(pCVar11);
		FUN_001daa60(fVar25 / fVar26, (int)puVar3);)
		return 1;
	}
	if (msg == 0x88) {
		IMPLEMENTATION_GUARD(
		FUN_001dc5c0(&(this->base).field_0xadc);)
		return 1;
	}
	if (msg == 0x84) {
		IMPLEMENTATION_GUARD(
		pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
		fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
		bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
		if (!bVar9) {
			iVar13 = this->actorState;
			uVar10 = 0;
			if (iVar13 != -1) {
				pAVar12 = GetStateCfg(iVar13);
				uVar10 = pAVar12->flags_0x4;
			}
			bVar9 = (uVar10 & 1) != 0;
		}
		if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
			ActorTimeFunc_00325c40((float)(int)pMsgParam, (Actor*)this, 1);
			return 1;
		})
		return 0;
	}
	if (msg == 0x81) {
		IMPLEMENTATION_GUARD(
		*(undefined4*)&(this->base).field_0x142c = 1;)
		return 1;
	}
	if (msg == 0x80) {
		IMPLEMENTATION_GUARD(
		*(undefined4*)&(this->base).field_0x142c = 0;)
		return 1;
	}
	if (msg == 0x8d) {
		IMPLEMENTATION_GUARD(
		pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
		fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)();
		bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
		if (!bVar9) {
			iVar13 = this->actorState;
			uVar10 = 0;
			if (iVar13 != -1) {
				pAVar12 = GetStateCfg(iVar13);
				uVar10 = pAVar12->flags_0x4;
			}
			bVar9 = (uVar10 & 1) != 0;
		}
		if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
			if (*(int*)&(this->base).field_0xd28 == 0) {
				if (LevelScheduleManager::gThis->currentLevelID == 0xd) {
					if (pMsgParam == (void*)0xb5f) {
						LevelScheduleManager::ScenVar_Set(9, 0);
					}
					else {
						if (pMsgParam == (void*)0x452) {
							LevelScheduleManager::ScenVar_Set(9, 0);
							LevelScheduleManager::ScenVar_Set(10, 0x3f);
						}
						else {
							if (pMsgParam == (void*)0x8a0) {
								LevelScheduleManager::ScenVar_Set(9, 1);
								LevelScheduleManager::ScenVar_Set(10, 0);
							}
						}
					}
				}
			}
			else {
				*(undefined4*)&(this->base).field_0xd30 = 2;
			}
			return 1;
		})
		return 0;
	}
	if (msg == 0x7e) {
		IMPLEMENTATION_GUARD(
		pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
		fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
		bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
		if (!bVar9) {
			iVar13 = this->actorState;
			uVar10 = 0;
			if (iVar13 != -1) {
				pAVar12 = GetStateCfg(iVar13);
				uVar10 = pAVar12->flags_0x4;
			}
			bVar9 = (uVar10 & 1) != 0;
		}
		if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
			iVar13 = *(int*)&(this->base).field_0xd28;
			if (iVar13 != 0) {
				iVar14 = LevelScheduleManager::gThis->currentLevelID;
				*(undefined4*)&(this->base).field_0xd30 = 1;
				*(undefined*)
					(&(this->base).field_0xd34 + (iVar13 - (int)(this->base).pEventChunk24_0xd24) / 0x28 + iVar14 * 0x10) = 3;
			}
			return 1;
		})
		return 0;
	}
	if (msg == 0x7d) {
		IMPLEMENTATION_GUARD(
		pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
		fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
		bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
		if (!bVar9) {
			iVar13 = this->actorState;
			uVar10 = 0;
			if (iVar13 != -1) {
				pAVar12 = GetStateCfg(iVar13);
				uVar10 = pAVar12->flags_0x4;
			}
			bVar9 = (uVar10 & 1) != 0;
		}
		if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
			GameFlags = GameFlags | 0x400;
			LevelScheduleManager::SetLevelTimerFunc_002df450(1.0, LevelScheduleManager::gThis, 0);
			return 1;
		})
		return 0;
	}
	if (msg == 0x4d) {
		uint boneType = reinterpret_cast<uint>(pMsgParam);
		switch (boneType) {
		case 0x5:
			return 0x45544554;
		default:
			return 1;
		case 0x7:
			IMPLEMENTATION_GUARD(
			return this->animKey_0x1588;)
		case 0xa:
		case 0xb:
			IMPLEMENTATION_GUARD(
			return this->field_0x157c;)
		case 0xc:
			IMPLEMENTATION_GUARD(
			return this->animKey_0x1584;)
		case 0xd:
			IMPLEMENTATION_GUARD(
			return this->animKey_0x158c;)
		}
	}
	if (msg == 0x5e) {
		IMPLEMENTATION_GUARD(
		pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
		fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
		bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
		if (!bVar9) {
			iVar13 = this->actorState;
			uVar10 = 0;
			if (iVar13 != -1) {
				pAVar12 = GetStateCfg(iVar13);
				uVar10 = pAVar12->flags_0x4;
			}
			bVar9 = (uVar10 & 1) != 0;
		}
		if ((bVar9) || (0.0 < (this->base).field_0x1558)) {
			return 0;
		}
		/* WARNING: Load size is inaccurate */
		if (*pMsgParam == 2) {
			edF32Vector4AddHard((edF32VECTOR4*)((int)pMsgParam + 0x10),
				&this->currentLocation,
				&this->rotationQuat);
			*(float*)((int)pMsgParam + 0x14) = *(float*)((int)pMsgParam + 0x14) + 0.15;
			uVar6 = *(undefined8*)&this->currentLocation;
			local_b0.z = this->currentLocation.z;
			local_b0.w = this->currentLocation.w;
			local_b0.x = (float)uVar6;
			fVar25 = (float)((ulong)uVar6 >> 0x20) + 0.7;
			local_b0.y = fVar25;
			edF32Vector4SubHard(&eStack160, (edF32VECTOR4*)((int)pMsgParam + 0x10), &local_b0);
			fVar25 = edF32Vector4NormalizeHard(&eStack160, &eStack160);
			CCollisionRay::CCollisionRay(fVar25, &CStack208, &local_b0, &eStack160);
			fVar25 = CCollisionRay::Intersect
			(&CStack208, 3, (CActor*)this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			if (fVar25 != 1e+30f) {
				fVar25 = fVar25 - 0.2;
				if (fVar25 < 0.0) {
					fVar25 = 0.0;
				}
				edF32Vector4ScaleHard(fVar25, &eStack160, &eStack160);
				edF32Vector4AddHard((edF32VECTOR4*)((int)pMsgParam + 0x10), &local_b0, &eStack160);
			}
			return 1;
		})
	}
	else {
		if (msg == 0x58) {
			IMPLEMENTATION_GUARD(
			this->field_0x2e4 = 0.0;
			(this->base).field_0xcbc = 0;
			*(undefined4*)&(this->base).field_0x1a48 = 0;
			(this->base).field_0x1a4c = 0;
			(this->base).field_0x1a50 = 0;
			pFVar8 = CScene::ptable.g_FrontendManager_00451680;
			pCVar11 = (*(this->pVTable)->GetLifeInterfaceOther)((CActor*)this);
			(*(code*)pFVar8->pManagerFunctionData[1].field_0x0)(pFVar8, 0, pCVar11);
			FUN_001d9df0((int)(CScene::ptable.g_FrontendManager_00451680)->pHealthBar, 0);
			FUN_001b9400((int)CScene::_pinstance, 1);)
			goto LAB_00344ed0;
		}
		if (msg == 0x5b) {
			IMPLEMENTATION_GUARD(
			/* WARNING: Load size is inaccurate */
			pfVar4 = *pMsgParam;
			fVar25 = pfVar4[1];
			fVar26 = pfVar4[2];
			(this->base).field_0xea4.valueMax = *pfVar4;
			(this->base).field_0xea4.field_0x10 = fVar25;
			*(float*)&(this->base).field_0xeb8 = fVar26;
			*(undefined4*)&(this->base).field_0xebc = 0x3f800000;
			puVar5 = *(undefined4**)((int)pMsgParam + 4);
			uVar21 = puVar5[1];
			uVar27 = puVar5[2];
			*(undefined4*)&(this->base).field_0xec0 = *puVar5;
			*(undefined4*)&(this->base).field_0xec4 = uVar21;
			*(undefined4*)&(this->base).field_0xec8 = uVar27;
			*(undefined4*)&(this->base).field_0xecc = 0;
			*(undefined4*)&(this->base).field_0xed0 = *(undefined4*)((int)pMsgParam + 8);
			(*(code*)(this->pVTable)->LifeRestore)();
			FUN_001b9400((int)CScene::_pinstance, 1);)
			goto LAB_00344ed0;
		}
		if (msg == 0x57) {
			IMPLEMENTATION_GUARD(
			(this->base).field_0xcbc = (int)pSender;
			if (((uint)pMsgParam & 1) == 0) {
				*(undefined4*)&(this->base).field_0x1a48 = 0;
			}
			else {
				*(undefined4*)&(this->base).field_0x1a48 = 1;
			}
			if (((uint)pMsgParam & 4) == 0) {
				(this->base).field_0x1a4c = 0;
			}
			else {
				(this->base).field_0x1a4c = 1;
			}
			if (((uint)pMsgParam & 2) == 0) {
				(this->base).field_0x1a50 = 0;
			}
			else {
				(this->base).field_0x1a50 = 1;
			}
			(*(code*)(CScene::ptable.g_FrontendManager_00451680)->pManagerFunctionData[1].field_0x0)
				(CScene::ptable.g_FrontendManager_00451680, 0, &(this->base).field_0xee4);
			iVar13 = LevelScheduleManager::ScenVar_Get(0x14);
			CLifeInterface::SetValueMax((float)iVar13, &(this->base).field_0xee4);
			(*(code*)(this->pVTable)->LifeRestore)(this);
			pFVar8 = CScene::ptable.g_FrontendManager_00451680;
			FUN_001d9df0((int)(CScene::ptable.g_FrontendManager_00451680)->pHealthBar, 1);
			puVar3 = pFVar8->pHealthBar;
			uVar21 = (*(this->pVTable)->GetLifeInterfaceOther)((CActor*)this);
			fVar25 = (float)(**(code**)(*(int*)uVar21 + 0x24))(uVar21);
			pCVar11 = (*(this->pVTable)->GetLifeInterfaceOther)((CActor*)this);
			fVar26 = CLifeInterface::GetValueMax(pCVar11);
			FUN_001daa60(fVar25 / fVar26, (int)puVar3);
			FUN_001daff0((long)(int)pFVar8->pHealthBar);
			FUN_001da810(pFVar8->pHealthBar);)
			goto LAB_00344ed0;
		}
		if (msg == 0x50) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
				(this->base).field_0x1554 = 0.0;
				return 1;
			})
			return 0;
		}
		if (msg == 0x4f) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
				if ((this->base).field_0x1554 == 0.0) {
					(this->base).field_0x1554 = 1.401298e-45;
					*(undefined4*)&(this->base).field_0x1550 = 0;
				}
				return 1;
			})
			return 0;
		}
		if (msg == 0x61) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if (((!bVar9) && (bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar9 == false)) &&
				((this->base).field_0x1558 <= 0.0)) {
				(this->base).field_0xf20 = pSender;
				(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x119, 0xffffffff);
				return 1;
			})
			return 0;
		}
		if (msg == 0x40) {
			IMPLEMENTATION_GUARD(
			iVar13 = this->actorState;
			if ((iVar13 == 0x117) || (iVar13 == 0x118)) {
				if (*(int*)&(this->base).field_0xd30 == 1) {
					*(undefined4*)&(this->base).field_0xd30 = 2;
				}
				uVar21 = ChooseStateFall((CActorHero*)this, 0);
				SetState(uVar21, 0xffffffff);
				return 1;
			})
			goto LAB_00344ed0;
		}
		if (msg == 0x3f) {
			IMPLEMENTATION_GUARD(
				pCVar11 = GetLifeInterface();
			fVar25 = pCVar11->GetValue();
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0f;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}

				bVar9 = (uVar10 & 1) != 0;
			}

			if (((!bVar9) && (bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar9 == false)) &&
				((this->base).field_0x1558 <= 0.0)) {
				if (this->actorState == 0x117) {
					if ((this->base).field_0xf20 != pSender) {
						return 0;
					}
					(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x118, 0xffffffff);
				}
				else {
					(this->base).field_0xf20 = pSender;
					(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x117, 0xffffffff);
				}
				return 1;
			})
			return 0;
		}
		if (msg == 0x3c) {
			IMPLEMENTATION_GUARD(
			if (this != (CActorHeroPrivate*)CActorHero::_gThis) {
				uVar10 = this->flags;
				CActorFighter::InterpretMessage((CActorFighter*)this, pSender, 0x3c, pMsgParam);
				if ((uVar10 & 2) != 0) {
					this->flags =
						this->flags | 2;
					this->flags =
						this->flags & 0xfffffffe;
					this->flags =
						this->flags | 0x80;
					this->flags =
						this->flags & 0xffffffdf;
					CActor::EvaluateDisplayState((CActor*)this);
					this->flags =
						this->flags | 0x800;
				}
				return 1;
			})
			goto LAB_00344ed0;
		}
		if (msg == 0x83) {
			pCVar11 = GetLifeInterface();
			fVar25 = pCVar11->GetValue();
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0f;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}

				bVar9 = (uVar10 & 1) != 0;
			}

			if (((!bVar9) && (bVar9 = CActorHero::TestState_IsInCheatMode(), bVar9 == false)) &&
				(this->field_0x1558 <= 0.0f)) {
				this->field_0x18dc = 1;

				if (this == (CActorHeroPrivate*)CActorHero::_gThis) {
					pCVar18 = GetPlayerInput(0);
					this->field_0x1610 = 0;

					if (pCVar18 == (CPlayerInput*)0x0) {
						this->field_0x18dc = 1;
					}
					else {
						this->pPlayerInput = pCVar18;
						this->field_0x18dc = 0;
					}
				}
				else {
					pCVar18 = GetPlayerInput(1);
					this->field_0x1610 = 0;
					if (pCVar18 == (CPlayerInput*)0x0) {
						this->field_0x18dc = 1;
					}
					else {
						this->pPlayerInput = pCVar18;
						this->field_0x18dc = 0;
					}
				}

				if (this->field_0xf24 != (char*)0x0) {
					IMPLEMENTATION_GUARD(
					CAnimation::UnRegisterBone(this->pAnimationController, 0x45544554);
					CAnimation::UnRegisterBone
					(this->pAnimationController, (int)&DAT_00554f43);
					(*(code*)(this->pVTable)->SetLookingAtOff)(this);
					this->field_0xf24 = (char*)0x0;
					pAVar19 = (AnimationType*)
						(*(this->pVTable)->GetStateCfg)
						((CActor*)this, this->actorState);
					CActor::PlayAnim((CActor*)this, *pAVar19);
					piVar17 = (int*)(*(this->pVTable)->GetStateCfg)
						((CActor*)this, this->actorState);
					this->prevAnimType = *piVar17;)
				}
				return 1;
			}
			return 0;
		}
		if (msg == 0x82) {
			pCVar11 = GetLifeInterface();
			fVar25 = pCVar11->GetValue();
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0f;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}

				bVar9 = (uVar10 & 1) != 0;
			}

			if (((!bVar9) && (bVar9 = TestState_IsInCheatMode(), bVar9 == false)) &&
				(this->field_0x1558 <= 0.0f)) {
				CPlayerInput* pInput = GetInputManager(1, 0);
				if (pInput != (CPlayerInput*)0x0) {
					pInput->FUN_001b6e20(0.0f, 0.0f);
				}
				this->field_0x1a10 = 0;
				IMPLEMENTATION_GUARD_LOG(
				*(undefined4*)&this->field_0x1a14 = 0;
				*(undefined4*)&this->field_0x1a18 = 0;
				*(undefined4*)&this->field_0x1a20 = 0;
				*(undefined4*)&this->field_0x1a24 = 0;
				*(undefined4*)&this->field_0x1a28 = 0;
				*(undefined4*)&this->field_0x1a2c = 0;
				*(undefined4*)&this->field_0x1a40 = 0;
				*(undefined4*)&this->field_0x1a44 = 0;)
				this->field_0x1610 = 0;
				this->field_0x18dc = 1;
				return 1;
			}
			return 0;
		}
		if (msg == 0x26) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->character.characterBase.field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if (((!bVar9) && (bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar9 == false)) &&
				(this->field_0x1558 <= 0.0)) {
				this->field_0x18dc = 1;
				if (this == (CActorHeroPrivate*)CActorHero::_gThis) {
					pCVar18 = GetPlayerInput(0);
					(this->base).field_0x1610 = 0;
					if (pCVar18 == (CPlayerInput*)0x0) {
						(this->base).field_0x18dc = 1;
					}
					else {
						(this->base).pPlayerInput = pCVar18;
						(this->base).field_0x18dc = 0;
					}
				}
				else {
					pCVar18 = GetPlayerInput(1);
					(this->base).field_0x1610 = 0;
					if (pCVar18 == (CPlayerInput*)0x0) {
						(this->base).field_0x18dc = 1;
					}
					else {
						(this->base).pPlayerInput = pCVar18;
						(this->base).field_0x18dc = 0;
					}
				}
				return 1;
			})
			return 0;
		}
		if (msg == 0x25) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if (((!bVar9) && (bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar9 == false)) &&
				((this->base).field_0x1558 <= 0.0)) {
				lVar22 = (*(code*)(this->pVTable)->GetInputManager)(this, 1, 0);
				if (lVar22 != 0) {
					InputManagerFloatFunc_001b6e20(0.0, 0.0, (CPlayerInput*)lVar22);
				}
				(this->base).field_0x1a10 = 0;
				*(undefined4*)&(this->base).field_0x1a14 = 0;
				*(undefined4*)&(this->base).field_0x1a18 = 0;
				*(undefined4*)&(this->base).field_0x1a20 = 0;
				*(undefined4*)&(this->base).field_0x1a24 = 0;
				*(undefined4*)&(this->base).field_0x1a28 = 0;
				*(undefined4*)&(this->base).field_0x1a2c = 0;
				*(undefined4*)&(this->base).field_0x1a40 = 0;
				*(undefined4*)&(this->base).field_0x1a44 = 0;
				(this->base).field_0x1610 = 1;
				(this->base).field_0x18dc = 1;
				return 1;
			})
			return 0;
		}
		if (msg == 0x2c) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
				(*(this->pVTable)->SetBehaviour)
					((CActor*)this, 0xffffffff, 0xffffffff, 0xffffffff);
				(*(this->pVTable)->SetBehaviour)
					((CActor*)this, (this->subObjA)->defaultBehaviourId, -1, -1);
				this->rotationQuat.x = *(float*)((int)pMsgParam + 0x10);
				this->rotationQuat.y = (float)*(undefined4*)((int)pMsgParam + 0x14);
				this->rotationQuat.z = (float)*(undefined4*)((int)pMsgParam + 0x18);
				this->rotationQuat.w = (float)*(undefined4*)((int)pMsgParam + 0x1c);
				GetAnglesFromVector(&this->rotationEuler,
					&this->rotationQuat);
				CActor::UpdatePosition((CActor*)this, (edF32VECTOR4*)pMsgParam, true);
				return 1;
			})
			return 0;
		}
		if (msg == 0x1e) {
			IMPLEMENTATION_GUARD(
			iVar13 = this->actorState;
			if ((iVar13 == 0x10b) || (iVar13 == 0x10d)) {
				edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x10), &eStack144, (edF32VECTOR4*)pMsgParam);
				pTVar15 = GetTimer();
				edF32Vector4ScaleHard(0.02 / pTVar15->cutsceneDeltaTime, aeStack256, &eStack144);
				peVar24 = this->dynamicExt.aVelocity;
				edF32Vector4AddHard(peVar24, peVar24, aeStack256);
				fVar25 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
				this->dynamicExt.aVelocityMagnitudes[0] = fVar25;
				SetJumpCfg(0.1, (this->base).runSpeed, (this->base).field_0x1158, 0.0, 0.0, 0, (edF32VECTOR4*)0x0);
				if (this->actorState == 0x10d) {
					SetState(0x10e, -1);
				}
				else {
					SetState(0x10c, -1);
				}
				local_8 = 0;
				CActor::DoMessage((CActor*)this, *(CActor**)&(this->base).field_0xf18, 0x1f, 0);
				return 1;
			})
			goto LAB_00344ed0;
		}
		if (msg == 0x1d) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if ((bVar9) || (0.0 < (this->base).field_0x1558)) {
				return 0;
			}
			uVar10 = FUN_00132a60((int*)this, 0xffffffff);
			if (uVar10 == 0) {
				return 0;
			}
			if ((0.1 < (this->base).dynamic.field_0x44 *
				(this->base).dynamic.currentLocation.y) &&
				((iVar13 = this->actorState, iVar13 == 0x79 || (iVar13 == 0x7c)))) {
				return 0;
			}
			(this->base).field_0x15a4 = (uint)pMsgParam;
			pReceiver = *(CActor**)&(this->base).field_0xf18;
			if ((pReceiver != (CActor*)0x0) && (pReceiver != pSender)) {
				local_4 = 1;
				CActor::DoMessage((CActor*)this, pReceiver, 0x1f, 1);
			}
			*(CActor**)&(this->base).field_0xf18 = pSender;
			uVar10 = CActorHero::TestState_IsFlying((CActorHero*)this, 0xffffffff);
			if (uVar10 == 0) {
				(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x10b, 0xffffffff);
			}
			else {
				(this->base).field_0x11f8 = 0;
				(this->base).field_0x11f4 = 0.0;
				(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x10d, 0xffffffff);
			})
			return 1;
		}
		if (msg == 0x16) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if ((((!bVar9) || (uVar10 = CActorHero::TestState_IsInTheWind((Actor*)this, 0xffffffff), uVar10 != 0)) &&
				((this->base).field_0x1558 <= 0.0)) &&
				((this->flags & 0x800000) == 0)) {
				bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this);
				if (((bVar9 == false) && (iVar13 = this->actorState, iVar13 != 0x117))
					&& ((iVar13 != 0x118 && ((this->flags & 0x400000) == 0)))) {
					edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x10), &eStack128, (edF32VECTOR4*)pMsgParam);
					peVar24 = this->dynamicExt.aVelocity + 2;
					edF32Vector4AddHard(peVar24, peVar24, &eStack128);
					fVar25 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity + 2);
					this->dynamicExt.aVelocityMagnitudes[2] = fVar25;
					lVar22 = (*(code*)(this->pVTable)->field_0x100)(this);
					if (lVar22 == 0) {
						fVar25 = 0.0;
					}
					else {
						iVar13 = (*(code*)(this->pVTable)->field_0x100)(this);
						fVar25 = *(float*)(iVar13 + 0x38);
					}
					if (fVar25 < 0.001) {
						fVar25 = *(float*)((int)pMsgParam + 0x10);
						lVar22 = (*(code*)(this->pVTable)->field_0x100)(this);
						if (lVar22 == 0) {
							bVar9 = false;
						}
						else {
							piVar17 = (int*)(*(code*)(this->pVTable)->field_0x100)(this);
							iVar13 = *piVar17;
							iVar14 = (*(code*)(this->pVTable)->field_0x100)(this);
							if (iVar13 == *(int*)(iVar14 + 4)) {
								bVar9 = true;
							}
							else {
								iVar13 = (*(code*)(this->pVTable)->field_0x100)(this);
								if (*(int*)(iVar13 + 4) == 0) {
									bVar9 = false;
								}
								else {
									bVar9 = true;
									if (edFCosinus[1820] <= ABS(*(float*)((int)pMsgParam + 4))) {
										bVar9 = false;
									}
								}
							}
						}
						if ((!bVar9) || (bVar9 = true, fVar25 <= 2.0)) {
							bVar9 = false;
						}
						if (!bVar9) {
							(this->base).dynamic.speed =
								(this->base).dynamic.speed * 0.4;
						}
					}
					return 1;
				}
				return 1;
			})
			return 1;
		}
		if (msg == 0xc) {
			IMPLEMENTATION_GUARD(
			*(undefined4*)pMsgParam = 0x40accccd;)
			return 1;
		}
		if (msg == 10) {
			IMPLEMENTATION_GUARD(
			pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
			if (!bVar9) {
				iVar13 = this->actorState;
				uVar10 = 0;
				if (iVar13 != -1) {
					pAVar12 = GetStateCfg(iVar13);
					uVar10 = pAVar12->flags_0x4;
				}
				bVar9 = (uVar10 & 1) != 0;
			}
			if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
				pTVar15 = Timer::GetTimer();
				(this->base).field_0x155c = (float)(int)pMsgParam + pTVar15->scaledTotalTime;
				return 1;
			})
			return 0;
		}
		if (msg == 0x85) {
			IMPLEMENTATION_GUARD(
			iVar14 = LevelScheduleManager::ScenVar_Get(0x14);
			iVar13 = LevelScheduleManager::ScenVar_Get(0x15);
			iVar14 = iVar14 + iVar13;
			iVar13 = LevelScheduleManager::ScenVar_Get(0x16);
			if (iVar13 < iVar14) {
				iVar14 = LevelScheduleManager::ScenVar_Get(0x16);
			}
			pCVar11 = (*((CActorHero::_gThis->character).characterBase.base.base.pVTable)->GetLifeInterfaceOther)
				((CActor*)CActorHero::_gThis);
			LevelScheduleManager::ScenVar_Set(0x14, iVar14);
			CLifeInterface::SetValueMax((float)iVar14, pCVar11);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			iVar13 = LevelScheduleManager::ScenVar_Get(0x15);
			(*(code*)pCVar11->pVtable->SetValue)(fVar25 + (float)iVar13, pCVar11);
			(*(code*)pCVar11->pVtable->Activate)(pCVar11, 1);)
			goto LAB_00344ed0;
		}
		if (msg != 9) {
			if (msg == 8) {
				IMPLEMENTATION_GUARD(
				LevelScheduleManager::FUN_002dadd0(LevelScheduleManager::gThis, (int)pMsgParam);)
			}
			else {
				if (msg == 0x8b) {
					IMPLEMENTATION_GUARD(
					pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
					fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
					bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
					if (!bVar9) {
						iVar13 = this->actorState;
						uVar10 = 0;
						if (iVar13 != -1) {
							pAVar12 = GetStateCfg(iVar13);
							uVar10 = pAVar12->flags_0x4;
						}
						bVar9 = (uVar10 & 1) != 0;
					}
					if (((!bVar9) && (bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar9 == false)) &&
						((this->base).field_0x1558 <= 0.0)) {
						if (this->curBehaviourId == 8) {
							pCVar16 = CActor::GetBehaviour
							((CActor*)this, this->curBehaviourId);
							pCVar16[0x16].pVTable = (CBehaviourVtable*)0x1;
						}
						else {
							(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0xa1, 0);
						}
						return 1;
					})
					return 0;
				}
				if (msg == 3) {
					IMPLEMENTATION_GUARD(
					pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
					fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
					bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
					if (!bVar9) {
						iVar13 = this->actorState;
						uVar10 = 0;
						if (iVar13 != -1) {
							pAVar12 = GetStateCfg(iVar13);
							uVar10 = pAVar12->flags_0x4;
						}
						bVar9 = (uVar10 & 1) != 0;
					}
					if (((!bVar9) && (bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar9 == false)) &&
						((this->base).field_0x1558 <= 0.0)) {
						if ((this->flags & 0x800000) == 0) {
							this->field_0x2e4 = 10.0;
							*(CActor**)&(this->base).field_0x1028 = pSender;
							iVar13 = ChooseStateDead(this, 0, (int)pMsgParam, 0);
							if (iVar13 == -1) {
								(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x97, -1);
							}
							else {
								(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, iVar13, -1);
							}
							return 1;
						}
						return 0;
					})
					return 0;
				}
				if (msg == 2) {
					IMPLEMENTATION_GUARD(
					pCVar11 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
					fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
					bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
					if (!bVar9) {
						iVar13 = this->actorState;
						uVar10 = 0;
						if (iVar13 != -1) {
							pAVar12 = GetStateCfg(iVar13);
							uVar10 = pAVar12->flags_0x4;
						}
						bVar9 = (uVar10 & 1) != 0;
					}
					if (((bVar9) || (bVar9 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar9 != false)) ||
						(0.0 < (this->base).field_0x1558)) {
						return 0;
					}
					if ((this->flags & 0x800000) != 0) {
						return 0;
					}
					iVar13 = this->actorState;
					bVar9 = false;
					if ((0xdb < iVar13) && (iVar13 < 0xdf)) {
						bVar9 = true;
					}
					/* WARNING: Load size is inaccurate */
					if ((bVar9) && (*pMsgParam != 10)) {
						return 0;
					}
					/* WARNING: Load size is inaccurate */
					iVar13 = *pMsgParam;
					if (((iVar13 != 10) && (iVar13 != 7)) && (iVar13 != 8)) {
						uVar10 = ActorFunc_0013ea40((CActor*)this, pSender, (int*)pMsgParam, 0);
						return uVar10;
					}
					uVar10 = CActorHero::TestState_IsInTheWind((Actor*)this, 0xffffffff);
					/* WARNING: Load size is inaccurate */
					if ((uVar10 != 0) && (*pMsgParam == 10)) {
						edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x30), &eStack112, (edF32VECTOR4*)((int)pMsgParam + 0x20));
						pTVar15 = GetTimer();
						edF32Vector4ScaleHard(0.02 / pTVar15->cutsceneDeltaTime, &eStack224, &eStack112);
						peVar24 = this->dynamicExt.aVelocity;
						edF32Vector4AddHard(peVar24, peVar24, &eStack224);
						fVar25 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
						this->dynamicExt.aVelocityMagnitudes[0] = fVar25;
						uVar10 = ActorFunc_0013ea40((CActor*)this, pSender, (int*)pMsgParam, 0);
						return uVar10;
					})
				}
				else {
					if (msg == 1) {
						IMPLEMENTATION_GUARD(
						pTVar15 = Timer::GetTimer();
						return pTVar15->scaledTotalTime < (this->base).field_0x155c ^ 1;)
					}
					if (msg == 0x79) {
						pCVar11 = GetLifeInterface();
						fVar25 = pCVar11->GetValue();
						bVar9 = fVar25 - this->field_0x2e4 <= 0.0f;
						if (!bVar9) {
							iVar13 = this->actorState;
							uVar10 = 0;
							if (iVar13 != -1) {
								pAVar12 = GetStateCfg(iVar13);
								uVar10 = pAVar12->flags_0x4;
							}
							bVar9 = (uVar10 & 1) != 0;
						}
						if ((bVar9) || (0.0f < this->field_0x1558)) {
							return 0;
						}
						IMPLEMENTATION_GUARD_LOG(
						UpdateMedallion())
					}
					else {
						if (msg == 0x6b) {
							pCVar11 = GetLifeInterface();
							fVar25 = pCVar11->GetValue();
							bVar9 = fVar25 - this->field_0x2e4 <= 0.0f;
							if (!bVar9) {
								iVar13 = this->actorState;
								uVar10 = 0;
								if (iVar13 != -1) {
									pAVar12 = GetStateCfg(iVar13);
									uVar10 = pAVar12->flags_0x4;
								}
								bVar9 = (uVar10 & 1) != 0;
							}
							if ((bVar9) || (0.0f < this->field_0x1558)) {
								return 0;
							}
							IMPLEMENTATION_GUARD_LOG(
							FUN_00326d90((int)this, (uint)pMsgParam);)
						}
						else {
							if (msg == 0x62) {
								pCVar11 = GetLifeInterface();
								fVar25 = pCVar11->GetValue();
								bVar9 = fVar25 - this->field_0x2e4 <= 0.0f;
								if (!bVar9) {
									iVar13 = this->actorState;
									uVar10 = 0;
									if (iVar13 != -1) {
										pAVar12 = GetStateCfg(iVar13);
										uVar10 = pAVar12->flags_0x4;
									}
									bVar9 = (uVar10 & 1) != 0;
								}

								if ((bVar9) || (0.0f < this->field_0x1558)) {
									return 0;
								}

								uint boomyId = reinterpret_cast<uint>(pMsgParam);

								if (boomyId == 3) {
									CActor::SV_PatchMaterial(gBoomyHashCodes[1], gBoomyHashCodes[3], (ed_g2d_manager*)0x0);
								}
								else {
									if (boomyId == 2) {
										CActor::SV_PatchMaterial(gBoomyHashCodes[1], gBoomyHashCodes[2], (ed_g2d_manager*)0x0)
											;
									}
									else {
										if (boomyId == 1) {
											CActor::SV_PatchMaterial(gBoomyHashCodes[1], gBoomyHashCodes[1], (ed_g2d_manager*)0x0);
										}
									}
								}
								if (pMsgParam == (void*)0x0) {
									SetBoomyHairOff();
								}
								else {
									SetBoomyHairOn();
								}
							}
							else {
								if (msg == 6) {
									IMPLEMENTATION_GUARD(
									pCVar11 = (*(this->pVTable)->GetLifeInterface)
										((CActor*)this);
									fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
									bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
									if (!bVar9) {
										iVar13 = this->actorState;
										uVar10 = 0;
										if (iVar13 != -1) {
											pAVar12 = (*(this->pVTable)->GetStateCfg)
												((CActor*)this, iVar13);
											uVar10 = pAVar12->flags_0x4;
										}
										bVar9 = (uVar10 & 1) != 0;
									}
									if ((!bVar9) && ((this->base).field_0x1558 <= 0.0)) {
										if (this->actorState == 0xdb) {
											(*(this->pVTable)->SetBehaviour)
												((CActor*)this, 7, 0x73, 0xffffffff);
										}
										return 1;
									})
									return 0;
								}
								if (msg == 5) {
									IMPLEMENTATION_GUARD(
									pCVar11 = (*(this->pVTable)->GetLifeInterface)
										((CActor*)this);
									fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
									bVar9 = fVar25 - this->field_0x2e4 <= 0.0;
									if (!bVar9) {
										iVar13 = this->actorState;
										uVar10 = 0;
										if (iVar13 != -1) {
											pAVar12 = (*(this->pVTable)->GetStateCfg)
												((CActor*)this, iVar13);
											uVar10 = pAVar12->flags_0x4;
										}
										bVar9 = (uVar10 & 1) != 0;
									}
									if ((bVar9) || (0.0 < (this->base).field_0x1558)) {
										return 0;
									}
									uVar10 = ActorFunc_00132c60((Actor*)this, 0xffffffff);
									if ((uVar10 != 0) && (*(int*)&(this->base).field_0x1b78 == 3)) {
										pCVar18 = (this->base).pPlayerInput;
										if ((pCVar18 == (CPlayerInput*)0x0) || ((this->base).field_0x18dc != 0)) {
											fVar25 = 0.0;
										}
										else {
											fVar25 = pCVar18->aButtons[9].clickValue;
										}
										if (fVar25 != 0.0) {
											*(undefined4*)&(this->base).field_0x1b64 = 1;
											(*(this->pVTable)->SetState)
												((CActor*)this, 0xd5, 0xffffffff);
											return 1;
										}
									}
									if ((this->base).boomyState_0x1b70 != 3) {
										uVar10 = ActorFunc_00132c60((Actor*)this, 0xffffffff);
										if (uVar10 == 0) {
											FUN_00136e50((int)this);
										}
										else {
											pAnimationController = this->pAnimationController;
											fVar25 = (this->base).field_0x1b98;
											*(undefined**)&(this->base).field_0x1000 = &DAT_bf800000;
											iVar13 = CAnimation::PhysicalLayerFromLayerId(pAnimationController, 8);
											peVar23 = (pAnimationController->anmBinMetaAnimator).base.aAnimData + iVar13;
											peVar23->field_0x0 = 3;
											peVar23->field_0x4 = fVar25;
											iVar14 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(137));
											edAnmBinMetaAnimator::SetAnimOnLayer(pAnimationController, iVar14, iVar13, 0xffffffff);
										}
										if (this->actorState - 0x73U < 3) {
											SetState(0x75, 0x82);
										}
										CActorHero::SetBoomyFunc((CActorHero*)this, 2);
									})
									return 1;
								}
								if (msg == 7) {
									/* WARNING: Load size is inaccurate */
									ActorMessage_7* pBoneMessage = reinterpret_cast<ActorMessage_7*>(pMsgParam);
									iVar13 = pBoneMessage->field_0x0;
									if (iVar13 == 5) {
										peVar1 = this->pMeshTransform;
										local_60 = peVar1->base.transformA.rowY;

										peVar2 = (this->pCollisionData)->pObbPrim;
										fVar25 = (peVar2->field_0xb0).y + (peVar2->field_0x90).y;
										bVar9 = IsFightRelated(this->curBehaviourId);
										if (bVar9 == false) {
											iVar13 = this->actorState;
											if (iVar13 == 0x11a) {
												fVar25 = fVar25 * 0.875f;
											}
											else {
												if (iVar13 == 0x11b) {
													fVar25 = fVar25 * 0.9f;
												}
												else {
													uVar10 = TestState_IsOnAToboggan(0xffffffff);
													if (uVar10 == 0) {
														fVar25 = fVar25 * 0.9f;
													}
													else {
														fVar25 = 1.4f;
													}
												}
											}
										}
										else {
											local_60 = g_xVector;
											fVar25 = fVar25 * 0.9f;
										}

										edF32Vector4ScaleHard(fVar25, &pBoneMessage->field_0x20, &local_60);
										return 1;
									}

									if (iVar13 == 1) {
										uVar10 = CActorFighter::InterpretMessage(pSender, 7, pMsgParam);
										return uVar10;
									}

									if ((iVar13 == 4) || (iVar13 == 3)) {
										IMPLEMENTATION_GUARD(
										SV_GetBoneWorldPosition(this->animKey_0x1588, &eStack64);
										edF32Vector4SubHard(&eStack64, &eStack64, &this->currentLocation);
										/* WARNING: Load size is inaccurate */
										if (*pMsgParam == 4) {
											local_50.x = 0.0;
											local_50.z = 0.0;
										}
										else {
											local_50.x = 0.25;
											local_50.z = 0.5;
										}
										local_50.w = 0.0;
										local_50.y = 0.5;
										edF32Matrix4MulF32Vector4Hard
										(&local_50, (edF32MATRIX4*)this->pMeshTransform,
											&local_50);
										edF32Vector4AddHard((edF32VECTOR4*)((int)pMsgParam + 0x20), &eStack64, &local_50);)
										return 1;
									}
									if (iVar13 == 2) {
										IMPLEMENTATION_GUARD(
										local_20.x = (float)_DAT_00425870;
										local_20.y = (float)((ulong)_DAT_00425870 >> 0x20);
										local_20.z = DAT_00425878;
										local_20.w = DAT_0042587c;
										SV_GetBoneWorldPosition((this->base).animKey_0x1588, &eStack48);
										edF32Vector4SubHard(&eStack48, &eStack48,
											&this->currentLocation);
										edF32Matrix4MulF32Vector4Hard
										(&local_20, (edF32MATRIX4*)this->pMeshTransform,
											&local_20);
										edF32Vector4AddHard((edF32VECTOR4*)((int)pMsgParam + 0x20), &eStack48, &local_20);)
										return 1;
									}
								}
								else {
									if ((msg == 0x49) &&
										(uVar10 = CActorHero::TestState_IsOnAToboggan(0xffffffff), uVar10 != 0)) {
										edF32VECTOR4* pOutMsgData = reinterpret_cast<edF32VECTOR4*>(pMsgParam);
										peVar1 = this->pMeshTransform;

										*pOutMsgData = (peVar1->base).transformA.rowT;
										return 1;
									}
								}
							}
						}
					}
				}
			}
			goto LAB_00344ed0;
		}
		IMPLEMENTATION_GUARD(
		piVar17 = (int*)&(this->base).field_0xab8;
		if (0 < (int)pMsgParam) {
			fVar26 = (float)(**(code**)(*piVar17 + 0x24))(piVar17);
			fVar25 = fVar26;
			GetParam1_0x18((int)piVar17);
			if (fVar26 < fVar25) goto LAB_00343180;
		}
		if ((int)pMsgParam < 0) {
			fVar25 = (float)(**(code**)(*piVar17 + 0x24))(piVar17);
			fVar26 = GetFloatParam1_0x1c((int)piVar17);
			fVar25 = fVar25 + fVar26;
			GetParam1_0x18((int)piVar17);
			if (fVar25 < fVar26) {
			LAB_00343180:
				fVar25 = GetFloatParam1_0x1c(&(this->base).field_0xab8);
				SetFloatParam1_0x1cToParam2(fVar25 - (float)(int)pMsgParam, &(this->base).field_0xab8);
				if (0.0 < (float)(int)pMsgParam) {
					fVar25 = (float)(**(code**)(*(int*)&(this->base).field_0xab8 + 0x24))(&(this->base).field_0xab8);
					(**(code**)(*(int*)&(this->base).field_0xab8 + 0x20))
						((float)(int)pMsgParam + fVar25, &(this->base).field_0xab8);
				}
				return 1;
			}
		})
	}
LAB_00344ed0:
	uVar20 = CActorFighter::InterpretMessage(pSender, msg, pMsgParam);
	return uVar20;
}

int CActorHeroPrivate::InterpretEvent(edCEventMessage* param_2, undefined8 param_3, int param_4, uint* param_5)
{
	uint levelId;
	bool bVar1;
	CLifeInterface* pCVar2;
	AnimResult* pAVar3;
	uint uVar4;
	undefined4 uVar5;
	uint uVar6;
	edF32MATRIX4* peVar7;
	Timer* pTVar8;
	edF32VECTOR4* v0;
	long lVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	undefined auStack80[76];
	//e_ed_event_prim3d_type eStack4;
	int iVar9;

	uVar6 = *param_5;
	lVar10 = (long)(int)uVar6;
	levelId = param_5[1];
	pCVar2 = GetLifeInterface();
	fVar11 = pCVar2->GetValue();

	bVar1 = fVar11 - this->field_0x2e4 <= 0.0f;

	if (!bVar1) {
		iVar9 = this->actorState;
		if (iVar9 == -1) {
			uVar4 = 0;
		}
		else {
			pAVar3 = GetStateCfg(iVar9);
			uVar4 = pAVar3->flags_0x4 & 1;
		}
		bVar1 = uVar4 != 0;
	}

	if (((!bVar1) && (this->field_0x1558 <= 0.0f)) || (iVar9 = 0, lVar10 == 1)) {
		switch (uVar6) {
		default:
			iVar9 = CActor::InterpretEvent(param_2, (uint)(uint*)param_3, (uint)param_4, param_5);
			break;
		case 1:
			IMPLEMENTATION_GUARD(
			if ((param_3 == 2) && (bVar1 = CActorHero::TestState_IsInCheatMode((CActorHero*)this), bVar1 == false)) {
				if (levelId == 3) {
					this->field_0x2e4 = 10.0;
					(*(this->base.base.pVTable)->SetBehaviour)((CActor*)this, 7, 0x9e, 0xffffffff);
					lVar10 = (*(code*)(this->base.base.pVTable)->GetInputManager)(this, 1, 0);
					if (lVar10 != 0) {
						FloatFunc_001b66f0(1.0, 0.0, 0.2, 0.0, (float*)((int)lVar10 + 0x40), 0);
					}
					iVar9 = 1;
				}
				else {
					if (levelId == 2) {
						this->field_0x2e4 = 10.0;
						(*(this->base.base.pVTable)->SetBehaviour)((CActor*)this, 7, 0x9d, 0xffffffff);
						lVar10 = (*(code*)(this->base.base.pVTable)->GetInputManager)(this, 1, 0);
						if (lVar10 != 0) {
							FloatFunc_001b66f0(1.0, 0.0, 0.2, 0.0, (float*)((int)lVar10 + 0x40), 0);
						}
						iVar9 = 1;
					}
					else {
						if (levelId == 1) {
							this->field_0x2e4 = 10.0;
							(*(this->base.base.pVTable)->SetBehaviour)
								((CActor*)this, 7, 0xa2, 0xffffffff);
							iVar9 = 1;
						}
						else {
							if (levelId == 0) {
								this->field_0x2e4 = 5.0;
								if (this->base.base.curBehaviourId == 8) {
									uVar5 = (*(this->base.base.pVTable)->GetLifeInterface)((CActor*)this);
									fVar11 = (float)(**(code**)(*(int*)uVar5 + 0x24))(uVar5);
									bVar1 = fVar11 - this->field_0x2e4 <= 0.0;
									if (!bVar1) {
										iVar9 = this->base.base.actorState;
										if (iVar9 == -1) {
											uVar6 = 0;
										}
										else {
											pAVar3 = (*(this->base.base.pVTable)->GetStateCfg)
												((CActor*)this, iVar9);
											uVar6 = pAVar3->flags_0x4 & 1;
										}
										bVar1 = uVar6 != 0;
									}
									if (bVar1) {
										return 0;
									}
								}
								(*(this->base.base.pVTable)->SetBehaviour)
									((CActor*)this, 7, 0xa1, 0xffffffff);
								iVar9 = 1;
							}
							else {
								this->field_0x2e4 = 10.0;
								(*(this->base.base.pVTable)->SetBehaviour)
									((CActor*)this, 7, 0x97, 0xffffffff);
								iVar9 = 1;
							}
						}
					}
				}
			}
			else {
				iVar9 = 1;
			})
			break;
		case 2:
		case 3:
			IMPLEMENTATION_GUARD(
			if ((param_3 == 2) && (levelId != 0xffffffff)) {
				CSectorManager::SwitchToSector(CScene::ptable.g_SectorManager_00451670, levelId, (uint)(lVar10 == 2));
			}
			iVar9 = 1;)
			break;
		case 5:
			IMPLEMENTATION_GUARD(
			if (param_3 == 2) {
				LevelScheduleManager::Level_Teleport
				(LevelScheduleManager::gThis, (CActor*)this, levelId, param_5[2], param_5[3], 0xffffffff);
			}
			iVar9 = 1;)
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
		case 0x10:
			IMPLEMENTATION_GUARD_AUDIO(
				CAudioManager::ReceiveEvent(CScene::ptable.g_AudioManager_00451698, (int*)param_2, lVar10, (uint*)param_3,
				(long)(int)((uint)param_4 - 1), param_5 + 1);)
			iVar9 = 1;
			break;
		case 0x15:
			IMPLEMENTATION_GUARD(
			if (levelId == 0x7f) {
				peVar7 = (edF32MATRIX4*)
					edEventGetChunkZonePrimitive
					((CScene::ptable.g_EventManager_006f5080)->activeChunkId,
						param_2->field_0x4->field_0x1c->pZone, 0, &eStack4);
				edF32Matrix4GetInverseGaussSoft((edF32MATRIX4*)auStack80, peVar7);
				edF32Vector4NormalizeHard(&eStack112, (edF32VECTOR4*)(auStack80 + 0x10));
				fVar12 = this->base.dynamic.linearJerk;
				fVar11 = 2.0;
				if (2.0 <= fVar12) {
					fVar11 = fVar12;
				}
				pTVar8 = GetTimer();
				edF32Vector4ScaleHard((fVar11 * -1.5) / pTVar8->cutsceneDeltaTime, &eStack96, &eStack112);
				v0 = this->dynamicExt.aVelocity;
				edF32Vector4AddHard(v0, v0, &eStack96);
				fVar11 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
				this->dynamicExt.aVelocityMagnitudes[0] = fVar11;
			}
			iVar9 = 0;)
		}
	}
	return iVar9;
}

void CActorHeroPrivate::ResetStdDefaultSettings()
{
	this->field_0x1040 = 9.77f;

	this->airRotationRate = 3.91f;
	this->field_0x104c = 2.5f;
	this->runSpeed = 5.4f;
	this->field_0x1054 = 1.0f;
	this->field_0x1058 = 15.0f;
	this->field_0x105c = 50.0f;
	this->airNoInputSpeed = 4.0f;
	this->field_0x1150 = 17.0f;
	this->field_0x1154 = 1.7f;
	this->field_0x1158 = 0.2f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x115c = 0.18f;)
	this->field_0x1160 = 0.1f;
	this->field_0x1164 = 1.5f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1174 = 5.0f;
	this->field_0x1178 = 1.9f;
	this->field_0x117c = 1.4f;
	this->field_0x1180 = 0.4f;)
	this->field_0x1184 = 0.1f;
	this->landSpeed = 0.0f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1430 = 1.396263f;
	this->field_0x1570 = 0.6f;)
	this->field_0x1574 = 0.0f;
	return;
}

void CActorHeroPrivate::ResetSlideDefaultSettings()
{
	bool bVar1;

	this->field_0x1068 = 0.5f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x106c = 12.0f;
	this->field_0x1070 = 35.0f;)
	this->field_0x1074 = 2.094395f;
	this->field_0x1078 = 0.6f;
	this->field_0x107c = 0.2f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1080 = 100.0f;
	this->field_0x1084 = 300.0f;
	this->field_0x1088 = 1.0472f;
	this->field_0x108c = 0.95f;
	this->field_0x10c0 = 19.0f;
	this->field_0x10c4 = 25.0f;
	this->field_0x10c8 = 17.0f;

	bVar1 = EvolutionTobogganCanJump();
	if (bVar1 == false) {
		this->field_0x10cc = this->field_0x10c8;
	}
	else {
		this->field_0x10cc = this->field_0x10c4;
	}

	this->field_0x10d0 = this->field_0x10c0;
	this->field_0x10d4 = 5.0f;

	this->field_0x10e0 = gF32Vector4UnitZ;
	
	this->field_0x10f0 = 0.0f;)
	this->field_0x10f4 = 0.0f;

	IMPLEMENTATION_GUARD_LOG(
	*(undefined4*)&this->field_0x10f8 = 0;
	this->field_0x1104 = 0.0;)
	this->field_0x1108 = 0.0;
	return;
}

void CActorHeroPrivate::ResetBoomyDefaultSettings()
{
	uint* puVar1;
	ulong uVar2;
	IMPLEMENTATION_GUARD_LOG(
	CScene::GetManager(MO_Camera);
	this->field_0x1ba0 = 0x41000000;
	*(undefined4*)&this->field_0x1b98 = 0;
	*(undefined4*)&this->field_0x1b9c = 0;
	this->field_0x1b94 = 0;
	*(undefined**)&this->field_0x1b90 = &DAT_bf800000;
	*(undefined4*)&this->field_0x1b64 = 0;
	this->field_0x1b68 = 0;
	this->field_0x1b6c = 0;)
	SetBoomyFunc(0);
	IMPLEMENTATION_GUARD_LOG(
	*(undefined4*)&this->field_0x1b78 = 0;
	this->field_0x1b74 = 0;
	*(undefined**)&this->field_0x1000 = &DAT_bf800000;
	*(undefined4*)&this->field_0x1004 = 0x41200000;
	*(undefined4*)&this->field_0x1008 = 0x40a00000;
	DisableLayer(this, 8);
	SetMagicMode(this, 0);
	uVar2 = FUN_0034cdc0();
	if (uVar2 == 0) {
		CAnimation::AddDisabledBone
		(this->data.pAnimationController, this->animKey_0x1584);
	}
	else {
		CAnimation::RemoveDisabledBone
		(this->data.pAnimationController, this->animKey_0x1584);
	}
	/* BOOMY_1 */
	this->field_0x1ba4 = 0;
	puVar1 = CActorFighter::FindBlowByName((CActorHero*)this, s_BOOMY_1_00429998);
	*(uint**)&this->field_0x1bb0 = puVar1;
	puVar1 = CActorFighter::FindBlowByName((CActorHero*)this, s_BOOMY_2_004299a0);
	*(uint**)&this->field_0x1bb4 = puVar1;
	puVar1 = CActorFighter::FindBlowByName((CActorHero*)this, s_BOOMY_3_004299a8);
	this->field_0x1bb8 = (undefined*)puVar1;)
	return;
}

void CActorHeroPrivate::ResetGripClimbDefaultSettings()
{
	//this->field_0x14ac = 2.0;
	this->field_0x14b0 = 1.22173f;
	this->field_0x14b4 = 0.5235988f;
	//*(undefined4*)&this->field_0x14b8 = 0x3f9c61aa;
	//*(undefined4*)&this->field_0x14bc = 0x3e99999a;
	//*(undefined4*)&this->field_0x14c0 = 0x3e4ccccd;
	//this->field_0x14c4 = 0.8726646;
	//*(undefined4*)&this->field_0x14c8 = 0x3fb33333;
	//*(undefined4*)&this->field_0x14cc = 0x40a00000;
	//*(undefined4*)&this->field_0x14d0 = 0x3fc00000;
	//*(undefined4*)&this->field_0x14d4 = 0x3fe66666;
	//*(undefined4*)&this->field_0x14d8 = 0x41400000;
	//*(undefined4*)&this->field_0x14dc = 0x41a00000;
	//*(undefined4*)&this->field_0x14e0 = 0x40400000;
	//*(undefined**)&this->field_0x14e4 = &DAT_41380000;
	//*(undefined4*)&this->field_0x14e8 = 0x41000000;
	//*(undefined4*)&this->field_0x14ec = 0x40a00000;
	//*(undefined4*)&this->field_0x14f0 = 0x41180000;
	//*(undefined4*)&this->field_0x1514 = 0x40000000;
	//*(undefined4*)&this->field_0x1510 = 0x40000000;
	//*(undefined4*)&this->field_0x1518 = 0x3f800000;
	this->field_0x151c = 4.0f;
	//*(undefined4*)&this->field_0x1520 = 0x41490fdb;
	//*(undefined4*)&this->field_0x1524 = 0x40000000;
	//*(undefined4*)&this->field_0x1528 = 0x41000000;
	//*(undefined4*)&this->field_0x152c = 0x42480000;
	//*(undefined4*)&this->field_0x1530 = 0x40e00000;
	//*(undefined4*)&this->field_0x1534 = 0x3ecccccd;
	return;
}

void CActorHeroPrivate::ResetWindDefaultSettings()
{
	//this->field_0x11f4 = 0.0;
	//this->field_0x11f8 = 0;
	//*(undefined4*)&this->field_0x11fc = 0;
	//*(undefined4*)&this->field_0x1200 = 0;
	//*(undefined4*)&this->field_0x1208 = 0;
	//*(undefined4*)&this->field_0x1204 = 0;
	//*(undefined4*)&this->field_0x13d8 = 0;
	//*(undefined4*)&this->field_0x11ec = 0;
	//*(undefined4*)&this->field_0x11e0 = 0x41400000;
	//*(undefined4*)&this->field_0x11e4 = 0x41400000;
	//*(undefined4*)&this->field_0x11e8 = 0x41a00000;
	//*(undefined4*)&this->field_0x13d4 = 0x40c00000;
	//*(undefined4*)&this->field_0x13dc = 0x3fa00000;
	//*(undefined4*)&this->field_0x13e0 = 0x3fa00000;
	//*(undefined4*)&this->field_0x13e4 = 0x40600000;
	this->field_0x1410 = 1.0f;
	this->field_0x1414 = 2.0f;
	this->field_0x1418 = 5.0f;
	this->field_0x141c = 10.0f;
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
				this->field_0x1454 = bVar3;
				iVar4 = 0xa9;
				if (this->field_0x1454 == false) {
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
						iVar4 = STATE_HERO_STAND;
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
									iVar4 = STATE_HERO_RUN;
								}
							}
						}
						else {
							iVar4 = STATE_HERO_STAND;
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
		iVar4 = STATE_HERO_STAND;
		SetState(STATE_HERO_STAND, -1);
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

bool CActorHeroPrivate::CanEnterToboggan()
{
	bool bCanEnterToboggan;
	CBehaviour* pBehaviour;

	bCanEnterToboggan = ColWithAToboggan();
	if (bCanEnterToboggan == false) {
		bCanEnterToboggan = false;
	}
	else {
		pBehaviour = CActor::GetBehaviour(0xb);
		if (pBehaviour == (CBehaviour*)0x0) {
			pBehaviour = CActor::GetBehaviour(10);
			if (pBehaviour == (CBehaviour*)0x0) {
				pBehaviour = CActor::GetBehaviour(9);
				bCanEnterToboggan = pBehaviour != (CBehaviour*)0x0;
			}
			else {
				bCanEnterToboggan = true;
			}
		}
		else {
			bCanEnterToboggan = true;
		}
	}
	return bCanEnterToboggan;
}

int CActorHeroPrivate::ChooseStateFall(int param_2)
{
	int fallState;

	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
		fallState = 0x7e;

		if (this->field_0x142c != 0) {
			if (this->distanceToGround < 10.3f) {
				this->field_0x1020 = 1;

				fallState = 0x7f;

				if (param_2 == 0) {
					fallState = 0x7e;
				}
			}
			else {
				this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
				fallState = 0xf0;
			}
		}
	}
	else {
		this->field_0x1020 = 1;
		fallState = 0x7a;
	}

	return fallState;
}

int CActorHeroPrivate::ChooseStateLanding(float speed)
{
	CPlayerInput* pCVar1;
	edF32VECTOR4* peVar2;
	bool bVar3;
	AnimResult* pAVar4;
	uint uVar5;
	Timer* pTVar6;
	undefined4 uVar7;
	edF32VECTOR4* peVar8;
	int landingState;
	long lVar9;
	edF32VECTOR4* peVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	landingState = this->actorState;

	fVar12 = 0.0f;

	if (landingState == -1) {
		uVar5 = 0;
	}
	else {
		pAVar4 = GetStateCfg(landingState);
		uVar5 = pAVar4->flags_0x4;
	}

	if ((uVar5 & 0x100) != 0) {
		return 0x73;
	}

	this->landSpeed = speed;

	if ((this->field_0x1420 == 0) || (this->field_0x1020 == 0)) {
	LAB_0014c070:
		this->field_0x1020 = 1;

		bVar3 = CanEnterToboggan();
		if (bVar3 == false) {
			landingState = 0xe4;

			if ((this->dynamic.flags & 2) == 0) {
				if (0.0f < fVar12) {
					pTVar6 = GetTimer();
					fVar12 = edFIntervalLERP(pTVar6->scaledTotalTime - this->field_0x1548, this->field_0x1410, this->field_0x1414, 1.0f, 0.2f);
					this->field_0x118c = fVar12;
					landingState = 0x81;
				}
				else {
					pCVar1 = this->pPlayerInput;
					if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar12 = 0.0f;
					}
					else {
						fVar12 = pCVar1->aButtons[5].clickValue;
					}

					if (fVar12 == 0.0f) {
						landingState = 0x7a;

						if (2.0f < this->dynamic.linearJerk) {
							landingState = 0x7d;
						}
					}
					else {
						if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar12 = 0.0f;
						}
						else {
							fVar12 = pCVar1->aAnalogSticks[0].magnitude;
						}

						landingState = 0x89;

						if (fVar12 <= 0.3f) {
							landingState = 0x83;
						}
					}
				}
			}
		}
		else {
			pTVar6 = GetTimer();
			fVar12 = edFIntervalUnitDstLERP(pTVar6->scaledTotalTime - this->time_0x1538, 0.3f, 1.5f);
			this->field_0x10f4 = fVar12;
			pTVar6 = GetTimer();
			fVar12 = edFIntervalLERP(pTVar6->scaledTotalTime - this->time_0x1538, 0.3f, 1.5f, 0.5f, 1.5f);
			this->field_0x1108 = fVar12;
			landingState = 0xee;
		}
	}
	else {
		pTVar6 = GetTimer();
		fVar12 = edFIntervalLERP(pTVar6->scaledTotalTime - this->field_0x1548, this->field_0x1410, this->field_0x1414, this->field_0x1418, this->field_0x141c);

		if (this->field_0x1424 == 0) {
			CLifeInterface* pLifeInterface = GetLifeInterface();
			fVar11 = pLifeInterface->GetValue();
			if (0.0f < fVar11 - fVar12) {
				IMPLEMENTATION_GUARD(
				lVar9 = GetInputManager(1, 0);
				if (lVar9 != 0) {
					FloatFunc_001b66f0(1.0, 0.0, 0.1, 0.0, (float*)((int)lVar9 + 0x1c), 0);
				}
				(*(code*)(this->pVTable)->LifeDecrease)(fVar12, this);
				ActorTimeFunc_00349a80(2.0, (Actor*)this, 1);
				goto LAB_0014c070;)
			}
		}
		if (this->field_0x1424 == 0) {
			LifeDecrease(fVar12);
		}
		else {
			this->field_0x2e4 = fVar12;
		}

		bVar3 = CanEnterToboggan();
		if (bVar3 == false) {
			IMPLEMENTATION_GUARD(
			peVar8 = (edF32VECTOR4*)&DAT_00000010;
			peVar10 = &local_10;
			peVar2 = peVar10;
			while (peVar2 != (edF32VECTOR4*)0x0) {
				*(undefined*)&peVar10->x = 0;
				peVar10 = (edF32VECTOR4*)((int)&peVar10->x + 1);
				peVar8 = (edF32VECTOR4*)((int)&peVar8[-1].w + 3);
				peVar2 = peVar8;
			}
			local_10.x = this->rotationEuler.x + 1.570796;
			local_10.y = this->rotationEuler.y;
			local_10.z = this->rotationEuler.z;
			local_20.x = (float)_DAT_0040e6e0;
			local_20.y = (float)((ulong)_DAT_0040e6e0 >> 0x20);
			local_20.z = _DAT_0040e6e8;
			local_20.w = _DAT_0040e6ec;
			edF32Vector4AddHard(&local_20, &local_20, (edF32VECTOR4*)&this->dynamic);
			edF32Vector4NormalizeHard(&local_20, &local_20);
			ActorFunc_0030a780(speed, (Actor*)this, &local_10, &local_20, 0);
			pTVar6 = GetTimer();
			landingState = -1;
			this->field_0x1558 = pTVar6->scaledTotalTime + 1.5f;)
		}
		else {
			pTVar6 = GetTimer();
			landingState = 0x9c;
			this->field_0x1558 = pTVar6->scaledTotalTime + 1.5f;
		}
	}

	return landingState;
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
	this->field_0x1020 = 1;
	this->field_0x1420 = 0;
	this->field_0x1424 = 0;
	//*(undefined4*)&this->field_0x1428 = 1;
	this->field_0x142c = 1;
	//*(undefined4*)&this->field_0x1024 = 0x3f800000;
	//this->field_0x1010 = 0;
	this->bFacingControlDirection = 1;
	this->field_0xff4 = 0;
	this->effort = 0.0f;
	this->animIdleSequenceIndex = 0xe6;
	//*(undefined4*)&this->field_0x100c = 0;
	//*(undefined4*)&this->field_0x1a54 = 0;
	this->field_0x1610 = 0;
	//*(undefined4*)&this->field_0x1614 = 0;
	pTVar5 = Timer::GetTimer();
	this->field_0x155c = pTVar5->scaledTotalTime;
	//*(undefined4*)&this->field_0x1560 = 0;
	//*(undefined4*)&this->field_0x1568 = 0;
	//ActorTimeFunc_00325c40(20.0, (Actor*)this, 0);
	this->field_0x1168 = 1;

	
	this->field_0xf00 = this->currentLocation;
	
	//this->field_0x102c.y = 0.0;
	//this->field_0x102c.z = 1.0;
	//this->field_0x102c.w = 0.0;
	//this->field_0x103c = 0.0;
	
	this->field_0x10a0.x = 0.0f;
	this->field_0x10a0.y = 1.0f;
	this->field_0x10a0.z = 0.0f;
	this->field_0x10a0.w = 0.0f;
	
	this->field_0x10b0.x = 0.0f;
	this->field_0x10b0.y = 1.0f;
	this->field_0x10b0.z = 0.0f;
	this->field_0x10b0.w = 0.0f;
	
	//*(undefined4*)&this->field_0x1090 = 0;
	this->field_0x1094 = 0;

	this->slideSlipIntensity = 0.0f;
	this->field_0x1454 = false;
	this->field_0x1455 = 0;
	this->pCheckpointManagerSubObjA_0x14a4 = (CActor*)0x0;
	//*(undefined4*)&this->field_0x14f4 = *(undefined4*)&this->field_0x14cc;
	//*(undefined4*)&this->field_0x14a8 = 0;
	//*(undefined4*)&this->field_0x1500 = 0;
	//*(undefined4*)&this->field_0x1504 = 0;
	//*(undefined4*)&this->field_0x1508 = 0;
	//*(undefined4*)&this->field_0x150c = 0;
	this->bounceLocation.x = -1.0f;
	this->bounceLocation.y = 0.0f;
	this->bounceLocation.z = 0.0f;
	this->bounceLocation.w = 0.0f;
	this->field_0x1460.x = -1.0f;
	this->field_0x1460.y = 0.0f;
	this->field_0x1460.z = 0.0f;
	this->field_0x1460.w = 0.0f;
	//*(undefined**)&this->field_0x1470 = &DAT_bf800000;
	//*(undefined4*)&this->field_0x1474 = 0;
	//*(undefined4*)&this->field_0x1478 = 0;
	//*(undefined4*)&this->field_0x147c = 0;
	//*(undefined4*)&this->field_0x1480 = 0;
	//*(undefined**)&this->field_0x1484 = &DAT_bf800000;
	//*(undefined4*)&this->field_0x1488 = 0;
	//*(undefined4*)&this->field_0x148c = 0;
	this->field_0x1490.x = 0.0f;
	this->field_0x1490.y = 0.0f;
	this->field_0x1490.z = 0.0f;
	this->field_0x1490.w = 0.0f;
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
	this->field_0xf18 = 0;
	this->pSoccerActor = 0;
	//this->field_0xf20 = (undefined*)0x0;
	this->field_0xf24 = (char*)0x0;
	//*(undefined4*)&this->field_0xf28 = 0;
	//this->field_0xf50 = (undefined*)0x0;
	//*(undefined4*)&this->field_0xf58 = 0;
	this->field_0x1490.x = 0.0f;
	this->field_0x1490.y = 0.0f;
	this->field_0x1490.z = 0.0f;
	this->field_0x1490.w = 0.0f;
	//*(undefined4*)&this->field_0xf5c = 0;
	this->field_0x15a0 = 1;
	this->field_0x1048 = 0.0f;
	this->time_0x1538 = 0.0f;
	this->time_0x153c = 0.0f;
	this->field_0x118c = 1.0f;
	//*(undefined4*)&this->field_0x1450 = 0xa7;
	pTVar5 = GetTimer();
	this->time_0x1538 = pTVar5->scaledTotalTime;
	pTVar5 = GetTimer();
	this->time_0x153c = pTVar5->scaledTotalTime;
	//pTVar5 = GetTimer();
	//*(float*)&this->field_0x1540 = pTVar5->scaledTotalTime;
	//pTVar5 = GetTimer();
	this->field_0x1548 = pTVar5->scaledTotalTime;
	//*(undefined4*)&this->field_0x154c = 0;
	//this->field_0x1554 = 0.0;
	//*(undefined4*)&this->field_0x1550 = 0;
	//this->field_0x1544 = 0.0;
	this->field_0x1558 = 0.0;
	//*(undefined4*)&this->field_0x1018 = 0;
	this->field_0xa80 = 0.0;
	this->field_0xa84 = 0.0;
	//*(undefined4*)&this->field_0xa88 = 0;
	//CVectorDyn::Reset((undefined4*)&this->field_0xf70);
	SetJumpCfg(0.1f, this->runSpeed, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
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
	this->field_0x1a10 = 0;
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
	//		iVar7 = (int)&(pCVar12->base).base.pVTable + iVar10;
	//		if (1 < *(byte*)(iVar7 + 0xd34)) {
	//			*(undefined*)(iVar7 + 0xd34) = 0;
	//		}
	//		iVar10 = iVar10 + 1;
	//	} while (iVar10 < 0x10);
	//	iVar8 = iVar8 + 1;
	//	pCVar12 = (CActorHeroPrivate*)&(pCVar12->base).base.data.field_0xc;
	//} while (iVar8 < 0x10);
	this->effort = 0.0;
	return;
}

void CActorHeroPrivate::BehaviourHero_InitState(int newState)
{
	int iVar1;
	CCollision* pCVar2;
	int* piVar3;
	int* piVar4;
	CCamera* pCamera;
	CCameraManager* pCVar5;
	uint uVar6;
	AnimResult* pAVar7;
	undefined4 uVar8;
	long lVar9;
	long lVar10;
	edF32VECTOR4* in_a2_lo;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	lVar9 = (long)(int)this;
	iVar1 = this->actorState;
	pCVar2 = this->pCollisionData;
	if (iVar1 != -1) {
		GetStateCfg(iVar1);
	}

	GetStateHeroFlags(this->actorState);
	
	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar6 = 0;
	}
	else {
		pAVar7 = GetStateCfg(iVar1);
		uVar6 = pAVar7->flags_0x4 & 1;
	}

	if (uVar6 == 0) {
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x1000;
	}
	else {
		pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffefff;
		this->field_0x1610 = 1;
		this->field_0x18dc = 1;
	}

	iVar1 = this->actorState;
	if (iVar1 == -1) {
		uVar6 = 0;
	}
	else {
		pAVar7 = GetStateCfg(iVar1);
		uVar6 = pAVar7->flags_0x4 & 0x100;
	}

	if (uVar6 == 0) {
		pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffffcf;
	}
	else {
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x30;
	}

	pCVar5 = (CCameraManager*)CScene::GetManager(MO_Camera);
	if ((pCVar5->pActiveCamera != (CCamera*)0x0) && ((pCVar5->pActiveCamera->flags_0xc & 0x40000) == 0)) {
		IMPLEMENTATION_GUARD_LOG(
		uVar6 = CActorHero::TestState_IsGripped((CActorHero*)this, 0xffffffff);
		if (((uVar6 != 0) &&
			(uVar6 = CActorHero::TestState_IsGripped((CActorHero*)this, this->heroFlags), uVar6 == 0)) ||
			((uVar6 = CActorHero::TestState_WindWall((Actor*)this, 0xffffffff), uVar6 != 0 &&
				(uVar6 = CActorHero::TestState_WindWall((Actor*)this, this->heroFlags), uVar6 == 0)))) {
			in_a2_lo = (edF32VECTOR4*)0x0;
			CCameraManager::PushCamera(pCVar5, this->pCameraViewBase_0x15b0, 0);
		}
		uVar6 = CActorHero::TestState_IsOnCeiling((int)this, 0xffffffff);
		if ((uVar6 != 0) && (uVar6 = CActorHero::TestState_IsOnCeiling((int)this, this->heroFlags), uVar6 == 0)) {
			in_a2_lo = (edF32VECTOR4*)0x0;
			CCameraManager::PushCamera(pCVar5, this->pWindWallCamera_0x15b4, 0);
		}
		if (this->actorState == 0xa1) {
			iVar1 = this->prevActorState;
			if (iVar1 == -1) {
				uVar6 = 0;
			}
			else {
				pAVar7 = GetStateCfg(iVar1);
				uVar6 = pAVar7->flags_0x4 & 1;
			}
			if (uVar6 == 0) {
				in_a2_lo = (edF32VECTOR4*)0x0;
				CCameraManager::PushCamera(pCVar5, (CCamera*)this->field_0xab4, 0);
			}
		})
	}

	uVar6 = TestState_IsFlying(this->heroFlags);
	if ((uVar6 == 0) && (uVar6 = TestState_IsFlying(0xffffffff), uVar6 != 0)) {
		IMPLEMENTATION_GUARD(
		StateHeroFlyInit(this);)
	}
	uVar6 = TestState_IsCrouched(this->heroFlags);
	if ((uVar6 == 0) && (uVar6 = TestState_IsCrouched(0xffffffff), uVar6 != 0)) {
		IMPLEMENTATION_GUARD(
		local_10.x = 0.5;
		local_10.y = 0.5;
		local_10.z = 0.5;
		local_20.y = 0.48;
		local_10.w = 0.0;
		local_20.w = 1.0;
		local_20.x = 0.0;
		local_20.z = 0.0;
		in_a2_lo = &local_20;
		(*(this->pVTable)->ChangeCollisionSphere)
			(0.1, (CActor*)this, &local_10, in_a2_lo);)
	}

	uVar6 = TestState_IsOnAToboggan(this->heroFlags);
	if ((uVar6 == 0) && (uVar6 = TestState_IsOnAToboggan(0xffffffff), uVar6 != 0)) {
		IMPLEMENTATION_GUARD(
		BeginToboggan(this);
		FxTobogganInit(this);)
	}

	uVar6 = TestState_IsOnAToboggan(this->heroFlags);
	if (uVar6 != 0) {
		iVar1 = this->prevActorState;
		if (iVar1 == -1) {
			uVar6 = 0;
		}
		else {
			pAVar7 = GetStateCfg(iVar1);
			uVar6 = pAVar7->flags_0x4 & 0x100;
		}
		if (uVar6 != 0) goto LAB_00341590;
	}

	uVar6 = TestState_IsOnAToboggan(0xffffffff);
	if (uVar6 != 0) {
		iVar1 = this->actorState;
		if (iVar1 == -1) {
			uVar6 = 0;
		}
		else {
			pAVar7 = GetStateCfg(iVar1);
			uVar6 = pAVar7->flags_0x4 & 0x100;
		}
		if (uVar6 != 0) {
			IMPLEMENTATION_GUARD(
			CFxHandle::SV_FX_Start(*(CFxHandle**)&this->field_0x10fc);
			piVar3 = *(int**)&this->field_0x10fc;
			piVar4 = (int*)piVar3[1];
			if (((piVar4 != (int*)0x0) && (iVar1 = *piVar3, iVar1 != 0)) && (iVar1 == piVar4[6])) {
				(**(code**)(*piVar4 + 0x10))(0, 0);
			})
		}
	}
LAB_00341590:
	uVar6 = TestState_IsFlying(this->heroFlags);
	if (((uVar6 == 0) ||
		(uVar6 = TestState_IsInTheWind(this->heroFlags), uVar6 != 0)) &&
		((uVar6 = TestState_IsFlying(0xffffffff), uVar6 != 0 &&
			(uVar6 = TestState_IsInTheWind(0xffffffff), uVar6 == 0)))) {
		IMPLEMENTATION_GUARD(
		StateHeroGlideInit();)
	}

	uVar6 = this->heroFlags;
	lVar10 = (long)(int)uVar6;

	IMPLEMENTATION_GUARD_LOG(
	uVar6 = CActorHero::TestState_WindWall((Actor*)this, uVar6);
	if (uVar6 == 0) {
		lVar10 = -1;
		uVar6 = CActorHero::TestState_WindWall((Actor*)this, 0xffffffff);
		if (uVar6 != 0) {
			pCVar5 = (CCameraManager*)CScene::GetManager(MO_Camera);
			pCamera = this->pJamgutCamera_0x15b8;
			lVar10 = (long)(int)pCamera;
			in_a2_lo = (edF32VECTOR4*)0x0;
			CCameraManager::PushCamera(pCVar5, pCamera, 0);
		}
	})

	switch (newState) {
	case STATE_HERO_SLIDE_SLIP_C:
	case STATE_HERO_JUMP_1_1_STAND:
	case STATE_HERO_JUMP_3_3_STAND:
	case STATE_HERO_JUMP_1_1_RUN:
	case STATE_HERO_JUMP_3_3_RUN:
	case STATE_HERO_FALL_A:
	case STATE_HERO_GRIP_B:
	case STATE_HERO_GRIP_HANG_IDLE:
	case STATE_HERO_GRIP_UP:
	case STATE_HERO_GRIP_GRAB:
		break;
	case STATE_HERO_STAND:
		StateHeroStandInit(1);
		break;
	case STATE_HERO_RUN:
		IMPLEMENTATION_GUARD_LOG(
		EnableFightCamera();)
		break;
	case STATE_HERO_JUMP_2_3_STAND:
		StateHeroJump_2_3Init();
			break;
	case STATE_HERO_JUMP_2_3_RUN:
		StateHeroJump_2_3Init();
		break;
	case STATE_HERO_JUMP_2_3_GRIP:
		StateHeroGripUpToJumpInit();
		StateHeroJump_2_3Init();
		break;
	case STATE_HERO_SLIDE_SLIP_A:
	case STATE_HERO_SLIDE_SLIP_B:
		StateHeroSlideSlipInit();
		break;
	case STATE_HERO_SLIDE_B:
		StateHeroSlideInit(1);
		break;
	case STATE_HERO_SLIDE_A:
		StateHeroSlideInit(0);
		break;
	default:
		assert(false);
		break;
	}
}

void CActorHeroPrivate::BehaviourHero_TermState(int oldState, int newState)
{
	switch (oldState) {
	case AS_None:
	case STATE_HERO_SLIDE_SLIP_A:
	case STATE_HERO_SLIDE_SLIP_B:
	case STATE_HERO_SLIDE_SLIP_C:
	case STATE_HERO_JUMP_1_1_STAND:
	case STATE_HERO_JUMP_3_3_STAND:
	case STATE_HERO_JUMP_1_1_RUN:
	case STATE_HERO_JUMP_3_3_RUN:
	case STATE_HERO_FALL_A:
	case STATE_HERO_GRIP_B:
	case STATE_HERO_GRIP_HANG_IDLE:
	case STATE_HERO_GRIP_UP:
	case STATE_HERO_GRIP_GRAB:
		// Nothing
		break;
	case STATE_HERO_RUN:
		StateHeroRunTerm();
		break;
	case STATE_HERO_STAND:
		StateHeroStandTerm();
		break;
	case STATE_HERO_JUMP_2_3_STAND:
	case STATE_HERO_JUMP_2_3_RUN:
	case STATE_HERO_JUMP_2_3_GRIP:
		StateHeroJump_2_3Term();
		break;
	default:
		assert(false);
		break;
	}
}

void CActorHeroPrivate::BehaviourHero_Manage()
{
	switch (this->actorState) {
	case STATE_HERO_STAND:
		StateHeroStand(1);
		break;
	case STATE_HERO_RUN:
		StateHeroRun();
		break;
	case STATE_HERO_JUMP_1_1_STAND:
		StateHeroJump_1_3(STATE_HERO_JUMP_2_3_STAND);
		break;
	case STATE_HERO_JUMP_2_3_STAND:
		StateHeroJump_2_3(0, 0, 0);
		break;
	case STATE_HERO_JUMP_3_3_STAND:
		StateHeroJump_3_3(0);
		break;
	case STATE_HERO_JUMP_1_1_RUN:
		StateHeroJump_1_3(STATE_HERO_JUMP_2_3_RUN);
		break;
	case STATE_HERO_JUMP_2_3_RUN:
		StateHeroJump_2_3(1, 1, 0);
		break;
	case STATE_HERO_JUMP_3_3_RUN:
		StateHeroJump_3_3(0);
		break;
	case STATE_HERO_FALL_A:
		StateHeroFall(this->airRotationRate, 1);
		break;
	case STATE_HERO_GRIP_B:
		StateHeroGrip(0.0f, STATE_HERO_GRIP_HANG_IDLE, 0);
		break;
	case STATE_HERO_GRIP_HANG_IDLE:
		StateHeroGrip(0.0f, -1, 0);
		break;
	case STATE_HERO_GRIP_UP:
		StateHeroGripUp(this->field_0x151c, -1.0f, STATE_HERO_JUMP_2_3_GRIP, 1);
		break;
	case STATE_HERO_JUMP_2_3_GRIP:
		StateHeroJump_2_3(0, 0, 0);
		break;
	case STATE_HERO_GRIP_GRAB:
		StateHeroGrip(0.0f, STATE_HERO_GRIP_B, 0);
		break;
	case STATE_HERO_SLIDE_SLIP_A:
		StateHeroSlideSlip(0xe3, false, false);
		break;
	case STATE_HERO_SLIDE_SLIP_B:
		StateHeroSlideSlip(0xe3, true, true);
		break;
	case STATE_HERO_SLIDE_SLIP_C:
		StateHeroSlideSlip(STATE_HERO_STAND, false, true);
		break;
	default:
		assert(false);
		break;
	}
}

void CActorHeroPrivate::StateHeroStandInit(int bCheckEffort)
{
	this->bFacingControlDirection = 1;
	this->field_0xff4 = 0;

	if (bCheckEffort != 0) {
		if (5.0f < this->effort) {
			PlayAnim(0xf0);
		}
		else {
			this->effort = 0.0f;
		}
	}
	return;
}

void CActorHeroPrivate::StateHeroStandTerm()
{
	if (0xea < this->animIdleSequenceIndex) {
		this->animIdleSequenceIndex = 0xe6;
	}
	return;
}

void CActorHeroPrivate::StateHeroStand(int bCheckEffort)
{
	char* pcVar1;
	int iVar2;
	CPlayerInput* pInput;
	int iVar4;
	uint uVar5;
	edF32VECTOR4* peVar6;
	AnimResult* pAVar7;
	Timer* pTVar8;
	int iVar9;
	bool bSuccess;
	long lVar10;
	ulong uVar11;
	undefined4 uVar12;
	int regularAnimType;
	float fVar13;
	float fVar14;
	float in_f21;
	float unaff_f20;

	pcVar1 = this->field_0xf24;
	iVar9 = (int)this->pCollisionData;
	CAnimation* pAnimController = this->pAnimationController;
	if (pcVar1 != (char*)0x0) {
		IMPLEMENTATION_GUARD(
		param_2 = 0;
		CActor::SV_UpdateOrientationToPosition2D(this->field_0x1040 * 0.5, (CActor*)this, pcVar1 + 0x30);
		this->bFacingControlDirection = 1;
		lVar10 = (*(code*)(this->pVTable)->IsLookingAt)(this);
		if ((lVar10 != 0) && (gPlayerInput.aButtons[4].clickValue != 0.0)) {
			fVar14 = gPlayerInput.field_0x5f4;
			if (gPlayerInput.field_0x5fc * 0.7853982 < ABS(gPlayerInput.field_0x5f8)) {
				fVar14 = 0.0;
			}
			fVar13 = gPlayerInput.field_0x5f8;
			if (gPlayerInput.field_0x5fc * 0.7853982 < ABS(fVar14)) {
				fVar13 = 0.0;
			}
			fVar13 = edFIntervalDotSrcLERP(fVar13, 0.5235988, -0.4363323);
			(**(code**)((int)(this->pVTable + 1) + 0x24))
				(fVar13 - 0.1745329, -(fVar14 * 50.0 * 0.01745329), 0x41490fdb, this);
		})
	}

	if (bCheckEffort != 0) {
		iVar4 = this->currentAnimType;
		if (iVar4 == 0xf0) {
			SV_UpdateValue(0.0f, 5.0f, &this->effort);
			if (this->effort == 0.0f) {
				edAnmLayer* peVar3 = (pAnimController->anmBinMetaAnimator).aAnimData;
				if ((peVar3->currentAnimDesc).animType == pAnimController->currentAnimType_0x30) {
					bSuccess = false;
					if (peVar3->animPlayState != 0) {
						bSuccess = (peVar3->field_0xcc & 2) != 0;
					}
				}
				else {
					bSuccess = false;
				}

				if (bSuccess) {
					PlayAnim(0xf1);
				}
			}

			bCheckEffort = 0;
		}
		else {
			if (iVar4 == 0xf1) {
				edAnmLayer* peVar3 = (pAnimController->anmBinMetaAnimator).aAnimData;
				if ((peVar3->currentAnimDesc).animType == pAnimController->currentAnimType_0x30) {
					bSuccess = false;
					if (peVar3->animPlayState != 0) {
						bSuccess = (peVar3->field_0xcc & 2) != 0;
					}
				}
				else {
					bSuccess = false;
				}
				if (bSuccess) {
					AnimResult* pAnimResult = GetStateCfg(this->actorState);
					PlayAnim(pAnimResult->animId);
				}
				bCheckEffort = 0;
			}
		}
	}

	this->dynamic.rotationQuat = this->rotationQuat;

	SV_MOV_UpdateSpeedIntensity(0.0f, this->field_0x105c);

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;

	this->dynamicExt.field_0x6c = 0.0f;

	fVar14 = ManageDyn(4.0f, 0x1002023b, 0);

	iVar4 = 0; // DetectGripablePrecipice(this);
	if (iVar4 == 0) {
		IMPLEMENTATION_GUARD_LOG(
		if ((*(byte*)(iVar9 + 4) & 2) == 0) {
			fVar14 = this->field_0x1184;
			if (fVar14 < this->timeInAir) {
				if ((*(byte*)((int)this->pCollisionData + 4) & 2) == 0) {
					uVar12 = STATE_HERO_FALL_A;
					if (this->field_0x142c != 0) {
						if (this->distanceToGround < 10.3) {
							uVar12 = STATE_HERO_FALL_A;
							this->field_0x1020 = 1;
						}
						else {
							uVar12 = 0xf0;
							this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
						}
					}
				}
				else {
					uVar12 = 0x7a;
					this->field_0x1020 = 1;
				}
				this->SetState(uVar12, 0xffffffff);
				return;
			}
		}
		else {
			this->timeInAir = 0.0f;
		})

		if (this->field_0x1574 == 0.0f) {
			pInput = this->pPlayerInput;

			if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				/* No input manager so jump key can't be pressed */
				uVar5 = 0;
			}
			else {
				/* Check with the input manager if the jump key is pressed */
				uVar5 = pInput->pressedBitfield & 0x10;
			}
			if (uVar5 == 0) {
				uVar11 = CanEnterToboggan();

				if (uVar11 == 0) {
					if ((this->dynamic.flags & 2) == 0) {
						pInput = this->pPlayerInput;

						if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar14 = 0.0f;
						}
						else {
							fVar14 = pInput->aButtons[5].clickValue;
						}

						if (fVar14 == 0.0f) {							
							pInput = this->pPlayerInput;

							if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
								fVar14 = 0.0f;
							}
							else {
								fVar14 = pInput->aAnalogSticks[0].magnitude;
							}

							if ((0.3f < fVar14) || (this->bFacingControlDirection == 0)) {

								pInput = this->pPlayerInput;
								if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar14 = 0.0;
								}
								else {
									fVar14 = pInput->aAnalogSticks[0].magnitude;
								}

								if (0.3f < fVar14) {
									/* Moving */
									this->bFacingControlDirection = 0;

									pInput = this->pPlayerInput;
									if ((pInput == (CPlayerInput*)0x0) ||
										(peVar6 = &pInput->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->controlDirection.x = peVar6->x;

									pInput = this->pPlayerInput;
									if ((pInput == (CPlayerInput*)0x0) ||
										(peVar6 = &pInput->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->controlDirection.y = peVar6->y;

									pInput = this->pPlayerInput;
									if ((pInput == (CPlayerInput*)0x0) ||
										(peVar6 = &pInput->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->controlDirection.z = peVar6->z;

									pInput = this->pPlayerInput;
									if ((pInput == (CPlayerInput*)0x0) ||
										(peVar6 = &pInput->lAnalogStick, this->field_0x18dc != 0)) {
										peVar6 = &gF32Vector4Zero;
									}
									this->controlDirection.w = peVar6->w;

									if (0.0f <= this->controlDirection.x * this->rotationQuat.z - this->rotationQuat.x * this->controlDirection.z) {
										PlayAnim(0xf6);
									}
									else {
										PlayAnim(0xf5);
									}

									if (0xea < this->animIdleSequenceIndex) {
										this->animIdleSequenceIndex = 0xe6;
									}
								}

								/* Moving */
								bSuccess = SV_UpdateOrientation2D(18.84f, &this->controlDirection, 0);
								this->bFacingControlDirection = (int)bSuccess;

								ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroStand Moving to face control direction: {} is facing: {}", this->controlDirection.ToString(), bSuccess);

								if (this->bFacingControlDirection != 0) {
									// Play shuffle animation.
									pAVar7 = GetStateCfg(this->actorState);
									regularAnimType = pAVar7->animId;
									PlayAnim(regularAnimType);
									this->prevAnimType = regularAnimType;
								}
							}

							if (this->field_0x1a4c == 0) {						
								pInput = this->pPlayerInput;
								if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar14 = 0.0f;
								}
								else {
									fVar14 = pInput->aAnalogSticks[0].magnitude;
								}

								ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroStand field_0x18dc {} fVar14 {}", this->field_0x18dc, fVar14);

								if (0.3f < fVar14) {
									pInput = this->pPlayerInput;
									if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
										fVar14 = 0.0f;
									}
									else {
										fVar14 = pInput->aAnalogSticks[0].magnitude;
									}

									if (fVar14 < 0.9f) {
										SV_UpdatePercent(0.0f, 0.9f, &this->field_0x1048);
										pTVar8 = GetTimer();
										this->time_0x153c = pTVar8->scaledTotalTime;
										ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroStand Reset run timer A");
										pTVar8 = GetTimer();
										if ((0.1f < pTVar8->scaledTotalTime - this->time_0x1538) && (this->bFacingControlDirection != 0))
										{
											this->SetState(0x77, 0xffffffff);
										}
									}
									else {
										pInput = this->pPlayerInput;
										if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
											fVar14 = 0.0f;
										}
										else {
											fVar14 = pInput->aAnalogSticks[0].magnitude;
										}

										if (0.9f < fVar14) {
											SV_UpdatePercent(1.0f, 0.9f, &this->field_0x1048);
											pTVar8 = GetTimer();

											ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroStand total time: {} saved time: {} is facing: {}", pTVar8->scaledTotalTime, this->time_0x153c, this->bFacingControlDirection);

											if ((0.1f < pTVar8->scaledTotalTime - this->time_0x153c) && (this->bFacingControlDirection != 0)) {
												ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroStand transitioning to StateHeroRun");
												this->SetState(STATE_HERO_RUN, 0xf3);
											}
										}
										else {
											pTVar8 = GetTimer();
											this->time_0x153c = pTVar8->scaledTotalTime;
											ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroStand Reset run timer B");
										}
									}
								}
								else {							
									pTVar8 = GetTimer();
									this->time_0x153c = pTVar8->scaledTotalTime;
									ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroStand Reset run timer C");
									this->field_0x1048 = 0.0f;
									if (((bCheckEffort == 0) || (this->bFacingControlDirection == 0)) || (this->boomyState_0x1b70 != 0))
									{
										this->idleTimer = 0.0f;
									}
									else {
										edAnmLayer* peVar5 = (pAnimController->anmBinMetaAnimator).aAnimData;
										if ((peVar5->currentAnimDesc).animType == pAnimController->currentAnimType_0x30) {
											bSuccess = false;
											if (peVar5->animPlayState != 0) {
												bSuccess = (peVar5->field_0xcc & 2) != 0;
											}
										}
										else {
											bSuccess = false;
										}
										if (bSuccess) {
											/* Idle animation trigger */
											regularAnimType = -1;

											if (this->field_0xff4 == 0) {
												fVar14 = 4.0f;
											}
											else {
												iVar9 = rand();
												fVar14 = ((float)iVar9 / 2.147484e+09f) * 2.4f + 0.9f;
											}

											switch (this->animIdleSequenceIndex) {
											case 0xe6:
												in_f21 = 1.0f;
												unaff_f20 = 1.8f;
												break;
											case 0xe7:
												in_f21 = 0.8f;
												unaff_f20 = 1.2f;
												break;
											case 0xe8:
												in_f21 = 1.0f;
												unaff_f20 = 2.0f;
												break;
											case 0xe9:
												in_f21 = 0.8f;
												unaff_f20 = 1.2f;
												break;
											case 0xea:
												in_f21 = 0.8f;
												unaff_f20 = 1.2f;
												regularAnimType = 0xf2;
												break;
											case 0xeb:
												in_f21 = 0.8f;
												unaff_f20 = 1.2f;
												break;
											case 0xec:
												in_f21 = 0.8f;
												unaff_f20 = 1.2f;
												break;
											case 0xed:
												in_f21 = 1.0f;
												unaff_f20 = 1.8f;
												break;
											case 0xee:
												in_f21 = 0.8f;
												unaff_f20 = 1.2f;
												break;
											case 0xef:
												in_f21 = 0.8f;
												regularAnimType = 0;
												unaff_f20 = 1.2f;
											}
											/* Start new animation */
											iVar9 = rand();
											bSuccess = PlayWaitingAnimation(fVar14, in_f21 +(unaff_f20 - in_f21) * ((float)iVar9 / 2.147484e+09f), this->animIdleSequenceIndex, regularAnimType, 1);
											if (bSuccess != false) {
												/* Increment the special idle so we get a new one next time */
												this->field_0xff4 = 1;
												this->animIdleSequenceIndex = this->animIdleSequenceIndex + 1;
												if (this->animIdleSequenceIndex == 0xf0) {
													this->animIdleSequenceIndex = 0xe6;
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
				/* Jumping */
				CCollision* pCVar2 = this->pCollisionData;
				fVar13 = this->runSpeed;
				if ((this->dynamic.flags & 2) != 0) {
					IMPLEMENTATION_GUARD(
					fVar14 = CCollisionManager::GetWallNormalYLimit(pCVar2->aCollisionContact + 1);
					fVar13 = CCollisionManager::GetGroundSpeedDecreaseNormalYLimit(pCVar2->aCollisionContact + 1);
					fVar13 = edFIntervalLERP(pCVar2->aCollisionContact[1].location.y, fVar13, fVar14, 1.0f, 0.3f);
					fVar13 = this->field_0x1134 * fVar13;)
				}
				/* Jumping */
				SetJumpCfg(0.1f, fVar13, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
				/* Jump animation */
				this->SetState(0x78, 0xffffffff);
			}
		}
	}
	else {
		this->SetState(200, 0xffffffff);
	}
	return;
}

void CActorHeroPrivate::StateHeroRun()
{
	CCollision* pCVar1;
	int iVar2;
	CPlayerInput* pInput;
	CAnimation* pCVar4;
	edAnmLayer* peVar5;
	bool bVar6;
	AnimResult* pAVar7;
	uint uVar8;
	Timer* pTVar9;
	edF32VECTOR4* peVar10;
	undefined4 uVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	//CCollisionRay CStack128;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 slidingForce;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	//_ray_info_out local_10;

	pCVar1 = this->pCollisionData;
	iVar2 = LevelScheduleManager::gThis->currentLevelID;

	IncreaseEffort(1.0f);

	if (((iVar2 == 0xd) && (this->pSoccerActor != (CActor*)0x0)) && (this->dynamic.weightB == 60.0f)) {
		IMPLEMENTATION_GUARD(
		fVar14 = this->field_0x105c;
		fVar13 = this->field_0x1074;
		fVar16 = this->field_0x1040;
		fVar15 = this->field_0x1054;
		fVar18 = this->field_0x1058;
		fVar17 = this->runSpeed * 0.9f;
		ComputeSlidingForce(&eStack64, 0);
		edF32Vector4ScaleHard(this->field_0x107c, &eStack64, &eStack64);
		peVar10 = &this->dynamicExt.velocity;
		edF32Vector4AddHard(peVar10, peVar10, &eStack64);
		fVar12 = edF32Vector4GetDistHard(&this->dynamicExt.velocity);
		this->dynamicExt.field_0x10[0] = fVar12;
		pInput = this->pPlayerInput;

		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar12 = 0.0f;
		}
		else {
			fVar12 = pInput->aAnalogSticks[0].magnitude;
		}

		if ((0.3f <= fVar12) || (0.1f <= this->dynamic.speed)) {
			BuildHorizontalSpeedVector(fVar17, fVar18, fVar15, fVar16, fVar13, this);
		}
		else {
			CActorMovable::SV_MOV_UpdateSpeedIntensity(0.0f, fVar14, (CActorMovable*)this);
		}

		fVar12 = ManageDyn(4.0f, 0x1002023b, 0);
		)
	}
	else {
		fVar14 = this->field_0x105c;
		fVar13 = this->field_0x1074;
		fVar16 = this->field_0x1040;
		fVar15 = this->field_0x1054;
		fVar18 = this->field_0x1058;
		fVar17 = this->runSpeed;

		ComputeSlidingForce(&slidingForce, 0);
		edF32Vector4ScaleHard(this->field_0x107c, &slidingForce, &slidingForce);
		edF32Vector4AddHard(&this->dynamicExt.aVelocity[0], &this->dynamicExt.aVelocity[0], &slidingForce);
		this->dynamicExt.aVelocityMagnitudes[0] = edF32Vector4GetDistHard(&this->dynamicExt.aVelocity[0]);

		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar12 = 0.0f;
		}
		else {
			fVar12 = pInput->aAnalogSticks[0].magnitude;
		}

		if ((0.3f <= fVar12) || (0.1f <= this->dynamic.speed)) {
			BuildHorizontalSpeedVector(fVar17, fVar18, fVar15, fVar16, fVar13);
		}
		else {
			SV_MOV_UpdateSpeedIntensity(0.0f, fVar14);
		}

		fVar12 = ManageDyn(4.0f, 0x1002023b, 0);
	}

	CActor* pSoccer = this->pSoccerActor;
	if ((pSoccer == (CActor*)0x0) || (pSoccer->currentAnimType != 9)) {
		pCVar4 = this->pAnimationController;
		peVar5 = (pCVar4->anmBinMetaAnimator).aAnimData;
		if ((peVar5->currentAnimDesc).animType == pCVar4->currentAnimType_0x30) {
			bVar6 = false;
			if (peVar5->animPlayState != 0) {
				bVar6 = (peVar5->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar6 = false;
		}
		if (bVar6) {
			pAVar7 = GetStateCfg(this->actorState);
			PlayAnim(pAVar7->animId);
		}
	}
	else {
		this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0);
		if ((0.26f < fVar12) && this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0), fVar12 < 0.3f) {
			PlayAnim(0x116);
		}
	}
	if ((this->dynamic.flags & 2) == 0) {
		fVar12 = edFIntervalUnitDstLERP(this->dynamic.linearJerk, this->field_0x104c, this->runSpeed);
		SV_UpdatePercent(fVar12, 0.9f, &this->field_0x1048);
	}
	else {
		pInput = this->pPlayerInput;

		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar12 = 0.0f;
		}
		else {
			fVar12 = pInput->aAnalogSticks[0].magnitude;
		}

		fVar12 = edFIntervalUnitDstLERP(fVar12, 0.3f, 1.0f);
		SV_UpdatePercent(fVar12, 0.9f, &this->field_0x1048);
	}

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar12 = 0.0f;
	}
	else {
		fVar12 = pInput->aAnalogSticks[0].magnitude;
	}

	fVar13 = 0.9f;

	IMPLEMENTATION_GUARD_LOG(
	if ((fVar12 < 0.9f) && (bVar6 = DetectGripablePrecipice(this), bVar6 != false)) {
		SetState(200, 0xffffffff);
		return;
	})

	if ((pCVar1->flags_0x4 & 2) == 0) {
		fVar13 = this->field_0x1184;
		if (fVar13 < this->timeInAir) {
			if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
				uVar11 = STATE_HERO_FALL_A;
				if (this->field_0x142c != 0) {
					if (this->distanceToGround < 10.3f) {
						uVar11 = STATE_HERO_FALL_A;
						this->field_0x1020 = 1;
					}
					else {
						uVar11 = 0xf0;
						this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
					}
				}
			}
			else {
				uVar11 = 0x7a;
				this->field_0x1020 = 1;
			}
			/* Fall */
			SetState(uVar11, 0xffffffff);
			return;
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		uVar8 = 0;
	}
	else {
		uVar8 = pInput->pressedBitfield & 0x10;
	}

	if (uVar8 != 0) {

		/* Jump */
		pCVar1 = this->pCollisionData;
		fVar12 = this->runSpeed;
		if ((this->dynamic.flags & 2) != 0) {
			IMPLEMENTATION_GUARD(
			FUN_00211550((int)(pCVar1->aCollisionContact + 1));
			fVar12 = fVar13;
			FUN_00211590((int)(pCVar1->aCollisionContact + 1));
			fVar12 = edFIntervalLERP(pCVar1->aCollisionContact[1].location.y, fVar12, fVar13, 1.0, 0.3);
			fVar12 = *(float*)&this->field_0x1134 * fVar12;)
		}

		SetJumpCfg(0.1f, fVar12, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
		SetState(STATE_HERO_JUMP_1_1_RUN, 0xffffffff);
		return;
	}

	bVar6 = CanEnterToboggan();
	if (bVar6 != false) {
		SetState(0xe8, 0xffffffff);
		return;
	}

	uVar8 = this->dynamic.flags;
	if ((uVar8 & 2) == 0) {
	LAB_00145210:
		pInput = this->pPlayerInput;
		if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar12 = 0.0f;
		}
		else {
			fVar12 = pInput->aButtons[5].clickValue;
		}

		if (fVar12 == 0.0f) {
			if (this->runSpeed - this->field_0x1068 < this->field_0xa84) {
				if ((this->dynamic.flags & 2) == 0) {
					pInput = this->pPlayerInput;
					if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar12 = 0.0f;
					}
					else {
						fVar12 = pInput->GetAngleWithPlayerStick(&this->dynamic.velocityDirectionEuler);
					}
					if (this->field_0x1074 < fabs(fVar12)) {
						/* Full speed stop */
						SetState(0xe2, 0xffffffff);
						return;
					}
				}

				if (((this->pSoccerActor == (CActor*)0x0) && ((pCVar1->flags_0x4 & 1) != 0)) && (this->field_0x1454 == false)) {
					IMPLEMENTATION_GUARD_LOG(
					edF32Vector4ScaleHard((float)&DAT_3f99999a, &eStack48, &g_xVector);
					fVar12 = ((this->pCollisionData)->pObbPrim->field_0x90).z;
					edF32Vector4AddHard(&eStack96, &this->currentLocation, &eStack48);
					CCollisionRay::CCollisionRay
					(fVar12 + 0.5, &CStack128, &eStack96, &this->rotationQuat);
					fVar12 = CCollisionRay::Intersect(&CStack128, 3, (CActor*)this, (CActor*)0x0, 0x40001000, &eStack32, &local_10);
					if ((fVar12 != 1e+30f) &&
						(fVar13 = this->field_0x1430,
							fVar12 = edF32Vector4DotProductHard(&this->rotationQuat, &eStack32)
							, fVar12 + 1.0 <= edFCosinus[(int)(ABS(fVar13 * 1303.797) + 0.5) & 0x1fff])) {
						SetState(0x96, 0xffffffff);
						return;
					})
				}
			}

			pInput = this->pPlayerInput;
			if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar12 = 0.0f;
			}
			else {
				fVar12 = pInput->aAnalogSticks[0].magnitude;
			}

			if (fVar12 < 0.9f) {
				pInput = this->pPlayerInput;
				if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar12 = 0.0f;
				}
				else {
					fVar12 = pInput->aAnalogSticks[0].magnitude;
				}
				if (fVar12 < 0.3) {
					/* If our speed? is under a certain value when we let go */
					pTVar9 = GetTimer();
					this->time_0x1538 = pTVar9->scaledTotalTime;
					pTVar9 = GetTimer();
					if (0.05f < pTVar9->scaledTotalTime - this->time_0x153c) {
						/* Return to idle */
						SetState(0x73, 0xffffffff);
					}
				}
				else {
					pTVar9 = GetTimer();
					this->time_0x153c = pTVar9->scaledTotalTime;
					pTVar9 = GetTimer();
					if (0.05f < pTVar9->scaledTotalTime - this->time_0x1538) {
						SetState(0x77, 0xffffffff);
					}
				}
			}
			else {
				pTVar9 = GetTimer();
				this->time_0x1538 = pTVar9->scaledTotalTime;
				pTVar9 = GetTimer();
				this->time_0x153c = pTVar9->scaledTotalTime;
			}
		}
		else {
			SetState(0x8a, 0xffffffff);
		}
	}
	else {
		if ((uVar8 & 1) == 0) {
			pInput = this->pPlayerInput;
			if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar12 = 0.0f;
			}
			else {
				fVar12 = pInput->aAnalogSticks[0].magnitude;
			}
			if (0.3f <= fVar12) goto LAB_00145210;
		}

		SetState(0xe4, 0xffffffff);
	}
	return;
}

void CActorHeroPrivate::StateHeroSlideSlipInit()
{
	ConvertSpeedSumForceExtToSpeedPlayer2D();
	this->slideSlipIntensity = this->dynamic.speed / 0.2;
}

void CActorHeroPrivate::StateHeroSlideSlip(int nextState, bool boolA, bool boolB)

{
	CCollision* pCVar1;
	CPlayerInput* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	bool bVar5;
	bool bVar6;
	undefined4 uVar7;
	CBehaviour* pCVar8;
	uint uVar9;
	int iVar10;
	float fVar11;
	CActorsTable local_110;

	pCVar1 = this->pCollisionData;
	bVar5 = false;
	SV_MOV_DecreaseSpeedIntensity(this->slideSlipIntensity);
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;
	local_110.entryCount = 0;
	ManageDyn(4.0f, 0x1002023b, &local_110);
	iVar10 = 0;
	do {
		if (local_110.entryCount <= iVar10) {
		LAB_0032b168:
			if (boolA == false) {
				if (boolB != false) {
					pCVar2 = this->pPlayerInput;
					if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar11 = 0.0f;
					}
					else {
						fVar11 = pCVar2->aAnalogSticks[0].magnitude;
					}
					if (0.3f < fVar11) {
						this->field_0x1048 = 0.0f;
						pCVar2 = this->pPlayerInput;
						if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar11 = 0.0f;
						}
						else {
							fVar11 = pCVar2->aAnalogSticks[0].magnitude;
						}
						if (0.9f <= fVar11) {
							SetState(0x76, 0xf3);
							return;
						}
						SetState(0x77, 0xffffffff);
						return;
					}
				}
			}
			else {
				pCVar2 = this->pPlayerInput;
				if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pCVar2->GetAngleWithPlayerStick(&this->dynamic.velocityDirectionEuler);
				}
				if (fabs(fVar11) <= this->field_0x1074) {
					pCVar2 = this->pPlayerInput;
					if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar11 = 0.0f;
					}
					else {
						fVar11 = pCVar2->aAnalogSticks[0].magnitude;
					}
					if (0.3f < fVar11) {
						this->field_0x1048 = 0.0f;
						pCVar2 = this->pPlayerInput;
						if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar11 = 0.0f;
						}
						else {
							fVar11 = pCVar2->aAnalogSticks[0].magnitude;
						}
						if (0.9f <= fVar11) {
							/* Quick transition to new move state */
							SetState(0x76, 0xf3);
							return;
						}
						SetState(0x77, 0xffffffff);
						return;
					}
				}
				pCVar2 = this->pPlayerInput;
				if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pCVar2->aAnalogSticks[0].magnitude;
				}
				if (((0.3 < fVar11) && (this->field_0xa84 < 4.5f)) &&
					(fVar11 = edF32Vector4DotProductHard
					(&this->rotationQuat,
						&this->dynamic.velocityDirectionEuler), 0.0f < fVar11)) {
					/* Do a 180 degree turn */
					SetState(0xe7, 0xffffffff);
					return;
				}
			}
			if (((pCVar1->flags_0x4 & 1) != 0) || (bVar5)) {
				SetState(0x73, 0xffffffff);
			}
			else {
				bVar5 = false;// DetectGripablePrecipice(this);
				if (bVar5 == false) {
					if ((pCVar1->flags_0x4 & 2) == 0) {
						if (this->field_0x1184 < this->timeInAir) {
							uVar7 = ChooseStateFall(0);
							SetState(uVar7, 0xffffffff);
							return;
						}
					}
					else {
						this->timeInAir = 0.0f;
					}
					bVar6 = ColWithAToboggan();
					bVar5 = false;
					if (bVar6 != false) {
						pCVar8 = GetBehaviour(0xb);
						bVar5 = true;
						if (pCVar8 == (CBehaviour*)0x0) {
							pCVar8 = GetBehaviour(10);
							bVar5 = true;
							if (pCVar8 == (CBehaviour*)0x0) {
								pCVar8 = GetBehaviour(9);
								bVar5 = true;
								if (pCVar8 == (CBehaviour*)0x0) {
									bVar5 = false;
								}
							}
						}
					}
					if (bVar5) {
						SetState(0xe8, 0xffffffff);
					}
					else {
						if ((this->dynamic.flags & 2) == 0) {
							pCVar2 = this->pPlayerInput;
							if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
								uVar9 = 0;
							}
							else {
								uVar9 = pCVar2->pressedBitfield & 0x10;
							}
							if (uVar9 == 0) {
								pCVar2 = this->pPlayerInput;
								if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar11 = 0.0f;
								}
								else {
									fVar11 = pCVar2->aButtons[5].clickValue;
								}
								if (fVar11 == 0.0f) {
									pCVar3 = this->pAnimationController;
									peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
									if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
										bVar5 = false;
										if (peVar4->animPlayState != 0) {
											bVar5 = (peVar4->field_0xcc & 2) != 0;
										}
									}
									else {
										bVar5 = false;
									}
									if (bVar5) {
										this->dynamic.speed = 0.0f;
										SetState(nextState, 0xffffffff);
									}
								}
								else {
									pCVar2 = this->pPlayerInput;
									if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
										fVar11 = 0.0;
									}
									else {
										fVar11 = pCVar2->aAnalogSticks[0].magnitude;
									}
									if (0.3f < fVar11) {
										SetState(0x8a, 0xffffffff);
									}
									else {
										SetState(0x85, 0xffffffff);
									}
								}
							}
							else {
								IMPLEMENTATION_GUARD(
								SetJumpCfgFromGround(this->runSpeed);)
								SetState(0x7b, 0xffffffff);
							}
						}
						else {
							SetState(0xe4, 0xffffffff);
						}
					}
				}
				else {
					SetState(200, 0xffffffff);
				}
			}
			return;
		}
		if (local_110.aEntries[iVar10]->typeID != 0xd) {
			bVar5 = true;
			goto LAB_0032b168;
		}
		iVar10 = iVar10 + 1;
	} while (true);
}

void CActorHeroPrivate::StateHeroSlideInit(int param_2)
{
	CCollision* pCVar1;
	int iVar2;
	AnimResult* pAVar3;
	uint uVar4;
	edF32VECTOR4* peVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;

	this->field_0xf00 = this->currentLocation;
	this->field_0x10b0 = g_xVector;
	this->field_0x10a0 = this->rotationQuat;

	fVar6 = g_xVector.z;
	fVar7 = g_xVector.x;

	if (((pCVar1->flags_0x4 & 2) != 0) && (pCVar1->aCollisionContact[1].location.y < 0.999f)) {
		fVar6 = pCVar1->aCollisionContact[1].location.z;
		fVar7 = pCVar1->aCollisionContact[1].location.x;
	}
	if (0.0f < this->rotationQuat.z * fVar6 + this->rotationQuat.x * fVar7) {
		this->field_0x1094 = 1;
	}
	else {
		this->field_0x1094 = 0;
	}

	this->flags = this->flags | 0x1000;

	iVar2 = this->prevActorState;
	if (iVar2 == -1) {
		uVar4 = 0;
	}
	else {
		pAVar3 = GetStateCfg(iVar2);
		uVar4 = pAVar3->flags_0x4 & 0x100;
	}

	if (((((uVar4 != 0) && (iVar2 = this->prevActorState, iVar2 != 0x7a)) &&
		(iVar2 != 0x7d)) || (param_2 != 0)) && ((this->dynamic.linearJerk < this->field_0x104c && (0.0f < this->dynamic.velocityDirectionEuler.y)))) {
		if (param_2 == 0) {
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;

			this->dynamicExt.field_0x6c = 0.0f;

			ComputeSlidingForce(&eStack32, 0);
			edF32Vector4ScaleHard(this->field_0x1078 * 10.0f, &eStack32, &eStack32);

			peVar5 = this->dynamicExt.aVelocity;
			edF32Vector4AddHard(peVar5, peVar5, &eStack32);
			fVar6 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
			this->dynamicExt.aVelocityMagnitudes[0] = fVar6;
		}
		else {
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;
			ComputeSlidingForce(&eStack16, 0);
			edF32Vector4ScaleHard(this->field_0x1078 * 25.0f, &eStack16, &eStack16);

			peVar5 = this->dynamicExt.aVelocity;
			edF32Vector4AddHard(peVar5, peVar5, &eStack16);
			fVar6 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
			this->dynamicExt.aVelocityMagnitudes[0] = fVar6;
		}
	}
	if (param_2 != 0) {
		ChangeCollisionSphereForLying(0.2f);
	}
	return;
}

void CActorHeroPrivate::StateHeroJump_1_3(int nextState)
{
	CPlayerInput* pCVar1;
	edF32VECTOR4* v0;
	float z;
	float w;
	float y;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 eStack16;

	y = this->field_0x105c;
	w = this->field_0x1074;
	fVar3 = this->field_0x1040;
	fVar2 = this->field_0x1054;
	fVar5 = this->field_0x1058;
	fVar4 = this->airHorizontalSpeed;
	ComputeSlidingForce(&eStack16, 0);
	edF32Vector4ScaleHard(this->field_0x107c, &eStack16, &eStack16);
	v0 = this->dynamicExt.aVelocity;
	edF32Vector4AddHard(v0, v0, &eStack16);
	z = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
	this->dynamicExt.aVelocityMagnitudes[0] = z;

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		z = 0.0f;
	}
	else {
		z = pCVar1->aAnalogSticks[0].magnitude;
	}
	if ((0.3f <= z) || (0.1f <= this->dynamic.speed)) {
		BuildHorizontalSpeedVector(fVar4, fVar5, fVar2, fVar3, w);
	}
	else {
		SV_MOV_UpdateSpeedIntensity(0.0f, y);
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	this->field_0xf00 = this->currentLocation;

	/* Transition to new state */
	SetState(nextState, 0xffffffff);
	return;
}

void CActorHeroPrivate::StateHeroJump_2_3Init()
{
	this->scalarDynJump.BuildFromSpeedDist(this->jmp_field_0x113c, 0.0f, this->jmp_field_0x1140);
	this->field_0x116c = 1.0f;
	this->field_0x1170 = 1.0f;
	this->field_0x1455 = 0;
	this->field_0x1168 = 0;
	this->field_0xf58 = this->field_0xf54;
	return;
}

void CActorHeroPrivate::StateHeroJump_2_3(int param_2, int bCheckBounce, int param_4)
{
	CCollision* pCVar1;
	CPlayerInput* pCVar2;
	CActor* pCVar3;
	bool bVar4;
	uint jumpPressed;
	Timer* pTVar5;
	int iVar6;
	edF32VECTOR4* v1;
	undefined4 uVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	undefined4 local_c;
	undefined4 local_8;
	undefined4 local_4;

	pCVar1 = this->pCollisionData;

	IncreaseEffort(2.0f);

	pCVar2 = this->pPlayerInput;
	if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		jumpPressed = 0;
	}
	else {
		jumpPressed = pCVar2->pressedBitfield & 0x10;
	}

	if (jumpPressed != 0) {
		this->field_0x1168 = 1;
		this->jmp_field_0x1144 = 0;
	}

	if ((this->jmp_field_0x1144 != 0) && (3.0 < this->scalarDynJump.field_0x20)) {
		pCVar2 = this->pPlayerInput;
		if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar8 = 1000000.0f;
		}
		else {
			fVar8 = pCVar2->aButtons[4].releasedDuration;
		}

		IMPLEMENTATION_GUARD(
		if (fVar8 < this->field_0x115c) {
			if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar8 = 0.0;
			}
			else {
				fVar8 = pCVar2->aButtons[4].clickValue;
			}
			if (fVar8 == 0.0) {
				this->field_0x1170 = 2.0;
			}
		})
	}

	if ((0.0f < this->jmp_field_0x1140) && (bVar4 = this->scalarDynJump.IsFinished(), bVar4 == false)) {
		pTVar5 = GetTimer();
		fVar9 = pTVar5->cutsceneDeltaTime;
		pTVar5 = GetTimer();
		fVar8 = this->field_0x1170;
		this->scalarDynJump.Integrate(fVar8 * this->field_0x116c * pTVar5->cutsceneDeltaTime, fVar8 * fVar9);
		this->dynamicExt.field_0x60.y = this->scalarDynJump.field_0x20;
		this->dynamic.field_0x4c = this->dynamic.field_0x4c | 0x8000;
	}

	MoveInAir(this->airMinSpeed, this->airHorizontalSpeed, this->airControlSpeed, this->airNoInputSpeed, this->airRotationRate);

	ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

	if ((pCVar1->flags_0x4 & 4) != 0) {
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
		this->jmp_field_0x1140 = 0.0f;

		ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroJump_2_3 Reset normalizedTranslation: {}", this->dynamicExt.normalizedTranslation.ToString());
	}
	if (this->field_0x1a10 == 0xd) {
		IMPLEMENTATION_GUARD(
		if (param_4 == 0) {
			AccomplishAction(this, 0);
			return;
		}
		if (this->dynamic.linearAcceleration *
			this->dynamic.velocityDirectionEuler.y < -1.0) {
			pCVar3 = this->field_0xf58;
			if (pCVar3 == (CActor*)0x0) {
				AccomplishAction(this, 0);
				return;
			}
			edF32Vector4SubHard(&eStack32, &pCVar3->currentLocation,
				&this->currentLocation);
			eStack32.y = 0.0;
			edF32Vector4SafeNormalize0Hard(&eStack32, &eStack32);
			pCVar2 = this->pPlayerInput;
			if ((pCVar2 == (CPlayerInput*)0x0) || (v1 = &pCVar2->lAnalogStick, this->field_0x18dc != 0)) {
				v1 = &gF32Vector4Zero;
			}
			fVar8 = edF32Vector4DotProductHard(&eStack32, v1);
			if (-0.5 <= fVar8) {
				AccomplishAction(this, 0);
				return;
			}
		})
	}

	if ((((this->field_0xf54 == (CActor*)0x0) || (param_4 == 0)) ||
		(-1.0f <= this->dynamic.linearAcceleration *
			this->dynamic.velocityDirectionEuler.y)) ||
		(this->field_0xf54->SV_GetBoneDefaultWorldPosition(this->field_0x15a0, &eStack48),
			eStack48.y <= this->currentLocation.y)) {
		if ((pCVar1->flags_0x4 & 2) == 0) {
			jumpPressed = CanGrip(param_2, &this->rotationQuat);
			if (jumpPressed == 0) {
				if (bCheckBounce != 0) {
					iVar6 = CanBounceAgainstWall();
					if (iVar6 != 0) {
						IMPLEMENTATION_GUARD(
						edF32Vector4SubHard(&eStack64, &this->currentLocation,
							&this->field_0xf00);
						fVar8 = edF32Vector4DotProductHard(&eStack64, (edF32VECTOR4*)&this->field_0xa90);
						if (fVar8 < -0.25) {
							pCVar3 = *(CActor**)&this->field_0xf18;
							if (pCVar3 != (CActor*)0x0) {
								local_8 = 1;
								CActor::DoMessage((CActor*)this, pCVar3, 0x1f, 1);
								*(undefined4*)&this->field_0xf18 = 0;
							}
							SetState(0xcb, 0xffffffff);
							return;
						})
					}
				}

				if (0.7f < this->timeInAir) {
					/* Land on uncertain ground */
					if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
						uVar7 = STATE_HERO_FALL_A;

						if (this->field_0x142c != 0) {
							if (this->distanceToGround < 10.3f) {
								uVar7 = STATE_HERO_FALL_A;
								this->field_0x1020 = 1;
							}
							else {
								uVar7 = 0xf0;
								this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
							}
						}
					}
					else {
						uVar7 = 0x7a;
						this->field_0x1020 = 1;
					}

					SetState(uVar7, 0xffffffff);
				}
			}
			else {
				pCVar3 = this->field_0xf18;

				if (pCVar3 != (CActor*)0x0) {
					local_4 = 1;
					DoMessage(pCVar3, (ACTOR_MESSAGE)0x1f, (MSG_PARAM)1);
					this->field_0xf18 = (CActor*)0x0;
				}

				SetGripState();
			}
		}
		else {
			if (0.0f < this->dynamic.velocityDirectionEuler.y) {
				this->field_0x1168 = 0;

				this->dynamicExt.normalizedTranslation.x = 0.0f;
				this->dynamicExt.normalizedTranslation.y = 0.0f;
				this->dynamicExt.normalizedTranslation.z = 0.0f;
				this->dynamicExt.normalizedTranslation.w = 0.0f;
				this->dynamicExt.field_0x6c = 0.0f;

				ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::StateHeroJump_2_3 Reset normalizedTranslation: {}", this->dynamicExt.normalizedTranslation.ToString());
			}

			pCVar3 = this->field_0xf18;
			if (pCVar3 != (CActor*)0x0) {
				IMPLEMENTATION_GUARD(
				local_c = 1;
				CActor::DoMessage((CActor*)this, pCVar3, 0x1f, 1);
				*(undefined4*)&this->field_0xf18 = 0;)
			}

			pCVar2 = this->pPlayerInput;
			if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar8 = 0.0f;
			}
			else {
				fVar8 = pCVar2->aButtons[5].clickValue;
			}
			if ((fVar8 == 0.0f) || (this->field_0x1a4c != 0)) {
				if (2.0f < this->dynamic.linearJerk) {
					SetState(0x7d, 0xffffffff);
				}
				else {
					/* Falling */
					SetState(0x7a, 0xffffffff);
				}
			}
			else {
				if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar8 = 0.0f;
				}
				else {
					fVar8 = pCVar2->aAnalogSticks[0].magnitude;
				}
				if (0.9f < fVar8) {
					SetState(0x89, 0xffffffff);
				}
				else {
					SetState(0x83, 0xffffffff);
				}
			}
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		this->field_0xf50 = (undefined*)this->field_0xf54;
		this->field_0x1a10 = 0xd;
		AccomplishAction(this, 0);)
	}
	return;
}

void CActorHeroPrivate::StateHeroJump_2_3Term()
{
	return;
}

void CActorHeroPrivate::StateHeroJump_3_3(int param_2)
{
	CCollision* pCVar1;
	CAnimation* pCVar2;
	CPlayerInput* pCVar3;
	edAnmLayer* peVar4;
	int AVar5;
	int AVar6;
	bool bVar7;
	uint uVar8;
	Timer* pTVar9;
	edF32VECTOR4* v0;
	undefined4 uVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;
	pCVar2 = this->pAnimationController;

	IncreaseEffort(1.0f);

	pCVar3 = this->pPlayerInput;
	if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar11 = 0.0f;
	}
	else {
		fVar11 = pCVar3->aAnalogSticks[0].magnitude;
	}

	if (fVar11 < 0.3f) {
		this->dynamic.speed = 0.0;
	}
	fVar13 = this->field_0x105c;
	fVar12 = this->field_0x1074;
	fVar15 = this->field_0x1040;
	fVar14 = this->field_0x1054;
	fVar17 = this->field_0x1058;
	fVar16 = this->airHorizontalSpeed;
	ComputeSlidingForce(&eStack16, 0);
	edF32Vector4ScaleHard(this->field_0x107c, &eStack16, &eStack16);
	v0 = this->dynamicExt.aVelocity;
	edF32Vector4AddHard(v0, v0, &eStack16);
	fVar11 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
	this->dynamicExt.aVelocityMagnitudes[0] = fVar11;

	pCVar3 = this->pPlayerInput;
	if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar11 = 0.0f;
	}
	else {
		fVar11 = pCVar3->aAnalogSticks[0].magnitude;
	}

	if ((0.3f <= fVar11) || (0.1f <= this->dynamic.speed)) {
		BuildHorizontalSpeedVector(fVar16, fVar17, fVar14, fVar15, fVar12);
	}
	else {
		SV_MOV_UpdateSpeedIntensity(0.0f, fVar13);
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	pCVar3 = this->pPlayerInput;
	if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		uVar8 = 0;
	}
	else {
		uVar8 = pCVar3->pressedBitfield & 0x10;
	}

	if (uVar8 != 0) {
		this->field_0x1168 = 1;
	}

	if (((this->dynamic.flags & 2) != 0) &&
		(this->airHorizontalSpeed < this->field_0x1164)) {
		this->field_0x1168 = 0;
	}

	pCVar3 = this->pPlayerInput;
	if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar11 = 1000000.0f;
	}
	else {
		fVar11 = pCVar3->aButtons[4].pressedDuration;
	}

	if (((this->field_0x1160 + 0.04f <= fVar11) ||
		(pTVar9 = GetTimer(), pTVar9->scaledTotalTime - this->time_0x1538 <= 0.04)) ||
		(this->field_0x1168 == 0)) {
		bVar7 = CanEnterToboggan();
		if (bVar7 == false) {
			if ((this->dynamic.flags & 2) == 0) {

				pCVar3 = this->pPlayerInput;
				if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar11 = 0.0f;
				}
				else {
					fVar11 = pCVar3->aAnalogSticks[0].magnitude;
				}

				if ((0.9f <= fVar11) || (bVar7 = false /*DetectGripablePrecipice(this)*/, bVar7 == false)) {
					if ((pCVar1->flags_0x4 & 2) == 0) {
						if (this->field_0x1184 < this->timeInAir) {
							if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
								uVar10 = STATE_HERO_FALL_A;
								if (this->field_0x142c != 0) {
									if (this->distanceToGround < 10.3f) {
										uVar10 = STATE_HERO_FALL_A;
										this->field_0x1020 = 1;
									}
									else {
										uVar10 = 0xf0;
										this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
									}
								}
							}
							else {
								uVar10 = 0x7a;
								this->field_0x1020 = 1;
							}
							SetState(uVar10, 0xffffffff);
							return;
						}
					}
					else {
						this->timeInAir = 0.0f;
					}

					peVar4 = (pCVar2->anmBinMetaAnimator).aAnimData;
					AVar5 = pCVar2->currentAnimType_0x30;
					AVar6 = (peVar4->currentAnimDesc).animType;
					bVar7 = false;
					if (AVar6 == AVar5) {
						if (peVar4->animPlayState == 0) {
							bVar7 = false;
						}
						else {
							bVar7 = (peVar4->field_0xcc & 2) != 0;
						}
					}

					if ((bVar7) || (param_2 != 0)) {
						if (this->field_0x1a4c == 0) {
							pCVar3 = this->pPlayerInput;
							if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
								fVar11 = 0.0f;
							}
							else {
								fVar11 = pCVar3->aButtons[5].clickValue;
							}
							if (fVar11 == 0.0f) {
								if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar11 = 0.0f;
								}
								else {
									fVar11 = pCVar3->aAnalogSticks[0].magnitude;
								}
								if (0.3f < fVar11) {
									this->field_0x1048 = 0.0f;
									pCVar3 = this->pPlayerInput;
									if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
										fVar11 = 0.0;
									}
									else {
										fVar11 = pCVar3->aAnalogSticks[0].magnitude;
									}
									if (fVar11 < 0.9f) {
										SetState(0x77, 0xffffffff);
									}
									else {
										SetState(0x76, 0xffffffff);
									}
								}
								else {
									bVar7 = false;
									if (AVar6 == AVar5) {
										if (peVar4->animPlayState == 0) {
											bVar7 = false;
										}
										else {
											bVar7 = (peVar4->field_0xcc & 2) != 0;
										}
									}
									if (bVar7) {
										/* Land Animation */
										SetState(0x73, 0xffffffff);
									}
								}
							}
							else {
								if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									fVar11 = 0.0;
								}
								else {
									fVar11 = pCVar3->aAnalogSticks[0].magnitude;
								}
								if (0.3 < fVar11) {
									SetState(0x8a, 0xffffffff);
								}
								else {
									SetState(0x85, 0xffffffff);
								}
							}
						}
						else {
							SetState(0x73, 0xffffffff);
						}
					}
				}
				else {
					SetState(200, 0xffffffff);
				}
			}
			else {
				if (this->airHorizontalSpeed < this->field_0x1164) {
					SetState(0xe5, 0xffffffff);
				}
				else {
					SetState(0xe4, 0xffffffff);
				}
			}
		}
		else {
			SetState(0xed, 0xffffffff);
		}
	}
	else {
		pCVar3 = this->pPlayerInput;
		if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar11 = 0.0f;
		}
		else {
			fVar11 = pCVar3->aAnalogSticks[0].magnitude;
		}

		if (0.9f < fVar11) {
			pCVar1 = this->pCollisionData;
			fVar11 = this->runSpeed;
			if ((this->dynamic.flags & 2) != 0) {
				IMPLEMENTATION_GUARD(
				fVar11 = CCollisionManager::GetWallNormalYLimit(pCVar1->aCollisionContact + 1);
				fVar12 = CCollisionManager::GetGroundSpeedDecreaseNormalYLimit(pCVar1->aCollisionContact + 1);
				fVar11 = edFIntervalLERP(pCVar1->aCollisionContact[1].location.y, fVar12, fVar11, 1.0, 0.3);
				fVar11 = this->airHorizontalSpeed * fVar11;)
			}
			SetJumpCfg(0.1f, fVar11, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
			SetState(STATE_HERO_JUMP_1_1_RUN, 0xffffffff);
		}
		else {
			pCVar1 = this->pCollisionData;
			fVar11 = this->field_0x104c;
			if ((this->dynamic.flags & 2) != 0) {
				IMPLEMENTATION_GUARD(
				fVar11 = CCollisionManager::GetWallNormalYLimit(pCVar1->aCollisionContact + 1);
				fVar12 = CCollisionManager::GetGroundSpeedDecreaseNormalYLimit(pCVar1->aCollisionContact + 1);
				fVar11 = edFIntervalLERP(pCVar1->aCollisionContact[1].location.y, fVar12, fVar11, 1.0, 0.3);
				fVar11 = this->airHorizontalSpeed * fVar11;)
			}

			SetJumpCfg(0.1f, fVar11, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
			SetState(0x78, 0xffffffff);
		}
	}
	return;
}

void CActorHeroPrivate::StateHeroGrip(float param_1, int nextState, int param_4)
{
	CCollision* pCVar1;
	CPlayerInput* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	bool bVar5;
	Timer* pTVar6;
	int iVar7;
	undefined4 uVar8;
	uint uVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	float local_20;
	float local_1c;
	float local_18;
	undefined4 local_14;
	float local_8;
	float local_4;

	pTVar6 = GetTimer();
	IncreaseEffort(1.0f);

	local_8 = 0.0f;
	local_4 = 0.0f;

	pCVar1 = this->pCollisionData;
	if (((pCVar1->flags_0x4 & 1) != 0) &&
		(fabs(pCVar1->aCollisionContact[0].location.y) < cosf(this->field_0x14b0))) {
		local_30.w = pCVar1->aCollisionContact[0].location.w;
		local_30.x = 0.0f - pCVar1->aCollisionContact[0].location.x;
		local_30.z = 0.0f - pCVar1->aCollisionContact[0].location.z;
		local_30.y = 0.0f;

		edF32Vector4SafeNormalize1Hard(&local_30, &local_30);

		bVar5 = true;
		if (0.0f <= local_30.x * this->rotationQuat.x + local_30.y * this->rotationQuat.y + local_30.z * this->rotationQuat.z) goto LAB_00139008;
	}

	bVar5 = false;
LAB_00139008:
	iVar7 = 0;
	if (bVar5) {
		iVar7 = DetectGripEdge(1, &this->currentLocation, &local_30, &local_4, &local_8, &eStack96);
	}

	if (iVar7 == 0) {
		iVar7 = DetectGripEdge(0, &this->currentLocation, &this->rotationQuat, &local_4, &local_8, &eStack96);
	}

	if (iVar7 == 0) {
		local_70.w = this->bounceLocation.w;
		local_70.xyz = 0.0f - this->bounceLocation.xyz;
		iVar7 = DetectGripEdge(0, &this->currentLocation, &local_70, &local_4, &local_8, &eStack96);
	}

	if (iVar7 == 0) {
		local_20 = 0.0f;
	}
	else {
		local_20 = param_1;
		UpdateNormal(8.0f, &this->bounceLocation, &eStack96);
		param_1 = local_20;
	}

	local_90 = this->bounceLocation;

	if (local_90.y != 0.0f) {
		local_90.y = 0.0f;
		edF32Vector4NormalizeHard(&local_90, &local_90);
	}

	local_80.y = 0.0f;
	local_80.w = 0.0f;
	local_80.x = -local_90.z;
	local_80.z = local_90.x;

	pCVar2 = this->pPlayerInput;
	if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		local_a0 = gF32Vector4Zero;
	}
	else {
		edF32Vector4ScaleHard(pCVar2->aAnalogSticks[0].magnitude, &local_a0, &pCVar2->lAnalogStick);
	}

	fVar10 = edF32Vector4DotProductHard(&local_a0, &local_80);
	fVar11 = edF32Vector4DotProductHard(&local_a0, &local_90);

	fVar11 = -fVar11;
	local_1c = 3.0f;

	if (fabs(local_4) < pTVar6->cutsceneDeltaTime * 3.0) {
		local_1c = local_4 / pTVar6->cutsceneDeltaTime;
	}
	else {
		if (local_4 <= 0.0f) {
			local_1c = -3.0f;
		}
	}

	local_18 = 3.0f;

	if (fabs(local_8) < pTVar6->cutsceneDeltaTime * 3.0f) {
		local_18 = local_8 / pTVar6->cutsceneDeltaTime;
	}
	else {
		if (local_8 <= 0.0f) {
			local_18 = -3.0f;
		}
	}

	local_14 = 0;
	fVar13 = this->bounceLocation.x;
	fVar14 = this->bounceLocation.y;
	fVar15 = this->bounceLocation.z;
	fVar16 = g_xVector.y * fVar15 - fVar14 * g_xVector.z;
	fVar17 = g_xVector.z * fVar13 - fVar15 * g_xVector.x;
	fVar18 = g_xVector.x * fVar14 - fVar13 * g_xVector.y;
	fVar14 = this->bounceLocation.x;
	fVar15 = this->bounceLocation.y;
	fVar12 = this->bounceLocation.z;
	fVar13 = -local_18;
	local_b0.x = (fVar15 * fVar18 - fVar17 * fVar12) * local_1c +
		fVar16 * local_20 + this->bounceLocation.x * fVar13;
	local_b0.y = (fVar12 * fVar16 - fVar18 * fVar14) * local_1c +
		fVar17 * local_20 + this->bounceLocation.y * fVar13;
	local_b0.z = (fVar14 * fVar17 - fVar16 * fVar15) * local_1c +
		fVar18 * local_20 + this->bounceLocation.z * fVar13;
	local_b0.w = in_vf0x * local_1c + in_vf0x * local_20 + this->bounceLocation.w * fVar13;

	fVar13 = edF32Vector4SafeNormalize0Hard(&local_b0, &local_b0);

	if (fVar13 != 0.0f) {
		this->dynamic.rotationQuat = local_b0;
	}

	this->dynamic.speed = fVar13;
	local_c0.w = this->bounceLocation.w;
	local_c0.xyz = 0.0f - this->bounceLocation.xyz;
	SV_UpdateOrientation(this->field_0x1040, &local_c0);

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;

	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	if ((((this->pCollisionData)->flags_0x4 & 2) == 0) || (1.7f <= this->distanceToGround)) {
		if (iVar7 == 0) {
			if (param_1 == 0.0f) {
				this->dynamic.speed = 0.0f;
				uVar8 = ChooseStateFall(1);
				SetState(uVar8, 0xffffffff);
			}
			else {
				if (param_1 < 0.0) {
					local_30.x = this->bounceLocation.z;
					local_30.z = -this->bounceLocation.x;
				}
				else {
					local_30.x = -this->bounceLocation.z;
					local_30.z = this->bounceLocation.x;
				}

				local_30.w = 0.0f;
				local_30.y = 0.0f;
				fVar10 = 0.1f - ((this->pCollisionData)->pObbPrim->field_0x90).z;
				fVar11 = -((this->pCollisionData)->pObbPrim->field_0x90).z - 0.1f;

				local_50 = this->bounceLocation * fVar11;

				local_40.x = local_30.x * fVar10 + local_50.x + this->currentLocation.x;
				local_40.y = fVar10 * 0.0f + local_50.y + this->currentLocation.y;
				local_40.z = local_30.z * fVar10 + local_50.z + this->currentLocation.z;
				local_40.w = fVar10 * 0.0f + local_50.w + this->currentLocation.w;

				iVar7 = DetectGripEdge(0, &local_40, &local_30, (float*)0x0, (float*)0x0, &this->field_0x1460);

				if (iVar7 == 0) {
					fVar10 = 0.1f - ((this->pCollisionData)->pObbPrim->field_0x90).z;
					fVar11 = -((this->pCollisionData)->pObbPrim->field_0x90).z - 0.3f;
					local_50 = this->bounceLocation * fVar11;

					local_40.x = local_30.x * fVar10 + local_50.x + this->currentLocation.x;
					local_40.y = local_30.y * fVar10 + local_50.y + this->currentLocation.y;
					local_40.z = local_30.z * fVar10 + local_50.z + this->currentLocation.z;
					local_40.w = local_30.w * fVar10 + local_50.w + this->currentLocation.w;
					iVar7 = DetectGripEdge(0, &local_40, &local_30, (float*)0x0, (float*)0x0, &this->field_0x1460);
				}

				if (iVar7 == 0) {
					this->dynamic.speed = 0.0f;
					uVar8 = ChooseStateFall(0);
					SetState(uVar8, 0xffffffff);
				}
				else {
					this->field_0x1490 = local_40;
					if (0.0f < param_1) {
						SetState(0xc4, 0xffffffff);
					}
					else {
						SetState(0xc2, 0xffffffff);
					}
				}
			}
		}
		else {
			if (param_4 == 0) {
				if (nextState != -1) {
					pCVar3 = this->pAnimationController;
					peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;

					if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
						bVar5 = false;

						if (peVar4->animPlayState != 0) {
							bVar5 = (peVar4->field_0xcc & 2) != 0;
						}
					}
					else {
						bVar5 = false;
					}

					if (bVar5) {
						SetState(nextState, 0xffffffff);
						return;
					}
				}
				if ((nextState == -1) || (0.2f <= this->timeInAir)) {
					if (0.8f <= -fVar10) {
						SetState(0xc1, 0xffffffff);
					}
					else {
						if (-fVar10 <= -0.8) {
							SetState(0xc0, 0xffffffff);
						}
						else {
							if (fVar11 < 0.8f) {
								pCVar2 = this->pPlayerInput;

								if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
									uVar9 = 0;
								}
								else {
									uVar9 = pCVar2->pressedBitfield & 0x10;
								}

								if (uVar9 == 0) {
									if (-0.8f < fVar11) {
										SetState(STATE_HERO_GRIP_HANG_IDLE, 0xffffffff);
										this->effort = 0.0f;
										return;
									}

									this->dynamic.speed = 0.0f;
									uVar8 = ChooseStateFall(0);
									SetState(uVar8, 0xffffffff);
									return;
								}
							}

							SetState(STATE_HERO_GRIP_UP, 0xffffffff);
						}
					}
				}
			}
			else {
				pCVar2 = this->pPlayerInput;
				if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar10 = 0.0f;
				}
				else {
					fVar10 = pCVar2->aAnalogSticks[0].magnitude;
				}

				if (fVar10 <= 0.6f) {
					SetState(STATE_HERO_GRIP_HANG_IDLE, 0xffffffff);
				}
			}
		}
	}
	else {
		this->dynamic.speed = 0.0f;
		SetState(0x7a, 0xffffffff);
	}

	return;
}

void CActorHeroPrivate::StateHeroGripUp(float param_1, float param_2, int nextState, int param_5)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	undefined4 local_10;
	float local_c;
	undefined4 local_8;
	undefined4 local_4;

	pCVar1 = this->pAnimationController;

	IncreaseEffort(2.0f);

	local_10 = 0;
	local_8 = 0;
	local_4 = 0;

	fVar4 = this->bounceLocation.x;
	fVar5 = this->bounceLocation.y;
	fVar6 = this->bounceLocation.z;
	fVar7 = g_xVector.y * fVar6 - fVar5 * g_xVector.z;
	fVar8 = g_xVector.z * fVar4 - fVar6 * g_xVector.x;
	fVar9 = g_xVector.x * fVar5 - fVar4 * g_xVector.y;
	fVar4 = this->bounceLocation.x;
	fVar5 = this->bounceLocation.y;
	fVar6 = this->bounceLocation.z;
	local_20.x = (fVar5 * fVar9 - fVar8 * fVar6) * param_1 + fVar7 * 0.0f + this->bounceLocation.x * -0.0f;
	local_20.y = (fVar6 * fVar7 - fVar9 * fVar4) * param_1 + fVar8 * 0.0f + this->bounceLocation.y * -0.0f;
	local_20.z = (fVar4 * fVar8 - fVar7 * fVar5) * param_1 + fVar9 * 0.0f + this->bounceLocation.z * -0.0f;
	local_20.w = in_vf0x * param_1 + in_vf0x * 0.0f + this->bounceLocation.w * -0.0f;
	local_c = param_1;

	fVar4 = edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
	if (fVar4 != 0.0f) {
		this->dynamic.rotationQuat = local_20;
	}

	this->dynamic.speed = fVar4;

	local_30.w = this->bounceLocation.w;
	local_30.xyz = 0.0 - this->bounceLocation.xyz;

	SV_UpdateOrientation(this->field_0x1040, &local_30);

	if ((param_5 != 0) || (((this->pCollisionData)->flags_0x4 & 1) != 0)) {
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;

		this->dynamicExt.field_0x6c = 0.0f;
	}

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	if ((param_2 <= 0.0f) || (this->timeInAir <= param_2)) {
		if (0.0f < param_2) {
			return;
		}

		peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;

		if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
			bVar3 = false;
			if (peVar2->animPlayState != 0) {
				bVar3 = (peVar2->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar3 = false;
		}

		if (!bVar3) {
			return;
		}
	}

	SetState(nextState, 0xffffffff);
	return;
}

void CActorHeroPrivate::StateHeroGripUpToJumpInit()
{
	this->dynamic.rotationQuat = this->rotationQuat;
	this->dynamic.speed = 2.5f;
	SetJumpCfg(0.1f, this->field_0x104c, this->field_0x1158 * 0.5f, this->field_0x1150 * 0.7f, this->field_0x1154 * 0.7f, 0, (edF32VECTOR4*)0x0);
	return;
}

void CActorHeroPrivate::StateHeroFall(float rotationRate, int param_3)
{
	CCollision* pCVar1;
	CActor* pReceiver;
	AnimResult* pAVar2;
	Timer* pTVar3;
	int iVar4;
	uint uVar5;
	long lVar6;
	float fVar7;

	pCVar1 = this->pCollisionData;

	MoveInAir(this->airMinSpeed, this->airHorizontalSpeed, this->airControlSpeed, this->airNoInputSpeed, rotationRate);

	fVar7 = ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

	iVar4 = this->actorState;
	if (iVar4 == -1) {
		uVar5 = 0;
	}
	else {
		pAVar2 = GetStateCfg(iVar4);
		uVar5 = pAVar2->flags_0x4 & 1;
	}

	if (uVar5 == 0) {
		if (param_3 == 0) {
			pTVar3 = GetTimer();
			fVar7 = 1.0f;
			if (1.0f < pTVar3->scaledTotalTime - this->time_0x1538) {
				SetState(STATE_HERO_FALL_A, 0xffffffff);
				return;
			}
		}
		if (this->field_0x1a10 == 0xd) {
			IMPLEMENTATION_GUARD(
			AccomplishAction(0);)
		}
		else {
			if ((pCVar1->flags_0x4 & 2) == 0) {
				if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
					lVar6 = STATE_HERO_FALL_A;

					if (this->field_0x142c != 0) {
						fVar7 = 10.3f;

						if (this->distanceToGround < 10.3f) {
							lVar6 = STATE_HERO_FALL_A;
							this->field_0x1020 = 1;
						}
						else {
							lVar6 = 0xf0;
							this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
							fVar7 = 10.3f;
						}
					}
				}
				else {
					lVar6 = 0x7a;
					this->field_0x1020 = 1;
				}
				if (lVar6 == 0xf0) {
					fVar7 = this->field_0x1184;

					if (fVar7 < this->timeInAir) {
						SetState(0xf0, 0xffffffff);
						return;
					}
				}
				else {
					this->timeInAir = 0.0f;
				}

				uVar5 = CanGrip(param_3, &this->rotationQuat);
				if (uVar5 == 0) {
					this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0);
					if ((0.28f < fVar7) && (uVar5 = TestState_IsCrouched(0xffffffff), uVar5 != 0)) {
						SetState(STATE_HERO_FALL_A, 0xffffffff);
					}
				}
				else {
					SetGripState();
				}
			}
			else {
				pReceiver = this->field_0xf18;
				if (pReceiver != (CActor*)0x0) {
					DoMessage(pReceiver, (ACTOR_MESSAGE)0x1f, (MSG_PARAM)1);
					this->field_0xf18 = (CActor*)0x0;
				}

				iVar4 = ChooseStateLanding(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y);

				SetState(iVar4, 0xffffffff);
			}
		}
	}
	else {
		if (1.5f < this->timeInAir) {
			IMPLEMENTATION_GUARD(
			(*(code*)(this->pVTable)->field_0x16c)(this);)
		}
	}
	return;
}

void CActorHeroPrivate::SetJumpCfg(float param_1, float horizonalSpeed, float param_3, float param_4, float param_5, int unused_7, edF32VECTOR4* param_8)
{
	if (0.0f <= param_1) {
		this->airMinSpeed = param_1;
	}

	if (0.0f <= horizonalSpeed) {
		this->airHorizontalSpeed = horizonalSpeed;
	}

	if (0.0f <= param_3) {
		this->airControlSpeed = param_3;
	}

	if (0.0f <= param_4) {
		this->jmp_field_0x113c = param_4;
	}

	if (0.0f <= param_5) {
		this->jmp_field_0x1140 = param_5;
	}

	this->jmp_field_0x1144 = 0;
	
	if (param_8 == (edF32VECTOR4*)0x0) {
		this->jmp_field_0x1120.x = 0.0f;
		this->jmp_field_0x1120.y = 1.0f;
		this->jmp_field_0x1120.z = 0.0f;
		this->jmp_field_0x1120.w = 0.0f;
	}
	else {
		this->jmp_field_0x1120 = *param_8;
	}
	return;
}

void CActorHeroPrivate::ConvertSpeedSumForceExtToSpeedPlayer2D()
{
	edF32VECTOR4 translation;
	edF32VECTOR4 directionalIntensity;

	edF32Vector4ScaleHard(this->dynamic.speed, &directionalIntensity, &this->dynamic.rotationQuat);
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &translation, &this->dynamicExt.normalizedTranslation);
	edF32Vector4AddHard(&translation, &translation, &directionalIntensity);
	translation.y = 0.0f;
	float newIntensity = edF32Vector4SafeNormalize0Hard(&translation, &translation);
	this->dynamic.speed = newIntensity;

	if (0.001f < newIntensity) {
		this->dynamic.rotationQuat = translation;
	}

	this->dynamicExt.field_0x6c = 0.0f;
	this->dynamicExt.normalizedTranslation = this->dynamic.rotationQuat;
	return;
}

void CActorHeroPrivate::SetBoomyFunc(int param_2)
{
	CCamera* pCVar1;
	CActor* pCVar2;
	int* piVar3;
	uint* puVar4;
	CActorHero* pCVar5;
	uint uVar6;
	float fVar7;

	if (param_2 != this->boomyState_0x1b70) {
		if ((this->boomyState_0x1b70 - 0xb < 3) && (2 < param_2 - 0xbU)) {
			IMPLEMENTATION_GUARD(
			pCVar1 = this->pActorBoomy;
			pCVar1->field_0x8 = pCVar1->field_0x8 & 0xfffffffd;
			pCVar1->field_0x8 = pCVar1->field_0x8 | 1;
			pCVar2 = (CActor*)this->pActorBoomy;
			(pCVar2->data).flags = (pCVar2->data).flags & 0xffffff7f;
			(pCVar2->data).flags = (pCVar2->data).flags | 0x20;
			CActor::EvaluateDisplayState(pCVar2);
			CAnimation::UnRegisterBone(this->base.data.pAnimationController, this->field_0x1598)
				;
			CAnimation::UnRegisterBone(this->base.data.pAnimationController, this->field_0x157c)
				;
			*(undefined4*)&this->field_0x1b64 = 0;
			this->field_0x1b68 = 0;
			this->field_0x1b6c = 0;
			*(undefined4*)&this->field_0x1b78 = 0;)
		}

		if (this->boomyState_0x1b70 == 0xd) {
			IMPLEMENTATION_GUARD(
			CAnimation::RemoveDisabledBone
			(this->base.data.pAnimationController, this->field_0x157c);
			uVar6 = 0;
			pCVar5 = this;
			do {
				FUN_00407310((int*)&pCVar5->field_0x1880);
				uVar6 = uVar6 + 1;
				pCVar5 = (CActorHero*)&(pCVar5->character).characterBase.base.base.data.actorFieldS;
			} while (uVar6 < 4);
			*(undefined4*)&this->field_0x18b0 = 0;)
		}
		else {
			if (this->boomyState_0x1b70 == 0xb) {
				IMPLEMENTATION_GUARD(
				piVar3 = *(int**)&this->field_0x18b8;
				if (((piVar3 != (int*)0x0) && (*(int*)&this->field_0x18b4 != 0)) && (*(int*)&this->field_0x18b4 == piVar3[6])
					) {
					(**(code**)(*piVar3 + 0xc))();
				}
				*(undefined4*)&this->field_0x18b8 = 0;
				*(undefined4*)&this->field_0x18b4 = 0;
				piVar3 = *(int**)&this->field_0x18c4;
				if (((piVar3 != (int*)0x0) && (*(int*)&this->field_0x18c0 != 0)) && (*(int*)&this->field_0x18c0 == piVar3[6])
					) {
					(**(code**)(*piVar3 + 0xc))();
				}
				*(undefined4*)&this->field_0x18c4 = 0;
				*(undefined4*)&this->field_0x18c0 = 0;
				piVar3 = *(int**)&this->field_0x18d0;
				if (((piVar3 != (int*)0x0) && (*(int*)&this->field_0x18cc != 0)) && (*(int*)&this->field_0x18cc == piVar3[6])
					) {
					(**(code**)(*piVar3 + 0xc))();
				}
				*(undefined4*)&this->field_0x18d0 = 0;
				*(undefined4*)&this->field_0x18cc = 0;)
			}
		}
	}
	if (param_2 == 0xd) {
		IMPLEMENTATION_GUARD(
		if (this->boomyState_0x1b70 != 0xd) {
			this->field_0x1b74 = 0;
			pCVar2 = (CActor*)this->pActorBoomy;
			(pCVar2->data).flags = (pCVar2->data).flags | 0x80;
			(pCVar2->data).flags = (pCVar2->data).flags & 0xffffffdf;
			CActor::EvaluateDisplayState(pCVar2);
			CAnimation::AddDisabledBone
			(this->base.data.pAnimationController, this->field_0x157c);
		})
	}
	else {
		if (param_2 == 0xc) {
			IMPLEMENTATION_GUARD(
			if (this->boomyState_0x1b70 != 0xc) {
				puVar4 = CActorFighter::FindBlowByName(this, s_BOOMY_STORM_00429948);
				if ((float)this->field_0x1b74 < 1.7) {
					fVar7 = edFIntervalLERP((float)this->field_0x1b74, 0.0, 1.7, 0.1, 0.65);
					*(float*)&this->field_0x1b84 = fVar7;
				}
				else {
					*(undefined4*)&this->field_0x1b84 = 0x3f800000;
				}
				if (((puVar4[0x11] != 0) && ((CCamera*)puVar4[0x12] != (CCamera*)0x0)) &&
					(*(float*)&this->field_0x1b84 != 0.0)) {
					*(CCamera**)&((CCamera*)puVar4[0x12])->objBase = this->pActorBoomy;
					*(undefined4*)(puVar4[0x12] + 4) = 0x656ad6d2;
					fVar7 = edFIntervalUnitSrcLERP(*(float*)&this->field_0x1b84, 0.1, 0.15);
					*(float*)(puVar4[0x12] + 0x1c) = fVar7;
					*(undefined4*)(puVar4[0x12] + 0x20) = 6;
				}
			})
		}
		else {
			if (param_2 == 0xb) {
				IMPLEMENTATION_GUARD(
				if (this->boomyState_0x1b70 != 0xb) {
					puVar4 = CActorFighter::FindBlowByName(this, s_BOOMY_STORM_00429948);
					this->field_0x1b74 = 0;
					*(undefined4*)&this->field_0x1b84 = 0;
					pCVar1 = this->pActorBoomy;
					pCVar1->field_0x8 = pCVar1->field_0x8 | 2;
					pCVar1->field_0x8 = pCVar1->field_0x8 & 0xfffffffe;
					pCVar2 = (CActor*)this->pActorBoomy;
					(pCVar2->data).flags = (pCVar2->data).flags & 0xffffff7f;
					(pCVar2->data).flags = (pCVar2->data).flags | 0x20;
					CActor::EvaluateDisplayState(pCVar2);
					*(undefined*)&this->pActorBoomy[4].transformationMatrix.bb = 0;
					(*(code*)(this->pActorBoomy->objBase).pVTable[1].Reset)
						(this->pActorBoomy, 5, 0xffffffffffffffff);
					CAnimation::RegisterBone
					(this->base.data.pAnimationController, this->field_0x1598);
					CAnimation::RegisterBone
					(this->base.data.pAnimationController, this->field_0x157c);
					FUN_004073b0((int*)&this->field_0x18b4, (long)(int)this->pActorBoomy, 0x656ad6d2);
					if ((puVar4[0x11] != 0) && (puVar4[0x12] != 0)) {
						*(undefined4*)(puVar4[0x12] + 0x20) = 0;
					}
				})
			}
			else {
				if (((param_2 == 4) || (param_2 == 3)) || ((param_2 == 2 || (param_2 == 1)))) {
					IMPLEMENTATION_GUARD(
					this->field_0x1b98 = 1.0f;
					this->field_0x1b9c = 1.0f;)
				}
				else {
					this->field_0x1b9c = 0.0f;
				}
			}
		}
	}

	this->boomyState_0x1b70 = param_2;
	return;
}

void CActorHeroPrivate::IncreaseEffort(float param_1)
{
	Timer* pTVar1;
	float fVar2;

	pTVar1 = GetTimer();
	fVar2 = this->effort + param_1 * pTVar1->cutsceneDeltaTime;
	this->effort = fVar2;
	if (20.0f < fVar2) {
		this->effort = 20.0f;
	}
	else {
		if (fVar2 < 0.0) {
			this->effort = 0.0f;
		}
	}
	return;
}

void CActorHeroPrivate::MoveInAir(float minSpeed, float newSpeed, float airControlSpeed, float noInputSpeed, float rotationRate)
{
	CPlayerInput* pCVar1;
	edF32VECTOR4* pNewOrientation;
	float inputMagnitude;
	float velocitySpeed;
	edF32VECTOR4 controlDirection;
	edF32VECTOR4 velocity;

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::MoveInAir minSpeed: {}, newSpeed: {}, airControlSpeed: {}, noInputSpeed: {}, rotationRate: {}", 
		minSpeed, newSpeed, airControlSpeed, noInputSpeed, rotationRate);

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		inputMagnitude = 0.0f;
	}
	else {
		inputMagnitude = pCVar1->aAnalogSticks[0].magnitude;
	}

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::MoveInAir inputMagnitude: {}", inputMagnitude);

	if (0.3f <= inputMagnitude) {
		if (this->field_0x1a50 == 0) {
			edF32Vector4ScaleHard(this->dynamic.speed, &velocity, &this->dynamic.rotationQuat);

			pCVar1 = this->pPlayerInput;
			if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				controlDirection = gF32Vector4Zero;
			}
			else {
				edF32Vector4ScaleHard(pCVar1->aAnalogSticks[0].magnitude, &controlDirection, &pCVar1->lAnalogStick);
			}

			ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::MoveInAir velocity: {}, controlDirection: {}", velocity.ToString(), controlDirection.ToString());

			edF32Vector4ScaleHard(airControlSpeed, &controlDirection, &controlDirection);
			edF32Vector4AddHard(&velocity, &velocity, &controlDirection);
			velocitySpeed = edF32Vector4NormalizeHard(&velocity, &velocity);

			ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::MoveInAir velocitySpeed <= newSpeed {} minSpeed <= velocitySpeed {}", velocitySpeed <= newSpeed, minSpeed <= velocitySpeed);

			if ((velocitySpeed <= newSpeed) && (newSpeed = minSpeed, minSpeed <= velocitySpeed)) {
				newSpeed = velocitySpeed;
			}

			ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::MoveInAir newSpeed: {}", newSpeed);

			this->dynamic.speed = newSpeed;

			if (0.001f < newSpeed) {
				this->dynamic.rotationQuat = velocity;
			}

			SV_UpdateOrientation2D(rotationRate, &this->dynamic.rotationQuat, 0);
		}
		else {
			if ((pCVar1 == (CPlayerInput*)0x0) || (pNewOrientation = &pCVar1->lAnalogStick, this->field_0x18dc != 0))
			{
				pNewOrientation = &gF32Vector4Zero;
			}

			SV_UpdateOrientation2D(rotationRate, pNewOrientation, 0);
			SV_MOV_UpdateSpeedIntensity(0.0f, noInputSpeed);
		}
	}
	else {
		SV_MOV_UpdateSpeedIntensity(0.0f, noInputSpeed);
	}
	return;
}

void CActorHeroPrivate::SetBoomyHairOff()
{
	IMPLEMENTATION_GUARD_LOG(
	this->pAnimationController->AddDisabledBone(this->animKey_0x1584);)
}

void CActorHeroPrivate::SetBoomyHairOn()
{
	IMPLEMENTATION_GUARD_LOG(
	this->pAnimationController->RemoveDisabledBone(this->animKey_0x1584);)
}

void CActorHeroPrivate::SetGripState()
{
	int curState;

	if (this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y < -8.0f) {
		SetState(0xbd, 0xffffffff);
	}
	else {
		curState = this->actorState;
		if (((curState == 0x79) || (curState == 0x7c)) || (curState == 199)) {
			SetState(STATE_HERO_GRIP_GRAB, 0xffffffff);
		}
		else {
			SetState(STATE_HERO_GRIP_B, 0xffffffff);
		}
	}
	return;
}

int CActorHeroPrivate::DetectGripEdge(int param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, float* param_5, float* param_6, edF32VECTOR4* param_7)
{
	CCollision* pCVar1;
	CActor* pCVar2;
	CActor* pTieActor;
	uint uVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	CCollisionRay CStack176;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	CCollisionRay CStack96;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	_ray_info_out local_10;

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge param_2: {}, param_3: {}, param_4: {}", param_2, param_3->ToString(), param_4->ToString());

	iVar4 = 0;

	fVar5 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.1;

	local_20.x = param_4->x * fVar5 + param_3->x;
	local_20.y = param_3->y + 1.75f;
	local_20.z = param_4->z * fVar5 + param_3->z;
	local_20.w = param_4->w * fVar5 + param_3->w;

	local_40.x = 0.0;
	local_40.y = -1.0f;
	local_40.z = 0.0;
	local_40.w = 0.0;

	CStack96 = CCollisionRay(0.35f, &local_20, &local_40);
	fVar5 = CStack96.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000010, &local_30, &local_10);

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar5: {}", fVar5);

	if ((((fVar5 == 1e+30f) || (fVar5 <= 0.01f)) || (local_30.y < cosf(this->field_0x14b4))) ||
		(0.0f < local_30.x * local_40.x + local_30.y * local_40.y + local_30.z * local_40.z)) {
		fVar5 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.3f;

		local_20.x = param_4->x * fVar5 + param_3->x;
		local_20.y = param_3->y + 1.75f;
		local_20.z = param_4->z * fVar5 + param_3->z;
		local_20.w = param_4->w * fVar5 + param_3->w;

		fVar5 = CStack96.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000010, &local_30, &local_10);
		ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar5: {}", fVar5);
	}

	if (((fVar5 != 1e+30f) && (0.01f < fVar5)) && ((((uint)local_10.pVector_0x4 & 0x1200) == 0 && ((cosf(this->field_0x14b4) <= local_30.y &&
				(local_30.x * local_40.x + local_30.y * local_40.y + local_30.z * local_40.z <= 0.0f)))))) {
		local_90.x = param_3->x;
		fVar5 = local_20.y + local_40.y * fVar5;
		local_90.y = fVar5 - 0.01f;
		local_90.z = param_3->z;
		local_90.w = 1.0f;
		local_70.x = param_4->x;
		local_70.y = 0.0f;
		local_70.z = param_4->z;
		local_70.w = 0.0f;

		if (param_4->y != 0.0f) {
			edF32Vector4NormalizeHard(&local_70, &local_70);
		}

		CStack176 = CCollisionRay(((this->pCollisionData)->pObbPrim->field_0x90).z + 0.5f, &local_90, &local_70);
		fVar6 = CStack176.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000010, &local_80, (_ray_info_out*)0x0);

		ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar6: {}", fVar6);

		if ((((fVar6 == 1e+30f) || (fVar6 <= 0.01f)) ||
			(cosf(this->field_0x14b0) <= fabs(local_80.y))) ||
			(0.0f < local_80.x * local_70.x + local_80.y * local_70.y + local_80.z * local_70.z)) {
			local_90.y = fVar5 - 0.1f;
			fVar6 = CStack176.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000010, &local_80, (_ray_info_out*)0x0);
		}

		if (((fVar6 != 1e+30f) && (0.01 < fVar6)) &&
			((((uint)local_10.pVector_0x4 & 0x1200) == 0 &&
				((fabs(local_80.y) < cosf(this->field_0x14b0) && (local_80.x * local_70.x + local_80.y * local_70.y + local_80.z * local_70.z <= 0.0f)))))) {
			fVar7 = ((this->pCollisionData)->pObbPrim->field_0x90).z - 0.2f;

			local_20.x = param_4->x * fVar7 + param_3->x;
			local_20.y = param_3->y + 1.8f;
			local_20.z = param_4->z * fVar7 + param_3->z;
			local_20.w = param_4->w * fVar7 + param_3->w;

			fVar7 = CStack96.Intersect(3, (CActor*)this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar7: {}", fVar7);


			if (fVar7 == 1e+30f) {
				if (param_2 != 0) {
					pCVar1 = this->pCollisionData;
					if (((pCVar1->flags_0x4 & 1) != 0) && (fabs(pCVar1->aCollisionContact[0].location.y) < cosf(this->field_0x14b0))) {
						local_80.xyz = pCVar1->aCollisionContact[0].location.xyz;
					}
				}

				local_80.y = 0.0f;

				edF32Vector4NormalizeHard(param_7, &local_80);

				if (param_5 != (float*)0x0) {
					*param_5 = fVar5 - (param_3->y + 1.7f);
				}

				if (param_6 != (float*)0x0) {
					*param_6 = fVar6 - ((this->pCollisionData)->pObbPrim->field_0x90).z;
				}

				pCVar2 = this->pCheckpointManagerSubObjA_0x14a4;

				if (pCVar2 != local_10.pActor_0x0) {
					if (pCVar2 != (CActor*)0x0) {
						pTieActor = this->pTiedActor;
						if (pCVar2 == pTieActor) {
							TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
							IMPLEMENTATION_GUARD(
							if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) &&
								(uVar3 = CCollision::IsVertexAboveAndAgainstObbTree
								(&(this->pCollisionData)->highestVertex,
									pTieActor->pCollisionData->pObbTree), uVar3 != 0)) {
								TieToActor(pTieActor, 0, 0, (edF32MATRIX4*)0x0);
							})
						}
						this->pCheckpointManagerSubObjA_0x14a4 = (CActor*)0x0;
					}

					this->pCheckpointManagerSubObjA_0x14a4 = local_10.pActor_0x0;
					pCVar2 = this->pCheckpointManagerSubObjA_0x14a4;
					if (pCVar2 != (CActor*)0x0) {
						TieToActor(pCVar2, 0, 1, (edF32MATRIX4*)0x0);
					}
				}

				iVar4 = 1;
			}
		}
	}

	return iVar4;
}

int CActorHeroPrivate::EvolutionBounceCanJump()
{
	IMPLEMENTATION_GUARD_LOG(
		return LevelScheduleManager::ScenVar_Get(7);)
		return false;
}

uint CActorHeroPrivate::CanGrip(int param_2, edF32VECTOR4* pRotation)
{
	CCollision* pCVar1;
	CActor* pCVar2;
	CActor* pTieActor;
	bool bVar3;
	uint uVar4;
	uint uVar5;
	float fVar6;
	edF32VECTOR4 local_10;

	if (this->field_0x1420 != 0) {
		return 0;
	}

	pCVar1 = this->pCollisionData;
	uVar4 = 0;

	if (((pCVar1->flags_0x4 & 1) != 0) && (fabs(pCVar1->aCollisionContact[0].location.y) < cosf(this->field_0x14b0))) {
		local_10.w = pCVar1->aCollisionContact[0].location.w;
		local_10.x = 0.0f - pCVar1->aCollisionContact[0].location.x;
		local_10.z = 0.0f - pCVar1->aCollisionContact[0].location.z;
		local_10.y = 0.0;
		edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
		bVar3 = true;

		if (0.0f <= local_10.x * pRotation->x + local_10.y * pRotation->y + local_10.z * pRotation->z) goto LAB_0013bd58;
	}

	bVar3 = false;
LAB_0013bd58:
	if (bVar3) {
		uVar4 = DetectGripEdge(1, &this->currentLocation, &local_10, (float*)0x0, (float*)0x0, &this->bounceLocation);
	}

	if (uVar4 == 0) {
		uVar4 = DetectGripEdge(0, &this->currentLocation, pRotation, (float*)0x0, (float*)0x0, &this->bounceLocation);
	}

	if ((uVar4 != 0) && (param_2 != 0)) {
		fVar6 = this->dynamic.speed;
		bVar3 = this->dynamic.rotationQuat.x * fVar6 * this->bounceLocation.x + this->dynamic.rotationQuat.y * fVar6 * this->bounceLocation.y +
			this->dynamic.rotationQuat.z * fVar6 * this->bounceLocation.z < 0.0;

		uVar4 = (uint)bVar3;

		if ((!bVar3) && (pCVar2 = this->pCheckpointManagerSubObjA_0x14a4, pCVar2 != (CActor*)0x0)) {
			IMPLEMENTATION_GUARD(
			pTieActor = this->pTiedActor;
			if (pCVar2 == pTieActor) {
				(*(this->pVTable)->TieToActor)((CActor*)this, 0, 0, (edF32MATRIX4*)0x1);
				if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) &&
					(uVar5 = CCollision::IsVertexAboveAndAgainstObbTree(&(this->pCollisionData)->highestVertex, pTieActor->pCollisionData->pObbTree), uVar5 != 0)) {
					TieToActor(pTieActor, 0, 0, (edF32MATRIX4*)0x0);
				}
			}
			this->pCheckpointManagerSubObjA_0x14a4 = (CActor*)0x0;)
		}
	}

	return uVar4;
}

bool CActorHeroPrivate::CanBounceAgainstWall()
{
	CCollision* pCVar1;
	bool bVar2;
	int iVar3;
	long lVar4;
	edF32VECTOR4* v0;
	float local_20;
	float local_1c;
	float local_18;
	float local_10;
	float local_c;
	float local_8;

	if ((this->field_0x1455 == 0) && (iVar3 = EvolutionBounceCanJump(), iVar3 != 0)) {
		IMPLEMENTATION_GUARD(
		pCVar1 = this->pCollisionData;
		bVar2 = false;
		if ((pCVar1->flags_0x4 & 1) != 0) {
			if (edFCosinus[(int)(ABS(this->field_0x14c4 * 1303.797) + 0.5) & 0x1fff] <=
				ABS(pCVar1->aCollisionContact[0].location.y)) {
				bVar2 = false;
			}
			else {
				local_10 = 0.0;
				local_c = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.2;
				local_8 = (this->base).field_0x14c4;
				local_20 = *(float*)&(this->base).field_0x14c8;
				local_1c = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.2;
				local_18 = (this->base).field_0x14c4;
				lVar4 = DetectWall(this, 3, 0x40000080, (float*)&this->rotationQuat,
					&local_10, 0, 0, 0);
				if ((lVar4 == 0) ||
					(lVar4 = DetectWall(this, 3, 0x40000080, (float*)&this->rotationQuat,
						&local_20, 0, 0, 0), lVar4 == 0)) {
					bVar2 = false;
				}
				else {
					v0 = &(this->base).bounceLocation;
					(this->base).bounceLocation.x = pCVar1->aCollisionContact[0].location.x;
					(this->base).bounceLocation.y = 0.0;
					(this->base).bounceLocation.z = pCVar1->aCollisionContact[0].location.z;
					(this->base).bounceLocation.w = 0.0;
					edF32Vector4NormalizeHard(v0, v0);
					bVar2 = true;
				}
			}
		})
	}
	else {
		bVar2 = false;
	}
	return bVar2;
}

void CActorHeroPrivate::ChangeCollisionSphereForLying(float param_2)
{
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.x = 0.8f;
	local_10.z = 0.8f;
	local_10.y = 0.4f;
	local_10.w = 0.0f;

	local_20.x = 0.0f;
	local_20.z = 0.0f;
	local_20.y = 0.38f;
	local_20.w = 1.0f;

	ChangeCollisionSphere(param_2, &local_10, &local_20);
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
	this->field_0xa80 = fVar8 * this->dynamic.linearAcceleration + fVar9 * this->field_0xa80;
	this->field_0xa84 = fVar8 * this->dynamic.linearJerk + fVar9 * this->field_0xa84;
	fVar9 = fVar9 * this->field_0xa88 + fVar8 * this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y;
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

void CActorHeroPrivate::BuildHorizontalSpeedVector(float runSpeed, float param_2, float param_3, float param_4, float param_5)
{
	CPlayerInput* pInput;
	edF32VECTOR4* v0;
	float puVar2;
	float puVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_20;
	edF32VECTOR4 newRotation;

	pInput = this->pPlayerInput;

	if ((pInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		local_20 = gF32Vector4Zero;
	}
	else {
		edF32Vector4ScaleHard(pInput->aAnalogSticks[0].magnitude, &local_20, &pInput->lAnalogStick);
	}

	pInput = this->pPlayerInput;
	if ((pInput == (CPlayerInput*)0x0) || (v0 = &pInput->lAnalogStick, this->field_0x18dc != 0)) {
		v0 = &gF32Vector4Zero;
	}

	puVar2 = edF32Vector4DotProductHard(v0, &this->dynamic.horizontalVelocityDirectionEuler);
	if (1.0f < puVar2) {
		puVar3 = 1.0f;
	}
	else {
		puVar3 = -1.0f;
		if (-1.0f <= puVar2) {
			puVar3 = puVar2;
		}
	}

	fVar4 = acosf(puVar3);
	fVar5 = edFIntervalLERP(fVar4, 0.0f, param_5, 0.5f, 2.0f);
	edF32Vector4ScaleHard(param_3 * fVar5, &local_20, &local_20);
	fVar4 = edFIntervalLERP(fVar4, 0.0f, param_5, 1.0f, 0.5f);
	fVar4 = runSpeed * fVar4;
	edF32Vector4ScaleHard(this->dynamic.speed, &newRotation, &this->dynamic.rotationQuat);
	edF32Vector4AddHard(&newRotation, &newRotation, &local_20);
	fVar5 = edF32Vector4SafeNormalize0Hard(&newRotation, &newRotation);

	if (0.001f < fVar5) {
		this->dynamic.rotationQuat = newRotation;
	}

	if (fVar5 <= fVar4) {
		fVar4 = fVar5;
	}

	SV_MOV_UpdateSpeedIntensity(fVar4, param_2);
	SV_UpdateOrientation2D(param_4, &newRotation, 0);
	return;
}

CLifeInterface* CActorHeroPrivate::GetLifeInterface()
{
	CLifeInterface* pLifeInterface;

	pLifeInterface = &this->lifeInterface;

	if (this->field_0xcbc != 0) {
		pLifeInterface = &this->field_0xee4;
	}

	return pLifeInterface;
}

void CActorHeroPrivate::LifeDecrease(float amount)
{
	Timer* pTVar1;
	int iVar2;

	if ((((amount != 0.0f) && (this->field_0xaa4 == 0)) && (pTVar1 = Timer::GetTimer(), this->field_0x155c <= pTVar1->scaledTotalTime)) && (this->field_0x1558 <= 0.0f)) {
		iVar2 = (int)amount;
		if (iVar2 < 1) {
			iVar2 = 1;
		}

		CActorAutonomous::LifeDecrease((float)iVar2);
	}

	return;
}

void CActorHeroPrivate::AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	int iVar1;
	edANM_HDR* peVar2;
	CBehaviour* pCVar3;
	uint* puVar4;
	uint* puVar5;
	float in_f0;
	float fVar6;
	undefined* puVar7;
	undefined* puVar8;
	//CBehaviourVtable* pCVar9;
	//CBehaviourVtable* pCVar10;
	//edAnmMacroBlendN local_c;
	//edAnmMacroBlendN local_8;
	edANM_HDR* local_4;

	if (((newAnim == 0x19f) || (newAnim == 0x19e)) || (newAnim == 0x191)) {
		IMPLEMENTATION_GUARD(
		local_c = ((edAnmMacroBlendN*)&pAnimator->pAnimKeyTableEntry)->field_0x0;
		peVar2 = (edANM_HDR*)(*(int*)((int)local_c + 8) * 4 + (int)local_c);
		if (this->curBehaviourId == 8) {
			pCVar3 = GetBehaviour(this->curBehaviourId);
			if ((newAnim == 0x19f) || (newAnim == 0x19e)) {
				*(CBehaviourVtable**)((int)((edANM_HDR*)(*(int*)((int)local_c + 8) * 4 + (int)local_c) + 1) + 4) =
					pCVar3[0x2c].pVTable;
				peVar2 = (edANM_HDR*)(*(int*)((int)local_c + 8) * 4 + (int)local_c);
				peVar2[1].flags = (uint)(1.0 - *(float*)((int)(peVar2 + 1) + 4));
			}
			else {
				if (newAnim == 0x191) {
					pCVar9 = pCVar3[0x1b].pVTable;
					if (0.0 <= (float)pCVar9) {
						pCVar10 = pCVar3[0x1c].pVTable;
						if (0.0 <= (float)pCVar10) {
							CActor::SV_Blend4AnimationsWith2Ratios((float)pCVar10, (float)pCVar9, &local_c, 4, 5, 7, 8);
							peVar2[2].flags = 0;
							peVar2[3].flags = 0;
						}
						else {
							CActor::SV_Blend4AnimationsWith2Ratios(-(float)pCVar10, (float)pCVar9, &local_c, 4, 3, 7, 6);
							*(int*)((int)(peVar2 + 2) + 8) = 0;
							*(int*)((int)(peVar2 + 3) + 8) = 0;
						}
						peVar2[1].flags = 0;
						*(int*)((int)(peVar2 + 1) + 4) = 0;
						*(int*)((int)(peVar2 + 1) + 8) = 0;
					}
					else {
						pCVar10 = pCVar3[0x1c].pVTable;
						if (0.0 <= (float)pCVar10) {
							CActor::SV_Blend4AnimationsWith2Ratios((float)pCVar10, -(float)pCVar9, &local_c, 4, 5, 1, 2);
							peVar2[1].flags = 0;
							peVar2[2].flags = 0;
						}
						else {
							CActor::SV_Blend4AnimationsWith2Ratios(-(float)pCVar10, -(float)pCVar9, &local_c, 4, 3, 1, 0);
							*(int*)((int)(peVar2 + 1) + 8) = 0;
							*(int*)((int)(peVar2 + 2) + 8) = 0;
						}
						peVar2[3].flags = 0;
						*(int*)((int)(peVar2 + 3) + 4) = 0;
						*(int*)((int)(peVar2 + 3) + 8) = 0;
					}
				}
			}
		})
	}
	else {
		if ((newAnim == 0xe1) || (newAnim == 0xd7)) {
			IMPLEMENTATION_GUARD(
			peVar2 = pAnimator->pAnimKeyTableEntry;
			(&peVar2[1].field_0x4)[peVar2->keyIndex_0x8] = *(int*)&this->field_0x10f8;
			(&peVar2->flags + peVar2->keyIndex_0x8)[3] = (uint)(1.0 - (float)(&peVar2->flags + peVar2->keyIndex_0x8)[4]);)
		}
		else {
			if ((newAnim == 0xe0) || (newAnim == 0xd6)) {
				IMPLEMENTATION_GUARD(
				peVar2 = pAnimator->pAnimKeyTableEntry;
				(&peVar2[1].field_0x4)[peVar2->keyIndex_0x8] = this->field_0x10f4;
				(&peVar2->flags + peVar2->keyIndex_0x8)[3] = (uint)(1.0 - (float)(&peVar2->flags + peVar2->keyIndex_0x8)[4]);)
			}
			else {
				if ((newAnim == 0xdc) || (newAnim == 0xd2)) {
					IMPLEMENTATION_GUARD(
					local_8 = ((edAnmMacroBlendN*)&pAnimator->pAnimKeyTableEntry)->field_0x0;
					fVar6 = this->field_0x10f0;
					peVar2 = (edANM_HDR*)(*(int*)((int)local_8 + 8) * 4 + (int)local_8);
					if (fVar6 < 0.0) {
						CActor::SV_Blend4AnimationsWith2Ratios(-fVar6, this->field_0x10f4, &local_8, 2, 0, 3, 1);
						*(int*)((int)(peVar2 + 2) + 4) = 0;
						*(int*)((int)(peVar2 + 2) + 8) = 0;
					}
					else {
						CActor::SV_Blend4AnimationsWith2Ratios(fVar6, this->field_0x10f4, &local_8, 2, 4, 3, 5);
						peVar2[1].flags = 0;
						*(int*)((int)(peVar2 + 1) + 4) = 0;
					})
				}
				else {
					if (newAnim == 0xcf) {
						char* pBase = (char*)pAnimator->pAnimKeyTableEntry;

						AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

						peVar2 = pAnimator->pAnimKeyTableEntry;
						if (this->field_0x1094 == 0) {
							float yLimit = CCollision::GetWallNormalYLimit(&this->collisionContact);
							fVar6 = edFIntervalUnitDstLERP(this->field_0x10b0.y, 1.0f, yLimit);

							pValue->field_0xc = 0.0f;
							pValue->field_0x14 = fVar6;
							pValue->field_0x10 = 1.0f - pValue->field_0x14;

							//(&peVar2->flags + iVar1)[3] = 0;
							//(&peVar2[1].keyIndex_0x8)[peVar2->keyIndex_0x8] = (int)fVar6;
							//(&peVar2->flags + peVar2->keyIndex_0x8)[4] =
							//	(uint)(1.0f - (float)(&peVar2->flags + peVar2->keyIndex_0x8)[5]);
						}
						else {
							float yLimit = CCollision::GetWallNormalYLimit(&this->collisionContact);
							fVar6 = edFIntervalUnitDstLERP(this->field_0x10b0.y, yLimit, 1.0f);

							pValue->field_0x10 = fVar6;
							pValue->field_0xc = 1.0f - pValue->field_0x10;
							pValue->field_0x14 = 1.0f - pValue->field_0x14;

							//(&peVar2[1].field_0x4)[peVar2->keyIndex_0x8] = (int)fVar6;
							//(&peVar2->flags + peVar2->keyIndex_0x8)[3] =
							//	(uint)(1.0 - (float)(&peVar2->flags + peVar2->keyIndex_0x8)[4]);
							//(&peVar2->flags + iVar1)[5] = 0;
						}
					}
					else {
						if (newAnim == 0xaa) {
							IMPLEMENTATION_GUARD(
							fVar6 = *(float*)&this->field_0x1200;
							puVar4 = &pAnimator->pAnimKeyTableEntry->flags + pAnimator->pAnimKeyTableEntry->keyIndex_0x8;
							if (0.0 <= fVar6) {
								puVar4[4] = (uint)fVar6;
								puVar4[3] = 0x3c23d70a;
							}
							else {
								puVar4[4] = 0x3c23d70a;
								puVar4[3] = (uint)-*(float*)&this->field_0x1200;
							}
							fVar6 = *(float*)&this->field_0x1204;
							if (0.0 <= fVar6) {
								puVar4[6] = (uint)fVar6;
								puVar4[5] = 0x3c23d70a;
							}
							else {
								puVar4[6] = 0x3c23d70a;
								puVar4[5] = (uint)-*(float*)&this->field_0x1204;
							}
							fVar6 = *(float*)&this->field_0x1208;
							if (0.0 <= fVar6) {
								puVar4[8] = (uint)fVar6;
								puVar4[7] = 0x3c23d70a;
							}
							else {
								puVar4[8] = 0x3c23d70a;
								puVar4[7] = (uint)-*(float*)&this->field_0x1208;
							})
						}
						else {
							if (newAnim == 0x102) {
								IMPLEMENTATION_GUARD(
								local_4 = pAnimator->pAnimKeyTableEntry;
								puVar7 = *(undefined**)&this->field_0x11f4;
								puVar4 = &local_4->flags + local_4->keyIndex_0x8;
								puVar5 = puVar4 + 3;
								if (1.0 < (float)puVar7) {
									puVar8 = (undefined*)0x3f800000;
								}
								else {
									puVar8 = &DAT_bf800000;
									if (-1.0 <= (float)puVar7) {
										puVar8 = puVar7;
									}
								}
								if ((float)puVar8 < 0.0) {
									fVar6 = *(float*)&this->field_0x11f8;
									if (fVar6 < 0.0) {
										CActor::SV_Blend3AnimationsWith2Ratios(-fVar6, -(float)puVar8, &local_4, 1, 0, 3);
										puVar4[5] = 0;
									}
									else {
										CActor::SV_Blend3AnimationsWith2Ratios(fVar6, -(float)puVar8, &local_4, 1, 2, 3);
										*puVar5 = 0;
									}
									puVar4[7] = 0;
									puVar4[8] = 0;
								}
								else {
									fVar6 = *(float*)&this->field_0x11fc;
									if (0.0 < fVar6) {
										puVar4[8] = (uint)fVar6;
										(&local_4->flags + local_4->keyIndex_0x8)[4] =
											(uint)(1.0 - (float)(&local_4->flags + local_4->keyIndex_0x8)[8]);
										puVar4[7] = 0;
										puVar4[5] = 0;
										*puVar5 = 0;
									}
									else {
										fVar6 = *(float*)&this->field_0x11f8;
										if (fVar6 < 0.0) {
											CActor::SV_Blend3AnimationsWith2Ratios(-fVar6, (float)puVar8, &local_4, 1, 0, 4);
											puVar4[5] = 0;
										}
										else {
											CActor::SV_Blend3AnimationsWith2Ratios(fVar6, (float)puVar8, &local_4, 1, 2, 4);
											*puVar5 = 0;
										}
										puVar4[8] = 0;
									}
									puVar4[6] = 0;
								})
							}
							else {
								if (newAnim == 0x104) {
									IMPLEMENTATION_GUARD(
									peVar2 = pAnimator->pAnimKeyTableEntry;
									fVar6 = *(float*)&this->field_0x13d8;
									puVar4 = &peVar2->flags + peVar2->keyIndex_0x8;
									if (fVar6 < 0.0) {
										puVar4[3] = (uint)-fVar6;
										(&peVar2->flags + peVar2->keyIndex_0x8)[4] =
											(uint)(1.0 - (float)(&peVar2->flags + peVar2->keyIndex_0x8)[3]);
										puVar4[5] = 0;
									}
									else {
										puVar4[5] = (uint)fVar6;
										(&peVar2->flags + peVar2->keyIndex_0x8)[4] =
											(uint)(1.0 - (float)(&peVar2->flags + peVar2->keyIndex_0x8)[5]);
										puVar4[3] = 0;
									})
								}
								else {
									if (newAnim == 0xfe) {
										
										char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
										fVar6 = edFIntervalUnitDstLERP(this->field_0x1048, 0.2f, 0.8f);

										AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

										pValue->field_0xc = fVar6;
										pValue->field_0x10 = 1.0f - pValue->field_0xc;
									}
									else {
										IMPLEMENTATION_GUARD(
										CActorFighter::AnimEvaluate();)
									}
								}
							}
						}
					}
				}
			}
		}
	}
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
				(**(GetAnimForState**)((int)((pCVar1->base).base.pVTable + 1) + 0x54))
					((CActor*)pCVar1, (int)pOwner);
				ActorFloatFunc_00147a70
				(0.1, in_f0, *(float*)&(pCVar1->base).field_0x1158, (pCVar1->base).field_0x1150, fVar3, (Actor*)pCVar1, 1,
					(edF32VECTOR4*)0x0);
			}
			else {
				pCVar1 = this->pHero;
				(**(GetAnimForState**)((int)((pCVar1->base).base.pVTable + 1) + 0x54))
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
