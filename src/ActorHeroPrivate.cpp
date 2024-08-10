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
#include "CollisionRay.h"
#include "ActorWind.h"
#include "ActorFactory.h"
#include "ActorManager.h"

CActorHeroPrivate::CActorHeroPrivate()
{
	int* piVar1;
	undefined4* puVar2;
	
	CActor();
	//this->pVTable = &CActorMovable::_vt;
	this->dynamic.weightB = 0.0f;
	//this->pVTable = &CActorAutonomous::_vt;
	this->dynamicExt.gravityScale = 1.0f;
	this->dynamicExt.field_0x4 = 0.0f;
	this->dynamicExt.field_0x8 = 0.0f;
	this->lifeInterface = CLifeInterface();
	//piVar1 = &(this->character).field_0x550;
	//this->pVTable = &CActorFighter::_vt;
	//do {
	//	*piVar1 = 0;
	//	piVar1[1] = 0;
	//	piVar1 = piVar1 + 3;
	//} while (piVar1 != (int*)&(this->character).field_0x568);
	//CPathFinderClient::CPathFinderClient((long)(int)&(this->character).field_0x570);
	//*(undefined4*)&(this->character).field_0x638 = 0;
	//CSlaveGroundSampler::CSlaveGroundSampler((long)&this->field_0x9e0);
	//this->pVTable = &CActorHero::_vt;
	//CMagicInterface::CMagicInterface((long)&this->field_0xab8);
	//CMoneyInterface::CMoneyInterface((long)&this->field_0xad8);
	//CInventoryInterface::CInventoryInterface((long)&this->field_0xadc);
	//FUN_001dbc60((long)&this->field_0xc84);
	//FUN_00324320((long)&this->field_0xcd0);
	//this->pVTable = (ActorVTable*)&_vt;
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
	//{
	//	CActorFighter::Create(pByteCode);
	//
	//	static bool bInitedFirst = true;
	//
	//	if (bInitedFirst) {
	//		char* pCurrent = pByteCode->currentSeekPos;
	//
	//		while (true) {
	//			if (strncmp(pCurrent, "TSNI", 4) == 0) {
	//				break;
	//			}
	//			pCurrent++;
	//		}
	//
	//		pByteCode->currentSeekPos = pCurrent;
	//
	//		this->pActorBoomy = (CActorBoomy*)0x0;
	//		return;
	//	}
	//
	//	bInitedFirst = true;
	//}

	CActorFighter::Create(pByteCode);

	//pByteCode->currentSeekPos += 0xdd78;

	// FOR LEVEL 1
	pByteCode->currentSeekPos += 0x2288;

	//this->field_0x1870 = 0;
	//this->field_0x1874 = this->field_0x444;
	//*(undefined4*)&this->field_0x1878 = *(undefined4*)&this->field_0x448;
	//this->field_0xff0 = 0;
	uVar2 = pByteCode->GetU32();
	assert(uVar2 == 0xDCF9FCDC);
	this->field_0x157c = uVar2;
	uVar2 = pByteCode->GetU32();
	this->animKey_0x1584 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->animKey_0x1588 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->animKey_0x158c = uVar2;
	uVar2 = pByteCode->GetU32();
	this->animKey_0x1590 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->animKey_0x1594 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->field_0x1594 = uVar2;
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
					IMPLEMENTATION_GUARD_LOG();
					piVar7 = (int*)pByteCode->currentSeekPos;
					pByteCode->currentSeekPos = (char*)(piVar7 + 1);
					if (*piVar7 != 0) {
						pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar7 * 0x1c;
					}
					//piVar5[7] = (int)piVar7;
					pcVar1 = pByteCode->currentSeekPos;
					pByteCode->currentSeekPos = pcVar1 + 0x20;
					//piVar5[8] = (int)pcVar1;
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
	this->field_0x12e0 = iVar11;
	iVar11 = this->field_0x12e0;
	if (iVar11 != -1) {
		IMPLEMENTATION_GUARD_LOG(
		CActor::SV_InstallMaterialId(iVar11);
		CFxTail::Create(1.0f, (CFxTail*)&this->fxTrailA.field_0x4, 0x10, 4, this->field_0x12e0);
		CFxTail::Create(1.0f, (CFxTail*)&this->fxTrailB.field_0x4, 0x10, 4, this->field_0x12e0);)
	}
	iVar11 = pByteCode->GetS32();
	//*(int*)&this->field_0x13c8 = iVar11;
	fVar13 = pByteCode->GetF32();
	//this->field_0x1990 = fVar13;
	//this->bCanUseCheats = 0;
	//this->pCameraViewBase_0xaa8 = (CCamera*)0x0;
	iVar11 = CLevelScheduler::ScenVar_Get(0x11);
	iVar10 = CLevelScheduler::ScenVar_Get(0x12);
	const int maxHealth = CLevelScheduler::ScenVar_Get(SCENE_VAR_MAX_HEALTH);
	assert(maxHealth != 0);
	this->lifeInterface.SetValueMax((float)maxHealth);
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
		this->pJamgutCamera_0x15b8 = pCVar3;

		/* Kim.act Jamgut Cossack */
		pCVar3 = pOther->AddCamera(CT_KyaJamgut, pByteCode, "Kim.act Jamgut Cossack");
		this->pIntViewCamera_0x15bc = pCVar3;
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
	float fVar14;
	//ByteCode BStack32;
	undefined4 local_10;
	uint local_c;
	uint local_8;
	uint local_4;
	
	this->field_0xcbc = 0;
	this->field_0x1a48 = 0;
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
	this->lastCheckPointSector = CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].sectorStartIndex;
	//this->field_0xe7c.y = this->field_0xe4c.y;
	//this->field_0xe7c.z = this->field_0xe4c.z;
	//this->field_0xe7c.w = this->field_0xe4c.w;
	//this->x_0xe90 = this->x_0xe60;
	//this->y_0xe94 = this->y_0xe64;
	//this->z_0xe98 = this->z_0xe68;
	//this->field_0xe98 = this->field_0xe68;
	//this->levelDataField1C_0xea0 = this->levelDataField1C_0xe70;
	this->field_0xea0 = this->lastCheckPointSector;
	this->pAnimationController->RegisterBone(this->field_0x157c);
	this->pAnimationController->RegisterBone(0x45544554);
	this->pAnimationController->RegisterBone(this->animKey_0x1588);
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

	pCVar5 = pCameraManager->GetDefGameCamera(CT_KyaJamgut);
	this->pCameraKyaJamgut = pCVar5;

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
	CCameraGame* pGameCamera = reinterpret_cast<CCameraGame*>(this->pMainCamera);
	pGameCamera->cameraConfig.flags = pGameCamera->cameraConfig.flags | 0x20000;

	pCVar5 = this->pCameraKyaJamgut;
	this->field_0x15c4 = pCVar5->fov;

	fVar14 = pCVar5->GetDistance();
	this->field_0x15c8 = fVar14;

	//*(float*)&this->field_0x15cc = (this->pMainCamera)->fov;
	//*(undefined4*)&this->field_0x15d0 = 6.0;
	//ByteCode::~ByteCode(&BStack32, -1);

	this->field_0xf0 = 10.3f;

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
	if (this->field_0x12e0 != -1) {
		this->pAnimationController->RegisterBone(this->animKey_0x158c);
		this->pAnimationController->RegisterBone(this->animKey_0x1590);

		IMPLEMENTATION_GUARD_LOG(
		CFxTail::Init(0.3333333f, (int)&this->fxTrailA.field_0x4, this->objectId);)

		this->pAnimationController->RegisterBone(this->animKey_0x1594);
		this->pAnimationController->RegisterBone(this->field_0x1594);

		IMPLEMENTATION_GUARD_LOG(
		CFxTail::Init(0.3333333f, (int)&this->fxTrailB.field_0x4, this->objectId);)
	}
	this->field_0xee0 = 0;
	local_4 = CLevelScheduler::ScenVar_Get(9);
	DoMessage(this, (ACTOR_MESSAGE)0x62, (MSG_PARAM)local_4);
	if (this->pActorBoomy != (CActorBoomy*)0x0) {
		local_8 = CLevelScheduler::ScenVar_Get(9);
		DoMessage(this->pActorBoomy, (ACTOR_MESSAGE)0x62, (MSG_PARAM)local_8);
	}

	local_c = CLevelScheduler::ScenVar_Get(10);
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

struct HeroActionCallbackData
{
	CActorHeroPrivate::HeroActionParams* pActionParams;
	CActor* pActor;
	float field_0x8;
};

void gHeroActionCallback(CActor* pActor, void* pParams)
{
	CCollision* pCVar1;
	CActor* pParamsActor;
	int* piVar2;
	int iVar3;
	float fVar4;
	int local_40;
	int local_3c;
	int local_38;
	int local_34;
	int local_30;
	int local_2c;
	int local_28;
	int local_24;
	edF32VECTOR4 eStack32;
	int* local_4;

	if ((CActorFactory::gClassProperties[pActor->typeID].flags & 1) != 0) {
		pCVar1 = pActor->pCollisionData;

		IMPLEMENTATION_GUARD(

		pParamsActor = pParams->field_0x4;

		if (pCVar1 == (CCollision*)0x0) {
			fVar4 = (pActor->currentLocation).y;
		}
		else {
			fVar4 = (pCVar1->highestVertex).y;
		}

		if (ABS(fVar4 - (pParamsActor->currentLocation).y) <= 3.0) {
			if ((pCVar1 == (CCollision*)0x0) || (pCVar1->pDynCol->nbTriangles != 0)) {
				edF32Vector4SubHard(&eStack32, &pActor->currentLocation, &pParamsActor->currentLocation);
				fVar4 = edF32Vector4GetDistHard(&eStack32);
			}
			else {
				fVar4 = CCollision::GetObbTreeDistanceFromPosition((edObbTREE*)pCVar1->pObbTree, &pParamsActor->currentLocation);
			}
			if (fVar4 <= pParams->field_0x8) {
				local_4 = &local_40;
				iVar3 = pParamsActor->DoMessage(pActor, 0x12, (uint)local_4);
				if (iVar3 != 0) {
					*pParams->field_0x0 = iVar3;
					pParams->field_0x0[2] = (int)pActor;
					pParams->field_0x8 = fVar4;
					piVar2 = pParams->field_0x0;
					piVar2[4] = local_40;
					piVar2[5] = local_3c;
					piVar2[6] = local_38;
					piVar2[7] = local_34;
					piVar2 = pParams->field_0x0;
					piVar2[8] = local_30;
					piVar2[9] = local_2c;
					piVar2[10] = local_28;
					piVar2[0xb] = local_24;
				}
			}
		})
	}

	return;
}

void CActorHeroPrivate::GetPossibleAction()
{
	bool bVar1;
	uint uVar2;
	HeroActionStateCfg* pHVar3;
	uint uVar4;
	int iVar5;
	CBehaviour* pCVar6;
	StateConfig* pSVar7;
	ulong uVar8;
	long lVar9;
	CActor* puVar10;
	edF32VECTOR4 local_20;

	HeroActionCallbackData params;

	this->heroActionParams.actionId = 0;
	this->heroActionParams.pActor = (CActor*)0x0;
	this->heroActionParams.field_0x10.x = 0.0f;
	this->heroActionParams.field_0x10.y = 0.0f;
	this->heroActionParams.field_0x10.z = 0.0f;
	this->heroActionParams.field_0x10.w = 0.0f;

	uVar2 = TestState_AllowAction(0xffffffff);
	if ((uVar2 == 0) && (uVar8 = GetBehaviourFlags(this->curBehaviourId), (uVar8 & 0x10) == 0)) {
		return;
	}

	params.pActionParams = &this->heroActionParams;
	bVar1 = false;
	puVar10 = (CActor*)0x0;
	local_20.w = this->sphereCentre.w + 2.0f;
	local_20.xyz = this->sphereCentre.xyz;
	params.field_0x8 = 2.0;
	params.pActor = this;
	CScene::ptable.g_ActorManager_004516a4->cluster.ApplyCallbackToActorsIntersectingSphere(&local_20, gHeroActionCallback, &params);

	iVar5 = this->heroActionParams.actionId;
	if (iVar5 == 0) goto LAB_00346f50;

	pHVar3 = GetActionCfg(iVar5);
	uVar2 = pHVar3->field_0x0;
	if ((uVar2 & 1) == 0) {
	LAB_00346e40:
		if (((uVar2 & 2) == 0) || (uVar4 = TestState_IsInTheWind(0xffffffff), uVar4 != 0)) {
			if (((uVar2 & 4) == 0) || (uVar4 = TestState_IsSliding(0xffffffff), uVar4 != 0)) {
				if (((uVar2 & 8) != 0) && (this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y < -0.1f)) {
					this->heroActionParams.actionId = 0;
				}
			}
			else {
				this->heroActionParams.actionId = 0;
			}
		}
		else {
			this->heroActionParams.actionId = 0;
		}
	}
	else {
		iVar5 = this->actorState;
		if (iVar5 == -1) {
			uVar4 = 0;
		}
		else {
			pSVar7 = GetStateCfg(iVar5);
			uVar4 = pSVar7->flags_0x4 & 0x100;
		}

		if (uVar4 != 0) goto LAB_00346e40;

		this->heroActionParams.actionId = 0;
	}

	if (this->heroActionParams.actionId == 0xd) {
		this->heroActionParams.actionId = 0;
		iVar5 = CLevelScheduler::ScenVar_Get(0xd);
		if ((iVar5 == 0) || (pCVar6 = GetBehaviour(8), pCVar6 == (CBehaviour*)0x0)) {
			iVar5 = this->actorState;
			if ((2 < iVar5 - 0x73U) && ((iVar5 != 0x7a && (iVar5 != 0x7d)))) {
				IMPLEMENTATION_GUARD(
				local_4 = 1;
				DoMessage(this->heroActionParams.pActor, 0x12, 1);)
			}
		}
		else {
			puVar10 = this->heroActionParams.pActor;
			bVar1 = true;
		}
	}

LAB_00346f50:
	if (this->heroActionParams.actionId == 0) {
		uVar2 = TestState_IsGripped(0xffffffff);
		if ((uVar2 == 0) || (iVar5 = DetectClimbCeilingFromGrip(&this->heroActionParams.pActor, &this->heroActionParams.field_0x10), iVar5 == 0)) {
			lVar9 = DetectClimbWall(1, &this->heroActionParams.pActor, 0);
			if (lVar9 == 0) {
				iVar5 = DetectClimbCeiling(&this->currentLocation, &this->heroActionParams.pActor);
				if (iVar5 == 0) {
					if (bVar1) {
						this->heroActionParams.actionId = 0xd;
						this->field_0xf50 = puVar10;
					}
					else {
						iVar5 = this->actorState;
						if (iVar5 == -1) {
							uVar2 = 0;
						}
						else {
							pSVar7 = GetStateCfg(iVar5);
							uVar2 = pSVar7->flags_0x4 & 0x100;
						}
						if ((((uVar2 != 0) && (uVar2 = TestState_IsSliding(0xffffffff), uVar2 == 0)) && (this->dynamic.linearAcceleration < 0.1f)) && (iVar5 = CLevelScheduler::ScenVar_Get(9), iVar5 < 1)) {
							this->heroActionParams.actionId = 3;
						}
					}
				}
				else {
					this->heroActionParams.actionId = 7;
				}
			}
			else {
				this->heroActionParams.actionId = 4;
			}
		}
		else {
			this->heroActionParams.actionId = 7;
		}
	}

	if (this->heroActionParams.actionId != 0) {
		uVar2 = TestState_IsCrouched(0xffffffff);
		if ((uVar2 != 0) && (uVar2 = GetSomethingInFrontOf_001473e0(), uVar2 == 0)) {
			this->heroActionParams.actionId = 0;
		}

		pHVar3 = GetActionCfg(this->heroActionParams.actionId);
		if (((pHVar3->field_0x0 & 0x10) != 0) && (uVar8 = GetBehaviourFlags(this->curBehaviourId), (uVar8 & 0x10) != 0)) {
			this->heroActionParams.actionId = 0;
		}
	}

	return;
}

bool gHeroMagicCallback(CActor* pActor, void* pParams)
{
	float fVar1;
	float fVar2;
	float fVar3;
	CLifeInterface* pCVar4;
	int iVar5;

	CActorAutonomous* pHero = reinterpret_cast<CActorAutonomous*>(pParams);

	if (((CActorFactory::gClassProperties[pActor->typeID].flags & 0x100) != 0) &&
		(fVar1 = (pActor->currentLocation).x - (pHero->currentLocation).x,
			fVar2 = (pActor->currentLocation).y - (pHero->currentLocation).y,
			fVar3 = (pActor->currentLocation).z - (pHero->currentLocation).z,
			fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3 <= 25.0f)) {
		if (pActor->typeID == 0x14) {
			pCVar4 = pHero->GetLifeInterfaceOther();
			pCVar4->field_0x10 = 1;
		}

		iVar5 = pHero->DoMessage(pActor, (ACTOR_MESSAGE)0x2f, 0);
		if (iVar5 != 0) {
			return true;
		}
	}

	return false;
}

void CActorHeroPrivate::GetPossibleMagicalTargets(CActorsTable* pTable)
{
	bool bVar1;
	int iVar2;
	int iVar3;
	CActor** ppCVar4;
	CActorsTable* pCVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_20;
	uint local_4;

	this->field_0x1a40 = 0;

	local_20.w = 5.0f;
	local_20.xyz = this->sphereCentre.xyz;

	CScene::ptable.g_ActorManager_004516a4->cluster.GetActorsIntersectingSphereWithCriterion(pTable, &local_20, gHeroMagicCallback, this);
	this->magicInterface.SetHasMagicInteractionAround(pTable->entryCount == 0 ^ 1);

	iVar3 = 0;
	iVar2 = pTable->entryCount;
	if (0 < iVar2) {
		IMPLEMENTATION_GUARD(
		local_4 = 0;
		pCVar5 = pTable;
		while (iVar2 = pTable->entryCount, iVar3 < iVar2) {
			iVar2 = CActor::DoMessage((CActor*)this, pCVar5->aEntries[0], 0x2f, local_4);
			if (iVar2 == 4) {
				pTable->Pop(iVar3);
			}
			else {
				pCVar5 = (CActorsTable*)pCVar5->aEntries;
				iVar3 = iVar3 + 1;
			}
		})
	}

	if (iVar2 != 0) {
		IMPLEMENTATION_GUARD(
		iVar2 = CActorsTable::IsInList((int*)pTable, 9);
		if (iVar2 == 0) {
			iVar2 = CActorsTable::IsInList((int*)pTable, 0x11);
			if (iVar2 == 0) {
				iVar2 = CActorsTable::IsInList((int*)pTable, 0x14);
				if (iVar2 != 0) {
					iVar2 = 0;
					pCVar5 = pTable;
					if (0 < pTable->entryCount) {
						do {
							if (pCVar5->aEntries[0]->typeID == 0x14) {
								pCVar5 = (CActorsTable*)pCVar5->aEntries;
								iVar2 = iVar2 + 1;
							}
							else {
								pTable->Pop(iVar2);
							}
						} while (iVar2 < pTable->entryCount);
					}
					this->field_0x1a40 = 1;
				}
			}
			else {
				iVar2 = 0;
				pCVar5 = pTable;
				if (0 < pTable->entryCount) {
					do {
						if (pCVar5->aEntries[0]->typeID == 0x11) {
							pCVar5 = (CActorsTable*)pCVar5->aEntries;
							iVar2 = iVar2 + 1;
						}
						else {
							pTable->Pop(iVar2);
						}
					} while (iVar2 < pTable->entryCount);
				}
				this->field_0x1a40 = 2;
			}
		}
		else {
			iVar2 = pTable->entryCount;
			iVar3 = 0;
			pCVar5 = pTable;
			if (0 < iVar2) {
				do {
					bVar1 = (*pCVar5->aEntries[0]->pVTable->IsKindOfObject)(pCVar5->aEntries[0], 0x10);
					if (bVar1 == false) {
						pTable->Pop(iVar3);
					}
					else {
						pCVar5 = (CActorsTable*)pCVar5->aEntries;
						iVar3 = iVar3 + 1;
					}
					iVar2 = pTable->entryCount;
				} while (iVar3 < iVar2);
			}
			iVar3 = 0;
			pCVar5 = pTable;
			if (0 < iVar2 + -1) {
				do {
					iVar6 = iVar3 + 1;
					if (iVar6 < iVar2) {
						ppCVar4 = pTable->aEntries + iVar3;
						do {
							if ((float)ppCVar4[1][8].pClusterNode - (float)ppCVar4[1][8].pMeshNode <
								(float)pCVar5->aEntries[0][8].pClusterNode - (float)pCVar5->aEntries[0][8].pMeshNode) {
								pTable->Swap(iVar3, iVar6);
							}
							iVar2 = pTable->entryCount;
							iVar6 = iVar6 + 1;
							ppCVar4 = ppCVar4 + 1;
						} while (iVar6 < iVar2);
					}
					iVar3 = iVar3 + 1;
					pCVar5 = (CActorsTable*)pCVar5->aEntries;
				} while (iVar3 < iVar2 + -1);
			}
			iVar2 = 0;
			fVar7 = (float)CActorHero::GetMagicalForce((CActorHero*)this);
			if (0 < pTable->entryCount) {
				do {
					fVar8 = (float)pTable->aEntries[iVar2][8].pClusterNode - (float)pTable->aEntries[iVar2][8].pMeshNode;
					if (fVar8 <= fVar7) {
						fVar7 = fVar7 - fVar8;
						iVar2 = iVar2 + 1;
					}
					else {
						if (fVar7 == 0.0) {
							pTable->Pop(iVar2);
						}
						else {
							fVar7 = 0.0;
							iVar2 = iVar2 + 1;
						}
					}
				} while (iVar2 < pTable->entryCount);
			}
			this->field_0x1a40 = 3;
		})
	}

	return;
}

bool CActorHeroPrivate::AbleTo_AttackByBoomyBlow()
{
	CActorBoomy* pCVar1;
	int iVar2;
	bool bVar3;
	uint uVar4;
	StateConfig* pSVar5;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar3 = true;

		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar3 = false;
		}

		if ((((bVar3) && (uVar4 = TestState_AllowAttack(0xffffffff), uVar4 != 0)) &&
			(bVar3 = EvolutionBoomyCanLaunch(), bVar3 != false)) && (((1 < this->actorState - 0x11aU && (uVar4 = TestState_00132b90(0xffffffff), uVar4 == 0)) &&
				(this->field_0x1bb0 != 0x0)))) {
			IMPLEMENTATION_GUARD(
			iVar2 = this->actorState;
			if (iVar2 == -1) {
				uVar4 = 0;
			}
			else {
				pSVar5 = GetStateCfg(iVar2);
				uVar4 = pSVar5->flags_0x4 & 0x100;
			}

			if (((uVar4 != 0) && (this->field_0x1a48 == 0)) && ((this->curBehaviourId != 8 &&
					((((iVar2 = this->boomyState_0x1b70, iVar2 == 0 || (iVar2 == 5)) || (iVar2 == 2)) || (iVar2 == 3))))))
			{
				return true;
			})
		}
	}
	return false;
}

bool CActorHeroPrivate::AbleTo_AttackByBoomyLaunch()
{
	CActorBoomy* pCVar1;
	int iVar2;
	bool bVar3;
	uint uVar4;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar3 = true;
		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar3 = false;
		}
		if (((bVar3) && (uVar4 = TestState_AllowAttack(0xffffffff), uVar4 != 0)) &&
			(bVar3 = true, this->field_0x1a48 == 0)) goto LAB_00136c48;
	}
	bVar3 = false;
LAB_00136c48:
	if ((bVar3) && (bVar3 = EvolutionBoomyCanLaunch(), bVar3 != false)) {
		IMPLEMENTATION_GUARD(
		iVar2 = this->actorState;
		if (((1 < iVar2 - 0x11aU) &&
			((iVar2 < 0xdc || 0xde < iVar2 &&
				(uVar4 = CActorHero::TestState_00132b90((CActorHero*)this, 0xffffffff), uVar4 == 0)))) &&
			((iVar2 = this->boomyState_0x1b70, iVar2 == 0 || ((iVar2 == 2 || (iVar2 == 3)))))) {
			return true;
		})
	}

	return false;
}

bool CActorHeroPrivate::AbleTo_AttackByBoomySnipe()
{
	CActorBoomy* pCVar1;
	int iVar2;
	bool bVar3;
	uint uVar4;
	StateConfig* pSVar5;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar3 = true;
		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar3 = false;
		}
		if (((bVar3) && (uVar4 = TestState_AllowAttack(0xffffffff), uVar4 != 0)) &&
			(bVar3 = true, this->field_0x1a48 == 0)) goto LAB_00136b08;
	}
	bVar3 = false;
LAB_00136b08:
	if (((bVar3) && (bVar3 = EvolutionBoomyCanSnipe(), bVar3 != false)) && (iVar2 = this->actorState, iVar2 - 0x11aU < 2)) {
		IMPLEMENTATION_GUARD(
		if (iVar2 == -1) {
			uVar4 = 0;
		}
		else {
			pSVar5 = GetStateCfg(iVar2);
			uVar4 = pSVar5->flags_0x4;
		}
		if ((((uVar4 & 0x100) != 0) &&
			(uVar4 = CActorHero::TestState_IsOnAToboggan((CActorHero*)this, 0xffffffff), uVar4 == 0)) &&
			((iVar2 = this->boomyState_0x1b70, iVar2 == 0 || ((iVar2 == 2 || (iVar2 == 3)))))) {
			return true;
		})
	}

	return false;
}

bool CActorHeroPrivate::AbleTo_AttackByBoomyControl()
{
	CActorBoomy* pCVar1;
	int iVar2;
	CActor* pCVar3;
	bool bVar4;
	uint uVar5;
	StateConfig* pSVar6;
	ulong uVar7;

	pCVar1 = this->pActorBoomy;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar2 = pCVar1->actorState;
		bVar4 = true;
		if ((iVar2 != 5) && (iVar2 != 9)) {
			bVar4 = false;
		}
		if (((bVar4) && (uVar5 = TestState_AllowAttack(0xffffffff), uVar5 != 0)) &&
			(bVar4 = true, this->field_0x1a48 == 0)) goto LAB_00136998;
	}
	bVar4 = false;
LAB_00136998:
	if ((((bVar4) && (bVar4 = EvolutionBoomyCanControl(), bVar4 != false)) && ((uVar7 = TestState_AllowInternalView(0xffffffff), uVar7 != 0 &&
			((uVar5 = TestState_AllowMagic(0xffffffff), uVar5 != 0 && (uVar5 = TestState_00132b90(0xffffffff), uVar5 == 0)))))) &&
		(iVar2 = this->actorState, 1 < iVar2 - 0x11aU)) {
		IMPLEMENTATION_GUARD(
		if (iVar2 == -1) {
			uVar5 = 0;
		}
		else {
			pSVar6 = GetStateCfg(iVar2);
			uVar5 = pSVar6->flags_0x4;
		}
		if ((((uVar5 & 0x100) != 0) && (this->field_0x1574 == 0.0)) && (this->boomyState_0x1b70 == 0)) {
			pCVar3 = this->pTiedActor;
			if ((pCVar3 != (CActor*)0x0) && ((pCVar3->actorFieldS & 0x200) == 0)) {
				return false;
			}
			return true;
		})
	}

	return false;
}

