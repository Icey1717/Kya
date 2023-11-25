#include "Actor.h"
#include "LargeObject.h"
#include "SectorManager.h"
#include "CinematicManager.h"
#include "ActorFactory.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include <string.h>
#include <math.h>
#include "TimeController.h"
#include "AnmManager.h"
#include "port/pointer_conv.h"

CActor::CActor()
	: CObject()
{
	float fVar1;
	float fVar2;
	this->objectId = -1;
	//this->field_0x134 = -1.0f;
	//this->field_0x13c = 0;
	//this->field_0x140 = 0;
	//this->field_0x144 = 0;
	//this->field_0x148 = 0;
	//this->funcData_0x150 = (undefined*)&CBehaviour::_vt;
	//this->funcData_0x150 = (undefined*)&CBehaviourStand::_vt;
	//this->index_0x20 = -1;
	this->objectId = -1;
	this->pCinData = (CinNamedObject30*)0x0;
	this->subObjA = (KyaUpdateObjA*)0x0;
	this->flags = 0;
	this->currentLocation.x = 0.0f;
	this->currentLocation.y = 0.0f;
	this->currentLocation.z = 0.0f;
	this->currentLocation.w = 1.0f;
	this->scale.x = 1.0;
	this->scale.y = 1.0;
	this->scale.z = 1.0;
	this->scale.w = 1.0;
	this->rotationEuler.x = 0.0;
	this->rotationEuler.y = 0.0;
	this->rotationEuler.z = 0.0;
	//*(undefined4*)&this->field_0x58 = 0;
	this->rotationQuat.x = 0.0;
	this->rotationQuat.y = 0.0;
	this->rotationQuat.z = 0.0;
	this->rotationQuat.w = 0.0;
	this->pMeshNode = (edNODE*)0x0;
	this->p3DHierNode = (ed_3d_hierarchy_node*)0x0;
	this->pHier = (ed_g3d_hierarchy*)0x0;
	this->pMeshTransform = (ed_3d_hierarchy_node*)0x0;
	memset(&this->hierarchySetup, 0, sizeof(ed_3d_hierarchy_setup));
	//this->field_0xcc = 1e+10;
	//this->field_0xd0 = 1e+10;
	//this->field_0xd4 = 1e+10;
	//this->field_0xd8 = 1e+10;
	this->sphereCentre.x = 0.0f;
	this->sphereCentre.y = 0.0f;
	this->sphereCentre.z = 0.0f;
	this->sphereCentre.w = 0.0f;
	this->pClusterNode = (CClusterNode*)0x0;
	this->adjustedMagnitude = 0.0;
	this->pAnimationController = (CAnimation*)0x0;
	this->pCollisionData = (CollisionData*)0x0;
	this->pShadow = (CShadow*)0x0;
	//this->actorFieldG = (Actor*)0x0;
	this->aComponents = (int*)0x0;
	this->curBehaviourId = -1;
	this->prevBehaviourId = -1;
	this->actorState = AS_None;
	this->prevActorState = AS_None;
	this->timeA = 0.0f;
	this->idleTimer = 0.0f;
	this->field_0x110 = (undefined*)0x0;
	this->currentAnimType = -1;
	this->prevAnimType = -1;
	//this->field_0xdc = 0;
	//this->field_0xe0 = 1.0;
	this->field_0xd = 0;
	//this->field_0xc = '\0';
	this->distanceToGround = -1.0f;
	//this->field_0xec = 3.0;
	//this->field_0xf0 = -1;
	fVar2 = gF32Vector3Zero.z;
	fVar1 = gF32Vector3Zero.y;

	this->previousLocation = gF32Vector3Zero;

	fVar2 = gF32Vector3Zero.z;
	fVar1 = gF32Vector3Zero.y;
	//this->vector_0x12c.x = gF32Vector3Zero.x;
	//this->vector_0x12c.y = fVar1;
	//this->vector_0x12c.z = fVar2;
	//this->field_0x138 = 0;
	//this->field_0x134 = 0.0;
	//this->field_0x94 = 0;
	return;
}

void CActor::PreInit()
{
	int iVar3;
	int uVar4;
	int uVar5;
	int componentCount;
	int outIntB;
	int outIntA;
	ComponentEntry* pCVar2;
	CBehaviour* pComponent;
	CShadow* pGVar1;
	
	if (this->pCollisionData != (CollisionData*)0x0) {
		IMPLEMENTATION_GUARD(
		CCollision::Init();)
	}
	pGVar1 = this->pShadow;
	if (pGVar1 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		CShadow::Init(pGVar1, this->field_0x0);)
	}
	this->flags = this->flags & 0xfffffffc;
	this->flags = this->flags & 0xffffff5f;
	EvaluateDisplayState();
	this->flags = this->flags & 0xfffff7ff;
	this->flags = this->flags & 0xffbfffff;
	if ((((this->actorFieldS & 1) != 0) && (this != (CActor*)0x0)) && ((this->flags & 0x2000000) == 0)) {
		ReceiveMessage(this, (ACTOR_MESSAGE)0x5d, 0);
	}
	ComponentList<1>* pComponentList = (ComponentList<1>*)this->aComponents;
	uVar5 = 0;
	uVar4 = 0;
	ComponentEntry* pEntry = pComponentList->aComponents;

	for (componentCount = pComponentList->count; componentCount != 0; componentCount = componentCount + -1) {
		pComponent = pEntry->pComponent;


		if (pComponent != (CBehaviour*)0x0) {
			pComponent->Init(this);
			pComponent->GetDlistPatchableNbVertexAndSprites(&outIntA, &outIntB);
		}
		if (uVar5 < outIntA) {
			uVar5 = outIntA;
		}
		if (uVar4 < outIntB) {
			uVar4 = outIntB;
		}
		pEntry = pEntry + 1;
	}
	this->dlistPatchId = -1;
	if (uVar5 + uVar4 != 0) {
		IMPLEMENTATION_GUARD(
		iVar3 = GameDListPatch_Register((CObject*)this, uVar5, uVar4);)
		this->dlistPatchId = iVar3;
	}
	return;
}

