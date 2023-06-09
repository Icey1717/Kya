#include "DebugMenu.h"

#include "imgui.h"

std::unordered_map<const PS2::GSTexEntry*, ImTextureID> debugTextures;

// Find or add function
ImTextureID FindOrAddTexture(const PS2::GSTexEntry& texEntry)
{
	printf("%p", &texEntry);
	auto it = debugTextures.find(&texEntry);
	if (it != debugTextures.end())
	{
		// Texture already exists in the map
		return it->second;
	}
	else
	{
		// Texture does not exist, add it to the map
		ImTextureID newTextureID = ImGui_ImplVulkan_AddTexture(texEntry.value.sampler, texEntry.value.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		debugTextures[&texEntry] = newTextureID;
		return newTextureID;
	}
}

void DebugMenu::BuildImguiCommands()
{
	static bool bShowDemoWindow = false;
	ImGui::ShowDemoWindow(&bShowDemoWindow);
		
	auto texCache = PS2::GetTextureCache();
	for (const auto& entry : texCache.GetEntries()) {
		ImGui::Begin("Image", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImVec2 imageSize(400, 300); // Set the image size
		
		// Use ImGui::Image to display the image
		ImGui::Image(FindOrAddTexture(entry), imageSize);
		ImGui::End();
	}

	// Calculate the framerate
	static double prevTime = 0.0;
	double currentTime = glfwGetTime();
	double deltaTime = currentTime - prevTime;
	prevTime = currentTime;
	double fps = 1.0 / deltaTime;

	const ImVec2 window_size(100.0f, 25.0f);

	// Render the framerate counter
	ImGui::SetNextWindowSize(window_size);
	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
	ImGui::Begin("Framerate Counter", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("FPS: %.1f", fps);
	ImGui::End();
}