EBoomyThrowState CActorHeroPrivate::ManageEnterAttack()
{
	CPlayerInput* pCVar1;
	bool bVar2;
	edF32VECTOR4* peVar3;
	uint uVar4;
	int iVar5;
	EBoomyThrowState EVar6;
	undefined* puVar7;
	uint uVar8;
	CActorHero* pCVar9;
	float fVar10;
	undefined4 uVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	IMPLEMENTATION_GUARD_LOG(APlayer::Func_00133b10(this);)

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar14 = 0.0f;
	}
	else {
		fVar14 = pCVar1->aButtons[9].clickedDuration;
	}

	if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		uVar8 = 0;
	}
	else {
		uVar8 = pCVar1->releasedBitfield & 0x200;
	}

	if (((uVar8 != 0) && (fVar14 < 0.25)) && (bVar2 = AbleTo_AttackByBoomyBlow(), bVar2 != false)) {
		IMPLEMENTATION_GUARD(
		pCVar1 = this->pPlayerInput;
		if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar10 = 0.0;
		}
		else {
			fVar10 = pCVar1->aAnalogSticks[0].y;
		}
		bVar2 = fVar10 != 0.0;
		if (!bVar2) {
			if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar10 = 0.0;
			}
			else {
				fVar10 = pCVar1->aAnalogSticks[0].x;
			}
			bVar2 = fVar10 != 0.0;
		}
		if (bVar2) {
			if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				local_50.x = 0.0;
			}
			else {
				local_50.x = pCVar1->aAnalogSticks[0].x;
			}
			local_50.y = 0.0;
			if ((this->pPlayerInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				local_50.z = 0.0;
			}
			else {
				local_50.z = this->pPlayerInput->aAnalogSticks[0].y;
			}
			local_50.w = 0.0;
			edF32Matrix4GetTransposeHard(&eStack64, &(CScene::ptable.g_CameraManager_0045167c)->field_0x390);
			edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &local_50);
			local_50.y = 0.0;
			local_50.z = local_50.z * -1.0;
			local_50.w = 0.0;
			edF32Vector4NormalizeHard(&local_50, &local_50);
			(this->vector_0x1be0).y = local_50.x;
			(this->vector_0x1be0).z = local_50.y;
			(this->vector_0x1be0).w = local_50.z;
			this->playerSubStruct_64_0x1bf0 = local_50.w;
		}
		else {
			fVar12 = (this->character).characterBase.base.base.rotationQuat.y;
			fVar10 = (this->character).characterBase.base.base.rotationQuat.z;
			fVar13 = (this->character).characterBase.base.base.rotationQuat.w;
			(this->vector_0x1be0).y = (this->character).characterBase.base.base.rotationQuat.x;
			(this->vector_0x1be0).z = fVar12;
			(this->vector_0x1be0).w = fVar10;
			*(float*)&this->playerSubStruct_64_0x1bf0 = fVar13;
		}
		WhatsInVision((CActorHeroPrivate*)this, (int*)&this->field_0x1a60, &(this->vector_0x1be0).y);
		puVar7 = this->field_0x1a60;
		iVar5 = 0;
		pCVar9 = this;
		if (0 < (int)puVar7) {
			do {
				peVar3 = CActor::GetBottomPosition(*(CActor**)&pCVar9->field_0x1a64);
				if (peVar3->y < (this->character).characterBase.base.base.currentLocation.y) {
					this->field_0x1a60.Pop(iVar5);
				}
				else {
					pCVar9 = (CActorHero*)&(pCVar9->character).characterBase.base.base.objectId;
					iVar5 = iVar5 + 1;
				}
				puVar7 = this->field_0x1a60;
			} while (iVar5 < (int)puVar7);
		}
		if ((puVar7 != (undefined*)0x0) || (uVar4 = APlayer::Func_00133fb0(this), uVar4 != 0)) {
			if (this->field_0x1a60 == &DAT_00000001) {
				edF32Vector4SubHard(&eStack96, (edF32VECTOR4*)(*(int*)&this->field_0x1a64 + 0x30),
					&(this->character).characterBase.base.base.currentLocation);
				eStack96.y = 0.0;
				edF32Vector4NormalizeHard(&eStack96, &eStack96);
				fVar14 = edF32Vector4DotProductHard((edF32VECTOR4*)&(this->vector_0x1be0).y, &eStack96);
				if (FLOAT_00434cc8 <= fVar14) {
					(this->field_0x1bbc).x = *(float*)&this->field_0x1a64;
					fVar14 = (this->field_0x1bbc).x;
					fVar13 = *(float*)((int)fVar14 + 0x34);
					fVar10 = *(float*)((int)fVar14 + 0x38);
					uVar11 = *(undefined4*)((int)fVar14 + 0x3c);
					(this->field_0x1bbc).y = *(float*)((int)fVar14 + 0x30);
					(this->field_0x1bbc).z = fVar13;
					(this->field_0x1bbc).w = fVar10;
					*(undefined4*)&this->field_0x1bcc = uVar11;
				}
				else {
					(this->field_0x1bbc).x = 0.0;
					edF32Vector4AddHard((edF32VECTOR4*)&(this->field_0x1bbc).y,
						&(this->character).characterBase.base.base.currentLocation,
						&(this->character).characterBase.base.base.rotationQuat);
				}
			}
			else {
				(this->field_0x1bbc).x = 0.0;
				edF32Vector4AddHard((edF32VECTOR4*)&(this->field_0x1bbc).y,
					&(this->character).characterBase.base.base.currentLocation,
					&(this->character).characterBase.base.base.rotationQuat);
			}
			return BTS_Melee;
		})
	}

	if (this->field_0x1b78 != 4) {
		if (((uVar8 != 0) && (fVar14 < 0.25f)) && (iVar5 = AbleTo_AttackByBoomyLaunch(), iVar5 != 0)) {
			return BTS_StandardThrow;
		}

		if ((0.0f < fVar14) && (bVar2 = AbleTo_AttackByBoomySnipe(), bVar2 != false)) {
			return BTS_TargettedThrow;
		}

		if ((this->pPlayerInput == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar10 = 1000000.0f;
		}

		else {
			fVar10 = this->pPlayerInput->aButtons[9].pressedDuration;
		}

		if (((0.25f <= fVar14) && (fVar10 < 2.25f)) &&
			(bVar2 = AbleTo_AttackByBoomyControl(), bVar2 != false)) {
			return BTS_ControlledThrow;
		}
	}

	if ((this->boomyState_0x1b70 != 10) || (EVar6 = BTS_Unknown_6, uVar8 == 0)) {
		if ((this->boomyState_0x1b70 == 2) && (uVar8 != 0)) {
			this->field_0x1b6c = 1;
		}

		EVar6 = BTS_None;
	}

	return EVar6;
}

bool CActorHeroPrivate::AccomplishAction(int bUpdateActiveActionId)
{
	CPlayerInput* pCVar1;
	uint uVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	if (bUpdateActiveActionId == 0) {
		this->heroActionParams.activeActionId = this->heroActionParams.actionId;
	}

	switch (this->heroActionParams.activeActionId) {
	case 1:
		IMPLEMENTATION_GUARD(
		CActor::DoMessage((CActor*)this, this->heroActionParams.pActor, 0x14, 0);
		*(CActor**)&this->field_0xf14 = this->heroActionParams.pActor;
		SetBehaviour(7, 0x106, 0xffffffff);)
		break;
	case 2:
		IMPLEMENTATION_GUARD(
		SetBehaviour(7, 0xa5, 0xffffffff);)
		break;
	case 3:
		IMPLEMENTATION_GUARD(
		SetBehaviour(7, 0xe0, 0xffffffff);)
		break;
	case 4:
		IMPLEMENTATION_GUARD(
		uVar2 = TestState_IsFalling(0xffffffff);
		if ((uVar2 == 0) && (uVar2 = TestState_IsInTheWind(0xffffffff), uVar2 == 0)) {
			SetBehaviour(7, 0xa9, 0xffffffff);
		}
		else {
			SetBehaviour(7, 0xbb, 0xffffffff);
		})
		break;
	case 5:
	case 0xb:
		break;
	case 6:
		IMPLEMENTATION_GUARD(
		CActor::DoMessage((CActor*)this, this->heroActionParams.pActor, 0x13, 0);
		SetBehaviour(7, 0x103, 0xffffffff);)
		break;
	case 7:
		IMPLEMENTATION_GUARD(
		uVar2 = CActorHero::TestState_IsGripped((CActorHero*)this, 0xffffffff);
		if (uVar2 == 0) {
			uVar2 = CActorHero::TestState_IsFalling((CActorHero*)this, 0xffffffff);
			if (uVar2 == 0) {
				SetBehaviour(7, 0x10f, 0xffffffff);
			}
			else {
				SetBehaviour(7, 0x115, 0xffffffff);
			}
			GripObject(this, (Actor*)this->heroActionParams.pActor);
		}
		else {
			this->field_0x1490.x = this->heroActionParams.field_0x10.x;
			this->field_0x1490.y = this->heroActionParams.field_0x10.y;
			this->field_0x1490.z = this->heroActionParams.field_0x10.z;
			this->field_0x1490.w = this->heroActionParams.field_0x10.w;
			SetBehaviour(7, 0x113, 0xffffffff);
		})
		break;
	case 8:
		IMPLEMENTATION_GUARD(
		fVar5 = this->heroActionParams.field_0x10.y;
		fVar3 = this->heroActionParams.field_0x10.z;
		fVar4 = this->heroActionParams.field_0x10.w;
		this->field_0xf30.x = this->heroActionParams.field_0x10.x;
		this->field_0xf30.y = fVar5;
		this->field_0xf30.z = fVar3;
		this->field_0xf30.w = fVar4;
		fVar5 = this->heroActionParams.field_0x20.y;
		fVar3 = this->heroActionParams.field_0x20.z;
		fVar4 = this->heroActionParams.field_0x20.w;
		this->this->field_0xf40.x = this->heroActionParams.field_0x20.x;
		this->field_0xf40.y = fVar5;
		this->field_0xf40.z = fVar3;
		this->field_0xf40.w = fVar4;
		CActor::DoMessage((CActor*)this, this->heroActionParams.pActor, 0x14, 0);
		SetBehaviour(7, 0x105, 0xffffffff);)
		break;
	case 9:
	case 0xc:
	case 0xe:
		IMPLEMENTATION_GUARD(
		CActor::DoMessage((CActor*)this, this->heroActionParams.pActor, 0x14, 0);)
		break;
	case 10:
		IMPLEMENTATION_GUARD(
		*(undefined4*)&this->field_0x187c = 1;)
		break;
	case 0xd:
		IMPLEMENTATION_GUARD(
		SetBehaviour(7, 0x11c, 0xffffffff);)
		break;
	case 0xf:
		IMPLEMENTATION_GUARD(
		CActor::DoMessage((CActor*)this, this->heroActionParams.pActor, 0x14, 0);
		CActor::PlayAnim((CActor*)this, 0x1a3);
		this->effort = 0.0;
		this->field_0xf24 = (char*)this->heroActionParams.pActor;
		(*(code*)(this->character.characterBase.base.base.pVTable)->SetLookingAtBones)(this, 0x45544554, 0x554f43);
		CAnimation::RegisterBone(this->character.characterBase.base.base.pAnimationController, 0x45544554);
		CAnimation::RegisterBone(this->character.characterBase.base.base.pAnimationController, (uint)&DAT_00554f43);
		(*(code*)(this->character.characterBase.base.base.pVTable)->SetLookingAtOn)(0, this);
		(*(code*)(this->character.characterBase.base.base.pVTable)->SetLookingAt)(0, 0, 0x42fb53d2, this);)
		break;
	default:
		return false;
	}

	if (bUpdateActiveActionId == 0) {
		this->heroActionParams.activeActionId = 0;
	}

	this->field_0xf10 = this->heroActionParams.pActor;

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 != (CPlayerInput*)0x0) && (this->field_0x18dc == 0)) {
		pCVar1->pressedBitfield = pCVar1->pressedBitfield & 0xfffffdff;
	}

	return true;
}

bool CActorHeroPrivate::ManageActions()
{
	CPlayerInput* pCVar1;
	bool bVar2;
	CLifeInterface* pCVar3;
	StateConfig* pSVar4;
	int iVar5;
	int iVar6;
	uint uVar7;
	EBoomyThrowState EVar8;
	long lVar9;
	ulong uVar10;
	CActorsTable* pCVar11;
	float fVar12;
	float fVar13;
	float valueMax;
	CActorsTable local_220;
	CActorsTable local_110;
	undefined4 local_4;

	pCVar3 = GetLifeInterfaceOther();
	pCVar3->field_0x10 = 0;
	iVar5 = this->actorState;
	if (iVar5 == -1) {
		uVar7 = 0;
	}
	else {
		pSVar4 = GetStateCfg(iVar5);
		uVar7 = pSVar4->flags_0x4;
	}

	if (((uVar7 & 1) == 0) && (lVar9 = this->inventory.IsActive(), lVar9 == 0)) {
		iVar5 = CLevelScheduler::ScenVar_Get(0x11);
		iVar6 = CLevelScheduler::ScenVar_Get(0x12);
		valueMax = (float)(iVar5 * iVar6);
		fVar12 = this->magicInterface.GetValueMax();

		if (valueMax != fVar12) {
			iVar5 = CLevelScheduler::ScenVar_Get(0x13);
			if ((float)iVar5 < valueMax) {
				iVar5 = CLevelScheduler::ScenVar_Get(0x13);
				valueMax = (float)iVar5;
				iVar5 = CLevelScheduler::ScenVar_Get(0x13);
				iVar6 = CLevelScheduler::ScenVar_Get(0x12);
				if (iVar6 == 0) {
					trap(7);
				}
				CLevelScheduler::ScenVar_Set(0x11, iVar5 / iVar6);
			}

			this->magicInterface.SetValueMax(valueMax);
			fVar12 = this->magicInterface.GetValue();
			fVar13 = this->magicInterface.GetValueMax();
			if (fVar13 < fVar12) {
				this->magicInterface.SetValue(valueMax);
			}
		}

		uVar10 = GetBehaviourFlags(this->curBehaviourId);
		if (((uVar10 & 0x400) == 0) || (uVar7 = TestState_AllowMagic(0xffffffff), uVar7 == 0)) {
			this->field_0x1a40 = 0;
			this->field_0x1a44 = 0;
		}
		else {
			if (this->field_0x1a44 == 0) {
				local_110.entryCount = 0;
				GetPossibleMagicalTargets(&local_110);
			}
		}

		pCVar1 = this->pPlayerInput;
		if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			uVar7 = 0;
		}
		else {
			uVar7 = pCVar1->pressedBitfield & 0x80;
		}

		if ((uVar7 != 0) && (uVar7 = TestState_AllowMagic(0xffffffff), uVar7 != 0)) {
			this->field_0x1a44 = this->field_0x1a40;
			IMPLEMENTATION_GUARD(
			iVar5 = AccomplishMagic();
			if (iVar5 != 0) {
				return true;
			})
		}

		if (this->field_0x1a44 == 1) {
			pCVar1 = this->pPlayerInput;
			if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				uVar7 = 0;
			}
			else {
				uVar7 = pCVar1->releasedBitfield & 0x80;
			}

			if (uVar7 != 0) {
				IMPLEMENTATION_GUARD(
				local_220.entryCount = 0;
				GetPossibleMagicalTargets(this, &local_220);
				iVar5 = 0;
				if (0 < local_220.entryCount) {
					pCVar11 = &local_220;
					do {
						if (pCVar11->aEntries[0]->typeID == 0x14) {
							local_4 = 0;
							CActor::DoMessage(pCVar11->aEntries[0], 0x10, 0);
						}
						iVar5 = iVar5 + 1;
						pCVar11 = (CActorsTable*)pCVar11->aEntries;
					} while (iVar5 < local_220.entryCount);
				}
				*(undefined4*)&this->field_0x1a40 = 0;
				this->field_0x1a44 = 0;
				(*(this->pVTable)->SetState)((CActor*)this, 0x73, 0xffffffff);)
			}
		}

		uVar10 = GetBehaviourFlags(this->curBehaviourId);
		if (((uVar10 & 0x100) == 0) && (uVar10 = GetBehaviourFlags(this->curBehaviourId), (uVar10 & 0x10) == 0))
		{
			this->heroActionParams.actionId = 0;
			this->heroActionParams.pActor = (CActor*)0x0;
			this->heroActionParams.field_0x10.x = 0.0f;
			this->heroActionParams.field_0x10.y = 0.0f;
			this->heroActionParams.field_0x10.z = 0.0f;
			this->heroActionParams.field_0x10.w = 0.0f;
		}
		else {
			if (this->heroActionParams.activeActionId == 0) {
				GetPossibleAction();
			}
		}

		uVar10 = GetBehaviourFlags(this->curBehaviourId);
		if (((uVar10 & 0x200) != 0) && ((iVar5 = this->heroActionParams.actionId, iVar5 == 0 || (iVar5 == 0xb)))) {
			this->boomyThrowState = BTS_None;
			uVar7 = TestState_AllowAttack(0xffffffff);
			if (uVar7 != 0) {
				if (this->field_0x1a00 == 0) {
					EVar8 = ManageEnterAttack();
					this->boomyThrowState = EVar8;
					if (this->boomyThrowState != BTS_None) {
						this->heroActionParams.actionId = 0xb;
					}
				}
				else {
					pCVar1 = this->pPlayerInput;

					if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar12 = 0.0f;
					}
					else {
						fVar12 = pCVar1->aButtons[9].clickValue;
					}

					if (fVar12 == 0.0f) {
						this->field_0x1a00 = 0;
					}
				}
			}
		}

		pCVar1 = this->pPlayerInput;
		if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			uVar7 = 0;
		}
		else {
			uVar7 = pCVar1->pressedBitfield & 0x200;
		}

		if (uVar7 != 0) {
			this->heroActionParams.activeActionId = this->heroActionParams.actionId;
		}

		pCVar1 = this->pPlayerInput;
		if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			uVar7 = 0;
		}
		else {
			uVar7 = pCVar1->releasedBitfield & 0x200;
		}

		if (uVar7 != 0) {
			IMPLEMENTATION_GUARD(
			this->field_0x1a14 = 0;
			this->field_0x19fc = 0;
			this->field_0x1a00 = 0;
			this->field_0x1b78 = 0;)
		}

		if (this->heroActionParams.actionId == 0xb) {
			uVar10 = GetBehaviourFlags(this->curBehaviourId);
			if ((uVar10 & 0x200) != 0) {
				IMPLEMENTATION_GUARD(
				this->field_0x19fc = this->boomyThrowState;
				bVar2 = AccomplishAttack();
				if (bVar2 != false) {
					return true;
				})
			}
		}
		else {
			uVar10 = GetBehaviourFlags(this->curBehaviourId);
			if (((uVar10 & 0x100) != 0) || (uVar10 = GetBehaviourFlags(this->curBehaviourId), (uVar10 & 0x10) != 0)) {
				bVar2 = false;

				if (this->heroActionParams.actionId == 2) {
					pCVar1 = this->pPlayerInput;
					if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar12 = 0.0f;
					}
					else {
						fVar12 = pCVar1->aButtons[9].clickValue;
					}

					IMPLEMENTATION_GUARD(
					if ((((fVar12 != 0.0f) && (uVar7 = ActorFunc_001328a0((Actor*)this, 0xffffffff), uVar7 == 0)) &&
						(uVar7 = TestState_AllowAction(0xffffffff), uVar7 != 0)) && (this->field_0x1574 == 0.0)) {
						bVar2 = true;
						this->field_0x1a14 = this->field_0x1a10;
					})
				}
				else {
					pCVar1 = this->pPlayerInput;
					if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						uVar7 = 0;
					}
					else {
						uVar7 = pCVar1->pressedBitfield & 0x200;
					}
					if (uVar7 != 0) {
						bVar2 = true;
					}
				}

				if ((bVar2) && (iVar5 = AccomplishAction(1), iVar5 != 0)) {
					return true;
				}
			}
		}
	}
	else {
		this->heroActionParams.actionId = 0;
		this->heroActionParams.pActor = (CActor*)0x0;
		this->heroActionParams.field_0x10.x = 0.0f;
		this->heroActionParams.field_0x10.y = 0.0f;
		this->heroActionParams.field_0x10.z = 0.0f;
		this->heroActionParams.field_0x10.w = 0.0f;

		this->field_0x1a40 = 0;
		this->field_0x1a44 = 0;
	}

	IMPLEMENTATION_GUARD_LOG(
	this->magicInterface.FUN_001dcd70();)
	return false;
}

int CActorHeroPrivate::GetPossibleWind(float param_1, edF32VECTOR4* param_3, CWayPoint* pWayPoint)
{
	bool bVar1;
	uint uVar2;
	undefined4 uVar3;
	int* piVar4;
	int iVar5;
	int iVar6;
	StateConfig* pSVar7;
	int iVar8;

	iVar8 = -1;
	uVar2 = TestState_IsInTheWind(0xffffffff);
	if ((((uVar2 == 0) && (uVar2 = TestState_IsFlying(0xffffffff), uVar2 == 0)) && (iVar6 = this->actorState, iVar6 != STATE_HERO_CAUGHT_TRAP_1)) && (iVar6 != STATE_HERO_CAUGHT_TRAP_2)) {
		if (GetWindState() == (CActorWindState*)0x0) {
			bVar1 = false;
		}
		else {
			iVar6 = GetWindState()->field_0x0;
			if (iVar6 == GetWindState()->field_0x4) {
				bVar1 = true;
			}
			else {
				if (GetWindState()->field_0x4 == 0) {
					bVar1 = false;
				}
				else {
					bVar1 = true;
					if (0.17398384f <= fabs(param_3->y)) {
						bVar1 = false;
					}
				}
			}
		}
		if (bVar1) {
			if (GetWindState() == (CActorWindState*)0x0) {
				bVar1 = false;
			}
			else {
				iVar6 = GetWindState()->field_0x0;
				if (iVar6 == GetWindState()->field_0x4) {
					bVar1 = true;
				}
				else {
					if (GetWindState()->field_0x4 == 0) {
						bVar1 = false;
					}
					else {
						bVar1 = true;
						if (0.17398384f <= fabs(param_3->y)) {
							bVar1 = false;
						}
					}
				}
			}

			if ((!bVar1) || (bVar1 = true, param_1 <= 2.0f)) {
				bVar1 = false;
			}

			if (bVar1) {
				iVar8 = this->actorState;
				uVar2 = 0;

				if (iVar8 != -1) {
					pSVar7 = GetStateCfg(iVar8);
					uVar2 = pSVar7->flags_0x4;
				}

				if ((uVar2 & 0x100) == 0) {
					iVar8 = 0xf5;
				}
				else {
					bVar1 = ColWithAToboggan();
					iVar8 = 0xef;

					if (bVar1 == false) {
						iVar8 = 0xf6;
					}
				}
			}
		}
		else {
			if ((0.001f < param_1) && (iVar8 = 0xf3, pWayPoint == (CWayPoint*)0x0)) {
				iVar8 = 0xf1;
			}
		}

		if ((iVar8 != -1) && (iVar8 != this->actorState)) {
			ConvertSpeedPlayerToSpeedSumForceExt();
		}
	}
	else {
		iVar8 = -1;
	}

	return iVar8;
}

