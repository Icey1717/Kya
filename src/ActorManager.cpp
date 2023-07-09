#include "ActorManager.h"
#include "MemoryStream.h"

void CActorManager::Level_AddAll(ByteCode* pMemoryStream)
{
	//ManagerFunctionData* pMVar1;
	undefined* puVar2;
	int actorCount;
	Actor* (*papAVar3)[187];
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
		//this->actorArray_0x54 = (Actor * (*)[187])0x0;
		//this->componentsToUpdate = (Actor**)0x0;
		//this->elementArrayStart = (Actor**)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(
		papAVar3 = (Actor * (*)[187])operator.new.array((long)(this->actorCount_0x58 * 0xc));
		this->actorArray_0x54 = papAVar3;
		this->componentsToUpdate = (Actor**)(*this->actorArray_0x54 + this->actorCount_0x58);
		this->elementArrayStart = (Actor**)(this->componentsToUpdate + this->actorCount_0x58);)
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
		(*this->actorArray_0x54)[iVar11] = (Actor*)pKVar12;
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
				iVar2 = *(Actor**)((int)*this->actorArray_0x54 + iVar6);
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
				pActorBase = *(Actor**)((int)*this->actorArray_0x54 + iVar11);
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
