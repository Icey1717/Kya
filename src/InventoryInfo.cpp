#include "InventoryInfo.h"
#include "MemoryStream.h"
#include "LargeObject.h"
#include "Actor.h"
#include "ActorBoomy.h"
#include "ActorAutonomous.h"
#include "LevelScheduleManager.h"
#include "ActorHero.h"

int CInventoryInfo::ProcessMessage(CActor* pSender, int msg, void* pMsgParams)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	if (msg == 0x5f) {
		if (this->field_0x0 != 0) {
			int msgType = reinterpret_cast<int>(pMsgParams);
			switch (msgType) {
			case 0x0:
				this->field_0x0 = 2;

				return 1;
			case 0x1:
				this->field_0x0 = 3;
				pSender->flags = pSender->flags & 0xffffff7f;
				pSender->flags = pSender->flags | 0x20;
				pSender->EvaluateDisplayState();
				pSender->flags = pSender->flags & 0xfffffffd;
				pSender->flags = pSender->flags | 1;

				return 1;
			case 0x2:
				this->field_0x0 = 2;

				return 1;
			case 0x3:
				this->field_0x0 = 3;

				return 1;
			case 0x4:
				pSender->flags = pSender->flags & 0xffffff5f;
				pSender->EvaluateDisplayState();
				pSender->flags = pSender->flags & 0xfffffffc;
				this->field_0x0 = 1;

				return 1;
			case 0x5:
				pSender->flags = pSender->flags & 0xffffff5f;
				pSender->EvaluateDisplayState();
				pSender->flags = pSender->flags & 0xfffffffc;

				return 1;
			case 0x6:
				pSender->flags = pSender->flags & 0xffffff7f;
				pSender->flags = pSender->flags | 0x20;
				pSender->EvaluateDisplayState();
				pSender->flags = pSender->flags & 0xfffffffd;
				pSender->flags = pSender->flags | 1;

				return 1;
			case 0x7:
				ObjectPurchased();

				return 1;
			}
		}
	}
	else {
		if (msg == 0x5e) {
			IMPLEMENTATION_GUARD(
			/* WARNING: Load size is inaccurate */
			iVar1 = *pMsgParams;
			if (iVar1 == 1) {
				return 1;
			}
			if (iVar1 == 2) {
				fVar4 = (pSender->currentLocation).y;
				fVar2 = (pSender->currentLocation).z;
				fVar3 = (pSender->currentLocation).w;
				*(float*)((int)pMsgParams + 0x10) = (pSender->currentLocation).x;
				*(float*)((int)pMsgParams + 0x14) = fVar4;
				*(float*)((int)pMsgParams + 0x18) = fVar2;
				*(float*)((int)pMsgParams + 0x1c) = fVar3;
				return 1;
			}
			if (iVar1 == 0) {
				*(int*)((int)pMsgParams + 4) = this->field_0x0;
				return 1;
			})
		}
		else {
			if (msg == 0x53) {
				pSender->UpdatePosition(reinterpret_cast<edF32VECTOR4*>(pMsgParams), true);
				return 1;
			}
		}
	}
	return 0;
}

void CInventoryInfo::Reset()
{
	if (this->field_0x0 != 0) {
		this->field_0x0 = 2;
	}

	return;
}

void CInventoryInfo::Init(CActor* pOwner)
{
	this->pOwner = pOwner;

	return;
}

void CInventoryInfo::Create(ByteCode* pByteCode)
{
	int iVar1;
	ulong uVar2;
	int iVar3;

	iVar1 = pByteCode->GetS32();
	iVar3 = 2;

	if (iVar1 == 0) {
		iVar3 = 0;
	}

	this->field_0x0 = iVar3;

	if (iVar1 != 0) {
		this->field_0x8 = pByteCode->GetU64();
		this->field_0x10 = pByteCode->GetU64();

		this->purchaseId = pByteCode->GetS32();
		this->field_0x28 = pByteCode->GetS32();
		this->moneyCost = pByteCode->GetS32();
		this->field_0x2c = pByteCode->GetS32();

		if (CScene::_pinstance->field_0x1c < 2.27f) {
			this->objectId = 0x20;
			this->field_0x18 = 0;
		}
		else {
			iVar1 = pByteCode->GetS32();
			if (iVar1 == 0) {
				this->objectId = 0x20;
				this->field_0x18 = 0;
			}
			else {
				this->objectId = pByteCode->GetS32();
				this->field_0x18 = pByteCode->GetU64();
			}
		}
	}

	return;
}

