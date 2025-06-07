#include "DebugShop.h"

#include <imgui.h>
#include "Types.h"

#include "InventoryInfo.h"
#include "DebugHelpers.h"

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
	}

	ImGui::End();
}