void CActorHeroPrivate::Manage()
{
	CAnimation* pCVar1;
	CPlayerInput* pCVar2;
	//CFrontendDisplay* pCVar3;
	bool bVar4;
	Timer* pTVar5;
	uint uVar6;
	undefined4 uVar7;
	int iVar8;
	StateConfig* pSVar9;
	ulong uVar10;
	edAnmLayer* peVar11;
	CWayPoint* peVar12;
	edF32VECTOR4* peVar13;
	float fVar14;

	if (this->pTiedActor == (CActor*)0x0) {
		if (((this->pCollisionData)->flags_0x4 & 2) != 0) {
			this->flags = this->flags & 0xfff7ffff;
		}
	}
	else {
		if ((((this->flags & 0x20000) == 0) ||
			(((this->pCollisionData)->flags_0x4 & 2) == 0)) ||
			(0.1 < this->distanceToGround)) {
			this->flags = this->flags | 0x80000;
		}
		else {
			this->flags = this->flags & 0xfff7ffff;
		}
	}

	IMPLEMENTATION_GUARD_LOG(
	UpdateNextAdversary(this);
	FUN_00325d00(this);)

	CActorFighter::Manage();

	IMPLEMENTATION_GUARD_LOG(
	pCVar3 = CScene::ptable.g_FrontendManager_00451680;
	if (this == (CActorHeroPrivate*)CActorHero::_gThis) {
		iVar8 = *(int*)&(this->base).field_0xd30;
		if ((iVar8 == 1) || (iVar8 == 2)) {
			(*(code*)(CScene::ptable.g_FrontendManager_00451680)->pManagerFunctionData[1].field_0x4)
				(CScene::ptable.g_FrontendManager_00451680, 0);
			(*(code*)pCVar3->pManagerFunctionData[1].free)(pCVar3);
		}
		else {
			(*(code*)(CScene::ptable.g_FrontendManager_00451680)->pManagerFunctionData[1].field_0x4)
				(CScene::ptable.g_FrontendManager_00451680, this->field_0x1a10);
			(*(code*)pCVar3->pManagerFunctionData[1].free)(pCVar3);
		}
	}

	ManageBoomyState(this);)

	if (0.0f <= this->field_0x1000) {
		IMPLEMENTATION_GUARD(
		pCVar1 = this->pAnimationController;
		iVar8 = CAnimation::PhysicalLayerFromLayerId(pCVar1, 8);
		peVar11 = (pCVar1->anmBinMetaAnimator).base.aAnimData + iVar8;
		if (peVar11->animPlayState == 0) {
			bVar4 = false;
		}
		else {
			bVar4 = (peVar11->field_0xcc & 2) != 0;
		}
		if (bVar4) {
			CActor::SV_UpdateValue
			(0.0, *(float*)&(this->base).field_0x1008, (CActor*)this, this->field_0x1000 = -1.0f);
			pCVar1 = this->pAnimationController;
			fVar14 = this->field_0x1000;
			iVar8 = CAnimation::PhysicalLayerFromLayerId(pCVar1, 8);
			peVar11 = (pCVar1->anmBinMetaAnimator).base.aAnimData + iVar8;
			peVar11->field_0x0 = 3;
			peVar11->field_0x4 = fVar14;
			if ((this->field_0x1000 == 0.0f) && (this->field_0x1000 = -1.0f, (this->base).field_0x1b94 == 0)) {
				iVar8 = CAnimation::PhysicalLayerFromLayerId
				(this->pAnimationController, 8);
				edAnmBinMetaAnimator::SetAnimOnLayer
				(this->pAnimationController, -1, iVar8, 0xffffffff);
			}
		}
		else {
			CActor::SV_UpdateValue
			(1.0, *(float*)&(this->base).field_0x1004, (CActor*)this, this->field_0x1000 = -1.0f);
			pCVar1 = this->pAnimationController;
			fVar14 = this->field_0x1000;
			iVar8 = CAnimation::PhysicalLayerFromLayerId(pCVar1, 8);
			peVar11 = (pCVar1->anmBinMetaAnimator).base.aAnimData + iVar8;
			peVar11->field_0x0 = 3;
			peVar11->field_0x4 = fVar14;
		})
	}

	IMPLEMENTATION_GUARD_LOG(
	ManageInventory(this);)

	if (this->field_0x1554 != 0.0f) {
		pTVar5 = GetTimer();
		this->field_0x1550 = this->field_0x1550 + pTVar5->cutsceneDeltaTime;
	}

	IMPLEMENTATION_GUARD_LOG(
	ManageInternalView(this);)

	pTVar5 = Timer::GetTimer();
	if (this->field_0x155c <= pTVar5->scaledTotalTime) {
		pTVar5 = Timer::GetTimer();
		this->field_0x155c = pTVar5->scaledTotalTime;
		this->field_0x1560 = 0;
	}
	else {
		this->field_0x1560 = this->field_0x1560 + 1 & 7;
	}

	IMPLEMENTATION_GUARD_LOG(
	FxManageGlideTail(this);)

	uVar6 = TestState_IsOnAToboggan(0xffffffff);
	if (uVar6 != 0) {
		IMPLEMENTATION_GUARD_LOG(
		FxManageToboggan(this);)
	}

	bVar4 = ManageActions();
	if (bVar4 == false) {
		uVar10 = GetBehaviourFlags(this->curBehaviourId);
		if ((uVar10 & 0x80) == 0) {
			pCVar2 = this->pPlayerInput;
			if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				bVar4 = false;
			}
			else {
				bVar4 = pCVar2->aButtons[6].clickValue != 0.0f;
				if (bVar4) {
					bVar4 = (pCVar2->pressedBitfield & 0x800) != 0;
				}
			}
			if (((bVar4) && (this->field_0x1558 == 0.0f)) && (this->bCanUseCheats == 1)) {
				this->field_0x1554 = 0.0f;
				this->field_0x1550 = 0.0f;
				SetBehaviour(7, 0xa8, 0xffffffff);
				return;
			}
		}

		if ((this->field_0x1558 <= 0.0f) ||
			(pTVar5 = GetTimer(), pTVar5->scaledTotalTime <= this->field_0x1558)) {
			uVar10 = GetBehaviourFlags(this->curBehaviourId);
			if ((uVar10 & 0x40) == 0) {
				CLifeInterface* pLifeInterface = GetLifeInterface();
				fVar14 = pLifeInterface->GetValue();
				if (fVar14 - this->field_0x2e4 <= 0.0f) {
					iVar8 = this->actorState;
					if (iVar8 == -1) {
						uVar6 = 0;
					}
					else {
						pSVar9 = this->GetStateCfg(iVar8);
						uVar6 = pSVar9->flags_0x4 & 1;
					}

					if (uVar6 == 0) {
						IMPLEMENTATION_GUARD(
						uVar7 = GetWindState();
						peVar13 = (edF32VECTOR4*)0x0;
						if (uVar7 != 0) {
							iVar8 = GetWindState();
							peVar13 = (edF32VECTOR4*)(iVar8 + 0x40);
						}
						uVar7 = GetWindState();
						if (uVar7 == 0) {
							fVar14 = 0.0;
						}
						else {
							iVar8 = GetWindState();
							fVar14 = *(float*)(iVar8 + 0x38);
						}
						uVar7 = GetWindState();
						peVar12 = (edF32VECTOR4*)0x0;
						if (uVar7 != 0) {
							iVar8 = GetWindState();
							peVar12 = *(edF32VECTOR4**)(iVar8 + 0x2c);
						}
						iVar8 = GetPossibleWind(fVar14, this, peVar13, peVar12);
						if (iVar8 != -1) {
							(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x99, -1);
						})
					}
				}
			}

			bVar4 = TestState_IsInCheatMode();
			if (bVar4 == false) {
				CLifeInterface* pLifeInterface = GetLifeInterface();
				fVar14 = pLifeInterface->GetValue();
				bVar4 = fVar14 - this->field_0x2e4 <= 0.0f;
				if (!bVar4) {
					iVar8 = this->actorState;
					if (iVar8 == -1) {
						uVar6 = 0;
					}
					else {
						pSVar9 = GetStateCfg(iVar8);
						uVar6 = pSVar9->flags_0x4 & 1;
					}
					bVar4 = uVar6 != 0;
				}
				if ((!bVar4) && (this->field_0x1558 <= 0.0)) {
					iVar8 = this->actorState;
					if (iVar8 == -1) {
						uVar6 = 0;
					}
					else {
						pSVar9 = GetStateCfg(iVar8);
						uVar6 = pSVar9->flags_0x4 & 0x400;
					}

					if ((uVar6 == 0) && (iVar8 = CheckHitAndDeath(), iVar8 == 0)) {
						uVar10 = GetBehaviourFlags(this->curBehaviourId);
						if ((uVar10 & 0x40) == 0) {
							CActorWindState* pWindState = GetWindState();
							peVar13 = (edF32VECTOR4*)0x0;
							if (pWindState != (CActorWindState*)0x0) {
								peVar13 = &GetWindState()->field_0x40;
							}

							pWindState = GetWindState();
							if (pWindState == (CActorWindState*)0x0) {
								fVar14 = 0.0f;
							}
							else {
								fVar14 = GetWindState()->field_0x38;
							}

							pWindState = GetWindState();
							peVar12 = (CWayPoint*)0x0;
							if (pWindState != (CActorWindState*)0x0) {
								peVar12 = GetWindState()->field_0x2c;
							}

							iVar8 = GetPossibleWind(fVar14, peVar13, peVar12);
							if (iVar8 != -1) {
								SetBehaviour(7, iVar8, -1);
								return;
							}
						}

						uVar10 = GetBehaviourFlags(this->curBehaviourId);
						if (((uVar10 & 0x20) == 0) && (uVar6 = TestState_IsOnAToboggan(0xffffffff), uVar6 == 0)) {			
							fVar14 = GetLifeInterface()->GetValue();
							bVar4 = fVar14 - this->field_0x2e4 <= 0.0f;

							if (!bVar4) {
								iVar8 = this->actorState;
								if (iVar8 == -1) {
									uVar6 = 0;
								}
								else {
									pSVar9 = GetStateCfg(iVar8);
									uVar6 = pSVar9->flags_0x4 & 1;
								}

								bVar4 = uVar6 != 0;
							}

							if ((!bVar4) && (bVar4 = CanEnterToboggan(), bVar4 != false)) {
								SetBehaviour(7, 0xe8, 0xffffffff);
								return;
							}
						}

						IMPLEMENTATION_GUARD_LOG(
						ActorFunc_00347660((CActorHero*)this);
						ActorFunc_00347290((CActorHero*)this);)
					}
				}
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			(*(code*)(this->pVTable)->field_0x16c)(this);)
		}
	}
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
	int iVar2;
	StateConfig* pStateCfg;
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

	const uint inHitState = TestState_IsInHit(0xffffffff);

	if (inHitState != 0) {
		CLifeInterface* pLifeInterface = GetLifeInterface();
		fVar8 = pLifeInterface->GetValue();
		if ((fVar8 <= 0.0f) || (0.0f < this->field_0x2e4)) {
			IMPLEMENTATION_GUARD(
			newState = ChooseStateDead(this, 0xc, 4, 1);
			animType = AT_None;)
		}
	}

	if ((currentState != newState) && (uVar6 = GetBehaviourFlags(this->curBehaviourId), (uVar6 & 0x200) != 0)) {
		GetStateHeroFlags(newState);
		//::EmptyFunction();
	}

	bFightRelated = this->IsFightRelated(this->curBehaviourId);

	if ((bFightRelated != false) && (newState != AS_None)) {
		IMPLEMENTATION_GUARD(
		animType = ActorStateFunc_00327470((Actor*)this, newState, animType);)
	}

	const uint stateHeroFlags = GetStateHeroFlags(newState);
	const uint onToboggan = TestState_IsOnAToboggan(stateHeroFlags);
	if (((onToboggan != 0) && (iVar2 = CLevelScheduler::ScenVar_Get(0x10), 0 < iVar2)) && (newState != 0xef)) {
		pStateCfg = GetStateCfg(newState);
		animType = pStateCfg->animId + 0xa;
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

	this->heroFlags = GetStateHeroFlags(this->actorState);
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
		CLevelScheduler::ScenVar_Get(0xd);
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

bool CActorHeroPrivate::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	CActorBoomy* pCVar1;
	float fVar2;
	edF32VECTOR4* peVar3;
	int iVar4;
	float* pfVar5;

	CActorFighter::CarriedByActor(pActor, m0);

	peVar3 = &this->field_0x1490;
	edF32Matrix4MulF32Vector4Hard(peVar3, m0, peVar3);

	IMPLEMENTATION_GUARD_LOG(
	peVar3 = &this->field_0x1470;
	edF32Matrix4MulF32Vector4Hard(peVar3, m0, peVar3);
	peVar3 = &this->field_0x1480;
	edF32Matrix4MulF32Vector4Hard(peVar3, m0, peVar3);)

	peVar3 = &this->bounceLocation;
	edF32Matrix4MulF32Vector4Hard(peVar3, m0, peVar3);
	peVar3 = &this->bounceLocation;
	this->bounceLocation.y = 0.0f;
	edF32Vector4NormalizeHard(peVar3, peVar3);
	peVar3 = &this->field_0x1460;
	edF32Matrix4MulF32Vector4Hard(peVar3, m0, peVar3);
	peVar3 = &this->field_0x1460;
	IMPLEMENTATION_GUARD_LOG(
	*(undefined4*)&this->field_0x1464 = 0;)
	edF32Vector4NormalizeHard(peVar3, peVar3);
	pCVar1 = this->pActorBoomy;
	IMPLEMENTATION_GUARD_LOG(
	pfVar5 = (float*)&pCVar1->field_0x610;
	if (pCVar1 != (CActorBoomy*)0x0) {
		iVar4 = 8;
		do {
			iVar4 = iVar4 + -1;
			fVar2 = m0->ab;
			*pfVar5 = m0->aa;
			m0 = (edF32MATRIX4*)&m0->ac;
			pfVar5[1] = fVar2;
			pfVar5 = pfVar5 + 2;
		} while (0 < iVar4);
	})
	return true;
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
	CLevelScheduler* pLVar7;
	//FrontendManager* pFVar8;
	bool bVar9;
	uint uVar10;
	CLifeInterface* pCVar11;
	StateConfig* pAVar12;
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

	pLVar7 = CLevelScheduler::gThis;
	if (msg == 0x8c) {
		IMPLEMENTATION_GUARD(
		iVar13 = CLevelScheduler::ScenVar_Get(SCENE_VAR_MAX_HEALTH);
		(*(code*)(this->lifeInterface.pVtable)->SetValue)
			((float)iVar13, &this->lifeInterface);
		fVar25 = (float)(*(code*)(this->lifeInterface.pVtable)->GetValue)
			(&this->lifeInterface);
		fVar26 = (float)(**(code**)(*(int*)&this->field_0xab8 + 0x24))(&this->field_0xab8);
		fVar28 = (float)(**(code**)(*(int*)&this->field_0xad8 + 0x24))(&this->field_0xad8);
		CLevelScheduler::FUN_002db590(fVar25, pLVar7, (int)fVar26, (int)fVar28);
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
		FUN_001dc5c0(&this->field_0xadc);)
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
		if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
			ActorTimeFunc_00325c40((float)(int)pMsgParam, (Actor*)this, 1);
			return 1;
		})
		return 0;
	}
	if (msg == 0x81) {
		IMPLEMENTATION_GUARD(
		*(undefined4*)&this->field_0x142c = 1;)
		return 1;
	}
	if (msg == 0x80) {
		IMPLEMENTATION_GUARD(
		*(undefined4*)&this->field_0x142c = 0;)
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
		if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
			if (*(int*)&this->field_0xd28 == 0) {
				if (CLevelScheduler::gThis->currentLevelID == 0xd) {
					if (pMsgParam == (void*)0xb5f) {
						CLevelScheduler::ScenVar_Set(9, 0);
					}
					else {
						if (pMsgParam == (void*)0x452) {
							CLevelScheduler::ScenVar_Set(9, 0);
							CLevelScheduler::ScenVar_Set(10, 0x3f);
						}
						else {
							if (pMsgParam == (void*)0x8a0) {
								CLevelScheduler::ScenVar_Set(9, 1);
								CLevelScheduler::ScenVar_Set(10, 0);
							}
						}
					}
				}
			}
			else {
				*(undefined4*)&this->field_0xd30 = 2;
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
		if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
			iVar13 = *(int*)&this->field_0xd28;
			if (iVar13 != 0) {
				iVar14 = CLevelScheduler::gThis->currentLevelID;
				*(undefined4*)&this->field_0xd30 = 1;
				*(undefined*)
					(&this->field_0xd34 + (iVar13 - (int)this->pEventChunk24_0xd24) / 0x28 + iVar14 * 0x10) = 3;
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
		if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
			GameFlags = GameFlags | 0x400;
			CLevelScheduler::SetLevelTimerFunc_002df450(1.0, CLevelScheduler::gThis, 0);
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
		if ((bVar9) || (0.0 < this->field_0x1558)) {
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
			this->field_0xcbc = 0;
			this->field_0x1a48 = 0;
			this->field_0x1a4c = 0;
			this->field_0x1a50 = 0;
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
			this->field_0xea4.valueMax = *pfVar4;
			this->field_0xea4.field_0x10 = fVar25;
			*(float*)&this->field_0xeb8 = fVar26;
			*(undefined4*)&this->field_0xebc = 0x3f800000;
			puVar5 = *(undefined4**)((int)pMsgParam + 4);
			uVar21 = puVar5[1];
			uVar27 = puVar5[2];
			*(undefined4*)&this->field_0xec0 = *puVar5;
			*(undefined4*)&this->field_0xec4 = uVar21;
			*(undefined4*)&this->field_0xec8 = uVar27;
			*(undefined4*)&this->field_0xecc = 0;
			*(undefined4*)&this->field_0xed0 = *(undefined4*)((int)pMsgParam + 8);
			(*(code*)(this->pVTable)->LifeRestore)();
			FUN_001b9400((int)CScene::_pinstance, 1);)
			goto LAB_00344ed0;
		}
		if (msg == 0x57) {
			IMPLEMENTATION_GUARD(
			this->field_0xcbc = (int)pSender;
			if (((uint)pMsgParam & 1) == 0) {
				this->field_0x1a48 = 0;
			}
			else {
				this->field_0x1a48 = 1;
			}
			if (((uint)pMsgParam & 4) == 0) {
				this->field_0x1a4c = 0;
			}
			else {
				this->field_0x1a4c = 1;
			}
			if (((uint)pMsgParam & 2) == 0) {
				this->field_0x1a50 = 0;
			}
			else {
				this->field_0x1a50 = 1;
			}
			(*(code*)(CScene::ptable.g_FrontendManager_00451680)->pManagerFunctionData[1].field_0x0)
				(CScene::ptable.g_FrontendManager_00451680, 0, &this->field_0xee4);
			iVar13 = CLevelScheduler::ScenVar_Get(SCENE_VAR_MAX_HEALTH);
			CLifeInterface::SetValueMax((float)iVar13, &this->field_0xee4);
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
			if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
				this->field_0x1554 = 0.0;
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
			if ((!bVar9) && (this->field_0x1558 <= 0.0f)) {
				if (this->field_0x1554 == 0.0f) {
					this->field_0x1554 = 1.401298e-45f;
					*(undefined4*)&this->field_0x1550 = 0;
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
				(this->field_0x1558 <= 0.0)) {
				this->pTrappedByActor = pSender;
				(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x119, 0xffffffff);
				return 1;
			})
			return 0;
		}
		if (msg == 0x40) {
			IMPLEMENTATION_GUARD(
			iVar13 = this->actorState;
			if ((iVar13 == 0x117) || (iVar13 == 0x118)) {
				if (*(int*)&this->field_0xd30 == 1) {
					*(undefined4*)&this->field_0xd30 = 2;
				}
				uVar21 = ChooseStateFall((CActorHero*)this, 0);
				SetState(uVar21, 0xffffffff);
				return 1;
			})
			goto LAB_00344ed0;
		}
		if (msg == 0x3f) {
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

			if (((!bVar9) && (bVar9 = TestState_IsInCheatMode(), bVar9 == false)) && (this->field_0x1558 <= 0.0f)) {
				if (this->actorState == STATE_HERO_CAUGHT_TRAP_1) {
					if (this->pTrappedByActor != pSender) {
						return 0;
					}

					SetBehaviour(7, STATE_HERO_CAUGHT_TRAP_2, 0xffffffff);
				}
				else {
					this->pTrappedByActor = pSender;
					SetBehaviour(7, STATE_HERO_CAUGHT_TRAP_1, 0xffffffff);
				}

				return 1;
			}

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

				this->heroActionParams.actionId = 0;
				this->heroActionParams.pActor = (CActor*)0x0;
				this->heroActionParams.field_0x10.x = 0.0f;
				this->heroActionParams.field_0x10.y = 0.0f;
				this->heroActionParams.field_0x10.z = 0.0f;
				this->heroActionParams.field_0x10.w = 0.0f;

				this->field_0x1a40 = 0;
				this->field_0x1a44 = 0;

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
				(this->field_0x1558 <= 0.0)) {
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
				(this->field_0x1558 <= 0.0)) {
				lVar22 = (*(code*)(this->pVTable)->GetInputManager)(this, 1, 0);
				if (lVar22 != 0) {
					InputManagerFloatFunc_001b6e20(0.0, 0.0, (CPlayerInput*)lVar22);
				}

				this->heroActionParams.actionId = 0;
				this->heroActionParams.pActor = (CActor*)0x0;
				this->heroActionParams.field_0x10.x = 0.0f;
				this->heroActionParams.field_0x10.y = 0.0f;
				this->heroActionParams.field_0x10.z = 0.0f;
				this->heroActionParams.field_0x10.w = 0.0f;

				this->field_0x1a40 = 0;
				this->field_0x1a44 = 0;

				this->field_0x1610 = 1;
				this->field_0x18dc = 1;
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
			if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
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
				SetJumpCfg(0.1, this->runSpeed, this->field_0x1158, 0.0, 0.0, 0, (edF32VECTOR4*)0x0);
				if (this->actorState == 0x10d) {
					SetState(0x10e, -1);
				}
				else {
					SetState(0x10c, -1);
				}
				local_8 = 0;
				CActor::DoMessage((CActor*)this, *(CActor**)&this->field_0xf18, 0x1f, 0);
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
			if ((bVar9) || (0.0 < this->field_0x1558)) {
				return 0;
			}
			uVar10 = FUN_00132a60((int*)this, 0xffffffff);
			if (uVar10 == 0) {
				return 0;
			}
			if ((0.1 < this->dynamic.field_0x44 *
				this->dynamic.currentLocation.y) &&
				((iVar13 = this->actorState, iVar13 == STATE_HERO_JUMP_2_3_STAND || (iVar13 == STATE_HERO_JUMP_2_3_RUN)))) {
				return 0;
			}
			this->field_0x15a4 = (uint)pMsgParam;
			pReceiver = *(CActor**)&this->field_0xf18;
			if ((pReceiver != (CActor*)0x0) && (pReceiver != pSender)) {
				local_4 = 1;
				CActor::DoMessage((CActor*)this, pReceiver, 0x1f, 1);
			}
			*(CActor**)&this->field_0xf18 = pSender;
			uVar10 = CActorHero::TestState_IsFlying((CActorHero*)this, 0xffffffff);
			if (uVar10 == 0) {
				(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x10b, 0xffffffff);
			}
			else {
				this->field_0x11f8 = 0.0f;
				this->field_0x11f4 = 0.0f;
				(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x10d, 0xffffffff);
			})
			return 1;
		}
		if (msg == 0x16) {

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

			if ((((!bVar9) || (uVar10 = TestState_IsInTheWind(0xffffffff), uVar10 != 0)) && (this->field_0x1558 <= 0.0f)) && ((this->flags & 0x800000) == 0)) {
				bVar9 = TestState_IsInCheatMode();
				if (((bVar9 == false) && (iVar13 = this->actorState, iVar13 != 0x117)) && ((iVar13 != 0x118 && ((this->flags & 0x400000) == 0)))) {
					NotifyWindParam* pParam = reinterpret_cast<NotifyWindParam*>(pMsgParam);
					edF32Vector4ScaleHard(pParam->field_0x10, &eStack128, &pParam->field_0x0);
					peVar24 = this->dynamicExt.aVelocity + 2;
					edF32Vector4AddHard(peVar24, peVar24, &eStack128);
					fVar25 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity + 2);
					this->dynamicExt.aVelocityMagnitudes[2] = fVar25;

					CActorWindState* pWindState = GetWindState();
					if (pWindState == (CActorWindState*)0x0) {
						fVar25 = 0.0f;
					}
					else {
						fVar25 = GetWindState()->field_0x38;
					}
					if (fVar25 < 0.001f) {
						fVar25 = pParam->field_0x10;
						pWindState = GetWindState();
						if (pWindState == (CActorWindState*)0x0) {
							bVar9 = false;
						}
						else {
							iVar13 = GetWindState()->field_0x0;
							if (iVar13 == GetWindState()->field_0x4) {
								bVar9 = true;
							}
							else {
								if (GetWindState()->field_0x4 == 0) {
									bVar9 = false;
								}
								else {
									bVar9 = true;
									if (0.17398384f <= fabs(pParam->field_0x0.y)) {
										bVar9 = false;
									}
								}
							}
						}

						if ((!bVar9) || (bVar9 = true, fVar25 <= 2.0f)) {
							bVar9 = false;
						}

						if (!bVar9) {
							this->dynamic.speed = this->dynamic.speed * 0.4f;
						}
					}

					return 1;
				}

				return 1;
			}

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
			if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
				pTVar15 = Timer::GetTimer();
				this->field_0x155c = (float)(int)pMsgParam + pTVar15->scaledTotalTime;
				return 1;
			})
			return 0;
		}
		if (msg == 0x85) {
			IMPLEMENTATION_GUARD(
			iVar14 = CLevelScheduler::ScenVar_Get(SCENE_VAR_MAX_HEALTH);
			iVar13 = CLevelScheduler::ScenVar_Get(0x15);
			iVar14 = iVar14 + iVar13;
			iVar13 = CLevelScheduler::ScenVar_Get(0x16);
			if (iVar13 < iVar14) {
				iVar14 = CLevelScheduler::ScenVar_Get(0x16);
			}
			pCVar11 = (*((CActorHero::_gThis->character).characterBase.base.base.pVTable)->GetLifeInterfaceOther)
				((CActor*)CActorHero::_gThis);
			CLevelScheduler::ScenVar_Set(0x14, iVar14);
			CLifeInterface::SetValueMax((float)iVar14, pCVar11);
			fVar25 = (float)(*(code*)pCVar11->pVtable->GetValue)(pCVar11);
			iVar13 = CLevelScheduler::ScenVar_Get(0x15);
			(*(code*)pCVar11->pVtable->SetValue)(fVar25 + (float)iVar13, pCVar11);
			(*(code*)pCVar11->pVtable->Activate)(pCVar11, 1);)
			goto LAB_00344ed0;
		}
		if (msg != 9) {
			if (msg == 8) {
				IMPLEMENTATION_GUARD(
				CLevelScheduler::FUN_002dadd0(CLevelScheduler::gThis, (int)pMsgParam);)
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
						(this->field_0x1558 <= 0.0)) {
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
						(this->field_0x1558 <= 0.0)) {
						if ((this->flags & 0x800000) == 0) {
							this->field_0x2e4 = 10.0;
							*(CActor**)&this->field_0x1028 = pSender;
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
						(0.0 < this->field_0x1558)) {
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
						return pTVar15->scaledTotalTime < this->field_0x155c ^ 1;)
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
									if ((!bVar9) && (this->field_0x1558 <= 0.0)) {
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
									if ((bVar9) || (0.0 < this->field_0x1558)) {
										return 0;
									}
									uVar10 = ActorFunc_00132c60((Actor*)this, 0xffffffff);
									if ((uVar10 != 0) && (this->field_0x1b78 == 3)) {
										pCVar18 = this->pPlayerInput;
										if ((pCVar18 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
											fVar25 = 0.0;
										}
										else {
											fVar25 = pCVar18->aButtons[9].clickValue;
										}
										if (fVar25 != 0.0) {
											*(undefined4*)&this->field_0x1b64 = 1;
											(*(this->pVTable)->SetState)
												((CActor*)this, 0xd5, 0xffffffff);
											return 1;
										}
									}
									if (this->boomyState_0x1b70 != 3) {
										uVar10 = ActorFunc_00132c60((Actor*)this, 0xffffffff);
										if (uVar10 == 0) {
											FUN_00136e50((int)this);
										}
										else {
											pAnimationController = this->pAnimationController;
											fVar25 = this->field_0x1b98;
											this->field_0x1000 = -1.0f;
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
										SV_GetBoneWorldPosition(this->animKey_0x1588, &eStack48);
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
		piVar17 = (int*)&this->field_0xab8;
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
				fVar25 = GetFloatParam1_0x1c(&this->field_0xab8);
				SetFloatParam1_0x1cToParam2(fVar25 - (float)(int)pMsgParam, &this->field_0xab8);
				if (0.0 < (float)(int)pMsgParam) {
					fVar25 = (float)(**(code**)(*(int*)&this->field_0xab8 + 0x24))(&this->field_0xab8);
					(**(code**)(*(int*)&this->field_0xab8 + 0x20))
						((float)(int)pMsgParam + fVar25, &this->field_0xab8);
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
	StateConfig* pAVar3;
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

	CPlayerInput* pInputManager;

	uVar6 = *param_5;
	lVar10 = (long)(int)uVar6;
	levelId = param_5[1];
	pCVar2 = GetLifeInterface();
	fVar11 = pCVar2->GetValue();

	ACTOR_HERO_LOG(LogLevel::Info, "CActorHeroPrivate::InterpretEvent {}", param_4);
	ACTOR_HERO_LOG(LogLevel::Info, "CActorHeroPrivate::InterpretEvent id: {} param: {}", uVar6, levelId);

	bVar1 = fVar11 - this->field_0x2e4 <= 0.0f;

	ACTOR_HERO_LOG(LogLevel::Info, "CActorHeroPrivate::InterpretEvent this->field_0x2e4: {} fVar11: {} dead: {}", this->field_0x2e4, fVar11, bVar1);

	if (!bVar1) {
		iVar9 = this->actorState;
		if (iVar9 == -1) {
			uVar4 = 0;
		}
		else {
			pAVar3 = GetStateCfg(iVar9);
			uVar4 = pAVar3->flags_0x4 & 1;
		}

		ACTOR_HERO_LOG(LogLevel::Info, "CActorHeroPrivate::InterpretEvent this->actorState: 0x{:x} uVar4: {}", this->actorState, uVar4);

		bVar1 = uVar4 != 0;
	}

	ACTOR_HERO_LOG(LogLevel::Info, "CActorHeroPrivate::InterpretEvent bVar1: {} this->field_0x1558: {} lVar10: {}", bVar1, this->field_0x1558, lVar10);

	if (((!bVar1) && (this->field_0x1558 <= 0.0f)) || (iVar9 = 0, lVar10 == 1)) {
		switch (uVar6) {
		default:
			iVar9 = CActor::InterpretEvent(param_2, (uint)(uint*)param_3, (uint)param_4, param_5);
			break;
		case 1:
			if ((param_3 == 2) && (bVar1 = TestState_IsInCheatMode(), bVar1 == false)) {
				if (levelId == 3) {
					this->field_0x2e4 = 10.0f;
					SetBehaviour(7, 0x9e, 0xffffffff);
					pInputManager = GetInputManager(1, 0);
					if (pInputManager != 0) {
						CPlayerInput::FUN_001b66f0(1.0f, 0.0f, 0.2f, 0.0f, &pInputManager->field_0x40, 0);
					}

					iVar9 = 1;
				}
				else {
					if (levelId == 2) {
						this->field_0x2e4 = 10.0f;
						SetBehaviour(7, 0x9d, 0xffffffff);
						pInputManager = GetInputManager(1, 0);
						if (pInputManager != 0) {
							CPlayerInput::FUN_001b66f0(1.0f, 0.0f, 0.2f, 0.0f, &pInputManager->field_0x40, 0);
						}

						iVar9 = 1;
					}
					else {
						if (levelId == 1) {
							this->field_0x2e4 = 10.0f;
							SetBehaviour(7, 0xa2, 0xffffffff);
							iVar9 = 1;
						}
						else {
							if (levelId == 0) {
								this->field_0x2e4 = 5.0f;

								if (this->curBehaviourId == 8) {
									CLifeInterface* pLifeInterface = GetLifeInterface();
									fVar11 = pLifeInterface->GetValue();
									bVar1 = fVar11 - this->field_0x2e4 <= 0.0f;

									if (!bVar1) {
										iVar9 = this->actorState;

										if (iVar9 == -1) {
											uVar6 = 0;
										}
										else {
											pAVar3 = GetStateCfg(iVar9);
											uVar6 = pAVar3->flags_0x4 & 1;
										}

										bVar1 = uVar6 != 0;
									}

									if (bVar1) {
										return 0;
									}
								}

								SetBehaviour(7, 0xa1, 0xffffffff);
								iVar9 = 1;
							}
							else {
								this->field_0x2e4 = 10.0f;
								SetBehaviour(7, 0x97, 0xffffffff);
								iVar9 = 1;
							}
						}
					}
				}
			}
			else {
				iVar9 = 1;
			}
			break;
		case 2:
		case 3:
			ACTOR_HERO_LOG(LogLevel::Info, "CActorHeroPrivate::InterpretEvent 2/3 Switch to Sector: {} - {}", param_3, levelId);
			if ((param_3 == 2) && (levelId != 0xffffffff)) {
				CScene::ptable.g_SectorManager_00451670->SwitchToSector(levelId, lVar10 == 2);
			}
			iVar9 = 1;
			break;
		case 5:
			IMPLEMENTATION_GUARD(
			if (param_3 == 2) {
				CLevelScheduler::Level_Teleport
				(CLevelScheduler::gThis, (CActor*)this, levelId, param_5[2], param_5[3], 0xffffffff);
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
						param_2->pEventCollider->field_0x1c->pZone, 0, &eStack4);
				edF32Matrix4GetInverseGaussSoft((edF32MATRIX4*)auStack80, peVar7);
				edF32Vector4NormalizeHard(&eStack112, (edF32VECTOR4*)(auStack80 + 0x10));
				fVar12 = this->dynamic.linearJerk;
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
	
	this->field_0x1174 = 5.0f;

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1178 = 1.9f;
	this->field_0x117c = 1.4f;)
	this->field_0x1180 = 0.4f;
	this->field_0x1184 = 0.1f;
	this->landSpeed = 0.0f;
	this->field_0x1430 = 1.396263f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1570 = 0.6f;)
	this->field_0x1574 = 0.0f;
	return;
}

void CActorHeroPrivate::ResetSlideDefaultSettings()
{
	bool bVar1;

	this->field_0x1068 = 0.5f;
	this->field_0x106c = 12.0f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1070 = 35.0f;)
	this->field_0x1074 = 2.094395f;
	this->field_0x1078 = 0.6f;
	this->field_0x107c = 0.2f;
	this->field_0x1080 = 100.0f;
	this->field_0x1084 = 300.0f;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1088 = 1.0472f;
	this->field_0x108c = 0.95f;)
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
	
	this->field_0x10f0 = 0.0f;
	this->field_0x10f4 = 0.0f;
	this->field_0x10f8 = 0.0f;

	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1104 = 0.0;)
	this->field_0x1108 = 0.0f;
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
	this->field_0x1b68 = 0;)
	this->field_0x1b6c = 0;
	SetBoomyFunc(0);
	this->field_0x1b78 = 0;
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x1b74 = 0;)
	this->field_0x1000 = -1.0f;

	IMPLEMENTATION_GUARD_LOG(
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
	this->field_0x11f4 = 0.0f;
	this->field_0x11f8 = 0.0f;
	this->field_0x11fc = 0.0f;
	//*(undefined4*)&this->field_0x1200 = 0;
	//*(undefined4*)&this->field_0x1208 = 0;
	//*(undefined4*)&this->field_0x1204 = 0;
	//*(undefined4*)&this->field_0x13d8 = 0;
	this->field_0x11ec = 0.0f;
	this->field_0x11e0 = 12.0f;
	this->field_0x11e4 = 12.0f;
	this->field_0x11e8 = 20.0f;
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
		lVar7 = (**(code**)((int)(this->pVTable + 1) + 8))();
		peVar9 = (edF32VECTOR4*)0x0;
		if (lVar7 != 0) {
			iVar4 = (**(code**)((int)(this->pVTable + 1) + 8))(this);
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
					pAVar6 = GetStateCfg(iVar4);
					uVar5 = pAVar6->flags_0x4 & 0x100;
				}
				if (uVar5 == 0) {
					iVar4 = ActorFunc_0014bdf0((float)this->dynamic.field_0x44 *
						this->dynamic.field_0x20.y, (CActorHero*)this);
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
		SetState(iVar4, -1);
		uVar8 = (**(code**)((int)(this->pVTable + 1) + 0x40))(this);
		fVar10 = (float)(**(code**)(*(int*)uVar8 + 0x24))(uVar8);
		bVar3 = fVar10 - *(float*)&this->field_0x2e4 <= 0.0;
		if (!bVar3) {
			iVar2 = this->actorState;
			if (iVar2 == -1) {
				uVar5 = 0;
			}
			else {
				pAVar6 = GetStateCfg(iVar2);
				uVar5 = pAVar6->flags_0x4 & 1;
			}
			bVar3 = uVar5 != 0;
		}
		if (bVar3) {
			iVar4 = ActorStateFunc_0013f0c0((Actor*)this, 0xc, 4, 1);
			SetState(iVar4, 0xffffffff);
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
		fallState = STATE_HERO_FALL_A;

		if (this->field_0x142c != 0) {
			if (this->distanceToGround < 10.3f) {
				this->field_0x1020 = 1;

				fallState = STATE_HERO_FALL_B;

				if (param_2 == 0) {
					fallState = STATE_HERO_FALL_A;
				}
			}
			else {
				this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
				fallState = STATE_HERO_GLIDE_1;
			}
		}
	}
	else {
		this->field_0x1020 = 1;
		fallState = STATE_HERO_JUMP_3_3_STAND;
	}

	return fallState;
}

int CActorHeroPrivate::ChooseStateLanding(float speed)
{
	CPlayerInput* pCVar1;
	edF32VECTOR4* peVar2;
	bool bVar3;
	StateConfig* pAVar4;
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
			landingState = STATE_HERO_SLIDE_A;

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
						landingState = STATE_HERO_JUMP_3_3_STAND;

						if (2.0f < this->dynamic.linearJerk) {
							landingState = STATE_HERO_JUMP_3_3_RUN;
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
			landingState = STATE_HERO_TOBOGGAN;
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
	CCameraManager* iVar4;
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
	this->field_0x1428 = 1;
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
	this->field_0x1560 = 0;
	//*(undefined4*)&this->field_0x1568 = 0;
	//ActorTimeFunc_00325c40(20.0, (Actor*)this, 0);
	this->field_0x1168 = 1;

	
	this->field_0xf00 = this->currentLocation;
	
	this->field_0x1030.x = 0.0f;
	this->field_0x1030.y = 1.0f;
	this->field_0x1030.z = 0.0f;
	this->field_0x1030.w = 0.0f;
	
	this->field_0x10a0.x = 0.0f;
	this->field_0x10a0.y = 1.0f;
	this->field_0x10a0.z = 0.0f;
	this->field_0x10a0.w = 0.0f;
	
	this->normalValue.x = 0.0f;
	this->normalValue.y = 1.0f;
	this->normalValue.z = 0.0f;
	this->normalValue.w = 0.0f;
	
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
	 
	this->field_0x1400.x = 0.0f;
	this->field_0x1400.y = 0.0f;
	this->field_0x1400.z = 1.0f;
	this->field_0x1400.w = 0.0f;
	 
	if (this == (CActorHeroPrivate*)CActorHero::_gThis) {
		iVar4 = (CCameraManager*)CScene::GetManager(MO_Camera);
		iVar4->cameraStack.Reset();
	}

	//*(undefined4*)&this->field_0xf14 = 0;
	this->field_0xf18 = 0;
	this->pSoccerActor = 0;
	this->pTrappedByActor = (CActor*)0x0;
	this->field_0xf24 = (char*)0x0;
	this->trapLinkedBone = 0;
	this->field_0xf50 = (CActor*)0x0;
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
	this->field_0x154c = 0.0f;
	this->field_0x1554 = 0.0f;
	this->field_0x1550 = 0.0f;
	//this->field_0x1544 = 0.0;
	this->field_0x1558 = 0.0;
	//*(undefined4*)&this->field_0x1018 = 0;
	this->field_0xa80 = 0.0f;
	this->field_0xa84 = 0.0f;
	this->field_0xa88 = 0.0f;
	//CVectorDyn::Reset((undefined4*)&this->field_0xf70);
	SetJumpCfg(0.1f, this->runSpeed, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
	this->field_0xf10 = (CActor*)0x0;
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
	this->field_0x11f0 = this->currentLocation.y;
	//_ResetHeroFight(this);
	CCollision* pCollision = this->pCollisionData;
	if (pCollision != (CCollision*)0x0) {
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x10000;
		pCollision = this->pCollisionData;
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x81003;
		pCollision = this->pCollisionData;
		pCollision->flags_0x0 = pCollision->flags_0x0 | 0x4000;
		CActor* pCVar1 = this->field_0xf54;
		if (pCVar1 != (CActor*)0x0) {
			pCollision = pCVar1->pCollisionData;
			pCollision->flags_0x0 = pCollision->flags_0x0 & 0xffffdfff;
		}
	}

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

	this->boomyThrowState = BTS_None;
	//this->field_0x19fc = 0;
	this->field_0x1a00 = 0;
	this->heroActionParams.actionId = 0;
	this->heroActionParams.pActor = (CActor*)0x0;
	this->heroActionParams.field_0x10.x = 0.0f;
	this->heroActionParams.field_0x10.y = 0.0f;
	this->heroActionParams.field_0x10.z = 0.0f;
	this->heroActionParams.field_0x10.w = 0.0f;
	//iVar8 = 0;

	this->field_0x1a40 = 0;
	this->field_0x1a44 = 0;

	this->flags = this->flags & 0xfff7ffff;
	this->field_0x10fc = (CFxHandle*)0x0;
	this->field_0x1100 = (int*)0x0;
	//*(undefined4*)&this->field_0x110c = 0;
	//*(undefined4*)&this->field_0x1110 = 0;
	//*(undefined4*)&this->field_0x1114 = 0;
	this->field_0x13cc = 0.0f;
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
	StateConfig* pAVar7;
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
		StateHeroFlyInit();
	}
	uVar6 = TestState_IsCrouched(this->heroFlags);
	if ((uVar6 == 0) && (uVar6 = TestState_IsCrouched(0xffffffff), uVar6 != 0)) {
		local_10.x = 0.5f;
		local_10.y = 0.5f;
		local_10.z = 0.5f;
		local_20.y = 0.48f;
		local_10.w = 0.0f;
		local_20.w = 1.0f;
		local_20.x = 0.0f;
		local_20.z = 0.0f;
		ChangeCollisionSphere(0.1f, &local_10, &local_20);
	}

	uVar6 = TestState_IsOnAToboggan(this->heroFlags);
	if ((uVar6 == 0) && (uVar6 = TestState_IsOnAToboggan(0xffffffff), uVar6 != 0)) {
		BeginToboggan();
		IMPLEMENTATION_GUARD_LOG(
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
			IMPLEMENTATION_GUARD_LOG(
			CFxHandle::SV_FX_Start(this->field_0x10fc);
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
		StateHeroGlideInit();
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
	case STATE_HERO_FALL_B:
	case STATE_HERO_GRIP_B:
	case STATE_HERO_GRIP_C:
	case STATE_HERO_GRIP_HANG_IDLE:
	case STATE_HERO_GRIP_UP:
	case STATE_HERO_GRIP_GRAB:
	case STATE_HERO_TOBOGGAN_2:
	case STATE_HERO_TOBOGGAN:
	case STATE_HERO_TOBOGGAN_JUMP_2:
	case STATE_HERO_GLIDE_1:
	case STATE_HERO_GLIDE_2:
	case STATE_HERO_GLIDE_3:
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
	case STATE_HERO_ROLL:
		StateHeroRollInit();
		break;
	case STATE_HERO_JUMP_2_3_GRIP:
		StateHeroGripUpToJumpInit();
		StateHeroJump_2_3Init();
		break;
	case STATE_HERO_SLIDE_SLIP_A:
	case STATE_HERO_SLIDE_SLIP_B:
		StateHeroSlideSlipInit();
		break;
	case STATE_HERO_SLIDE_A:
		StateHeroSlideInit(0);
		break;
	case STATE_HERO_SLIDE_B:
		StateHeroSlideInit(1);
		break;
	case STATE_HERO_U_TURN:
		StateHeroUTurnInit();
		break;
	case STATE_HERO_TOBOGGAN_3:
		StateHeroTobogganInit();
		break;
	case STATE_HERO_TOBOGGAN_JUMP_1:
		this->field_0x10f8 = 0.0f;
		break;
	case STATE_HERO_CAUGHT_TRAP_2:
	case STATE_HERO_CAUGHT_TRAP_1:
		this->dynamic.speed = 0.0f;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
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
	case STATE_HERO_FALL_B:
	case STATE_HERO_GRIP_B:
	case STATE_HERO_GRIP_C:
	case STATE_HERO_GRIP_HANG_IDLE:
	case STATE_HERO_GRIP_UP:
	case STATE_HERO_GRIP_GRAB:
	case STATE_HERO_TOBOGGAN_3:
	case STATE_HERO_TOBOGGAN_JUMP_1:
	case STATE_HERO_TOBOGGAN_JUMP_2:
	case STATE_HERO_TOBOGGAN_2:
	case STATE_HERO_TOBOGGAN:
	case STATE_HERO_GLIDE_1:
	case STATE_HERO_GLIDE_2:
	case STATE_HERO_GLIDE_3:
		// Nothing
		break;
	case STATE_HERO_RUN:
		StateHeroRunTerm();
		break;
	case STATE_HERO_STAND:
		StateHeroStandTerm();
		break;
	case STATE_HERO_ROLL:
		StateHeroRunTerm();
		break;
	case STATE_HERO_JUMP_2_3_STAND:
	case STATE_HERO_JUMP_2_3_RUN:
	case STATE_HERO_JUMP_2_3_GRIP:
		StateHeroJump_2_3Term();
		break;
	case STATE_HERO_SLIDE_A:
		StateHeroSlideTerm(0);
		break;
	case STATE_HERO_SLIDE_B:
		StateHeroSlideTerm(1);
		break;
	case STATE_HERO_U_TURN:
		StateHeroUTurnTerm();
		break;
	case STATE_HERO_CAUGHT_TRAP_2:
	{
		if (this->pTrappedByActor != (CActor*)0x0) {
			IMPLEMENTATION_GUARD(
				if (this->trapLinkedBone != 0) {
					UnlinkFromActor();
					(this->pTrappedByActor)->pAnimationController->UnRegisterBone(this->trapLinkedBone);
					this->trapLinkedBone = 0;
				}
			local_140[0] = 0xb;
			local_c = local_140;
			DoMessage(this->pTrappedByActor, (ACTOR_MESSAGE)2, (uint)local_c);
			this->pTrappedByActor = (CActor*)0x0;)
		}
	}
	break;
	case STATE_HERO_CAUGHT_TRAP_1:
	{
		if ((newState != STATE_HERO_CAUGHT_TRAP_2) && (this->pTrappedByActor != (CActor*)0x0)) {
			IMPLEMENTATION_GUARD(
				if (this->trapLinkedBone != 0) {
					UnlinkFromActor();
					(this->pTrappedByActor)->pAnimationController->UnRegisterBone(this->trapLinkedBone);
					this->trapLinkedBone = 0;
				}
			local_140[0] = 0xb;
			local_c = local_140;
			DoMessage(this->pTrappedByActor, (ACTOR_MESSAGE)2, (uint)local_c);
			this->pTrappedByActor = (CActor*)0x0;)
		}
	}
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
	case STATE_HERO_FALL_B:
		StateHeroFall(this->airRotationRate, 0);
		break;
	case STATE_HERO_ROLL:
		StateHeroRoll();
		break;
	case STATE_HERO_GRIP_B:
		StateHeroGrip(0.0f, STATE_HERO_GRIP_HANG_IDLE, 0);
		break;
	case STATE_HERO_GRIP_C:
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
		StateHeroSlideSlip(STATE_HERO_SLIDE_SLIP_C, false, false);
		break;
	case STATE_HERO_SLIDE_SLIP_B:
		StateHeroSlideSlip(STATE_HERO_SLIDE_SLIP_C, true, true);
		break;
	case STATE_HERO_SLIDE_SLIP_C:
		StateHeroSlideSlip(STATE_HERO_STAND, false, true);
		break;
	case STATE_HERO_SLIDE_A:
		StateHeroSlide(1);
		break;
	case STATE_HERO_SLIDE_B:
		StateHeroSlide(0);
		break;
	case STATE_HERO_U_TURN:
		StateHeroUTurn();
		break;
	case STATE_HERO_TOBOGGAN_3:
		StateHeroToboggan(0);
		break;
	case STATE_HERO_TOBOGGAN_JUMP_1:
		StateHeroTobogganJump(0, 1, 0, STATE_HERO_TOBOGGAN);
		break;
	case STATE_HERO_TOBOGGAN_JUMP_2:
		StateHeroTobogganJump(1, 0, 0, STATE_HERO_TOBOGGAN);
		break;
	case STATE_HERO_TOBOGGAN_2:
		StateHeroToboggan(1);
		break;
	case STATE_HERO_TOBOGGAN:
		StateHeroToboggan(1);
		break;
	case STATE_HERO_GLIDE_1:
		StateHeroGlide(0, -1);
		break;
	case STATE_HERO_GLIDE_2:
		StateHeroGlide(1, STATE_HERO_GLIDE_3);
		break;
	case STATE_HERO_GLIDE_3:
		StateHeroGlide(1, -1);
		break;
	case STATE_HERO_CAUGHT_TRAP_2:
	case STATE_HERO_CAUGHT_TRAP_1:
	{
		const int local_4 = 0;
		const uint newTrapBone = DoMessage(this->pTrappedByActor, (ACTOR_MESSAGE)0x4d, 0);
		const uint existingTrapBone = this->trapLinkedBone;

		if (existingTrapBone != newTrapBone) {
			if (existingTrapBone != 0) {
				IMPLEMENTATION_GUARD(
				UnlinkFromActor();
				this->pTrappedByActor->pAnimationController->UnRegisterBone(this->trapLinkedBone);)
			}

			if (newTrapBone != 0) {
				this->pTrappedByActor->pAnimationController->RegisterBone(newTrapBone);
				LinkToActor(this->pTrappedByActor, newTrapBone, 1);
			}

			this->trapLinkedBone = newTrapBone;
		}

		ManageDyn(4.0f, 0, (CActorsTable*)0x0);
		break;
	}
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
	StateConfig* pAVar7;
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
					if (peVar3->animPlayState != STATE_ANIM_NONE) {
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
					if (peVar3->animPlayState != STATE_ANIM_NONE) {
						bSuccess = (peVar3->field_0xcc & 2) != 0;
					}
				}
				else {
					bSuccess = false;
				}
				if (bSuccess) {
					StateConfig* pAnimResult = GetStateCfg(this->actorState);
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

	ManageDyn(4.0f, 0x1002023b, 0);

	iVar4 = DetectGripablePrecipice();
	if (iVar4 == 0) {
		if ((this->pCollisionData->flags_0x4 & 2) == 0) {
			fVar14 = this->field_0x1184;
			if (fVar14 < this->timeInAir) {
				if ((this->pCollisionData->flags_0x4 & 2) == 0) {
					uVar12 = STATE_HERO_FALL_A;

					if (this->field_0x142c != 0) {
						if (this->distanceToGround < 10.3f) {
							uVar12 = STATE_HERO_FALL_A;
							this->field_0x1020 = 1;
						}
						else {
							uVar12 = STATE_HERO_GLIDE_1;
							this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
						}
					}
				}
				else {
					uVar12 = STATE_HERO_JUMP_3_3_STAND;
					this->field_0x1020 = 1;
				}

				this->SetState(uVar12, 0xffffffff);
				return;
			}
		}
		else {
			this->timeInAir = 0.0f;
		}

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
											if (peVar5->animPlayState != STATE_ANIM_NONE) {
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
						this->SetState(STATE_HERO_SLIDE_A, 0xffffffff);
					}
				}
				else {
					this->SetState(STATE_HERO_TOBOGGAN_3, 0xffffffff);
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
				this->SetState(STATE_HERO_JUMP_1_1_STAND, 0xffffffff);
			}
		}
	}
	else {
		this->SetState(200, 0xffffffff);
	}
	return;
}

void CActorHeroPrivate::StateHeroTobogganInit()
{
	float fVar1;
	float fVar2;
	float puVar3;
	float puVar4;
	float puVar5;

	puVar3 = this->normalValue.y;

	puVar4 = 0.50022143f;
	if (0.50022143f <= puVar3) {
		puVar4 = puVar3;
	}

	if (1.0f < puVar4) {
		puVar5 = 1.0f;
	}
	else {
		puVar5 = -1.0f;

		if (-1.0f <= puVar4) {
			puVar5 = puVar4;
		}
	}

	fVar1 = asinf(puVar5);
	fVar2 = edFIntervalUnitDstLERP(fVar1, 1.570796f, 0.5235988f);
	fVar1 = edFIntervalUnitDstLERP(this->field_0xa80, this->field_0x10c0, this->field_0x10c4);

	fVar2 = fVar2 + fVar1;
	fVar1 = 1.0f;
	if ((fVar2 <= 1.0f) && (fVar1 = fVar2, fVar2 < 0.0f)) {
		fVar1 = 0.0f;
	}

	this->field_0x10f4 = fVar1;

	return;
}

void BuildMatrixFromNormalAndSpeed(edF32MATRIX4* m0, edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	m0->rowY = *v0;
	m0->rowZ = *v1;
	edF32Vector4CrossProductHard(&m0->rowX, &m0->rowY, &m0->rowZ);
	edF32Vector4NormalizeHard(&m0->rowX, &m0->rowX);
	edF32Vector4CrossProductHard(&m0->rowZ, &m0->rowX, &m0->rowY);
	edF32Vector4NormalizeHard(&m0->rowZ, &m0->rowZ);
	return;
}

void CActorHeroPrivate::StateHeroToboggan(int param_2)
{
	CCollision* pCVar1;
	CAnimation* pCVar2;
	CPlayerInput* pCVar3;
	CFxHandle* pCVar4;
	int* piVar5;
	edAnmLayer* peVar6;
	bool bVar7;
	StateConfig* pAVar8;
	uint uVar9;
	int iVar10;
	Timer* pTVar11;
	ulong uVar12;
	edF32VECTOR4* v0;
	float fVar13;
	float puVar14;
	float fVar15;
	float fVar16;
	edF32VECTOR4 eStack240;
	edF32VECTOR4 local_e0;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 eStack176;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 local_80;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 local_60;
	edF32MATRIX4 auStack80;
	edF32VECTOR3 local_10;
	CActor* local_4;

	pCVar1 = this->pCollisionData;
	pCVar2 = this->pAnimationController;

	IncreaseEffort(1.0f);

	if ((pCVar1->flags_0x4 & 2) == 0) {
		local_60 = this->collisionContact.location;
	}
	else {
		local_60 = pCVar1->aCollisionContact[1].location;
	}

	fVar13 = edF32Vector4DotProductHard(&this->normalValue, &local_60);

	if (fVar13 < 0.5f) {
		fVar13 = edFIntervalUnitSrcLERP(fVar13, 8.0f, 1.0f);
	}
	else {
		fVar13 = edFIntervalUnitSrcLERP(fVar13, 4.0f, 1.0f);
	}

	fVar15 = this->field_0xa80;
	fVar16 = this->field_0x10c4;
	if (fVar16 < fVar15) {
		fVar15 = edFIntervalLERP(fVar15, fVar16, this->field_0x10cc, 1.5f, 3.0f);
	}
	else {
		fVar15 = edFIntervalLERP(fVar15, this->field_0x10c0, fVar16, 1.0f, 1.5f);
	}

	UpdateNormal(fVar13 * fVar15, &this->normalValue, &local_60);

	if (0.1f < this->dynamic.linearJerk) {
		BuildMatrixFromNormalAndSpeed(&auStack80, &this->normalValue, &this->dynamic.velocityDirectionEuler);
	}
	else {
		BuildMatrixFromNormalAndSpeed(&auStack80, &this->normalValue, &this->rotationQuat);
	}

	edF32Matrix4ToEulerSoft(&auStack80, &local_10, "XYZ");

	this->rotationEuler.xyz = local_10;
	this->rotationQuat = auStack80.rowZ;

	iVar10 = this->actorState;
	uVar9 = 0;

	if (iVar10 != -1) {
		pAVar8 = GetStateCfg(iVar10);
		uVar9 = pAVar8->flags_0x4;
	}
	if (((uVar9 & 1) == 0) && (this->pPlayerInput != (CPlayerInput*)0x0)) {
		pCVar3 = this->pPlayerInput;
		if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			local_80 = gF32Vector4Zero;
		}
		else {
			pCVar3->ComputeForce3D(&local_80);
		}

		edProjectVectorOnPlane(0.0f, &eStack112, &local_80, &auStack80.rowY, 1);
		fVar13 = edF32Vector4DotProductHard(&local_80, &auStack80.rowX);
		fVar13 = -fVar13;
		puVar14 = edF32Vector4DotProductHard(&local_80, &auStack80.rowZ);
	}
	else {
		puVar14 = -1.0f;
		fVar13 = 0.0f;
	}

	SV_UpdatePercent(fVar13, 0.9f, &this->field_0x10f0);

	float puVar17 = this->normalValue.y;
	float puVar16;
	float puVar15 = 0.50022143f;
	if (0.50022143f <= puVar17) {
		puVar15 = puVar17;
	}
	if (1.0f < puVar15) {
		puVar16 = 1.0f;
	}
	else {
		puVar16 = -1.0f;
		if (-1.0f <= puVar15) {
			puVar16 = puVar15;
		}
	}

	fVar15 = asinf(puVar16);
	fVar16 = edFIntervalUnitDstLERP(fVar15, 1.570796f, 0.5235988f);
	fVar15 = edFIntervalUnitDstLERP(this->field_0xa80, this->field_0x10c0, this->field_0x10c4);
	fVar16 = fVar16 + fVar15;
	fVar15 = 1.0f;

	if ((fVar16 <= 1.0f) && (fVar15 = fVar16, fVar16 < 0.0f)) {
		fVar15 = 0.0f;
	}

	SV_UpdatePercent(fVar15, 0.9f, &this->field_0x10f4);
	this->dynamic.speed = 0.0f;
	this->dynamic.rotationQuat = this->dynamic.velocityDirectionEuler;

	SlideOnToboggan(this->field_0x10c0, this->field_0x10cc, fVar13, puVar14, &auStack80.rowZ ,&auStack80.rowX, 0x400d3);

	bVar7 = EvolutionTobogganCanJump();
	if (bVar7 == false) {
		SV_UpdateValue(this->field_0x10c8, this->field_0x10d4, &this->field_0x10cc);
	}
	else {
		SV_UpdateValue(this->field_0x10c4, this->field_0x10d4, &this->field_0x10cc);
	}

	edF32Vector4ScaleHard(this->field_0xa80, &eStack144, &this->dynamic.velocityDirectionEuler);
	edProjectVectorOnPlane(0.0f, &eStack160, &eStack144, &this->normalValue, 0);
	fVar13 = edF32Vector4GetDistHard(&eStack160);
	fVar13 = fVar13 * 1.2f;
	this->field_0x10d0 = fVar13;

	fVar15 = this->field_0x10cc;
	if (fVar15 < fVar13) {
		this->field_0x10d0 = fVar15;
	}
	else {
		fVar15 = this->runSpeed;
		if (fVar13 < fVar15) {
			this->field_0x10d0 = fVar15;
		}
	}

	fVar13 = 0.0f;
	bVar7 = EvolutionTobogganCanJump();
	if (bVar7 != false) {
		fVar13 = 0.2f;
	}

	AdjustLocalMatrixFromNormal(fVar13, &this->normalValue);

	iVar10 = this->actorState;
	uVar9 = 0;
	if (iVar10 != -1) {
		pAVar8 = GetStateCfg(iVar10);
		uVar9 = pAVar8->flags_0x4;
	}
	if ((uVar9 & 1) != 0) {
		if (this->timeInAir <= 2.0f) {
			return;
		}
		IMPLEMENTATION_GUARD(
		(*(code*)(this->pVTable)->field_0x16c)(this);)
		return;
	}

	fVar13 = edFIntervalLERP(local_60.y, 0.93972176f, 0.0f, 1.0f, 1.3f);
	fVar15 = edFIntervalLERP(this->field_0xa80, 0.0f, this->field_0x10c4, 0.8f,	1.2f);

	// Effect
	IMPLEMENTATION_GUARD_LOG(
	pCVar4 = this->field_0x10fc;
	piVar5 = (int*)pCVar4->field_0x4;
	if ((piVar5 != (int*)0x0) && (pCVar4->field_0x0 != 0)) {
		(**(code**)(*piVar5 + 0x40))(fVar13 * fVar15);
	})

	if (param_2 != 0) {
		peVar6 = (pCVar2->anmBinMetaAnimator).aAnimData;
		if ((peVar6->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
			bVar7 = false;
			if (peVar6->animPlayState != STATE_ANIM_NONE) {
				bVar7 = (peVar6->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar7 = false;
		}

		if (bVar7) {
			SetState(STATE_HERO_TOBOGGAN_3, 0xffffffff);
			return;
		}
	}

	if ((pCVar1->flags_0x4 & 2) == 0) {
		if (this->field_0x1184 * 1.5f < this->timeInAir) {
			SetState(STATE_HERO_TOBOGGAN_JUMP_1, 0xffffffff);
			return;
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	if ((pCVar1->flags_0x4 & 1) != 0) {
		local_4 = (CActor*)0x0;
		local_c0 = auStack80.rowZ;

		edF32Vector4ScaleHard(1.2f, &eStack208, &auStack80.rowY);

		bVar7 = GetNormalInFrontOf(((this->pCollisionData)->pObbPrim->field_0x90).z + 0.5f, &local_c0, &eStack208, 0x40001000, &eStack176, &local_4);

		if ((bVar7 == false) || (fVar15 = this->field_0x1430, fVar13 = edF32Vector4DotProductHard(&local_c0, &eStack176), cosf(fVar15) < fVar13 + 1.0f)) {
			fVar15 = this->field_0x1430;
			fVar13 = edF32Vector4DotProductHard(&local_c0, &pCVar1->aCollisionContact->location);
			if (cosf(fVar15) < fVar13 + 1.0f) goto LAB_00329a38;
			bVar7 = TobogganBounceOnWall(&pCVar1->aCollisionContact->location, &this->normalValue, (CActor*)0x0);
		}
		else {
			bVar7 = TobogganBounceOnWall(&eStack176, &this->normalValue, local_4);
		}

		if (bVar7 != false) {
			return;
		}
	}

LAB_00329a38:
	bVar7 = EvolutionTobogganCanJump();
	if (bVar7 != false) {
		pCVar3 = this->pPlayerInput;

		if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			uVar9 = 0;
		}
		else {
			uVar9 = pCVar3->pressedBitfield & 0x10;
		}

		if ((uVar9 != 0) && (0.50022143f < local_60.y)) {
			this->field_0xf00 = this->currentLocation;
			local_e0.y = 500.0f;
			local_e0.x = 0.0f;
			local_e0.z = 0.0f;
			local_e0.w = 0.0f;

			pTVar11 = GetTimer();
			edF32Vector4ScaleHard(0.02f / pTVar11->cutsceneDeltaTime, &eStack240, &local_e0);

			v0 = this->dynamicExt.aVelocity;
			edF32Vector4AddHard(v0, v0, &eStack240);
			fVar13 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
			this->dynamicExt.aVelocityMagnitudes[0] = fVar13;
			SetState(0xe9, 0xffffffff);
			return;
		}
	}

	if (((((pCVar1->flags_0x4 & 2) == 0) || (bVar7 = ColWithAToboggan(), bVar7 != false)) ||
		(uVar12 = ColWithLava(), uVar12 != 0)) ||
		(uVar12 = ColWithCactus(), uVar12 != 0)) {
		pTVar11 = GetTimer();
		this->time_0x153c = pTVar11->scaledTotalTime;
	}
	else {
		pTVar11 = GetTimer();
		if (this->field_0x1184 < pTVar11->scaledTotalTime - this->time_0x153c) {
			if ((this->dynamic.flags & 2) == 0) {
				if (this->runSpeed < this->field_0xa84) {
					SetState(STATE_HERO_ROLL, 0xffffffff);
				}
				else {
					SetState(STATE_HERO_SLIDE_SLIP_B, 0xffffffff);
				}
			}
			else {
				SetState(STATE_HERO_SLIDE_A, 0xffffffff);
			}
		}
	}
	return;
}

void CActorHeroPrivate::StateHeroTobogganJump(int param_2, int param_3, int param_4, int nextState)
{
	CCollision* pCVar1;
	CPlayerInput* this_00;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	bool cVar5;
	uint uVar6;
	Timer* pTVar7;
	int iVar8;
	edF32VECTOR4* peVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	edF32VECTOR4 eStack224;
	edF32VECTOR4 local_d0;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 eStack176;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32MATRIX4 auStack80;
	edF32VECTOR3 local_10;
	CActor* local_4;

	pCVar1 = this->pCollisionData;

	if (param_3 != 0) {
		SV_UpdateValue(1.0f, 2.0f, &this->field_0x10f8);
	}

	edFIntervalLERP(this->field_0xa80, this->field_0x10c0, this->field_0x10c4, 1.0f, 4.0f);

	if (0.1f < this->dynamic.linearJerk) {
		local_60 = this->dynamic.velocityDirectionEuler;
	}
	else {
		local_60 = this->rotationQuat;
	}

	if (param_2 != 0) {
		local_60 = this->field_0x10e0;
	}

	BuildMatrixFromNormalAndSpeed(&auStack80, &this->normalValue, &local_60);
	edF32Matrix4ToEulerSoft(&auStack80, &local_10, "XYZ");

	this->rotationEuler.xyz = local_10;

	this->rotationQuat = auStack80.rowZ;

	if (param_2 == 0) {
		this_00 = this->pPlayerInput;

		if ((this_00 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			local_70 = gF32Vector4Zero;
		}
		else {
			this_00->ComputeForce3D(&local_70);
		}

		edF32Vector4ScaleHard(-1.0f, &eStack128, &auStack80.rowX);
		fVar10 = edF32Vector4DotProductHard(&local_70, &eStack128);
		fVar11 = edF32Vector4DotProductHard(&local_70, &auStack80.rowZ);

		bVar4 = EvolutionTobogganUnknown();
		if (bVar4 == false) {
			fVar12 = edFIntervalLERP(this->field_0xa80, 0.0f, this->field_0x10c4, 6.0f, 4.0f);
		}
		else {
			fVar12 = this->field_0x10c4;

			if ((this->field_0x10cc <= fVar12) || (fVar13 = this->field_0xa80, fVar13 <= fVar12)) {
				fVar12 = edFIntervalLERP(this->field_0xa80, 0.0f, this->field_0x10c4, 10.0f, 6.0f);
			}
			else {
				fVar12 = edFIntervalLERP(fVar13, fVar12, 65.0f, 14.0f, 10.0f);
			}
		}

		uVar6 = TestState_IsInHit(0xffffffff);

		if (uVar6 != 0) {
			fVar12 = fVar12 * 3.0f;
		}

		edF32Vector4ScaleHard(fVar12 * fVar10, &eStack128, &eStack128);
		edF32Vector4ScaleHard(fVar12 * fVar11 * 0.5f, &eStack144, &auStack80.rowZ);
		edF32Vector4AddHard(&eStack128, &eStack128, &eStack144);

		peVar9 = this->dynamicExt.aVelocity;
		edF32Vector4AddHard(peVar9, peVar9, &eStack128);
		fVar10 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
		this->dynamicExt.aVelocityMagnitudes[0] = fVar10;

		if ((((param_3 == 0) && (param_4 != 0)) && (pTVar7 = GetTimer(), pTVar7->scaledTotalTime - this->time_0x153c <= 0.15f)) &&
			(3.0f < this->distanceToGround)) {
			PlayAnim(0xde);
		}

		if ((param_3 == 0) && (pTVar7 = GetTimer(), pTVar7->scaledTotalTime - this->time_0x153c < 0.36f)) {
			edF32Vector4ScaleHard(this->dynamic.linearAcceleration, &eStack160, &this->dynamic.velocityDirectionEuler);
			edProjectVectorOnPlane(0.0, &eStack176, &eStack160, &this->normalValue, 0);
			fVar10 = edF32Vector4GetDistHard(&eStack176);

			if (this->field_0x10d0 < fVar10) {
				pTVar7 = GetTimer();
				fVar10 = this->field_0x10d0;
				edF32Vector4ScaleHard(fVar10 * -0.5f * fVar10 * pTVar7->cutsceneDeltaTime, &eStack176, &eStack176);
				peVar9 = this->dynamicExt.aVelocity + 1;
				edF32Vector4AddHard(peVar9, peVar9, &eStack176);
				fVar10 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity + 1);
				this->dynamicExt.aVelocityMagnitudes[1] = fVar10;
			}
		}
	}

	uVar6 = 0x40020;
	if (this->dynamic.velocityDirectionEuler.y <= 0.0f) {
		uVar6 = 0x40021;
	}

	ManageDyn(4.0f, uVar6, (CActorsTable*)0x0);

	fVar11 = this->field_0x10d0;
	fVar10 = 65.0f;
	if (fVar11 <= 65.0f) {
		fVar10 = fVar11;
	}

	if ((this->field_0x10cc < fVar10) && (fVar10 < this->dynamic.linearAcceleration)) {
		this->dynamicExt.field_0x6c = fVar10;
	}

	fVar10 = 0.0f;
	bVar4 = EvolutionTobogganCanJump();
	if (bVar4 != false) {
		fVar10 = 0.2f;
	}

	AdjustLocalMatrixFromNormal(fVar10, &this->normalValue);

	if (((1.0f <= this->field_0xa88) || (this->field_0x142c != 0)) || (bVar4 = FUN_0014cb60(&this->currentLocation), bVar4 != false)) {
		this->field_0x11f0 = this->currentLocation.y;
	}
	else {
		if (this->distanceToGround < 10.3f) {
			this->field_0x11f0 = this->currentLocation.y;
		}
		else {
			if (100.0f < this->field_0x11f0 - this->currentLocation.y) {
				SetState(0xa1, 0xffffffff);
				return;
			}
		}
	}

	if ((pCVar1->flags_0x4 & 1) != 0) {
		local_4 = (CActor*)0x0;
		local_d0 = auStack80.rowZ;
		edF32Vector4ScaleHard(1.2f, &eStack224, &auStack80.rowY);
		bVar4 = GetNormalInFrontOf(((this->pCollisionData)->pObbPrim->field_0x90).z + 1.0f, &local_d0, &eStack224, 0x40001000, &eStack192, &local_4);

		if ((bVar4 == false) || (fVar11 = this->field_0x1430, fVar10 = edF32Vector4DotProductHard(&local_d0, &eStack192), cosf(fVar11) < fVar10 + 1.0f)) {
			fVar11 = this->field_0x1430;
			fVar10 = edF32Vector4DotProductHard(&local_d0, (edF32VECTOR4*)pCVar1->aCollisionContact);

			if (cosf(fVar11) < fVar10 + 1.0f) goto LAB_00328d38;

			cVar5 = TobogganBounceOnWall(&pCVar1->aCollisionContact->location, &this->normalValue, (CActor*)0x0);
		}
		else {
			cVar5 = TobogganBounceOnWall(&eStack192, &this->normalValue, local_4);
		}

		if (cVar5 != false) {
			return;
		}
	}

LAB_00328d38:
	if (param_4 != 0) {
		pCVar2 = this->pAnimationController;
		peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
		if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
			bVar4 = false;
			if (peVar3->animPlayState != STATE_ANIM_NONE) {
				bVar4 = (peVar3->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar4 = false;
		}

		if (bVar4) {
			SetState(STATE_HERO_TOBOGGAN_JUMP_1, 0xffffffff);
			return;
		}
	}

	bVar4 = ColWithAToboggan();
	if ((bVar4 == false) && ((bVar4 = ColWithLava(), bVar4 == false || (uVar6 = TestState_IsInHit(0xffffffff), uVar6 == 0)))) {
		if ((((pCVar1->flags_0x4 & 2) == 0) || (bVar4 = ColWithAToboggan(), bVar4 != false)) || (bVar4 = ColWithLava(), bVar4 != false)) {
			iVar8 = ChooseStateFall(0);

			if (iVar8 == STATE_HERO_GLIDE_1) {
				if (1.0f < this->timeInAir) {
					SetState(STATE_HERO_GLIDE_1, 0xffffffff);
				}
			}
			else {
				this->timeInAir = 0.0f;
			}
		}
		else {
			if ((this->dynamic.flags & 2) == 0) {
				if (this->runSpeed < this->field_0xa84) {
					SetState(STATE_HERO_ROLL, 0xffffffff);
				}
				else {
					SetState(STATE_HERO_JUMP_3_3_STAND, 0xffffffff);
				}
			}
			else {
				SetState(STATE_HERO_SLIDE_A, 0xffffffff);
			}
		}
	}
	else {
		if ((pCVar1->flags_0x4 & 2) == 0) {
			if ((pCVar1->flags_0x4 & 1) == 0) {
				this->collisionContact.location = pCVar1->aCollisionContact[2].location;
				this->collisionContact.field_0x10 = pCVar1->aCollisionContact[2].field_0x10;
				this->collisionContact.materialFlags = pCVar1->aCollisionContact[2].materialFlags;
				this->collisionContact.nbCollisionsA = pCVar1->aCollisionContact[2].nbCollisionsA;
				this->collisionContact.nbCollisionsB = pCVar1->aCollisionContact[2].nbCollisionsB;
			}
			else {
				this->collisionContact.location = pCVar1->aCollisionContact[0].location;
				this->collisionContact.field_0x10 = pCVar1->aCollisionContact[0].field_0x10;
				this->collisionContact.materialFlags = pCVar1->aCollisionContact[0].materialFlags;
				this->collisionContact.nbCollisionsA = pCVar1->aCollisionContact[0].nbCollisionsA;
				this->collisionContact.nbCollisionsB = pCVar1->aCollisionContact[0].nbCollisionsB;
			}
		}

		Landing();

		if (param_2 == 0) {
			if ((param_3 == 0) || (pTVar7 = GetTimer(), pTVar7->scaledTotalTime - this->time_0x153c < 0.6f)) {
				nextState = STATE_HERO_TOBOGGAN_2;
			}

			pTVar7 = GetTimer();
			fVar10 = edFIntervalLERP(pTVar7->scaledTotalTime - this->time_0x153c, 0.3f, 1.5f, 0.5f, 1.5f);
			this->field_0x1108 = fVar10;
		}
		else {
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;
			this->field_0x1108 = 0.0f;
		}

		SetState(nextState, 0xffffffff);
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
	StateConfig* pAVar7;
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
	edF32VECTOR4 eStack96;
	edF32VECTOR4 slidingForce;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	_ray_info_out local_10;

	pCVar1 = this->pCollisionData;
	iVar2 = CLevelScheduler::gThis->currentLevelID;

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

		ManageDyn(4.0f, 0x1002023b, 0);
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

		ManageDyn(4.0f, 0x1002023b, 0);
	}

	CActor* pSoccer = this->pSoccerActor;
	if ((pSoccer == (CActor*)0x0) || (pSoccer->currentAnimType != 9)) {
		pCVar4 = this->pAnimationController;
		peVar5 = (pCVar4->anmBinMetaAnimator).aAnimData;
		if ((peVar5->currentAnimDesc).animType == pCVar4->currentAnimType_0x30) {
			bVar6 = false;
			if (peVar5->animPlayState != STATE_ANIM_NONE) {
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
		fVar12 = this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0, 0);
		if ((0.26f < fVar12) && (fVar12 = this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0, 0)), fVar12 < 0.3f) {
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

	if ((fVar12 < 0.9f) && (bVar6 = DetectGripablePrecipice(), bVar6 != false)) {
		SetState(200, 0xffffffff);
		return;
	}

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
						uVar11 = STATE_HERO_GLIDE_1;
						this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
					}
				}
			}
			else {
				uVar11 = STATE_HERO_JUMP_3_3_STAND;
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
		SetState(STATE_HERO_TOBOGGAN_3, 0xffffffff);
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
						SetState(STATE_HERO_SLIDE_SLIP_B, 0xffffffff);
						return;
					}
				}

				if (((this->pSoccerActor == (CActor*)0x0) && ((pCVar1->flags_0x4 & 1) != 0)) && (this->field_0x1454 == false)) {
					edF32Vector4ScaleHard(1.2f, &eStack48, &g_xVector);
					fVar12 = ((this->pCollisionData)->pObbPrim->field_0x90).z;
					edF32Vector4AddHard(&eStack96, &this->currentLocation, &eStack48);
					CCollisionRay CStack128 = CCollisionRay(fVar12 + 0.5f, &eStack96, &this->rotationQuat);
					fVar12 = CStack128.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40001000, &eStack32, &local_10);
					if ((fVar12 != 1e+30f) && (fVar13 = this->field_0x1430,	fVar12 = edF32Vector4DotProductHard(&this->rotationQuat, &eStack32) , fVar12 + 1.0f <= cosf(fVar13))) {
						SetState(0x96, 0xffffffff);
						return;
					}
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
						SetState(STATE_HERO_STAND, 0xffffffff);
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
			SetState(STATE_HERO_ROLL, 0xffffffff);
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

		SetState(STATE_HERO_SLIDE_A, 0xffffffff);
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
							SetState(STATE_HERO_RUN, 0xf3);
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
							SetState(STATE_HERO_RUN, 0xf3);
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
					SetState(STATE_HERO_U_TURN, 0xffffffff);
					return;
				}
			}
			if (((pCVar1->flags_0x4 & 1) != 0) || (bVar5)) {
				SetState(STATE_HERO_STAND, 0xffffffff);
			}
			else {
				bVar5 = DetectGripablePrecipice();
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
						SetState(STATE_HERO_TOBOGGAN_3, 0xffffffff);
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
										if (peVar4->animPlayState != STATE_ANIM_NONE) {
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
										SetState(STATE_HERO_ROLL, 0xffffffff);
									}
									else {
										SetState(0x85, 0xffffffff);
									}
								}
							}
							else {
								IMPLEMENTATION_GUARD(
								SetJumpCfgFromGround(this->runSpeed);)
								SetState(STATE_HERO_JUMP_1_1_RUN, 0xffffffff);
							}
						}
						else {
							SetState(STATE_HERO_SLIDE_A, 0xffffffff);
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
	StateConfig* pAVar3;
	uint uVar4;
	edF32VECTOR4* peVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;

	this->field_0xf00 = this->currentLocation;
	this->normalValue = g_xVector;
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

	if (((((uVar4 != 0) && (iVar2 = this->prevActorState, iVar2 != STATE_HERO_JUMP_3_3_STAND)) &&
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

void CActorHeroPrivate::StateHeroSlide(int param_2)
{
	CCollision* pCVar1;
	CPlayerInput* pCVar2;
	bool bVar3;
	bool bVar4;
	int iVar5;
	CBehaviour* pCVar6;
	edF32VECTOR4* peVar7;
	uint uVar8;
	float fVar9;
	float fVar10;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	edF32VECTOR3 local_18;
	float local_c;
	float local_8;
	float local_4;

	pCVar1 = this->pCollisionData;
	pCVar2 = this->pPlayerInput;

	if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		local_c = 0.0f;
		local_8 = 0.0f;
		local_4 = 0.0f;
	}
	else {
		pCVar2->GetPadRelativeToNormal2D(&this->dynamic.velocityDirectionEuler, &local_4, &local_8, &local_c);
	}

	if ((pCVar1->flags_0x4 & 2) == 0) {
		local_70 = this->collisionContact.location;
	}
	else {
		local_70 = pCVar1->aCollisionContact[1].location;
	}

	UpdateNormal(4.0f, &this->normalValue, &local_70);

	if (this->normalValue.y < 0.999f) {
		edProjectVectorOnPlane(0.0f, &eStack128, &CDynamicExt::gForceGravity, &this->normalValue, 0);

		if (0.0f < this->rotationQuat.z * this->normalValue.z + this->rotationQuat.x * this->normalValue.x) {
			this->field_0x1094 = 1;
		}
		else {
			edF32Vector4ScaleHard(-1.0f, &eStack128, &eStack128);
			this->field_0x1094 = 0;
		}

		edF32Vector4NormalizeHard(&eStack128, &eStack128);

		UpdateNormal(4.0f, &this->field_0x10a0, &eStack128);
	}

	BuildMatrixFromNormalAndSpeed(&eStack96, &this->normalValue, &this->field_0x10a0);
	edF32Matrix4ToEulerSoft(&eStack96, &local_18, "XYZ");

	this->rotationEuler.xyz = local_18;

	this->rotationQuat = eStack96.rowZ;

	this->dynamic.speed = 0.0f;
	if (0.001f < local_c) {
		this->dynamic.rotationQuat = this->dynamic.horizontalVelocityDirectionEuler;
	}

	iVar5 = SlideOnGround(this->field_0x1080, this->field_0x1084, local_4, local_8, 0x53);

	if (((this->dynamic.flags & 2) == 0) && (param_2 != 0)) {
		pCVar2 = this->pPlayerInput;
		if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar9 = 0.0f;
		}
		else {
			fVar9 = pCVar2->GetAngleWithPlayerStick(&this->dynamic.velocityDirectionEuler);
		}
		if (fabs(fVar9) <= this->field_0x1074) {
			pCVar2 = this->pPlayerInput;
			if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar9 = 0.0f;
			}
			else {
				fVar9 = pCVar2->aAnalogSticks[0].magnitude;
			}

			if (0.3f < fVar9) {
				ConvertSpeedSumForceExtToSpeedPlayer();
				this->field_0x1048 = 0.0f;
				pCVar2 = this->pPlayerInput;
				if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar9 = 0.0f;
				}
				else {
					fVar9 = pCVar2->aAnalogSticks[0].magnitude;
				}

				if (0.9f <= fVar9) {
					SetState(STATE_HERO_RUN, 0xffffffff);
					return;
				}

				SetState(0x77, 0xffffffff);
				return;
			}
		}

		pCVar2 = this->pPlayerInput;
		if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar9 = 0.0f;
		}
		else {
			fVar9 = pCVar2->aAnalogSticks[0].magnitude;
		}

		if (((0.3f < fVar9) && (this->field_0xa84 < this->field_0x104c + 2.0f)) &&
			(fVar9 = edF32Vector4DotProductHard(&this->rotationQuat, &this->dynamic.velocityDirectionEuler), 0.0f < fVar9)) {
			ConvertSpeedSumForceExtToSpeedPlayer();
			SetState(STATE_HERO_U_TURN, 0xffffffff);
			return;
		}

		if (((pCVar1->flags_0x4 & 1) != 0) || (iVar5 != 0)) {
			SetState(STATE_HERO_STAND, 0xffffffff);
			return;
		}
	}

	bVar3 = DetectGripablePrecipice();
	if (bVar3 == false) {
		if ((pCVar1->flags_0x4 & 2) == 0) {
			if (this->field_0x1184 < this->timeInAir) {
				iVar5 = ChooseStateFall(0);
				SetState(iVar5, 0xffffffff);
				return;
			}
		}
		else {
			this->timeInAir = 0.0;
		}

		bVar4 = ColWithAToboggan();
		bVar3 = false;
		if (bVar4 != false) {
			pCVar6 = GetBehaviour(0xb);
			bVar3 = true;
			if (pCVar6 == (CBehaviour*)0x0) {
				pCVar6 = GetBehaviour(10);
				bVar3 = true;
				if (pCVar6 == (CBehaviour*)0x0) {
					pCVar6 = GetBehaviour(9);
					bVar3 = true;
					if (pCVar6 == (CBehaviour*)0x0) {
						bVar3 = false;
					}
				}
			}
		}

		if (bVar3) {
			SetState(STATE_HERO_TOBOGGAN_3, 0xffffffff);
		}
		else {
			pCVar2 = this->pPlayerInput;
			if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				uVar8 = 0;
			}
			else {
				uVar8 = pCVar2->pressedBitfield & 0x10;
			}
			if ((uVar8 == 0) || (param_2 == 0)) {
				if ((this->dynamic.linearJerk < 0.5f) &&
					((this->dynamic.flags & 2) == 0)) {
					if (param_2 == 0) {
						SetState(0xe6, 0xffffffff);
					}
					else {
						SetState(STATE_HERO_STAND, 0xffffffff);
					}
				}
			}
			else {
				SetJumpCfgFromGround(this->runSpeed);

				if (this->dynamic.velocityDirectionEuler.y <= 0.0f) {
					pCVar2 = this->pPlayerInput;

					if ((pCVar2 == (CPlayerInput*)0x0) || (peVar7 = &pCVar2->lAnalogStick, this->field_0x18dc != 0)) {
						peVar7 = &gF32Vector4Zero;
					}

					fVar9 = edF32Vector4DotProductHard(peVar7, &this->dynamic.velocityDirectionEuler);
					if (fVar9 < 0.0) {
						this->dynamicExt.normalizedTranslation.x = 0.0f;
						this->dynamicExt.normalizedTranslation.y = 0.0f;
						this->dynamicExt.normalizedTranslation.z = 0.0f;
						this->dynamicExt.normalizedTranslation.w = 0.0f;
						this->dynamicExt.field_0x6c = 0.0f;
						pCVar2 = this->pPlayerInput;
						if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
							fVar9 = 0.0;
						}
						else {
							fVar9 = pCVar2->aAnalogSticks[0].magnitude;
						}

						fVar10 = this->field_0x104c;
						fVar9 = this->airHorizontalSpeed * fVar9;
						if (fVar9 <= fVar10) {
							fVar10 = fVar9;
						}

						this->dynamic.speed = fVar10;
						pCVar2 = this->pPlayerInput;
						if ((pCVar2 == (CPlayerInput*)0x0) || (peVar7 = &pCVar2->lAnalogStick, this->field_0x18dc != 0)) {
							peVar7 = &gF32Vector4Zero;
						}

						this->dynamic.rotationQuat = *peVar7;
					}
				}

				SetState(STATE_HERO_JUMP_1_1_RUN, 0xffffffff);
			}
		}
	}
	else {
		SetState(200, 0xffffffff);
	}
	return;
}

void CActorHeroPrivate::StateHeroSlideTerm(int param_2)
{
	if ((this->dynamic.flags & 2) == 0) {
		this->airHorizontalSpeed = this->runSpeed;
	}

	this->flags = this->flags & 0xffffefff;

	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
	RestoreVerticalOrientation();

	if (param_2 != 0) {
		RestoreCollisionSphere(0.2f);
	}
	return;
}

void CActorHeroPrivate::StateHeroUTurnInit()
{
	CPlayerInput* pCVar1;
	edF32VECTOR4* peVar2;
	float fVar3;
	float inMin;

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (peVar2 = &pCVar1->lAnalogStick, this->field_0x18dc != 0)) {
		peVar2 = &gF32Vector4Zero;
	}

	this->field_0x1030.x = peVar2->x;

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (peVar2 = &pCVar1->lAnalogStick, this->field_0x18dc != 0)) {
		peVar2 = &gF32Vector4Zero;
	}
	this->field_0x1030.y = peVar2->y;

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (peVar2 = &pCVar1->lAnalogStick, this->field_0x18dc != 0)) {
		peVar2 = &gF32Vector4Zero;
	}
	this->field_0x1030.z = peVar2->z;

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (peVar2 = &pCVar1->lAnalogStick, this->field_0x18dc != 0)) {
		peVar2 = &gF32Vector4Zero;
	}
	this->field_0x1030.w = peVar2->w;

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar3 = 0.0f;
	}
	else {
		fVar3 = pCVar1->GetAngleWithPlayerStick(&this->dynamic.velocityDirectionEuler);
	}

	if (fVar3 < 0.0f) {
		fVar3 = fVar3 + 6.283185f;
	}

	inMin = this->field_0x1074;
	fVar3 = edFIntervalLERP(fVar3, inMin, (3.141593f - inMin) * 2.0f + inMin, 1.3f, 0.7f);
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(fVar3, 0);
	return;
}