void CInventoryInfo::ObjectPurchased()
{
	CActorBoomy* pCVar1;
	uint uVar2;
	int iVar3;
	int iVar4;
	CLifeInterface* pLifeInterface;
	float fVar5;

	switch (this->purchaseId) {
	case INVENTORY_ITEM_BASE_BOOMY:
		if (CLevelScheduler::ScenVar_Get(SCENE_VAR_BOOMY) < 1) {
			this->pOwner->DoMessage(CActorHero::_gThis, MESSAGE_BOOMY_CHANGED, (void*)1);
			this->pOwner->DoMessage(CActorHero::_gThis->pActorBoomy, MESSAGE_BOOMY_CHANGED, (void*)1);
		}

		CLevelScheduler::ScenVar_Set(SCENE_VAR_BOOMY, 1);
		break;
	case INVENTORY_ITEM_WHITE_BRACELET:
	case INVENTORY_ITEM_YELLOW_BRACELET:
	case INVENTORY_ITEM_GREEN_BRACELET:
	case INVENTORY_ITEM_BLUE_BRACELET:
	case INVENTORY_ITEM_BROWN_BRACELET:
	case INVENTORY_ITEM_BLACK_BRACELET:
	case INVENTORY_ITEM_SILVER_BRACELET:
	case INVENTORY_ITEM_GOLD_BRACELET:
		uVar2 = CLevelScheduler::ScenVar_Get(SCENE_VAR_FIGHT_RING);
		uVar2 = uVar2 | 1 << (this->purchaseId - 5U & 0x1f);
		CLevelScheduler::ScenVar_Set(SCENE_VAR_FIGHT_RING, uVar2);
		this->pOwner->DoMessage(CActorHero::_gThis, MESSAGE_FIGHT_RING_CHANGED, (void*)uVar2);
		break;
	default:
		IMPLEMENTATION_GUARD();
		return;
	}
}

uint CInventoryInfo::IsObjectPurchased(int objId)
{
	int iVar1;
	uint uVar2;
	undefined auStack8[8];

	switch (objId) {
	case 0:
		uVar2 = CLevelScheduler::ScenVar_Get(7);
		break;
	case 1:
		uVar2 = CLevelScheduler::ScenVar_Get(8);
		break;
	case INVENTORY_ITEM_BASE_BOOMY:
		iVar1 = CLevelScheduler::ScenVar_Get(SCENE_VAR_BOOMY);
		uVar2 = iVar1 < 1 ^ 1;
		break;
	case INVENTORY_ITEM_SILVER_BOOMY:
		iVar1 = CLevelScheduler::ScenVar_Get(SCENE_VAR_BOOMY);
		uVar2 = iVar1 < 2 ^ 1;
		break;
	case INVENTORY_ITEM_GOLD_BOOMY:
		iVar1 = CLevelScheduler::ScenVar_Get(SCENE_VAR_BOOMY);
		uVar2 = iVar1 < 3 ^ 1;
		break;
	case INVENTORY_ITEM_WHITE_BRACELET:
	case INVENTORY_ITEM_YELLOW_BRACELET:
	case INVENTORY_ITEM_GREEN_BRACELET:
	case INVENTORY_ITEM_BLUE_BRACELET:
	case INVENTORY_ITEM_BROWN_BRACELET:
	case INVENTORY_ITEM_BLACK_BRACELET:
	case INVENTORY_ITEM_SILVER_BRACELET:
	case INVENTORY_ITEM_GOLD_BRACELET:
		uVar2 = CLevelScheduler::ScenVar_Get(SCENE_VAR_FIGHT_RING);
		uVar2 = 1 << (objId - 5U & 0x1f) & uVar2;
		break;
	case 0xd:
		uVar2 = CLevelScheduler::ScenVar_Get(0xb);
		break;
	case 0xe:
		uVar2 = CLevelScheduler::ScenVar_Get(0xc);
		break;
	case 0xf:
		uVar2 = CLevelScheduler::ScenVar_Get(0xd);
		break;
	case 0x10:
		uVar2 = 1;
		break;
	case 0x11:
		uVar2 = 1;
		break;
	case 0x12:
		iVar1 = CLevelScheduler::ScenVar_Get(0x10);
		uVar2 = iVar1 < 1 ^ 1;
		break;
	case 0x13:
		iVar1 = CLevelScheduler::ScenVar_Get(0x10);
		uVar2 = iVar1 < 2 ^ 1;
		break;
	case 0x14:
		uVar2 = CLevelScheduler::ScenVar_Get(0xe);
		break;
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
		IMPLEMENTATION_GUARD(
		uVar2 = CInventoryInterface::FindByType
		(*(CInventoryInterface**)(CScene::GetManager(MO_Frontend)->pInventory->field_0x4.pInterface, (long)(int)auStack8, objId);))
		break;
	default:
		uVar2 = 0;
	}

	return uVar2;
}
