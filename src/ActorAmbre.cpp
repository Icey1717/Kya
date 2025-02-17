#include "ActorAmbre.h"
#include "MemoryStream.h"
#include "CinematicManager.h"
#include "ActorHero.h"
#include "LargeObject.h"
#include "FileManager3D.h"
#include "MathOps.h"
#include "TimeController.h"
#include "LevelScheduleManager.h"

StaticMeshComponent::StaticMeshComponent()
{
	this->pMeshTransformParent = (edNODE*)0x0;
	this->pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	this->meshIndex = -1;
	this->textureIndex = -1;
}

void StaticMeshComponent::Reset()
{
	this->pMeshTransformParent = (edNODE*)0x0;
	this->pMeshTransformData = (ed_3d_hierarchy_node*)0x0;

	return;
}

void StaticMeshComponent::SetHidden(ed_3D_Scene* pScene)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if ((this->pMeshTransformParent != (edNODE*)0x0) && (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0)) {
		ed3DHierarchyNodeSetRenderOff(pScene, this->pMeshTransformParent);
	}

	return;
}

void StaticMeshComponent::SetVisible(ed_3D_Scene* pScene)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if ((this->pMeshTransformParent != (edNODE*)0x0) && (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0)) {
		ed3DHierarchyNodeSetRenderOn(pScene, this->pMeshTransformParent);
	}

	return;
}

bool StaticMeshComponent::HasMesh()
{
	return this->pMeshTransformParent != (edNODE*)0x0;
}

void StaticMeshComponent::Init(ed_3D_Scene* pScene, ed_g3d_manager* pMeshManager, ed_3d_hierarchy_setup* pHierarchySetup, char* szString)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if (pMeshManager == (ed_g3d_manager*)0x0) {
		pMeshManager = CScene::ptable.g_C3DFileManager_00451664->GetG3DManager(this->meshIndex, this->textureIndex);
	}

	this->pMeshTransformParent = ed3DHierarchyAddToScene(pScene, pMeshManager, szString);

	if (this->pMeshTransformParent != (edNODE*)0x0) {
		this->pMeshTransformData = reinterpret_cast<ed_3d_hierarchy_node*>(this->pMeshTransformParent->pData);

		if (pHierarchySetup != (ed_3d_hierarchy_setup*)0x0) {
			ed3DHierarchySetSetup((ed_3d_hierarchy*)this->pMeshTransformData, pHierarchySetup);
		}

		if (this->pMeshTransformData != (ed_3d_hierarchy_node*)0x0) {
			edF32Matrix4CopyHard(&this->perspectiveMatrix, &this->pMeshTransformData->base.transformA);
		}

		SetHidden((ed_3D_Scene*)0x0);
	}

	return;
}

void StaticMeshComponent::Term(ed_3D_Scene* pScene)
{
	if (pScene == (ed_3D_Scene*)0x0) {
		pScene = CScene::_scene_handleA;
	}

	if (this->pMeshTransformParent != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(pScene, this->pMeshTransformParent);
	}

	this->pMeshTransformParent = (edNODE*)0x0;

	return;
}

StateConfig CActorAmbre::_gStateCfg_AMB[6]
{
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x0),
};

CActorAmbre::CActorAmbre()
{
	//this->field_0x1d4 = 0;
	//this->field_0x1d8 = (undefined*)0x0;
}

