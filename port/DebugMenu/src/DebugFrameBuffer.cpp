#include "DebugFrameBuffer.h"
#include <imgui.h>
#include "DebugSetting.h"
#include "DebugRenderer.h"
#include "DebugRendering.h"
#include "FrameBuffer.h"

namespace Debug {
	namespace FrameBuffer {
		static Debug::Setting<float> gDisplyScale = { "Display Scale", 1.0f };

		void ShowMenu(bool* bOpen) {
			ImGui::Begin("FrameBuffer", bOpen);
			gDisplyScale.DrawImguiControl();
			ImGui::End();
		}

		static ImVec2 SetupGameFramebuffer() {
			const ImVec2 windowSize = GetGameWindowSize();
			ImGui::SetNextWindowSize(windowSize);
			ImGui::SetNextWindowPos(GetGameWindowPosition());
			return windowSize;
		}
	}
}

void Debug::FrameBuffer::ShowNativeFrameBuffer(bool* bOpen) {

	// Use ImGui::Image to display the image
	static ImTextureID gFrameBuffer = DebugMenu::AddNativeFrameBuffer();

	if (Debug::Rendering::GetEnableEmulatedRendering()) {
		ImGui::Begin("NativeFrameBuffer", bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		const ImVec2 image_size(400.0f * 4.0f, 300.0f * 4.0f);
		ImGui::Image(gFrameBuffer, image_size);
	}
	else {
		auto windowSize = SetupGameFramebuffer();
		ImGui::Begin("NativeFrameBuffer", bOpen, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav);
		ImGui::Image(gFrameBuffer, windowSize);
	}

	ImGui::End();
}

int gRenderFramebufferIndex = 0;

void Debug::FrameBuffer::ShowFramebuffers(bool* bOpen) {
	auto& frameBuffers = PS2::FrameBuffer::GetAll();

	// Get the display size
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	const ImVec2 windowSize(640.0f, 480.0f);
	ImGui::SetNextWindowSize(windowSize);
	ImVec2 windowPos = ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f);
	ImGui::Begin("FrameBuffer", bOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Select Framebuffer")) {
			int i = 0;
			for (auto& frameBuffer : frameBuffers) {
				char buffer[0x100] = {};
				sprintf_s(buffer, sizeof(buffer), "%d | FBP: %X", i, frameBuffer.first);
				if (ImGui::MenuItem(buffer)) {
					gRenderFramebufferIndex = i;
				}

				i++;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	int i = 0;
	for (auto& frameBuffer : frameBuffers) {
		if (i == gRenderFramebufferIndex) {
			const ImVec2 image_size(640.0f * 2.5f, 480.0f * 2.0f);
			// Use ImGui::Image to display the image
			const ImTextureID gFrameBuffer = DebugMenu::AddFrameBuffer(frameBuffer.second);
			ImGui::Image(gFrameBuffer, image_size);
		}

		i++;
	}

	ImGui::End();
}

static bool swapValue = false;

void Debug::FrameBuffer::ShowGame() {
	if (!PS2::FrameBuffer::Exists(swapValue ? 0x100 : 0x80) || !Debug::Rendering::GetEnableEmulatedRendering()) {
		return;
	}

	auto& frameBuffer = PS2::FrameBuffer::Get(swapValue ? 0x100 : 0x80);
	swapValue = !swapValue;

	SetupGameFramebuffer();
	ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);

	const ImTextureID gFrameBuffer = DebugMenu::AddFrameBuffer(frameBuffer);
	const ImVec2 image_size(640.0f * 2.5f * gDisplyScale, 480.0f * 2.0f * gDisplyScale);
	ImGui::Image(gFrameBuffer, image_size);

	ImGui::End();
}

ImVec2 Debug::FrameBuffer::GetGameWindowPosition()
{
	const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	const ImVec2 windowSize = GetGameWindowSize();
	return ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f);
}

ImVec2 Debug::FrameBuffer::GetGameWindowSize()
{
	return ImVec2(640.0f * gDisplyScale, 480.0f * gDisplyScale);
}
