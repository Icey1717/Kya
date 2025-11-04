#include "Actor_Cinematic.h"
#include "ed3D.h"
#include "ed3D/ed3DG3D.h"
#include "edList.h"
#include <string>
#include "MathOps.h"
#include "FileManager3D.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "ActorFactory.h"
#include "ActorHero.h"
#include "LevelScheduleManager.h"


CActorCinematic::CActorCinematic()
{
	//CSound* pSound;
	//
	//CActor::CActor(this);
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
	(this->behaviourCinematic).field_0x144.macroAnimId = -1;
	this->animationController = CAnimation();
}

ed_Chunck* edChunckGetSpecial(ed_Chunck* pChunk, char* param_2)
{
	ed_Chunck* peVar1;

	peVar1 = (ed_Chunck*)((char*)pChunk + pChunk->nextChunckOffset);
	if ((param_2 != (char*)0x0) && (param_2 <= (char*)peVar1)) {
		peVar1 = (ed_Chunck*)0x0;
	}
	return peVar1;
}

void CActorCinematic::PatchMaterialForCutscene(ed_g3d_manager* pG3D, ed_g2d_manager* pG2D)
{
	ed_Chunck* peVar1;
	ulong* puVar2;
	ed_Chunck* peVar3;
	ulong* puVar4;
	ulong* puVar5;
	uint uVar6;
	int iVar7;
	int foundEquipmentHashes;
	uint uVar9;
	ulong local_20[4];

	peVar3 = pG3D->MBNA;
	peVar3 = edChunckGetSpecial(peVar3, (char*)peVar3 + peVar3->size);
	uVar9 = peVar3->size - 0x10U >> 4;
	if (1 < uVar9) {
		//puVar4 = (ulong*)0x18;
		//puVar5 = local_20;
		//puVar2 = puVar5;
		//while (puVar2 != (ulong*)0x0) {
		//	*(undefined*)puVar5 = 0;
		//	puVar5 = (ulong*)((int)puVar5 + 1);
		//	puVar4 = (ulong*)((int)puVar4 + -1);
		//	puVar2 = puVar4;
		//}

		foundEquipmentHashes = 0;
		iVar7 = 0;
		puVar5 = local_20;
		local_20[0] = gBoomyHashCodes[1];
		local_20[1] = gMedallionHashCodes[1];
		local_20[2] = gFightHashCodes[1];
		uVar6 = uVar9;
		peVar1 = peVar3;
		do {
			while (true) {
				if ((uVar6 == 0) || (2 < foundEquipmentHashes)) break;
				if (*(ulong*)(peVar1 + 1) == *puVar5) {
					puVar5 = puVar5 + 1;
					foundEquipmentHashes = foundEquipmentHashes + 1;
				}
				uVar6 = uVar6 - 1;
				peVar1 = peVar1 + 1;
			}
			iVar7 = iVar7 + 1;
			uVar6 = uVar9;
			peVar1 = peVar3;
		} while (iVar7 < 3);

		if (foundEquipmentHashes == 3) {
			iVar7 = CLevelScheduler::GetBoomyLevel();
			if (0 < iVar7) {
				this->SV_PatchMaterial(gBoomyHashCodes[1], gBoomyHashCodes[iVar7], pG2D);
			}

			uVar9 = CLevelScheduler::GetMedallionLevel();
			if (0 < (int)uVar9) {
				this->SV_PatchMaterial(gMedallionHashCodes[1], gMedallionHashCodes[uVar9], pG2D);
			}

			iVar7 = CLevelScheduler::GetFightLevel();
			if (0 < iVar7) {
				this->SV_PatchMaterial(gFightHashCodes[1], gFightHashCodes[iVar7], pG2D);
			}
		}
	}
	return;
}