void CActorHeroPrivate::StateHeroUTurn()
{
	CCollision* pCVar1;
	CPlayerInput* pCVar2;
	bool bVar3;
	undefined4 uVar4;
	float fVar5;

	pCVar1 = this->pCollisionData;

	IncreaseEffort(1.0f);

	this->dynamic.rotationQuat = this->field_0x1030;

	SV_MOV_UpdateSpeedIntensity(3.5f, 10.9375f);

	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;

	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	bVar3 = SV_UpdateOrientation2D(10.47198f, &this->field_0x1030, 1);
	this->bFacingControlDirection = (int)bVar3;

	if (this->bFacingControlDirection == 0) {
		bVar3 = DetectGripablePrecipice();
		if (bVar3 == false) {
			if ((pCVar1->flags_0x4 & 2) == 0) {
				if (this->field_0x1184 < this->timeInAir) {
					if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
						uVar4 = STATE_HERO_FALL_A;
						if (this->field_0x142c != 0) {
							if (this->distanceToGround < 10.3f) {
								uVar4 = STATE_HERO_FALL_A;
								this->field_0x1020 = 1;
							}
							else {
								uVar4 = STATE_HERO_GLIDE_1;
								this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
							}
						}
					}
					else {
						uVar4 = STATE_HERO_JUMP_3_3_STAND;
						this->field_0x1020 = 1;
					}
					SetState(uVar4, 0xffffffff);
				}
			}
			else {
				this->timeInAir = 0.0;
			}
		}
		else {
			SetState(200, 0xffffffff);
		}
	}
	else {
		this->dynamic.rotationQuat = this->rotationQuat;

		pCVar2 = this->pPlayerInput;
		if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar5 = 0.0f;
		}
		else {
			fVar5 = pCVar2->aAnalogSticks[0].magnitude;
		}

		if (0.3f < fVar5) {
			if ((pCVar2 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar5 = 0.0f;
			}
			else {
				fVar5 = pCVar2->aAnalogSticks[0].magnitude;
			}

			if (fVar5 < 0.9f) {
				this->dynamic.speed = this->field_0x104c;
				SetState(0x77, 0xffffffff);
			}
			else {
				this->dynamic.speed = 3.5f;
				this->field_0x1048 = 1.0f;
				SetState(STATE_HERO_RUN, 0xffffffff);
			}
		}
		else {
			SetState(STATE_HERO_STAND, 0xffffffff);
		}
	}

	return;
}

