#include "DebugMenu.h"
#include "DebugCinematic.h"

#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "CinematicManager.h"
#include "DebugSetting.h"
#include "TimeController.h"
#include "Actor.h"

namespace Debug::Cinematic
{
	static Debug::Setting<bool> gSkipCutscenes = { "Skip Cutscenes", false };
	static Debug::Setting<bool> gShowSaveContextWindow = { "Show Save Context", false };

	static const char* GetStateName(ECinematicState state) {
		switch (state) {
		case CS_Playing:     return "Playing";
		case CS_Interpolate: return "Interpolate";
		case CS_Stopped:     return "Stopped";
		default:             return "Unknown";
		}
	}

	static ImVec4 GetStateColor(ECinematicState state) {
		switch (state) {
		case CS_Playing:     return ImVec4(0.2f, 0.85f, 0.2f, 1.0f);
		case CS_Interpolate: return ImVec4(0.9f, 0.85f, 0.1f, 1.0f);
		case CS_Stopped:     return ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
		default:             return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	struct CinematicStepState {
		bool bEnabled = false;
		float baseTime = 0.0f;
		float stepOffset = 0.0f;
	};

	struct SaveContextPreviewEntry {
		int saveIndex = -1;
		int cinematicIndex = -1;
		const char* pFileName = "<unnamed>";
		ECinematicState state = CS_Stopped;
		uint bitFlags = 0;
		uint saveFlags = 0;
		float totalCutsceneDelta = 0.0f;
	};

	static std::unordered_map<CCinematic*, CinematicStepState> sStepStates;
	static std::unordered_map<CCinematic*, ECinematicState> sPrevStates;

	static uint BuildSaveContextFlags(const CCinematic* pCinematic) {
		uint flags = 0;
		if ((pCinematic->flags_0x8 & 0x400) != 0) {
			flags |= 2;
		}
		if (pCinematic->state != CS_Stopped) {
			flags |= 1;
		}
		if ((pCinematic->flags_0x8 & 0x20) != 0) {
			flags |= 4;
		}
		if ((pCinematic->flags_0x8 & 0x40) != 0) {
			flags |= 8;
		}
		if ((pCinematic->flags_0x8 & 0x1000) != 0) {
			flags |= 0x10;
		}
		return flags;
	}

	static void ShowSaveContextWindow(bool* bOpen, CCinematicManager* pCinematicManager)
	{
		ImGui::SetNextWindowSize(ImVec2(1100.0f, 420.0f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Cinematic Save Context", bOpen)) {
			ImGui::End();
			return;
		}

		if (pCinematicManager == nullptr) {
			ImGui::TextDisabled("Cinematic manager unavailable.");
			ImGui::End();
			return;
		}

		std::vector<SaveContextPreviewEntry> entries;
		entries.reserve(pCinematicManager->numCutscenes_0x8);

		int saveIndex = 0;
		for (int i = 0; i < pCinematicManager->numCutscenes_0x8; i++) {
			CCinematic* pCinematic = pCinematicManager->ppCinematicObjB_A[i];
			if ((pCinematic == nullptr) || ((pCinematic->flags_0x4 & 0x40) == 0)) {
				continue;
			}

			SaveContextPreviewEntry entry;
			entry.saveIndex = saveIndex++;
			entry.cinematicIndex = i;
			entry.pFileName = (pCinematic->fileName != nullptr) ? pCinematic->fileName : "<unnamed>";
			entry.state = pCinematic->state;
			entry.bitFlags = *reinterpret_cast<const uint*>(&pCinematic->field_0x10);
			entry.saveFlags = BuildSaveContextFlags(pCinematic);
			entry.totalCutsceneDelta = pCinematic->totalCutsceneDelta;
			entries.push_back(entry);
		}

		ImGui::Text("BLCI entries: %d / %d cutscenes", static_cast<int>(entries.size()), pCinematicManager->numCutscenes_0x8);
		ImGui::TextDisabled("Matches CCinematicManager::Level_SaveContext serialization.");
		ImGui::TextDisabled("Included when (flags_0x4 & 0x40) != 0.");
		ImGui::Separator();

		if (entries.empty()) {
			ImGui::TextDisabled("No entries would be serialized right now.");
			ImGui::End();
			return;
		}

		const ImGuiTableFlags tableFlags =
			ImGuiTableFlags_Borders |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_ScrollY |
			ImGuiTableFlags_ScrollX |
			ImGuiTableFlags_Resizable;

		if (ImGui::BeginTable("##BLCISaveData", 12, tableFlags, ImVec2(0.0f, 300.0f))) {
			ImGui::TableSetupColumn("Save #", ImGuiTableColumnFlags_WidthFixed, 60.0f);
			ImGui::TableSetupColumn("Cutscene #", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 90.0f);
			ImGui::TableSetupColumn("field_0x10", ImGuiTableColumnFlags_WidthFixed, 95.0f);
			ImGui::TableSetupColumn("Save Flags", ImGuiTableColumnFlags_WidthFixed, 85.0f);
			ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("state!=stopped", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("f8&0x400", ImGuiTableColumnFlags_WidthFixed, 85.0f);
			ImGui::TableSetupColumn("f8&0x20", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("f8&0x40", ImGuiTableColumnFlags_WidthFixed, 80.0f);
			ImGui::TableSetupColumn("f8&0x1000", ImGuiTableColumnFlags_WidthFixed, 90.0f);
			ImGui::TableHeadersRow();

			for (const SaveContextPreviewEntry& entry : entries) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", entry.saveIndex);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%d", entry.cinematicIndex);
				ImGui::TableSetColumnIndex(2);
				ImGui::TextUnformatted(entry.pFileName);
				ImGui::TableSetColumnIndex(3);
				ImGui::TextColored(GetStateColor(entry.state), "%s", GetStateName(entry.state));
				ImGui::TableSetColumnIndex(4);
				ImGui::Text("0x%08X", entry.bitFlags);
				ImGui::TableSetColumnIndex(5);
				ImGui::Text("0x%02X", entry.saveFlags);
				ImGui::TableSetColumnIndex(6);
				ImGui::Text("%.2f", entry.totalCutsceneDelta);
				ImGui::TableSetColumnIndex(7);
				ImGui::TextUnformatted((entry.saveFlags & 0x1) ? "Y" : "-");
				ImGui::TableSetColumnIndex(8);
				ImGui::TextUnformatted((entry.saveFlags & 0x2) ? "Y" : "-");
				ImGui::TableSetColumnIndex(9);
				ImGui::TextUnformatted((entry.saveFlags & 0x4) ? "Y" : "-");
				ImGui::TableSetColumnIndex(10);
				ImGui::TextUnformatted((entry.saveFlags & 0x8) ? "Y" : "-");
				ImGui::TableSetColumnIndex(11);
				ImGui::TextUnformatted((entry.saveFlags & 0x10) ? "Y" : "-");
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}

	static void ShowDetails(CCinematic* pCinematic)
	{
		if (pCinematic->zoneRefA.index >= 0 && pCinematic->zoneRefA.Get() && ImGui::CollapsingHeader("Zone A")) {
			ed_zone_3d* pZone = pCinematic->zoneRefA.Get();
			ImGui::Text("Bounding Sphere: %s", pZone->boundSphere.ToString().c_str());
			ImGui::Text("Zone Flags: 0x%x", pZone->flags);
		}

		if (pCinematic->zoneRefB.index >= 0 && pCinematic->zoneRefB.Get() && ImGui::CollapsingHeader("Zone B")) {
			ed_zone_3d* pZone = pCinematic->zoneRefB.Get();
			ImGui::Text("Bounding Sphere: %s", pZone->boundSphere.ToString().c_str());
			ImGui::Text("Zone Flags: 0x%x", pZone->flags);
		}

		if (pCinematic->zoneRefC.index >= 0 && pCinematic->zoneRefC.Get() && ImGui::CollapsingHeader("Zone C")) {
			ed_zone_3d* pZone = pCinematic->zoneRefC.Get();
			ImGui::Text("Bounding Sphere: %s", pZone->boundSphere.ToString().c_str());
			ImGui::Text("Zone Flags: 0x%x", pZone->flags);
		}

		ImGui::Text("Trigger: %s",
			(pCinematic->triggerActorRef.index >= 0 && pCinematic->triggerActorRef.Get())
			? pCinematic->triggerActorRef.Get()->name : "None");

		if (pCinematic->cineBankLoadStage_0x2b4 == 4 && pCinematic->cinFileData.pCinTag) {
			auto& currentTime = pCinematic->totalCutsceneDelta;
			float totalTime = pCinematic->cinFileData.pCinTag->totalPlayTime;

			bool bPlaying = (pCinematic->state == CS_Playing);
			if (ImGui::Button(bPlaying ? "Pause" : "Play")) {
				pCinematic->state = bPlaying ? CS_Stopped : CS_Playing;
			}
			ImGui::SameLine();
			if (ImGui::Button("|<")) { currentTime = 0.0f; }
			ImGui::SameLine();
			if (ImGui::Button(">|")) { currentTime = totalTime; }

			ImGui::SetNextItemWidth(-1.0f);
			ImGui::SliderFloat("##seekbar", &currentTime, 0.0f, totalTime, "%.2fs");

			auto& stepState = sStepStates[pCinematic];
			bool bWasEnabled = stepState.bEnabled;
			ImGui::Checkbox("Frame Step", &stepState.bEnabled);

			if (stepState.bEnabled) {
				if (!bWasEnabled) {
					stepState.baseTime = currentTime;
					stepState.stepOffset = 0.0f;
				}

				pCinematic->totalCutsceneDelta = std::clamp(stepState.baseTime + stepState.stepOffset, 0.0f, totalTime - 1.0f);

				if (ImGui::Button("<<")) { stepState.stepOffset -= Timer::GetTimer()->cutsceneDeltaTime; }
				ImGui::SameLine();
				if (ImGui::Button(">>")) { stepState.stepOffset += Timer::GetTimer()->cutsceneDeltaTime; }
				ImGui::SameLine();
				if (ImGui::Button("Commit")) {
					stepState.baseTime += stepState.stepOffset;
					stepState.stepOffset = 0.0f;
				}
				ImGui::SameLine();
				ImGui::TextDisabled("offset: %+.3fs", stepState.stepOffset);
			} else {
				stepState.baseTime = currentTime;
				stepState.stepOffset = 0.0f;
			}
		} else {
			bool bBankReady = (pCinematic->cineBankLoadStage_0x2b4 == 4);
			ImGui::BeginDisabled(!bBankReady);
			if (ImGui::Button("Start")) {
				pCinematic->Start();
			}
			ImGui::EndDisabled();
			if (!bBankReady) {
				ImGui::SameLine();
				ImGui::TextDisabled("(loading, stage %d)", pCinematic->cineBankLoadStage_0x2b4);
			}
		}
	}

	void ShowMenu(bool* bOpen) {
		ImGui::Begin("Cinematics", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		gSkipCutscenes.DrawImguiControl();
		gShowSaveContextWindow.DrawImguiControl();

		auto* pCinematicManager = g_CinematicManager_0048efc;
		if (pCinematicManager == nullptr) {
			ImGui::TextDisabled("Cinematic manager unavailable.");
			ImGui::End();
			return;
		}

		if (pCinematicManager->activeCinematicCount == 0) {
			ImGui::TextDisabled("No active cinematics.");
		}
		else {
			ImGui::Separator();

			static char sFilterBuf[128] = {};
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::InputTextWithHint("##filter", "Filter by filename...", sFilterBuf, sizeof(sFilterBuf));
			ImGui::Spacing();

			// Sort indices so Playing comes first, then Interpolate, then Stopped.
			auto StatePriority = [](ECinematicState s) {
				switch (s) {
				case CS_Playing:     return 0;
				case CS_Interpolate: return 1;
				default:             return 2;
				}
			};
			std::vector<int> sortedIndices(pCinematicManager->activeCinematicCount);
			std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
			std::stable_sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b) {
				return StatePriority(pCinematicManager->ppCinematicObjB_B[a]->state)
					< StatePriority(pCinematicManager->ppCinematicObjB_B[b]->state);
			});

			for (int i : sortedIndices) {
				auto* pCinematic = pCinematicManager->ppCinematicObjB_B[i];

				if (sFilterBuf[0] != '\0') {
					bool bMatch = false;
					for (const char* h = pCinematic->fileName; *h; h++) {
						const char* n = sFilterBuf;
						const char* hh = h;
						while (*n && tolower((unsigned char)*hh) == tolower((unsigned char)*n)) { hh++; n++; }
						if (!*n) { bMatch = true; break; }
					}
					if (!bMatch) continue;
				}

				// Auto-expand the header when this cinematic transitions to Playing.
				ECinematicState prevState = sPrevStates[pCinematic];
				if (prevState != CS_Playing && pCinematic->state == CS_Playing) {
					ImGui::SetNextItemOpen(true, ImGuiCond_Always);
				}
				sPrevStates[pCinematic] = pCinematic->state;

				char headerLabel[512];
				sprintf(headerLabel, "[%s] %d - %s###cine_%d", GetStateName(pCinematic->state), i, pCinematic->fileName, i);

				ImGui::PushStyleColor(ImGuiCol_Text, GetStateColor(pCinematic->state));
				bool bExpanded = ImGui::CollapsingHeader(headerLabel);
				ImGui::PopStyleColor();

				if (bExpanded) {
					ImGui::PushID(i);
					ImGui::Indent();
					ShowDetails(pCinematic);
					ImGui::Unindent();
					ImGui::PopID();
				}

				if (gSkipCutscenes && pCinematic->state == CS_Playing && pCinematic->cinFileData.pCinTag) {
					pCinematic->totalCutsceneDelta = pCinematic->cinFileData.pCinTag->totalPlayTime;
				}
			}
		}

		ImGui::End();

		bool bSaveContextWindowWasOpen = gShowSaveContextWindow;
		bool& bSaveContextWindowOpen = gShowSaveContextWindow.operator bool&();
		if (bSaveContextWindowOpen) {
			ShowSaveContextWindow(&bSaveContextWindowOpen, pCinematicManager);
		}
		if (bSaveContextWindowWasOpen != bSaveContextWindowOpen) {
			gShowSaveContextWindow.UpdateValue();
		}
	}
}

namespace Debug {
    MenuRegisterer sDebugCinematicMenuReg("Cutscene", Debug::Cinematic::ShowMenu, true);
}