void CActorAmbre::Create(ByteCode* pByteCode)
{
	S_STREAM_NTF_TARGET_SWITCH* pSVar1;
	CActor* pCVar2;
	S_STREAM_EVENT_CAMERA* pSVar3;
	int materialId;
	undefined* puVar4;
	int iVar5;
	CFxSparkNoAlloc<4, 12>* pFxSpark;

	CActor::Create(pByteCode);
	materialId = pByteCode->GetS32();
	/*(this->effectsStructC).field_0x8 = */pByteCode->GetS32();

	(this->staticMesh).textureIndex = pByteCode->GetS32();
	(this->staticMesh).meshIndex = pByteCode->GetS32();

	staticMesh.Reset();

	this->field_0x1d0 = pByteCode->GetS32();

	S_TARGET_STREAM_REF* pTargetStreamRef = (S_TARGET_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pTargetStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
	}
	this->pMagicalSwitch1CBase_0x1224 = pTargetStreamRef;

	pSVar3 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSVar3 + 1);
	this->field_0x1228 = pSVar3;

	CActor::SV_InstallMaterialId(materialId);

	iVar5 = 0;
	pFxSpark = this->aFxSparks;
	do {
		pFxSpark->Create(4, 0x10, pFxSpark->aVectorData, pFxSpark->aFloatData, materialId);
		pFxSpark->field_0xe4 = pFxSpark->aUnknown;
		pFxSpark->SetParameters(0.35f, 0.05f, 30.0f, 0.1f, 2.0f, 1);
		pFxSpark->field_0x90 = 0x800060c0;
		pFxSpark->field_0x94 = 0x802020c0;
		iVar5 = iVar5 + 1;
		pFxSpark = pFxSpark + 1;
	} while (iVar5 < 3);

	this->aFxSparkProps[0].field_0x4 = 0.0f;
	this->aFxSparkProps[1].field_0x4 = 0.2f;
	this->aFxSparkProps[2].field_0x4 = -0.2f;

	this->aFxSparkProps[0].field_0x8 = 0.2f;
	this->aFxSparkProps[1].field_0x8 = 0.05f;
	this->aFxSparkProps[2].field_0x8 = 0.05f;

	return;
}

void CActorAmbre::Init()
{
	ed_3d_hierarchy_node* peVar1;
	bool bVar2;
	CActorHero* pReceiver;
	int iVar3;
	S_TARGET_STREAM_REF* pSVar4;
	AmberSparkProps* pCurProps;
	CFxSparkNoAlloc<4, 12>* pFxSpark;
	CActorAmbre* iVar6;
	int iVar7;
	float fVar8;

	bVar2 = (this->staticMesh).textureIndex != -1;
	if (bVar2) {
		bVar2 = (this->staticMesh).meshIndex != -1;
	}
	if (bVar2) {
		this->staticMesh.Init((ed_3D_Scene*)0x0, (ed_g3d_manager*)0x0, (ed_3d_hierarchy_setup*)0x0, (char*)0x0);

		peVar1 = (this->staticMesh).pMeshTransformData;
		if (peVar1 != (ed_3d_hierarchy_node*)0x0) {
			(((this->staticMesh).pMeshTransformData)->base).transformA.rowT = this->currentLocation;
		}
	}

	CActor::Init();

	this->aFxSparkProps[0].boneId = DoMessage(CActorHero::_gThis, MESSAGE_GET_BONE_ID, (MSG_PARAM)5);
	this->aFxSparkProps[1].boneId = DoMessage(CActorHero::_gThis, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xa);
	this->aFxSparkProps[2].boneId = DoMessage(CActorHero::_gThis, MESSAGE_GET_BONE_ID, (MSG_PARAM)0xd);

	iVar3 = 0;
	pCurProps = this->aFxSparkProps;
	pFxSpark = this->aFxSparks;
	do {
		pFxSpark->Init(this->objectId);

		iVar3 = iVar3 + 1;
		pFxSpark->vector_0x80.x = pCurProps->field_0x4;
		pFxSpark->vector_0x80.y = pCurProps->field_0x8;
		pFxSpark->vector_0x80.z = 0.0f;
		pFxSpark->vector_0x80.w = 0.0f;

		pCurProps = pCurProps + 1;
		pFxSpark = pFxSpark + 1;
	} while (iVar3 < 3);

	for (int i = 0; i < this->pMagicalSwitch1CBase_0x1224->entryCount; i++) {
		this->pMagicalSwitch1CBase_0x1224->aEntries[i].Init();
	}

	this->field_0x1228->Init();

	for (int i = 0; i < this->pMagicalSwitch1CBase_0x1224->entryCount; i++) {
		this->pMagicalSwitch1CBase_0x1224->aEntries[i].Reset();
	}

	this->field_0x1228->Reset(this);

	return;
}

