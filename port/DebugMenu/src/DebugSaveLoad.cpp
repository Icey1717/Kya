#include "DebugMenu.h"
#include "DebugSaveLoad.h"
#include "imgui.h"

#include "SaveManagement.h"
#include "LevelScheduler.h"
#include "Pause.h"
#include "log.h"

#include "DebugSetting.h"

#include <chrono>

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

	namespace
	{
		enum class AutoLoadState { Disabled, Waiting, Loading, Done, Failed, Cancelled };
		AutoLoadState autoLoadState = AutoLoadState::Disabled;
		int autoLoadSlot = -1;
		bool autoLoadTaskPending = false;
		const char* autoLoadStatus = "Disabled";
		std::chrono::steady_clock::time_point lastAutoLoadLog;

		void SetAutoLoadStatus(AutoLoadState state, const char* status)
		{
			const auto now = std::chrono::steady_clock::now();
			// Repeat waiting diagnostics occasionally, but never use elapsed time for readiness.
			if (state != autoLoadState || status != autoLoadStatus || now - lastAutoLoadLog >= std::chrono::seconds(5)) {
				MY_LOG_CATEGORY("AutoLoad", LogLevel::Info, "AutoLoad: slot={} {}", autoLoadSlot, status);
				// Preserve the last operation in the log even if the subsequent load hangs.
				Log::GetInstance().ForceFlush();
				lastAutoLoadLog = now;
			}
			autoLoadState = state;
			autoLoadStatus = status;
		}

		void AutoLoad()
		{
			auto* scene = CScene::_pinstance;
			auto* scheduler = CLevelScheduler::gThis;
			auto* pause = CScene::ptable.g_PauseManager_00451688;
			if (!scene || !scheduler || !pause) {
				SetAutoLoadStatus(AutoLoadState::Waiting, "Waiting for scene managers");
				return;
			}

			// 0xe is the title level, 0xf the preintro, and 0x10 means no level.
			if (scheduler->currentLevelID >= 0 && scheduler->currentLevelID < 0xe) {
				SetAutoLoadStatus(AutoLoadState::Cancelled, "Cancelled: gameplay already started");
				return;
			}
			if (scheduler->currentLevelID != 0xe || !(GameFlags & 0x40) ||
				!pause->pSimpleMenu || !pause->pSplashScreen || !gSaveManagement.pBigAlloc_0x34) {
				SetAutoLoadStatus(AutoLoadState::Waiting, "Waiting for initialized title screen");
				return;
			}
			if (scheduler->nextLevelID != 0x10 || scene->IsFadeTermActive() || (GameFlags & GAME_REQUEST_TERM)) {
				SetAutoLoadStatus(AutoLoadState::Cancelled, "Cancelled: another level transition has started");
				return;
			}

			// The title screen is ready even before Start is pressed. No menu navigation is needed.
			// Mark Loading before entering code that can render nested frames/message boxes.
			SetAutoLoadStatus(AutoLoadState::Loading, "Checking save metadata");
			const int deviceState = gSaveManagement.test_device_has_enough_room();
			MY_LOG_CATEGORY("AutoLoad", LogLevel::Info,
				"AutoLoad: slot={} level={} nextLevel={} flags={:#x} titleState={} device={} files={:#x}",
				autoLoadSlot, scheduler->currentLevelID, scheduler->nextLevelID, GameFlags,
				pause->field_0x34, deviceState, gSaveManagement.fileExistsFlags);
			if (deviceState != ROOM_CHECK_RESULT_OK && deviceState != ROOM_CHECK_RESULT_NOT_ENOUGH_ROOM) {
				SetAutoLoadStatus(AutoLoadState::Failed, "Failed: save device unavailable");
				return;
			}
			// Despite its name, is_valid returns true for an empty/invalid slot.
			if (gSaveManagement.is_valid(autoLoadSlot)) {
				SetAutoLoadStatus(AutoLoadState::Failed, "Failed: save slot is empty or invalid");
				return;
			}

			SetAutoLoadStatus(AutoLoadState::Loading, "Calling MemCardLoad0");
			MemCardLoad0(autoLoadSlot);
			const bool requested = scheduler->bShouldLoad && scheduler->nextLevelID != 0x10 && scene->IsFadeTermActive();
			MY_LOG_CATEGORY("AutoLoad", LogLevel::Info,
				"AutoLoad: load returned; slot={} shouldLoad={} nextLevel={} fadeTerm={} flags={:#x}",
				autoLoadSlot, scheduler->bShouldLoad, scheduler->nextLevelID, scene->IsFadeTermActive(), GameFlags);
			SetAutoLoadStatus(requested ? AutoLoadState::Done : AutoLoadState::Failed,
				requested ? "Save loaded; level transition requested" : "Failed: load returned without a level transition");
		}

		void UpdateAutoLoad()
		{
			static bool initialized = false;
			if (!initialized) {
				initialized = true;
				autoLoadSlot = gDefaultSaveSlot;
				if (gAutoLoadOnStart) {
					SetAutoLoadStatus(autoLoadSlot >= 0 && autoLoadSlot < 4 ? AutoLoadState::Waiting : AutoLoadState::Failed,
						autoLoadSlot >= 0 && autoLoadSlot < 4 ? "Armed; waiting for title screen" : "Failed: default slot must be 0-3");
				}
				else {
					SetAutoLoadStatus(AutoLoadState::Disabled, "Disabled at startup");
				}
			}
			if (autoLoadState == AutoLoadState::Waiting && autoLoadTaskPending) {
				SetAutoLoadStatus(AutoLoadState::Waiting, "Waiting for queued level-management check");
			}
			if (autoLoadState == AutoLoadState::Waiting && !autoLoadTaskPending) {
				autoLoadTaskPending = true;
				EnqueueLevelManageTask([]() {
					AutoLoad();
					// Keep pending through nested rendering. Retry only from a later debug update,
					// never by appending to the queue while Level_Manage is iterating it.
					autoLoadTaskPending = false;
				});
			}
		}
	}
}

void Debug::SaveLoad::ShowMenu(bool* bOpen)
{
	// Show a window with a grid of items
	if (ImGui::Begin("Debug Save/Load", bOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
		gDefaultSaveSlot.DrawImguiControl();

		gAutoLoadOnStart.DrawImguiControl();
		ImGui::TextWrapped("Auto-load: %s (slot %d)", autoLoadStatus, autoLoadSlot);

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
	UpdateAutoLoad();
	// Save loading can render nested debug frames. Do not append hotkey tasks to
	// the scheduler's queue while the auto-load callback is executing.
	if (autoLoadState == AutoLoadState::Loading) {
		return;
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

namespace Debug {
	MenuRegisterer sDebugSaveLoadMenuReg("Save/Load", Debug::SaveLoad::ShowMenu, true);
	UpdateRegisterer sDebugSaveLoadUpdateReg(Debug::SaveLoad::Update);
}
