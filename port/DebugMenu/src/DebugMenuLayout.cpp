#include "DebugMenuLayout.h"
#include "DebugMenu.h"
#include "DebugRenderer.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <string>

namespace Debug {

	static constexpr float kGameAspectRatio = 640.0f / 480.0f;
	static constexpr const char* kGameViewportWindowName = "GameViewport";

	static ImGuiID gLeftDockId = 0;
	static ImGuiID gRightDockId = 0;
	static ImGuiID gBottomDockId = 0;
	static ImGuiID gCenterDockId = 0;

	static bool gResetDockLayout = false;
	static bool gDockLayoutInitialized = false;
	static ImVec2 gLastGameViewportImageSize = ImVec2(0, 0);

	ImGuiID GetRightDockId() { return gRightDockId; }
	ImGuiID GetBottomDockId() { return gBottomDockId; }

	void RequestResetDockLayout() { gResetDockLayout = true; }

	bool HasSavedWindowSettings(const char* pWindowName) {
		return ImGui::FindWindowSettingsByID(ImHashStr(pWindowName)) != nullptr;
	}

	void BuildDefaultDockLayout(ImGuiID dockspaceId) {
		ImGuiDockNode* pExistingNode = ImGui::DockBuilderGetNode(dockspaceId);
		if (!gResetDockLayout) {
			if (gDockLayoutInitialized) {
				return;
			}

			if (pExistingNode != nullptr && (pExistingNode->IsSplitNode() || pExistingNode->Windows.Size > 0)) {
				gDockLayoutInitialized = true;
				gCenterDockId = dockspaceId;
				return;
			}
		}

		gResetDockLayout = false;

		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, pViewport->WorkSize);

		ImGuiID centerId = dockspaceId;
		ImGuiID leftId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Left, 0.20f, nullptr, &centerId);
		ImGuiID rightId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Right, 0.25f, nullptr, &centerId);
		ImGuiID bottomId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Down, 0.25f, nullptr, &centerId);
		gLeftDockId = leftId;
		gRightDockId = rightId;
		gBottomDockId = bottomId;
		gCenterDockId = centerId;

		ImGui::DockBuilderDockWindow("World", leftId);
		ImGui::DockBuilderDockWindow("Inspector", rightId);
		ImGui::DockBuilderDockWindow(kGameViewportWindowName, centerId);
		ImGui::DockBuilderDockWindow("Debug", bottomId);
		ImGui::DockBuilderDockWindow("Camera", rightId);
		ImGui::DockBuilderDockWindow("Scene", bottomId);

		ImGui::DockBuilderFinish(dockspaceId);
		gDockLayoutInitialized = true;
	}

	static ImGuiID GetPreferredDockIdForMenu(const std::string& menuName) {
		if (menuName == "Log" || menuName == "Framebuffer" || menuName == "Framebuffers" ||
			menuName == "Rendering" || menuName == "Memory" || menuName == "Texture" ||
			menuName == "Mesh" || menuName == "Collision" || menuName == "Hero Replay" ||
			menuName == "Cutscene" || menuName == "Demo") {
			return gBottomDockId;
		}

		if (menuName == "Actor" || menuName == "Hero" || menuName == "Checkpoint" ||
			menuName == "Event" || menuName == "Shop" || menuName == "Wolfen" ||
			menuName == "Scenario" || menuName == "Save/Load" || menuName == "Scene" ||
			menuName == "Tutorial" || menuName == "Input" || menuName == "Sector" ||
			menuName == "Level Scheduler") {
			return gRightDockId;
		}

		return gBottomDockId;
	}

	static void DrawGameViewportImage() {
		static ImTextureID gFrameBuffer = DebugMenu::AddNativeFrameBuffer();
		DebugMenu::RefreshNativeFrameBuffer(gFrameBuffer);

		const ImVec2 available = ImGui::GetContentRegionAvail();
		if (available.x <= 0.0f || available.y <= 0.0f) {
			return;
		}

		ImVec2 imageSize = available;
		if ((imageSize.x / imageSize.y) > kGameAspectRatio) {
			imageSize.x = imageSize.y * kGameAspectRatio;
		}
		else {
			imageSize.y = imageSize.x / kGameAspectRatio;
		}

		gLastGameViewportImageSize = imageSize;

		const ImVec2 cursorPos = ImGui::GetCursorPos();
		const ImVec2 centeredCursor(
			cursorPos.x + (available.x - imageSize.x) * 0.5f,
			cursorPos.y + (available.y - imageSize.y) * 0.5f);
		ImGui::SetCursorPos(centeredCursor);
		ImGui::Image(gFrameBuffer, imageSize);
	}

	void DrawGameViewportWindow() {
		ImGui::Begin(kGameViewportWindowName, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		DrawGameViewportImage();
		ImGui::End();
	}

	void DrawFullscreenGameViewportWindow() {
		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(pViewport->Pos);
		ImGui::SetNextWindowSize(pViewport->Size);
		ImGui::SetNextWindowViewport(pViewport->ID);
		ImGui::Begin(
			"GameViewportFullscreen",
			nullptr,
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoInputs);
		DrawGameViewportImage();
		ImGui::End();
	}

	void DrawLegacyMenus() {
		for (auto& menu : Debug::MenuRegisterer::GetMenus()) {
			if (!menu.GetOpen()) {
				continue;
			}

			const ImGuiID preferredDockId = GetPreferredDockIdForMenu(menu.name);
			if (preferredDockId != 0 && !HasSavedWindowSettings(menu.name.c_str())) {
				ImGui::SetNextWindowDockID(preferredDockId, ImGuiCond_FirstUseEver);
			}

			menu.Show();
		}
	}

	ImVec2 GetGameViewportImageSize() {
		return gLastGameViewportImageSize;
	}

} // namespace Debug