void CActorAmbre::Term()
{
	bool bVar1;

	CActor::Term();

	bVar1 = (this->staticMesh).textureIndex != -1;
	if (bVar1) {
		bVar1 = (this->staticMesh).meshIndex != -1;
	}

	if (bVar1) {
		this->staticMesh.Term((ed_3D_Scene*)0x0);
	}

	return;
}

void CActorAmbre::Draw()
{
	IMPLEMENTATION_GUARD_LOG();
}

CBehaviour* CActorAmbre::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
		pBehaviour = new CBehaviourAmbreUnknown();
	}
	else {
		if (behaviourType == 2) {
			pBehaviour = &this->behaviourAmber;
		}
		else {
			pBehaviour = CActor::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

StateConfig* CActorAmbre::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 6);
		pStateConfig = _gStateCfg_AMB + state + -5;
	}

	return pStateConfig;
}

void CActorAmbre::ChangeManageState(int state)
{
	undefined* puVar1;
	int iVar2;
	int* piVar3;
	bool bVar4;

	CActor::ChangeManageState(state);

	bVar4 = (this->staticMesh).textureIndex != -1;

	if (bVar4) {
		bVar4 = (this->staticMesh).meshIndex != -1;
	}
	if (bVar4) {
		if (state == 0) {
			this->staticMesh.SetVisible((ed_3D_Scene*)0x0);
		}
		else {
			this->staticMesh.SetHidden((ed_3D_Scene*)0x0);
		}
	}

	IMPLEMENTATION_GUARD_FX(
	if ((this->effectsStructC).field_0x8 != (undefined*)0xffffffff) {
		if ((state == 0) || (this->actorState == 5)) {
			piVar3 = (int*)(this->effectsStructC).field_0x4;
			if ((piVar3 == (int*)0x0) ||
				((iVar2 = (this->effectsStructC).field_0x0, iVar2 == 0 || (bVar4 = true, iVar2 != piVar3[6])))) {
				bVar4 = false;
			}
			if (bVar4) {
				if (((piVar3 != (int*)0x0) && (iVar2 = (this->effectsStructC).field_0x0, iVar2 != 0)) && (iVar2 == piVar3[6]))
				{
					(**(code**)(*piVar3 + 0xc))();
				}
				(this->effectsStructC).field_0x4 = (undefined*)0x0;
				(this->effectsStructC).field_0x0 = 0;
			}
			(this->effectsStructC).field_0x0 = 0;
			(this->effectsStructC).field_0x4 = (undefined*)0x0;
		}
		else {
			puVar1 = (this->effectsStructC).field_0x4;
			if (((puVar1 == (undefined*)0x0) || (iVar2 = (this->effectsStructC).field_0x0, iVar2 == 0)) ||
				(iVar2 != *(int*)(puVar1 + 0x18))) {
				bVar4 = false;
			}
			else {
				bVar4 = true;
			}
			if (!bVar4) {
				CFxHandle::SV_FX_Start(&this->effectsStructC);
			}
			piVar3 = (int*)(this->effectsStructC).field_0x4;
			if (((piVar3 == (int*)0x0) || (iVar2 = (this->effectsStructC).field_0x0, iVar2 == 0)) ||
				(bVar4 = true, iVar2 != piVar3[6])) {
				bVar4 = false;
			}
			if (bVar4) {
				if (((piVar3 != (int*)0x0) && (iVar2 = (this->effectsStructC).field_0x0, iVar2 != 0)) && (iVar2 == piVar3[6]))
				{
					(**(code**)(*piVar3 + 0x38))(piVar3, 6, this);
				}
				piVar3 = (int*)(this->effectsStructC).field_0x4;
				if (((piVar3 != (int*)0x0) && (iVar2 = (this->effectsStructC).field_0x0, iVar2 != 0)) && (iVar2 == piVar3[6]))
				{
					(**(code**)(*piVar3 + 0x10))(0, 0);
				}
			}
		}
	}
	if (this->field_0x1d0 != 0xffffffff) {
		if ((state == 0) || (this->actorState != 5)) {
			piVar3 = (int*)this->field_0x1d8;
			if ((piVar3 == (int*)0x0) || ((this->field_0x1d4 == 0 || (bVar4 = true, this->field_0x1d4 != piVar3[6])))) {
				bVar4 = false;
			}
			if (bVar4) {
				if (((piVar3 != (int*)0x0) && (this->field_0x1d4 != 0)) && (this->field_0x1d4 == piVar3[6])) {
					(**(code**)(*piVar3 + 0xc))();
				}
				this->field_0x1d8 = (undefined*)0x0;
				this->field_0x1d4 = 0;
			}
			this->field_0x1d4 = 0;
			this->field_0x1d8 = (undefined*)0x0;
		}
		else {
			if (((this->field_0x1d8 == (undefined*)0x0) || (this->field_0x1d4 == 0)) ||
				(this->field_0x1d4 != *(int*)(this->field_0x1d8 + 0x18))) {
				bVar4 = false;
			}
			else {
				bVar4 = true;
			}
			if (!bVar4) {
				CParticlesManager::GetDynamicFx
				(CScene::ptable.g_EffectsManager_004516b8, &this->field_0x1d4, this->field_0x1d0, 0xffffffffffffffff);
			}
			piVar3 = (int*)this->field_0x1d8;
			if (((piVar3 == (int*)0x0) || (this->field_0x1d4 == 0)) || (bVar4 = true, this->field_0x1d4 != piVar3[6])) {
				bVar4 = false;
			}
			if (bVar4) {
				if (((piVar3 != (int*)0x0) && (this->field_0x1d4 != 0)) && (this->field_0x1d4 == piVar3[6])) {
					(**(code**)(*piVar3 + 0x38))(piVar3, 6, this, 0);
				}
				piVar3 = (int*)this->field_0x1d8;
				if (((piVar3 != (int*)0x0) && (this->field_0x1d4 != 0)) && (this->field_0x1d4 == piVar3[6])) {
					(**(code**)(*piVar3 + 0x10))(0, 0);
				}
			}
		}
	})

	return;
}

