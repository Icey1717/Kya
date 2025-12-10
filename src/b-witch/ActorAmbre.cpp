#include "ActorAmbre.h"
#include "MemoryStream.h"
#include "CinematicManager.h"
#include "ActorHero.h"
#include "LargeObject.h"
#include "FileManager3D.h"
#include "MathOps.h"
#include "TimeController.h"
#include "LevelScheduler.h"

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

void StaticMeshComponent::Term()
{
	this->pMeshTransformParent = (edNODE*)0x0;
	this->pMeshTransformData = (ed_3d_hierarchy_node*)0x0;
	this->meshIndex = -1;
	this->textureIndex = -1;

	return;
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
	this->effectsStructC.type = pByteCode->GetS32();

	(this->staticMesh).textureIndex = pByteCode->GetS32();
	(this->staticMesh).meshIndex = pByteCode->GetS32();

	staticMesh.Reset();

	this->field_0x1d0 = pByteCode->GetS32();

	this->ntfSwitch.Create(pByteCode);

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
	S_NTF_TARGET_STREAM_REF* pSVar4;
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
		pFxSpark->Init(this->sectorId);

		iVar3 = iVar3 + 1;
		pFxSpark->vector_0x80.x = pCurProps->field_0x4;
		pFxSpark->vector_0x80.y = pCurProps->field_0x8;
		pFxSpark->vector_0x80.z = 0.0f;
		pFxSpark->vector_0x80.w = 0.0f;

		pCurProps = pCurProps + 1;
		pFxSpark = pFxSpark + 1;
	} while (iVar3 < 3);

	this->ntfSwitch.Init();
	this->ntfSwitch.Reset(this);

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
	// Draw sparks.
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
	CNewFx* pFx;
	int iVar2;
	bool bVar3;

	CActor::ChangeManageState(state);

	bVar3 = (this->staticMesh).textureIndex != -1;
	if (bVar3) {
		bVar3 = (this->staticMesh).meshIndex != -1;
	}

	if (bVar3) {
		if (state == 0) {
			this->staticMesh.SetVisible((ed_3D_Scene*)0x0);
		}
		else {
			this->staticMesh.SetHidden((ed_3D_Scene*)0x0);
		}
	}

	if (this->effectsStructC.type != -1) {
		if ((state == 0) || (this->actorState == 5)) {
			pFx = this->effectsStructC.pFx;
			if ((pFx == (CNewFx*)0x0) || ((iVar2 = this->effectsStructC.id, iVar2 == 0 || (bVar3 = true, iVar2 != pFx->id)))) {
				bVar3 = false;
			}

			if (bVar3) {
				if (((pFx != (CNewFx*)0x0) && (iVar2 = this->effectsStructC.id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->Kill();
				}

				this->effectsStructC.pFx = (CNewFx*)0x0;
				this->effectsStructC.id = 0;
			}

			this->effectsStructC.id = 0;
			this->effectsStructC.pFx = (CNewFx*)0x0;
		}
		else {
			pFx = this->effectsStructC.pFx;

			if (((pFx == (CNewFx*)0x0) || (iVar2 = this->effectsStructC.id, iVar2 == 0)) || (iVar2 != pFx->id)) {
				bVar3 = false;
			}
			else {
				bVar3 = true;
			}

			if (!bVar3) {
				SV_FX_Start(&this->effectsStructC);
			}

			pFx = this->effectsStructC.pFx;
			if (((pFx == (CNewFx*)0x0) || (iVar2 = this->effectsStructC.id, iVar2 == 0)) ||
				(bVar3 = true, iVar2 != pFx->id)) {
				bVar3 = false;
			}

			if (bVar3) {
				if (((pFx != (CNewFx*)0x0) && (iVar2 = this->effectsStructC.id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->SpatializeOnActor(6, this, 0x0);
				}

				pFx = this->effectsStructC.pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar2 = this->effectsStructC.id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->Start(0, 0);
				}
			}
		}
	}
	if (this->field_0x1d0 != 0xffffffff) {
		if ((state == 0) || (this->actorState != 5)) {
			pFx = (this->effectsStructD).pFx;
			if ((pFx == (CNewFx*)0x0) ||
				((iVar2 = (this->effectsStructD).id, iVar2 == 0 || (bVar3 = true, iVar2 != pFx->id)))) {
				bVar3 = false;
			}

			if (bVar3) {
				if (((pFx != (CNewFx*)0x0) && (iVar2 = (this->effectsStructD).id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->Kill();
				}

				(this->effectsStructD).pFx = (CNewFx*)0x0;
				(this->effectsStructD).id = 0;
			}

			(this->effectsStructD).id = 0;
			(this->effectsStructD).pFx = (CNewFx*)0x0;
		}
		else {
			pFx = (this->effectsStructD).pFx;
			if (((pFx == (CNewFx*)0x0) || (iVar2 = (this->effectsStructD).id, iVar2 == 0)) || (iVar2 != pFx->id)) {
				bVar3 = false;
			}
			else {
				bVar3 = true;
			}
			if (!bVar3) {
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->effectsStructD, this->field_0x1d0, FX_MATERIAL_SELECTOR_NONE);
			}

			pFx = (this->effectsStructD).pFx;
			if (((pFx == (CNewFx*)0x0) || (iVar2 = (this->effectsStructD).id, iVar2 == 0)) ||
				(bVar3 = true, iVar2 != pFx->id)) {
				bVar3 = false;
			}
			if (bVar3) {
				if (((pFx != (CNewFx*)0x0) && (iVar2 = (this->effectsStructD).id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->SpatializeOnActor(6, this, 0);
				}

				pFx = (this->effectsStructD).pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar2 = (this->effectsStructD).id, iVar2 != 0)) && (iVar2 == pFx->id)) {
					pFx->Start(0, 0);
				}
			}
		}
	}

	return;
}

static float FLOAT_00448f00 = 0.0f;
static _msg_input_param gInputParam = { 0, 0.15f, 0.02f };

void CActorAmbre::BehaviourStand_Manage(CBehaviourAmbre* pBehaviour)
{
	CNewFx* puVar1;
	CNewFx* pFx;
	float fVar3;
	undefined8 uVar4;
	bool bVar5;
	CActorHero* pCVar6;
	CLifeInterface* pCVar7;
	int iVar8;
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

	pCVar6 = CActorHero::_gThis;
	puVar1 = this->effectsStructC.pFx;
	if (((puVar1 == (CNewFx*)0x0) || (iVar8 = this->effectsStructC.id, iVar8 == 0)) || (iVar8 != puVar1->id)) {
		bVar5 = false;
	}
	else {
		bVar5 = true;
	}

	if (bVar5) {
		v1 = &CActorHero::_gThis->currentLocation;
		v2 = &this->currentLocation;
		local_20 = gF32Vertex4Zero;
		fVar12 = v1->x - v2->x;
		fVar13 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
		fVar3 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
		if (fVar12 * fVar12 + fVar13 * fVar13 + fVar3 * fVar3 <= 25.0f) {
			edF32Vector4SubHard(&local_20, v1, v2);
			local_20.w = 1.0f;
		}

		SV_FX_UpdateEffectorPosition(&this->effectsStructC, "effector_target", &local_20);
	}

	if (this->actorState != 5) {
		return;
	}

	if (pCVar6 != (CActorHero*)0x0) {
		fVar12 = pCVar6->GetLifeInterface()->GetValueMax();
		fVar13 = pCVar6->GetLifeInterface()->GetValue();
		if ((fVar13 < fVar12) && ((pBehaviour->UnknownFunction() != 0 || (fVar12 = pCVar6->GetMagicalForce(), 0.0f < fVar12)))) {
			bVar5 = true;
			goto LAB_001ce600;
		}
	}

	bVar5 = false;

	LAB_001ce600:
	if (bVar5) {
		this->ntfSwitch.pStreamEventCamera->Manage(this);

		fVar12 = pCVar6->GetLifeInterface()->GetValueMax();
		iVar8 = CLevelScheduler::ScenVar_Get(SCN_LEVEL_LIFE_UPDATE);

		fVar12 = (fVar12 / (float)iVar8) * GetTimer()->cutsceneDeltaTime * 10.0f;
		if (pBehaviour->UnknownFunction() == false) {
			pCVar6->MagicDecrease(fVar12 / 5.0f);
		}

		pCVar6->LifeIncrease(fVar12);

		FLOAT_00448f00 = FLOAT_00448f00 - GetTimer()->cutsceneDeltaTime;

		if (FLOAT_00448f00 <= 0.0f) {
			_msg_input_param inputParam = gInputParam;
			DoMessage(pCVar6, (ACTOR_MESSAGE)0x6c, &inputParam);
			fVar12 = pCVar6->GetLifeInterface()->GetValueMax();
			fVar13 = pCVar6->GetLifeInterface()->GetValue();
			FLOAT_00448f00 = (fVar13 / fVar12) * -0.7f + 1.0f;
		}
	}
	else {
		SetState(0, -1);

		if (((this->field_0x1d0 != -1) && (pFx = this->effectsStructD.pFx, pFx != (CNewFx*)0x0)) && ((this->effectsStructD.id != 0 && (this->effectsStructD.id == pFx->id)))) {
			pFx->Stop(-1.0f);
		}

		if (this->effectsStructC.type != 0xffffffff) {
			puVar1 = this->effectsStructC.pFx;
			if (((puVar1 == (CNewFx*)0x0) || (iVar8 = this->effectsStructC.id, iVar8 == 0)) || (iVar8 != puVar1->id)) {
				bVar5 = false;
			}
			else {
				bVar5 = true;
			}

			if (bVar5) {
				pFx = this->effectsStructC.pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar8 = this->effectsStructC.id, iVar8 != 0)) && (iVar8 == pFx->id))
				{
					pFx->Kill();
				}

				this->effectsStructC.pFx = (CNewFx*)0x0;
				this->effectsStructC.id = 0;
			}
			this->effectsStructC.id = 0;
			this->effectsStructC.pFx = (CNewFx*)0x0;

			SV_FX_Start(&this->effectsStructC);

			puVar1 = this->effectsStructC.pFx;

			if (((puVar1 == (CNewFx*)0x0) || (iVar8 = this->effectsStructC.id, iVar8 == 0)) || (iVar8 != puVar1->id)) {
				bVar5 = false;
			}
			else {
				bVar5 = true;
			}

			if (bVar5) {
				pFx = this->effectsStructC.pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar8 = this->effectsStructC.id, iVar8 != 0)) && (iVar8 == pFx->id))
				{
					pFx->SpatializeOnActor(6, this, 0);
				}

				pFx = this->effectsStructC.pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar8 = this->effectsStructC.id, iVar8 != 0)) && (iVar8 == pFx->id))
				{
					pFx->Start(0.0f, 0.0f);
				}
			}
		}

		bVar5 = false;
		if (((this->staticMesh).textureIndex != -1) && ((this->staticMesh).meshIndex != -1)) {
			bVar5 = true;
		}

		if (bVar5) {
			this->staticMesh.SetHidden((ed_3D_Scene*)0x0);
		}
	}

	IMPLEMENTATION_GUARD_FX(
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
	CNewFx* pFx;
	bool bVar3;
	CLifeInterface* pCVar4;
	int iVar5;
	long lVar6;
	int iVar7;
	float fVar8;
	float fVar9;

	if (msg == 0x10) {

		this->pOwner->SetState(0, -1);

		pCVar1 = this->pOwner;
		if ((((pCVar1->field_0x1d0 != -1) && (pFx = pCVar1->effectsStructD.pFx, pFx != (CNewFx*)0x0)) &&
			(pCVar1->effectsStructD.id != 0)) && (pCVar1->effectsStructD.id == pFx->id)) {
			pFx->Stop(-1.0f);
		}

		if (pCVar1->effectsStructC.type != 0xffffffff) {
			pFx = pCVar1->effectsStructC.pFx;
			if (((pFx == (CNewFx*)0x0) || (iVar5 = pCVar1->effectsStructC.id, iVar5 == 0)) || (bVar3 = true, iVar5 != pFx->id)) {
				bVar3 = false;
			}

			if (bVar3) {
				if (((pFx != (CNewFx*)0x0) && (iVar5 = pCVar1->effectsStructC.id, iVar5 != 0)) && (iVar5 == pFx->id)) {
					pFx->Kill();
				}

				pCVar1->effectsStructC.pFx = (CNewFx*)0x0;
				pCVar1->effectsStructC.id = 0;
			}
			pCVar1->effectsStructC.id = 0;
			pCVar1->effectsStructC.pFx = (CNewFx*)0x0;

			SV_FX_Start(&pCVar1->effectsStructC);

			pFx = pCVar1->effectsStructC.pFx;
			if (((pFx == (CNewFx*)0x0) || (iVar5 = pCVar1->effectsStructC.id, iVar5 == 0)) || (bVar3 = true, iVar5 != pFx->id)) {
				bVar3 = false;
			}

			if (bVar3) {
				if (((pFx != (CNewFx*)0x0) && (iVar5 = pCVar1->effectsStructC.id, iVar5 != 0)) && (iVar5 == pFx->id)) {
					pFx->SpatializeOnActor(6, pCVar1, 0);
				}

				pFx = pCVar1->effectsStructC.pFx;
				if (((pFx != (CNewFx*)0x0) && (iVar5 = pCVar1->effectsStructC.id, iVar5 != 0)) && (iVar5 == pFx->id)) {
					pFx->Start(0.0f, 0.0f);
				}
			}
		}

		bVar3 = (pCVar1->staticMesh).textureIndex != -1;
		if (bVar3) {
			bVar3 = (pCVar1->staticMesh).meshIndex != -1;
		}

		if (bVar3) {
			pCVar1->staticMesh.SetVisible((ed_3D_Scene*)0x0);
		}
	}
	else {
		if (msg == MESSAGE_MAGIC_ACTIVATE) {
			this->pOwner->SetState(5, -1);

			pCVar1 = this->pOwner;
			pCVar1->ntfSwitch.Switch(pCVar1);

			pCVar1 = this->pOwner;
			if (pCVar1->field_0x1d0 != -1) {
				pFx = pCVar1->effectsStructD.pFx;
				if (((pFx == (CNewFx*)0x0) || (pCVar1->effectsStructD.id == 0)) || (bVar3 = true, pCVar1->effectsStructD.id != pFx->id))
				{
					bVar3 = false;
				}

				if (bVar3) {
					if (((pFx != (CNewFx*)0x0) && (pCVar1->effectsStructD.id != 0)) && (pCVar1->effectsStructD.id == pFx->id)) {
						pFx->Kill();
					}

					pCVar1->effectsStructD.pFx = (CNewFx*)0x0;
					pCVar1->effectsStructD.id = 0;
				}

				pCVar1->effectsStructD.id = 0;
				pCVar1->effectsStructD.pFx = (CNewFx*)0x0;
				CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&pCVar1->effectsStructD, pCVar1->field_0x1d0, FX_MATERIAL_SELECTOR_NONE);

				pFx = pCVar1->effectsStructD.pFx;
				if (((pFx == (CNewFx*)0x0) || (pCVar1->effectsStructD.id == 0)) || (bVar3 = true, pCVar1->effectsStructD.id != pFx->id))
				{
					bVar3 = false;
				}

				if (bVar3) {
					if (((pFx != (CNewFx*)0x0) && (pCVar1->effectsStructD.id != 0)) && (pCVar1->effectsStructD.id == pFx->id)) {
						pFx->SpatializeOnActor(6, pCVar1, 0);
					}

					pFx = pCVar1->effectsStructD.pFx;
					if (((pFx != (CNewFx*)0x0) && (pCVar1->effectsStructD.id != 0)) && (pCVar1->effectsStructD.id == pFx->id)) {
						pFx->Start(0.0f, 0.0f);
					}
				}
			}

			if (((pCVar1->effectsStructC.type != 0xffffffff) &&
				(pFx = pCVar1->effectsStructC.pFx, pFx != (CNewFx*)0x0)) &&
				((iVar5 = pCVar1->effectsStructC.id, iVar5 != 0 && (iVar5 == pFx->id)))) {
				pFx->Stop(-1.0f);
			}

			bVar3 = (pCVar1->staticMesh).textureIndex != -1;
			if (bVar3) {
				bVar3 = (pCVar1->staticMesh).meshIndex != -1;
			}

			if (bVar3) {
				pCVar1->staticMesh.SetHidden((ed_3D_Scene*)0x0);
			}

			return 1;
		}

		if (msg == MESSAGE_MAGIC_DEACTIVATE) {
			CActorHero* pHero = reinterpret_cast<CActorHero*>(pSender);

			assert(pHero->typeID == ACTOR_HERO_PRIVATE);

			fVar8 = pHero->GetLifeInterface()->GetValueMax();
			fVar9 = pHero->GetLifeInterface()->GetValue();

			if (fVar8 <= fVar9) {
				return 0;
			}

			lVar6 = UnknownFunction();
			if ((lVar6 == 0) &&
				((iVar5 = CLevelScheduler::ScenVar_Get(SCN_ABILITY_MAGIC_REGENERATE), iVar5 == 0 || (fVar8 = pHero->GetMagicalForce(), fVar8 <= 0.0f)))) {
				return 4;
			}

			return 1;
		}
	}

	return 0;
}
