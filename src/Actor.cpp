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
#include "MemoryStream.h"
#include "PoolAllocators.h"
#include "Actor_Cinematic.h"

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
	this->actorManagerIndex = -1;
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
	BehaviourEntry* pCVar2;
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
	BehaviourList<1>* pComponentList = (BehaviourList<1>*)this->aComponents;
	uVar5 = 0;
	uVar4 = 0;
	BehaviourEntry* pEntry = pComponentList->aComponents;

	for (componentCount = pComponentList->count; componentCount != 0; componentCount = componentCount + -1) {
		pComponent = pEntry->GetBehaviour();


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
		IMPLEMENTATION_GUARD_LOG(
		iVar3 = GameDListPatch_Register(this, uVar5, uVar4);)
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

ulong ed3DComputeHashCode(char* inString)
{
	size_t sVar1;
	char currentCharacter;
	uint uVar2;
	ulong uVar3;
	ulong local_8;
	bool isTildeOrCurlyBracket;

	local_8 = 0;
	sVar1 = strlen(inString);
	uVar3 = 0;
	while (true) {
		uVar2 = (uint)uVar3;
		if (sVar1 <= uVar3) break;
		currentCharacter = inString[uVar2];
		isTildeOrCurlyBracket = false;
		if (('`' < currentCharacter) && (currentCharacter < '{')) {
			isTildeOrCurlyBracket = true;
		}
		if (isTildeOrCurlyBracket) {
			currentCharacter = currentCharacter + -0x20;
		}
		uVar3 = uVar2 + 1;
		*(char*)((char*)&local_8 + (uVar2 & 7)) = *(char*)((char*)&local_8 + (uVar2 & 7)) + currentCharacter;
	}
	return local_8;
}

void CActor::Create(ByteCode* pByteCode)
{
	CinNamedObject30* pCVar1;
	KyaUpdateObjA* pKVar2;
	CollisionData* pCVar3;
	char* newPos;
	char* pcVar4;
	char* inString;
	char* newPos_00;
	ulong lVar5;
	ulong uVar6;
	int iVar7;
	int iVar8;
	int* piVar9;
	MeshTextureHash* pMVar9;
	float fVar11;
	float fVar12;
	float fVar13;
	MeshTextureHash local_110[16];
	ScenaricCondition local_4;

	char* name = pByteCode->GetString();

	ACTOR_LOG(LogLevel::Info, "CActor::Create {}", name);

	pByteCode->Align(4);
	pCVar1 = (CinNamedObject30*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pCVar1 + 1);
	this->pCinData = pCVar1;
	piVar9 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar9 + 1);
	if (*piVar9 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar9 * 8;
	}
	this->field_0x110 = (undefined*)piVar9;
	if (*(int*)this->field_0x110 < 1) {
		this->field_0x110 = (undefined*)0x0;
	}
	newPos = pByteCode->GetPosition();
	pByteCode->Align(4);
	piVar9 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar9 + 1);
	if (*piVar9 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar9 * 8;
	}
	iVar7 = *piVar9;
	piVar9 = piVar9 + 1;
	if (0 < iVar7) {
		for (; 0 < iVar7; iVar7 = iVar7 + -1) {
			pcVar4 = pByteCode->GetPosition();
			pByteCode->SetPosition(pcVar4 + piVar9[1]);
			piVar9 = piVar9 + 2;
		}
	}
	iVar7 = pByteCode->GetS32();
	iVar8 = 0;
	if (0 < iVar7) {
		pMVar9 = local_110;
		do {
			pcVar4 = pByteCode->GetString();
			inString = pByteCode->GetString();
			lVar5 = ed3DComputeHashCode(pcVar4);
			pMVar9->meshHash = lVar5;
			lVar5 = ed3DComputeHashCode(inString);
			pMVar9->textureHash = lVar5;
			iVar8 = iVar8 + 1;
			pMVar9 = pMVar9 + 1;
		} while (iVar8 < iVar7);
	}
	pKVar2 = (KyaUpdateObjA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pKVar2 + 1);
	this->subObjA = pKVar2;
	pKVar2 = this->subObjA;
	if (pKVar2->mainComponentID_0x0 == 1) {
		pKVar2->mainComponentID_0x0 = -1;
	}
	this->actorFieldS = *(uint*)&(this->subObjA)->field_0x38;
	this->objectId = (this->subObjA)->field_0x24;
	if (this->objectId == 0) {
		this->objectId = -1;
	}
	SetupModel(iVar7, local_110);
	SetupDefaultPosition();
	this->hierarchySetup.clipping_0x0 = (char*)&(this->subObjA)->floatFieldB;
	this->hierarchySetup.pBoundingSphere = &(this->subObjA)->boundingSphere;
	//this->field_0xcc = (this->subObjA)->field_0x2c;
	//this->field_0xd0 = (this->subObjA)->field_0x30;
	//this->field_0xd4 = 10000.0;
	//this->field_0xd8 = 1e+10;
	//fVar11 = this->field_0xcc;
	//this->field_0xcc = fVar11 * fVar11;
	//fVar11 = this->field_0xd0;
	//this->field_0xd0 = fVar11 * fVar11;
	//fVar11 = this->field_0xd4;
	//this->field_0xd4 = fVar11 * fVar11;
	//fVar11 = this->field_0xd8;
	//this->field_0xd8 = fVar11 * fVar11;
	//this->field_0xdc = 1;
	//if (((this->subObjA)->flags_0x48 & 1) != 0) {
	//	this->field_0xdc = this->field_0xdc | 2;
	//}
	//if (((this->subObjA)->flags_0x48 & 2) == 0) {
	//	this->field_0xdc = this->field_0xdc | 4;
	//}
	//this->field_0xe0 = (this->subObjA)->field_0x4c;
	//if ((CActorFactory::gClassProperties[this->typeID].flags_0x0 & 0x20) != 0) {
	//	this->actorFieldS = this->actorFieldS | 0x10;
	//}
	//if (CActorFactory::gClassProperties[this->typeID].field_0x14 == 0) {
	//	this->actorFieldS = this->actorFieldS & 0xffffffef;
	//}
	//if ((this->actorFieldS & 0x80) != 0) {
	//	FUN_00115ea0(this, 0);
	//}
	local_4.field_0x0 = 0;
	local_4.Create(pByteCode);
	uVar6 = local_4.IsVerified();
	if (uVar6 == 0) {
		this->flags = this->flags | 0x2000000;
	}
	newPos_00 = pByteCode->GetPosition();
	pByteCode->SetPosition(newPos);
	LoadBehaviours(pByteCode);
	pByteCode->SetPosition(newPos_00);
	if (((this->actorFieldS & 4) != 0) && (pCVar3 = this->pCollisionData, pCVar3 != (CollisionData*)0x0)) {
		IMPLEMENTATION_GUARD(
		pCVar3->flags_0x0 = pCVar3->flags_0x0 | 0x20000;)
	}
	pCVar1 = this->pCinData;
	fVar12 = pCVar1->floatFieldA;
	fVar11 = pCVar1->floatFieldC;
	fVar13 = pCVar1->floatFieldB;
	this->scale.x = fVar12;
	this->scale.y = fVar13;
	this->scale.z = fVar11;
	this->scale.w = 1.0;
	if (((fVar12 == 1.0) && (fVar13 == 1.0)) && (fVar11 == 1.0)) {
		this->flags = this->flags & 0xfbffffff;
	}
	else {
		this->flags = this->flags | 0x4000000;
	}
	pCVar1 = this->pCinData;
	fVar11 = (pCVar1->vectorThree_0x18).y;
	fVar12 = (pCVar1->vectorThree_0x18).z;
	this->rotationEuler.x = (pCVar1->vectorThree_0x18).x;
	this->rotationEuler.y = fVar11;
	this->rotationEuler.z = fVar12;
	this->field_0x58 = 0;
	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler);
	UpdatePosition(&this->baseLocation, true);
	this->flags = this->flags | 0x80000;
	this->distanceToGround = -1.0f;
	this->flags = this->flags & 0xffdfffff;
	this->field_0xf0 = -1;
	//this->field_0xec = 3.0f;
	return;
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
			IMPLEMENTATION_GUARD_LOG(
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
	CollisionData* pCVar1;
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
		if ((pActorConfig->flags & 0x20) == 0) {
			this->CinematicMode_InterpreteCinMessage(0.0f, 1.0f, 4, 0);
		}
		else {
			this->CinematicMode_InterpreteCinMessage(0.0f, 1.0f, 3, 0);
		}
		if (((pActorConfig->flags & 0x80) != 0) && (pCVar1 = this->pCollisionData, pCVar1 != (CollisionData*)0x0)) {
			IMPLEMENTATION_GUARD(
			pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xfff7efff;)
		}
		if (((pActorConfig->flags & 0x100) != 0) && (this->actorFieldG != (CActor*)0x0)) {
			IMPLEMENTATION_GUARD(
			(*(code*)this->pVTable->field_0xd0)(this, 0, 0, 1, 0);)
		}
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
	edANM_HDR* pDstHdr;
	edANM_HDR* pSrcHdr;
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
		pDstHdr = (edANM_HDR*)LOAD_SECTION(pTag->dstAnim.pHdr);
		pSrcHdr = (edANM_HDR*)LOAD_SECTION(pTag->srcAnim.pHdr);
		if ((pDstHdr == (edANM_HDR*)0x0) || (param_3 != 0)) {
			peVar3 = (pAnimation->anmBinMetaAnimator).aAnimData;
			cVar1 = pTag->srcAnim.field_0x8;
			fVar4 = pTag->srcAnim.field_0x4;
			peVar3->animPlayState = 0;
			peVar3->SetRawAnim(pSrcHdr, (uint)(cVar1 != '\0'), 0xfffffffe);
			edAnmStage::ComputeAnimParams
			(fVar4, (peVar3->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_10, false,
				(uint)(((peVar3->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));
			(peVar3->currentAnimDesc).state.time_0x10 = local_10[0];
			(peVar3->currentAnimDesc).state.time_0xc = local_10[1];
		}
		else {
			peVar3 = (pAnimation->anmBinMetaAnimator).aAnimData;
			cVar1 = pTag->dstAnim.field_0x8;
			fVar5 = pTag->field_0x18;
			cVar2 = pTag->srcAnim.field_0x8;
			fVar4 = pTag->srcAnim.field_0x4;
			fVar6 = pTag->dstAnim.field_0x4;
			peVar3->animPlayState = 0;
			peVar3->SetRawAnim(pDstHdr, (uint)(cVar1 != '\0'), (int)&pDstHdr->flags + 1);
			peVar3->SetRawAnim(pSrcHdr, (uint)(cVar2 != '\0'), (int)&pSrcHdr->flags + 2);
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
	BehaviourList<1>* pComponentList = (BehaviourList<1>*)this->aComponents;
	CBehaviour* pOutBehaviour = (CBehaviour*)0x0;

	for (int i = 0; i < pComponentList->count; i++) {
		if (pComponentList->aComponents[i].id == behaviourId) {
			pOutBehaviour = pComponentList->aComponents[i].GetBehaviour();
		}
	}

	return pOutBehaviour;
}

void CActor::SetupClippingInfo()
{
	this->hierarchySetup.clipping_0x0 = 0x0;
	this->hierarchySetup.pBoundingSphere = &this->subObjA->boundingSphere;
}

void SV_BuildAngleWithOnlyY(edF32VECTOR3* v0, edF32VECTOR3* v1)
{
	float fVar1;
	edF32MATRIX4 m0;

	edF32Matrix4FromEulerSoft(&m0, v1, "XYZ");
	v0->x = 0.0;
	fVar1 = GetAngleYFromVector(&m0.rowZ);
	v0->y = fVar1;
	v0->z = 0.0;
	return;
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
	if ((CActorFactory::gClassProperties[this->typeID].flags & 2) != 0) {
		v0 = &(this->pCinData)->vectorThree_0x18;
		SV_BuildAngleWithOnlyY(v0, v0);
	}
	if ((CActorFactory::gClassProperties[this->typeID].flags & 0x800) == 0) {
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

void CActor::LoadBehaviours(ByteCode* pByteCode)
{
	int iVar1;
	char* pcVar2;
	char* pcVar3;
	char* pcVar4;
	int componentCount;
	BehaviourEntry* pComponentStrm;

	pByteCode->Align(4);

#ifdef PLATFORM_WIN
	static_assert(sizeof(BehaviourEntry) == 8, "BehaviourEntry must be 8 bytes to match serialized data!");
	static_assert(offsetof(BehaviourList<1>, aComponents) == 4, "Components array must be 4 bytes ahead of count!");
#endif

	BehaviourList<1>* pComponentList = (BehaviourList<1>*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)&pComponentList->aComponents;
	if (pComponentList->count != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (pComponentList->count * sizeof(BehaviourEntry));
	}

	this->aComponents = pComponentList;
	componentCount = pComponentList->count;
	if (0 < componentCount) {
		if (((this->flags & 0x2000000) == 0) ||
			((CActorFactory::gClassProperties[this->typeID].flags & 0x40000) == 0)) {
			for (; 0 < componentCount; componentCount = componentCount + -1) {
				BehaviourEntry* pEntry = &pComponentList->aComponents[componentCount - 1];

				int componentStrmLen = pEntry->GetSize();

				CBehaviour* pNewBehaviour = this->BuildBehaviour(pEntry->id);
				pEntry->SetBehaviour(pNewBehaviour);

				pcVar2 = pByteCode->GetPosition();
				if (pEntry->GetBehaviour() != (CBehaviour*)0x0) {
					pEntry->GetBehaviour()->Create(pByteCode);
				}
				pcVar3 = pByteCode->GetPosition();

				int processedLen = pcVar3 - pcVar2;

				if (processedLen != componentStrmLen) {
					pByteCode->SetPosition(pcVar2 + componentStrmLen);
				}
			}
		}
		else {
			for (; 0 < componentCount; componentCount = componentCount + -1) {
				BehaviourEntry* pEntry = &pComponentList->aComponents[componentCount - 1];
				pcVar4 = pByteCode->GetPosition();
				pByteCode->SetPosition(pcVar4 + pEntry->GetSize());
			}
			pComponentList->count = 0;
			(this->subObjA)->mainComponentID_0x0 = -1;
		}
	}
	return;
}

void CActor::SetupModel(int count, MeshTextureHash* aHashes)
{
	CinNamedObject30* pCVar1;
	int index;
	ed_3d_hierarchy_node* peVar2;
	CollisionData* pCollisionData;

	pCVar1 = this->pCinData;
	SV_SetModel(pCVar1->meshIndex, pCVar1->textureIndex, count, aHashes);
	if (this->pMeshTransform == (ed_3d_hierarchy_node*)0x0) {
		peVar2 = (ed_3d_hierarchy_node*)NewPool_edF32MATRIX4(1);
		this->pMeshTransform = peVar2;
	}
	index = (this->pCinData)->collisionDataIndex;
	if (index != -1) {
		IMPLEMENTATION_GUARD_LOG(
		pCollisionData = CCollisionManager::NewCCollision(CScene::ptable.g_CollisionManager_00451690);
		CCollision::Create(pCollisionData, this, index);
		this->pCollisionData = pCollisionData;
		if ((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x200) != 0) {
			CCollision::PatchObbTreeFlagsRecurse(pCollisionData->pCollisionDataSubObjA, 0x200, 0xffffffffffffffff, 0);
		}
		if ((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x8000) != 0) {
			pCollisionData->flags_0x0 = pCollisionData->flags_0x0 | 0x800;
		}
		if ((CActorFactory::gClassProperties[this->typeID].field_0x4 & 0x10000) != 0) {
			pCollisionData->flags_0x0 = pCollisionData->flags_0x0 & 0xfffbffff;
		})
	}
	return;
}

void CActor::SV_SetModel(int meshIndex, int textureIndex, int count, MeshTextureHash* aHashes)
{
	ed_g3d_manager* pMeshInfo;
	ed_g2d_manager* pTVar1;
	ed_g2d_manager* pTVar2;
	ed_g2d_manager* pTextureInfo;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pMeshInfo = (ed_g3d_manager*)0x0;
	pTextureInfo = (ed_g2d_manager*)0x0;
	if (textureIndex == -1) {
		textureIndex = CScene::_pinstance->defaultTextureIndex_0x2c;
	}
	if ((meshIndex != -1) && (textureIndex != -1)) {
		pMeshInfo = pFileManager->GetG3DManager(meshIndex, textureIndex);
		pTVar1 = pFileManager->GetActorsCommonMaterial(textureIndex);
		pTVar2 = pFileManager->GetActorsCommonMeshMaterial(meshIndex);
		if (pTVar2 != pTVar1) {
			pTextureInfo = pTVar1;
		}
	}
	SV_SetModel(pMeshInfo, count, aHashes, pTextureInfo);
	return;
}

void CActor::SV_SetModel(ed_g3d_manager* pMeshInfo, int count, MeshTextureHash* aHashes, ed_g2d_manager* pTextureInfo)
{
	ed_3d_hierarchy_node* pHier;
	ed_g2d_manager* pTextureInfo_00;
	ed_g3d_hierarchy* peVar1;
	edNODE* peVar2;
	int iVar3;
	void* pvVar4;
	ed_hash_code* pHashCode;
	ed_3D_Scene* peVar5;
	void* uVar6;
	ed_hash_code* lVar7;
	ed_hash_code* peVar8;
	int iVar9;
	ulong uVar10;
	int iVar11;

	if (pMeshInfo == (ed_g3d_manager*)0x0) {
		this->pHier = (ed_g3d_hierarchy*)0x0;
		this->pMeshNode = (edNODE*)0x0;
		this->p3DHierNode = (ed_3d_hierarchy_node*)0x0;
		this->pMBNK = 0;
		this->pMeshTransform = (ed_3d_hierarchy_node*)0x0;
	}
	else {
		pTextureInfo_00 = CScene::ptable.g_C3DFileManager_00451664->LoadDefaultTexture_001a65d0();
		if (pTextureInfo_00 != (ed_g2d_manager*)0x0) {
			ed3DLinkG2DToG3D(pMeshInfo, pTextureInfo_00);
		}
		peVar1 = ed3DG3DHierarchyGetFromIndex(pMeshInfo, 0);
		this->pHier = peVar1;
		peVar1 = this->pHier;
		if (peVar1 != (ed_g3d_hierarchy*)0x0) {
			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x1000) != 0) {
				if (1 < peVar1->lodCount) {
					peVar1->flags_0x9e = peVar1->flags_0x9e | 0x100;
				}
				ed3DG3DHierarchySetStripShadowCastFlag(this->pHier, 0xffff);
			}
			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x2000) != 0) {
				IMPLEMENTATION_GUARD_LOG(
				ed3DG3DHierarchySetStripShadowReceiveFlag(this->pHier, 0xffff);)
			}
		}
		peVar2 = ed3DHierarchyAddToScene(CScene::_scene_handleA, pMeshInfo, (char*)0x0);
		this->pMeshNode = peVar2;
		this->p3DHierNode = (ed_3d_hierarchy_node*)(this->pMeshNode)->pData;
		this->pMeshTransform = this->p3DHierNode;
		peVar2 = this->pMeshNode;
		if (peVar2 != (edNODE*)0x0) {
			ed3DSetMeshTransformFlag_002abd80(peVar2, 0xffff);
			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x1000) != 0) {
				ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, 1);
			}
			if ((CActorFactory::gClassProperties[this->typeID].flags & 0x2000) != 0) {
				ed3DSetMeshTransformFlag_002abff0(this->pMeshNode, 0xffff);
			}
		}
		if ((((CActorFactory::gClassProperties[this->typeID].flags & 0x4000) == 0) &&
			(pTextureInfo == (ed_g2d_manager*)0x0)) && (count == 0)) {
			this->pMBNK = 0;
		}
		else {
			iVar3 = ed3DHierarchyBankMatGetSize((ed_3d_hierarchy*)this->p3DHierNode);
			pvVar4 = edMemAlloc(TO_HEAP(H_MAIN), iVar3);
			uVar6 = ed3DHierarchyBankMatInstanciate((ed_3d_hierarchy*)this->p3DHierNode, pvVar4);
			this->pMBNK = uVar6;
		}
		if (pTextureInfo != (ed_g2d_manager*)0x0) {
			IMPLEMENTATION_GUARD(
			ed3DHierarchyBankMatLinkG2D(this->p3DHierNode, pTextureInfo);
			ed3DHierarchyBankMatLinkG2D(this->p3DHierNode, pTextureInfo_00);)
		}
		if (((count != 0) && (pHier = this->p3DHierNode, pHier != (ed_3d_hierarchy_node*)0x0)) &&
			(pTextureInfo_00 != (ed_g2d_manager*)0x0)) {
			pHashCode = ed3DHierarchyGetMaterialBank((ed_3d_hierarchy*)pHier);
			iVar3 = ed3DG2DGetG2DNbMaterials(pHashCode);
			iVar11 = 0;
			if (0 < count) {
				do {
					uVar10 = aHashes->meshHash;
					lVar7 = ed3DG2DGetMaterial(pTextureInfo_00, aHashes->textureHash);
					if ((lVar7 != (ed_hash_code*)0x0) && (iVar9 = 0, peVar8 = pHashCode, 0 < iVar3)) {
						do {
							if (uVar10 == peVar8->hash.number) {
								pHashCode[iVar9].pData = STORE_SECTION(lVar7);
								break;
							}
							iVar9 = iVar9 + 1;
							peVar8 = peVar8 + 1;
						} while (iVar9 < iVar3);
					}
					iVar11 = iVar11 + 1;
					aHashes = aHashes + 1;
				} while (iVar11 < count);
			}
		}
		memset(&this->hierarchySetup, 0, sizeof(ed_3d_hierarchy_setup));
		if (1 < ((this->p3DHierNode)->base).lodCount) {
			this->hierarchySetup.pNext = (ed_3d_hierarchy_setup*)&this->field_0xcc;
		}
		ed3DHierarchySetSetup(&this->p3DHierNode->base, &this->hierarchySetup);
		((this->p3DHierNode)->base).pAnimMatrix = (edF32MATRIX4*)0x0;
		((this->p3DHierNode)->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;
		ed3DHierarchyNodeGetSkeletonChunck(this->pMeshNode, false);
		if ((this->flags & 0x4000) == 0) {
			peVar5 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOn(peVar5, this->pMeshNode);
		}
		else {
			peVar5 = GetStaticMeshMasterA_001031b0();
			ed3DHierarchyNodeSetRenderOff(peVar5, this->pMeshNode);
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

int INT_00448e04 = 0;

void CActorMovable::Create(ByteCode* pByteCode)
{
	float fVar1;

	INT_00448e04 = INT_00448e04 + 1;
	CActor::Create(pByteCode);
	//CDynamic::Reset(&this->dynamic, (CActor*)this);
	fVar1 = pByteCode->GetF32();
	//(this->dynamic).field_0x54 = fVar1;
	this->field_0x1c0 = 1e+30;
	return;
}

int INT_00448e08 = 0;

void CActorAutonomous::Create(ByteCode* pByteCode)
{
	float fVar1;
	float fVar2;
	edF32MATRIX4* peVar3;
	uint uVar4;
	long lVar5;
	ByteCode* pBVar6;
	float fVar7;

	INT_00448e08 = INT_00448e08 + 1;
	pBVar6 = pByteCode;
	CActorMovable::Create(pByteCode);
	//CDynamicExt::ClearLocalData(&this->dynamicExt);
	//lVar5 = (*(code*)(this->base).base.pVTable[1].free)(this);
	//if (lVar5 != 0) {
	//	peVar3 = (edF32MATRIX4*)(*(code*)(this->base).base.pVTable[1].free)(this);
	//	FUN_0019e4a0(peVar3);
	//}
	//this->field_0x2cc = 0;
	//this->field_0x2c8 = 0;
	if (this->pCollisionData != (CollisionData*)0x0) {
		IMPLEMENTATION_GUARD(
		(*(this->base).base.pVTable[1].IsKindOfObject)((CActor*)this, (int)pBVar6);)
	}
	uVar4 = pByteCode->GetU32();
	//CLifeInterface::SetValueMax((float)uVar4, &this->field_0x2d0);
	fVar7 = pByteCode->GetF32();
	//(this->dynamicExt).base.rotationQuat.x = fVar7;
	fVar7 = pByteCode->GetF32();
	//(this->dynamicExt).base.rotationQuat.y = fVar7;
	fVar7 = pByteCode->GetF32();
	//(this->dynamicExt).base.rotationQuat.z = fVar7;
	//fVar2 = gF32Vector4Zero.w;
	//fVar1 = gF32Vector4Zero.z;
	//fVar7 = gF32Vector4Zero.y;
	//*(float*)&this->field_0x2f0 = gF32Vector4Zero.x;
	//*(float*)&this->field_0x2f4 = fVar7;
	//*(float*)&this->field_0x2f8 = fVar1;
	//this->field_0x2fc = fVar2;
	//this->field_0x308 = 3.141593;
	//this->field_0x318 = -1.047198;
	//this->field_0x31c = 1.047198;
	//this->field_0x314 = 0.0;
	//this->field_0x324 = -1.047198;
	//this->field_0x328 = 1.047198;
	//this->field_0x320 = 0.0;
	//this->field_0x32c = 3396.333;
	//this->field_0x330 = 3396.333;
	return;
}

void CAddOnGenerator::Create(CActor* pActor, ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	int iVar3;
	int* piVar4;
	void* pvVar5;
	int iVar6;
	int iVar7;

	//iVar6 = (pActor->data).objectId;
	uVar1 = pByteCode->GetU32();
	//this->field_0x0 = (ushort)uVar1 & 3;
	uVar1 = pByteCode->GetU32();
	//this->field_0x2 = (ushort)uVar1;
	iVar2 = pByteCode->GetS32();
	//this->field_0x4 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->currentOrbs_0x8 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0xc = (float)iVar2 * 0.01;
	this->subObj.Create(pByteCode);
	//this->field_0x28 = 0;
	//this->maxOrbs_0x2c = 0;
	//this->field_0x30 = 0;
	//if (_gAddOn_sectors == (void*)0x0) {
	//	iVar7 = LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->currentLevelID].sectorCount_0x14 + 1;
	//	_gAddOn_sectors = operator.new.array((long)(iVar7 * 0x10));
	//	iVar2 = 0;
	//	if (0 < iVar7) {
	//		iVar3 = 0;
	//		do {
	//			iVar2 = iVar2 + 1;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3) = 0;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3 + 4) = 0;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3 + 8) = 0;
	//			*(undefined4*)((int)_gAddOn_sectors + iVar3 + 0xc) = 0;
	//			iVar3 = iVar3 + 0x10;
	//		} while (iVar2 < iVar7);
	//	}
	//}
	//_gAddOn_NbTotalAddOn = _gAddOn_NbTotalAddOn + 1;
	//(&_gAddOn_NbTotalMoneyInLevel)[(uint)this->field_0x2 * 4] =
	//	(&_gAddOn_NbTotalMoneyInLevel)[(uint)this->field_0x2 * 4] + this->field_0x4;
	//_gAddOn_NbTotalBonusInLevel = _gAddOn_NbTotalBonusInLevel + this->currentOrbs_0x8;
	//if (iVar6 == -1) {
	//	iVar6 = 0;
	//}
	//piVar4 = (int*)((int)_gAddOn_sectors + (uint)this->field_0x2 * 4 + iVar6 * 0x10);
	//*piVar4 = *piVar4 + this->field_0x4;
	//pvVar5 = (void*)(iVar6 * 0x10 + (int)_gAddOn_sectors);
	//*(int*)((int)pvVar5 + 0xc) = *(int*)((int)pvVar5 + 0xc) + this->currentOrbs_0x8;
	return;
}

void CAddOnGenerator_SubObj::Create(ByteCode* pByteCode)
{
	uint uVar1;
	float fVar2;

	uVar1 = pByteCode->GetU32();
	if (uVar1 == 1) {
		fVar2 = pByteCode->GetF32();
		this->field_0x0 = fVar2 * 0.01745329f;
		fVar2 = pByteCode->GetF32();
		this->field_0x4 = fVar2 * 0.01745329f;
		this->field_0x8 = 0.0;
		fVar2 = pByteCode->GetF32();
		this->field_0xc = fVar2 * 0.01745329f;
		fVar2 = pByteCode->GetF32();
		this->field_0x10 = fVar2;
		fVar2 = pByteCode->GetF32();
		this->field_0x14 = fVar2;
	}
	else {
		this->field_0x0 = 0.7853982f;
		this->field_0x4 = 0.3926991f;
		this->field_0x8 = 0.0f;
		this->field_0xc = 3.141593f;
		this->field_0x10 = 3.0f;
		this->field_0x14 = 2.5f;
	}
	return;
}

bool CActor::CinematicMode_InterpreteCinMessage(float, float, int param_2, int param_3)
{
	bool bSuccess;
	CBehaviourCinematic* pBehaviourCinematic;

	bSuccess = false;
	pBehaviourCinematic = (CBehaviourCinematic*)GetBehaviour(1);
	if (pBehaviourCinematic != (CBehaviourCinematic*)0x0) {
		bSuccess = pBehaviourCinematic->CinematicMode_InterpreteCinMessage(param_2, param_3);
	}
	return bSuccess;
}

void CActor::SkipToNextActor(ByteCode* pByteCode) 
{
	ACTOR_LOG(LogLevel::Info, "CActor::Create SKIPPED");
	CActor::Create(pByteCode);

	char* pCurrent = pByteCode->currentSeekPos;

	while (true) {
		if (strncmp(pCurrent, "TSNI", 4) == 0) {
			break;
		}
		pCurrent++;
	}

	pByteCode->currentSeekPos = pCurrent;
}

CBehaviour* CActor::BuildBehaviour(int behaviourType)
{
	CBehaviour* pCVar1;

	if (behaviourType == 1) {
		pCVar1 = (CBehaviour*)NewPool_CBehaviourCinematic(1);
	}
	else {
		if (behaviourType == 0) {
			pCVar1 = &this->standBehaviour;
		}
		else {
			pCVar1 = (CBehaviour*)0x0;
		}
	}
	return pCVar1;
}

void CBehaviourStand::Init(CActor* pOwner)
{
	this->pOwner = pOwner;
}

bool CBehaviourStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
	return true;
}

bool CActor::SV_PatchMaterial(ulong originalHash, ulong newHash, ed_g2d_manager* pMaterial)
{
	int index;
	bool bVar1;
	ed_hash_code* pHashCode;
	ed_hash_code* peVar2;
	int iVar3;
	ed_g2d_manager* pTextureInfo;
	int iVar4;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	if (pMaterial == (ed_g2d_manager*)0x0) {
		pMaterial = pFileManager->LoadDefaultTexture_001a65d0();
	}
	pHashCode = ed3DHierarchyGetMaterialBank((ed_3d_hierarchy*)this->p3DHierNode);
	peVar2 = ed3DG2DGetMaterial(pMaterial, newHash);
	iVar3 = ed3DG2DGetG2DNbMaterials(pHashCode);
	iVar4 = 0;
	if ((peVar2 == (ed_hash_code*)0x0) && (index = (this->pCinData)->textureIndex, index != -1)) {
		pTextureInfo = pFileManager->GetActorsCommonMaterial(index);
		peVar2 = ed3DG2DGetMaterial(pTextureInfo, newHash);
	}
	for (; (pHashCode->hash.number != originalHash && (iVar4 < iVar3)); iVar4 = iVar4 + 1) {
		pHashCode = pHashCode + 1;
	}
	if ((iVar4 == iVar3) || (peVar2 == (ed_hash_code*)0x0)) {
		bVar1 = false;
	}
	else {
		pHashCode->pData = STORE_SECTION(peVar2);
		bVar1 = true;
	}
	return bVar1;
}