void CActor::EvaluateDisplayState()
{
	int iVar1;
	uint uVar2;

	iVar1 = this->objectId;
	if ((iVar1 == ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID) || (iVar1 == -1)) {
		uVar2 = this->flags;
		if ((uVar2 & 0x2000060) == 0) {
			if ((uVar2 & 0x200) == 0) {
				uVar2 = 1;
			}
			else {
				uVar2 = uVar2 & 0x80 | (uint)(0 < this->field_0xd);
			}
			goto LAB_001033c0;
		}
	}
	uVar2 = 0;
LAB_001033c0:
	if ((this->flags & 0x100) == 0) {
		if (uVar2 != 0) {
			this->ChangeDisplayState(1);
		}
	}
	else {
		if (uVar2 == 0) {
			this->ChangeDisplayState(0);
		}
	}
	return;
}

void CActor::SetScale(float x, float y, float z)
{
	this->scale.x = x;
	this->scale.y = y;
	this->scale.z = z;
	this->scale.w = 1.0f;
	if (((x == 1.0f) && (y == 1.0f)) && (z == 1.0f)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}
	return;
}

bool CActor::IsKindOfObject(ulong kind)
{
	return (kind & 1) != 0;
}

void CActor::Init()
{
	uint* puVar1;

	///* While this pointer is invalid */
	//for (puVar1 = (uint*)this->field_0x144; puVar1 != (uint*)0x0; puVar1 = (uint*)puVar1[0xe]) {
	//	CActorSound::Init(puVar1);
	//}

	this->SetBehaviour((this->subObjA)->mainComponentID_0x0, -1, -1);
	return;
}

void CActor::Manage()
{
	uint uVar1;
	uint* puVar2;
	bool bVar3;
	CBehaviour* pBehaviour;
	Timer* pTimeController;
	ulong uVar4;
	float fVar5;

	pBehaviour = GetBehaviour(this->curBehaviourId);
	if (pBehaviour != (CBehaviour*)0x0) {
		if ((GameFlags & GAME_STATE_PAUSED | this->flags & 0x400000) == 0) {
			pBehaviour->Manage();
		}
		else {
			IMPLEMENTATION_GUARD(
			pBehaviour->ManageFrozen();)
		}
	}
	uVar1 = this->flags;
	fVar5 = (this->subObjA)->floatFieldB;
	if (((uVar1 & 0x100) == 0) || (fVar5 < this->adjustedMagnitude)) {
		if ((uVar1 & 0x4000) != 0) {
			ChangeVisibleState(0);
		}
	}
	else {
		bVar3 = CCameraManager::_gThis->IsSphereVisible(fVar5, &this->sphereCentre);
		if (bVar3 == false) {
			if ((this->flags & 0x4000) != 0) {
				ChangeVisibleState(0);
			}
		}
		else {
			if ((this->flags & 0x4000) == 0) {
				ChangeVisibleState(1);
			}
		}
	}
	if (this->pAnimationController != (CAnimation*)0x0) {
		uVar4 = this->currentAnimType == -1;
		if (this->actorState == 3) {
			fVar5 = 0.0f;
			uVar4 = 0;
		}
		else {
			if ((this->flags & 0x400000) == 0) {
				pTimeController = GetTimer();
				fVar5 = pTimeController->cutsceneDeltaTime;
			}
			else {
				fVar5 = 0.0f;
			}
		}
		this->pAnimationController->Manage(fVar5, this, this->flags & 0x4800, uVar4);
	}
	//ActorFunc_00104c90((Actor*)this);
	//puVar2 = (uint*)this->field_0x144;
	//if (puVar2 != (uint*)0x0) {
	//	(*(code*)this->pVTable->field_0x5c)(this);
	//	for (; puVar2 != (uint*)0x0; puVar2 = (uint*)puVar2[0xe]) {
	//		CActorSound::Manage(puVar2, this);
	//	}
	//}
	pTimeController = Timer::GetTimer();
	this->timeA = this->timeA + pTimeController->cutsceneDeltaTime;
	pTimeController = Timer::GetTimer();
	this->idleTimer = this->idleTimer + pTimeController->cutsceneDeltaTime;
	return;
}

