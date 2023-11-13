#include "Actor_Cinematic.h"
#include "ed3D.h"
#include "edList.h"
#include <string>
#include "MathOps.h"
#include "FileManager3D.h"


CActorCinematic::CActorCinematic()
	:CActor()
{
	//CSound* pSound;
	//
	//CActor::CActor((CActor*)this);
	//this->pVTable = &_vt;
	//pSound = &(this->behaviourCinematic).cinActor.soundStruct;
	//(this->behaviourCinematic).pVtable = &CBehaviour::_vt;
	//(this->behaviourCinematic).pVtable = &CBehaviourCinematic::_vt;
	//(this->behaviourCinematic).cinActor.pVTable = &edCinActorInterface::_vt;
	//(this->behaviourCinematic).cinActor.pVTable = (undefined*)&CBWCinActor::_vt;
	//(this->behaviourCinematic).cinActor.pParent = (CActor*)0x0;
	//CSound::CSound(pSound);
	//pSound->vt = &CSoundSample::_vt;
	//*(undefined4*)&(this->behaviourCinematic).cinActor.soundInternalStruct = 0;
	//*(undefined4*)&(this->behaviourCinematic).cinActor.soundInternalStruct.field_0x4 = 0;
	//*(undefined4*)&(this->behaviourCinematic).cinActor.soundInternalStruct.field_0x8 = 0;
	//(this->behaviourCinematic).cinActor.soundInternalStruct.SoundStructPtr = (CSound*)0x0;
	//(this->behaviourCinematic).cinActor.soundInternalStruct.SoundID = 0;
	//(this->behaviourCinematic).cinActor.soundInternalStruct.field_0x14 = 0;
	//(this->behaviourCinematic).cinActor.soundInternalStruct.field_0x18 = 0.0;
	//(this->behaviourCinematic).cinActor.soundInternalStruct.field_0x1c = 0.0;
	//(this->behaviourCinematic).cinActor.usedInInstCreate = -1;
	//(this->behaviourCinematic).cinActor.field_0xbc = 0;
	//(this->behaviourCinematic).cinActor.field_0xc0 = 0;
	//GetParam1((long)&(this->behaviourCinematic).cinActor.field_0x110);
	//(this->behaviourCinematic).field_0x140 = -1;
	//(this->behaviourCinematic).field_0x144 = -1;
	//CAnimation::CAnimation(&this->animationController);
}

ed_Chunck* edChunckGetSpecial(ed_Chunck* pChunk, char* param_2)
{
	ed_Chunck* peVar1;

	peVar1 = (ed_Chunck*)((char*)pChunk + pChunk->field_0xc);
	if ((param_2 != (char*)0x0) && (param_2 <= (char*)peVar1)) {
		peVar1 = (ed_Chunck*)0x0;
	}
	return peVar1;
}

