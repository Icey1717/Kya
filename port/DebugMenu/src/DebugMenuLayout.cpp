#include "DebugMenuLayout.h"
#include "DebugMenu.h"
#include "DebugRenderer.h"
#include "DebugSetting.h"
#include "DebugMenuWorld.h"
#include "DebugMenuDebugPanel.h"
#include "DebugMenuToolbar.h"

#include <profiling.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <string>
#include <unordered_map>

namespace Debug {
	static Setting<bool> gShowLeftDock("Workspace.Left", true);
	static Setting<bool> gShowRightDock("Workspace.Right", true);
	static Setting<bool> gShowBottomDock("Workspace.Bottom", true);
	static std::unordered_map<ImGuiID, DockRegion> gWindowRegions;

	static bool IsRegionVisible(DockRegion region)
	{
		switch (region) {
		case DockRegion::Left: return gShowLeftDock;
		case DockRegion::Right: return gShowRightDock;
		case DockRegion::Bottom: return gShowBottomDock;
		}
		return true;
	}

	void RevealDockWindow(const char* name, DockRegion fallback)
	{
		const auto it = gWindowRegions.find(ImHashStr(name));
		const DockRegion region = it != gWindowRegions.end() ? it->second : fallback;
		switch (region) {
		case DockRegion::Left: if (!gShowLeftDock) gShowLeftDock = true; break;
		case DockRegion::Right: if (!gShowRightDock) gShowRightDock = true; break;
		case DockRegion::Bottom: if (!gShowBottomDock) gShowBottomDock = true; break;
		}
	}

	bool ShouldDrawDockWindow(const char* name, DockRegion fallback)
	{
		const ImGuiID id = ImHashStr(name);
		const auto cached = gWindowRegions.find(id);
		// ImGui can detach inactive windows while collapsing an empty dock. Retain
		// their region while hidden instead of treating them as newly floating.
		if (cached != gWindowRegions.end() && !IsRegionVisible(cached->second)) return false;
		// Follow actual docking when users move panels, including restored layouts.
		auto* window = ImGui::FindWindowByName(name);
		auto* game = ImGui::FindWindowByName("GameViewport");
		if (window && !window->DockIsActive && window->DockId == 0) {
			// A panel deliberately undocked while visible is independent of its old region.
			gWindowRegions.erase(id);
			return true;
		}
		if (window && window->DockNode && game && game->DockNode) {
			const auto* center = game->DockNode;
			if (window->DockNode == center) return true;
			// Split ancestry stays stable when a hidden region has no visible bounds.
			for (auto* branch = window->DockNode; branch->ParentNode; branch = branch->ParentNode) {
				const auto* parent = branch->ParentNode;
				bool containsGame = false;
				for (auto* node = center; node; node = node->ParentNode) {
					if (node == parent) { containsGame = true; break; }
				}
				if (!containsGame) continue;
				fallback = parent->SplitAxis == ImGuiAxis_X
					? (parent->ChildNodes[0] == branch ? DockRegion::Left : DockRegion::Right)
					: DockRegion::Bottom;
				break;
			}
		}
		if (window && window->DockNode) gWindowRegions[id] = fallback;
		else if (cached != gWindowRegions.end()) fallback = cached->second;
		return IsRegionVisible(fallback);
	}

	void DrawWorkspaceMenu()
	{
		if (ImGui::BeginMenu("Workspace")) {
			bool left = gShowLeftDock, right = gShowRightDock, bottom = gShowBottomDock;
			if (ImGui::MenuItem("Left dock", nullptr, &left)) gShowLeftDock = left;
			ImGui::SetItemTooltip("Hide or restore the left dock without closing its panels.");
			if (ImGui::MenuItem("Right dock", nullptr, &right)) gShowRightDock = right;
			if (ImGui::MenuItem("Bottom dock", nullptr, &bottom)) gShowBottomDock = bottom;
			ImGui::Separator();
			if (ImGui::MenuItem("Show all docks")) {
				gShowLeftDock = true;
				gShowRightDock = true;
				gShowBottomDock = true;
			}
			if (ImGui::MenuItem("Hide all docks")) {
				gShowLeftDock = false;
				gShowRightDock = false;
				gShowBottomDock = false;
			}
			ImGui::TextDisabled("Open panels and docking are preserved.");
			ImGui::EndMenu();
		}
	}