void CActor::ChangeManageState(int state)
{
	EActorState actorState;
	ed_3d_hierarchy_node* pHierNode;
	CAnimation* pAnimation;
	CClusterNode* pClusterNode;
	//AnimResult* pAVar4;
	uint uVar5;
	//CActorSound* pActorSound;

	if (state == 0) {
		IMPLEMENTATION_GUARD(
		this->flags = this->flags & 0xfffffffb;
		pClusterNode = this->pClusterNode;
		if (pClusterNode != (CClusterNode*)0x0) {
			CCluster::DeleteNode((CCluster*)&(CScene::ptable.g_ActorManager_004516a4)->cluster, pClusterNode);
			this->pClusterNode = (CClusterNode*)0x0;
		}
		for (pActorSound = (CActorSound*)this->field_0x144; pActorSound != (CActorSound*)0x0;
			pActorSound = *(CActorSound**)&pActorSound[1].field_0x4) {
			CActorSound::DisableSounds(pActorSound);
		}
		pHierNode = this->p3DHierNode;
		if (pHierNode != (ed_3d_hierarchy_node*)0x0) {
			(pHierNode->base).pAnimMatrix = (edF32MATRIX4*)0x0;
			(pHierNode->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;
		}
		actorState = this->actorState;
		if (actorState == AS_None) {
			uVar5 = 0;
		}
		else {
			pAVar4 = (*this->pVTable->GetStateCfg)(this, actorState);
			uVar5 = pAVar4->flags_0x4 & 0x80;
		}
		if (uVar5 != 0) {
			StateTransitionSoundFunc_00184470((int)CScene::ptable.g_GlobalSoundPtr_00451698);
		})
	}
	else {
		this->flags = this->flags | 4;
		if ((this->pClusterNode == (CClusterNode*)0x0) && (this->typeID != 1)) {
			IMPLEMENTATION_GUARD(
			pClusterNode = CCluster::NewNode((CCluster*)&(CScene::ptable.g_ActorManager_004516a4)->cluster, this);
			this->pClusterNode = pClusterNode;)
		}
		actorState = this->actorState;
		if (actorState == AS_None) {
			uVar5 = 0;
		}
		else {
			IMPLEMENTATION_GUARD_LOG(
			pAVar4 = (*this->pVTable->GetStateCfg)(this, actorState);
			uVar5 = pAVar4->flags_0x4 & 0x80;)
		}
		if (uVar5 != 0) {
			IMPLEMENTATION_GUARD_LOG(
			StateTransitionSoundFunc_001844a0((int)CScene::ptable.g_GlobalSoundPtr_00451698);)
		}
	}
	pAnimation = this->pAnimationController;
	if (pAnimation != (CAnimation*)0x0) {
		pAnimation->StopEventTrack(state);
	}
	return;
}

void CActor::ChangeDisplayState(int state)
{
	CShadow* pCVar1;
	uint ret;
	float BfloatB;

	if (state == 0) {
		pCVar1 = this->pShadow;
		if (pCVar1 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar1 + 0x14))();)
		}
		this->flags = this->flags & 0xfffffeff;
	}
	else {
		pCVar1 = this->pShadow;
		if (pCVar1 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar1 + 0x14))();)
		}
		this->flags = this->flags | 0x100;
	}
	ret = this->flags;
	BfloatB = (this->subObjA)->floatFieldB;
	if (((ret & 0x100) == 0) || (BfloatB < this->adjustedMagnitude)) {
		if ((ret & 0x4000) != 0) {
			this->ChangeVisibleState(0);
		}
	}
	else {
		bool bSphereVisible = CCameraManager::_gThis->IsSphereVisible(BfloatB, &this->sphereCentre);
		if (bSphereVisible == 0) {
			if ((this->flags & 0x4000) != 0) {
				this->ChangeVisibleState(0);
			}
		}
		else {
			if ((this->flags & 0x4000) == 0) {
				this->ChangeVisibleState(1);
			}
		}
	}
	return;
}

void CActor::SetState(int newState, int animType)
{
	EActorState EVar1;
	CAnimation* pAnimationController;
	GlobalSound_00451698* pGVar2;
	CBehaviour* pCVar3;
	//AnimResult* pDesiredAnim;
	uint uVar4;
	uint uVar5;

	pCVar3 = GetBehaviour(this->curBehaviourId);
	if ((animType == -1) && (newState != AS_None)) {
		IMPLEMENTATION_GUARD_LOG(
		pDesiredAnim = (*pActor->pVTable->GetStateCfg)(pActor, newState);
		animType = pDesiredAnim->field_0x0;)
	}
	pGVar2 = CScene::ptable.g_GlobalSoundPtr_00451698;
	EVar1 = this->actorState;
	if (EVar1 == newState) {
		/* State is the same as our current state */
		if (this->currentAnimType != animType) {
			IMPLEMENTATION_GUARD(
			PlayAnim(pActor, animType);)
		}
	}
	else {
		/* New state */
		if (EVar1 == AS_None) {
			uVar4 = 0;
		}
		else {
			IMPLEMENTATION_GUARD(
			pDesiredAnim = (*pActor->pVTable->GetStateCfg)(pActor, EVar1);
			uVar4 = pDesiredAnim->flags_0x4;)
		}
		if (newState == AS_None) {
			uVar5 = 0;
		}
		else {
			IMPLEMENTATION_GUARD_LOG(
			pDesiredAnim = (*pActor->pVTable->GetStateCfg)(pActor, newState);
			uVar5 = pDesiredAnim->flags_0x4;)
		}
		if (((uVar4 & 0x80) == 0) || ((uVar5 & 0x80) != 0)) {
			if (((uVar4 & 0x80) == 0) && ((uVar5 & 0x80) != 0)) {
				IMPLEMENTATION_GUARD_LOG(
				StateTransitionSoundFunc_001844a0((int)pGVar2);)
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			StateTransitionSoundFunc_00184470((int)pGVar2);)
		}
		if ((pCVar3 != (CBehaviour*)0x0) && (EVar1 = this->actorState, EVar1 != AS_None)) {
			/* End old state? */
			IMPLEMENTATION_GUARD(
			(*(code*)pCVar3->pVTable->TermState)(pCVar3, EVar1, newState);)
		}
		this->prevActorState = this->actorState;
		this->actorState = (EActorState)newState;
		if ((this->numIdleLoops != 0) &&
			(pAnimationController = this->pAnimationController, pAnimationController != (CAnimation*)0x0)) {
				pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		}
		if (this->actorState == AS_None) {
			IMPLEMENTATION_GUARD(
			PlayAnim(animType);)
		}
		else {
			PlayAnim(animType);
			if (pCVar3 != (CBehaviour*)0x0) {
				pCVar3->InitState(this->actorState);
			}
		}
		this->timeA = 0.0f;
		this->idleTimer = 0.0f;
		this->numIdleLoops = 0;
	}
	return;
}

