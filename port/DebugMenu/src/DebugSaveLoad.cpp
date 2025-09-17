#include "DebugSaveLoad.h"
#include "imgui.h"

#include "SaveManagement.h"
#include "DebugSetting.h"

Debug::Setting<int> gDefaultSaveSlot = { "Default Save Slot", -1 };

void Debug::SaveLoad::ShowMenu(bool* bOpen)
{
	// Show a window with a grid of items
	if (ImGui::Begin("Debug Save/Load", bOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
		gDefaultSaveSlot.DrawImguiControl();
	}

	ImGui::End();
}

void Debug::SaveLoad::Update()
{
	// Listen for F5 and F7 key presses
	if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
		if (gSaveManagement.slotID_0x28 >= 0) {
			SaveManagement_MemCardSave(gSaveManagement.slotID_0x28);
		}
		else
		{
			SaveManagement_MemCardSave(gDefaultSaveSlot);
		}
	}

	if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
		if (gSaveManagement.slotID_0x28 >= 0) {
			SaveManagement_MemCardLoad(gSaveManagement.slotID_0x28);
		}
		else
		{
			SaveManagement_MemCardLoad(gDefaultSaveSlot);
		}
	}
}
