#include "DebugMenu.h"
#include "DebugSaveLoad.h"
#include "DebugSaveLoadPaths.h"
#include "DebugSaveCheckpoint.h"
#include "DebugWorldNames.h"
#include "ActorManager.h"
#include "ActorCheckpointManager.h"
#include "WayPoint.h"
#include "imgui.h"

#include "SaveManagement.h"
#include "LevelScheduler.h"
#include "Pause.h"
#include "log.h"

#include "DebugSetting.h"
#include "edFile/edFileCRC32.h"
#include "EdenLib/edFile/sources/ps2/WinSaveFile.h"

#include <chrono>
#include <cmath>
#include <cstring>
#include <vector>

namespace Debug::SaveLoad
{
	Debug::Setting<int> gDefaultSaveSlot = { "Default Save Slot", -1 };

	Debug::Setting<bool> gAutoLoadOnStart = { "Auto Load On Start", false };

	void DrawSaveInfo(const SaveDataDesc& desc)
	{
		ImGui::Text("Level: %02X", desc.levelId);
		const double seconds = std::isfinite(desc.gameTime) && desc.gameTime >= 0 ? desc.gameTime : 0;
		ImGui::Text("Time: %.0f:%02d:%02d", std::floor(seconds / 3600),
			(int)std::fmod(seconds / 60, 60), (int)std::fmod(seconds, 60));
		ImGui::Text("Wolfen: %d   Magic: %d   Money: %d", desc.nbFreedWolfen, desc.nbMagic, desc.nbMoney);
		ImGui::Text("Completed: %s", desc.bGameCompleted ? "Yes" : "No");
	}

