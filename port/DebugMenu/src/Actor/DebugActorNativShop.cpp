#include "DebugActorNativShop.h"
#include "ActorNativShop.h"
#include "ActorHero.h"
#include "LevelScheduler.h"
#include "InventoryInfo.h"
#include <imgui.h>

namespace Debug::Actor::NativShop
{
	static const char* GetShopStateName(int state)
	{
		switch (state) {
		case 5:                        return "Idle (5)";
		case NATIV_SHOP_STATE_ENTERING: return "Entering (6)";
		case NATIV_SHOP_STATE_DISPLAY:  return "Display (7)";
		case NATIV_SHOP_STATE_LEAVING:  return "Leaving (8)";
		case 9:                        return "Purchasing (9)";
		default:                       return "Unknown";
		}
	}

	static const char* GetItemNameFromPurchaseId(int purchaseId)
	{
		switch (purchaseId) {
		case INVENTORY_ITEM_BOUNCE: return "Bounce";
		case INVENTORY_ITEM_CLIMBING_GLOVES: return "Climbing Gloves";
		case INVENTORY_ITEM_BASE_BOOMY: return "Boomy";
		case INVENTORY_ITEM_SILVER_BOOMY: return "Silver Boomy";
		case INVENTORY_ITEM_GOLD_BOOMY: return "Gold Boomy";
		case INVENTORY_ITEM_WHITE_BRACELET: return "White Bracelet";
		case INVENTORY_ITEM_YELLOW_BRACELET: return "Yellow Bracelet";
		case INVENTORY_ITEM_GREEN_BRACELET: return "Green Bracelet";
		case INVENTORY_ITEM_BLUE_BRACELET: return "Blue Bracelet";
		case INVENTORY_ITEM_BROWN_BRACELET: return "Brown Bracelet";
		case INVENTORY_ITEM_BLACK_BRACELET: return "Black Bracelet";
		case INVENTORY_ITEM_SILVER_BRACELET: return "Silver Bracelet";
		case INVENTORY_ITEM_GOLD_BRACELET: return "Gold Bracelet";
		case INVENTORY_ITEM_JAMGUT_WHISTLE: return "JamGut Whistle";
		case INVENTORY_ITEM_LIFE_BAR: return "Life Bar";
		case INVENTORY_ITEM_BASIC_BOARD: return "Basic Board";
		case INVENTORY_ITEM_MAGIC_BOARD: return "Magic Board";
		case INVENTORY_ITEM_TELESCOPE: return "Telescope";
		case INVENTORY_ITEM_ELECTRIC_BOMB: return "Electric Bomb";
		case INVENTORY_ITEM_TRAP_BOMB: return "Trap Bomb";
		case INVENTORY_ITEM_INVINCIBILITY_FRUIT: return "Invincibility Fruit";
		case INVENTORY_ITEM_LIFE_FRUIT: return "Life Fruit";
		case INVENTORY_ITEM_STRENGTH_FRUIT: return "Strength Fruit";
		default: return "(unknown)";
		}
	}