void CActorCinematic::FUN_0011c1b0(ed_g3d_manager* pG3D, ed_g2d_manager* pG2D)
{
	ed_Chunck* peVar1;
	long* plVar2;
	ed_Chunck* peVar3;
	long* plVar4;
	long* plVar5;
	uint uVar6;
	int iVar7;
	int iVar8;
	uint uVar9;
	long local_20[4];

	peVar3 = pG3D->MBNA;
	peVar3 = edChunckGetSpecial(peVar3, (char*)peVar3 + peVar3->size);
	uVar9 = peVar3->size - 0x10U >> 4;
	if (1 < uVar9) {
		IMPLEMENTATION_GUARD_LOG(
		plVar4 = (long*)0x18;
		plVar5 = local_20;
		plVar2 = plVar5;
		while (plVar2 != (long*)0x0) {
			*(undefined*)plVar5 = 0;
			plVar5 = (long*)((int)plVar5 + 1);
			plVar4 = (long*)((int)plVar4 + -1);
			plVar2 = plVar4;
		}
		iVar8 = 0;
		iVar7 = 0;
		plVar5 = local_20;
		local_20[0] = s_BOOMY_P0BOOMY_P1BOOMY_P2_00435140._0_8_;
		local_20[1] = s_FIGHT_01FIGHT_02FIGHT_03FIGHT_04_00435160._64_8_;
		local_20[2] = s_FIGHT_01FIGHT_02FIGHT_03FIGHT_04_00435160._0_8_;
		uVar6 = uVar9;
		peVar1 = peVar3;
		do {
			while (true) {
				if ((uVar6 == 0) || (2 < iVar8)) break;
				if (*(long*)(peVar1 + 1) == *plVar5) {
					plVar5 = plVar5 + 1;
					iVar8 = iVar8 + 1;
				}
				uVar6 = uVar6 - 1;
				peVar1 = peVar1 + 1;
			}
			iVar7 = iVar7 + 1;
			uVar6 = uVar9;
			peVar1 = peVar3;
		} while (iVar7 < 3);
		if (iVar8 == 3) {
			iVar7 = LevelScheduleManager::FUN_002d9e40();
			if (0 < iVar7) {
				FUN_00115890((int)this, s_BOOMY_P0BOOMY_P1BOOMY_P2_00435140._0_8_, *(ulong*)(&DAT_00435138 + iVar7 * 8), (int)pG2D
				);
			}
			uVar9 = LevelScheduleManager::FUN_002d9e50();
			if (0 < (int)uVar9) {
				FUN_00115890((int)this, s_FIGHT_01FIGHT_02FIGHT_03FIGHT_04_00435160._64_8_,
					*(ulong*)(s_FIGHT_01FIGHT_02FIGHT_03FIGHT_04_00435160 + uVar9 * 8 + 0x38), (int)pG2D);
			}
			iVar7 = LevelScheduleManager::FUN_002d9e00();
			if (0 < iVar7) {
				FUN_00115890((int)this, s_FIGHT_01FIGHT_02FIGHT_03FIGHT_04_00435160._0_8_,
					*(ulong*)(s_BOOMY_P0BOOMY_P1BOOMY_P2_00435140 + iVar7 * 8 + 0x18), (int)pG2D);
			}
		})
	}
	return;
}