	void DrawSlot(int slotIndex)
	{
		auto slotDesc = gSaveManagement.get_save_data_desc(slotIndex);
		if (slotDesc) {
			ImGui::PushID(slotIndex);
			// Show a button with the slot index
			ImGui::TextColored((gSaveManagement.slotID_0x28 == slotIndex) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Slot %d", slotIndex);

			// Show the slot name and timestamp
			DrawSaveInfo(*slotDesc);

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
		bool backupsOpen = false;
		bool restorePending = false;
		bool hotkeyTaskPending = false;
		bool refreshBackups = false;
		std::string backupStatus;
		std::string backupDirectory;

		struct SaveBackup
		{
			int slot;
			int index;
			std::filesystem::path destination;
			SaveDataDesc desc;
			std::vector<char> data;
			std::string error;
			SYSTEMTIME modified = {};
			SavedCheckpoint checkpoint;
			bool checkpointReadable = false;
		};
		std::vector<SaveBackup> backups;

		void ReadBackup(SaveBackup& backup, const std::filesystem::path& path)
		{
			WIN32_FILE_ATTRIBUTE_DATA attributes;
			FILETIME localTime;
			if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attributes) &&
				FileTimeToLocalFileTime(&attributes.ftLastWriteTime, &localTime)) {
				FileTimeToSystemTime(&localTime, &backup.modified);
			}
			std::ifstream input(path, std::ios::binary | std::ios::ate);
			const auto size = input.tellg();
			constexpr size_t prefixSize = sizeof(SaveDataHeader) + sizeof(SaveDataDesc);
			if (!input || size < static_cast<std::streamoff>(prefixSize) || size > 0x10000 + prefixSize) {
				backup.error = "Unreadable or invalid save size";
				return;
			}
			backup.data.resize(static_cast<size_t>(size));
			input.seekg(0);
			if (!input.read(backup.data.data(), size)) {
				backup.error = "Could not read backup";
				return;
			}
			SaveDataHeader header;
			std::memcpy(&header, backup.data.data(), sizeof(header));
			std::memcpy(&backup.desc, backup.data.data() + sizeof(header), sizeof(backup.desc));
			if (header.hash != 0x4544454e || header.headerSize != sizeof(header) ||
				header.initialBlockSize != sizeof(SaveDataDesc) || header.mainBlockSize <= 0 ||
				header.mainBlockSize > backup.data.size() - prefixSize ||
				header.headerCrc != edFileComputeCRC32(&header.headerSize, 0x14) ||
				header.initialBlockCrc != edFileComputeCRC32(&backup.desc, sizeof(backup.desc)) ||
				header.mainBlockCrc != edFileComputeCRC32(backup.data.data() + prefixSize, header.mainBlockSize) ||
				backup.desc.levelId >= 0xe || !std::isfinite(backup.desc.gameTime) || backup.desc.gameTime < 0) {
				backup.error = "Invalid save header, data, or checksum";
			}
			else {
				backup.checkpointReadable = ReadSavedCheckpoint(
					std::string_view(backup.data.data() + prefixSize, header.mainBlockSize), backup.checkpoint);
			}
		}

		void DrawCheckpoint(const SaveBackup& backup)
		{
			const auto& saved = backup.checkpoint;
			if (!backup.checkpointReadable) {
				ImGui::TextUnformatted("Checkpoint: save layout not recognized");
				return;
			}
			const auto sectorName = WorldNames::GetSectorName(saved.level, saved.sector);
			ImGui::Text("Checkpoint sector: %d%s%s", saved.sector, sectorName.empty() ? "" : " - ", sectorName.c_str());
			if (!saved.hasPosition) {
				ImGui::TextUnformatted("Checkpoint: no saved hero position");
				return;
			}
			ImGui::Text("Checkpoint position: %.3f, %.3f, %.3f", saved.position[0], saved.position[1], saved.position[2]);
			auto* scheduler = CLevelScheduler::gThis;
			auto* actors = CScene::ptable.g_ActorManager_004516a4;
			if (!scheduler || scheduler->currentLevelID != saved.level || !actors ||
				!CScene::_pinstance || CScene::_pinstance->IsFadeTermActive() || (GameFlags & GAME_REQUEST_TERM)) {
				ImGui::TextUnformatted("Checkpoint name: load this level to resolve");
				return;
			}
			int matches = 0;
			std::string label;
			for (int actorIndex = 0; actorIndex < actors->nbActors; ++actorIndex) {
				auto* actor = actors->aActors[actorIndex];
				if (!actor || actor->typeID != CHECKPOINT_MANAGER) continue;
				auto* manager = static_cast<CActorCheckpointManager*>(actor);
				for (int index = 0; index < manager->checkpointCount; ++index) {
					auto& checkpoint = manager->aCheckpoints[index];
					const auto* waypoint = checkpoint.pWayPointA.Get();
					if (!waypoint || (checkpoint.sectorId > 0 && checkpoint.sectorId != saved.sector)) continue;
					const auto& p = waypoint->location;
					const auto& r = waypoint->rotation;
					if (std::fabs(p.x - saved.position[0]) > 0.01f || std::fabs(p.y - saved.position[1]) > 0.01f ||
						std::fabs(p.z - saved.position[2]) > 0.01f || std::fabs(r.x - saved.rotation[0]) > 0.001f ||
						std::fabs(r.y - saved.rotation[1]) > 0.001f || std::fabs(r.z - saved.rotation[2]) > 0.001f) continue;
					++matches;
					const auto name = WorldNames::GetCheckpointName(saved.level, manager->name, index);
					label = std::string(manager->name) + " / " + std::to_string(index);
					if (!name.empty()) label = name + " (" + label + ")";
				}
			}
			if (matches == 1) ImGui::TextWrapped("Checkpoint: %s (waypoint match)", label.c_str());
			else if (matches > 1) ImGui::Text("Checkpoint name: ambiguous (%d matching waypoints)", matches);
			else ImGui::TextUnformatted("Checkpoint name: no matching waypoint");
		}

		void RefreshBackups()
		{
			backups.clear();
			backupDirectory.clear();
			refreshBackups = false;
			try {
				// The Windows memory-card filer strips the leading slash and uses the working directory.
				std::filesystem::path directory;
				if (!TryGetBackupDirectory(
					std::string_view(gSaveManagement.memCardPathEnd, sizeof(gSaveManagement.memCardPathEnd)),
					std::string_view(gSaveManagement.serialNumber, sizeof(gSaveManagement.serialNumber)), directory)) {
					backupStatus = "Cannot list backups: save directory is uninitialized or invalid";
					return;
				}
				backupDirectory = directory.string();
				for (int slot = 0; slot < 4; ++slot) {
					for (int index = 1; index <= 10; ++index) {
						SaveBackup backup;
						backup.slot = slot;
						backup.index = index;
						backup.destination = directory / ("slot_" + std::to_string(slot) + ".dat");
						auto path = backup.destination;
						path += ".bak." + std::to_string(index);
						if (!std::filesystem::exists(path)) continue;
						ReadBackup(backup, path);
						backups.push_back(std::move(backup));
					}
				}
			}
			catch (const std::exception& error) {
				backupStatus = std::string("Could not list backups: ") + error.what();
			}
		}

		bool CanRestore()
		{
			auto* scene = CScene::_pinstance;
			auto* scheduler = CLevelScheduler::gThis;
			auto* pause = CScene::ptable.g_PauseManager_00451688;
			return !restorePending && !hotkeyTaskPending && !autoLoadTaskPending && autoLoadState != AutoLoadState::Loading &&
				scene && scheduler && pause && pause->pSimpleMenu && pause->pSplashScreen &&
				gSaveManagement.pBigAlloc_0x34 && !gSaveManagement.has_queued_file_action() &&
				scheduler->currentLevelID >= 0 && scheduler->currentLevelID <= 0xe &&
				scheduler->nextLevelID == 0x10 && !scene->IsFadeTermActive() && !(GameFlags & GAME_REQUEST_TERM);
		}

		void QueueRestore(const SaveBackup& backup, bool load)
		{
			// Capture the displayed bytes: autosave may rotate the backup filenames before execution.
			restorePending = true;
			backupStatus = "Restore queued";
			EnqueueLevelManageTask([backup, load]() {
				restorePending = false;
				if (!CanRestore()) {
					backupStatus = "Restore cancelled: save/load or level transition in progress";
					return;
				}
				restorePending = true;
				try {
					// Reuse atomic save replacement and preserve the current slot in the backup rotation.
					WinSaveFile output(backup.destination, 0x2e);
					if (!output.IsOpen() || !output.Write(backup.data.data(), static_cast<uint>(backup.data.size())) || !output.Commit()) {
						backupStatus = "Restore failed: could not back up or replace the current slot";
					}
					else {
						gSaveManagement.read_slot_info(backup.slot);
						gSaveManagement.fileExistsFlags |= 0x20u << backup.slot;
						backupStatus = "Restored slot " + std::to_string(backup.slot);
						if (load) {
							MemCardLoad0(backup.slot);
							backupStatus += CLevelScheduler::gThis->bShouldLoad && CLevelScheduler::gThis->nextLevelID != 0x10
								? "; load requested" : "; load failed";
						}
					}
				}
				catch (const std::exception& error) {
					backupStatus = std::string("Restore failed: ") + error.what();
				}
				restorePending = false;
				refreshBackups = true;
			});
		}

		void ShowBackups()
		{
			if (!backupsOpen) return;
			if (refreshBackups && !restorePending) RefreshBackups();
			ImGui::SetNextWindowSize(ImVec2(620, 550), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Save Backups", &backupsOpen)) {
				ImGui::BeginDisabled(restorePending);
				if (ImGui::Button("Refresh")) RefreshBackups();
				ImGui::EndDisabled();
				ImGui::TextWrapped("Newest first in each slot. Restore replaces that slot and backs up its current save.");
				if (!backupDirectory.empty()) ImGui::TextWrapped("Directory: %s", backupDirectory.c_str());
				if (!backupStatus.empty()) ImGui::TextWrapped("%s", backupStatus.c_str());
				if (!CanRestore()) ImGui::TextWrapped("Restore is available when the title screen or gameplay is ready and no save/load is active.");
				if (backups.empty()) ImGui::TextUnformatted("No backed up saves found. Autosaves create backups of existing slots.");
				ImGui::BeginChild("Backups", ImVec2(0, 0), true);
				for (const auto& backup : backups) {
					ImGui::PushID(backup.slot * 10 + backup.index);
					ImGui::Text("Slot %d - backup %d%s", backup.slot, backup.index, backup.index == 1 ? " (newest)" : "");
					if (backup.modified.wYear) {
						const auto& time = backup.modified;
						ImGui::Text("Modified: %04d-%02d-%02d %02d:%02d:%02d (local)",
							time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
					}
					if (backup.error.empty()) {
						DrawSaveInfo(backup.desc);
						DrawCheckpoint(backup);
					}
					else ImGui::TextWrapped("%s", backup.error.c_str());
					ImGui::BeginDisabled(!backup.error.empty() || !CanRestore());
					if (ImGui::Button("Restore")) QueueRestore(backup, false);
					ImGui::SameLine();
					if (ImGui::Button("Restore & Load")) QueueRestore(backup, true);
					ImGui::EndDisabled();
					ImGui::Separator();
					ImGui::PopID();
				}
				ImGui::EndChild();
			}
			ImGui::End();
		}

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
		if (ImGui::Button("Browse Backed Up Saves")) {
			backupsOpen = true;
			refreshBackups = true;
		}

		ImGui::BeginChild("SaveLoadSlots", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

		// Show the 4 slots in a list
		for (int i = 0; i < 4; i++) {
			Debug::SaveLoad::DrawSlot(i);
			ImGui::Separator();
		}

		ImGui::EndChild();
	}

	ImGui::End();
	ShowBackups();
}

void Debug::SaveLoad::Update()
{
	if (restorePending || hotkeyTaskPending) return;
	UpdateAutoLoad();
	// Save loading can render nested debug frames. Do not append hotkey tasks to
	// the scheduler's queue while the auto-load callback is executing.
	if (autoLoadState == AutoLoadState::Loading) {
		return;
	}

	// Listen for F5 and F7 key presses
	if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
		const int slotId = gSaveManagement.slotID_0x28 >= 0 ? gSaveManagement.slotID_0x28 : gDefaultSaveSlot;
		hotkeyTaskPending = true;
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
			hotkeyTaskPending = false;
			});
	}

	if (!hotkeyTaskPending && ImGui::IsKeyPressed(ImGuiKey_F7)) {
		const int slotId = gSaveManagement.slotID_0x28 >= 0 ? gSaveManagement.slotID_0x28 : gDefaultSaveSlot;
		hotkeyTaskPending = true;
		EnqueueLevelManageTask([slotId]() {
			CScene::_pinstance->SetGlobalPaused_001b8c30(1);
			MemCardLoad0(slotId);
			hotkeyTaskPending = false;
			});
	}
}

namespace Debug {
	MenuRegisterer sDebugSaveLoadMenuReg("Save/Load", Debug::SaveLoad::ShowMenu, true);
	UpdateRegisterer sDebugSaveLoadUpdateReg(Debug::SaveLoad::Update);
}
