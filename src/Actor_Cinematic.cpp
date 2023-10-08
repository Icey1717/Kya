#include "Actor_Cinematic.h"


CActorCinematic::CActorCinematic()
{

}

void CActorCinematic::Create(const edCinGameInterface::ACTORV_CREATIONtag* pGameInterface, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D, ed_3D_Scene* pScene)
{
	//ed_3d_hierarchy_node* peVar1;
	//edNODE* peVar2;
	//MeshData_OBB* pMVar3;
	int value;
	//float fVar4;
	//float fVar5;
	//
	//memset(&this->namedObjSectionStart, 0, 0x30);
	//this->pFileData = &this->namedObjSectionStart;
	//(this->namedObjSectionStart).meshIndex = -1;
	//(this->namedObjSectionStart).textureIndex = -1;
	//(this->namedObjSectionStart).collisionDataIndex = -1;
	//(this->namedObjSectionStart).floatFieldA = 1.0;
	//(this->namedObjSectionStart).floatFieldB = 1.0;
	//(this->namedObjSectionStart).floatFieldC = 1.0;
	//memset(&this->otherSectionStart, 0, 0x50);
	//this->subObjA = (KyaUpdateObjA*)&this->otherSectionStart;
	//(this->otherSectionStart).firstField = -1;
	//fVar5 = pGameInterface->field_0x44;
	//fVar4 = pGameInterface->field_0x48;
	//(this->otherSectionStart).pointerA = (int)pGameInterface->field_0x40;
	//(this->otherSectionStart).pointerB = (int)fVar5;
	//(this->otherSectionStart).pointerC = (int)fVar4;
	//(this->otherSectionStart).floatFieldA = pGameInterface->field_0x30 * 10.0;
	//(this->otherSectionStart).floatFieldB = 1000.0;
	//(this->otherSectionStart).floatFieldC = 1000.0;
	//(this->otherSectionStart).floatFieldD = 1000.0;
	//(this->otherSectionStart).floatFieldE = 100.0;
	//(this->otherSectionStart).intFieldA = 1;
	//(this->otherSectionStart).floatFieldF = 1e+10;
	//(this->otherSectionStart).FloatFieldG = 1e+10;
	//(this->otherSectionStart).intFieldB = 1;
	//(this->otherSectionStart).floatFieldH = 1.0;
	//this->index_0x20 = -1;
	//this->typeID = 1;
	for (value = 0; (value < 0x1f && (pGameInterface->name[value] != '\0')); value = value + 1) {
		/* Copy the name into the cine object */
		this->name[value] = pGameInterface->name[value];
	}
	/* Add a null term on */
	this->name[value] = '\0';
	//if (pG3D != (ed_g3d_manager*)0x0) {
	//	value = FUN_0029f130(pG3D, 0);
	//	this->field_0x8c = value;
	//	value = this->field_0x8c;
	//	if (value != 0) {
	//		if (1 < *(ushort*)(value + 0x9c)) {
	//			*(ushort*)(value + 0x9e) = *(ushort*)(value + 0x9e) | 0x100;
	//		}
	//		FUN_0029f350(this->field_0x8c, 0xffff);
	//	}
	//	peVar2 = ed3DHierarchyAddToScene(pScene, pG3D, (char*)0x0);
	//	this->pMeshTransformParent = peVar2;
	//	peVar2 = this->pMeshTransformParent;
	//	if (peVar2 != (edNODE*)0x0) {
	//		this->pMeshTransformData = (ed_3d_hierarchy_node*)peVar2->pData;
	//		SetMeshTransformFlag_002abd80(this->pMeshTransformParent, 0xffff);
	//		value = GetParam1_0x18((int)g_CinematicManager_0048efc);
	//		if ((*(uint*)(value + 4) & 0x400000) == 0) {
	//			ClearMeshTransformFlag_002abff0(this->pMeshTransformParent, 0);
	//		}
	//		else {
	//			ClearMeshTransformFlag_002abff0(this->pMeshTransformParent, 1);
	//		}
	//		memset(&this->field_0x9c, 0, 0x20);
	//		if (1 < ((this->pMeshTransformData)->base).count_0x9c) {
	//			this->field_0x9c.field_0xc = (char*)&this->field_0xcc;
	//		}
	//		ed3DHierarchySetSetup((ed_3d_hierarchy*)this->pMeshTransformData, &this->field_0x9c);
	//		FUN_0011c1b0((int)this, (int)pG3D, (int)pG2D);
	//	}
	//}
	//peVar1 = this->pMeshTransformData;
	//if (peVar1 == (ed_3d_hierarchy_node*)0x0) {
	//	this->pMeshTransform = (ed_3d_hierarchy_node*)&this->field_0x580;
	//}
	//else {
	//	this->pMeshTransform = peVar1;
	//}
	//this->field_0x110 = (undefined*)0x0;
	this->components.count = 1;
	this->components.aComponents[0].id = 1;
	this->components.aComponents[0].pComponent = &behaviourCinematic;
	this->aComponents = (void*)&this->components;
	//CActor::SetupDefaultPosition((CActor*)this);
	//CActor::SetupClippingInfo(this);
	//CActor::SetupLodInfo(this);
	//CActor::SetupShadow((CActor*)this, (GroundObject*)0x0);
	//CActor::SetupLighting(this);
	//this->flags = this->flags | 0x100000;
	//this->flags = this->flags | 0x1000;
	//CActor::RestoreInitData((CActor*)this);
	//peVar2 = this->pMeshTransformParent;
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