bool CActor::SetBehaviour(int behaviourId, int newState, int animationType)
{
	uint uVar1;
	bool bVar2;
	CBehaviour* pComponent;
	CBehaviour* pSVar3;
	int iVar4;

	uVar1 = this->flags;
	if (((uVar1 & 0x800000) == 0) && (((uVar1 & 0x2000000) == 0 || (behaviourId == 1)))) {
		if (this->curBehaviourId == behaviourId) {
			if (newState != -1) {
				this->SetState(newState, animationType);
			}
			bVar2 = false;
		}
		else {
			pComponent = (CBehaviour*)0x0;
			if ((behaviourId == -1) || (pComponent = this->GetBehaviour(behaviourId), pComponent != (CBehaviour*)0x0)) {
				pSVar3 = this->GetBehaviour(this->curBehaviourId);
				if (pSVar3 != (CBehaviour*)0x0) {
					if (pComponent == (CBehaviour*)0x0) {
						this->SetState(-1, -1);
					}
					else {
						this->SetState(-1, this->currentAnimType);
					}
					IMPLEMENTATION_GUARD();
					pSVar3->End(behaviourId);
				}
				this->prevBehaviourId = this->curBehaviourId;
				this->curBehaviourId = behaviourId;
				if ((pComponent != (CBehaviour*)0x0) && pComponent->Begin(this, newState, animationType), this->dlistPatchId != -1) {
					IMPLEMENTATION_GUARD(
					iVar4 = GetManagerObject(MO_17);
					ActorFunc_002d79e0(iVar4, this->dlistPatchId, 0, (Actor*)this));
				}
				bVar2 = true;
			}
			else {
				bVar2 = false;
			}
		}
	}
	else {
		bVar2 = false;
	}
	return bVar2;
}

void CActor::CinematicMode_Enter(bool bSetState)
{
	//CollisionData* pCVar1;
	CCinematic* pCinematic;
	CCineActorConfig* pActorConfig;

	IMPLEMENTATION_GUARD_LOG(edEventComputeZoneAgainstVertex(this, this, 0x3e, (ActorCompareStruct*)0x0));
	if (bSetState == false) {
		this->SetBehaviour(1, -1, -1);
	}
	else {
		this->SetBehaviour(1, 1, -1);
	}
	this->flags = this->flags | 0x800000;
	IMPLEMENTATION_GUARD_LOG(ActorMatrixFieldSet_00101110((Actor*)this, 0));
	if ((this->flags & 0x2000060) != 0) {
		this->flags = this->flags | 0x8000000;
		this->flags = this->flags & 0xffffff5f;
		this->EvaluateDisplayState();
	}
	pCinematic = g_CinematicManager_0048efc->GetCurCinematic();
	pActorConfig = pCinematic->GetActorConfig(this);
	if (pActorConfig != (CCineActorConfig*)0x0) {
		IMPLEMENTATION_GUARD(
		if ((pActorConfig->flags & 0x20) == 0) {
			(*(code*)this->pVTable->CinematicMode_InterpreteCinMessage)(0, 0x3f800000, this, 4, 0);
		}
		else {
			(*(code*)this->pVTable->CinematicMode_InterpreteCinMessage)(0, 0x3f800000, this, 3, 0);
		}
		if (((pActorConfig->flags & 0x80) != 0) && (pCVar1 = this->pCollisionData, pCVar1 != (CollisionData*)0x0)) {
			pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xfff7efff;
		}
		if (((pActorConfig->flags & 0x100) != 0) && (this->actorFieldG != (Actor*)0x0)) {
			(*(code*)this->pVTable->field_0xd0)(this, 0, 0, 1, 0);
		})
	}
	return;
}

void CActor::CinematicMode_UpdateMatrix(edF32MATRIX4* pPosition)
{
	UpdatePosition(pPosition, 1);
}

