#include <gtest/gtest.h>
#include "../../DebugMenu/src/DebugWatchValue.h"
#include "../../DebugMenu/src/DebugMenuLog.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "log.h"
#include <chrono>
#include <thread>

TEST(DebugWatch, InitialSampleDoesNotLookLikeAChange)
{
	Debug::Watch::ValueState state;
	state.Update("Standing", 0.0);
	EXPECT_FLOAT_EQ(state.Highlight(0.0), 0.0f);
}

TEST(DebugWatch, ChangeFadesAndUnchangedSamplesDoNotRestartIt)
{
	Debug::Watch::ValueState state;
	state.Update("Standing", 0.0);
	state.Update("Jumping", 1.0);
	EXPECT_FLOAT_EQ(state.Highlight(1.0), 1.0f);
	state.Update("Jumping", 1.4);
	EXPECT_NEAR(state.Highlight(1.4), 0.5f, 0.001f);
	EXPECT_FLOAT_EQ(state.Highlight(1.81), 0.0f);
}

TEST(DebugWatch, MissingSourceClearsStaleValueAndCanRecover)
{
	Debug::Watch::ValueState state;
	state.Update("Level 2", 0.0);
	state.Update(std::nullopt, 1.0);
	EXPECT_FALSE(state.value.has_value());
	EXPECT_FLOAT_EQ(state.Highlight(1.0), 0.0f);
	state.Update("Level 3", 2.0);
	ASSERT_TRUE(state.value.has_value());
	EXPECT_EQ(*state.value, "Level 3");
	EXPECT_FLOAT_EQ(state.Highlight(2.0), 1.0f);
}

TEST(DebugLog, RetainsBoundedRecentMessagesWithCategories)
{
	auto logger = Log::CreateLog("UiCaptureTest");
	for (int i = 0; i < 525; ++i) logger->info("ui-capture-message-{}", i);
	// Category logging is asynchronous. Wait for the last submitted message.
	std::vector<std::string> messages;
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
	do {
		messages = Log::GetRecentMessages();
		if (!messages.empty() && messages.back().find("ui-capture-message-524") != std::string::npos) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	} while (std::chrono::steady_clock::now() < deadline);
	ASSERT_EQ(messages.size(), 512u);
	EXPECT_NE(messages.back().find("ui-capture-message-524"), std::string::npos);
	EXPECT_NE(messages.back().find("[UiCaptureTest]"), std::string::npos);
	EXPECT_NE(messages.front().find("ui-capture-message-13"), std::string::npos);
}

TEST(DebugLog, ExpandAndRestoreReturnsToOriginalDock)
{
	auto* previousContext = ImGui::GetCurrentContext();
	auto* context = ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.DisplaySize = ImVec2(1280, 900);
	io.DeltaTime = 1.0f / 60.0f;
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	bool open = true;
	ImGuiID originalDock = 0;
	auto frame = [&](bool initialize = false) {
		ImGui::NewFrame();
		const auto dockspace = ImGui::DockSpaceOverViewport();
		if (initialize) {
			ImGui::DockBuilderRemoveNode(dockspace);
			ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace, io.DisplaySize);
			ImGuiID remainingDock;
			originalDock = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.3f, nullptr, &remainingDock);
			ImGui::DockBuilderDockWindow("Log Window", originalDock);
			ImGui::DockBuilderDockWindow("Test viewport", remainingDock);
			ImGui::DockBuilderFinish(dockspace);
		}
		ImGui::Begin("Test viewport");
		ImGui::End();
		Debug::ShowLogWindow(&open);
		ImGui::Render();
	};
	frame(true);
	frame();
	auto* window = ImGui::FindWindowByName("Log Window");
	EXPECT_EQ(window->DockId, originalDock);
	auto clickFirstButton = [&](ImGuiWindow* target) {
		const auto position = target->DC.CursorStartPos;
		io.AddMousePosEvent(position.x + 10, position.y + 10);
		frame();
		io.AddMouseButtonEvent(0, true);
		frame();
		io.AddMouseButtonEvent(0, false);
		frame();
		frame();
	};
	clickFirstButton(window);
	auto* expandedWindow = ImGui::FindWindowByName("Log Window (Expanded)");
	EXPECT_EQ(window->DockId, originalDock);
	ASSERT_NE(expandedWindow, nullptr);
	EXPECT_TRUE(expandedWindow->Active);
	EXPECT_GT(expandedWindow->Size.y, io.DisplaySize.y * 0.8f);
	clickFirstButton(expandedWindow);
	EXPECT_EQ(window->DockId, originalDock);
	EXPECT_LT(window->Size.y, io.DisplaySize.y * 0.5f);
	EXPECT_FALSE(expandedWindow->Active);
	ImGui::DestroyContext(context);
	ImGui::SetCurrentContext(previousContext);
}