void CActorHeroPrivate::StateHeroUTurnTerm()
{
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
}

void CActorHeroPrivate::StateHeroRollInit()
{
	this->field_0xf00 = this->currentLocation;
}

void CActorHeroPrivate::StateHeroRoll()
{
	CAnimation* pCVar1;
	CCollision* pCVar2;
	CPlayerInput* pCVar3;
	uint uVar4;
	edAnmLayer* peVar5;
	bool bVar6;
	edF32VECTOR4* peVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	CCollisionRay CStack64;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pAnimationController;
	pCVar2 = this->pCollisionData;
	IncreaseEffort(1.0f);
	this->dynamic.speed = this->field_0x1174;
	fVar10 = this->field_0x105c;
	fVar9 = this->field_0x1074;
	fVar12 = this->field_0x1040;
	fVar11 = this->field_0x1180;
	fVar14 = this->field_0x1058;
	fVar13 = this->field_0x1174;

	CActorAutonomous::ComputeSlidingForce(&eStack16, 0);

	edF32Vector4ScaleHard(this->field_0x107c, &eStack16, &eStack16);
	peVar7 = this->dynamicExt.aVelocity;
	edF32Vector4AddHard(peVar7, peVar7, &eStack16);
	fVar8 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
	this->dynamicExt.aVelocityMagnitudes[0] = fVar8;

	pCVar3 = this->pPlayerInput;
	if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar8 = 0.0f;
	}
	else {
		fVar8 = pCVar3->aAnalogSticks[0].magnitude;
	}

	if ((0.3f <= fVar8) || (0.1f <= this->dynamic.speed)) {
		BuildHorizontalSpeedVector(fVar13, fVar14, fVar11, fVar12, fVar9);
	}
	else {
		SV_MOV_UpdateSpeedIntensity(0.0f, fVar10);
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if ((pCVar2->flags_0x4 & 2) == 0) {
		if (this->field_0x1184 < this->timeInAir) {
			SetState(0x8d, 0xffffffff);
			return;
		}
	}
	else {
		this->timeInAir = 0.0;
	}

	bVar6 = CanEnterToboggan();
	if (bVar6 != false) {
		SetState(STATE_HERO_TOBOGGAN_3, 0xffffffff);
		return;
	}

	uVar4 = this->dynamic.flags;
	if (((uVar4 & 2) == 0) || (this->dynamicExt.field_0x6c <= 0.2)) {
	LAB_00141cf8:
		peVar5 = (pCVar1->anmBinMetaAnimator).aAnimData;

		if ((peVar5->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
			bVar6 = false;
			if (peVar5->animPlayState != STATE_ANIM_NONE) {
				bVar6 = (peVar5->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar6 = false;
		}

		if (bVar6) {
			pCVar3 = this->pPlayerInput;
			if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar8 = 0.0;
			}
			else {
				fVar8 = pCVar3->aButtons[5].clickValue;
			}

			if (fVar8 == 0.0f) {
				local_20.x = this->currentLocation.x;
				local_20.y = this->currentLocation.y + 0.2f;
				local_20.z = this->currentLocation.z;
				local_20.w = 1.0f;
				CCollisionRay CStack64 = CCollisionRay(1.4f, &local_20, &g_xVector);
				fVar8 = CStack64.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40000040, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);

				if (fVar8 == 1e+30f) {
					pCVar3 = this->pPlayerInput;
					if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar8 = 1000000.0f;
					}
					else {
						fVar8 = pCVar3->aButtons[4].pressedDuration;
					}

					if (fVar8 < 0.2f) {
						SetJumpCfg(0.1, this->runSpeed, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
						SetState(STATE_HERO_JUMP_1_1_RUN, 0xffffffff);
						return;
					}

					pCVar3 = this->pPlayerInput;
					if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar8 = 0.0f;
					}
					else {
						fVar8 = pCVar3->aAnalogSticks[0].magnitude;
					}

					if (fVar8 <= 0.3f) {
						SetState(STATE_HERO_STAND, 0xffffffff);
						return;
					}
					pCVar3 = this->pPlayerInput;
					if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar8 = 0.0f;
					}
					else {
						fVar8 = pCVar3->aAnalogSticks[0].magnitude;
					}

					if (0.9f <= fVar8) {
						SetState(STATE_HERO_RUN, 0xffffffff);
						return;
					}

					SetState(0x77, 0xffffffff);
					return;
				}
			}

			pCVar3 = this->pPlayerInput;
			if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar8 = 0.0f;
			}
			else {
				fVar8 = pCVar3->aAnalogSticks[0].magnitude;
			}

			if (0.3f < fVar8) {
				pCVar3 = this->pPlayerInput;
				if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar8 = 0.0f;
				}
				else {
					fVar8 = pCVar3->aAnalogSticks[0].magnitude;
				}

				if (fVar8 < 0.9f) {
					SetState(0x86, 0xffffffff);
					return;
				}

				pCVar3 = this->pPlayerInput;
				if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar8 = 1000000.0f;
				}
				else {
					fVar8 = pCVar3->aButtons[4].pressedDuration;
				}

				if (fVar8 < 0.1f) {
					pCVar3 = this->pPlayerInput;
					if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
						fVar8 = 0.0f;
					}
					else {
						fVar8 = pCVar3->GetAngleWithPlayerStick(&this->dynamic.velocityDirectionEuler);
					}

					if (this->field_0x1074 < fabs(fVar8)) {
						pCVar3 = this->pPlayerInput;
						if ((pCVar3 == (CPlayerInput*)0x0) || (peVar7 = &pCVar3->lAnalogStick, this->field_0x18dc != 0)) {
							peVar7 = &gF32Vector4Zero;
						}

						this->rotationQuat = *peVar7;

						pCVar3 = this->pPlayerInput;
						if ((pCVar3 == (CPlayerInput*)0x0) || (peVar7 = &pCVar3->lAnalogStick, this->field_0x18dc != 0)) {
							peVar7 = &gF32Vector4Zero;
						}

						this->dynamic.rotationQuat = *peVar7;

						SetState(0x8b, 0xffffffff);
						return;
					}

					bVar6 = (pCVar2->flags_0x4 & 1) == 0;

					if ((bVar6) || ((!bVar6 && (fVar9 = this->field_0x1430,
								fVar8 = edF32Vector4DotProductHard(&this->rotationQuat, &pCVar2->aCollisionContact->location), cosf(fVar9) < fVar8 + 1.0f)))) {
						SetState(0x8b, 0xffffffff);
						return;
					}
				}
			}

			SetState(0x8c, 0xffffffff);
		}
	}
	else {
		if ((uVar4 & 1) == 0) {
			pCVar3 = this->pPlayerInput;
			if ((pCVar3 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar8 = 0.0f;
			}
			else {
				fVar8 = pCVar3->aAnalogSticks[0].magnitude;
			}

			if (0.3f <= fVar8) goto LAB_00141cf8;
		}

		SetState(STATE_HERO_SLIDE_A, 0xffffffff);
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
	if (this->heroActionParams.actionId == 0xd) {
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
								uVar7 = STATE_HERO_GLIDE_1;
								this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
							}
						}
					}
					else {
						uVar7 = STATE_HERO_JUMP_3_3_STAND;
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
					SetState(STATE_HERO_JUMP_3_3_RUN, 0xffffffff);
				}
				else {
					/* Falling */
					SetState(STATE_HERO_JUMP_3_3_STAND, 0xffffffff);
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
		this->field_0xf50 = this->field_0xf54;
		this->heroActionParams.actionId = 0xd;

		IMPLEMENTATION_GUARD(
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

				if ((0.9f <= fVar11) || (bVar7 = DetectGripablePrecipice(), bVar7 == false)) {
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
										uVar10 = STATE_HERO_GLIDE_1;
										this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
									}
								}
							}
							else {
								uVar10 = STATE_HERO_JUMP_3_3_STAND;
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
						if (peVar4->animPlayState == STATE_ANIM_NONE) {
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
										SetState(STATE_HERO_RUN, 0xffffffff);
									}
								}
								else {
									bVar7 = false;
									if (AVar6 == AVar5) {
										if (peVar4->animPlayState == STATE_ANIM_NONE) {
											bVar7 = false;
										}
										else {
											bVar7 = (peVar4->field_0xcc & 2) != 0;
										}
									}
									if (bVar7) {
										/* Land Animation */
										SetState(STATE_HERO_STAND, 0xffffffff);
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
								if (0.3f < fVar11) {
									SetState(STATE_HERO_ROLL, 0xffffffff);
								}
								else {
									SetState(0x85, 0xffffffff);
								}
							}
						}
						else {
							SetState(STATE_HERO_STAND, 0xffffffff);
						}
					}
				}
				else {
					SetState(200, 0xffffffff);
				}
			}
			else {
				if (this->airHorizontalSpeed < this->field_0x1164) {
					SetState(STATE_HERO_SLIDE_B, 0xffffffff);
				}
				else {
					SetState(STATE_HERO_SLIDE_A, 0xffffffff);
				}
			}
		}
		else {
			SetState(STATE_HERO_TOBOGGAN_2, 0xffffffff);
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
			SetState(STATE_HERO_JUMP_1_1_STAND, 0xffffffff);
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

						if (peVar4->animPlayState != STATE_ANIM_NONE) {
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
		SetState(STATE_HERO_JUMP_3_3_STAND, 0xffffffff);
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
			if (peVar2->animPlayState != STATE_ANIM_NONE) {
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
	StateConfig* pAVar2;
	Timer* pTVar3;
	int iVar4;
	uint uVar5;
	long lVar6;
	float fVar7;

	pCVar1 = this->pCollisionData;

	MoveInAir(this->airMinSpeed, this->airHorizontalSpeed, this->airControlSpeed, this->airNoInputSpeed, rotationRate);

	ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

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
		if (this->heroActionParams.actionId == 0xd) {
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
							lVar6 = STATE_HERO_GLIDE_1;
							this->dynamic.flags = this->dynamic.flags & 0xfffffffb;
							fVar7 = 10.3f;
						}
					}
				}
				else {
					lVar6 = STATE_HERO_JUMP_3_3_STAND;
					this->field_0x1020 = 1;
				}
				if (lVar6 == STATE_HERO_GLIDE_1) {
					fVar7 = this->field_0x1184;

					if (fVar7 < this->timeInAir) {
						SetState(STATE_HERO_GLIDE_1, 0xffffffff);
						return;
					}
				}
				else {
					this->timeInAir = 0.0f;
				}

				uVar5 = CanGrip(param_3, &this->rotationQuat);
				if (uVar5 == 0) {
					fVar7 = this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0, 0);
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

void CActorHeroPrivate::StateHeroFlyInit()
{
	int* piVar1;
	int iVar2;
	Timer* pTVar3;

	this->field_0x11ec = 0.0f;
	this->field_0x11f4 = 0.0f;
	this->field_0x11f8 = 0.0f;
	ConvertSpeedSumForceExtToSpeedPlayer2D();
	pTVar3 = GetTimer();
	this->field_0x1548 = pTVar3->scaledTotalTime;
	this->field_0x1420 = 0;
	this->field_0x1424 = 0;
	this->field_0xcb4.Init(0.0f, 800.0f);
	this->field_0x1428 = 1;

	IMPLEMENTATION_GUARD_LOG(
	CFxHandle::SV_FX_Start((CFxHandle*)&this->field_0x13c0);
	piVar1 = (int*)this->field_0x13c4;
	if (((piVar1 != (int*)0x0) && (iVar2 = *(int*)&this->field_0x13c0, iVar2 != 0)) && (iVar2 == piVar1[6])) {
		(**(code**)(*piVar1 + 0x10))(0, 0);
	})

	return;
}

void CActorHeroPrivate::StateHeroGlideInit()
{
	return;
}

void CActorHeroPrivate::StateHeroGlide(int param_2, int nextState)
{
	byte bVar1;
	CCollision* pCVar2;
	CAnimation* pCVar3;
	ed_zone_3d* pZone;
	edAnmLayer* peVar4;
	bool bVar5;
	bool bVar6;
	CEventManager* pCVar7;
	bool bVar8;
	Timer* pTVar9;
	uint uVar10;
	int iVar11;
	StateConfig* pAVar12;
	CCamera* pCVar13;
	ECameraType EVar14;
	CActorWindState* uVar15;
	CActorWindState* iVar16;
	CActorWindState* pCVar17;
	StateConfig* pAVar18;
	CPlayerInput* pCVar19;
	int iVar20;
	CActorWindState* piVar21;
	uint uVar22;
	ulong uVar23;
	long lVar24;
	edF32VECTOR4* peVar25;
	float fVar26;
	float fVar27;
	float fVar28;
	float fVar29;
	float fVar30;
	float fVar31;
	edF32VECTOR4 eStack416;
	edF32VECTOR4 local_190;
	edF32VECTOR4 local_180;
	edF32VECTOR4 local_170;
	edF32VECTOR4 local_160;
	edF32VECTOR4 local_150;
	edF32VECTOR4 eStack320;
	CActorsTable local_130;
	edF32VECTOR4 local_20;
	float local_10;
	float local_c;
	float local_8;
	float local_4;
	CCameraManager* pCameraManager;

	iVar20 = 0;
	pCVar2 = this->pCollisionData;
	pCVar3 = this->pAnimationController;
	bVar5 = false;
	local_130.entryCount = 0;
	uVar22 = 0;

	fVar26 = edFIntervalUnitSrcLERP(this->field_0x11ec, 0.5f, 1.0f);
	IncreaseEffort(fVar26);

	local_20 = this->dynamicExt.aVelocity[2];

	fVar26 = this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y;
	fVar31 = this->dynamicExt.aVelocityMagnitudes[2];
	bVar6 = 0.001f < fVar31;
	if (this->field_0x1428 == 0) {
		pTVar9 = GetTimer();
		this->field_0x1548 = pTVar9->scaledTotalTime;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
		IMPLEMENTATION_GUARD(
		ClearAllSumForceExt());
	}
	else {
		bVar8 = UpdateOrientationFromWind(&local_20, (edF32VECTOR4*)0x0);
		iVar20 = (int)bVar8;

		pTVar9 = GetTimer();
		fVar27 = powf(0.9f, pTVar9->cutsceneDeltaTime * 50.0f);
		if (1.0f < fVar27) {
			fVar27 = 1.0f;
		}
		else {
			if (fVar27 < 0.0f) {
				fVar27 = 0.0f;
			}
		}

		bVar1 = pCVar2->flags_0x4;
		if (((bVar1 & 2) == 0) && ((bVar1 & 4) == 0)) {
			uVar10 = TestState_IsInHit(0xffffffff);
			if ((uVar10 == 0) || (bVar6)) {
				pCVar19 = this->pPlayerInput;
				if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
					fVar28 = 0.0f;
				}
				else {
					fVar28 = pCVar19->aButtons[4].clickValue;
				}

				if ((int)fVar28 < 0) {
					fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
					fVar28 = fVar28 + fVar28;
				}
				else {
					fVar28 = (float)(int)fVar28;
				}

				this->field_0x11ec = this->field_0x11ec * fVar27 + (1.0f - fVar27) * fVar28;
			}
			else {
				this->field_0x11ec = 0.0f;
				this->field_0x11f4 = 0.0f;
			}
		}
		else {
			pCVar19 = this->pPlayerInput;
			if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
				fVar28 = 0.0f;
			}
			else {
				fVar28 = pCVar19->aButtons[4].clickValue;
			}

			if ((int)fVar28 < 0) {
				fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
				fVar28 = fVar28 + fVar28;
			}
			else {
				fVar28 = (float)(int)fVar28;
			}

			this->field_0x11ec = fVar28;
		}

		pCVar19 = this->pPlayerInput;
		if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar28 = 0.0f;
		}
		else {
			fVar28 = pCVar19->aButtons[4].clickValue;
		}

		if ((int)fVar28 < 0) {
			fVar28 = (float)((uint)fVar28 >> 1 | (uint)fVar28 & 1);
			fVar28 = fVar28 + fVar28;
		}
		else {
			fVar28 = (float)(int)fVar28;
		}

		this->field_0x11f4 = this->field_0x11f4 * fVar27 + (1.0f - fVar27) * fVar28;

		ChangeCollisionSphereForGlide(0.1f, this->field_0x11f4);

		if ((1.0f < this->field_0xa88) || (bVar6)) {
			pTVar9 = GetTimer();
			this->field_0x1548 = pTVar9->scaledTotalTime;
			this->field_0x1420 = 0;
			this->field_0x1424 = 0;
		}
		else {
			pTVar9 = GetTimer();
			if (this->field_0x1410 < pTVar9->scaledTotalTime - this->field_0x1548) {
				uVar22 = 0x40000;
				this->field_0x1420 = 1;
			}
			else {
				this->field_0x11f0 = this->currentLocation.y;
			}

			pTVar9 = GetTimer();
			if (this->field_0x1414 < pTVar9->scaledTotalTime - this->field_0x1548) {
				this->field_0x1424 = 1;
				this->animKey_0x157c = this->currentLocation.y + 20.0f;

				pCVar7 = CScene::ptable.g_EventManager_006f5080;
				iVar16 = 0;
				if (2.24f <= CScene::_pinstance->field_0x1c) {
					while (true) {
						IMPLEMENTATION_GUARD(
						piVar21 = *(int**)&this->field_0xe48;
						if (piVar21 == (int*)0x0) {
							iVar11 = 0;
						}
						else {
							iVar11 = *piVar21;
						}
						if (iVar11 <= iVar16) goto LAB_00149ea0;
						pZone = *(ed_zone_3d**)(*(int*)&this->field_0xe48 + iVar16 * 4 + 4);
						if ((pZone != (ed_zone_3d*)0x0) &&
							(uVar10 = edEventComputeZoneAgainstVertex
							(pCVar7->activeChunkId, pZone,
								&this->currentLocation, 0),
								(uVar10 & 1) != 0)) break;
						iVar16 = iVar16 + 1;)
					}

					bVar8 = true;
				}
				else {
				LAB_00149ea0:
					bVar8 = false;
				}

				if (!bVar8) {
					if (this->distanceToGround < 10.3f) {
						this->field_0x11f0 = this->currentLocation.y;
					}
					else {
						if (100.0f < this->field_0x11f0 - this->currentLocation.y) {
							bVar5 = true;
						}
					}
				}
			}
			else {
				if (this->distanceToGround < 1.0f) {
					CLifeInterface* pLifeInterface = GetLifeInterface();
					fVar27 = pLifeInterface->GetValue();
					bVar8 = fVar27 - this->field_0x2e4 <= 0.0f;
					if (!bVar8) {
						int iVar162 = this->actorState;
						uVar10 = 0;
						if (iVar162 != -1) {
							pAVar18 = GetStateCfg(iVar162);
							uVar10 = pAVar18->flags_0x4;
						}
						bVar8 = (uVar10 & 1) != 0;
					}

					if (!bVar8) {
						PlayAnim(0x103);
						goto LAB_0014a028;
					}
				}

				pAVar12 = GetStateCfg(this->actorState);
				PlayAnim(pAVar12->animId);
			}
		}
	}