void CActorCinematic::Create(const edCinGameInterface::ACTORV_CREATIONtag* pGameInterface, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D, ed_3D_Scene* pScene)
{
	ed_3d_hierarchy_node* peVar1;
	edNODE* peVar2;
	//MeshData_OBB* pMVar3;
	int value;
	float fVar4;
	float fVar5;
	
	memset(&this->namedObjSectionStart, 0, sizeof(CinNamedObject30));
	this->pCinData = &this->namedObjSectionStart;
	(this->namedObjSectionStart).meshIndex = -1;
	(this->namedObjSectionStart).textureIndex = -1;
	(this->namedObjSectionStart).collisionDataIndex = -1;
	(this->namedObjSectionStart).floatFieldA = 1.0f;
	(this->namedObjSectionStart).floatFieldB = 1.0f;
	(this->namedObjSectionStart).floatFieldC = 1.0f;
	memset(&this->otherSectionStart, 0, sizeof(KyaUpdateObjA));
	this->subObjA = (KyaUpdateObjA*)&this->otherSectionStart;
	(this->otherSectionStart).mainComponentID_0x0 = -1;
	(this->otherSectionStart).boundingSphere.xyz = pGameInterface->boundingSphere.xyz;
	(this->otherSectionStart).boundingSphere.w = pGameInterface->field_0x30 * 10.0f;
	(this->otherSectionStart).floatFieldB = 1000.0f;
	(this->otherSectionStart).floatField = 1000.0f;
	(this->otherSectionStart).field_0x1c = 1000.0f;
	(this->otherSectionStart).field_0x20 = 100.0f;
	(this->otherSectionStart).count_0x3c = 1;
	(this->otherSectionStart).field_0x2c = 1e+10f;
	(this->otherSectionStart).field_0x30 = 1e+10f;
	(this->otherSectionStart).flags_0x48 = 1;
	(this->otherSectionStart).field_0x4c = 1.0f;
	//this->index_0x20 = -1;
	this->typeID = 1;
	for (value = 0; (value < 0x1f && (pGameInterface->name[value] != '\0')); value = value + 1) {
		/* Copy the name into the cine object */
		this->name[value] = pGameInterface->name[value];
	}
	/* Add a null term on */
	this->name[value] = '\0';
	if (pG3D != (ed_g3d_manager*)0x0) {
		ed_g3d_hierarchy* pHier = ed3DG3DHierarchyGetFromIndex(pG3D, 0);
		this->pHier = pHier;
		pHier = this->pHier;
		if (pHier != (ed_g3d_hierarchy*)0x0) {
			if (1 < pHier->lodCount) {
				pHier->flags_0x9e = pHier->flags_0x9e | 0x100;
			}
			ed3DG3DHierarchySetStripShadowCastFlag(this->pHier, 0xffff);
		}
		peVar2 = ed3DHierarchyAddToScene(pScene, pG3D, (char*)0x0);
		this->pMeshNode = peVar2;
		peVar2 = this->pMeshNode;
		if (peVar2 != (edNODE*)0x0) {
			this->p3DHierNode = (ed_3d_hierarchy_node*)peVar2->pData;
			ed3DSetMeshTransformFlag_002abd80(this->pMeshNode, 0xffff);

			CCinematic* pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
			if ((pCinematic->flags_0x4 & 0x400000) == 0) {
				ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, 0);
			}
			else {
				ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, 1);
			}
			memset(&this->hierarchySetup, 0, sizeof(ed_3d_hierarchy_setup));
			if (1 < this->p3DHierNode->base.lodCount) {
				IMPLEMENTATION_GUARD(
				this->hierarchySetup.pNext = (ed_3d_hierarchy_setup*)&this->field_0xcc;)
			}
			ed3DHierarchySetSetup(&this->p3DHierNode->base, &this->hierarchySetup);
			this->FUN_0011c1b0(pG3D, pG2D);
		}
	}
	peVar1 = this->p3DHierNode;
	if (peVar1 == (ed_3d_hierarchy_node*)0x0) {
		this->pMeshTransform = &this->field_0x580;
	}
	else {
		this->pMeshTransform = peVar1;
	}
	//this->field_0x110 = (undefined*)0x0;
	this->components.count = 1;
	this->components.aComponents[0].id = 1;
	this->components.aComponents[0].pComponent = &behaviourCinematic;
	this->aComponents = (void*)&this->components;
	this->SetupDefaultPosition();
	this->SetupClippingInfo();
	//CActor::SetupLodInfo(this);
	//CActor::SetupShadow((CActor*)this, (GroundObject*)0x0);
	//CActor::SetupLighting(this);
	this->flags = this->flags | 0x100000;
	this->flags = this->flags | 0x1000;
	RestoreInitData();
	//peVar2 = this->pMeshNode;
	///* Doesn't go in here for tunnel */
	//if (peVar2 != (edNODE*)0x0) {
	//	pMVar3 = ed3DHierarchyNodeGetSkeletonChunck(peVar2, false);
	//	if (pMVar3 != (MeshData_OBB*)0x0) {
	//		CAnimation::Create(&this->animationController, (CActor*)this, 5, (AnimationSubObj*)&this->field_0x2f0, 2);
	//		this->pAnimationController = &this->animationController;
	//	}
	//}
	return;
}

void CActorCinematic::Init()
{
	CActor::Init();
}

void CActorCinematic::CinematicMode_Enter(bool bSetState)
{
	edNODE* pNode;
	ed_3D_Scene* pScene;

	this->CActor::CinematicMode_Enter(bSetState);
	pNode = this->pMeshNode;
	if (pNode != (edNODE*)0x0) {
		pScene = GetStaticMeshMasterA_001031b0();
		ed3DHierarchyNodeSetRenderOff(pScene, pNode);
	}
	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;
	this->flags = this->flags | 0x80;
	this->flags = this->flags & 0xffffffdf;
	this->EvaluateDisplayState();
	return;
}

