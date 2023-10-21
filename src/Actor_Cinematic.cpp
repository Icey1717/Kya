#include "Actor_Cinematic.h"
#include "ed3D.h"
#include "edList.h"


CActorCinematic::CActorCinematic()
{

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
			IMPLEMENTATION_GUARD_LOG(
			FUN_0011c1b0((int)this, (int)pG3D, (int)pG2D));
		}
	}
	peVar1 = this->p3DHierNode;
	if (peVar1 == (ed_3d_hierarchy_node*)0x0) {
		IMPLEMENTATION_GUARD_LOG(
		this->pMeshTransform = (ed_3d_hierarchy_node*)&this->field_0x580;)
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
	//this->flags = this->flags | 0x100000;
	//this->flags = this->flags | 0x1000;
	//CActor::RestoreInitData((CActor*)this);
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
