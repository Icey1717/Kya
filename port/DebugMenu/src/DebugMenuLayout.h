#pragma once

#include <imgui.h>

namespace Debug {
	ImGuiID GetRightDockId();
	ImGuiID GetBottomDockId();

	void RequestResetDockLayout();
	bool HasSavedWindowSettings(const char* pWindowName);
	void BuildDefaultDockLayout(ImGuiID dockspaceId);

	void DrawGameViewportWindow();
	void DrawFullscreenGameViewportWindow();
	void DrawLegacyMenus();
}
