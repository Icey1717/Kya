#include "ActorHero_Inventory.h"
#include "LargeObject.h"
#include "InventoryInfo.h"
#include "ActorHero.h"
#include "ActorManager.h"
#include "FrontendDisp.h"

CInventoryInterface::CInventoryInterface()
{
	this->bActive = 0;
	this->field_0x8 = 0;
	this->bDirty = 0;

	Clear();

	return;
}

// Should be in: D:/Projects/b-witch/Frontend.cpp
bool CInventoryInterface::Activate(int bActive)
{
	if (bActive == 0) {
		this->bActive = 0;
	}
	else {
		this->bActive = 1;
		this->aHeaderInfo[0].activeItemIndex = 0;
		this->aHeaderInfo[1].activeItemIndex = 0;
	}

	return true;
}

bool CInventoryInterface::CanActivate()
{
	return this->bActive == 0;
}

bool CInventoryInterface::IsActive()
{
	return this->bActive;
}

struct InvTransTable
{
	int itemId;
	int actorClass;
	int headerSlot;
};

InvTransTable _InvTransTable[12] = {
	{ 0x15, PROJECTILE, 0x0 },
	{ 0x16, PROJECTILE, 0x0 },
	{ 0x17, PROJECTILE, 0x0 },
	{ 0x18, PROJECTILE, 0x0 },
	{ 0x19, PROJECTILE, 0x0 },
	{ 0x1a, PROJECTILE, 0x0 },
	{ 0x1b, PROJECTILE, 0x0 },
	{ 0x1c, PROJECTILE, 0x0 },

	{ 0x1d, FRUIT, 0x1 },
	{ 0x1e, FRUIT, 0x1 },
	{ 0x1f, FRUIT, 0x1 },

	{ -1, -1, -1 }
};

InvTransTable* GetInvTransTable(int ItemId)
{
	InvTransTable* pIVar1;

	pIVar1 = _InvTransTable;
	if (_InvTransTable[0].headerSlot != -1) {
		do {
			if (pIVar1->itemId == ItemId) {
				return pIVar1;
			}

			pIVar1 = pIVar1 + 1;
		} while (pIVar1->headerSlot != -1);
	}

	return (InvTransTable*)0x0;
}

InventorySlot* CInventoryInterface::GetExistingInventorySlot(int headerSlot, int ItemId)
{
	int slotIndex = 0;
	InventorySlot* pSlot = this->aSlots[headerSlot];
	do {
		if (pSlot->itemId == ItemId) {
			return pSlot;
		}

		slotIndex = slotIndex + 1;
		pSlot = pSlot + 1;
	} while (slotIndex < 0x10);

	return (InventorySlot*)0x0;
}

InventorySlot* CInventoryInterface::GetFreeSlot(int headerSlot)
{
	InventorySlot* pSlot = (InventorySlot*)0x0;

	if (this->aHeaderInfo[headerSlot].nbUsedSlots < 0x10) {
		pSlot = this->aHeaderInfo[headerSlot].pSlot;
	}

	return pSlot;
}

struct _msg_5e_param
{
	int field_0x0;
	int field_0x4;
};

bool Criterion_FindTransitItem(CActor* pActor, CActor* param_2)
{
	_msg_5e_param local_30;

	local_30.field_0x0 = 0;
	local_30.field_0x4 = -1;
	param_2->DoMessage(param_2, (ACTOR_MESSAGE)0x5e, &local_30);

	return local_30.field_0x4 == 3; // State Transit
}

bool Criterion_FindReadyItem(CActor* pActor, CActor* param_2)
{
	_msg_5e_param local_30;

	local_30.field_0x0 = 0;
	local_30.field_0x4 = -1;
	param_2->DoMessage(param_2, (ACTOR_MESSAGE)0x5e, &local_30);

	return local_30.field_0x4 == 2; // State Ready
}

CActor* CInventoryInterface::FindActorInState(FE_Position* pPosition, int mode)
{
	int iVar1;
	CActorManager* pActorManager;
	CActor* pFound;
	CInventoryInfo* pCVar2;
	CriterionActorFunc* pFunc;
	InvTransTable* pTransTable;
	CActorsTable local_110;

	pActorManager = static_cast<CActorManager*>(CScene::GetManager(MO_Actor));
	local_110.nbEntries = 0;
	pFunc = (CriterionActorFunc*)0x0;
	pTransTable = GetInvTransTable(this->aSlots[pPosition->headerSlot][pPosition->itemSlot].itemId);
	if (pTransTable == (InvTransTable*)0x0) {
	LAB_001dbe88:
		pFound = (CActor*)0x0;
	}
	else {
		if (mode == 3) {
			pFunc = Criterion_FindTransitItem;
		}
		else {
			if (mode == 2) {
				pFunc = Criterion_FindReadyItem;
			}
		}

		pActorManager->GetClassActorsWithCriterion(&local_110, pTransTable->actorClass, (CActor*)0x0, pFunc);

		do {
			if (local_110.nbEntries == 0) goto LAB_001dbe88;
			pFound = local_110.PopCurrent();
			pCVar2 = pFound->GetInventoryInfo();
		} while (pTransTable->itemId != pCVar2->purchaseId);
	}

	return pFound;
}

