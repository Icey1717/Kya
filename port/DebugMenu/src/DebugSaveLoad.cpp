#include "DebugSaveLoad.h"
#include "imgui.h"

#include "SaveManagement.h"
#include "LevelScheduleManager.h"
#include "Pause.h"
#include "InputManager.h"

#include "DebugSetting.h"

#include <future>

namespace Debug::SaveLoad
{
	Debug::Setting<int> gDefaultSaveSlot = { "Default Save Slot", -1 };

	Debug::Setting<bool> gAutoLoadOnStart = { "Auto Load On Start", false };

	void DrawSlot(int slotIndex)
	{
		auto slotDesc = gSaveManagement.get_save_data_desc(slotIndex);
		if (slotDesc) {
			ImGui::PushID(slotIndex);
			// Show a button with the slot index
			ImGui::TextColored((gSaveManagement.slotID_0x28 == slotIndex) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Slot %d", slotIndex);

			// Show the slot name and timestamp
			ImGui::TextWrapped("Level: %02X", slotDesc->levelId);
			ImGui::TextWrapped("Time: %02d:%02d:%02d", (int)(slotDesc->gameTime / 3600.0f), (int)((slotDesc->gameTime / 60.0f) / 60.0f), (int)(slotDesc->gameTime / 60.0f));
			ImGui::TextWrapped("Wolfen: %d", slotDesc->nbFreedWolfen);
			ImGui::TextWrapped("Magic: %d", slotDesc->nbMagic);
			ImGui::TextWrapped("Money: %d", slotDesc->nbMoney);
			ImGui::TextWrapped("Completed: %s", slotDesc->bGameCompleted ? "Yes" : "No");

			ImGui::PopID();
		}
		else
		{
			ImGui::PushID(slotIndex);
			ImGui::TextColored((gSaveManagement.slotID_0x28 == slotIndex) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Slot %d", slotIndex);
			ImGui::TextWrapped("Empty");
			ImGui::PopID();
		}
	}

	std::future<void> gAutoLoadFuture;

	void AutoLoad()
	{
		gAutoLoadFuture = std::async(std::launch::async, []() {
			gPlayerInput.bActive = 0;

			// Wait for a few seconds to allow the game to initialize by sleeping
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			// Simulate pressing Enter to load the game
			CPlayerInput& input = gPlayerInput;

			input.releasedBitfield |= PAD_BITMASK_START;

			while (CScene::ptable.g_PauseManager_00451688->pSimpleMenu == nullptr) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			CScene::ptable.g_PauseManager_00451688->pSimpleMenu->currentPage = PM_LoadMenu;
			CScene::ptable.g_PauseManager_00451688->pSimpleMenu->selectedIndex = gDefaultSaveSlot;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			input.pressedBitfield = 0x01000010;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			input.pressedBitfield = 0;
			gPlayerInput.bActive = 1;
		});
	}
}

void Debug::SaveLoad::ShowMenu(bool* bOpen)
{
	// Show a window with a grid of items
	if (ImGui::Begin("Debug Save/Load", bOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
		gDefaultSaveSlot.DrawImguiControl();

		gAutoLoadOnStart.DrawImguiControl();

		ImGui::TextWrapped("Press F5 to save, F7 to load");

		ImGui::BeginChild("SaveLoadSlots", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

		// Show the 4 slots in a list
		for (int i = 0; i < 4; i++) {
			Debug::SaveLoad::DrawSlot(i);
			ImGui::Separator();
		}

		ImGui::EndChild();
	}

	ImGui::End();
}

void Debug::SaveLoad::Update()
{
	static bool bAutoLoad = gAutoLoadOnStart;

	if (bAutoLoad) {
		AutoLoad();
		bAutoLoad = false;
	}

	// Listen for F5 and F7 key presses
	if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
		const int slotId = gSaveManagement.slotID_0x28 >= 0 ? gSaveManagement.slotID_0x28 : gDefaultSaveSlot;
		EnqueueLevelManageTask([slotId]() {
			uint uVar4 = GameFlags | 4;
			if ((GameFlags & 0x800) == 0) {
				if ((GameFlags & 0x40) == 0) {
					uint uVar1 = GameFlags & 0xc;
					GameFlags = uVar4;
					if ((uVar1 | 4) != 0) {
						CScene::_pinstance->SetGlobalPaused_001b8c30(1);
					}

					CScene::_pinstance->Level_PauseChange(1);
				}
				else {
					GameFlags = uVar4;
				}
			}
			else {
				GameFlags = uVar4;
				CScene::_pinstance->Level_PauseChange(1);
				CScene::_pinstance->SetGlobalPaused_001b8c30(1);
			}
			SaveManagement_MemCardSave(slotId);
			CScene::_pinstance->Level_PauseChange(0);
			CScene::_pinstance->SetGlobalPaused_001b8c30(0);
			});
	}

	if (ImGui::IsKeyPressed(ImGuiKey_F7)) {
		const int slotId = gSaveManagement.slotID_0x28 >= 0 ? gSaveManagement.slotID_0x28 : gDefaultSaveSlot;
		EnqueueLevelManageTask([slotId]() {
			CScene::_pinstance->SetGlobalPaused_001b8c30(1);
			MemCardLoad0(slotId);
			});
	}
}