void CActor::CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int param_3)
{
	char cVar1;
	char cVar2;
	edANM_HDR* pHdr;
	edANM_HDR* pHdr_00;
	edAnmLayer* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float local_30[4];
	float local_20[4];
	float local_10[4];
	CAnimation* pAnimation;

	pAnimation = this->pAnimationController;
	if (pAnimation != (CAnimation*)0x0) {
		pAnimation->anmBinMetaAnimator.SetLayerTimeWarper(0.0f, 0);
		pHdr = (edANM_HDR*)LOAD_SECTION(pTag->pHdrB);
		pHdr_00 = (edANM_HDR*)LOAD_SECTION(pTag->pHdrA);
		if ((pHdr == (edANM_HDR*)0x0) || (param_3 != 0)) {
			peVar3 = (pAnimation->anmBinMetaAnimator).aAnimData;
			cVar1 = pTag->field_0x8;
			fVar4 = pTag->field_0x4;
			peVar3->animPlayState = 0;
			peVar3->SetRawAnim(pHdr_00, (uint)(cVar1 != '\0'), 0xfffffffe);
			edAnmStage::ComputeAnimParams
			(fVar4, (peVar3->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_10, false,
				(uint)(((peVar3->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));
			(peVar3->currentAnimDesc).state.time_0x10 = local_10[0];
			(peVar3->currentAnimDesc).state.time_0xc = local_10[1];
		}
		else {
			peVar3 = (pAnimation->anmBinMetaAnimator).aAnimData;
			cVar1 = pTag->field_0x14;
			fVar5 = pTag->field_0x18;
			cVar2 = pTag->field_0x8;
			fVar4 = pTag->field_0x4;
			fVar6 = pTag->field_0x10;
			peVar3->animPlayState = 0;
			peVar3->SetRawAnim(pHdr, (uint)(cVar1 != '\0'), (int)&pHdr->flags + 1);
			peVar3->SetRawAnim(pHdr_00, (uint)(cVar2 != '\0'), (int)&pHdr_00->flags + 2);
			peVar3->field_0xbc = 1.0f;
			peVar3->MorphingStartDT();
			(peVar3->currentAnimDesc).field_0x4c = 1.0f;
			(peVar3->nextAnimDesc).field_0x4c = fVar5;
			edAnmStage::ComputeAnimParams
			(fVar6, (peVar3->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_20, false,
				(uint)(((peVar3->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));
			(peVar3->currentAnimDesc).state.time_0x10 = local_20[0];
			(peVar3->currentAnimDesc).state.time_0xc = local_20[1];
			edAnmStage::ComputeAnimParams
			(fVar4, (peVar3->nextAnimDesc).state.keyStartTime_0x14, 0.0f, local_30, false,
				(uint)(((peVar3->nextAnimDesc).state.currentAnimDataFlags & 1) != 0));
			(peVar3->nextAnimDesc).state.time_0x10 = local_30[0];
			(peVar3->nextAnimDesc).state.time_0xc = local_30[1];
		}
	}
	return;
}

uint CActor::IsLookingAt()
{
	return this->flags & 0x2000;
}

void CActor::UpdateAnimEffects()
{
	CBehaviour* pCVar1;
	Timer* pTVar2;

	if (((this->flags & 0x800000) != 0) && (pCVar1 = GetBehaviour(1), pCVar1 != (CBehaviour*)0x0)) {
		pTVar2 = Timer::GetTimer();
		IMPLEMENTATION_GUARD_LOG(
		CEmotionInfo::DoAnimation(pTVar2->cutsceneDeltaTime, 1.0, (int*)(pCVar1 + 0x51), (int)pCVar1[1].pVTable);)
	}
	return;
}

void CActor::UpdatePostAnimEffects()
{
	return;
}

void CActor::ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam)
{
	bool bHandled;
	CBehaviour* pBehaviour;

	pBehaviour = GetBehaviour(this->curBehaviourId);
	if (pBehaviour == (CBehaviour*)0x0) {
		bHandled = false;
	}
	else {
		bHandled = pBehaviour->InterpretMessage(pSender, msg, pMsgParam);
	}
	if (bHandled == false) {
		InterpretMessage(pSender, msg, pMsgParam);
	}
	return;
}

bool CActor::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	CollisionData* pCVar2;
	CActor* pCVar3;
	bool bVar4;
	long lVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float local_18;
	float local_c;
	float local_8;

	fVar8 = gF32Vector4Zero.w;
	fVar7 = gF32Vector4Zero.z;
	fVar6 = gF32Vector4Zero.y;
	if (((msg == 0x78) || (msg == 0x77)) || (msg == 0x76)) {
		IMPLEMENTATION_GUARD(
		if (msg == 0x78) {
			local_18 = 0.3;
		}
		else {
			if (msg == 0x77) {
				local_18 = 0.2;
			}
			else {
				if (msg == 0x76) {
					local_18 = 0.1;
				}
			}
		}
		lVar5 = (*(code*)this->pVTable->GetInputManager)(this, 1, 0);
		if (lVar5 != 0) {
			FloatFunc_001b66f0(1.0, 0.0, local_18, 0.0, (float*)((int)lVar5 + 0x1c), 0);
		}
		bVar4 = true;)
	}
	else {
		if (((((msg == 0x75) || (msg == 0x74)) || ((msg == 0x73 || ((msg == 0x72 || (msg == 0x71)))))) || (msg == 0x70)) ||
			(((msg == 0x6f || (msg == 0x6e)) || (msg == 0x6d)))) {
			IMPLEMENTATION_GUARD(
			switch (msg) {
			case 0x6d:
			case 0x6e:
			case 0x6f:
				local_c = 0.4;
				if (msg == 0x6f) {
					local_8 = 0.6;
				}
				else {
					if (msg == 0x6e) {
						local_8 = 0.4;
					}
					else {
						if (msg == 0x6d) {
							local_8 = 0.2;
						}
					}
				}
				break;
			case 0x70:
			case 0x71:
			case 0x72:
				local_c = 0.6;
				if (msg == 0x72) {
					local_8 = 0.5;
				}
				else {
					if (msg == 0x71) {
						local_8 = 0.3;
					}
					else {
						if (msg == 0x70) {
							local_8 = 0.15;
						}
					}
				}
				break;
			case 0x73:
			case 0x74:
			case 0x75:
				local_c = 1.0;
				if (msg == 0x75) {
					local_8 = 0.3;
				}
				else {
					if (msg == 0x74) {
						local_8 = 0.2;
					}
					else {
						if (msg == 0x73) {
							local_8 = 0.1;
						}
					}
				}
			}
			lVar5 = (*(code*)this->pVTable->GetInputManager)(this, 1, 0);
			if (lVar5 != 0) {
				FloatFunc_001b66f0(local_c, 0.0, local_8, 0.0, (float*)((int)lVar5 + 0x40), 0);
			}
			bVar4 = true;)
		}
		else {
			if (msg == 0x6c) {
				IMPLEMENTATION_GUARD(
				lVar5 = (*(code*)this->pVTable->GetInputManager)(this, 1, 0);
				if (lVar5 != 0) {
					/* WARNING: Load size is inaccurate */
					if (*pMsgParam == 0) {
						FloatFunc_001b66f0(*(float*)((int)pMsgParam + 4), 0.0, *(float*)((int)pMsgParam + 8), 0.0,
							(float*)((int)lVar5 + 0x1c), 0);
					}
					else {
						FloatFunc_001b66f0(*(float*)((int)pMsgParam + 4), 0.0, *(float*)((int)pMsgParam + 8), 0.0,
							(float*)((int)lVar5 + 0x40), 0);
					}
				}
				bVar4 = true;)
			}
			else {
				if (msg == 0x5c) {
					IMPLEMENTATION_GUARD(
					this->flags = this->flags & 0xffffff5f;
					EvaluateDisplayState();
					this->flags = this->flags & 0xfffffffc;
					pCVar2 = this->pCollisionData;
					if (pCVar2 != (CollisionData*)0x0) {
						pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;
					}
					bVar4 = true;)
				}
				else {
					if (msg == 0x5d) {
						pCVar2 = this->pCollisionData;
						if (pCVar2 != (CollisionData*)0x0) {
							IMPLEMENTATION_GUARD(
							pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7efff;)
						}
						this->flags = this->flags & 0xffffff7f;
						this->flags = this->flags | 0x20;
						EvaluateDisplayState();
						bVar4 = true;
						this->flags = this->flags & 0xfffffffd;
						this->flags = this->flags | 1;
					}
					else {
						IMPLEMENTATION_GUARD(
						if (msg == 0x32) {
							pCVar2 = this->pCollisionData;
							if ((pCVar2 != (CollisionData*)0x0) && (pMsgParam != (void*)0x0)) {
								pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;
							}
							this->flags = this->flags | 0x80;
							this->flags = this->flags & 0xffffffdf;
							EvaluateDisplayState();
							bVar4 = true;
						}
						else {
							if (msg == 0x31) {
								pCVar2 = this->pCollisionData;
								if ((pCVar2 != (CollisionData*)0x0) && (pMsgParam != (void*)0x0)) {
									pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7efff;
								}
								this->flags = this->flags & 0xffffff7f;
								this->flags = this->flags | 0x20;
								EvaluateDisplayState();
								bVar4 = true;
							}
							else {
								if (msg == 0x3c) {
									pCVar3 = this->actorFieldG;
									ActorFunc_00104310(this);
									(*(code*)this->pVTable->Reset)(this);
									if (((pCVar3 != (CActor*)0x0) && (pCVar3 != (CActor*)0x0)) &&
										(((pCVar3->data).flags & 0x2000000) == 0)) {
										(*(code*)pCVar3->pVTable->ReceiveMessage)(pCVar3, this, 0x3d, 0);
									}
									bVar4 = true;
								}
								else {
									if (msg == 0x38) {
										if ((this->flags & 0x400000) == 0) {
											this->flags = this->flags | 0x400000;
										}
										else {
											this->flags = this->flags & 0xffbfffff;
										}
										bVar4 = true;
									}
									else {
										if (msg == 0x37) {
											bVar4 = true;
											this->flags = this->flags & 0xffbfffff;
										}
										else {
											if (msg == 0x36) {
												bVar4 = true;
												this->flags = this->flags | 0x400000;
											}
											else {
												if (msg == 0x49) {
													bVar4 = true;
													fVar8 = this->currentLocation.y;
													fVar6 = this->currentLocation.z;
													fVar7 = this->currentLocation.w;
													*(float*)pMsgParam = this->currentLocation.x;
													*(float*)((int)pMsgParam + 4) = fVar8;
													*(float*)((int)pMsgParam + 8) = fVar6;
													*(float*)((int)pMsgParam + 0xc) = fVar7;
												}
												else {
													if (msg == 7) {
														/* WARNING: Load size is inaccurate */
														iVar1 = *pMsgParam;
														if (iVar1 == 5) {
															if (((this->pMeshTransform != (ed_3d_hierarchy_node*)0x0) &&
																(pCVar2 = this->pCollisionData, pCVar2 != (CollisionData*)0x0)) &&
																(pCVar2->subObjFieldA != 0)) {
																*(undefined4*)((int)pMsgParam + 0x20) = 0;
																*(undefined4*)((int)pMsgParam + 0x24) =
																	*(undefined4*)((this->pCollisionData)->subObjFieldA + 0xb4);
																*(undefined4*)((int)pMsgParam + 0x28) = 0;
																*(undefined4*)((int)pMsgParam + 0x2c) = 0;
																edF32Matrix4MulF32Vector4Hard
																((edF32VECTOR4*)((int)pMsgParam + 0x20),
																	(edF32MATRIX4*)this->pMeshTransform,
																	(edF32VECTOR4*)((int)pMsgParam + 0x20));
																return true;
															}
														}
														else {
															if ((iVar1 == 1) || (iVar1 == 0)) {
																*(float*)((int)pMsgParam + 0x20) = gF32Vector4Zero.x;
																*(float*)((int)pMsgParam + 0x24) = fVar6;
																*(float*)((int)pMsgParam + 0x28) = fVar7;
																*(float*)((int)pMsgParam + 0x2c) = fVar8;
																pCVar2 = this->pCollisionData;
																if ((pCVar2 != (CollisionData*)0x0) && (iVar1 = pCVar2->subObjFieldA, iVar1 != 0)) {
																	edF32Matrix4MulF32Vector4Hard
																	((edF32VECTOR4*)((int)pMsgParam + 0x20),
																		(edF32MATRIX4*)this->pMeshTransform, (edF32VECTOR4*)(iVar1 + 0xb0))
																		;
																	edF32Vector4SubHard((edF32VECTOR4*)((int)pMsgParam + 0x20),
																		(edF32VECTOR4*)((int)pMsgParam + 0x20),
																		&this->currentLocation);
																	return true;
																}
																return true;
															}
														}
													}
													bVar4 = false;
												}
											}
										}
									}
								}
							}
						})
					}
				}
			}
		}
	}
	return bVar4;
}

void CActor::ChangeVisibleState(int bVisible)
{
	edNODE* peVar1;
	CShadow* pCVar2;
	bool bVar3;
	ed_3D_Scene* peVar4;

	if (bVisible == 0) {
		peVar1 = this->pMeshNode;
		if (peVar1 != (edNODE*)0x0) {
			peVar4 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(peVar4, peVar1);
		}
		pCVar2 = this->pShadow;
		if (pCVar2 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar2 + 0x14))(pCVar2, 0);)
		}
		this->flags = this->flags & 0xffffbfff;
	}
	else {
		peVar1 = this->pMeshNode;
		if (peVar1 != (edNODE*)0x0) {
			peVar4 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOff(peVar4, peVar1);
		}
		pCVar2 = this->pShadow;
		if (pCVar2 != (CShadow*)0x0) {
			bVar3 = (this->flags & 0x4400) != 0;
			if (bVar3) {
				bVar3 = this->adjustedMagnitude <= (this->subObjA)->field_0x1c;
			}
			if (bVar3 != false) {
				bVar3 = (this->flags & 0x100) != 0;
			}
			IMPLEMENTATION_GUARD(
			(**(code**)(*(int*)pCVar2 + 0x14))(pCVar2, bVar3);)
		}
		this->flags = this->flags | 0x4000;
	}
	return;
}

CBehaviour* CActor::GetBehaviour(int behaviourId)
{
	ComponentList<1>* pComponentList = (ComponentList<1>*)this->aComponents;
	CBehaviour* pOutBehaviour = (CBehaviour*)0x0;

	for (int i = 0; i < pComponentList->count; i++) {
		if (pComponentList->aComponents[i].id == behaviourId) {
			pOutBehaviour = pComponentList->aComponents[i].pComponent;
		}
	}

	return pOutBehaviour;
}

void CActor::SetupClippingInfo()
{
	this->hierarchySetup.clipping_0x0 = 0x0;
	this->hierarchySetup.pBoundingSphere = &this->subObjA->boundingSphere;
}

void CActor::SetupDefaultPosition()
{
	edF32VECTOR3* v0;
	float fVar1;
	float fVar2;
	CinNamedObject30* namedObj30;

	namedObj30 = this->pCinData;
	this->baseLocation.xyz = namedObj30->position;
	this->baseLocation.w = 1.0;
	namedObj30 = this->pCinData;
	if (fabs(namedObj30->floatFieldA - 1.0f) <= 0.0001f) {
		namedObj30->floatFieldA = 1.0f;
	}
	if (fabs(namedObj30->floatFieldB - 1.0f) <= 0.0001f) {
		namedObj30->floatFieldB = 1.0f;
	}
	if (fabs(namedObj30->floatFieldC - 1.0f) <= 0.0001f) {
		namedObj30->floatFieldC = 1.0f;
	}
	if ((CActorFactory::gClassProperties[this->typeID].flags_0x0 & 2) != 0) {
		IMPLEMENTATION_GUARD(
		v0 = &(this->pCinData)->vectorThree_0x18;
		SV_BuildAngleWithOnlyY(v0, v0);)
	}
	if ((CActorFactory::gClassProperties[this->typeID].flags_0x0 & 0x800) == 0) {
		this->flags = this->flags & 0xffffefff;
	}
	else {
		this->flags = this->flags | 0x1000;
	}
	return;
}

void CActor::RestoreInitData()
{
	CollisionData* pCVar1;
	CinNamedObject30* pCVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	if (((this->actorFieldS & 4) != 0) && (pCVar1 = this->pCollisionData, pCVar1 != (CollisionData*)0x0)) {
		IMPLEMENTATION_GUARD(
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x20000;)
	}
	pCVar2 = this->pCinData;
	fVar4 = pCVar2->floatFieldA;
	fVar3 = pCVar2->floatFieldC;
	fVar5 = pCVar2->floatFieldB;
	this->scale.x = fVar4;
	this->scale.y = fVar5;
	this->scale.z = fVar3;
	this->scale.w = 1.0;
	if (((fVar4 == 1.0) && (fVar5 == 1.0)) && (fVar3 == 1.0)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}
	pCVar2 = this->pCinData;
	fVar3 = (pCVar2->vectorThree_0x18).y;
	fVar4 = (pCVar2->vectorThree_0x18).z;
	this->rotationEuler.x = (pCVar2->vectorThree_0x18).x;
	this->rotationEuler.y = fVar3;
	this->rotationEuler.z = fVar4;
	this->field_0x58 = 0;
	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler);
	UpdatePosition(&this->baseLocation, true);
	this->flags = this->flags | 0x80000;
	this->distanceToGround = -1.0f;
	this->flags = this->flags & 0xffdfffff;
	this->field_0xf0 = -1;
	return;
}

