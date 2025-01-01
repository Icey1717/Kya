#include "DebugCinematic.h"

#include <imgui.h>

#include "CinematicManager.h"
#include "DebugSetting.h"
#include "TimeController.h"
#include "Actor.h"

namespace Debug::Cinematic
{
	static bool GetCutsceneName(void* pData, int index, const char** ppOut) {
		auto* options = static_cast<std::vector<std::string>*>(pData);
		*ppOut = ((*options)[index]).c_str();
		return true;
	}

	static Debug::Setting<bool> gSkipCutscenes = { "Skip Cutscenes", false };
	static Debug::Setting<float> gCustomCutsceneTime = { "Custom Cutscene Time", 0.0f };

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

		ImGui::Text("Trigger Actor: %s", pCinematic->triggerActorRef.index >= 0 ? pCinematic->triggerActorRef.Get()->name : "None");

		if (pCinematic->cineBankLoadStage_0x2b4 == 4 && pCinematic->cinFileData.pCinTag && pCinematic->state != CS_Stopped) {
			// Play/Pause button
			if (pCinematic->state != CS_Stopped)
			{
				if (ImGui::Button("Pause"))
				{
					pCinematic->state = CS_Stopped;
				}
			}
			else
			{
				if (ImGui::Button("Play"))
				{
					pCinematic->state = CS_Playing;
				}
			}

			auto& currentTime = pCinematic->totalCutsceneDelta;
			auto& totalTime = ((pCinematic->cinFileData).pCinTag)->totalPlayTime;

			// Jump to beginning button
			if (ImGui::Button("Jump to Beginning"))
			{
				currentTime = 0.0f;
				// Seek to the beginning of the video here
			}

			// Jump to end button
			if (ImGui::Button("Jump to End") || ImGui::IsKeyPressed(ImGuiKey_F9))
			{
				currentTime = totalTime;
				// Seek to the end of the video here
			}

			// Current time and total time labels
			ImGui::Text("Time: %.2f / %.2f", currentTime, totalTime);

			// Seek bar for video playback
			ImGui::SliderFloat("##seekbar", &currentTime, 0.0f, totalTime);

			static bool bCutsceneStepEnabled = false;
			ImGui::Checkbox("Custom Time Control", &bCutsceneStepEnabled);

			if (bCutsceneStepEnabled) {
				gCustomCutsceneTime.DrawImguiControl();
				static float cutsceneStepTime = 0.0f;
				pCinematic->totalCutsceneDelta = gCustomCutsceneTime + cutsceneStepTime;

				pCinematic->totalCutsceneDelta = std::clamp(pCinematic->totalCutsceneDelta, 0.0f, totalTime - 1.0f);

				if (ImGui::Button("Set to current")) {
					gCustomCutsceneTime = gCustomCutsceneTime + cutsceneStepTime;
					cutsceneStepTime = 0.0f;
				}

				if (ImGui::Button("<<")) {
					auto pTimer = Timer::GetTimer();
					cutsceneStepTime -= pTimer->cutsceneDeltaTime;
				}
				ImGui::SameLine();
				if (ImGui::Button(">>")) {
					auto pTimer = Timer::GetTimer();
					cutsceneStepTime += pTimer->cutsceneDeltaTime;
				}

			}
		}
		else {
			// Jump to end button
			if (ImGui::Button("Start"))
			{
				pCinematic->Start();
				// Seek to the end of the video here
			}
		}
	}

	void ShowMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Video Player Controls", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::CollapsingHeader("Persistent Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			gSkipCutscenes.DrawImguiControl();
		}

		auto* pCinematicManager = g_CinematicManager_0048efc;

		if (ImGui::CollapsingHeader("Selector", ImGuiTreeNodeFlags_DefaultOpen)) {
			static int selectedCutsceneId = 0;
			static bool bSelectedFirstPlaying = false;

			if (pCinematicManager->activeCinematicCount > 0) {
				if (selectedCutsceneId > pCinematicManager->activeCinematicCount) {
					selectedCutsceneId = 0;
				}

				std::vector<std::string> options;

				for (int i = 0; i < pCinematicManager->activeCinematicCount; i++) {
					options.emplace_back(std::to_string(i));
				}

				// Create the dropdown box
				if (ImGui::Combo("Select an Option", &selectedCutsceneId, GetCutsceneName, &options, pCinematicManager->activeCinematicCount, -1))
				{
					// Handle the selected option change here
					// The selected option index will be stored in 'selectedOption'
					// You can perform actions based on the selected option.
				}

				if (!bSelectedFirstPlaying) {
					for (int i = 0; i < pCinematicManager->activeCinematicCount; i++) {
						if (pCinematicManager->ppCinematicObjB_B[i]->state == CS_Playing) {
							selectedCutsceneId = i;
							bSelectedFirstPlaying = true;
						}
					}
				}

				//ShowDetails(pCinematicManager->ppCinematicObjB_B[selectedCutsceneId]);
			}
		}

		if (ImGui::CollapsingHeader("All Active", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (pCinematicManager->activeCinematicCount > 0) {
				for (int i = 0; i < pCinematicManager->activeCinematicCount; i++) {
					auto* pCinematic = pCinematicManager->ppCinematicObjB_B[i];
					char buffer[512];
					sprintf(buffer, "%d - %s", i, pCinematic->fileName);
					if (ImGui::CollapsingHeader(buffer) || pCinematic->state == CS_Playing) {
						ShowDetails(pCinematic);
					}

					if (gSkipCutscenes) {
						if (pCinematic->state == CS_Playing) {
							pCinematic->totalCutsceneDelta = ((pCinematic->cinFileData).pCinTag)->totalPlayTime;
						}
					}
				}
			}
		}

		// End the ImGui window
		ImGui::End();
	}
}