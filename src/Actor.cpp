#include "Actor.h"
#include "LargeObject.h"
#include "SectorManager.h"
#include "CinematicManager.h"
#include "ActorFactory.h"
#include "MathOps.h"
#include "CameraViewManager.h"

CActor::CActor()
	: CObject()
{
	float fVar1;
	float fVar2;
	//this->field_0x134 = -1.0f;
	//this->field_0x13c = 0;
	//this->field_0x140 = 0;
	//this->field_0x144 = 0;
	//this->field_0x148 = 0;
	//this->funcData_0x150 = (undefined*)&CBehaviour::_vt;
	//this->funcData_0x150 = (undefined*)&CBehaviourStand::_vt;
	//this->index_0x20 = -1;
	this->typeID = -1;
	this->pCinData = (CinNamedObject30*)0x0;
	this->subObjA = (KyaUpdateObjA*)0x0;
	this->flags = 0;
	//this->currentLocation.x = 0.0;
	//this->currentLocation.y = 0.0;
	//this->currentLocation.z = 0.0;
	//this->currentLocation.w = 1.0;
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
	//this->anotherSubObj = (astruct_1*)0x0;
	this->adjustedMagnitude = 0.0;
	//this->pAnimationController = (CAnimation*)0x0;
	//this->pCollisionData = (CollisionData*)0x0;
	//this->pShadow = (CShadow*)0x0;
	//this->actorFieldG = (Actor*)0x0;
	this->aComponents = (int*)0x0;
	this->curBehaviourId = -1;
	this->prevBehaviourId = -1;
	//this->actorState = AS_None;
	//this->prevActorState = AS_None;
	//this->timeA = 0.0;
	//this->idleTimer = 0.0;
	//this->field_0x110 = (undefined*)0x0;
	this->currentAnimType = -1;
	//this->prevAnimType = AT_None;
	//this->field_0xdc = 0;
	//this->field_0xe0 = 1.0;
	this->field_0xd = '\0';
	//this->field_0xc = '\0';
	//this->distanceToGround = (float)&DAT_bf800000;
	//this->field_0xec = 3.0;
	//this->field_0xf0 = -1;
	fVar2 = gF32Vector3Zero.z;
	fVar1 = gF32Vector3Zero.y;
	//this->vector_0x120.x = gF32Vector3Zero.x;
	//this->vector_0x120.y = fVar1;
	//this->vector_0x120.z = fVar2;
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
	//CShadow* pGVar1;
	
	//if (this->pCollisionData != (CollisionData*)0x0) {
	//	CCollision::Init();
	//}
	//pGVar1 = this->pGroundObj;
	//if (pGVar1 != (CShadow*)0x0) {
	//	CShadow::Init(pGVar1, this->field_0x0);
	//}
	this->flags = this->flags & 0xfffffffc;
	this->flags = this->flags & 0xffffff5f;
	EvaluateDisplayState();
	this->flags = this->flags & 0xfffff7ff;
	this->flags = this->flags & 0xffbfffff;
	if ((((this->actorFieldS & 1) != 0) && (this != (CActor*)0x0)) && ((this->flags & 0x2000000) == 0)) {
		IMPLEMENTATION_GUARD_LOG(
		ReceiveMessage(this, 0x5d, 0);)
	}
	ComponentList<0>* pComponentList = (ComponentList<0>*)this->aComponents;
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

	iVar1 = this->field_0x0;
	if ((iVar1 == ((Scene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID) || (iVar1 == -1)) {
		uVar2 = this->flags;
		if ((uVar2 & 0x2000060) == 0) {
			if ((uVar2 & 0x200) == 0) {
				uVar2 = 1;
			}
			else {
				uVar2 = uVar2 & 0x80 | (uint)('\0' < this->field_0xd);
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

void CActor::ChangeDisplayState(int state)
{
	//CShadow* pCVar1;
	uint ret;
	float BfloatB;

	if (state == 0) {
		//pCVar1 = this->pShadow;
		//if (pCVar1 != (CShadow*)0x0) {
		//	(**(code**)(*(int*)pCVar1 + 0x14))();
		//}
		this->flags = this->flags & 0xfffffeff;
	}
	else {
		//pCVar1 = this->pShadow;
		//if (pCVar1 != (CShadow*)0x0) {
		//	(**(code**)(*(int*)pCVar1 + 0x14))();
		//}
		this->flags = this->flags | 0x100;
	}
	ret = this->flags;
	BfloatB = (this->subObjA)->floatFieldB;
	if (((ret & 0x100) == 0) || (BfloatB < this->adjustedMagnitude)) {
		if ((ret & 0x4000) != 0) {
			IMPLEMENTATION_GUARD(
			this->ChangeVisibleState(0);)
		}
	}
	else {
		bool bSphereVisible = CCameraManager::_gThis->IsSphereVisible(BfloatB, &this->sphereCentre);
		if (bSphereVisible == 0) {
			if ((this->flags & 0x4000) != 0) {
				IMPLEMENTATION_GUARD(
				this->ChangeVisibleState(0);)
			}
		}
		else {
			if ((this->flags & 0x4000) == 0) {
				IMPLEMENTATION_GUARD(
				this->ChangeVisibleState(1);)
			}
		}
	}
	return;
}

void CActor::SetState(int newState, int AnimationType)
{

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

void CActor::CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int)
{

}

CBehaviour* CActor::GetBehaviour(int behaviourId)
{
	ComponentList<0>* pComponentList = (ComponentList<0>*)this->aComponents;
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

void CBehaviour::GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites)
{
	*nbVertex = 0;
	*nbSprites = 0;
	return;
}