void CActorCinematic::Create(const edCinGameInterface::ACTORV_CREATIONtag* pGameInterface, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D, ed_3D_Scene* pScene)
{
	ed_3d_hierarchy_node* peVar1;
	edNODE* peVar2;
	ed_Chunck* pMVar3;
	int value;
	float fVar4;
	float fVar5;

	ACTOR_LOG(LogLevel::Info, "CActorCinematic::Create");
	
	memset(&this->namedObjSectionStart, 0, sizeof(CinNamedObject30));
	this->pCinData = &this->namedObjSectionStart;
	(this->namedObjSectionStart).meshIndex = -1;
	(this->namedObjSectionStart).textureIndex = -1;
	(this->namedObjSectionStart).collisionDataIndex = -1;
	(this->namedObjSectionStart).scale.x = 1.0f;
	(this->namedObjSectionStart).scale.y = 1.0f;
	(this->namedObjSectionStart).scale.z = 1.0f;

	memset(&this->otherSectionStart, 0, sizeof(KyaUpdateObjA));
	this->subObjA = (KyaUpdateObjA*)&this->otherSectionStart;
	(this->otherSectionStart).defaultBehaviourId = -1;
	(this->otherSectionStart).boundingSphere.xyz = pGameInterface->boundingSphere.xyz;
	(this->otherSectionStart).boundingSphere.w = pGameInterface->field_0x30 * 10.0f;
	(this->otherSectionStart).floatFieldB = 1000.0f;
	(this->otherSectionStart).visibilityDistance = 1000.0f;
	(this->otherSectionStart).field_0x1c = 1000.0f;
	(this->otherSectionStart).field_0x20 = 100.0f;
	(this->otherSectionStart).animLayerCount = 1;
	(this->otherSectionStart).lodBiases[0] = 1e+10f;
	(this->otherSectionStart).lodBiases[1] = 1e+10f;
	(this->otherSectionStart).flags_0x48 = 1;
	(this->otherSectionStart).lightingFloat_0x4c = 1.0f;

	this->actorManagerIndex = -1;
	this->typeID = CINEMATIC;

	for (value = 0; (value < 0x1f && (pGameInterface->name[value] != '\0')); value = value + 1) {
		/* Copy the name into the cine object */
		this->cinematicName[value] = pGameInterface->name[value];
	}

	/* Add a null term on */
	this->cinematicName[value] = '\0';

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
				this->hierarchySetup.pLodBiases = this->lodBiases;
			}

			ed3DHierarchySetSetup(&this->p3DHierNode->base, &this->hierarchySetup);
			this->PatchMaterialForCutscene(pG3D, pG2D);
		}
	}

	peVar1 = this->p3DHierNode;
	if (peVar1 == (ed_3d_hierarchy_node*)0x0) {
		this->pMeshTransform = &this->field_0x580;
	}
	else {
		this->pMeshTransform = peVar1;
	}

	this->field_0x110 = (undefined*)0x0;
	this->components.count = 1;
	this->components.aComponents[0].id = 1;
	this->components.aComponents[0].SetBehaviour(&behaviourCinematic);
	this->aComponents = (void*)&this->components;
	this->SetupDefaultPosition();
	this->SetupClippingInfo();
	this->SetupLodInfo();
	//CActor::SetupShadow(this, (GroundObject*)0x0);
	this->SetupLighting();

	this->flags = this->flags | 0x100000;
	this->flags = this->flags | 0x1000;

	RestoreInitData();

	peVar2 = this->pMeshNode;

	if (peVar2 != (edNODE*)0x0) {
		pMVar3 = ed3DHierarchyNodeGetSkeletonChunck(peVar2, false);
		if (pMVar3 != (ed_Chunck*)0x0) {
			this->animationController.Create(this, 5, this->anmLayers, 2);
			this->pAnimationController = &this->animationController;
		}
	}

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

void CActorCinematic::CinematicMode_Leave(int behaviourId)
{
	edNODE* pNode;
	ed_3D_Scene* pScene;

	CActor::CinematicMode_Leave(behaviourId);

	pNode = this->pMeshNode;
	if (pNode != (edNODE*)0x0) {
		pScene = GetStaticMeshMasterA_001031b0();
		ed3DHierarchyNodeSetRenderOn(pScene, pNode);
	}

	return;
}

void CBehaviourCinematic::Create(ByteCode* pByteCode)
{
	int iVar1;

	iVar1 = pByteCode->GetS32();
	this->leaveCinematicBehaviourId = iVar1;
	if (this->leaveCinematicBehaviourId == 1) {
		this->leaveCinematicBehaviourId = -1;
	}
	return;
}

