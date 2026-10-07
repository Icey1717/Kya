#pragma once

#include <imgui.h>

namespace Debug {
	enum class DockRegion { Left, Right, Bottom };
	enum class TaskLayout { Custom, WorldInspection, HeroDebugging, Replay };
	void RequestTaskLayout(TaskLayout task);
	TaskLayout GetTaskLayout();
	bool ShouldDrawDockWindow(const char* name, DockRegion fallback);
	void RevealDockWindow(const char* name, DockRegion fallback);
	void DrawWorkspaceMenu();
	void DrawTaskToolbar();
	void UpdateTaskLayout();
	ImGuiID GetRightDockId();
	ImGuiID GetBottomDockId();

	void RequestResetDockLayout();
	bool HasSavedWindowSettings(const char* pWindowName);
	void BuildDefaultDockLayout(ImGuiID dockspaceId);

	void DrawGameViewportWindow();
	void DrawFullscreenGameViewportWindow();
	void DrawLegacyMenus();

	ImVec2 GetGameViewportImagePosition();
	ImVec2 GetGameViewportImageSize();
}