void CBehaviourCinematic::Init(CActor* pOwner)
{
	CActorCinematic* pActor;
	//CAnimation* pAnimationController;
	//AnimationSubObj* pAVar1;
	bool bVar2;
	int physLayer;
	float fVar3;

	this->pOwner = (CActorCinematic*)pOwner;
	(this->cinActor).pParent = (CActor*)this->pOwner;
	//physLayer = CActor::GetIdMacroAnim((CActor*)this->pOwner, New_Name_(3));
	//if (physLayer != -1) {
	//	pActor = this->pOwner;
	//	physLayer = CActor::GetIdMacroAnim((CActor*)pActor, New_Name_(3));
	//	this->field_0x144 = physLayer;
	//	if (this->field_0x144 != -1) {
	//		this->field_0x14c = -1;
	//		this->field_0x148 = -1;
	//		this->field_0x160 = 0.0;
	//		this->field_0x150 = 0.0;
	//		this->field_0x154 = 0.0;
	//		this->field_0x158 = 0.0;
	//		this->field_0x15c = 1.0;
	//		if (this->field_0x144 != -1) {
	//			this->field_0x16c = 1.0;
	//			this->field_0x170 = 2.0;
	//			this->field_0x168 = 0.0;
	//			this->field_0x174 = 0.3;
	//			if ((this->field_0x144 != -1) && (this->field_0x148 != 0)) {
	//				this->field_0x14c = this->field_0x148;
	//				this->field_0x148 = 0;
	//				this->field_0x160 = 0.0;
	//				this->field_0x164 = 0.5;
	//				if (this->field_0x148 == -1) {
	//					fVar3 = 0.0;
	//				}
	//				else {
	//					fVar3 = 0.3;
	//					if (this->field_0x14c == -1) {
	//						this->field_0x154 = 0.0;
	//						fVar3 = 0.3;
	//					}
	//				}
	//				if (this->field_0x144 != -1) {
	//					this->field_0x150 = this->field_0x154;
	//					this->field_0x158 = fVar3;
	//					this->field_0x15c = 0.5;
	//				}
	//			}
	//		}
	//		pAnimationController = (pActor->baseData).pAnimationController;
	//		bVar2 = CAnimation::IsLayerActive(pAnimationController, 2);
	//		if (bVar2 == false) {
	//			this->field_0x144 = -1;
	//		}
	//		else {
	//			physLayer = CAnimation::PhysicalLayerFromLayerId(pAnimationController, 2);
	//			pAVar1 = pAnimationController->pAnimData;
	//			pAVar1[physLayer].field_0x0 = 3;
	//			pAVar1[physLayer].field_0x4 = 0.0;
	//		}
	//	}
	//}
	return;
}

