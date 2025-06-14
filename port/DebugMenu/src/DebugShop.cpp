#include "DebugShop.h"

#include <imgui.h>
#include "Types.h"

#include "InventoryInfo.h"
#include "DebugHelpers.h"
#include "DebugSetting.h"
#include "../../../src/ActorHero.h"

namespace Debug
{
	namespace Shop
	{
		constexpr int gNumBraceletColors = 8;

		Debug::Setting<bool> gAutoBuyBoomy("Auto Buy Boomy", false);

		struct BraceletInfo
		{
			BraceletInfo(const char* color) : color(color), name(color + std::string(" Bracelet")), autoBuySetting(std::string("Auto Buy ") + name, false) {}
			std::string color;
			std::string name;
			Debug::Setting<bool> autoBuySetting;
		};

		std::array<BraceletInfo, gNumBraceletColors> gBracelets = {
			BraceletInfo("White"), BraceletInfo("Yellow"),
			BraceletInfo("Green"), BraceletInfo("Blue"),
			BraceletInfo("Brown"), BraceletInfo("Black"),
			BraceletInfo("Silver"), BraceletInfo("Gold")
		};
	}
}

void Debug::Shop::ShowMenu(bool* bOpen)
{
	// Show a window with a grid of items
	if (ImGui::Begin("Debug Shop", bOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
		ImGui::PushStyleColor(ImGuiCol_Button, DebugHelpers::GetValidatedColor(CInventoryInfo::IsObjectPurchased(INVENTORY_ITEM_BASE_BOOMY)));
		if (ImGui::Button("Buy Boomy")) {
			CInventoryInfo info;
			info.purchaseId = INVENTORY_ITEM_BASE_BOOMY;

			info.ObjectPurchased();
		}

		ImGui::PopStyleColor();
		ImGui::SameLine(0.0f, 10.0f);

		gAutoBuyBoomy.DrawImguiControl();

		for (int i = 0; i < gBracelets.size(); ++i) {
			ImGui::PushStyleColor(ImGuiCol_Button, DebugHelpers::GetValidatedColor(CInventoryInfo::IsObjectPurchased(INVENTORY_ITEM_WHITE_BRACELET + i)));
			if (ImGui::Button(gBracelets[i].name.c_str())) {
				CInventoryInfo info;
				info.purchaseId = INVENTORY_ITEM_WHITE_BRACELET + i;
				info.ObjectPurchased();
			}
			ImGui::PopStyleColor();
			ImGui::SameLine(0.0f, 10.0f);
			gBracelets[i].autoBuySetting.DrawImguiControl();
		}
	}

	ImGui::End();
}

void Debug::Shop::Update()
{
	if (CActorHero::_gThis != nullptr) {
		if (gAutoBuyBoomy) {
			if (!CInventoryInfo::IsObjectPurchased(INVENTORY_ITEM_BASE_BOOMY)) {
				CInventoryInfo info;
				info.purchaseId = INVENTORY_ITEM_BASE_BOOMY;
				info.ObjectPurchased();
			}
		}

		for (int i = 0; i < gBracelets.size(); ++i) {
			if (gBracelets[i].autoBuySetting) {
				if (!CInventoryInfo::IsObjectPurchased(INVENTORY_ITEM_WHITE_BRACELET + i)) {
					CInventoryInfo info;
					info.purchaseId = INVENTORY_ITEM_WHITE_BRACELET + i;
					info.ObjectPurchased();
				}
			}
		}
	}
}
