#include "DebugMenu.h"
#include "DebugRenderer.h"

#include "TextureCache.h"
#include "FrameBuffer.h"

#include "imgui.h"

#include "log.h"

#include <cstdio>
#include "ed3D.h"
#include "edDlist.h"
#include "port/pointer_conv.h"

namespace DebugMenu_Internal {

	struct DebugMaterial {
		DebugMaterial(edDList_material* pInMaterial)
			: pMaterial(pInMaterial)
			, texture(LoadTextureData())
			, texID(DebugMenu::AddTexture(texture))
		{
		
		}

		edDList_material* pMaterial;
		PS2::GSTexValue texture;
		ImTextureID texID;

		Renderer::TextureData LoadTextureData() {
			ed_g2d_bitmap* pPaletteBitmap;
			edpkt_data* pRVar4;
			ed_g2d_bitmap* pTextureBitmap;
			bool bHasPalette;

			if (pMaterial != (edDList_material*)0x0) {
				if ((pMaterial->mode & 4U) == 0) {
					if (pMaterial->pMAT != (ed_g2d_material*)0x0) {
						bHasPalette = false;
						pTextureBitmap = (ed_g2d_bitmap*)0x0;
						pPaletteBitmap = edDListGetG2DBitmap(pMaterial->pMAT, 0, &bHasPalette, &pTextureBitmap);

						if (pPaletteBitmap != (ed_g2d_bitmap*)0x0) {
							pRVar4 = (edpkt_data*)(((char*)LOAD_SECTION(pPaletteBitmap->pPSX2)) + 0x40);

							return MakeTextureDataFromPacket(pRVar4, pTextureBitmap, pPaletteBitmap);
						}
					}
				}
			}

			return Renderer::TextureData();
		}

		ImTextureID GetTextureID() {
			return texID;
		}
	};

	std::unordered_map<const PS2::GSTexEntry*, ImTextureID> debugTextures;
	std::vector<DebugMaterial> debugMaterials;

	// Find or add function
	static ImTextureID FindOrAddTexture(const PS2::GSTexEntry& texEntry)
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
			ImTextureID newTextureID = DebugMenu::AddTexture(texEntry);
			debugTextures[&texEntry] = newTextureID;
			return newTextureID;
		}
	}

	// Find or add function
	static ImTextureID FindOrAddMaterial(edDList_material* pMaterial)
	{
		for (auto& material : debugMaterials) {
			if (pMaterial == material.pMaterial) {
				return material.GetTextureID();
			}
		}

		debugMaterials.emplace_back(pMaterial);
		return debugMaterials.back().GetTextureID();
	}

	void ShowFrameCounter() {
		// Calculate the framerate
		static double prevTime = 0.0;
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - prevTime;
		prevTime = currentTime;
		double fps = 1.0 / deltaTime;

		const ImVec2 windowSize(100.0f, 25.0f);

		// Get the screen size
		const ImVec2 screenSize = ImGui::GetIO().DisplaySize;

		// Render the framerate counter
		ImGui::SetNextWindowSize(windowSize);
		ImGui::SetNextWindowPos(ImVec2(screenSize.x - windowSize.x - 10.0f, 10.0f), ImGuiCond_Always);
		ImGui::Begin("Framerate Counter", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("FPS: %.1f", fps);
		ImGui::End();
	}

	static bool bShowTextures = false;
	static bool bShowDemoWindow = false;
	static bool bShowLogWindow = true;
	static bool bShowMaterialList = true;

	void DrawMenu() {
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Textures"))
			{
				bShowTextures = !bShowTextures;
			}
			if (ImGui::MenuItem("Demo"))
			{
				bShowDemoWindow = !bShowDemoWindow;
			}
			if (ImGui::MenuItem("Log"))
			{
				bShowLogWindow = !bShowLogWindow;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	void ShowTextures() {
		auto texCache = PS2::GetTextureCache();
		ImGui::Begin("Image", &bShowTextures, ImGuiWindowFlags_AlwaysAutoResize);
		for (const auto& entry : texCache.GetEntries()) {
			ImVec2 imageSize(400, 300); // Set the image size

			// Use ImGui::Image to display the image
			ImGui::Image(FindOrAddTexture(entry), imageSize);
		}
		ImGui::End();
	}

	int gRenderFramebufferIndex = 0;

	void ShowFramebuffers() {
		auto& frameBuffers = FrameBuffer::GetAll();

		// Get the display size
		const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		const ImVec2 windowSize(640.0f, 480.0f);
		ImGui::SetNextWindowSize(windowSize);
		ImVec2 windowPos = ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f);
		ImGui::Begin("FrameBuffer", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

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

	void ShowLogWindow()
	{
		const auto& logMessages = Log::GetInstance().GetLogMessages();
		const auto& logCategories = Log::GetInstance().GetLogCategories();
		auto& verboseLevels = Log::GetInstance().GetLogVerboseLevels();

		ImGui::Begin("Log Window", &bShowLogWindow);

		// Checkbox controls for verbose levels
		for (int level = static_cast<int>(LogLevel::VeryVerbose); level < static_cast<int>(LogLevel::Max); ++level)
		{
			LogLevel logLevel = static_cast<LogLevel>(level);
			ImGui::Checkbox(LogLevelToString(logLevel).c_str(), &verboseLevels[logLevel]);

			if (level != static_cast<int>(LogLevel::Max) - 1) {
				ImGui::SameLine();
			}
		}

		// Begin tab bar
		if (ImGui::BeginTabBar("LogTabs"))
		{
			// Loop through log categories
			for (const std::string& category : logCategories)
			{
				// Begin tab item for the category
				if (ImGui::BeginTabItem(category.c_str()))
				{
					ImGui::BeginChild(category.c_str(), ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

					// Display log messages for the current category
					for (const auto& log : logMessages)
					{
						// Display log message if it belongs to the current category
						if (log.category == category && verboseLevels[log.level])
						{
							ImGui::TextWrapped("%s", log.message.c_str());
						}
					}

					ImGui::EndChild();
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	void ShowMaterialList() {
		auto materials = ed3DGetLoadedMaterials();

		ImGui::Begin("Material List", &bShowMaterialList, ImGuiWindowFlags_AlwaysAutoResize);

		static int selectedMaterialIndex = -1;

		for (int i = 0; i < materials.size(); i++) {
			char buttonText[256]; // Buffer to hold the formatted text

			// Format the text using sprintf
			std::sprintf(buttonText, "Material %d", i + 1);

			if (ImGui::Selectable(buttonText)) {
				selectedMaterialIndex = i;
			}
		}

		if (selectedMaterialIndex >= 0) {
			ImVec2 imageSize(400, 300); // Set the image size

			// Use ImGui::Image to display the image
			ImGui::Image(FindOrAddMaterial(materials[selectedMaterialIndex]), imageSize);
		}

		ImGui::End();
	}

	void DrawInternal() {
		if (bShowDemoWindow) {
			ImGui::ShowDemoWindow(&bShowDemoWindow);
		}

		if (bShowTextures) {
			ShowTextures();
		}

		if (bShowLogWindow) {
			ShowLogWindow();
		}

		if (bShowMaterialList) {
			ShowMaterialList();
		}

		ShowFramebuffers();
		DrawMenu();
		ShowFrameCounter();
	}
}

void DebugMenu::BuildImguiCommands()
{
	DebugMenu_Internal::DrawInternal();
}
