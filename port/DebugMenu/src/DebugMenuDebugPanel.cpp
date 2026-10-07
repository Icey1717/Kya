#include "DebugMenuDebugPanel.h"
#include "DebugMenuLog.h"
#include "DebugMenu.h"
#include "DebugWatch.h"

#include <profiling.h>
#include <imgui.h>

#include "DebugRendering.h"
#include "DebugCollision.h"
#include "DebugFrameBuffer.h"
#include "DebugHeroReplay.h"
#include "DebugAudio.h"
#include "Native/NativeRenderer.h"
#include "TimeController.h"
#include "Actor.h"
#include "Audio.h"

namespace Debug {

	static bool gShowDebugPanel = false;

	bool GetShowDebugPanel() { return gShowDebugPanel; }
	void SetShowDebugPanel(bool bShow) { gShowDebugPanel = bShow; }

	static constexpr const char* kDebugWindowName = "Debug";

	static void DrawPerformanceContents() {
		Watch::DrawReadout("Performance.Fps");
		Watch::DrawReadout("Performance.Frame");
		ImGui::Separator();
		Watch::DrawReadout("Performance.Render");
		ImGui::Text("Render Wait Time: %.1f ms", Renderer::Native::GetRenderWaitTime());
		ImGui::Text("Render Thread Time: %.1f ms", Renderer::Native::GetRenderThreadTime());
		ImGui::Text("Alpha Slow Path Time: %.3f ms", Renderer::Native::GetAlphaTestSlowPathTime());
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Total CPU time recording per-triangle alpha-test replays in the last completed frame, including preview. GPU time is not measured.");

		if (auto* pTimer = GetTimer(); pTimer != nullptr) {
			ImGui::Separator();
			ImGui::Text("Timer Scale: %.3f", pTimer->timeScale);
			ImGui::Text("Scaled Total Time: %.3f", pTimer->scaledTotalTime);
			ImGui::Text("Total Play Time: %.3f", pTimer->totalPlayTime);
		}
	}

	void DrawDebugPanel() {
		if (!gShowDebugPanel) {
			return;
		}

		ZONE_SCOPED;

		ImGui::Begin(kDebugWindowName, &gShowDebugPanel);
		if (ImGui::BeginTabBar("DebugTabs")) {
			if (ImGui::BeginTabItem("Logs")) {
				DrawLogContents();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Performance")) {
				DrawPerformanceContents();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Rendering Debug")) {
				if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
					Debug::Rendering::DrawContents();
				}
				if (ImGui::CollapsingHeader("Collision", ImGuiTreeNodeFlags_DefaultOpen)) {
					Debug::Collision::DrawContents();
				}
				if (ImGui::CollapsingHeader("Framebuffer", ImGuiTreeNodeFlags_DefaultOpen)) {
					Debug::FrameBuffer::DrawContents();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Replay")) {
				Debug::HeroReplay::DrawContents();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Audio")) {
				Debug::Audio::DrawContents();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		ImGui::End();
	}

} // namespace Debug
