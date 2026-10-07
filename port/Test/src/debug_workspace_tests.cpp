#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>
#include "../../DebugMenu/src/DebugMenuLayout.h"
#include "../../DebugMenu/src/DebugMenuWorld.h"
#include "../../DebugMenu/src/DebugMenuDebugPanel.h"
#include "../../DebugMenu/src/DebugMenuToolbar.h"
#include "DebugMenu.h"
#include <filesystem>

namespace Debug { extern const char* gSettingsFile; }

TEST(DebugWorkspace, SwitchingTasksRestoresCustomDockingAndPanelVisibility)
{
	auto* previousContext = ImGui::GetCurrentContext();
	auto* context = ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.DisplaySize = ImVec2(1600, 1000);
	io.DeltaTime = 1.0f / 60.0f;
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	// UI settings normally persist immediately; keep this test away from user settings.
	const auto settingsPath = std::filesystem::temp_directory_path() / "kya-workspace-test.json";
	const std::string settingsName = settingsPath.string();
	const char* previousSettingsFile = Debug::gSettingsFile;
	Debug::gSettingsFile = settingsName.c_str();
	const bool originalWorld = Debug::GetShowWorldPanel();
	const bool originalInspector = Debug::GetShowInspectorPanel();
	const bool originalDebug = Debug::GetShowDebugPanel();
	const bool originalCamera = Debug::GetShowCameraWindow();
	std::vector<bool> originalMenus;
	for (auto& menu : Debug::MenuRegisterer::GetMenus()) originalMenus.push_back(menu.GetOpen());

	bool customDockBuilt = false;
	auto frame = [&] {
		ImGui::NewFrame();
		Debug::UpdateTaskLayout();
		Debug::DrawTaskToolbar();
		const auto dockspace = ImGui::DockSpaceOverViewport();
		Debug::BuildDefaultDockLayout(dockspace);
		if (!customDockBuilt) {
			customDockBuilt = true;
			ImGui::DockBuilderRemoveNode(dockspace);
			ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace, io.DisplaySize);
			ImGuiID center;
			const auto left = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Left, 0.23f, nullptr, &center);
			ImGui::DockBuilderDockWindow("World", left);
			ImGui::DockBuilderDockWindow("Inspector", left);
			ImGui::DockBuilderDockWindow("GameViewport", center);
			ImGui::DockBuilderFinish(dockspace);
		}
		for (const char* name : { "World", "Inspector", "GameViewport", "Hero", "Watch", "Hero Replay", "Debug Save/Load" }) {
			ImGui::Begin(name);
			ImGui::TextUnformatted("Test panel");
			ImGui::End();
		}
		ImGui::EndFrame();
	};
	frame();
	Debug::SetShowWorldPanel(true);
	Debug::SetShowInspectorPanel(false);
	Debug::SetShowDebugPanel(true);
	Debug::SetShowCameraWindow(true);
	for (auto& menu : Debug::MenuRegisterer::GetMenus()) menu.SetOpen(menu.name == "Scene");
	frame();
	const ImGuiID customWorldDock = ImGui::FindWindowByName("World")->DockId;
	const ImGuiID customGameDock = ImGui::FindWindowByName("GameViewport")->DockId;
	const ImVec2 customWorldSize = ImGui::FindWindowByName("World")->Size;

	Debug::RequestTaskLayout(Debug::TaskLayout::HeroDebugging);
	frame();
	frame();
	EXPECT_EQ(Debug::GetTaskLayout(), Debug::TaskLayout::HeroDebugging);
	EXPECT_FALSE(Debug::GetShowWorldPanel());
	// Move a task panel to prove that the task remembers user adjustments.
	const ImGuiID heroGameDock = ImGui::FindWindowByName("GameViewport")->DockId;
	ImGui::DockBuilderDockWindow("Hero", heroGameDock);
	frame();
	Debug::RequestTaskLayout(Debug::TaskLayout::Replay);
	frame();
	frame();
	EXPECT_NE(ImGui::FindWindowByName("Hero Replay")->DockId, ImGui::FindWindowByName("Debug Save/Load")->DockId);
	Debug::RequestTaskLayout(Debug::TaskLayout::HeroDebugging);
	frame();
	frame();
	EXPECT_EQ(ImGui::FindWindowByName("Hero")->DockId, heroGameDock);
	Debug::RequestTaskLayout(Debug::TaskLayout::Custom);
	frame();
	frame();
	EXPECT_EQ(Debug::GetTaskLayout(), Debug::TaskLayout::Custom);
	EXPECT_EQ(ImGui::FindWindowByName("World")->DockId, customWorldDock);
	EXPECT_EQ(ImGui::FindWindowByName("Inspector")->DockId, customWorldDock);
	EXPECT_EQ(ImGui::FindWindowByName("GameViewport")->DockId, customGameDock);
	EXPECT_NEAR(ImGui::FindWindowByName("World")->Size.x, customWorldSize.x, 1.0f);
	EXPECT_TRUE(Debug::GetShowWorldPanel());
	EXPECT_FALSE(Debug::GetShowInspectorPanel());
	EXPECT_TRUE(Debug::GetShowDebugPanel());
	EXPECT_TRUE(Debug::GetShowCameraWindow());
	for (auto& menu : Debug::MenuRegisterer::GetMenus()) EXPECT_EQ(menu.GetOpen(), menu.name == "Scene") << menu.name;

	Debug::SetShowWorldPanel(originalWorld);
	Debug::SetShowInspectorPanel(originalInspector);
	Debug::SetShowDebugPanel(originalDebug);
	Debug::SetShowCameraWindow(originalCamera);
	int index = 0;
	for (auto& menu : Debug::MenuRegisterer::GetMenus()) menu.SetOpen(originalMenus[index++]);
	Debug::gSettingsFile = previousSettingsFile;
	std::filesystem::remove(settingsPath);
	ImGui::DestroyContext(context);
	ImGui::SetCurrentContext(previousContext);
}