	static constexpr float kGameAspectRatio = 640.0f / 480.0f;
	static constexpr const char* kGameViewportWindowName = "GameViewport";

	static ImGuiID gLeftDockId = 0;
	static ImGuiID gRightDockId = 0;
	static ImGuiID gBottomDockId = 0;
	static ImGuiID gCenterDockId = 0;

	static bool gResetDockLayout = false;
	static bool gDockLayoutInitialized = false;
	static int gActiveTask = 0;
	static int gPendingTask = -1;
	static Setting<int> gSavedActiveTask("Workspace.ActiveTask", 0);
	static Setting<nlohmann::json> gTaskWorkspaces("Workspace.Tasks", nlohmann::json::object());
	static const char* gTaskNames[] = { "Custom", "World inspection", "Hero debugging", "Replay" };
	static std::string gCustomIniFilename;
	static std::string gTaskIniFilename;

	void RequestTaskLayout(TaskLayout task) { gPendingTask = static_cast<int>(task); }
	TaskLayout GetTaskLayout() { return static_cast<TaskLayout>(gActiveTask); }

	static nlohmann::json CaptureWorkspace()
	{
		nlohmann::json state = {
			{ "ini", ImGui::SaveIniSettingsToMemory() },
			{ "left", bool(gShowLeftDock) }, { "right", bool(gShowRightDock) }, { "bottom", bool(gShowBottomDock) },
			{ "world", GetShowWorldPanel() }, { "inspector", GetShowInspectorPanel() },
			{ "debug", GetShowDebugPanel() }, { "camera", GetShowCameraWindow() }
		};
		for (auto& menu : MenuRegisterer::GetMenus()) state["menus"][menu.name] = menu.GetOpen();
		return state;
	}

	static void RestoreWorkspace(const nlohmann::json& state)
	{
		const std::string ini = state.value("ini", std::string());
		if (!ini.empty()) {
			ImGui::ClearIniSettings();
			ImGui::LoadIniSettingsFromMemory(ini.c_str(), ini.size());
		}
		gShowLeftDock = state.value("left", true);
		gShowRightDock = state.value("right", true);
		gShowBottomDock = state.value("bottom", true);
		SetShowWorldPanel(state.value("world", true));
		SetShowInspectorPanel(state.value("inspector", true));
		SetShowDebugPanel(state.value("debug", false));
		SetShowCameraWindow(state.value("camera", false));
		for (auto& menu : MenuRegisterer::GetMenus()) {
			menu.SetOpen(state.contains("menus") && state["menus"].value(menu.name, false));
		}
		gWindowRegions.clear();
		gDockLayoutInitialized = false;
		gResetDockLayout = false;
	}

	void UpdateTaskLayout()
	{
		static bool initialized = false;
		if (!initialized) {
			initialized = true;
			if (ImGui::GetIO().IniFilename) gCustomIniFilename = ImGui::GetIO().IniFilename;
			// A task must never replace the user's normal workspace on the next launch.
			if (gSavedActiveTask.get() != 0 && gTaskWorkspaces.get().contains("Custom")) {
				RestoreWorkspace(gTaskWorkspaces.get()["Custom"]);
				gSavedActiveTask = 0;
			}
		}
		if (gPendingTask < 0) return;
		const int next = gPendingTask;
		gPendingTask = -1;
		auto workspaces = gTaskWorkspaces.get();
		workspaces[gTaskNames[gActiveTask]] = CaptureWorkspace();
		gTaskWorkspaces = workspaces;
		if (ImGui::GetIO().IniFilename) ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
		gActiveTask = next;
		gSavedActiveTask = next;
		// Keep ImGui's automatic disk saves separate while a task is active.
		gTaskIniFilename = gCustomIniFilename + ".task-" + std::to_string(next);
		ImGui::GetIO().IniFilename = gCustomIniFilename.empty() ? nullptr :
			(next == 0 ? gCustomIniFilename.c_str() : gTaskIniFilename.c_str());
		if (workspaces.contains(gTaskNames[next])) {
			RestoreWorkspace(workspaces[gTaskNames[next]]);
			if (next != 0 && !gCustomIniFilename.empty()) ImGui::LoadIniSettingsFromDisk(gTaskIniFilename.c_str());
		} else {
			RequestResetDockLayout();
			SetShowWorldPanel(next == 1);
			SetShowInspectorPanel(next == 1);
			SetShowDebugPanel(false);
			SetShowCameraWindow(false);
			for (auto& menu : MenuRegisterer::GetMenus()) {
				menu.SetOpen(menu.name == "Watch" || (next == 2 && menu.name == "Hero") ||
					(next == 3 && (menu.name == "Hero Replay" || menu.name == "Save/Load")));
			}
		}
	}