void CBehaviourCinematic::Init(CActor* pOwner)
{
	CActorCinematic* pActor;
	CAnimation* pAnimationController;
	edAnmLayer* pAVar1;
	bool bVar2;
	int physLayer;
	float fVar3;

	this->pOwner = (CActorCinematic*)pOwner;
	(this->cinActor).pParent = this->pOwner;

	physLayer = this->pOwner->GetIdMacroAnim(0x3);
	if (physLayer != -1) {
		pActor = this->pOwner;
		physLayer = this->pOwner->GetIdMacroAnim(0x3);
		this->field_0x144.macroAnimId = physLayer;

		if ((this->field_0x144).macroAnimId != -1) {
			(this->field_0x144).field_0x8 = -1;
			(this->field_0x144).field_0x4 = -1;
			(this->field_0x144).field_0x1c = 0.0f;
			(this->field_0x144).field_0xc = 0.0f;
			(this->field_0x144).field_0x10 = 0.0f;
			(this->field_0x144).field_0x14 = 0.0f;
			(this->field_0x144).field_0x18 = 1.0f;

			if ((this->field_0x144).macroAnimId != -1) {
				(this->field_0x144).field_0x28 = 1.0f;
				(this->field_0x144).field_0x2c = 2.0f;
				(this->field_0x144).field_0x24 = 0.0f;
				(this->field_0x144).field_0x30 = 0.3f;

				if (((this->field_0x144).macroAnimId != -1) && (physLayer = (this->field_0x144).field_0x4, physLayer != 0)) {
					(this->field_0x144).field_0x8 = physLayer;
					(this->field_0x144).field_0x4 = 0;
					(this->field_0x144).field_0x1c = 0.0f;
					(this->field_0x144).field_0x20 = 0.5f;

					if ((this->field_0x144).field_0x4 == -1) {
						fVar3 = 0.0f;
					}
					else {
						fVar3 = 0.3f;

						if ((this->field_0x144).field_0x8 == -1) {
							(this->field_0x144).field_0x10 = 0.0f;
							fVar3 = 0.3f;
						}
					}

					if ((this->field_0x144).macroAnimId != -1) {
						(this->field_0x144).field_0xc = (this->field_0x144).field_0x10;
						(this->field_0x144).field_0x14 = fVar3;
						(this->field_0x144).field_0x18 = 0.5f;
					}
				}
			}

			pAnimationController = pActor->pAnimationController;
			if (pAnimationController->IsLayerActive(2) == false) {
				this->field_0x144.macroAnimId = -1;
			}
			else {
				physLayer = pAnimationController->PhysicalLayerFromLayerId(2);
				pAVar1 = pAnimationController->anmBinMetaAnimator.aAnimData + physLayer;
				pAVar1[physLayer].blendOp = 3;
				pAVar1[physLayer].field_0x4 = 0.0f;
			}
		}
	}
	return;
}

void CBehaviourCinematic::PauseChange(int bPaused)
{
	bool bVar1;
	long lVar2;
	IMPLEMENTATION_GUARD_AUDIO(
	SoundStructAInternal* soundInternal;

	soundInternal = &(this->cinActor).soundInternalStruct;
	bVar1 = edSoundInstanceIsAlive(soundInternal);
	if (bVar1 != false) {
		if (bPaused == 0) {
			if (((NoAudio == 0) && (bVar1 = edSoundInstanceIsAlive(soundInternal), bVar1 != false)) &&
				(lVar2 = (long)(int)(this->cinActor).soundInternalStruct.SoundStructPtr, lVar2 != 0)) {
				FUN_001884d0(lVar2, (this->cinActor).soundInternalStruct.SoundID, 0);
			}
		}
		else {
			if (((NoAudio == 0) && (bVar1 = edSoundInstanceIsAlive(soundInternal), bVar1 != false)) &&
				(lVar2 = (long)(int)(this->cinActor).soundInternalStruct.SoundStructPtr, lVar2 != 0)) {
				FUN_001884d0(lVar2, (this->cinActor).soundInternalStruct.SoundID, 1);
			}
		}
	})
	return;
}