CActor* CInventoryInterface::FindActorForCell(FE_Position* pPosition)
{
	CActorManager* pActorManager;
	CInventoryInfo* pInventoryInfo;
	int actorIndex;
	InvTransTable* pTransTable;
	CActor* pActor;

	pActorManager = static_cast<CActorManager*>(CScene::GetManager(MO_Actor));
	pTransTable = GetInvTransTable(this->aSlots[pPosition->headerSlot][pPosition->itemSlot].itemId);
	
	if (pTransTable != (InvTransTable*)0x0) {
		actorIndex = 0;
		if (0 < pActorManager->aClassInfo[pTransTable->actorClass].totalCount) {
			do {
				pActor = reinterpret_cast<CActor*>(reinterpret_cast<char*>(pActorManager->aClassInfo[pTransTable->actorClass].aActors) + actorIndex * pActorManager->aClassInfo[pTransTable->actorClass].size);
				pInventoryInfo = pActor->GetInventoryInfo();
				if (((pInventoryInfo != (CInventoryInfo*)0x0) && (pTransTable->itemId == pInventoryInfo->purchaseId)) && (pInventoryInfo->field_0x0 != 0)) {
					return pActor;
				}

				actorIndex = actorIndex + 1;
			} while (actorIndex < pActorManager->aClassInfo[pTransTable->actorClass].totalCount);
		}
	}

	return (CActor*)0x0;
}

bool CInventoryInterface::Cmd_Apply()
{
	bool bVar1;
	CActorHero* pHero;
	CInventoryInfo* pInventoryInfo;
	CActor* pActor;
	FE_Position fePosition;
	undefined4 local_8;
	edF32VECTOR4* pWorldPosition;

	pHero = CActorHero::_gThis;
	bVar1 = false;
	if (this->aHeaderInfo[0].activeItemIndex != 0) {
		fePosition.headerSlot = 0;
		fePosition.itemSlot = this->aHeaderInfo[0].activeItemIndex;
		pActor = FindActorInState(&fePosition, 2);
		if (pActor != (CActor*)0x0) {
			pInventoryInfo = pActor->GetInventoryInfo();
			pInventoryInfo->PrepareTransit(1, (edF32VECTOR4*)0x0, CActorHero::_gThis);
			bVar1 = true;
		}
	}

	if (this->aHeaderInfo[1].activeItemIndex != 0) {
		fePosition.headerSlot = 1;
		fePosition.itemSlot = this->aHeaderInfo[1].activeItemIndex;
		pActor = FindActorInState(&fePosition, 2);
		if (pActor != (CActor*)0x0) {
			pWorldPosition = &pHero->currentLocation;
			pActor->DoMessage(pActor, (ACTOR_MESSAGE)0x53, pWorldPosition);
			local_8 = 4;
			pActor->DoMessage(pActor, (ACTOR_MESSAGE)0x5f, (void*)4);
			bVar1 = true;
		}

		pInventoryInfo = pActor->GetInventoryInfo();
		Cmd_RemoveItem(pInventoryInfo->purchaseId, 1);
	}

	if (bVar1) {
		CScene::ptable.g_FrontendManager_00451680->pFrontendSamplePlayer->PlaySample(1.0f, 0, 0);
	}
	else {
		CScene::ptable.g_FrontendManager_00451680->pFrontendSamplePlayer->PlaySample(1.0f, 3, 0);
	}

	return bVar1;
}

bool CInventoryInterface::Cmd_AddItem(int ItemId, int count, uint flags)
{
	bool bVar2;
	InvTransTable* pTransTable;
	InventorySlot* pSlot;
	int iVar7;

	pTransTable = GetInvTransTable(ItemId);

	if (pTransTable == (InvTransTable*)0x0) {
		bVar2 = false;
	}
	else {
		pSlot = GetExistingInventorySlot(pTransTable->headerSlot, pTransTable->itemId);
		if (pSlot == (InventorySlot*)0x0) {
			pSlot = GetFreeSlot(pTransTable->headerSlot);

			this->aHeaderInfo[pTransTable->headerSlot].nbUsedSlots = this->aHeaderInfo[pTransTable->headerSlot].nbUsedSlots + 1;

			if (this->aHeaderInfo[pTransTable->headerSlot].nbUsedSlots == 0x10) {
				this->aHeaderInfo[pTransTable->headerSlot].pSlot = (InventorySlot*)0x0;
			}
			else {
				this->aHeaderInfo[pTransTable->headerSlot].pSlot = pSlot + 1;
			}
		}

		bVar2 = false;
		if (pSlot != (InventorySlot*)0x0) {
			pSlot->itemId = ItemId;
			pSlot->nbItems = pSlot->nbItems + count;
			pSlot->bInUse = 1;
			
			if ((flags & 1) != 0) {
				this->bDirty = 1;
			}

			bVar2 = true;
		}
	}

	return bVar2;
}