void CActorAmbre::BehaviourStand_Manage(CBehaviourAmbre* pBehaviour)
{
	undefined* puVar1;
	int* piVar2;
	float fVar3;
	undefined8 uVar4;
	bool bVar5;
	CActorHero* pCVar6;
	CLifeInterface* pCVar7;
	int iVar8;
	Timer* pTVar9;
	int iVar10;
	long lVar11;
	edF32VECTOR4* v1;
	edF32VECTOR4* v2;
	float fVar12;
	float fVar13;
	edF32VECTOR4 someVector;
	undefined4 local_30;
	float local_2c;
	float fStack40;
	float fStack36;
	edF32VECTOR4 local_20;
	undefined8 local_10;
	undefined4 local_8;
	undefined8* local_4;

	IMPLEMENTATION_GUARD_FX(
	pCVar6 = CActorHero::_gThis;
	puVar1 = (this->effectsStructC).field_0x4;
	if (((puVar1 == (undefined*)0x0) || (iVar8 = (this->effectsStructC).field_0x0, iVar8 == 0)) ||
		(iVar8 != *(int*)(puVar1 + 0x18))) {
		bVar5 = false;
	}
	else {
		bVar5 = true;
	}
	if (bVar5) {
		v1 = &(CActorHero::_gThis->character).characterBase.base.base.currentLocation;
		v2 = &this->currentLocation;
		local_20.x = (float)gF32Vertex4Zero._0_8_;
		local_20.y = (float)((ulong)gF32Vertex4Zero._0_8_ >> 0x20);
		local_20.z = gF32Vertex4Zero.z;
		local_20.w = gF32Vertex4Zero.w;
		fVar12 = v1->x - v2->x;
		fVar13 = (CActorHero::_gThis->character).characterBase.base.base.currentLocation.y - this->currentLocation.y;
		fVar3 = (CActorHero::_gThis->character).characterBase.base.base.currentLocation.z - this->currentLocation.z;
		if (fVar12 * fVar12 + fVar13 * fVar13 + fVar3 * fVar3 <= 25.0) {
			edF32Vector4SubHard(&local_20, v1, v2);
			local_20.w = 1.0;
		}
		SV_FX_UpdateEffectorPosition(&this->effectsStructC, s_effector_target_0042b9d0, &local_20);
	})

	if (this->actorState != 5) {
		return;
	}

	IMPLEMENTATION_GUARD(
	if (pCVar6 != (CActorHero*)0x0) {
		pCVar7 = (*((pCVar6->character).characterBase.base.base.pVTable)->GetLifeInterface)((CActor*)pCVar6);
		fVar12 = CLifeInterface::GetValueMax(pCVar7);
		pCVar7 = (*((pCVar6->character).characterBase.base.base.pVTable)->GetLifeInterface)((CActor*)pCVar6);
		fVar13 = (float)(**(code**)((int)pCVar7->pVtable + 0x24))(pCVar7);
		if ((fVar13 < fVar12) &&
			((lVar11 = (*(code*)(((pBehaviour->field_0x0).pVTable)->actorBaseVTable).field_0x4c)(pBehaviour), lVar11 != 0 ||
				(fVar12 = CActorHero::GetMagicalForce(pCVar6), 0.0 < fVar12)))) {
			bVar5 = true;
			goto LAB_001ce600;
		}
	}
	bVar5 = false;)
	LAB_001ce600:
	IMPLEMENTATION_GUARD(
	if (bVar5) {
		S_STREAM_EVENT_CAMERA::Manage(this->field_0x1228, (CActor*)this);
		pCVar7 = (*((pCVar6->character).characterBase.base.base.pVTable)->GetLifeInterface)((CActor*)pCVar6);
		fVar12 = CLifeInterface::GetValueMax(pCVar7);
		iVar8 = CLevelScheduler::ScenVar_Get(0x15);
		pTVar9 = GetTimer();
		fVar12 = (fVar12 / (float)iVar8) * pTVar9->cutsceneDeltaTime * 10.0;
		lVar11 = (*(code*)(((pBehaviour->field_0x0).pVTable)->actorBaseVTable).field_0x4c)(pBehaviour);
		if (lVar11 == 0) {
			CActorHero::MagicDecrease(fVar12 / 5.0, pCVar6);
		}
		(*(code*)((pCVar6->character).characterBase.base.base.pVTable)->LifeIncrease)(fVar12, pCVar6);
		pTVar9 = GetTimer();
		FLOAT_00448f00 = FLOAT_00448f00 - pTVar9->cutsceneDeltaTime;
		if (FLOAT_00448f00 <= 0.0) {
			local_4 = &local_10;
			local_10 = _DAT_0040ec80;
			local_8 = _DAT_0040ec88;
			CActor::DoMessage((CActor*)this, (CActor*)pCVar6, 0x6c, (uint)local_4);
			pCVar7 = (*((pCVar6->character).characterBase.base.base.pVTable)->GetLifeInterface)((CActor*)pCVar6);
			fVar12 = CLifeInterface::GetValueMax(pCVar7);
			pCVar7 = (*((pCVar6->character).characterBase.base.base.pVTable)->GetLifeInterface)((CActor*)pCVar6);
			fVar13 = (float)(**(code**)((int)pCVar7->pVtable + 0x24))(pCVar7);
			FLOAT_00448f00 = (fVar13 / fVar12) * -0.7 + 1.0;
		}
	}
	else {
		(*(this->pVTable)->SetState)((CActor*)this, 0, -1);
		if (((this->field_0x1d0 != -1) && (piVar2 = (int*)this->field_0x1d8, piVar2 != (int*)0x0)) &&
			((this->field_0x1d4 != 0 && (this->field_0x1d4 == piVar2[6])))) {
			(**(code**)(*piVar2 + 0x24))(&DAT_bf800000);
		}
		if ((this->effectsStructC).field_0x8 != (undefined*)0xffffffff) {
			puVar1 = (this->effectsStructC).field_0x4;
			if (((puVar1 == (undefined*)0x0) || (iVar8 = (this->effectsStructC).field_0x0, iVar8 == 0)) ||
				(iVar8 != *(int*)(puVar1 + 0x18))) {
				bVar5 = false;
			}
			else {
				bVar5 = true;
			}
			if (bVar5) {
				piVar2 = (int*)(this->effectsStructC).field_0x4;
				if (((piVar2 != (int*)0x0) && (iVar8 = (this->effectsStructC).field_0x0, iVar8 != 0)) && (iVar8 == piVar2[6]))
				{
					(**(code**)(*piVar2 + 0xc))();
				}
				(this->effectsStructC).field_0x4 = (undefined*)0x0;
				(this->effectsStructC).field_0x0 = 0;
			}
			(this->effectsStructC).field_0x0 = 0;
			(this->effectsStructC).field_0x4 = (undefined*)0x0;
			CFxHandle::SV_FX_Start(&this->effectsStructC);
			puVar1 = (this->effectsStructC).field_0x4;
			if (((puVar1 == (undefined*)0x0) || (iVar8 = (this->effectsStructC).field_0x0, iVar8 == 0)) ||
				(iVar8 != *(int*)(puVar1 + 0x18))) {
				bVar5 = false;
			}
			else {
				bVar5 = true;
			}
			if (bVar5) {
				piVar2 = (int*)(this->effectsStructC).field_0x4;
				if (((piVar2 != (int*)0x0) && (iVar8 = (this->effectsStructC).field_0x0, iVar8 != 0)) && (iVar8 == piVar2[6]))
				{
					(**(code**)(*piVar2 + 0x38))(piVar2, 6, this, 0);
				}
				piVar2 = (int*)(this->effectsStructC).field_0x4;
				if (((piVar2 != (int*)0x0) && (iVar8 = (this->effectsStructC).field_0x0, iVar8 != 0)) && (iVar8 == piVar2[6]))
				{
					(**(code**)(*piVar2 + 0x10))(0, 0);
				}
			}
		}
		bVar5 = false;
		if (((this->staticMesh).textureIndex != -1) && ((this->staticMesh).meshIndex != -1)) {
			bVar5 = true;
		}
		if (bVar5) {
			(*((this->staticMesh).pVTable)->SetHidden)(&this->staticMesh, (ed_3D_Scene*)0x0);
		}
	})

	IMPLEMENTATION_GUARD(
	pCVar6 = CActorHero::_gThis;
	for (iVar8 = 0; iVar8 < 3; iVar8 = iVar8 + 1) {
		//uVar4 = *(undefined8*)&this->currentLocation;
		//fStack40 = this->currentLocation.z;
		//fStack36 = this->currentLocation.w;
		//local_30 = (undefined4)uVar4;
		//local_2c = (float)((ulong)uVar4 >> 0x20);
		//iVar10 = rand();
		//local_2c = local_2c + ((float)iVar10 / 2.147484e+09) * 0.6 + 0.2;
		//CActor::SV_GetBoneWorldPosition((CActor*)pCVar6, *(int*)(&this->field_0x1200 + iVar8 * 0xc), &someVector);
		//CFxSpark::Manage(((int)(this->aFxSparks + iVar8), &local_30, (undefined4*)&someVector);
	})

	return;
}