LAB_0014a028:
	if (bVar6) {
		SV_UpdateValue(0.0f, 1.2f, &this->field_0x13cc);
		IMPLEMENTATION_GUARD_AUDIO(
		SV_FX_Sound_SetVolume(this->field_0x13cc, (CFxHandle*)&this->field_0x13c0);)
	}
	else {
		IMPLEMENTATION_GUARD_AUDIO(
		SV_UpdateValue
		(edFCosinus
			[(int)(ABS((this->timeInAir * 2.5 - 1.570796) * 1303.797) + 0.5)
			& 0x1fff] * 0.1 + 1.0, (float)&DAT_3f4ccccd, (CActor*)this, &this->field_0x13cc);
		SV_FX_Sound_SetVolume(this->field_0x13cc, (CFxHandle*)&this->field_0x13c0);
		)

		fVar27 = fabs(this->field_0xa88);
		if (fVar27 < 19.0f) {
			fVar28 = edFIntervalLERP(fVar27, 0.0f, 19.0f, 0.9f, 1.1f);
			this->field_0x13d0 = fVar28;
		}
		else {
			fVar28 = edFIntervalLERP(fVar27, 19.0f, 27.0f, 1.1f, 1.5f);
			this->field_0x13d0 = fVar28;
		}

		IMPLEMENTATION_GUARD_AUDIO(
			fVar28 = edFCosinus
			[(int)(ABS((this->timeInAir * 4.0 - 1.570796) * 1303.797) + 0.5) &
			0x1fff];)

		fVar28 = cosf(this->timeInAir * 4.0f - 1.570796f);

		if (fVar27 < 5.0) {
			fVar27 = edFIntervalLERP(fVar27, 0.0, 5.0, 0.2, 0.1);
			fVar28 = fVar28 * fVar27;
		}
		else {
			fVar27 = edFIntervalLERP(fVar27, 5.0, 27.0, 0.1, 0.02);
			fVar28 = fVar28 * fVar27;
		}

		this->field_0x13d0 = this->field_0x13d0 + fVar28;
		IMPLEMENTATION_GUARD_AUDIO(
		piVar21 = (int*)this->field_0x13c4;
		if ((piVar21 != (int*)0x0) && (*(int*)&this->field_0x13c0 != 0)) {
			(**(code**)(*piVar21 + 0x40))();
		})
	}

	pCameraManager = CCameraManager::_gThis;
	pTVar9 = GetTimer();
	if ((pTVar9->scaledTotalTime - this->field_0x1548 <= this->field_0x1410) || (this->distanceToGround != this->field_0xf0)) {
		pCVar13 = pCameraManager->GetDefGameCamera((ECameraType)9);
		pCameraManager->PushCamera(pCVar13, 0);
		pCVar13 = pCameraManager->GetDefGameCamera((ECameraType)8);
		pCameraManager->PushCamera(pCVar13, 0);
	}

	EVar14 = pCameraManager->pActiveCamera->GetMode();
	if (EVar14 == 8) {
		pCVar13 = pCameraManager->pActiveCamera;
		this->field_0xcb4.UpdateLerp(this->rotationEuler.z);
		pCVar13->SetAngleGamma(this->field_0xcb4.field_0x0);
	}

	if (this->field_0x1428 == 0) {
		fVar27 = edFIntervalUnitDstLERP(this->field_0x37c.field_0x4, this->field_0x37c.field_0x8, 0.0f);
		this->field_0x11fc = fVar27;

		pTVar9 = GetTimer();
		this->scalarDynJump.Integrate(pTVar9->cutsceneDeltaTime);
		pTVar9 = GetTimer();
		this->field_0x37c.Integrate(pTVar9->cutsceneDeltaTime);
		fVar27 = this->field_0x37c.GetInstantSpeed();
		edF32Vector4ScaleHard(fVar27, &local_180, &this->field_0x1400);
		local_180.y = this->scalarDynJump.field_0x20;
		fVar27 = edF32Vector4NormalizeHard(&local_180, &local_180);
		this->dynamic.rotationQuat = local_180;
		this->dynamic.speed = fVar27;

		if (0.001f < this->dynamic.linearAcceleration) {
			local_190.x = this->dynamic.velocityDirectionEuler.x;
			local_190.z = this->dynamic.velocityDirectionEuler.z;
			local_190.w = this->dynamic.velocityDirectionEuler.w;
			local_190.y = this->dynamic.velocityDirectionEuler.y * this->field_0x11fc;

			edF32Vector4SafeNormalize1Hard(&local_190, &local_190);

			this->rotationQuat = local_190;
		}

		ManageDyn(4.0f, 0x4000d, (CActorsTable*)0x0);
		uVar23 = this->scalarDynJump.OnLastValidSample();
		if (uVar23 != 0) {
			SetState(STATE_HERO_GLIDE_1, 0xffffffff);
			return;
		}
	}
	else {
		pCVar19 = this->pPlayerInput;
		fVar27 = 0.0f;
		if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			fVar28 = 0.0f;
		}
		else {
			fVar28 = pCVar19->aAnalogSticks[0].magnitude;
		}

		if (0.3f < fVar28) {
			local_150.x = -this->rotationQuat.z;
			local_150.y = 0.0f;
			local_150.z = this->rotationQuat.x;
			local_150.w = 0.0f;
			pCVar19 = this->pPlayerInput;
			if ((pCVar19 == (CPlayerInput*)0x0) || (peVar25 = &pCVar19->lAnalogStick, this->field_0x18dc != 0)) {
				peVar25 = &gF32Vector4Zero;
			}

			fVar28 = edF32Vector4DotProductHard(peVar25, &local_150);
			if (0.70710677f < fVar28) {
				fVar27 = 0.5235988f;
			}
			else {
				if (fVar28 < -0.70710677f) {
					fVar27 = -0.5235988f;
				}
			}
		}

		local_10 = this->rotationEuler.z;
		SV_UpdateValue(fVar27, 1.0472f, &local_10);
		this->rotationEuler.z = local_10;
		this->field_0x11f8 = local_10 / 0.5235988;
		if (bVar6) {
			uVar15 = GetWindState();
			if (uVar15 == (CActorWindState*)0x0) {
				fVar27 = 0.0f;
			}
			else {
				iVar16 = GetWindState();
				fVar27 = iVar16->field_0x1c;
			}

			pCVar17 = GetWindState();
			if (pCVar17 == (CActorWindState*)0x0) {
				fVar28 = 0.0f;
			}
			else {
				pCVar17 = GetWindState();
				fVar28 = pCVar17->field_0x20;
			}

			edF32Vector4ScaleHard(this->field_0x11ec * (CDynamicExt::gForceGravity.y * fVar28 + local_20.y * fVar27), &eStack320, &g_xVector);
			peVar25 = this->dynamicExt.aVelocity;
			edF32Vector4AddHard(peVar25, peVar25, &eStack320);
			fVar27 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
			this->dynamicExt.aVelocityMagnitudes[0] = fVar27;
		}

		pCVar19 = this->pPlayerInput;
		if ((pCVar19 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			local_c = 0.0f;
			local_8 = 0.0f;
			local_4 = 0.0f;
		}
		else {
			pCVar19->GetPadRelativeToNormal2D(&this->dynamic.horizontalVelocityDirectionEuler, &local_4, &local_8, &local_c);
		}

		if (bVar6) {
			uVar15 = GetWindState();

			int iVar162;

			if (uVar15 == (CActorWindState*)0x0) {
				iVar162 = 0;
			}
			else {
				iVar16 = GetWindState();
				iVar162 = iVar16->field_0x28;
			}

			if (iVar162 < 1) {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar27 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar27 = iVar16->field_0x10;
				}
			}
			else {
				fVar27 = 1.0f;
				uVar22 = 0x40000;
			}
		}
		else {
			if (this->field_0x1424 == 0) {
				fVar27 = 4.0f;
			}
			else {
				fVar27 = edFIntervalUnitSrcLERP(this->field_0x11ec, 4.0f, 2.0f);
			}
		}

		if (bVar6) {
			if (0.0f <= local_8) {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar28 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar28 = iVar16->field_0xc;
				}

				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar29 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar29 = iVar16->field_0x14;
				}

				fVar29 = edFIntervalUnitSrcLERP(local_8, fVar28, fVar29);
			}
			else {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar28 = 0.0;
				}
				else {
					iVar16 = GetWindState();
					fVar28 = iVar16->field_0xc;
				}

				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar29 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar29 = iVar16->field_0x18;
				}

				fVar29 = edFIntervalUnitSrcLERP(-local_8, fVar28, fVar29);
			}

			uVar15 = GetWindState();
			if (uVar15 == (CActorWindState*)0x0) {
				fVar28 = 0.0f;
			}
			else {
				iVar16 = GetWindState();
				fVar28 = iVar16->field_0x1c;
			}

			if (fVar28 < -1.0f) {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar28 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar28 = iVar16->field_0x24;
				}
				fVar28 = fVar28 * 0.42f;
			}
			else {
				uVar15 = GetWindState();
				if (uVar15 == (CActorWindState*)0x0) {
					fVar30 = 0.0f;
				}
				else {
					iVar16 = GetWindState();
					fVar30 = iVar16->field_0x24;
				}

				fVar28 = edFIntervalUnitSrcLERP(this->field_0x11ec, 0.42f, 0.24f);
				fVar28 = fVar28 * fVar30;
			}

			SV_UpdateValue(fVar29, 8.0f, &this->field_0x11e4);
		}
		else {
			fVar28 = edFIntervalUnitSrcLERP(this->field_0x11ec, 0.42f, 0.24f);
			SV_UpdateValue(this->field_0x11e0, 3.0f, &this->field_0x11e4);
		}

		if (0.0 < this->field_0x11e4) {
			uVar15 = GetWindState();
			int iVar162;
			if (uVar15 == (CActorWindState*)0x0) {
				iVar162 = 0;
			}
			else {
				iVar16 = GetWindState();
				iVar162 = iVar16->field_0x4;
			}

			if (0 < iVar162) {
				uVar15 = GetWindState();
				CWayPoint* pWaypoint;
				if (uVar15 == (CActorWindState*)0x0) {
					pWaypoint = (CWayPoint*)0x0;
				}
				else {
					iVar16 = GetWindState();
					pWaypoint = iVar16->field_0x2c;
				}

				if (pWaypoint == (CWayPoint*)0x0) {
					IMPLEMENTATION_GUARD(
					CActorAutonomous::ComputeFrictionForce2DWithSpeedMax
					(*(float*)&this->field_0x11e4, (CActorAutonomous*)this, &eStack320, 1);
					peVar25 = this->dynamicExt.aVelocity + 1;
					edF32Vector4AddHard(peVar25, peVar25, &eStack320);
					fVar29 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity + 1);
					this->dynamicExt.aVelocityMagnitudes[1] = fVar29;)
				}
			}
		}

		MoveInFreeFall(0.0f, this->field_0x11e4, fVar28, this->field_0x11e8, this->airRotationRate, iVar20);
		
		pCVar17 = GetWindState();
		if (pCVar17 == (CActorWindState*)0x0) {
			iVar20 = 0;
		}
		else {
			pCVar17 = GetWindState();
			iVar20 = pCVar17->field_0x4;
		}

		if (0 < iVar20) {
			local_160.y = 0.0f;
			local_160.x = local_20.x;
			local_160.z = local_20.z;
			local_160.w = local_20.w;

			fVar28 = edF32Vector4NormalizeHard(&local_160, &local_160);
			if (2.0f < fVar28) {
				edF32Vector4ScaleHard(this->dynamic.speed, &local_170, &this->dynamic.rotationQuat);
				edProjectVectorOnPlane(0.0f, &local_170, &local_170, &local_160, 0);
				local_170.y = 0.0f;
				fVar28 = edF32Vector4SafeNormalize1Hard(&local_170, &local_170);
				this->dynamic.speed = fVar28;
				this->dynamic.rotationQuat = local_170;
			}
		}

		uVar10 = uVar22 | 0xf00010d;
		if (bVar6) {
			uVar10 = uVar22 | 0xf10010d;
		}

		if (this->field_0x1424 == 0) {
			ManageDyn(fVar27, uVar10, (CActorsTable*)0x0);
		}
		else {
			ManageDyn(fVar27, uVar10, &local_130);
		}
	}

	iVar20 = this->actorState;
	uVar22 = 0;
	if (iVar20 != -1) {
		pAVar18 = GetStateCfg(iVar20);
		uVar22 = pAVar18->flags_0x4;
	}

	if ((uVar22 & 1) == 0) {
		if (bVar5) {
			iVar20 = this->actorState;
			uVar22 = 0;
			if (iVar20 != -1) {
				pAVar18 = GetStateCfg(iVar20);
				uVar22 = pAVar18->flags_0x4;
			}
			if ((uVar22 & 1) == 0) {
				SetState(0xa1, 0xffffffff);
				return;
			}
		}

		bVar5 = (pCVar2->flags_0x4 & 2) != 0;
		if ((this->field_0x1424 != 0) && (bVar8 = false, bVar5)) {
			for (iVar20 = 0; iVar20 < local_130.entryCount; iVar20 = iVar20 + 1) {
				IMPLEMENTATION_GUARD(
				lVar24 = BreakActor(0, 5.0, 10.0, local_130.aEntries[iVar20], 1, 0, 0);
				if (lVar24 == 1) {
					bVar5 = false;
					fVar27 = this->dynamicExt.normalizedTranslation.y *
						this->dynamicExt.field_0x6c;
					this->dynamicExt.field_0x6c =
						SQRT(this->dynamicExt.field_0x6c *
							this->dynamicExt.field_0x6c - fVar27 * fVar27);
					this->dynamicExt.normalizedTranslation.y = 0.0;
				}
				else {
					if (lVar24 == 0) {
						bVar5 = false;
						bVar8 = true;
					}
				})
			}

			if (bVar8) {
				pTVar9 = Timer::GetTimer();
				if (this->field_0x155c <= pTVar9->scaledTotalTime) {
					fVar27 = edFIntervalUnitSrcLERP(this->field_0x11ec, 5.0f, 10.0f);
					LifeDecrease(fVar27);
					CLifeInterface* pLifeInterface = GetLifeInterface();
					fVar27 = pLifeInterface->GetValue();
					if (fVar27 == 0.0f) {
						SetState(0x99, 0xffffffff);
						return;
					}

					SetInvincible(2.0f, 1);
				}

				edF32Vector4ScaleHard(this->field_0xa80, &eStack416, &this->dynamic.velocityDirectionEuler);
				edF32Vector4ScaleHard(0.25f, &eStack416, &eStack416);
				fVar27 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &eStack416);
				this->dynamicExt.field_0x6c = fVar27;
				this->field_0x11f4 = -1.0f;
				pCVar19 = GetInputManager(1, 0);
				if (pCVar19 != (CPlayerInput*)0x0) {
					CPlayerInput::FUN_001b66f0(0.6f, 0.0f, 0.2f, 0.0f, &pCVar19->field_0x40, 0);
				}
			}
		}

		if ((!bVar5) || (bVar6)) {
			uVar15 = GetWindState();
			if (uVar15 == (CActorWindState*)0x0) {
				bVar5 = false;
			}
			else {
				piVar21 = GetWindState();
				iVar20 = piVar21->field_0x0;
				iVar16 = GetWindState();
				if (iVar20 == iVar16->field_0x4) {
					bVar5 = true;
				}
				else {
					iVar16 = GetWindState();
					if (iVar16->field_0x4 == 0) {
						bVar5 = false;
					}
					else {
						bVar5 = true;
						if (0.17398384f <= fabs(local_20.y)) {
							bVar5 = false;
						}
					}
				}
			}

			if ((!bVar5) || (bVar5 = true, fVar31 <= 2.0f)) {
				bVar5 = false;
			}

			if (bVar5) {
				this->field_0x1020 = 1;
				SetState(0xf5, 0xffffffff);
			}
			else {
				uVar22 = GetStateHeroFlags(this->prevActorState);
				uVar22 = TestState_IsGripped(uVar22);
				if ((bVar6) || (uVar22 = CanGrip(uVar22, &this->rotationQuat), uVar22 == 0)) {
					if (nextState != -1) {
						peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
						if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
							bVar5 = false;
							if (peVar4->animPlayState != STATE_ANIM_NONE) {
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

					if (param_2 != -1) {
						if ((param_2 == 0) || (bVar6)) {
							if ((param_2 == 0) && (bVar6)) {
								SetState(STATE_HERO_GLIDE_2, 0xffffffff);
							}
						}
						else {
							SetState(0xf0, -1);
						}
					}
				}
				else {
					this->field_0x1020 = 1;
					SetGripState();
				}
			}
		}
		else {
			Landing();
			iVar20 = ChooseStateLanding(fabs(fVar26));
			if (iVar20 != -1) {
				SetState(iVar20, -1);
			}
		}
	}
	else {
		if (2.0f < this->timeInAir) {
			IMPLEMENTATION_GUARD(
			(*(code*)(this->pVTable)->field_0x16c)(this);)
		}
	}
	return;
}