void CInventoryInterface::Cmd_Abort()
{
	this->aHeaderInfo[0].activeItemIndex = 0;
	this->aHeaderInfo[1].activeItemIndex = 0;

	CScene::ptable.g_FrontendManager_00451680->pFrontendSamplePlayer->PlaySample(1.0f, 3, 0);

	return;
}

void CInventoryInterface::Cmd_NextItem(int headerSlot)
{
	InventorySlot** ppIVar1;

	this->aHeaderInfo[headerSlot].activeItemIndex = this->aHeaderInfo[headerSlot].activeItemIndex + 1;
	if (this->aSlots[headerSlot][this->aHeaderInfo[headerSlot].activeItemIndex].bInUse == 0) {
		this->aHeaderInfo[headerSlot].activeItemIndex = 0;
	}

	CScene::ptable.g_FrontendManager_00451680->pFrontendSamplePlayer->PlaySample(1.0f, 1, 0);

	return;
}

bool CInventoryInterface::Cmd_RemoveItem(int itemId, uint flags)
{
	InvTransTable* pIVar1;
	bool bSuccess;
	InventorySlot* pIVar2;
	InventorySlot* pIVar3;
	InvTransTable* pIVar4;
	int iVar6;

	pIVar4 = GetInvTransTable(itemId);
	bSuccess = false;

	if (pIVar4 != (InvTransTable*)0x0) {
		pIVar2 = GetExistingInventorySlot(pIVar4->headerSlot, pIVar4->itemId);
		
		if (pIVar2 == (InventorySlot*)0x0) {
			bSuccess = false;
		}
		else {
			pIVar2->nbItems = pIVar2->nbItems + -1;

			if (pIVar2->nbItems < 1) {
				if ((this->aHeaderInfo[pIVar4->headerSlot].nbUsedSlots != 0x10) && (pIVar3 = pIVar2, pIVar2 != this->aHeaderInfo[pIVar4->headerSlot].pSlot)) {
					do {
						pIVar2 = pIVar3 + 1;
						pIVar3->itemId = pIVar2->itemId;
						pIVar3->nbItems = pIVar3[1].nbItems;
						pIVar3->bInUse = pIVar3[1].bInUse;
						pIVar3 = pIVar2;
					} while (pIVar2 != this->aHeaderInfo[pIVar4->headerSlot].pSlot);
				}

				pIVar2->itemId = -1;
				pIVar2->nbItems = 0;
				pIVar2->bInUse = 0;

				this->aHeaderInfo[pIVar4->headerSlot].nbUsedSlots = this->aHeaderInfo[pIVar4->headerSlot].nbUsedSlots + -1;
				this->aHeaderInfo[pIVar4->headerSlot].pSlot = pIVar2 + -1;
			}

			this->aHeaderInfo[pIVar4->headerSlot].activeItemIndex = 0;

			if ((flags & 1) != 0) {
				this->bDirty = 1;
			}

			bSuccess = true;
		}
	}

	return bSuccess;
}

void CInventoryInterface::Clear()
{
	for (int i = 0; i < 2; i++) {
		this->aHeaderInfo[i].activeItemIndex = 0;
		this->aHeaderInfo[i].nbUsedSlots = 1;
		this->aHeaderInfo[i].pSlot = this->aSlots[i] + 1;

		for (int j = 0; j < 16; j++) {
			this->aSlots[i][j].itemId = -1;
			this->aSlots[i][j].nbItems = 0;
			this->aSlots[i][j].bInUse = 0;
		}

		this->aSlots[i][0].bInUse = -1;
	}

	return;
}

bool CInventoryInterface::FindByType(FE_Position* pPosition, int itemId)
{
	int headerSlot;
	InvTransTable* pIVar2;
	bool bFound;
	InventorySlot* pInventorySlot;
	CInventoryInterface* ppIVar4;
	int iVar4;
	InvTransTable* pTransTable;

	pTransTable = GetInvTransTable(itemId);

	bFound = false;
	if (pTransTable != (InvTransTable*)0x0) {
		headerSlot = pTransTable->headerSlot;
		pInventorySlot = GetExistingInventorySlot(headerSlot, pTransTable->itemId);
		bFound = false;

		if (pInventorySlot != (InventorySlot*)0x0) {
			if (pPosition != (FE_Position*)0x0) {
				pPosition->headerSlot = headerSlot;
				const int inventorySlot = ((uintptr_t)pInventorySlot - (uintptr_t)this->aSlots[pTransTable->headerSlot]) / sizeof(InventorySlot);
				pPosition->itemSlot = inventorySlot;
			}
			bFound = true;
		}
	}

	return bFound;
}

bool CInventoryInterface::HasChanged()
{
	bool bChanged;

	bChanged = this->bDirty != 0;
	if (bChanged) {
		this->bDirty = 0;
	}

	return bChanged;
}