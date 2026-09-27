#include "DebugMenuDebugPanel.h"
#include "DebugMenuLog.h"
#include "DebugMenu.h"

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
		const double deltaTime = DebugMenu::GetDeltaTime();
		const double fps = deltaTime > 0.0 ? (1.0 / deltaTime) : 0.0;
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0);
		ImGui::Separator();
		ImGui::Text("Render Time: %.1f ms", Renderer::Native::GetRenderTime());
		ImGui::Text("Render Wait Time: %.1f ms", Renderer::Native::GetRenderWaitTime());
		ImGui::Text("Render Thread Time: %.1f ms", Renderer::Native::GetRenderThreadTime());

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
