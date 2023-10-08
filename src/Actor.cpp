#include "Actor.h"
#include "LargeObject.h"
#include "SectorManager.h"

CActor::CActor()
{
	this->aComponents = (ComponentEntry*)0x0;
	this->actorFieldS = 0;
	this->typeID = -1;
	this->adjustedMagnitude = 0.0f;
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
		IMPLEMENTATION_GUARD(
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
	if ((iVar1 == ((Scene::ptable.g_SectorManager_00451670)->baseSector).field_0x0) || (iVar1 == -1)) {
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

void CActor::Init()
{
	uint* puVar1;

	///* While this pointer is invalid */
	//for (puVar1 = (uint*)this->field_0x144; puVar1 != (uint*)0x0; puVar1 = (uint*)puVar1[0xe]) {
	//	CActorSound::Init(puVar1);
	//}
	//(*(code*)this->pVTable->setComponent_0x78)
	//	(this, (this->subObjA)->mainComponentID_0x0, 0xffffffffffffffff, 0xffffffffffffffff);
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
	//BfloatB = (this->subObjA)->floatFieldB;
	//if (((ret & 0x100) == 0) || (BfloatB < this->adjustedMagnitude)) {
	//	if ((ret & 0x4000) != 0) {
	//		this->ChangeVisibleState(0);
	//	}
	//}
	//else {
	//	ret = CCameraManager::IsSphereVisible(BfloatB, CCameraManager::_gThis, &this->sphereCentre);
	//	if (ret == 0) {
	//		if ((this->flags & 0x4000) != 0) {
	//			this->ChangeVisibleState(0);
	//		}
	//	}
	//	else {
	//		if ((this->flags & 0x4000) == 0) {
	//			this->ChangeVisibleState(1);
	//		}
	//	}
	//}
	return;
}

void CActor::SetState(int newState, int AnimationType)
{

}

void CActor::CinematicMode_Enter(bool param_2)
{

}

void CActor::CinematicMode_SetAnimation(edCinActorInterface::ANIM_PARAMStag* const pTag, int)
{

}

void CBehaviour::GetDlistPatchableNbVertexAndSprites(int* nbVertex, int* nbSprites)
{
	*nbVertex = 0;
	*nbSprites = 0;
	return;
}