void CActor::UpdatePosition(edF32VECTOR4* v0, bool bUpdateCollision)
{
	ed_3d_hierarchy_node* pHier;
	KyaUpdateObjA* pKVar1;
	CollisionData* pCVar2;
	CClusterNode* pSubObj;
	int iVar3;
	edF32MATRIX4* pIdentityMatrix;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32MATRIX4 eStack224;
	edF32VECTOR4 local_a0;
	edF32MATRIX4 eStack144;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	this->currentLocation = *v0;
	pHier = this->pMeshTransform;

	if ((this->flags & 0x4000000) == 0) {
		if ((this->flags & 0x1000) == 0) {
			edF32Matrix4BuildFromVectorUnitSoft(&pHier->base.transformA, &this->rotationQuat);
			fVar5 = this->rotationEuler.z;
			if (0.0001f <= fabs(fVar5)) {
				edF32Matrix4RotateZHard(fVar5, &eStack224, &gF32Matrix4Unit);
				edF32Matrix4MulF32Matrix4Hard(&pHier->base.transformA, &eStack224, &pHier->base.transformA);
			}
		}
		else {
			edF32Matrix4FromEulerSoft(&pHier->base.transformA, &this->rotationEuler, "XYZ");
		}

		(pHier->base).transformA.rowT = this->currentLocation;

		pKVar1 = this->subObjA;
		local_50.xyz = pKVar1->boundingSphere.xyz;
		local_50.w = 1.0;
		edF32Matrix4MulF32Vector4Hard(&local_50, &pHier->base.transformA, &local_50);


		this->sphereCentre.xyz = local_50.xyz;
		this->sphereCentre.w = (pKVar1->boundingSphere).w;
		if ((bUpdateCollision != false) && (pCVar2 = this->pCollisionData, pCVar2 != (CollisionData*)0x0)) {
			IMPLEMENTATION_GUARD(
			CCollision::UpdateMatrix(pCVar2, &pHier->base.transformA);)
		}
	}
	else {
		if ((this->flags & 0x1000) == 0) {
			edF32Matrix4BuildFromVectorUnitSoft(&eStack64, &this->rotationQuat);
			fVar5 = this->rotationEuler.z;
			if (0.0001f <= fabs(fVar5)) {
				edF32Matrix4RotateZHard(fVar5, &eStack144, &gF32Matrix4Unit);
				edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack144, &eStack64);
			}
		}
		else {
			edF32Matrix4FromEulerSoft(&eStack64, &this->rotationEuler, "XYZ");
		}
		eStack64.rowT = this->currentLocation;

		(pHier->base).transformA = gF32Matrix4Unit;

		
		(pHier->base).transformA.rowX.x = this->scale.x;
		(pHier->base).transformA.rowY.y = this->scale.y;
		(pHier->base).transformA.rowZ.z = this->scale.z;
		edF32Matrix4MulF32Matrix4Hard(&pHier->base.transformA, &pHier->base.transformA, &eStack64);
		pKVar1 = this->subObjA;
		local_a0.xyz = (pKVar1->boundingSphere).xyz;
		local_a0.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_a0, &pHier->base.transformA, &local_a0);
		this->sphereCentre.xyz = local_a0.xyz;
		fVar6 = this->scale.y;
		fVar5 = this->scale.x;
		IMPLEMENTATION_GUARD();
		fVar5 = (float)((int)fVar5 * (uint)(fVar6 < fVar5) | (int)fVar6 * (uint)(fVar6 >= fVar5));
		fVar6 = this->scale.z;
		this->sphereCentre.w =
			(pKVar1->boundingSphere).w * (float)((int)fVar6 * (uint)(fVar5 < fVar6) | (int)fVar5 * (uint)(fVar5 >= fVar6));
		if (bUpdateCollision != false) {
			pCVar2 = this->pCollisionData;
			if (pCVar2 != (CollisionData*)0x0) {
				IMPLEMENTATION_GUARD(
				CCollision::UpdateMatrix(pCVar2, &eStack64);)
			}
		}
	}
	pSubObj = this->pClusterNode;
	if (pSubObj != (CClusterNode*)0x0) {
		IMPLEMENTATION_GUARD(
		pSubObj->Update((CCluster*)&(CScene::ptable.g_ActorManager_004516a4)->cluster);)
	}
	return;
}

