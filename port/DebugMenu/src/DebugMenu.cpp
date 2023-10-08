#include "DebugMenu.h"
#include "DebugRenderer.h"

#include "FrameBuffer.h"

#include "imgui.h"

#include "log.h"

#include <string>
#include <vector>
#include <cstdio>
#include "ed3D.h"
#include "port/pointer_conv.h"
#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "Log.h"
#include "CameraViewManager.h"
#include "DebugCamera.h"
#include "TextureCache.h"
#include "../../../src/CinematicManager.h"
#include "../../../src/port/vu1_emu.h"

#define DEBUG_LOG(level, format, ...) MY_LOG_CATEGORY("Debug", level, format, ##__VA_ARGS__)

namespace DebugMenu_Internal {

	std::unordered_map<const PS2::GSTexEntry*, ImageTextureID> debugTextures;
	std::vector<MaterialPreviewerEntry> materialList;

	void OnMaterialLoaded(edDList_material* pNewMaterial) {
		std::vector<DWORD64> backtrace;
		CollectBacksTrace(backtrace);
		materialList.emplace_back(pNewMaterial, backtrace);
	}

	void OnMaterialUnloaded(edDList_material* pMaterial) {
		auto it = std::find(materialList.begin(), materialList.end(), pMaterial);
		if (it != materialList.end()) {
			materialList.erase(it);
		}
	}

	struct TextureListEntry {
		TextureListEntry(struct ed_g2d_manager* texture, std::vector<DWORD64> inBacktrace)
			: pTexture(texture)
			, callstackEntry(inBacktrace)
		{}

		struct ed_g2d_manager* pTexture;
		CallstackPreviewerEntry callstackEntry;
	};

	std::vector<TextureListEntry> textureList;

	void OnTextureLoaded(ed_g2d_manager* pNewTexture) {
		std::vector<DWORD64> backtrace;
		CollectBacksTrace(backtrace);
		textureList.emplace_back(pNewTexture, backtrace);
	}

