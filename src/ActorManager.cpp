#include "ActorManager.h"
#include "MemoryStream.h"
#include "CameraViewManager.h"

void CActorManager::Level_AddAll(ByteCode* pMemoryStream)
{
	//ManagerFunctionData* pMVar1;
	undefined* puVar2;
	int actorCount;
	void* papAVar3;
	int* pAlloc;
	int* piVar4;
	AnimationController* pAVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	//ManagerFunctionData** ppMVar9;
	uint uVar10;
	int iVar11;
	Actor* pKVar12;
	int iVar12;
	uint uVar13;
	Actor* pActorBase;
	Actor* iVar2;

	actorCount = pMemoryStream->GetS32();
	this->actorCount_0x58 = actorCount;
	if (this->actorCount_0x58 < 1) {
		this->aActors = (CActor**)0x0;
		this->componentsToUpdate = (void**)0x0;
		this->elementArrayStart = (void**)0x0;
	}
	else {
		papAVar3 = (new void*)[this->actorCount_0x58 * 3];
		this->aActors = (CActor**)papAVar3;
		this->componentsToUpdate = (void**)(this->aActors + this->actorCount_0x58);
		this->elementArrayStart = (void**)(this->componentsToUpdate + this->actorCount_0x58);
	}
	for (; 0 < actorCount; actorCount = actorCount + -1) {
		IMPLEMENTATION_GUARD(
		ByteCode::GetChunk(pMemoryStream);
		iVar11 = ByteCode::GetS32(pMemoryStream);
		uVar13 = ByteCode::GetU32(pMemoryStream);
		/* Get the type data from the db. */
		ppMVar9 = &this->pVTable + uVar13 * 4;
		pMVar1 = ppMVar9[0x26];
		ppMVar9[0x26] = (ManagerFunctionData*)((int)&pMVar1->field_0x0 + 1);
		pKVar12 = (Actor*)((int)&ppMVar9[0x24]->field_0x0 + (int)pMVar1 * (int)ppMVar9[0x27]);
		(pKVar12->actorBase).data.index_0x20 = iVar11;
		(pKVar12->actorBase).data.typeID = uVar13;
		(*(code*)((pKVar12->actorBase).pVTable)->deserializeFunc)();
		(*this->aActors)[iVar11] = (Actor*)pKVar12;
		)
	}
	if (this->actorCount_0x58 != 0) {
		IMPLEMENTATION_GUARD(
		edMemSetFlags(H_MAIN, 0x100);
		pAlloc = (int*)operator.new.array((long)(this->actorCount_0x58 << 2));
		edMemClearFlags(H_MAIN, 0x100);
		actorCount = 0;
		uVar13 = 0;
		iVar12 = 0;
		iVar11 = 0;
		if (0 < this->actorCount_0x58) {
			iVar6 = 0;
			piVar4 = pAlloc;
			do {
				iVar2 = *(Actor**)((int)*this->aActors + iVar6);
				if ((iVar2->data).field_0x110 == (undefined*)0x0) {
					*piVar4 = 0;
				}
				else {
					actorCount = actorCount + 1;
					iVar7 = 0;
					iVar8 = 0;
					uVar10 = 1;
					do {
						if ((((iVar2->data).subObjA)->count_0x3c & uVar10) != 0) {
							iVar7 = iVar7 + 1;
						}
						iVar8 = iVar8 + 1;
						uVar10 = uVar10 << 1;
					} while (iVar8 < 4);
					*piVar4 = iVar7;
					uVar13 = uVar13 + iVar7;
				}
				if (*(int*)&((iVar2->data).subObjA)->field_0x40 != 0) {
					iVar12 = iVar12 + 1;
				}
				iVar11 = iVar11 + 1;
				iVar6 = iVar6 + 4;
				piVar4 = piVar4 + 1;
			} while (iVar11 < this->actorCount_0x58);
		}
		if (actorCount != 0) {
			this->count_0x70 = 0;
			this->field_0x74 = actorCount;
			uVar10 = this->field_0x74;
			piVar4 = (int*)operator.new.array((long)(int)(uVar10 * 0x58 + 0x10));
			pAVar5 = (AnimationController*)
				__construct_new_array
				(piVar4, AnimationController::Constructor_00180ea0, AnimationController::Free_00180c10, 0x58,
					uVar10);
			this->pAnimationControllerArray_0x6c = pAVar5;
			this->field_0x7c = 0;
			this->field_0x80 = uVar13;
			piVar4 = (int*)operator.new.array((long)(int)(uVar13 * 0xd8 + 0x10));
			piVar4 = __construct_new_array(piVar4, (ActorConstructorA*)&LAB_00107a60, (ActorConstructorB*)0x0, 0xd8, uVar13);
			this->field_0x78 = (undefined*)piVar4;
		}
		if (iVar12 != 0) {
			this->field_0x88 = 0;
			this->field_0x8c = iVar12;
			uVar13 = this->field_0x8c;
			piVar4 = (int*)operator.new.array((long)(int)(uVar13 * 0x60 + 0x10));
			piVar4 = __construct_new_array(piVar4, FUN_00113b20, FUN_001074d0, 0x60, uVar13);
			this->pActor_0x84 = (undefined*)piVar4;
		}
		actorCount = 0;
		if (0 < this->actorCount_0x58) {
			iVar11 = 0;
			piVar4 = pAlloc;
			do {
				iVar12 = *piVar4;
				pActorBase = *(Actor**)((int)*this->aActors + iVar11);
				if (iVar12 != 0) {
					iVar6 = this->field_0x7c;
					puVar2 = this->field_0x78;
					this->field_0x7c = iVar6 + iVar12;
					iVar7 = this->count_0x70;
					this->count_0x70 = iVar7 + 1;
					pAVar5 = this->pAnimationControllerArray_0x6c;
					(pActorBase->data).pAnimationController = pAVar5 + iVar7;
					AnimationController::Setup_00180c60
					(pAVar5 + iVar7, pActorBase, ((pActorBase->data).subObjA)->count_0x3c,
						(AnimationSubObj*)(puVar2 + iVar6 * 0xd8), iVar12);
				}
				if (*(int*)&((pActorBase->data).subObjA)->field_0x40 == 0) {
					Actor::SetupFunc_00105040(pActorBase, (GroundObject*)0x0);
				}
				else {
					iVar12 = this->field_0x88;
					this->field_0x88 = iVar12 + 1;
					Actor::SetupFunc_00105040(pActorBase, (GroundObject*)(this->pActor_0x84 + iVar12 * 0x60));
				}
				actorCount = actorCount + 1;
				iVar11 = iVar11 + 4;
				piVar4 = piVar4 + 1;
			} while (actorCount < this->actorCount_0x58);
		}
		edMemFree(pAlloc);)
	}
	return;
}

void CActorManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	int iVar8;
	CActor* pCVar9;
	CActor* pActor;
	Actor* pOtherActor;

	if (oldSectorId != -1) {
		pCVar9 = (CActor*)this->elementArrayStart;
		for (iVar8 = this->numElements; 0 < iVar8; iVar8 = iVar8 + -1) {
			IMPLEMENTATION_GUARD(
			pActor = *(CActor**)pCVar9;
			if (oldSectorId == (pActor->data).field_0x0) {
				CActor::EvaluateManageState(pActor);
				CActor::EvaluateDisplayState(pActor);
			}
			pCVar9 = (CActor*)((int)pCVar9 + 4);)
		}
	}
	if (newSectorId == -1) {
		IMPLEMENTATION_GUARD(
		SetWorldBox((int)&this->kyaChild, (undefined8*)this->pActorManagerSectorArray, 0);)
	}
	else {
		IMPLEMENTATION_GUARD_LOG(
		SetWorldBox((int)&this->kyaChild, (undefined8*)(this->pActorManagerSectorArray + newSectorId),
			(long)*(int*)((Scene::ptable.g_SectorManager_00451670)->subObjArray[newSectorId].pFileData + 8) & 1);)
	}
	fVar5 = (CCameraManager::_gThis->transformationMatrix).da;
	fVar6 = (CCameraManager::_gThis->transformationMatrix).db;
	fVar7 = (CCameraManager::_gThis->transformationMatrix).dc;
	this->numElements = 0;
	pCVar9 = (CActor*)this->aActors;
	for (iVar8 = this->actorCount_0x58; 0 < iVar8; iVar8 = iVar8 + -1) {
		IMPLEMENTATION_GUARD(
		pOtherActor = *(Actor**)pCVar9;
		iVar1 = (pOtherActor->actorBase).data.field_0x0;
		if ((iVar1 == newSectorId) || (iVar1 == -1)) {
			if (iVar1 == newSectorId) {
				fVar2 = (pOtherActor->actorBase).data.sphereCentre.x - fVar5;
				fVar3 = (pOtherActor->actorBase).data.sphereCentre.y - fVar6;
				fVar4 = (pOtherActor->actorBase).data.sphereCentre.z - fVar7;
				(pOtherActor->actorBase).data.adjustedMagnitude =
					SQRT(fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4) - (pOtherActor->actorBase).data.sphereCentre.w;
			}
			CActor::EvaluateManageState((CActor*)pOtherActor);
			CActor::EvaluateDisplayState((CActor*)pOtherActor);
			this->elementArrayStart[this->numElements] = pOtherActor;
			this->numElements = this->numElements + 1;
			CActor::UpdateClusterNode((CActor*)pOtherActor);
		}
		pCVar9 = (CActor*)((int)pCVar9 + 4);)
	}
	for (iVar8 = 0; iVar8 < this->actorCount_0x58; iVar8 = iVar8 + 1) {
		IMPLEMENTATION_GUARD(
		pCVar9 = this->aActors[iVar8];
		iVar1 = (pCVar9->data).field_0x0;
		if ((newSectorId == iVar1) || (oldSectorId == iVar1)) {
			(*(code*)pCVar9->pVTable->field_0x3c)(pCVar9, oldSectorId, newSectorId);
		})
	}
	return;
}