void CActorHeroPrivate::SetJumpCfgFromGround(float param_1)
{
	CCollision* pCVar1;
	float fVar2;
	float inMin;

	pCVar1 = this->pCollisionData;
	if ((this->dynamic.flags & 2) != 0) {
		fVar2 = CCollisionManager::GetWallNormalYLimit(pCVar1->aCollisionContact + 1);
		inMin = CCollisionManager::GetGroundSpeedDecreaseNormalYLimit(pCVar1->aCollisionContact + 1);
		fVar2 = edFIntervalLERP(pCVar1->aCollisionContact[1].location.y, inMin, fVar2, 1.0f, 0.3f);
		param_1 = this->airHorizontalSpeed * fVar2;
	}

	SetJumpCfg(0.1f, param_1, this->field_0x1158, this->field_0x1150, this->field_0x1154, 1, (edF32VECTOR4*)0x0);
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
			CAnimation::UnRegisterBone(this->data.pAnimationController, this->field_0x1598);
			CAnimation::UnRegisterBone(this->data.pAnimationController, this->field_0x157c);
			*(undefined4*)&this->field_0x1b64 = 0;
			this->field_0x1b68 = 0;
			this->field_0x1b6c = 0;
			this->field_0x1b78 = 0;)
		}

		if (this->boomyState_0x1b70 == 0xd) {
			IMPLEMENTATION_GUARD(
			CAnimation::RemoveDisabledBone
			(this->data.pAnimationController, this->field_0x157c);
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
			CAnimation::AddDisabledBone(this->data.pAnimationController, this->field_0x157c);
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
					CAnimation::RegisterBone(this->data.pAnimationController, this->field_0x1598);
					CAnimation::RegisterBone(this->data.pAnimationController, this->field_0x157c);
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

void CActorHeroPrivate::RestoreVerticalOrientation()
{
	edF32VECTOR4 newRotationQuat;

	if ((this->flags & 0x1000) == 0) {
		this->rotationEuler.z = 0.0f;
		this->rotationEuler.x = 0.0f;

		newRotationQuat.x = this->rotationQuat.x;
		newRotationQuat.z = this->rotationQuat.z;
		newRotationQuat.w = this->rotationQuat.w;
		newRotationQuat.y = 0.0f;

		edF32Vector4SafeNormalize1Hard(&newRotationQuat, &newRotationQuat);

		this->rotationQuat = newRotationQuat;
	}
	return;
}

bool CActorHeroPrivate::DetectGripablePrecipice()
{
	IMPLEMENTATION_GUARD_LOG();
	return false;
}

int CActorHeroPrivate::SlideOnGround(float param_1, float param_2, float param_3, float param_4, uint flags)
{
	CCollision* pCVar1;
	edF32VECTOR4* peVar2;
	int* piVar3;
	int iVar4;
	float fVar5;
	CActorsTable local_150;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;
	edF32Vector4ScaleHard(this->dynamic.linearAcceleration, &eStack32, &this->dynamic.velocityDirectionEuler);
	edF32Vector4CrossProductHard(&eStack16, &eStack32, &g_xVector);
	edF32Vector4ScaleHard(param_3, &eStack16, &eStack16);
	peVar2 = this->dynamicExt.aVelocity;
	edF32Vector4AddHard(peVar2, peVar2, &eStack16);

	fVar5 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
	this->dynamicExt.aVelocityMagnitudes[0] = fVar5;

	if ((pCVar1->flags_0x4 & 2) == 0) {
		flags = flags | 0x120;
	}
	else {
		ComputeSlidingForce(&eStack48, 0);
		edF32Vector4ScaleHard(this->field_0x1078, &eStack48, &eStack48);
		peVar2 = this->dynamicExt.aVelocity;

		if ((this->dynamic.flags & 2) == 0) {
			fVar5 = edFIntervalDotSrcLERP(param_4, param_2, param_1);
			ComputeFrictionForce(fVar5, &eStack64, 0);
			peVar2 = this->dynamicExt.aVelocity + 1;
			edF32Vector4AddHard(peVar2, peVar2, &eStack64);
			fVar5 = edF32Vector4GetDistHard(peVar2);
			this->dynamicExt.aVelocityMagnitudes[1] = fVar5;
		}
		else {
			edF32Vector4AddHard(peVar2, peVar2, &eStack48);
			fVar5 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
			this->dynamicExt.aVelocityMagnitudes[0] = fVar5;
			ComputeFrictionForceWithSpeedMax(this->field_0x106c, &eStack64, 1);
			peVar2 = this->dynamicExt.aVelocity + 1;
			edF32Vector4AddHard(peVar2, peVar2, &eStack64);
			fVar5 = edF32Vector4GetDistHard(peVar2);
			this->dynamicExt.aVelocityMagnitudes[1] = fVar5;
		}
	}

	local_150.entryCount = 0;
	ManageDyn(4.0f, flags, &local_150);

	iVar4 = 0;
	if (0 < local_150.entryCount) {
		do {
			if (local_150.aEntries[iVar4]->typeID != 0xd) {
				return 1;
			}

			iVar4 = iVar4 + 1;
		} while (iVar4 < local_150.entryCount);
	}

	return 0;
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

void CActorHeroPrivate::MoveInFreeFall(float param_1, float param_2, float param_3, float param_4, float param_5, int param_7)
{
	CPlayerInput* pCVar1;
	edF32VECTOR4* v0;
	float fVar2;
	float puVar3;
	float puVar4;
	float fVar5;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::MoveInFreeFall param_1: {}, param_2: {}, param_3: {}, param_4: {}, param_5: {}, param_7: {}",
		param_1, param_2, param_3, param_4, param_5, param_7);

	pCVar1 = this->pPlayerInput;
	if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
		fVar2 = 0.0f;
	}
	else {
		fVar2 = pCVar1->aAnalogSticks[0].magnitude;
	}

	if (0.3f <= fVar2) {
		edF32Vector4ScaleHard(this->dynamic.speed, &local_10, &this->dynamic.rotationQuat);

		pCVar1 = this->pPlayerInput;
		if ((pCVar1 == (CPlayerInput*)0x0) || (this->field_0x18dc != 0)) {
			local_20 = gF32Vector4Zero;
		}
		else {
			edF32Vector4ScaleHard(pCVar1->aAnalogSticks[0].magnitude, &local_20, &pCVar1->lAnalogStick);
		}

		pCVar1 = this->pPlayerInput;
		if ((pCVar1 == (CPlayerInput*)0x0) || (v0 = &pCVar1->lAnalogStick, this->field_0x18dc != 0)) {
			v0 = &gF32Vector4Zero;
		}

		puVar3 = edF32Vector4DotProductHard(v0, &this->dynamic.velocityDirectionEuler);
		if (1.0f < (float)puVar3) {
			puVar4 = 1.0f;
		}
		else {
			puVar4 = -1.0f;
			if (-1.0 <= puVar3) {
				puVar4 = puVar3;
			}
		}

		fVar2 = acosf(puVar4);
		fVar5 = edFIntervalLERP(fVar2, 0.0f, 3.141593f, 0.5f, 2.0f);

		edF32Vector4ScaleHard(param_3 * fVar5, &local_20, &local_20);
		edF32Vector4AddHard(&local_10, &local_10, &local_20);

		fVar5 = edF32Vector4NormalizeHard(&local_10, &local_10);
		if ((fVar5 <= param_2) && (param_2 = param_1, param_1 <= fVar5)) {
			param_2 = fVar5;
		}

		this->dynamic.speed = param_2;
		if (0.001 < param_2) {
			this->dynamic.rotationQuat = local_10;
		}

		if (param_7 == 0) {
			fVar2 = edFIntervalLERP(fVar2, 0.0, 3.141593f, 2.0f, 0.5f);
			SV_UpdateOrientation2D(param_5 * fVar2, &this->dynamic.rotationQuat, 0);
		}
	}
	else {
		SV_MOV_UpdateSpeedIntensity(0.0f, param_4);
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
		SetState(STATE_HERO_GRIP_C, 0xffffffff);
	}
	else {
		curState = this->actorState;
		if (((curState == STATE_HERO_JUMP_2_3_STAND) || (curState == STATE_HERO_JUMP_2_3_RUN)) || (curState == 199)) {
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

	fVar5 = CStack96.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40000010, &local_30, &local_10);

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar5: {}", fVar5);

	if ((((fVar5 == 1e+30f) || (fVar5 <= 0.01f)) || (local_30.y < cosf(this->field_0x14b4))) ||
		(0.0f < local_30.x * local_40.x + local_30.y * local_40.y + local_30.z * local_40.z)) {
		fVar5 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.3f;

		local_20.x = param_4->x * fVar5 + param_3->x;
		local_20.y = param_3->y + 1.75f;
		local_20.z = param_4->z * fVar5 + param_3->z;
		local_20.w = param_4->w * fVar5 + param_3->w;

		fVar5 = CStack96.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40000010, &local_30, &local_10);
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
		fVar6 = CStack176.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40000010, &local_80, (_ray_info_out*)0x0);

		ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge fVar6: {}", fVar6);

		if ((((fVar6 == 1e+30f) || (fVar6 <= 0.01f)) ||
			(cosf(this->field_0x14b0) <= fabs(local_80.y))) ||
			(0.0f < local_80.x * local_70.x + local_80.y * local_70.y + local_80.z * local_70.z)) {
			local_90.y = fVar5 - 0.1f;
			fVar6 = CStack176.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40000010, &local_80, (_ray_info_out*)0x0);
		}

		if (((fVar6 != 1e+30f) && (0.01 < fVar6)) &&
			((((uint)local_10.pVector_0x4 & 0x1200) == 0 &&
				((fabs(local_80.y) < cosf(this->field_0x14b0) && (local_80.x * local_70.x + local_80.y * local_70.y + local_80.z * local_70.z <= 0.0f)))))) {
			fVar7 = ((this->pCollisionData)->pObbPrim->field_0x90).z - 0.2f;

			local_20.x = param_4->x * fVar7 + param_3->x;
			local_20.y = param_3->y + 1.8f;
			local_20.z = param_4->z * fVar7 + param_3->z;
			local_20.w = param_4->w * fVar7 + param_3->w;

			fVar7 = CStack96.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
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

							if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) &&
								(uVar3 = CCollision::IsVertexAboveAndAgainstObbTree(&(this->pCollisionData)->highestVertex, pTieActor->pCollisionData->pObbTree), uVar3 != 0)) {
								TieToActor(pTieActor, 0, 0, (edF32MATRIX4*)0x0);
							}
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

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::DetectGripEdge return {}", iVar4);

	return iVar4;
}

struct S_DETECT_WALL_CFG
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
};

int CActorHeroPrivate::DetectClimbCeiling(edF32VECTOR4* v0, CActor** pOutActor)
{
	CActor* pCVar1;
	CActor* pTieActor;
	bool bVar2;
	int bDetectClimbCeiling;
	float fVar4;
	CCollisionRay CStack80;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	_ray_info_out local_10;

	if ((this->field_0x1420 == 0) && (bDetectClimbCeiling = EvolutionCanClimb(), bDetectClimbCeiling != 0)) {
		IMPLEMENTATION_GUARD(
		local_20.x = v0->x;
		local_20.z = v0->z;
		local_20.w = v0->w;
		bDetectClimbCeiling = 0;
		local_20.y = v0->y + 1.16f;
		CCollisionRay::CCollisionRay((float)&DAT_3f4ccccd, &CStack80, &local_20, &g_xVector);
		if (this->field_0x1454 == false) {
			fVar4 = CCollisionRay::IntersectActors(&CStack80, (CActor*)this, (CActor*)0x0, 0x40000100, &local_30, &local_10);
		}
		else {
			fVar4 = CCollisionRay::Intersect(&CStack80, 3, (CActor*)this, (CActor*)0x0, 0x40000100, &local_30, &local_10);
		}
		if ((((fVar4 != 1e+30) && (local_30.y <= -edFCosinus[1138])) &&
			((local_10.pActor_0x0 == (CActor*)0x0 || (((uint)local_10.pVector_0x4 & 0x800) != 0)))) &&
			((((uint)local_10.pVector_0x4 & 0x200) == 0 &&
				(local_30.x * g_xVector.x + local_30.y * g_xVector.y + local_30.z * g_xVector.z <= 0.0)))) {
			this->field_0x14a8 = fVar4 - 0.4;
			if (pOutActor == (CActor**)0x0) {
				pCVar1 = this->pCheckpointManagerSubObjA_0x14a4;
				if (pCVar1 != local_10.pActor_0x0) {
					if (pCVar1 != (CActor*)0x0) {
						pTieActor = this->pTiedActor;
						if (pCVar1 == pTieActor) {
							(*(this->pVTable)->TieToActor)
								((CActor*)this, (CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
							if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) &&
								(bVar2 = CCollision::IsVertexAboveAndAgainstObbTree
								(&(this->pCollisionData)->highestVertex,
									pTieActor->pCollisionData->pObbTree), bVar2 != false)) {
								(*(this->pVTable)->TieToActor)
									((CActor*)this, pTieActor, 0, 0, (edF32MATRIX4*)0x0);
							}
						}
						this->pCheckpointManagerSubObjA_0x14a4 = (CActor*)0x0;
					}
					this->pCheckpointManagerSubObjA_0x14a4 = local_10.pActor_0x0;
					pCVar1 = this->pCheckpointManagerSubObjA_0x14a4;
					if (pCVar1 != (CActor*)0x0) {
						(*(this->pVTable)->TieToActor)
							((CActor*)this, pCVar1, 0, 1, (edF32MATRIX4*)0x0);
					}
				}
			}
			else {
				*pOutActor = local_10.pActor_0x0;
			}
			bDetectClimbCeiling = 1;
			this->field_0x1480.x = local_30.x;
			this->field_0x1480.y = local_30.y;
			this->field_0x1480.z = local_30.z;
			this->field_0x1480.w = local_30.w;
		})
	}
	else {
		bDetectClimbCeiling = 0;
	}

	return bDetectClimbCeiling;
}

bool CActorHeroPrivate::DetectClimbWall(int param_2, CActor** pOutActor, float* param_4)
{
	CCollision* pCVar1;
	CActor* pCVar2;
	CActor* pTieActor;
	bool bDetectClimbWall;
	bool bVar3;
	int iVar4;
	edF32VECTOR4* peVar5;
	float fVar6;
	float fVar7;
	float unaff_f20;
	float fVar8;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	S_DETECT_WALL_CFG local_20;
	_ray_info_out local_10;

	if (((this->field_0x1420 != 0) || (this->field_0x1454 == false)) || (iVar4 = EvolutionCanClimb(), iVar4 == 0)) {
		return false;
	}

	IMPLEMENTATION_GUARD(
	pCVar1 = this->pCollisionData;
	bDetectClimbWall = false;
	if (((pCVar1->flags_0x4 & 1) != 0) &&
		(ABS(pCVar1->aCollisionContact[0].location.y) <
			edFCosinus[(int)(ABS(this->field_0x14b8 * 1303.797) + 0.5) & 0x1fff])) {
		local_30.w = pCVar1->aCollisionContact[0].location.w;
		local_30.x = 0.0 - pCVar1->aCollisionContact[0].location.x;
		local_30.z = 0.0 - pCVar1->aCollisionContact[0].location.z;
		local_30.y = 0.0;
		edF32Vector4SafeNormalize1Hard(&local_30, &local_30);
		bVar3 = true;
		if (0.0 <= local_30.x * this->rotationQuat.x +
			local_30.y * this->rotationQuat.y +
			local_30.z * this->rotationQuat.z) goto LAB_0013d298;
	}
	bVar3 = false;
LAB_0013d298:
	local_20.field_0x0 = 1.6;
	fVar7 = 0.4;
	local_20.field_0x4 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 1.0;
	local_20.field_0x8 = this->field_0x14b8;
	if (param_2 == 0) {
		peVar5 = &eStack64;
	}
	else {
		peVar5 = &this->field_0x1470;
	}
	if (bVar3) {
		bDetectClimbWall = DetectWall(this, 3, 0x40000100, &local_30, &local_20, peVar5, &eStack80, &local_10);
		if (((local_10.pActor_0x0 != (CActor*)0x0) && (((uint)local_10.pVector_0x4 & 0x400) == 0)) ||
			(((uint)local_10.pVector_0x4 & 0x200) != 0)) {
			bDetectClimbWall = false;
		}
		if (bDetectClimbWall != false) {
			edF32Vector4SubHard(&eStack96, &this->currentLocation, &eStack80);
			eStack96.y = 0.0;
			fVar8 = ((this->pCollisionData)->pObbPrim->field_0x90).z;
			fVar6 = edF32Vector4DotProductHard(&eStack96, &this->field_0x1470);
			unaff_f20 = fVar6 - fVar8;
			if (fVar7 < unaff_f20) {
				bDetectClimbWall = false;
			}
		}
	}
	if (bDetectClimbWall == false) {
		bDetectClimbWall =
			DetectWall(this, 3, 0x40000100, &this->rotationQuat, &local_20, peVar5,
				&eStack80, &local_10);
		if (((local_10.pActor_0x0 != (CActor*)0x0) && (((uint)local_10.pVector_0x4 & 0x400) == 0)) ||
			(((uint)local_10.pVector_0x4 & 0x200) != 0)) {
			bDetectClimbWall = false;
		}
		if (bDetectClimbWall != false) {
			edF32Vector4SubHard(&eStack96, &this->currentLocation, &eStack80);
			eStack96.y = 0.0;
			fVar8 = ((this->pCollisionData)->pObbPrim->field_0x90).z;
			fVar6 = edF32Vector4DotProductHard(&eStack96, &this->field_0x1470);
			unaff_f20 = fVar6 - fVar8;
			if (fVar7 < unaff_f20) {
				bDetectClimbWall = false;
			}
		}
	}
	if (bDetectClimbWall != false) {
		if (param_2 == 0) {
			CActor::UpdateNormal(8.0, (CActor*)this, &this->field_0x1470, &eStack64);
		}
		if (pOutActor == (CActor**)0x0) {
			pCVar2 = this->pCheckpointManagerSubObjA_0x14a4;
			if (pCVar2 != local_10.pActor_0x0) {
				if (pCVar2 != (CActor*)0x0) {
					pTieActor = this->pTiedActor;
					if (pCVar2 == pTieActor) {
						(*(this->pVTable)->TieToActor)
							((CActor*)this, (CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
						if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) &&
							(bVar3 = CCollision::IsVertexAboveAndAgainstObbTree
							(&(this->pCollisionData)->highestVertex,
								pTieActor->pCollisionData->pObbTree), bVar3 != false)) {
							(*(this->pVTable)->TieToActor)
								((CActor*)this, pTieActor, 0, 0, (edF32MATRIX4*)0x0);
						}
					}
					this->pCheckpointManagerSubObjA_0x14a4 = (CActor*)0x0;
				}
				this->pCheckpointManagerSubObjA_0x14a4 = local_10.pActor_0x0;
				pCVar2 = this->pCheckpointManagerSubObjA_0x14a4;
				if (pCVar2 != (CActor*)0x0) {
					(*(this->pVTable)->TieToActor)
						((CActor*)this, pCVar2, 0, 1, (edF32MATRIX4*)0x0);
				}
			}
		}
		else {
			*pOutActor = local_10.pActor_0x0;
		}
		if (param_4 != (float*)0x0) {
			*param_4 = unaff_f20;
		}
	})

	return bDetectClimbWall;
}