	// Find or add function
	static ImageTextureID FindOrAddTexture(const PS2::GSTexEntry& texEntry)
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
			ImTextureID newImageTextureID = DebugMenu::AddTexture(texEntry.value.image);
			ImTextureID newPaletteTextureID = DebugMenu::AddTexture(texEntry.value.paletteImage);
			debugTextures[&texEntry] = { newImageTextureID, newPaletteTextureID };
			return debugTextures[&texEntry];
		}
	}

	double deltaTime;

	void Update() {
		// Calculate the framerate
		static double prevTime = 0.0;
		double currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;
	}

	void ShowFrameCounter() {
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

	static bool bShowTextureCache = true;
	static bool bShowDemoWindow = false;
	static bool bShowLogWindow = true;
	static bool bShowMaterialList = false;
	static bool bShowTextureList = false;
	static bool bShowFramebuffers = false;
	static bool bShowCutsceneMenu = true;
	static bool bShowRenderingMenu = true;

	void DrawMenu() {
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Textures"))
			{
				bShowTextureCache = !bShowTextureCache;
			}
			if (ImGui::MenuItem("Demo"))
			{
				bShowDemoWindow = !bShowDemoWindow;
			}
			if (ImGui::MenuItem("Log"))
			{
				bShowLogWindow = !bShowLogWindow;
			}
			if (ImGui::MenuItem("Material List"))
			{
				bShowMaterialList = !bShowMaterialList;
			}
			if (ImGui::MenuItem("Texture List"))
			{
				bShowTextureList = !bShowTextureList;
			}
			if (ImGui::MenuItem("Rendering"))
			{
				bShowRenderingMenu = !bShowRenderingMenu;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

#include <stdio.h>
#include <stdlib.h>

	bool GetCutsceneName(void* pData, int index, const char** ppOut) {
		auto* options = static_cast<std::vector<std::string>*>(pData);
		*ppOut = ((*options)[index]).c_str();
		return true;
	}

	void ShowCutsceneMenu() {
		// Create a new ImGui window
		ImGui::Begin("Video Player Controls", &bShowCutsceneMenu, ImGuiWindowFlags_AlwaysAutoResize);

		auto pCinematicManager = g_CinematicManager_0048efc;

		static int selectedCutsceneId = 0;

		if (pCinematicManager->activeCinematicCount > 0) {
			if (selectedCutsceneId > pCinematicManager->activeCinematicCount) {
				selectedCutsceneId = 0;
			}

			std::vector<std::string> options;

			for (int i = 0; i < pCinematicManager->activeCinematicCount; i++) {
				options.emplace_back(std::to_string(i));
			}

			// Create the dropdown box
			if (ImGui::Combo("Select an Option", &selectedCutsceneId, GetCutsceneName, &options, pCinematicManager->activeCinematicCount, -1))
			{
				// Handle the selected option change here
				// The selected option index will be stored in 'selectedOption'
				// You can perform actions based on the selected option.
			}

			auto pCutscene = pCinematicManager->ppCinematicObjB_B[selectedCutsceneId];

			if (pCutscene->cineBankLoadStage_0x2b4 == 4) {
				// Play/Pause button
				if (pCutscene->state != CS_Stopped)
				{
					if (ImGui::Button("Pause"))
					{
						pCutscene->state = CS_Stopped;
					}
				}
				else
				{
					if (ImGui::Button("Play"))
					{
						pCutscene->state = CS_Playing;
					}
				}

				auto& currentTime = pCutscene->totalCutsceneDelta;
				auto& totalTime = ((pCutscene->cinFileData).pCinTag)->totalPlayTime;

				// Jump to beginning button
				if (ImGui::Button("Jump to Beginning"))
				{
					currentTime = 0.0f;
					// Seek to the beginning of the video here
				}

				// Jump to end button
				if (ImGui::Button("Jump to End"))
				{
					currentTime = totalTime;
					// Seek to the end of the video here
				}

				// Current time and total time labels
				ImGui::Text("Time: %.2f / %.2f", currentTime, totalTime);

				// Seek bar for video playback
				ImGui::SliderFloat("##seekbar", &currentTime, 0.0f, totalTime);
			}
			else {
				// Jump to end button
				if (ImGui::Button("Start"))
				{
					pCutscene->Start();
					// Seek to the end of the video here
				}
			}
		}

		// End the ImGui window
		ImGui::End();
	}

	void ShowRenderingMenu() {
		// Create a new ImGui window
		ImGui::Begin("Rendering", &bShowCutsceneMenu, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Use Interpreter", &VU1Emu::GetInterpreterEnabled());
		if (ImGui::Checkbox("Complex Blending", &Renderer::GetUseComplexBlending())) {
			Renderer::ResetRenderer();
		}

		// End the ImGui window
		ImGui::End();
	}

	void ShowTextureCache() {
		auto texCache = PS2::GetTextureCache();
		ImGui::Begin("Texture Cache", &bShowTextureCache, ImGuiWindowFlags_AlwaysAutoResize);

		for (int i = 0; i < texCache.GetEntries().size(); i++) {
			char buttonText[256]; // Buffer to hold the formatted text

			// Format the text using sprintf
			std::sprintf(buttonText, "Cached Texture %d", i + 1);

			if (ImGui::Selectable(buttonText)) {
				auto& selectedMaterial = texCache.GetEntries()[i];
				std::string name = "Material " + std::to_string(i + 1);
				MaterialPreviewer::Open(selectedMaterial.value, FindOrAddTexture(selectedMaterial), name);
			}
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
		ImGui::Begin("FrameBuffer", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

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

	bool swapValue = false;

	void ShowGame() {
		if (!FrameBuffer::Exists(swapValue ? 0x100 : 0x80)) {
			return;
		}
		auto& frameBuffer = FrameBuffer::Get(swapValue ? 0x100 : 0x80);
		swapValue = !swapValue;

		// Get the display size
		const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		const ImVec2 windowSize(640.0f, 480.0f);
		ImGui::SetNextWindowSize(windowSize);
		ImVec2 windowPos = ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f);
		ImGui::SetNextWindowPos(windowPos);
		ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);

		const ImVec2 image_size(640.0f * 2.5f, 480.0f * 2.0f);
		// Use ImGui::Image to display the image
		const ImTextureID gFrameBuffer = DebugMenu::AddFrameBuffer(frameBuffer);
		ImGui::Image(gFrameBuffer, image_size);

		ImGui::End();
	}

	void ShowLogWindow()
	{
		const auto& logMessages = Log::GetInstance().GetLogMessages();
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

		int usage = 0;
		for (const auto& log : logMessages)
		{
			//usage += log.second.capacity() + log.category.capacity();
		}

		ImGui::SameLine();
		ImGui::Text("Usage: %dmb", usage / 1024 / 1024);

		// Begin tab bar
		if (ImGui::BeginTabBar("LogTabs"))
		{
			// Loop through log categories
			for (auto& message : logMessages)
			{
				// Begin tab item for the category
				if (ImGui::BeginTabItem(message.first.c_str()))
				{
					ImGui::BeginChild(message.first.c_str(), ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

					// Display log messages for the current category
					for (const auto& log : message.second)
					{
						// Display log message if it belongs to the current category
						if (verboseLevels[log.level])
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
		ImGui::Begin("Material List", &bShowMaterialList, ImGuiWindowFlags_AlwaysAutoResize);

		for (int i = 0; i < materialList.size(); i++) {
			char buttonText[256]; // Buffer to hold the formatted text

			// Format the text using sprintf
			std::sprintf(buttonText, "Material %d", i + 1);

			if (ImGui::Selectable(buttonText)) {
				std::string name = "Material " + std::to_string(i + 1);

				auto& selectedMaterial = materialList[i];
				selectedMaterial.name = name;
				MaterialPreviewer::Open(selectedMaterial);
			}
		}

		ImGui::End();
	}

	int GetMaterialCountFromTexture(ed_g2d_manager* pTexture) {
		TextureData_HASH_Internal_MAT* pHash = nullptr;
		int i = 0;

		while (ed3DG2DGetMaterialFromIndex(pTexture, i)) {
			i++;
		}

		return i;
	}

	int selectedTextureIndex = -1;
	int selectedMaterialIndex = -1;
	std::vector<edDList_material> textureMaterials;

	void ShowTextureList() {
		ImGui::Begin("Texture List", &bShowTextureList, ImGuiWindowFlags_AlwaysAutoResize);

		bool bOpenedMaterial = false;

		auto ClearMaterials = [&](){
			for (auto& material : textureMaterials) {
				edDListTermMaterial(&material);
			}
			textureMaterials.clear();
		};

		for (int i = 0; i < textureList.size(); i++) {
			char buttonText[256];
			std::sprintf(buttonText, "Texture %d", i + 1);

			if (ImGui::Selectable(buttonText)) {
				ClearMaterials();
				selectedTextureIndex = i;
				bOpenedMaterial = true;
				const int materialCount = GetMaterialCountFromTexture(textureList[selectedTextureIndex].pTexture);
				textureMaterials.resize(materialCount);
				DEBUG_LOG(LogLevel::Info, "Discovered %d materials for texture %p", materialCount, textureList[selectedTextureIndex].pTexture);
			}
		}

		ImGui::End();

		if (selectedTextureIndex >= 0) {
			auto& selectedTexture = textureList[selectedTextureIndex];

			static bool bOpen = true;
			ImGui::Begin("Texture Previewer", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			TextureData_HASH_Internal_MAT* pHash = nullptr;

			int i = 0;
			for (auto& material : textureMaterials) {
				char buttonText[256];
				std::sprintf(buttonText, "Material %d", i + 1);
				if (ImGui::Selectable(buttonText) || bOpenedMaterial) {
					DEBUG_LOG(LogLevel::Info, "Selected material %d", i + 1);
					if (material.textureInfo == nullptr) {
						DEBUG_LOG(LogLevel::Info, "Loading material %d from texture %p", i + 1, selectedTexture.pTexture);
						edDListCreatMaterialFromIndex(&material, i, selectedTexture.pTexture, 2);
					}
					selectedMaterialIndex = i;
					auto entry = MaterialPreviewerEntry(&textureMaterials[selectedMaterialIndex]);
					entry.name = "None";
					MaterialPreviewer::Open(entry);
					bOpenedMaterial = false;
				}
				i++;
			}

			CallstackPreviewer::Show(selectedTexture.callstackEntry);

			ImGui::End();

			if (!bOpen) {
				selectedTextureIndex = -1;
			}
		}
	}

	void DrawInternal() {
		if (bShowDemoWindow) {
			ImGui::ShowDemoWindow(&bShowDemoWindow);
		}

		if (bShowTextureCache) {
			ShowTextureCache();
		}

		if (bShowLogWindow) {
			ShowLogWindow();
		}

		if (bShowMaterialList) {
			ShowMaterialList();
		}

		if (bShowTextureList) {
			ShowTextureList();
		}

		if (bShowFramebuffers) {
			ShowFramebuffers();
		}

		if (bShowCutsceneMenu) {
			ShowCutsceneMenu();
		}

		if (bShowRenderingMenu) {
			ShowRenderingMenu();
		}

		MaterialPreviewer::Update();
		ShowGame();
		DrawMenu();
		DebugCamera::ShowCamera();
		ShowFrameCounter();
	}
}

void DebugMenu::BuildImguiCommands()
{
	DebugMenu_Internal::Update();
	DebugMenu_Internal::DrawInternal();
}

double DebugMenu::GetDeltaTime()
{
	return DebugMenu_Internal::deltaTime;
}

void DebugMenu::Init()
{
	ed3DGetMaterialLoadedDelegate() += DebugMenu_Internal::OnMaterialLoaded;
	ed3DGetMaterialUnloadedDelegate() += DebugMenu_Internal::OnMaterialUnloaded;
	ed3DGetTextureLoadedDelegate() += DebugMenu_Internal::OnTextureLoaded;
}
