#include "DebugMenuToolbar.h"
#include "DebugMenuWorld.h"
#include "DebugMenuDebugPanel.h"
#include "DebugMenuLayout.h"
#include "DebugMenu.h"
#include "DebugCamera.h"

#include <profiling.h>
#include <imgui.h>
#include <cstdio>

#include "SectorManager.h"
#include "LevelScheduler.h"
#include "TimeController.h"
#include "Actor.h"

namespace Debug {
	static void ShowLevelSchedulerMenu(bool* bOpen) {
		ImGui::Begin("Level Scheduler", bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Boomy Level: %d", CLevelScheduler::GetBoomyLevel());
		ImGui::Text("Medallion Level: %d", CLevelScheduler::GetMedallionLevel());
		ImGui::Text("Fight Level: %d", CLevelScheduler::GetFightLevel());
		ImGui::InputInt("Current Level", &CLevelScheduler::gThis->currentLevelID);
		ImGui::End();
	}

	static bool gShowCameraWindow = false;
	static int gToolbarLevelId = 0;
	static int gToolbarSectorId = 0;
	static int gStepFramesPending = 0;
	static float gResumeTimeScale = 1.0f;

	static float gMinisculeTimeScale = 0.00001f;

	bool GetShowCameraWindow() { return gShowCameraWindow; }

	void UpdateSingleStepState() {
		if (gStepFramesPending <= 0) {
			return;
		}

		--gStepFramesPending;
		if (gStepFramesPending == 0) {
			if (auto* pTimer = GetTimer(); pTimer != nullptr) {
				pTimer->timeScale = gMinisculeTimeScale;
			}
		}
	}

	void DrawToolbar() {
		ZONE_SCOPED;
		auto* pSectorManager = CScene::ptable.g_SectorManager_00451670;
		auto* pTimer = GetTimer();
		const double deltaTime = DebugMenu::GetDeltaTime();
		const double fps = deltaTime > 0.0 ? (1.0 / deltaTime) : 0.0;

		if (CLevelScheduler::gThis != nullptr) {
			gToolbarLevelId = CLevelScheduler::gThis->currentLevelID;
		}
		if (pSectorManager != nullptr) {
			gToolbarSectorId = pSectorManager->baseSector.currentSectorID;
		}

		if (ImGui::BeginMainMenuBar()) {
			ZONE_SCOPED_NAME("MenuBar");
			bool bPaused = pTimer != nullptr && pTimer->timeScale <= gMinisculeTimeScale;

			bool bWorld = Debug::GetShowWorldPanel();
			bool bInspector = Debug::GetShowInspectorPanel();
			bool bDebug = Debug::GetShowDebugPanel();

			if (ImGui::Checkbox("World", &bWorld)) {
				Debug::SetShowWorldPanel(bWorld);
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Inspector", &bInspector)) {
				Debug::SetShowInspectorPanel(bInspector);
			}
			ImGui::SameLine();
			if (ImGui::Checkbox("Debug", &bDebug)) {
				Debug::SetShowDebugPanel(bDebug);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset Layout")) {
				Debug::RequestResetDockLayout();
			}

			ImGui::SameLine();
			ImGui::Separator();
			ImGui::SameLine();
			if (ImGui::BeginMenu("Tools")) {
				ZONE_SCOPED_NAME("Tools");
				ImGui::MenuItem("Camera", nullptr, &gShowCameraWindow);
				ImGui::Separator();

				for (auto& menu : Debug::MenuRegisterer::GetMenus()) {
					bool bOpen = menu.GetOpen();
					if (ImGui::MenuItem(menu.name.c_str(), nullptr, &bOpen)) {
						menu.SetOpen(bOpen);
					}
				}

				ImGui::EndMenu();
			}

			ImGui::SameLine();
			ImGui::Separator();
			ImGui::SameLine();

			if (pTimer != nullptr) {
				// Keep the most recent non-zero value so resume restores the previous simulation speed.
				if (!bPaused && pTimer->timeScale > gMinisculeTimeScale) {
					gResumeTimeScale = pTimer->timeScale;
				}

				if (ImGui::SmallButton(bPaused ? "Unpause" : "Pause")) {
					if (bPaused) {
						if (gResumeTimeScale <= gMinisculeTimeScale) {
							gResumeTimeScale = 1.0f;
						}
						pTimer->timeScale = gResumeTimeScale;
					}
					else {
						if (pTimer->timeScale > gMinisculeTimeScale) {
							gResumeTimeScale = pTimer->timeScale;
						}
						pTimer->timeScale = gMinisculeTimeScale;
					}
					bPaused = pTimer->timeScale <= gMinisculeTimeScale;
				}

				ImGui::SameLine();
				float toolbarTimeScale = bPaused ? gResumeTimeScale : pTimer->timeScale;
				ImGui::SetNextItemWidth(155.0f);
				if (ImGui::InputFloat("Time Scale##Toolbar", &toolbarTimeScale, 0.05f, 0.25f, "%.3f")) {
					if (toolbarTimeScale < gMinisculeTimeScale) {
						toolbarTimeScale = gMinisculeTimeScale;
					}

					if (bPaused) {
						gResumeTimeScale = toolbarTimeScale;
					}
					else {
						pTimer->timeScale = toolbarTimeScale;
						if (toolbarTimeScale > gMinisculeTimeScale) {
							gResumeTimeScale = toolbarTimeScale;
						}
						bPaused = pTimer->timeScale <= gMinisculeTimeScale;
					}
				}
			}
			else {
				ImGui::TextDisabled("Timer unavailable");
			}

			ImGui::SameLine();
			ImGui::Separator();
			ImGui::SameLine();
			ImGui::Text("FPS %.1f | %s", fps, bPaused ? "Paused" : "Running");

			ImGui::Separator();
			ImGui::SameLine();

			// Level and sector display
			ImGui::Text("Level: 0x%x | Sector: 0x%x", gToolbarLevelId, gToolbarSectorId);

			ImGui::EndMainMenuBar();
		}
	}

	void DrawCameraWindow() {
		if (!gShowCameraWindow) {
			return;
		}

		ZONE_SCOPED;

		if (GetRightDockId() != 0 && !HasSavedWindowSettings("Camera")) {
			ImGui::SetNextWindowDockID(GetRightDockId(), ImGuiCond_FirstUseEver);
		}
		Debug::Camera::ShowCamera();
	}

} // namespace Debug

namespace {
	Debug::MenuRegisterer sDemoMenuReg("Demo", ImGui::ShowDemoWindow);
	Debug::MenuRegisterer sLevelSchedulerMenuReg("Level Scheduler", Debug::ShowLevelSchedulerMenu, true);
}