int CActorHeroPrivate::DetectClimbCeilingFromGrip(CActor** pOutActor, edF32VECTOR4* pPosition)
{
	int bClimbCeilingFromGrip;
	float fVar1;
	CCollisionRay CStack64;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	bClimbCeilingFromGrip = EvolutionCanClimb();
	if (bClimbCeilingFromGrip == 0) {
		bClimbCeilingFromGrip = 0;
	}
	else {
		IMPLEMENTATION_GUARD(
		fVar1 = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.1;
		local_10.x = this->rotationQuat.x * fVar1 +
			this->currentLocation.x;
		local_10.z = this->rotationQuat.z * fVar1 +
			this->currentLocation.z;
		local_10.w = this->rotationQuat.w * fVar1 +
			this->currentLocation.w;
		local_10.y = (this->rotationQuat.y * fVar1 +
			this->currentLocation.y) - 0.2;
		bClimbCeilingFromGrip = DetectClimbCeiling(this, &local_10, pOutActor);
		if (bClimbCeilingFromGrip != 0) {
			local_20.x = this->currentLocation.x;
			local_20.z = this->currentLocation.z;
			local_20.w = this->currentLocation.w;
			local_20.y = this->currentLocation.y + 1.16;
			CCollisionRay::CCollisionRay
			(((this->pCollisionData)->pObbPrim->field_0x90).z + 0.1,
				&CStack64, &local_20, &this->rotationQuat);
			fVar1 = CCollisionRay::Intersect
			(&CStack64, 3, (CActor*)this, (CActor*)0x0, 0x40000001, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			if (fVar1 != 1e+30) {
				bClimbCeilingFromGrip = 0;
			}
		}
		if ((pPosition != (edF32VECTOR4*)0x0) && (bClimbCeilingFromGrip != 0)) {
			pPosition->x = local_10.x;
			pPosition->y = local_10.y;
			pPosition->z = local_10.z;
			pPosition->w = local_10.w;
			pPosition->y = pPosition->y + this->field_0x14a8;
		})
	}

	return bClimbCeilingFromGrip;
}

bool CActorHeroPrivate::EvolutionBoomyCanControl()
{
	return 2 < CLevelScheduler::ScenVar_Get(SCENE_VAR_BOOMY);
}

bool CActorHeroPrivate::EvolutionBoomyCanSnipe()
{
	return 1 < CLevelScheduler::ScenVar_Get(SCENE_VAR_BOOMY);
}

bool CActorHeroPrivate::EvolutionBoomyCanLaunch()
{
	return 0 < CLevelScheduler::ScenVar_Get(SCENE_VAR_BOOMY);
}

int CActorHeroPrivate::EvolutionBounceCanJump()
{
	return CLevelScheduler::ScenVar_Get(SCENE_VAR_BOUNCE_JUMP);
}

bool CActorHeroPrivate::EvolutionCanClimb()
{
	return CLevelScheduler::ScenVar_Get(SCENE_VAR_CLIMB);
}

bool CActorHeroPrivate::EvolutionTobogganCanJump()
{
	int iVar1;

	iVar1 = CLevelScheduler::ScenVar_Get(0x10);
	return 0 < iVar1;
}

bool CActorHeroPrivate::EvolutionTobogganUnknown()
{
	int iVar1;

	iVar1 = CLevelScheduler::ScenVar_Get(0x10);
	return 1 < iVar1;
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
			this->dynamic.rotationQuat.z * fVar6 * this->bounceLocation.z < 0.0f;

		uVar4 = (uint)bVar3;

		if ((!bVar3) && (pCVar2 = this->pCheckpointManagerSubObjA_0x14a4, pCVar2 != (CActor*)0x0)) {
			pTieActor = this->pTiedActor;
			if (pCVar2 == pTieActor) {
				TieToActor((CActor*)0x0, 0, 0x1, (edF32MATRIX4*)0x0);
				if (((pTieActor->pCollisionData->flags_0x0 & 0x80) != 0) &&
					(uVar5 = CCollision::IsVertexAboveAndAgainstObbTree(&(this->pCollisionData)->highestVertex, pTieActor->pCollisionData->pObbTree), uVar5 != 0)) {
					TieToActor(pTieActor, 0, 0, (edF32MATRIX4*)0x0);
				}
			}
			this->pCheckpointManagerSubObjA_0x14a4 = (CActor*)0x0;
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
				local_8 = this->field_0x14c4;
				local_20 = *(float*)&this->field_0x14c8;
				local_1c = ((this->pCollisionData)->pObbPrim->field_0x90).z + 0.2;
				local_18 = this->field_0x14c4;
				lVar4 = DetectWall(this, 3, 0x40000080, (float*)&this->rotationQuat,
					&local_10, 0, 0, 0);
				if ((lVar4 == 0) ||
					(lVar4 = DetectWall(this, 3, 0x40000080, (float*)&this->rotationQuat,
						&local_20, 0, 0, 0), lVar4 == 0)) {
					bVar2 = false;
				}
				else {
					v0 = &this->bounceLocation;
					this->bounceLocation.x = pCVar1->aCollisionContact[0].location.x;
					this->bounceLocation.y = 0.0;
					this->bounceLocation.z = pCVar1->aCollisionContact[0].location.z;
					this->bounceLocation.w = 0.0;
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

void CActorHeroPrivate::ChangeCollisionSphereForToboggan(float param_2)
{
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.w = 0.0f;
	local_10.x = 0.5f;
	local_10.y = 0.5f;
	local_10.z = 0.5f;

	local_20.x = this->field_0x28c.x;
	local_20.z = this->field_0x28c.z;
	local_20.w = 1.0f;
	local_20.y = this->field_0x28c.y * 0.6f;

	ChangeCollisionSphere(param_2, &local_10, &local_20);
	return;
}

void CActorHeroPrivate::ChangeCollisionSphereForGlide(float param_1, float param_2)
{
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.w = 0.0f;
	local_20.x = 0.0f;
	local_20.z = 0.0f;
	local_10.x = 0.6f;
	local_10.y = 0.4f;
	local_10.z = 0.6f;
	local_20.y = 0.4f;
	local_20.w = 1.0f;
	local_40.w = 1.0f;
	local_30.x = 0.4f;
	local_30.z = 0.4f;
	local_30.y = 0.6f;
	local_40.y = 0.6f;
	local_30.w = 0.0f;
	local_40.x = 0.0f;
	local_40.z = 0.0f;

	edF32Vector4LERPHard(param_2, &local_10, &local_10, &local_30);
	edF32Vector4LERPHard(param_2, &local_20, &local_20, &local_40);

	local_20.w = 1.0f;

	ChangeCollisionSphere(param_1, &local_10, &local_20);
}

void CActorHeroPrivate::ConvertSpeedPlayerToSpeedSumForceExt()

{
	float fVar1;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	edF32Vector4ScaleHard(this->dynamic.speed, &eStack16, &this->dynamic.rotationQuat);
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &eStack32, &this->dynamicExt.normalizedTranslation);
	edF32Vector4AddHard(&eStack32, &eStack32, &eStack16);

	fVar1 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &eStack32);
	this->dynamicExt.field_0x6c = fVar1;

	this->dynamic.speed = 0.0f;

	if (this->dynamicExt.field_0x6c != 0.0f) {
		this->dynamic.rotationQuat = this->dynamicExt.normalizedTranslation;
	}

	return;
}

void CActorHeroPrivate::BeginToboggan()
{
	CFxHandle* pCVar1;
	int* piVar2;
	int iVar3;
	//StaticMeshComponent* pSVar4;
	CCamera* pCVar5;
	bool bVar6;
	CBehaviour* pCVar7;
	CCameraManager* iVar7;

	this->field_0x10fc = (CFxHandle*)0x0;
	this->field_0x1100 = (int*)0x0;

	pCVar7 = CActor::GetBehaviour(0xb);

	if (pCVar7 == (CBehaviour*)0x0) {
		EvolutionTobogganUnknown();
	}
	else {
		IMPLEMENTATION_GUARD_LOG(
		this->field_0x10fc = &this->field_0x1fe0;
		this->field_0x1100 = &this->cameraPan_0x1ff0.textureIndex;
		this->field_0x110c = &this->field_0x20b0;
		this->field_0x1110 = &this->field_0x20bc;
		this->field_0x1114 = &this->cameraPan_0x20d0.textureIndex;)
	}

	bVar6 = EvolutionTobogganUnknown();

	if ((bVar6 == false) || (this->field_0x10fc == (CFxHandle*)0x0)) {
		pCVar7 = CActor::GetBehaviour(10);

		if (pCVar7 == (CBehaviour*)0x0) {
			EvolutionTobogganCanJump();
		}
		else {
			IMPLEMENTATION_GUARD_LOG(
			this->field_0x10fc = &this->field_0x1e20;
			this->field_0x1100 = &this->cameraPan_0x1e30.textureIndex;
			this->field_0x110c = &this->field_0x1ef0;
			this->field_0x1110 = &this->field_0x1efc;
			this->field_0x1114 = &this->cameraPan_0x1f10.textureIndex;)
		}

		bVar6 = EvolutionTobogganCanJump();

		if (((bVar6 == false) || (this->field_0x10fc == (CFxHandle*)0x0)) &&
			(pCVar7 = CActor::GetBehaviour(9), pCVar7 != (CBehaviour*)0x0)) {
			IMPLEMENTATION_GUARD_LOG(
			this->field_0x10fc = &this->field_0x1c60;
			this->field_0x1100 = &this->cameraPan_0x1c70.textureIndex;
			this->field_0x110c = &this->field_0x1d30;
			this->field_0x1110 = &this->field_0x1d3c;
			this->field_0x1114 = &this->cameraPan_0x1d50.textureIndex;
			this->field_0x110c = (CFxHandle*)0x0;
			this->field_0x1110 = (CFxHandle*)0x0;
			this->field_0x1114 = 0;)
		}
	}

	IMPLEMENTATION_GUARD_LOG(
	pCVar1 = this->field_0x110c;
	if (pCVar1 != (CFxHandle*)0x0) {
		CFxHandle::SV_FX_Start(pCVar1);
		pCVar1 = this->field_0x110c;
		piVar2 = (int*)pCVar1->field_0x4;
		if (((piVar2 != (int*)0x0) && (iVar3 = pCVar1->field_0x0, iVar3 != 0)) && (iVar3 == piVar2[6])) {
			(**(code**)(*piVar2 + 0x10))(0, 0);
		}
	}

	pCVar1 = this->field_0x1110;
	if (pCVar1 != (CFxHandle*)0x0) {
		CFxHandle::SV_FX_Start(pCVar1);
		pCVar1 = this->field_0x1110;
		piVar2 = (int*)pCVar1->field_0x4;
		if (((piVar2 != (int*)0x0) && (iVar3 = pCVar1->field_0x0, iVar3 != 0)) && (iVar3 == piVar2[6])) {
			(**(code**)(*piVar2 + 0x10))(0, 0);
		}
	}

	pSVar4 = (StaticMeshComponent*)this->field_0x1114;
	if (pSVar4 != (StaticMeshComponent*)0x0) {
		FUN_00114bc0(0.0, 0.0, pSVar4, (ed_3D_Scene*)0x0, (ed_g3d_manager*)0x0, (char*)0x0);
		*(undefined4*)&this->field_0x1118 = 0;
	})

	ConvertSpeedPlayerToSpeedSumForceExt();

	this->flags = this->flags | 0x1000;

	ChangeCollisionSphereForToboggan(0.2f);

	this->normalValue = g_xVector;

	iVar7 = (CCameraManager*)CScene::GetManager(MO_Camera);

	if ((iVar7->pActiveCamera != (CCamera*)0x0) && ((iVar7->pActiveCamera->flags_0xc & 0x40000) == 0)) {
		CCameraManager::_gThis->PushCamera(this->pCameraKyaJamgut, 0);
	}

	pCVar5 = this->pCameraKyaJamgut;
	pCVar5->fov = this->field_0x15c4;

	pCVar5->SetDistance(this->field_0x15c8);
	this->field_0x154c = 0.0f;
	return;
}

void CActorHeroPrivate::SlideOnToboggan(float param_1, float param_2, float param_3, float param_4, edF32VECTOR4* param_6, edF32VECTOR4* param_7, uint dynFlags)
{
	CCollision* pCVar1;
	bool bVar2;
	edF32VECTOR4* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pCVar1 = this->pCollisionData;

	if ((pCVar1->flags_0x4 & 2) == 0) {
		fVar4 = edFIntervalLERP(this->collisionContact.location.y, 0.8658976f, 1.0f, param_2 , param_1 + (param_2 - this->field_0x10c4));
		dynFlags = dynFlags | 0x21;
	}
	else {
		ComputeSlidingForce(&eStack16, 1);
		edF32Vector4ScaleHard(this->field_0x1078, &eStack16, &eStack16);

		peVar3 = this->dynamicExt.aVelocity;
		edF32Vector4AddHard(peVar3, peVar3, &eStack16);

		fVar4 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
		this->dynamicExt.aVelocityMagnitudes[0] = fVar4;
		fVar5 = edF32Vector4SafeNormalize0Hard(&eStack16, &eStack16);
		fVar4 = edF32Vector4DotProductHard(&this->dynamic.velocityDirectionEuler, &eStack16);
		edF32Vector4ScaleHard(-param_3, &eStack32, param_7);
		edF32Vector4ScaleHard(param_4, &eStack48, param_6);
		edF32Vector4AddHard(&eStack32, &eStack32, &eStack48);
		fVar6 = edF32Vector4DotProductHard(&eStack32, &eStack16);

		bVar2 = EvolutionTobogganUnknown();
		if (bVar2 == false) {
			fVar7 = edFIntervalLERP(pCVar1->aCollisionContact[1].location.y, -0.8658976f, 0.8658976f, 0.2f, 1.0f);
		}
		else {
			fVar7 = edFIntervalLERP(fabs(pCVar1->aCollisionContact[1].location.y), 0.0f, 0.8658976f, 0.8f, 1.2f);
		}

		fVar8 = this->field_0xa80;
		fVar9 = param_2;
		if ((fVar8 <= param_2) && (fVar9 = 2.0f, 2.0f <= fVar8)) {
			fVar9 = fVar8;
		}

		edF32Vector4ScaleHard(fVar9 * fVar7, &eStack32, &eStack32);

		if (((fVar4 < -0.8658976f) && (fVar6 < -0.8658976f)) &&
			(fVar7 = edF32Vector4SafeNormalize0Hard(&eStack48, &eStack32), fVar5 < fVar7)) {
			if (fVar6 <= fVar4) {
				fVar4 = fVar6;
			}
			fVar4 = edFIntervalUnitDstLERP(fVar4, -1.0f, -0.8658976f);
			edF32Vector4ScaleHard(fVar5 * fVar4, &eStack32, &eStack48);
		}

		peVar3 = this->dynamicExt.aVelocity;
		edF32Vector4AddHard(peVar3, peVar3, &eStack32);
		fVar4 = edF32Vector4GetDistHard(this->dynamicExt.aVelocity);
		this->dynamicExt.aVelocityMagnitudes[0] = fVar4;
		fVar4 = edFIntervalLERP(pCVar1->aCollisionContact[1].location.y, 0.8658976f, 1.0f, param_2, param_1 + (param_2 - this->field_0x10c4));
	}

	ManageDyn(4.0f, dynFlags, (CActorsTable*)0x0);

	fVar5 = 65.0f;
	if (fVar4 <= 65.0f) {
		fVar5 = fVar4;
	}
	if (fVar5 < this->dynamic.linearAcceleration) {
		this->dynamicExt.field_0x6c = fVar5;
	}

	return;
}

CAM_QUAKE wall_quake = {
	{ 0.5f, 0.5f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 0.0f, 0.0f },
	0xa,
	0.2f,
	0.1f,
	0.1f
};

bool CActorHeroPrivate::TobogganBounceOnWall(edF32VECTOR4* param_2, edF32VECTOR4* param_3, CActor* pActor)
{
	Timer* pTVar1;
	CCameraManager* pCameraManager;
	CPlayerInput* uVar2;
	CPlayerInput* uVar3;
	long lVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	CAM_QUAKE local_50;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	fVar3 = edFIntervalLERP(this->field_0xa80, 10.0f, this->field_0x10c4, 2.0f, 5.0f);

	if (pActor == (CActor*)0x0) {
		if (this->field_0xa80 <= 10.0f) {
			return false;
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		lVar2 = BreakActor(fVar3, 6.0f, 6.0f, this, pActor, 0, 1, 0);
		if (lVar2 == 1) {
			return false;
		})
	}

	fVar5 = param_3->y;
	local_20.y = sqrtf(1.0f - fVar5 * fVar5);
	local_20.x = param_2->x;
	local_20.y = local_20.y + (fabs(fVar5) - local_20.y) * 0.75f;
	local_20.z = param_2->z;
	local_20.w = param_2->w;

	fVar5 = param_2->y;
	if (param_2->y < local_20.y) {
		edF32Vector4NormalizeHard(&local_20, &local_20);
		fVar5 = local_20.y;
	}

	local_20.y = fVar5;
	edF32Vector4ScaleHard(this->field_0xa80, &eStack16, &this->dynamic.velocityDirectionEuler);
	edReflectVectorOnPlane(0.7f, &eStack16, &eStack16, &local_20, 1);
	fVar4 = edF32Vector4SafeNormalize0Hard(&this->dynamicExt.normalizedTranslation, &eStack16);
	fVar5 = 25.0f;
	if (fVar4 <= 25.0f) {
		fVar5 = fVar4;
	}
	this->dynamicExt.field_0x6c = fVar5;
	this->field_0x10cc = this->field_0x10cc * 0.7f;
	edF32Vector4GetNegHard(&this->field_0x10e0, &this->dynamicExt.normalizedTranslation);

	pTVar1 = Timer::GetTimer();
	if (this->field_0x155c <= pTVar1->scaledTotalTime) {
		LifeDecrease(fVar3);
	}

	SetInvincible(2.0f, 1);

	this->field_0xf00 = this->currentLocation;

	// Probably from a constructor.
	local_50 = wall_quake;
	
	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
	local_50.field_0x24 = edFIntervalLERP(fVar3, 2.0f, 5.0f, 0.2f, 0.4f);

	local_50.field_0x0.x = edFIntervalLERP(fVar3, 2.0f, 5.0f, 0.05f, 0.2f);
	local_50.field_0x0.z = 0.0f;
	local_50.field_0x0.w = 0.0f;
	local_50.field_0x0.y = local_50.field_0x0.x;

	local_50.field_0x10.x = edFIntervalLERP(fVar3, 2.0f, 5.0f, 0.5f, 1.0f);
	local_50.field_0x10.z = 0;
	local_50.field_0x10.w = 0;
	local_50.field_0x10.y = local_50.field_0x10.x;

	pCameraManager->SetEarthQuake(&local_50);

	if ((this->field_0x10c0 + this->field_0x10c4) * 0.5f < this->field_0xa80) {
		uVar2 = GetInputManager(1, 0);
		if (uVar2 != (CPlayerInput*)0x0) {
			CPlayerInput::FUN_001b66f0(1.0, 0.0, 0.2, 0.0, &uVar2->field_0x40, 0);
		}

		SetState(0xec, 0xffffffff);
	}
	else {
		uVar3 = GetInputManager(1, 0);
		if (uVar3 != (CPlayerInput*)0x0) {
			CPlayerInput::FUN_001b66f0(1.0, 0.0, 0.1, 0.0, &uVar3->field_0x40, 0);
		}

		SetState(STATE_HERO_TOBOGGAN_JUMP_2, 0xffffffff);
	}

	return true;
}


void CActorHeroPrivate::Landing()
{
	CActor* pReceiver;

	pReceiver = this->field_0xf18;
	if (pReceiver != (CActor*)0x0) {
		DoMessage(pReceiver, (ACTOR_MESSAGE)0x1f, (MSG_PARAM)1);
		this->field_0xf18 = (CActor*)0x0;
	}
	return;
}

void CActorHeroPrivate::AdjustLocalMatrixFromNormal(float param_1, edF32VECTOR4* pNormal)

{
	ed_3d_hierarchy_node* peVar1;
	edColPRIM_OBJECT* peVar2;
	edF32VECTOR4* v0;
	edF32VECTOR4 eStack16;

	peVar1 = this->pMeshTransform;
	peVar2 = (this->pCollisionData)->pObbPrim;

	edF32Vector4ScaleHard(-(peVar2->field_0x90).y, &eStack16, pNormal);

	v0 = &peVar1->base.transformA.rowT;
	edF32Vector4AddHard(v0, v0, &eStack16);
	(peVar1->base).transformA.db = (peVar1->base).transformA.db + (peVar2->field_0xb0).y + param_1;
	return;
}

bool CActorHeroPrivate::UpdateOrientationFromWind(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	bool ret;
	float fVar1;
	float fVar2;
	edF32VECTOR4 local_10;

	local_10.x = v0->x;
	local_10.z = v0->z;
	local_10.w = v0->w;
	local_10.y = 0.0f;

	fVar1 = edF32Vector4SafeNormalize0Hard(&local_10, &local_10);

	if (2.0f < fVar1) {
		fVar2 = this->field_0x1040;
		fVar1 = edFIntervalLERP(fVar1, 5.0f, 60.0f, fVar2 * 0.2f, fVar2 * 4.0f);
		SV_UpdateOrientation2D(fVar1, &local_10, 0);

		if (v1 != (edF32VECTOR4*)0x0) {
			*v1 = local_10;
		}

		ret = true;
	}
	else {
		if (v1 != (edF32VECTOR4*)0x0) {
			*v1 = gF32Vector4UnitX;
		}

		ret = false;
	}

	return ret;
}

void CActorHeroPrivate::ConvertSpeedSumForceExtToSpeedPlayer()
{
	float fVar1;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	edF32Vector4ScaleHard(this->dynamic.speed, &eStack16, &this->dynamic.rotationQuat);
	edF32Vector4ScaleHard(this->dynamicExt.field_0x6c, &local_20, &this->dynamicExt.normalizedTranslation);
	edF32Vector4AddHard(&local_20, &local_20, &eStack16);

	fVar1 = edF32Vector4SafeNormalize0Hard(&local_20, &local_20);
	this->dynamic.speed = fVar1;

	if (0.001f < fVar1) {
		this->dynamic.rotationQuat = local_20;
	}

	this->dynamicExt.field_0x6c = 0.0f;
	this->dynamicExt.normalizedTranslation = this->dynamic.rotationQuat;
	return;
}

bool CActorHeroPrivate::GetNormalInFrontOf(float rayLength, edF32VECTOR4* pRayDirection, edF32VECTOR4* param_4, uint flags, edF32VECTOR4* pHitLocation, CActor** pActor)
{
	float hitDistance;
	CCollisionRay collisionRay;
	edF32VECTOR4 eStack32;
	_ray_info_out rayInfoOut;

	edF32Vector4AddHard(&eStack32, &this->currentLocation, param_4);
	collisionRay = CCollisionRay(rayLength, &eStack32, pRayDirection);
	hitDistance = collisionRay.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, this, (CActor*)0x0, flags, pHitLocation, &rayInfoOut);

	if (pActor != (CActor**)0x0) {
		*pActor = rayInfoOut.pActor_0x0;
	}

	return hitDistance != 1e+30f;
}

void CActorHeroPrivate::SetInvincible(float t0, int param_3)
{
	Timer* pTVar1;

	if (param_3 == 0) {
		pTVar1 = Timer::GetTimer();
		this->field_0x155c = pTVar1->scaledTotalTime;
		this->field_0x1560 = 0;
	}
	else {
		pTVar1 = Timer::GetTimer();
		this->field_0x155c = t0 + pTVar1->scaledTotalTime;
	}
	return;
}

bool CActorHeroPrivate::CheckHitAndDeath()
{
	CCollision* pCVar1;
	bool bVar2;
	StateConfig* pSVar3;
	uint uVar4;
	undefined4 uVar5;
	int iVar6;
	CLifeInterface* pLifeInterface;
	ulong uVar7;
	float fVar8;
	//_msg_hit_param local_90[2];
	edF32VECTOR4 local_10;

	uVar7 = GetBehaviourFlags(this->curBehaviourId);
	if ((uVar7 & 8) == 0) {
		return false;
	}

	iVar6 = this->actorState;
	if (iVar6 == -1) {
		uVar4 = 0;
	}
	else {
		pSVar3 = this->GetStateCfg(iVar6);
		uVar4 = pSVar3->flags_0x4 & 1;
	}

	if (uVar4 != 0) {
		return false;
	}

	bVar2 = ColWithLava();
	if ((bVar2 == false) && (bVar2 = ColWithCactus(), bVar2 == false)) {
		return false;
	}

	pCVar1 = this->pCollisionData;
	bVar2 = ColWithCactus();
	if (bVar2 != false) {
		this->field_0x2e4 = 10.0f;
		if (((pCVar1->flags_0x4 & 2) == 0) || ((pCVar1->flags_0x4 & 4) == 0)) {
			this->SetBehaviour(7, 0xa3, 0xffffffff);
		}
		else {
			this->SetBehaviour(7, 0x9e, 0xffffffff);
		}
		return true;
	}

	IMPLEMENTATION_GUARD(
	local_10.x = 0.0;
	local_10.y = 0.0;
	local_10.z = 0.0;
	local_10.w = 0.0;
	uVar4 = pCVar1->aCollisionContact[1].materialFlags & 0xf;
	if (uVar4 == 0) {
		uVar4 = CScene::_pinstance->defaultMaterialIndex;
	}
	if (uVar4 == 3) {
		edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)(pCVar1->aCollisionContact + 1));
	}
	uVar4 = pCVar1->aCollisionContact[0].materialFlags & 0xf;
	if (uVar4 == 0) {
		uVar4 = CScene::_pinstance->defaultMaterialIndex;
	}
	if (uVar4 == 3) {
		edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)pCVar1->aCollisionContact);
	}
	uVar4 = pCVar1->aCollisionContact[2].materialFlags & 0xf;
	if (uVar4 == 0) {
		uVar4 = CScene::_pinstance->defaultMaterialIndex;
	}
	if (uVar4 == 3) {
		edF32Vector4AddHard(&local_10, &local_10, (edF32VECTOR4*)(pCVar1->aCollisionContact + 2));
	}
	edF32Vector4NormalizeHard(&local_10, &local_10);
	uVar4 = CActorHero::TestState_IsFlying((CActorHero*)this, 0xffffffff);
	if ((uVar4 != 0) && (uVar4 = CActorHero::TestState_IsInTheWind((CActorHero*)this, 0xffffffff), uVar4 != 0)) {
		uVar5 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
		if (uVar5 == 0) {
			fVar8 = 0.0;
		}
		else {
			iVar6 = (int)(*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
			fVar8 = *(float*)(iVar6 + 0x1c);
		}
		if (fVar8 < -1.0) {
			pLifeInterface = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
			fVar8 = CLifeInterface::GetValueMax(pLifeInterface);
			iVar6 = (int)fVar8 / 0x32;
			if (5 < iVar6) {
				iVar6 = 5;
			}
			local_90[0].field_0xc = (float)iVar6 * 2.0;
			goto LAB_0013e9f0;
		}
	}
	local_90[0].field_0xc = 2.0;
LAB_0013e9f0:
	local_90[0].field_0x8 = 1;
	local_90[0].field_0x0 = 0;
	AccomplishHit(this, (CActor*)0x0, local_90, &local_10);)
	return true;
}

bool CActorHeroPrivate::GetSomethingInFrontOf_001473e0()
{
	edF32VECTOR4 local_20;
	local_20.x = this->currentLocation.x;
	local_20.y = this->currentLocation.y + 0.2f;
	local_20.z = this->currentLocation.z;
	local_20.w = 1.0f;

	CCollisionRay CStack64 = CCollisionRay(1.4f, &local_20, &g_xVector);
	float fVar8 = CStack64.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, (CActor*)this, (CActor*)0x0, 0x40000040, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
	return fVar8 == 1e+30;
}

void CActorHeroPrivate::ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable)
{
	Timer* pTVar4;
	StateConfig* pAVar5;
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
			iVar7 = (int)this->data.pCollisionData;
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

	return;
}

CActorWindState* CActorHeroPrivate::GetWindState()
{
	return &field_0x1190;
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
	float* puVar5;
	float in_f0;
	float fVar6;
	float puVar7;
	float puVar8;
	//CBehaviourVtable* pCVar9;
	//CBehaviourVtable* pCVar10;
	//edAnmMacroBlendN local_c;
	//edAnmMacroBlendN local_8;
	edAnmMacroBlendN local_4;

	ACTOR_HERO_LOG(LogLevel::Verbose, "CActorHeroPrivate::AnimEvaluate param_2: {} newAnim: {}", param_2, newAnim);

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
			char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
			AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

			peVar2 = pAnimator->pAnimKeyTableEntry;
			pValue->field_0x10 = this->field_0x10f8;
			pValue->field_0xc = 1.0f - pValue->field_0x10;
		}
		else {
			if ((newAnim == 0xe0) || (newAnim == 0xd6)) {
				char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
				AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
				peVar2 = pAnimator->pAnimKeyTableEntry;
				pValue->field_0x10 = this->field_0x10f4;
				pValue->field_0xc = 1.0f - pValue->field_0x10;
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
							fVar6 = edFIntervalUnitDstLERP(this->normalValue.y, 1.0f, yLimit);

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
							fVar6 = edFIntervalUnitDstLERP(this->normalValue.y, yLimit, 1.0f);

							pValue->field_0x10 = fVar6;
							pValue->field_0xc = 1.0f - pValue->field_0x10;
							pValue->field_0x14 = 0.0f;

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
								local_4 = edAnmMacroBlendN(pAnimator->pAnimKeyTableEntry);
								puVar7 = this->field_0x11f4;
								//puVar4 = &local_4->flags + local_4->keyIndex_0x8;
								char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
								AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
								puVar5 = &pValue->field_0xc;

								if (1.0f < puVar7) {
									puVar8 = 1.0f;
								}
								else {
									puVar8 = -1.0f;
									if (-1.0 <= puVar7) {
										puVar8 = puVar7;
									}
								}
								if (puVar8 < 0.0f) {
									IMPLEMENTATION_GUARD(
									fVar6 = this->field_0x11f8;
									if (fVar6 < 0.0) {
										CActor::SV_Blend3AnimationsWith2Ratios(-fVar6, -puVar8, &local_4, 1, 0, 3);
										puVar4[5] = 0;
									}
									else {
										CActor::SV_Blend3AnimationsWith2Ratios(fVar6, -puVar8, &local_4, 1, 2, 3);
										*puVar5 = 0.0f;
									}
									puVar4[7] = 0;
									puVar4[8] = 0;)
								}
								else {
									fVar6 = this->field_0x11fc;
									if (0.0 < fVar6) {
										IMPLEMENTATION_GUARD(
										puVar4[8] = (uint)fVar6;
										(&local_4->flags + local_4->keyIndex_0x8)[4] =
											(uint)(1.0 - (float)(&local_4->flags + local_4->keyIndex_0x8)[8]);
										puVar4[7] = 0;
										puVar4[5] = 0;
										*puVar5 = 0.0f;)
									}
									else {
										fVar6 = this->field_0x11f8;
										if (fVar6 < 0.0) {
											CActor::SV_Blend3AnimationsWith2Ratios(-fVar6, puVar8, &local_4, 1, 0, 4);
											pValue->field_0x14 = 0.0f;
										}
										else {
											CActor::SV_Blend3AnimationsWith2Ratios(fVar6, puVar8, &local_4, 1, 2, 4);
											*puVar5 = 0.0f;
										}

										pValue->field_0x1c = 0.0f;
									}

									pValue->field_0x14 = 0.0f;
								}
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

int CBehaviourHeroDefault::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorHeroPrivate* pHeroRef;
	CCollision* pCollisionRef;
	bool bVar3;
	CLifeInterface* pLifeInterface;
	CPlayerInput* pPlayerInput;
	uint uVar6;
	int iVar7;
	Timer* pTimer;
	StateConfig* pStateCfg;
	float* pfVar10;
	edF32VECTOR4* v0;
	float fVar11;
	float fVar12;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	pHeroRef = this->pHero;

	pLifeInterface = pHeroRef->GetLifeInterface();

	fVar11 = pLifeInterface->GetValue();
	bVar3 = fVar11 - pHeroRef->field_0x2e4 <= 0.0f;

	if (!bVar3) {
		iVar7 = pHeroRef->actorState;
		if (iVar7 == -1) {
			uVar6 = 0;
		}
		else {
			pStateCfg = pHeroRef->GetStateCfg(iVar7);
			uVar6 = pStateCfg->flags_0x4 & 1;
		}
		bVar3 = uVar6 != 0;
	}

	if (((bVar3) || (0.0f < this->pHero->field_0x1558) ||
		(bVar3 = this->pHero->TestState_IsInCheatMode()), bVar3 != false)) {
		iVar7 = 0;
	}
	else {
		if (msg == 0x6a) {
			uVar6 = this->pHero->TestState_IsCrouched(0xffffffff);

			if (uVar6 == 0) {
				pHeroRef = this->pHero;
				iVar7 = pHeroRef->actorState;

				if (iVar7 == -1) {
					uVar6 = 0;
				}
				else {
					pStateCfg = pHeroRef->GetStateCfg(iVar7);
					uVar6 = pStateCfg->flags_0x4 & 0x100;
				}

				if (uVar6 != 0) {
					pHeroRef = this->pHero;
					iVar7 = pHeroRef->actorState;

					if (iVar7 == 0x11a) {
						pHeroRef->SetState(0x11b, 0xffffffff);
					}
					else {
						if (iVar7 == 0x76) {
							pHeroRef->SetState(0x8a, 0xffffffff);
						}
						else {
							pHeroRef->SetState(0x84, 0xffffffff);
						}
					}
				}
			}
			iVar7 = 1;
		}
		else {
			if (msg == 0x4b) {
				IMPLEMENTATION_GUARD(
				uVar6 = this->pHero->TestState_IsOnAToboggan(0xffffffff);

				if ((uVar6 == 0) || (iVar7 = CLevelScheduler::ScenVar_Get(0x10), iVar7 < 2)) {
					iVar7 = 0;
				}
				else {
					pTimer = GetTimer();
					edF32Vector4ScaleHard
					(*(float*)((int)pMsgParam + 0x10) / pTimer->cutsceneDeltaTime, &eStack96, (edF32VECTOR4*)pMsgParam);
					pHeroRef = this->pHero;
					pTimer = GetTimer();
					edF32Vector4ScaleHard(0.02 / pTimer->cutsceneDeltaTime, &eStack112, &eStack96);
					v0 = pHeroRef->dynamicExt.aVelocity;
					edF32Vector4AddHard(v0, v0, &eStack112);
					fVar11 = edF32Vector4GetDistHard(pHeroRef->dynamicExt.aVelocity);
					pHeroRef->dynamicExt.aVelocityMagnitudes[0] = fVar11;
					pHeroRef = this->pHero;
					fVar11 = *(float*)((int)pMsgParam + 0x10) + pHeroRef->field_0xa80;
					if (pHeroRef->field_0x10cc < fVar11) {
						pHeroRef->field_0x10cc = fVar11;
						pHeroRef = this->pHero;
						fVar11 = pHeroRef->field_0x10cc;
						pfVar10 = &pHeroRef->field_0x10cc;
						if (65.0 < fVar11) {
							*pfVar10 = 65.0;
						}
						else {
							fVar12 = pHeroRef->field_0x10c0;
							if (fVar11 < fVar12) {
								*pfVar10 = fVar12;
							}
						}
					}
					iVar7 = 1;
					(this->pHero->base).field_0x10d4 = *(float*)((int)pMsgParam + 0x14);
				})
			}
			else {
				if (msg == 0x33) {
					IMPLEMENTATION_GUARD(
					edF32Matrix4FromEulerSoft(&eStack64, *(edF32VECTOR3**)((int)pMsgParam + 8), "XYZ");
					pHeroRef = this->pHero;
					pHeroRef->rotationQuat.x = eStack64.ca;
					pHeroRef->rotationQuat.y = eStack64.cb;
					pHeroRef->rotationQuat.z = eStack64.cc;
					pHeroRef->rotationQuat.w = eStack64.cd;

					/* WARNING: Load size is inaccurate */
					if (((*pMsgParam != 0) && (pfVar10 = *(float**)((int)pMsgParam + 4), pfVar10 != (float*)0x0)) &&
						(0.0 < *(float*)((int)pMsgParam + 0xc))) {
						local_50.x = *pfVar10;
						local_50.y = pfVar10[1];
						local_50.z = pfVar10[2];
						local_50.w = 1.0;
						/* WARNING: Load size is inaccurate */
						edF32Vector4SubHard(&local_50, &local_50, *pMsgParam);
						FUN_002003d0(*(undefined4*)((int)pMsgParam + 0xc), &(this->pHero->base).field_0xf70, 0x40eb90, &local_50, 1);
						(*((this->pHero->base).character.characterBase.base.base.pVTable)->SetState)
							((CActor*)this->pHero, 0x116, 0xffffffff);
						return 1;
					})
				}
				else {
					if (msg == 0x2a) {
						IMPLEMENTATION_GUARD(
						if ((int)pMsgParam == 1) {
							(*((this->pHero->base).character.characterBase.base.base.pVTable)->SetState)
								((CActor*)this->pHero, 0x73, 0xffffffff);
						})
						return 1;
					}
					if (msg == 0x29) {
						IMPLEMENTATION_GUARD(
						if ((int)pMsgParam == 1) {
							(*((this->pHero->base).character.characterBase.base.base.pVTable)->SetState)
								((CActor*)this->pHero, 0xdf, 0xffffffff);
						})
						return 1;
					}

					if (msg == 0x5d) {
						IMPLEMENTATION_GUARD(
						pHeroRef = this->pHero;
						pHeroRef->flags = pHeroRef->flags & 0xfffffffd;
						pHeroRef->flags = pHeroRef->flags | 1;
						pHeroRef = this->pHero;
						pHeroRef->flags = pHeroRef->flags & 0xffffff7f;
						pHeroRef->flags = pHeroRef->flags | 0x20;
						CActor::EvaluateDisplayState((CActor*)pHeroRef);
						(*((this->pHero->base).character.characterBase.base.base.pVTable)->SetState)
							((CActor*)this->pHero, 0x72, 0xffffffff);
						pCollisionRef = (this->pHero->base).character.characterBase.base.base.pCollisionData;
						pCollisionRef->flags_0x0 = pCollisionRef->flags_0x0 & 0xfff7efff;
						pHeroRef = this->pHero;
						if (pHeroRef != (CActorHeroPrivate*)CActorHero::_gThis) {
							pPlayerInput = (*(pHeroRef->pVTable)->GetInputManager)
								((CActor*)pHeroRef, 0, 0);
							pPlayerInput->playerId = 0;
						}
						return 1;)
					}

					if (msg == 0x5c) {
						IMPLEMENTATION_GUARD(
						pHeroRef = this->pHero;
						pHeroRef->flags = pHeroRef->flags | 2;
						pHeroRef->flags = pHeroRef->flags & 0xfffffffe;
						pHeroRef = this->pHero;
						pHeroRef->flags = pHeroRef->flags | 0x80;
						pHeroRef->flags = pHeroRef->flags & 0xffffffdf;
						CActor::EvaluateDisplayState((CActor*)pHeroRef);
						(*((this->pHero->base).character.characterBase.base.base.pVTable)->SetState)
							((CActor*)this->pHero, 0x73, 0xffffffff);
						pCollisionRef = (this->pHero->base).character.characterBase.base.base.pCollisionData;
						pCollisionRef->flags_0x0 = pCollisionRef->flags_0x0 | 0x81000;
						pHeroRef = this->pHero;
						if (pHeroRef != (CActorHeroPrivate*)CActorHero::_gThis) {
							pPlayerInput = (*(pHeroRef->pVTable)->GetInputManager)
								((CActor*)pHeroRef, 0, 0);
							pPlayerInput->playerId = 1;
						}
						return 1;)
					}

					if (msg == 0x32) {
						IMPLEMENTATION_GUARD(
						if (pMsgParam != (void*)0x0) {
							pCollisionRef = (this->pHero->base).character.characterBase.base.base.pCollisionData;
							pCollisionRef->flags_0x0 = pCollisionRef->flags_0x0 | 0x81000;
						}
						pHeroRef = this->pHero;
						pHeroRef->flags = pHeroRef->flags | 0x80;
						pHeroRef->flags = pHeroRef->flags & 0xffffffdf;
						CActor::EvaluateDisplayState((CActor*)pHeroRef);
						return 1;)
					}

					if (msg == MESSAGE_TRAP_CAUGHT) {
						if (pMsgParam != (void*)0x0) {
							pCollisionRef = this->pHero->pCollisionData;
							pCollisionRef->flags_0x0 = pCollisionRef->flags_0x0 & 0xfff7efff;
						}

						pHeroRef = this->pHero;
						pHeroRef->flags = pHeroRef->flags & 0xffffff7f;
						pHeroRef->flags = pHeroRef->flags | 0x20;
						pHeroRef->EvaluateDisplayState();

						return 1;
					}

					if (msg == 0x24) {
						IMPLEMENTATION_GUARD(
						(*((this->pHero->base).character.characterBase.base.base.pVTable)->SetState)
							((CActor*)this->pHero, 0x73, 0xffffffff);
						return 1;)
					}

					if (msg == 0x23) {
						IMPLEMENTATION_GUARD(
						/* WARNING: Load size is inaccurate */
						if (*pMsgParam != 0) {
							pHeroRef = this->pHero;
							pHeroRef->flags =
								pHeroRef->flags & 0xffffff7f;
							pHeroRef->flags =
								pHeroRef->flags | 0x20;
							CActor::EvaluateDisplayState((CActor*)pHeroRef);
						}
						*(undefined4*)&(this->pHero->base).field_0x1614 = *(undefined4*)((int)pMsgParam + 4);
						*(CActor**)&(this->pHero->base).field_0xf14 = pSender;
						if (*(int*)((int)pMsgParam + 8) == 0) {
							pCollisionRef = (this->pHero->base).character.characterBase.base.base.pCollisionData;
							pCollisionRef->flags_0x0 = pCollisionRef->flags_0x0 & 0xfffffffd;
						}
						else {
							pCollisionRef = (this->pHero->base).character.characterBase.base.base.pCollisionData;
							pCollisionRef->flags_0x0 = pCollisionRef->flags_0x0 | 2;
						}
						(this->pHero->base).field_0x1610 = *(int*)((int)pMsgParam + 0xc);
						(*((this->pHero->base).character.characterBase.base.base.pVTable)->SetState)
							((CActor*)this->pHero, 0x72, 0xffffffff);
						return 1;)
					}
				}

				iVar7 = 0;
			}
		}
	}

	return iVar7;
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
