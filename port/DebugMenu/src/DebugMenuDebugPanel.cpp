#include "DebugMenuDebugPanel.h"
#include "DebugMenuLog.h"
#include "DebugMenu.h"

#include <imgui.h>

#include "DebugRendering.h"
#include "DebugCollision.h"
#include "DebugFrameBuffer.h"
#include "DebugHeroReplay.h"
#include "Native/NativeRenderer.h"
#include "TimeController.h"
#include "Actor.h"

namespace Debug {

	static bool gShowDebugPanel = false;

	bool GetShowDebugPanel() { return gShowDebugPanel; }
	void SetShowDebugPanel(bool bShow) { gShowDebugPanel = bShow; }

	static constexpr const char* kDebugWindowName = "Debug";

	static void DrawAudioContents() {
		auto* pAudioManager = CScene::ptable.g_AudioManager_00451698;
		if (pAudioManager == nullptr) {
			ImGui::TextDisabled("Audio manager unavailable.");
			return;
		}

		ImGui::Text("Audio Manager: 0x%p", pAudioManager);
		ImGui::Text("field_0xcc: %.3f", pAudioManager->field_0xcc);
		ImGui::Separator();
		ImGui::TextWrapped("Audio debugging has not been split into a dedicated panel yet. This tab is the new home for audio-specific tools.");
	}

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
				DrawAudioContents();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		ImGui::End();
	}

} // namespace Debug