void CBehaviourAmbre::Init(CActor* pOwner)
{
	this->pOwner = reinterpret_cast<CActorAmbre*>(pOwner);

	return;
}

void CBehaviourAmbre::Manage()
{
	this->pOwner->BehaviourStand_Manage(this);

	return;
}

void CBehaviourAmbre::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorAmbre* pAmber;
	bool bVar2;

	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	pAmber = this->pOwner;

	bVar2 = (pAmber->staticMesh).textureIndex != -1;
	if (bVar2) {
		bVar2 = (pAmber->staticMesh).meshIndex != -1;
	}

	if (bVar2) {
		pAmber->staticMesh.SetHidden((ed_3D_Scene*)0x0);
	}

	return;
}

int CBehaviourAmbre::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorAmbre* pCVar1;
	int* piVar2;
	bool bVar3;
	CLifeInterface* pCVar4;
	int iVar5;
	long lVar6;
	int iVar7;
	float fVar8;
	float fVar9;

	if (msg == 0x10) {
		IMPLEMENTATION_GUARD(
		(*((this->pOwner->base).pVTable)->SetState)((CActor*)this->pOwner, 0, -1);
		pCVar1 = this->pOwner;
		if ((((pCVar1->field_0x1d0 != -1) && (piVar2 = (int*)pCVar1->field_0x1d8, piVar2 != (int*)0x0)) &&
			(pCVar1->field_0x1d4 != 0)) && (pCVar1->field_0x1d4 == piVar2[6])) {
			(**(code**)(*piVar2 + 0x24))(&DAT_bf800000);
		}
		if ((pCVar1->effectsStructC).field_0x8 != (undefined*)0xffffffff) {
			piVar2 = (int*)(pCVar1->effectsStructC).field_0x4;
			if (((piVar2 == (int*)0x0) || (iVar5 = (pCVar1->effectsStructC).field_0x0, iVar5 == 0)) ||
				(bVar3 = true, iVar5 != piVar2[6])) {
				bVar3 = false;
			}
			if (bVar3) {
				if (((piVar2 != (int*)0x0) && (iVar5 = (pCVar1->effectsStructC).field_0x0, iVar5 != 0)) && (iVar5 == piVar2[6])
					) {
					(**(code**)(*piVar2 + 0xc))();
				}
				(pCVar1->effectsStructC).field_0x4 = (undefined*)0x0;
				(pCVar1->effectsStructC).field_0x0 = 0;
			}
			(pCVar1->effectsStructC).field_0x0 = 0;
			(pCVar1->effectsStructC).field_0x4 = (undefined*)0x0;
			CFxHandle::SV_FX_Start(&pCVar1->effectsStructC);
			piVar2 = (int*)(pCVar1->effectsStructC).field_0x4;
			if (((piVar2 == (int*)0x0) || (iVar5 = (pCVar1->effectsStructC).field_0x0, iVar5 == 0)) ||
				(bVar3 = true, iVar5 != piVar2[6])) {
				bVar3 = false;
			}
			if (bVar3) {
				if (((piVar2 != (int*)0x0) && (iVar5 = (pCVar1->effectsStructC).field_0x0, iVar5 != 0)) && (iVar5 == piVar2[6])
					) {
					(**(code**)(*piVar2 + 0x38))(piVar2, 6, pCVar1, 0);
				}
				piVar2 = (int*)(pCVar1->effectsStructC).field_0x4;
				if (((piVar2 != (int*)0x0) && (iVar5 = (pCVar1->effectsStructC).field_0x0, iVar5 != 0)) && (iVar5 == piVar2[6])
					) {
					(**(code**)(*piVar2 + 0x10))(0, 0);
				}
			}
		}
		bVar3 = (pCVar1->staticMesh).textureIndex != -1;
		if (bVar3) {
			bVar3 = (pCVar1->staticMesh).meshIndex != -1;
		}
		if (bVar3) {
			(*((pCVar1->staticMesh).pVTable)->setVisible)(&pCVar1->staticMesh, (ed_3D_Scene*)0x0);
		})
	}
	else {
		if (msg == 0x30) {
			IMPLEMENTATION_GUARD(
			(*((this->pOwner->base).pVTable)->SetState)((CActor*)this->pOwner, 5, -1);
			pCVar1 = this->pOwner;
			iVar5 = 0;
			if (0 < pCVar1->pMagicalSwitch1CBase_0x1224->entryCount) {
				iVar7 = 0;
				do {
					S_STREAM_NTF_TARGET_SWITCH::Switch
					((S_STREAM_NTF_TARGET_SWITCH*)
						((int)&pCVar1->pMagicalSwitch1CBase_0x1224->aEntries[0].base.pRef + iVar7), (CActor*)pCVar1);
					iVar5 = iVar5 + 1;
					iVar7 = iVar7 + 0x1c;
				} while (iVar5 < pCVar1->pMagicalSwitch1CBase_0x1224->entryCount);
			}
			S_STREAM_EVENT_CAMERA::SwitchOn(pCVar1->field_0x1228, (CActor*)pCVar1);
			pCVar1 = this->pOwner;
			if (pCVar1->field_0x1d0 != -1) {
				piVar2 = (int*)pCVar1->field_0x1d8;
				if (((piVar2 == (int*)0x0) || (pCVar1->field_0x1d4 == 0)) || (bVar3 = true, pCVar1->field_0x1d4 != piVar2[6]))
				{
					bVar3 = false;
				}
				if (bVar3) {
					if (((piVar2 != (int*)0x0) && (pCVar1->field_0x1d4 != 0)) && (pCVar1->field_0x1d4 == piVar2[6])) {
						(**(code**)(*piVar2 + 0xc))();
					}
					pCVar1->field_0x1d8 = (undefined*)0x0;
					pCVar1->field_0x1d4 = 0;
				}
				pCVar1->field_0x1d4 = 0;
				pCVar1->field_0x1d8 = (undefined*)0x0;
				CParticlesManager::GetDynamicFx
				(CScene::ptable.g_EffectsManager_004516b8, &pCVar1->field_0x1d4, pCVar1->field_0x1d0, 0xffffffffffffffff)
					;
				piVar2 = (int*)pCVar1->field_0x1d8;
				if (((piVar2 == (int*)0x0) || (pCVar1->field_0x1d4 == 0)) || (bVar3 = true, pCVar1->field_0x1d4 != piVar2[6]))
				{
					bVar3 = false;
				}
				if (bVar3) {
					if (((piVar2 != (int*)0x0) && (pCVar1->field_0x1d4 != 0)) && (pCVar1->field_0x1d4 == piVar2[6])) {
						(**(code**)(*piVar2 + 0x38))(piVar2, 6, pCVar1, 0);
					}
					piVar2 = (int*)pCVar1->field_0x1d8;
					if (((piVar2 != (int*)0x0) && (pCVar1->field_0x1d4 != 0)) && (pCVar1->field_0x1d4 == piVar2[6])) {
						(**(code**)(*piVar2 + 0x10))(0, 0);
					}
				}
			}
			if ((((pCVar1->effectsStructC).field_0x8 != (undefined*)0xffffffff) &&
				(piVar2 = (int*)(pCVar1->effectsStructC).field_0x4, piVar2 != (int*)0x0)) &&
				((iVar5 = (pCVar1->effectsStructC).field_0x0, iVar5 != 0 && (iVar5 == piVar2[6])))) {
				(**(code**)(*piVar2 + 0x24))(&DAT_bf800000);
			}
			bVar3 = (pCVar1->staticMesh).textureIndex != -1;
			if (bVar3) {
				bVar3 = (pCVar1->staticMesh).meshIndex != -1;
			}
			if (bVar3) {
				(*((pCVar1->staticMesh).pVTable)->setHidden)(&pCVar1->staticMesh, (ed_3D_Scene*)0x0);
			})
			return 1;
		}
		if (msg == 0x2f) {
			CActorHero* pHero = reinterpret_cast<CActorHero*>(pSender);

			assert(pHero->typeID == ACTOR_HERO_PRIVATE);

			fVar8 = pHero->GetLifeInterface()->GetValueMax();
			fVar9 = pHero->GetLifeInterface()->GetValue();

			if (fVar8 <= fVar9) {
				return 0;
			}

			lVar6 = UnknownFunction();
			if ((lVar6 == 0) &&
				((iVar5 = CLevelScheduler::ScenVar_Get(0xc), iVar5 == 0 || (fVar8 = pHero->GetMagicalForce(), fVar8 <= 0.0f)))) {
				return 4;
			}

			return 1;
		}
	}

	return 0;
}