void CActor::UpdatePosition(edF32MATRIX4* pPosition, int bUpdateCollision)
{
	ed_3d_hierarchy_node* pHier;
	KyaUpdateObjA* pKVar1;
	CollisionData* pCollisionData;
	CClusterNode* pSubObj;
	int iVar2;
	edF32MATRIX4* peVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	this->currentLocation = pPosition->rowT;

	pHier = this->pMeshTransform;
	if ((this->flags & 0x4000000) == 0) {
		(pHier->base).transformA = *pPosition;
		
		pKVar1 = this->subObjA;
		local_10.xyz = (pKVar1->boundingSphere).xyz;
		local_10.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_10, &pHier->base.transformA, &local_10);
		this->sphereCentre.xyz = local_10.xyz;
		this->sphereCentre.w = (pKVar1->boundingSphere).w;
	}
	else {
		(pHier->base).transformA = gF32Matrix4Unit;


		(pHier->base).transformA.rowX.x = this->scale.x;
		(pHier->base).transformA.rowY.y = this->scale.y;
		(pHier->base).transformA.rowZ.z = this->scale.z;

		edF32Matrix4MulF32Matrix4Hard(&pHier->base.transformA, &pHier->base.transformA, pPosition);
		pKVar1 = this->subObjA;
		local_20.xyz = (pKVar1->boundingSphere).xyz;
		local_20.w = 1.0f;
		edF32Matrix4MulF32Vector4Hard(&local_20, &pHier->base.transformA, &local_20);
		this->sphereCentre.xyz = local_20.xyz;
		fVar6 = this->scale.y;
		fVar5 = this->scale.x;
		fVar5 = (float)((int)fVar5 * (uint)(fVar6 < fVar5) | (int)fVar6 * (uint)(fVar6 >= fVar5));
		fVar6 = this->scale.z;
		this->sphereCentre.w = (pKVar1->boundingSphere).w * (float)((int)fVar6 * (uint)(fVar5 < fVar6) | (int)fVar5 * (uint)(fVar5 >= fVar6));
	}
	if ((bUpdateCollision != 0) && (pCollisionData = this->pCollisionData, pCollisionData != (CollisionData*)0x0))
	{
		IMPLEMENTATION_GUARD(
		CCollision::UpdateMatrix(pCollisionData, pPosition);)
	}
	pSubObj = this->pClusterNode;
	if (pSubObj != (CClusterNode*)0x0) {
		IMPLEMENTATION_GUARD(
		CClusterNode::Update(pSubObj, (CCluster*)&(CScene::ptable.g_ActorManager_004516a4)->cluster);)
	}
	return;
}

void CActor::PlayAnim(int inAnimType)
{
	int animType;
	CAnimation* pAnimationController;

	if (inAnimType != this->currentAnimType) {
		/* Continue playing existing animation */
		pAnimationController = this->pAnimationController;
		if ((this->field_0x110 == (undefined*)0x0) || (pAnimationController == (CAnimation*)0x0)) {
			this->currentAnimType = -1;
		}
		else {
			/* Play new animation */
			animType = -1;
			if (inAnimType != -1) {
				/* Remove for T-Pose */
				IMPLEMENTATION_GUARD(
				animType = GetIdMacroAnim(this, inAnimType);)
			}
			if (animType == -1) {
				this->currentAnimType = -1;
				IMPLEMENTATION_GUARD(
				CAnimation::ResetLayers(pAnimationController, this);)
			}
			else {
				this->prevAnimType = this->currentAnimType;
				this->currentAnimType = inAnimType;
				pAnimationController->PlayAnim(this, animType, inAnimType);
			}
		}
	}
	return;
}

void CBehaviour::GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites)
{
	*nbVertex = 0;
	*nbSprites = 0;
	return;
}

bool CBehaviour::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return false;
}