	void DrawTaskToolbar()
	{
		if (ImGui::BeginViewportSideBar("Task toolbar", ImGui::GetMainViewport(), ImGuiDir_Up,
			ImGui::GetFrameHeight(), ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
			if (ImGui::BeginMenuBar()) {
				ImGui::TextDisabled("Tasks");
				for (int task = 1; task <= 3; ++task) {
					if (ImGui::MenuItem(gTaskNames[task], nullptr, gActiveTask == task)) gPendingTask = gActiveTask == task ? 0 : task;
					ImGui::SetItemTooltip("Toggle off to restore your custom workspace. Each task remembers its layout.");
				}
				if (gActiveTask != 0 && ImGui::MenuItem("Restore custom layout")) gPendingTask = 0;
				ImGui::EndMenuBar();
			}
		}
		ImGui::End();
	}
	static ImVec2 gLastGameViewportImagePosition = ImVec2(0, 0);
	static ImVec2 gLastGameViewportImageSize = ImVec2(0, 0);

	ImGuiID GetRightDockId() { return gRightDockId; }
	ImGuiID GetBottomDockId() { return gBottomDockId; }

	void RequestResetDockLayout() {
		gResetDockLayout = true;
		gWindowRegions.clear();
		gShowLeftDock = true;
		gShowRightDock = true;
		gShowBottomDock = true;
	}

	bool HasSavedWindowSettings(const char* pWindowName) {
		return ImGui::FindWindowSettingsByID(ImHashStr(pWindowName)) != nullptr;
	}

	void BuildDefaultDockLayout(ImGuiID dockspaceId) {
		ZONE_SCOPED;

		ImGuiDockNode* pExistingNode = ImGui::DockBuilderGetNode(dockspaceId);
		if (!gResetDockLayout) {
			if (gDockLayoutInitialized) {
				return;
			}

			if (pExistingNode != nullptr && (pExistingNode->IsSplitNode() || pExistingNode->Windows.Size > 0)) {
				gDockLayoutInitialized = true;
				auto savedDock = [](const char* name) -> ImGuiID {
					const auto* settings = ImGui::FindWindowSettingsByID(ImHashStr(name));
					return settings ? settings->DockId : 0;
				};
				gLeftDockId = savedDock("World");
				gRightDockId = savedDock("Inspector");
				gBottomDockId = savedDock("Log Window");
				gCenterDockId = savedDock(kGameViewportWindowName);
				return;
			}
		}

		gResetDockLayout = false;

		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, pViewport->WorkSize);

		ImGuiID centerId = dockspaceId;
		ImGuiID leftId = gActiveTask == 2 || gActiveTask == 3 ? 0 : ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Left, gActiveTask == 0 ? 0.17f : 0.20f, nullptr, &centerId);
		ImGuiID rightId = gActiveTask == 3 ? 0 : ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Right, gActiveTask == 0 ? 0.25f : 0.28f, nullptr, &centerId);
		ImGuiID bottomId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Down, gActiveTask == 0 ? 0.18f : (gActiveTask == 3 ? 0.30f : 0.20f), nullptr, &centerId);
		gLeftDockId = leftId;
		gRightDockId = rightId;
		gBottomDockId = bottomId;
		gCenterDockId = centerId;

		ImGui::DockBuilderDockWindow("World", leftId ? leftId : bottomId);
		ImGui::DockBuilderDockWindow("Inspector", rightId ? rightId : bottomId);
		ImGui::DockBuilderDockWindow(kGameViewportWindowName, centerId);
		ImGui::DockBuilderDockWindow("Debug", bottomId);
		ImGui::DockBuilderDockWindow("Camera", rightId ? rightId : bottomId);
		ImGui::DockBuilderDockWindow("Hero", rightId ? rightId : bottomId);
		if (gActiveTask == 3) {
			ImGuiID saveId = 0;
			ImGuiID replayId = ImGui::DockBuilderSplitNode(bottomId, ImGuiDir_Left, 0.50f, nullptr, &saveId);
			ImGui::DockBuilderDockWindow("Hero Replay", replayId);
			ImGui::DockBuilderDockWindow("Debug Save/Load", saveId);
			bottomId = saveId;
			gBottomDockId = saveId;
		}
		ImGui::DockBuilderDockWindow("Scene", bottomId);
		ImGui::DockBuilderDockWindow("Log Window", bottomId);
		ImGui::DockBuilderDockWindow("Watch", bottomId);

		ImGui::DockBuilderFinish(dockspaceId);
		gDockLayoutInitialized = true;
		if (gActiveTask != 0 && !gTaskWorkspaces.get().contains(gTaskNames[gActiveTask])) {
			auto workspaces = gTaskWorkspaces.get();
			workspaces[gTaskNames[gActiveTask]] = CaptureWorkspace();
			gTaskWorkspaces = workspaces;
		}
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
			gLastGameViewportImagePosition = ImVec2(0, 0);
			gLastGameViewportImageSize = ImVec2(0, 0);
			return;
		}

		ImVec2 imageSize = available;
		if ((imageSize.x / imageSize.y) > kGameAspectRatio) {
			imageSize.x = imageSize.y * kGameAspectRatio;
		}
		else {
			imageSize.y = imageSize.x / kGameAspectRatio;
		}

		const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
		const ImVec2 centeredScreenPos(
			cursorScreenPos.x + (available.x - imageSize.x) * 0.5f,
			cursorScreenPos.y + (available.y - imageSize.y) * 0.5f);

		gLastGameViewportImagePosition = centeredScreenPos;
		gLastGameViewportImageSize = imageSize;

		ImGui::SetCursorScreenPos(centeredScreenPos);
		ImGui::Image(gFrameBuffer, imageSize);
		DrawSelectedActorMarker();
	}

	void DrawGameViewportWindow() {
		ImGui::Begin(kGameViewportWindowName, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		DrawGameViewportImage();
		ImGui::End();
	}

	void DrawFullscreenGameViewportWindow() {
		ZONE_SCOPED;
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
		ZONE_SCOPED;

		for (auto& menu : Debug::MenuRegisterer::GetMenus()) {
			if (!menu.GetOpen()) {
				continue;
			}

			const ImGuiID preferredDockId = GetPreferredDockIdForMenu(menu.name);
			const char* windowName = menu.name.c_str();
			if (menu.name == "Log") windowName = "Log Window";
			if (menu.name == "Save/Load") windowName = "Debug Save/Load";
			if (menu.name == "Cutscene") windowName = "Cinematics";
			if (menu.name == "Input") windowName = "Gamepad Debug";
			if (menu.name == "Collision") windowName = "Collision Debug";
			if (menu.name == "Scenario") windowName = "Debug Scenario";
			if (menu.name == "Shop") windowName = "Debug Shop";
			if (menu.name == "Tutorial") windowName = "Debug Tutorial";
			if (menu.name == "Framebuffer") windowName = "FrameBuffer";
			if (menu.name == "Demo") windowName = "Dear ImGui Demo";
			const DockRegion fallback = preferredDockId != 0 && preferredDockId == gRightDockId ? DockRegion::Right : DockRegion::Bottom;
			if (!ShouldDrawDockWindow(windowName, fallback)) continue;
			if (preferredDockId != 0 && !HasSavedWindowSettings(windowName)) {
				ImGui::SetNextWindowDockID(preferredDockId, ImGuiCond_FirstUseEver);
			}

			menu.Show();
		}
	}

	ImVec2 GetGameViewportImagePosition() {
		return gLastGameViewportImagePosition;
	}

	ImVec2 GetGameViewportImageSize() {
		return gLastGameViewportImageSize;
	}

} // namespace Debug