void CBehaviourCinematic::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorCinematic* pCVar1;
	bool bVar2;
	long lVar3;
	float fVar4;

	ed_g3d_manager* peVar1;

	(this->pOwner)->flags = (this->pOwner)->flags | 0x1000;
	peVar1 = (this->cinActor).pAltModelManager;
	if (peVar1 != (ed_g3d_manager*)0x0) {
		IMPLEMENTATION_GUARD();
		//CActor::SV_SwitchToModel((CActor *)this->pOwner,&(this->cinActor).alternateModel,peVar1,(edF32VECTOR4 *)0x0);
	}
	if (newState == -1) {
		this->pOwner->SetState(3, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	bVar2 = this->pOwner->IsKindOfObject(OBJ_TYPE_AUTONOMOUS);
	if (bVar2 != false) {
		pCVar1 = this->pOwner;
		IMPLEMENTATION_GUARD_AUDIO(
		*(undefined4*)&(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.field_0x8 = 0;
		(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.SoundStructPtr = (CSound*)0x0;
		(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.SoundID = 0;
		(pCVar1->behaviourCinematic).cinActor.soundInternalStruct.field_0x14 = 0;
		(pCVar1->behaviourCinematic).cinActor.field_0xbc = 0;)
	}

	this->field_0x178 = 0;

	bool bVar3 = this->field_0x144.macroAnimId != -1;
	if (bVar3) {
		if ((bVar3) && ((this->field_0x144).field_0x4 != -1)) {
			(this->field_0x144).field_0x8 = this->field_0x144.field_0x4;
			(this->field_0x144).field_0x4 = -1;
			(this->field_0x144).field_0x1c = 0.0f;
			(this->field_0x144).field_0x20 = 0.5f;

			if ((this->field_0x144).field_0x4 == -1) {
				fVar4 = 0.0f;
			}
			else {
				fVar4 = 1.0f;
				if ((this->field_0x144).field_0x8 == -1) {
					(this->field_0x144).field_0x10 = 0.0f;
					fVar4 = 1.0f;
				}
			}

			if ((this->field_0x144).macroAnimId != -1) {
				(this->field_0x144).field_0xc = (this->field_0x144).field_0x10;
				(this->field_0x144).field_0x14 = fVar4;
				(this->field_0x144).field_0x18 = 0.5f;
			}
		}

		(this->field_0x144).field_0x24 = -1.0f;
	}

	return;
}

void CBehaviourCinematic::End(int newBehaviourId)
{
	edNODE* pNode;
	CinNamedObject30* pCVar1;
	CActorCinematic* pCVar2;
	bool bVar3;
	ed_3D_Scene* pScene;
	CCinematic* pCinematic;
	CCineActorConfig* pActorConfig;
	Timer* pTVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR3 local_10;
	CAnimation* pAnimation;
	int iVar1;

	bVar3 = (this->field_0x144).macroAnimId != -1;
	if (bVar3) {
		if ((bVar3) && (iVar1 = (this->field_0x144).field_0x4, iVar1 != -1)) {
			(this->field_0x144).field_0x8 = iVar1;
			(this->field_0x144).field_0x4 = -1;
			(this->field_0x144).field_0x1c = 0.0f;
			(this->field_0x144).field_0x20 = 0.5f;

			if ((this->field_0x144).field_0x4 == -1) {
				fVar7 = 0.0f;
			}
			else {
				fVar7 = 1.0f;
				if ((this->field_0x144).field_0x8 == -1) {
					(this->field_0x144).field_0x10 = 0.0f;
					fVar7 = 1.0f;
				}
			}

			if ((this->field_0x144).macroAnimId != -1) {
				(this->field_0x144).field_0xc = (this->field_0x144).field_0x10;
				(this->field_0x144).field_0x14 = fVar7;
				(this->field_0x144).field_0x18 = 0.5f;
			}
		}

		(this->field_0x144).field_0x24 = -1.0f;
	}

	if ((this->cinActor).pAltModelManager != (ed_g3d_manager*)0x0) {
		IMPLEMENTATION_GUARD(
		FUN_001159f0(this->pOwner, &(this->cinActor).alternateModel);)
	}

	pNode = this->pOwner->pMeshNode;
	if ((pNode != (edNODE*)0x0) && ((this->pOwner->flags & 0x4000) != 0)) {
		pScene = GetStaticMeshMasterA_001031b0();
		ed3DHierarchyNodeSetRenderOff(pScene, pNode);
	}

	pAnimation = this->pOwner->pAnimationController;
	if (pAnimation != (CAnimation*)0x0) {
		pAnimation->Reset(this->pOwner);
	}

	this->pOwner->PlayAnim(-1);

	pCVar1 = this->pOwner->pCinData;
	this->pOwner->SetScaleVector((pCVar1->scale).x, (pCVar1->scale).y, (pCVar1->scale).z);

	pCVar2 = this->pOwner;
	if ((CActorFactory::gClassProperties[pCVar2->typeID].flags & 0x800) == 0) {
		pCVar2->flags = pCVar2->flags & 0xffffefff;
	}
	else {
		pCVar2->flags = pCVar2->flags | 0x1000;
	}

	if ((CActorFactory::gClassProperties[this->pOwner->typeID].flags & 2) != 0) {
		this->pOwner->SV_BuildAngleWithOnlyY(&local_10, &this->pOwner->rotationEuler.xyz);
		pCVar2 = this->pOwner;
		pCVar2->rotationEuler.xyz = local_10;
	}

	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	pActorConfig = pCinematic->GetActorConfig(this->pOwner);
	if ((pActorConfig == (CCineActorConfig*)0x0) || ((pActorConfig->flags & 0x10) == 0)) {
		SetVectorFromAngles(&this->pOwner->rotationQuat, &this->pOwner->rotationEuler.xyz);
		this->pOwner->UpdatePosition(&this->pOwner->currentLocation, true);
	}
	else {
		if ((pActorConfig->flags & 0x80000000) == 0) {
			this->pOwner->flags = this->pOwner->flags & 0xffffefff;
		}
		else {
			this->pOwner->flags = this->pOwner->flags | 0x1000;
		}

		pCVar2 = this->pOwner;
		pCVar2->rotationEuler.xyz = pActorConfig->postCinematicRotationEuler.xyz;
		SetVectorFromAngles(&this->pOwner->rotationQuat, &this->pOwner->rotationEuler.xyz);
		this->pOwner->UpdatePosition(&pActorConfig->postCinematicLocation, true);
	}

	bVar3 = this->pOwner->IsKindOfObject(2);
	if (bVar3 != false) {
		CActorMovable* pMoveable = reinterpret_cast<CActorMovable*>(this->pOwner);
		pMoveable->dynamic.Reset(pMoveable);

		bVar3 = this->pOwner->IsKindOfObject(OBJ_TYPE_AUTONOMOUS);
		if (bVar3 != false) {
			CActorAutonomous* pAutonomous = reinterpret_cast<CActorAutonomous*>(this->pOwner);
			pCVar2 = this->pOwner;
			pTVar5 = GetTimer();
			pAutonomous->dynamicExt.scaledTotalTime = pTVar5->scaledTotalTime;
		}
	}

	this->pOwner->flags = this->pOwner->flags & 0xffdfffff;

	this->pOwner->ComputeAltitude();
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

	// Force cinematic mesh to always use lod 0
	peVar1 = this->pOwner->p3DHierNode;
	if (peVar1 != (ed_3d_hierarchy_node*)0x0) {
		ed3DLockLOD(peVar1, 0);
	}

	if ((pConfig != (CCineActorConfig*)0x0) && ((pConfig->flags & 2) != 0)) {
		this->pOwner->CinematicMode_Manage();
	}

	pCVar2 = this->pOwner;

	AVar3 = pCVar2->actorState;
	if (AVar3 == 3) {
		edF32Matrix4FromEulerSoft(&eStack80, &pCVar2->rotationEuler.xyz, "XYZ");
		eStack80.rowT = (this->cinActor).nextPos;

		if ((pConfig == (CCineActorConfig*)0x0) || ((pConfig->flags & 4) == 0)) {
			this->pOwner->UpdatePosition(&eStack80, 1);
		}
		else {
			pOwner->CinematicMode_UpdateMatrix(&eStack80);
		}
		pCVar2 = this->pOwner;
		pCVar2->previousLocation = pCVar2->currentLocation.xyz;
		if ((this->field_0x178 == 3) || (this->field_0x178 == 2)) {
			IMPLEMENTATION_GUARD_LIP(
			ManageLipsync(this);)
		}
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
				this->pOwner->SetState(2, -1);
			})
		}
	}
	bVar4 = this->pOwner->IsKindOfObject(2);
	if (bVar4 != false) {
		CActorMovable* pMoveable = reinterpret_cast<CActorMovable*>(this->pOwner);
		pMoveable->ComputeRealMoving(&local_10);
		if (pMoveable->actorState == 0x3) {
			fVar6 = (pMoveable->dynamic).linearAcceleration;
			pMoveable->vector_0x120c = (pMoveable->dynamic).velocityDirectionEuler.xyz * fVar6;
		}
	}
	return;
}

// Should be in: D:/Projects/b-witch/Actor_Cinematic.h
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

bool CBehaviourCinematic::CinematicMode_InterpreteCinMessage(int param_2, int param_3)
{
	bool bVar1;
	int iVar2;
	bool bVar3;
	float fVar4;

	bVar3 = false;
	switch (param_2) {
	case 6:
		IMPLEMENTATION_GUARD(
		bVar3 = this->field_0x144 != -1;
		if (bVar3) {
			if ((bVar3) && (this->field_0x148 != -1)) {
				this->field_0x14c = this->field_0x148;
				this->field_0x148 = -1;
				this->field_0x160 = 0.0;
				this->field_0x164 = 0.5;
				if (this->field_0x148 == -1) {
					fVar4 = 0.0;
				}
				else {
					fVar4 = 1.0;
					if (this->field_0x14c == -1) {
						this->field_0x154 = 0.0;
						fVar4 = 1.0;
					}
				}
				if (this->field_0x144 != -1) {
					this->field_0x150 = this->field_0x154;
					this->field_0x158 = fVar4;
					this->field_0x15c = 0.5;
				}
			}
			this->field_0x168 = -1.0f;
		})
		bVar3 = true;
		break;
	case 7:
		IMPLEMENTATION_GUARD(
		this->field_0x168 = 0.0;)
		bVar3 = true;
		break;
	case 8:
		iVar2 = -1;
		switch (param_3) {
		case 0:
			break;
		case 1:
			iVar2 = 1;
			break;
		case 2:
			iVar2 = 2;
			break;
		case 3:
			iVar2 = 5;
			break;
		case 4:
			iVar2 = 3;
			break;
		case 5:
			iVar2 = 0;
			break;
		case 6:
			iVar2 = 4;
		}
		IMPLEMENTATION_GUARD(
		bVar1 = this->field_0x144 != -1;
		bVar3 = true;
		if (bVar1) {
			if ((bVar1) && (this->field_0x148 != -1)) {
				this->field_0x14c = this->field_0x148;
				this->field_0x148 = -1;
				this->field_0x160 = 0.0;
				this->field_0x164 = 0.5;
				if (this->field_0x148 == -1) {
					fVar4 = 0.0;
				}
				else {
					fVar4 = 1.0;
					if (this->field_0x14c == -1) {
						this->field_0x154 = 0.0;
						fVar4 = 1.0;
					}
				}
				if (this->field_0x144 != -1) {
					this->field_0x150 = this->field_0x154;
					this->field_0x158 = fVar4;
					this->field_0x15c = 0.5;
				}
			}
			this->field_0x168 = -1.0f;
		}
		if ((this->field_0x144 != -1) && (iVar2 != this->field_0x148)) {
			this->field_0x14c = this->field_0x148;
			this->field_0x148 = iVar2;
			this->field_0x160 = 0.0;
			this->field_0x164 = 0.5;
			if (this->field_0x148 == -1) {
				fVar4 = 0.0;
			}
			else {
				fVar4 = 1.0;
				if (this->field_0x14c == -1) {
					this->field_0x154 = 0.0;
					fVar4 = 1.0;
				}
			}
			if (this->field_0x144 != -1) {
				this->field_0x150 = this->field_0x154;
				this->field_0x158 = fVar4;
				this->field_0x15c = 0.5;
			}
		})
		break;
	case 0xb:
		this->pOwner->pAnimationController->PhysicalLayerFromLayerId(4);
		iVar2 = this->field_0x178;
		if (((iVar2 == 3) || (iVar2 == 2)) || ((iVar2 == 1 || (iVar2 == 0)))) {
			this->field_0x178 = 2;
		}
		break;
	case 0xc:
		this->pOwner->pAnimationController->PhysicalLayerFromLayerId(4);
		iVar2 = this->field_0x178;
		if ((((iVar2 == 3) || (iVar2 == 2)) || (iVar2 == 1)) || (iVar2 == 0)) {
			this->field_0x178 = 3;
		}
		break;
	case 0xd:
		IMPLEMENTATION_GUARD(
			CActor::DoMessage
		(this->pOwner, this->pOwner, (long)param_3, (GetPositionMsgParams*)0x0);
		bVar3 = true;)
		break;
	case 0xe:
		IMPLEMENTATION_GUARD(
		if (this->pOwner->typeID == 0x54) {
			FUN_00404a70((int*)this->pOwner, (long)param_3 & 0xff);
		})
		break;
	case 0xf:
	case 0x10:
	case 0x11:
		IMPLEMENTATION_GUARD(
		if (this->pOwner->typeID == ACTOR_HERO_PRIVATE) {
			if (param_2 == 0x11) {
				CLevelScheduler::ScenVar_Set(SCN_ABILITY_RECEPTACLE, 1);
			}
			else {
				if (param_2 == 0x10) {
					CLevelScheduler::ScenVar_Set(SCN_ABILITY_FIGHT, param_3);
				}
				else {
					if (param_2 == 0xf) {
						CLevelScheduler::ScenVar_Set(SCN_ABILITY_BOOMY_TYPE, param_3);
					}
				}
			}
			CLevelScheduler::ActorGlobalFunc_002db5b0();
		})
	}
	return bVar3;
}