bool CBehaviourCinematic::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorCinematic* pCVar1;
	bool bVar2;
	long lVar3;
	float fVar4;

	(this->pOwner)->flags = (this->pOwner)->flags | 0x1000;
	lVar3 = (long)*(int*)&(this->cinActor).field_0x124;
	if (lVar3 != 0) {
		IMPLEMENTATION_GUARD_LOG();
		//FUN_00115ba0((long)(int)this->pOwner, (undefined4*)&(this->cinActor).field_0x110, lVar3, 0);
	}
	if (newState == -1) {
		this->pOwner->SetState(3, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
	bVar2 = this->pOwner->IsKindOfObject(4);
	IMPLEMENTATION_GUARD_LOG(
	if (bVar2 != false) {
		pCVar1 = this->pOwner;
		*(undefined4*)&(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.field_0x8 = 0;
		(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.SoundStructPtr = (CSound*)0x0;
		(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.SoundID = 0;
		(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.field_0x14 = 0;
		(pCVar1->behaviourCinematic).cinActor.field_0xbc = 0;
	}
	this->field_0x178 = 0;
	bool bVar3 = this->field_0x144 != -1;
	if (bVar3) {
		if ((bVar3) && (this->field_0x148 != -1)) {
			this->field_0x14c = this->field_0x148;
			this->field_0x148 = -1;
			this->field_0x160 = 0.0f;
			this->field_0x164 = 0.5f;
			if (this->field_0x148 == -1) {
				fVar4 = 0.0;
			}
			else {
				fVar4 = 1.0;
				if (this->field_0x14c == -1) {
					this->field_0x154 = 0.0f;
					fVar4 = 1.0;
				}
			}
			if (this->field_0x144 != -1) {
				this->field_0x150 = this->field_0x154;
				this->field_0x158 = fVar4;
				this->field_0x15c = 0.5f;
			}
		}
		this->field_0x168 = -1.0f;
	})
	return bVar2;
}

void SetHierFlags_00295a30(ed_3d_hierarchy_node* pNode, byte param_2)
{
	(pNode->base).flags_0x9e = (pNode->base).flags_0x9e | 0x80;
	(pNode->base).size_0xae = param_2;
	return;
}

void CBehaviourCinematic::Manage()
{
	ed_3d_hierarchy_node* peVar1;
	CActorCinematic* pCVar2;
	EActorState AVar3;
	bool bVar4;
	CCinematic* pCinematic;
	CCineActorConfig* pConfig;
	long lVar5;
	float fVar6;
	float extraout_f0;
	CActor* pCVar7;
	float fVar8;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	pConfig = pCinematic->GetActorConfig(this->pOwner);
	local_10 = this->pOwner->currentLocation;
	fVar6 = local_10.w;
	peVar1 = this->pOwner->p3DHierNode;
	local_10.w = fVar6;
	if (peVar1 != (ed_3d_hierarchy_node*)0x0) {
		SetHierFlags_00295a30(peVar1, 0);
	}
	if ((pConfig != (CCineActorConfig*)0x0) && ((pConfig->flags & 2) != 0)) {
		IMPLEMENTATION_GUARD(
		fVar6 = (float)(*(code*)this->pOwner->pVTable->CinematicMode_Manage)();)
	}
	pCVar2 = this->pOwner;
	AVar3 = pCVar2->actorState;
	if (AVar3 == 3) {
		edF32Matrix4FromEulerSoft(&eStack80, &pCVar2->rotationEuler, "XYZ");
		eStack80.rowT = (this->cinActor).nextPos;

		if ((pConfig == (CCineActorConfig*)0x0) || ((pConfig->flags & 4) == 0)) {
			this->pOwner->UpdatePosition(&eStack80, 1);
		}
		else {
			pOwner->CinematicMode_UpdateMatrix(&eStack80);
		}
		pCVar2 = this->pOwner;
		pCVar2->previousLocation = pCVar2->currentLocation.xyz;
		//if ((this->field_0x178 == 3) || (this->field_0x178 == 2)) {
		//	ManageLipsync(this);
		//}
	}
	else {
		if (((AVar3 != 2) && (AVar3 == 1)))
		{
			// Guard hack check src.
			IMPLEMENTATION_GUARD(
			if (((AVar3 != 2) && (AVar3 == 1)) &&
				(lVar5 = (*(code*)pCVar2->pVTable->CinematicMode_InterpolateTo)
					(pCVar2, pConfig, &(this->cinActor).vectorFieldA, &pCinematic->matrix_0x120), fVar6 = extraout_f0
					, lVar5 != 0)) {
				fVar6 = (float)(*this->pOwner->pVTable->SetState)((CActor*)this->pOwner, 2, -1);
			})
		}
	}
	bVar4 = this->pOwner->IsKindOfObject(2);
	if (bVar4 != false) {
		IMPLEMENTATION_GUARD(
		pCVar2 = this->pOwner;
		CActorMoveable::ComputeRealMoving((ACharacterBase*)pCVar2, &local_10, fVar6);
		if ((((CharacterBaseData*)&pCVar2->baseData)->actorBase).actorState == New_Name_(3)) {
			fVar6 = *(float*)&(pCVar2->behaviourCinematic).cinActor.soundStruct.field_0x18;
			pCVar7 = (pCVar2->behaviourCinematic).cinActor.pParent;
			fVar8 = (float)(pCVar2->behaviourCinematic).cinActor.field_0x8;
			(((CharacterBaseData*)&pCVar2->baseData)->actorBase).vector_0x12c.x =
				(float)(pCVar2->behaviourCinematic).cinActor.pVTable * fVar6;
			(((CharacterBaseData*)&pCVar2->baseData)->actorBase).vector_0x12c.y = (float)pCVar7 * fVar6;
			(((CharacterBaseData*)&pCVar2->baseData)->actorBase).vector_0x12c.z = fVar8 * fVar6;
		})
	}
	return;
}

void CBehaviourCinematic::InitState(int newState)
{
	CCinematic* pCVar1;
	CBWCinActor* pCVar2;
	edF32MATRIX4 eStack64;

	if (this->pOwner->actorState == 3) {
		pCVar1 = g_CinematicManager_0048efc->GetCurCinematic();
		pCVar2 = &this->cinActor;
		pCVar2->SetPos((this->cinActor).position.x, (this->cinActor).position.y, (this->cinActor).position.z);
		pCVar2->SetScale((this->cinActor).scale.x, (this->cinActor).scale.y, (this->cinActor).scale.z);
		pCVar2->SetHeadingQuat((this->cinActor).heading.x, (this->cinActor).heading.y, (this->cinActor).heading.z, (this->cinActor).heading.w);
		edQuatToMatrix4Hard(&(this->cinActor).heading, &eStack64);
		eStack64.rowT = (this->cinActor).position;
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack64, &pCVar1->matrix_0x120);
		this->pOwner->CinematicMode_UpdateMatrix(&eStack64);
	}
	return;
}