	static void ShowSubObjsTable(CBehaviourNativShopSell* pBehaviour)
	{
		if (!ImGui::BeginTable("SubObjs", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
			return;
		}

		ImGui::TableSetupColumn("Slot");
		ImGui::TableSetupColumn("Actor");
		ImGui::TableSetupColumn("Can Buy");
		ImGui::TableSetupColumn("Purchased");
		ImGui::TableSetupColumn("Nav (L/R/U/D)");
		ImGui::TableHeadersRow();

		for (int i = 0; i < 5; ++i) {
			NativShopSubObj& sub = pBehaviour->aSubObjs[i];
			CActor* pActor = sub.streamRefActor.Get();

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d%s", i, (i == pBehaviour->selectedPurchaseItemIndex) ? " <" : "");

			ImGui::TableSetColumnIndex(1);
			if (pActor) {
				CInventoryInfo* pInfo = pActor->GetInventoryInfo();
				if (pInfo) {
					ImGui::Text("id=%s (%d) obj=0x%x cost=%d", GetItemNameFromPurchaseId(pInfo->purchaseId), pInfo->purchaseId, pInfo->objectId, pInfo->moneyCost);
				}
				else {
					ImGui::Text("%s (%p)", pActor->name, pActor);
				}
			}
			else {
				ImGui::TextDisabled("(null)");
			}

			ImGui::TableSetColumnIndex(2);
			if (sub.bCanPurchase) {
				ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Yes");
			}
			else {
				ImGui::TextDisabled("No");
			}

			ImGui::TableSetColumnIndex(3);
			if (pActor) {
				CInventoryInfo* pInfo = pActor->GetInventoryInfo();
				if (pInfo) {
					bool bPurchased = (pInfo->objectId == 0x20) || (CInventoryInfo::IsObjectPurchased(pInfo->objectId) != 0);
					if (bPurchased) {
						ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Yes");
					}
					else {
						ImGui::Text("No");
					}
				}
				else {
					ImGui::TextDisabled("-");
				}
			}
			else {
				ImGui::TextDisabled("-");
			}

			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%d / %d / %d / %d", sub.field_0x4, sub.field_0x8, sub.field_0xc, sub.field_0x10);
		}

		ImGui::EndTable();
	}

	// Find the NativShopLevelSubObj in CLevelScheduler that corresponds to this shop.
	// Matched by field_0x0 == pNativShop->field_0x170 (the ScenVar threshold copied in Level_PostInit).
	static NativShopLevelSubObj* FindSchedulerSubObj(CActorNativShop* pNativShop)
	{
		CLevelScheduler* pScheduler = CLevelScheduler::gThis;
		if (!pScheduler) {
			return nullptr;
		}

		for (int i = 0; i < pScheduler->nbNativShopSubObjs; ++i) {
			if (pScheduler->aNativShopSubObjs[i].episodeThresholdLink == pNativShop->episodeThresholdLink) {
				return &pScheduler->aNativShopSubObjs[i];
			}
		}

		return nullptr;
	}

	static void ShowSchedulerSubObj(NativShopLevelSubObj* pSubObj)
	{
		ImGui::Text("Current Episode: %d", CLevelScheduler::ScenVar_Get(SCN_GAME_CURRENT_EPISODE));

		ImGui::Separator();

		ImGui::Text("Episode Threshold Link: 0x%x  (field_0x0)", pSubObj->episodeThresholdLink);
		ImGui::Text("Level ID:               %d    (currentLevelId)", pSubObj->currentLevelId);
		ImGui::Text("Visited Flag:           %d    (field_0x8)", pSubObj->field_0x8);

		const edF32VECTOR4& loc = pSubObj->currentLocation;
		ImGui::Text("Saved Position:    (%.2f, %.2f, %.2f)", loc.x, loc.y, loc.z);

		ImGui::Separator();
		ImGui::TextDisabled("Saved item snapshot (from FUN_002d8d10 / Level_PostInit)");

		if (!ImGui::BeginTable("SchedSubObjs", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
			return;
		}

		ImGui::TableSetupColumn("Slot");
		ImGui::TableSetupColumn("Purchase ID (0x20=empty)");
		ImGui::TableSetupColumn("Money Cost");
		ImGui::TableHeadersRow();

		for (int i = 0; i < 5; ++i) {
			const NativShopLevelSubObjSubObj& s = pSubObj->aSubObjs[i];
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d", i);
			ImGui::TableSetColumnIndex(1);
			if (static_cast<int>(s.purchaseId) == 0x20) {
				ImGui::TextDisabled("0x20 (empty)");
			}
			else {
				ImGui::Text("%s (0x%x)", GetItemNameFromPurchaseId(s.purchaseId), s.purchaseId);
			}
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%d", s.moneyCost);
		}

		ImGui::EndTable();
	}

	void ShowNativShopActorDetails(CActorNativShop* pNativShop)
	{
		if (!pNativShop) {
			ImGui::Text("NativShop Actor: null");
			return;
		}

		if (ImGui::CollapsingHeader("Shop State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Actor State:       %d  (%s)", pNativShop->actorState, GetShopStateName(pNativShop->actorState));
			ImGui::Text("Shop Visited:      %d  (field_0x17c)", pNativShop->field_0x17c);
			ImGui::Text("Last Interaction:  %d  (field_0x168, timestamp)", pNativShop->field_0x168);

			const float proximityRadius = sqrtf(pNativShop->field_0x164);
			ImGui::Text("Proximity Radius:  %.2f  (field_0x164, sqrt)", proximityRadius);

			if (pNativShop->field_0x16c) {
				ImGui::Text("Shop Name:         %s  (field_0x16c)", pNativShop->field_0x16c);
			}
			else {
				ImGui::TextDisabled("Shop Name:         (null)");
			}

			ImGui::Text("Episode Threshold: 0x%x  (field_0x170)", pNativShop->episodeThresholdLink);
			ImGui::Text("ScenVar Flag:      0x%x  (field_0x174)", pNativShop->field_0x174);
		}

		CBehaviourNativShopSell* pSell = &pNativShop->behaviourNativShop;

		if (ImGui::CollapsingHeader("Sell Behaviour", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Selected Item:     %d", pSell->selectedPurchaseItemIndex);
			ImGui::Text("Any Purchasable:   %d  (field_0x98)", pSell->field_0x98);
			ImGui::Text("bLocked:           %d  (bLocked)", pSell->bLocked);

			ImGui::Separator();
			ImGui::TextDisabled("UI / Transition");
			ImGui::Text("Slide Offset:      %.3f  (field_0xa4, panel slide-in X)", pSell->field_0xa4);
			ImGui::Text("Starting Money:    %.0f  (field_0xa8, at purchase start)", pSell->field_0xa8);
			ImGui::Text("Display Counter:   %d    (field_0xac, animated money count)", pSell->field_0xac);
			ImGui::Text("Player Money Now:  %d", CLevelScheduler::_gGameNfo.nbMoney);
			ImGui::Text("Help Text Mode:    %d    (field_0xb0)", pSell->field_0xb0);
			ImGui::Text("Fade Alpha:        %.3f  (field_0xb4)", pSell->field_0xb4);
			ImGui::Text("Material ID:       %d", pSell->materialId);

			ImGui::Separator();
			ImGui::TextDisabled("Audio");
			ImGui::Text("Music:             %p  (field_0xb8)", pSell->field_0xb8);
			ImGui::Text("Audio Handle:      %d  (field_0xc0, -1 = inactive)", pSell->field_0xc0);
			ImGui::Text("Audio Fade In:     %.2f  (field_0xc4)", pSell->field_0xc4);
			ImGui::Text("Audio Fade Out:    %.2f  (field_0xc8)", pSell->field_0xc8);
			ImGui::Text("Audio Fade Time:   %.2f  (field_0xcc)", pSell->field_0xcc);
		}

		if (ImGui::CollapsingHeader("Shop Items", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowSubObjsTable(pSell);
		}

		if (ImGui::CollapsingHeader("Actor References", ImGuiTreeNodeFlags_DefaultOpen)) {
			CActor* pNative = pNativShop->actorRef.Get();
			ImGui::Text("Native Actor:  %s  (%p)", pNative ? pNative->name : "(null)", pNative);

			CCamera* pCamera = pSell->streamRefCamera.Get();
			ImGui::Text("Shop Camera:   %p", pCamera);

			ed_zone_3d* pZone = pSell->streamRefZone.Get();
			ImGui::Text("Music Zone:    %p", pZone);

			S_ACTOR_STREAM_REF* pActorStream = pNativShop->pActorStream;
			if (pActorStream) {
				ImGui::Text("Shopkeeper Stream: %d entries", pActorStream->entryCount);
				for (int i = 0; i < pActorStream->entryCount; ++i) {
					CActor* pEntry = pActorStream->aEntries[i].Get();
					ImGui::Text("  [%d] %s (%p)", i, pEntry ? pEntry->name : "(null)", pEntry);
				}
			}
			else {
				ImGui::TextDisabled("Shopkeeper Stream: (null)");
			}
		}

		if (ImGui::CollapsingHeader("Display Object", ImGuiTreeNodeFlags_DefaultOpen)) {
			const ShopObjData& obj = pSell->objData;
			ImGui::Text("Actor:     %s  (%p)", obj.pActor ? obj.pActor->name : "(null)", obj.pActor);
			ImGui::Text("Node:      %p", obj.pNode);
			ImGui::Text("Hierarchy: %p", obj.pHierarchy);
			ImGui::Text("TexInfo:   %p", obj.pTextureInfo);
		}

		// Cross-reference with CLevelScheduler's global shop tracking table.
		// Populated in Level_PostInit (levelID==0) and persisted across saves via SaveGame_SaveGameObj.
		if (ImGui::CollapsingHeader("Scheduler State (Global Persistence)", ImGuiTreeNodeFlags_DefaultOpen)) {
			CLevelScheduler* pScheduler = CLevelScheduler::gThis;
			if (!pScheduler) {
				ImGui::TextDisabled("CLevelScheduler not available.");
			}
			else {
				ImGui::Text("Total tracked shops: %d / 10", pScheduler->nbNativShopSubObjs);
				ImGui::Spacing();

				NativShopLevelSubObj* pSubObj = FindSchedulerSubObj(pNativShop);
				if (pSubObj) {
					ShowSchedulerSubObj(pSubObj);
				}
				else {
					ImGui::TextDisabled("No matching entry found (field_0x170 = 0x%x not in scheduler table).", pNativShop->episodeThresholdLink);
					ImGui::TextDisabled("Entry is only created for levelID 0 shops in Level_PostInit.");
				}
			}
		}
	}
}
