#include "DebugMenu.h"
#include "DebugRenderer.h"

#include "FrameBuffer.h"

#include "imgui.h"

#include "log.h"

#include "DebugHelpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cstdio>
#include "ed3D.h"
#include "edDlist.h"
#include "port/pointer_conv.h"
#include "DebugMaterialPreviewer.h"
#include "Callstack.h"
#include "Log.h"
#include "CameraViewManager.h"
#include "DebugCamera.h"
#include "TextureCache.h"
#include "CinematicManager.h"
#include "port/vu1_emu.h"
#include "DebugMeshViewer.h"
#include "TimeController.h"
#include "FileManager3D.h"
#include "ActorHeroPrivate.h"

#include "input_functions.h"

#include <fstream>
#include <iostream>
#include "InputManager.h"


#define DEBUG_LOG(level, format, ...) MY_LOG_CATEGORY("Debug", level, format, ##__VA_ARGS__)

extern bool bOther;

namespace DebugMenu_Internal {

	std::unordered_map<const PS2::GSTexEntry*, ImageTextureID> debugTextures;
	std::vector<MaterialPreviewerEntry> materialList;

	void OnMaterialLoaded(edDList_material* pNewMaterial) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		materialList.emplace_back(pNewMaterial, backtrace);
	}

	void OnMaterialUnloaded(edDList_material* pMaterial) {
		auto it = std::find(materialList.begin(), materialList.end(), pMaterial);
		if (it != materialList.end()) {
			materialList.erase(it);
		}
	}

	struct TextureListEntry {
		TextureListEntry(struct ed_g2d_manager* texture, std::vector<DWORD64> inBacktrace, std::string inName)
			: pTexture(texture)
			, callstackEntry(inBacktrace)
			, name(inName)
		{}

		struct ed_g2d_manager* pTexture;
		CallstackPreviewerEntry callstackEntry;
		std::string name;
	};

	std::vector<TextureListEntry> textureList;

	void OnTextureLoaded(ed_g2d_manager* pNewTexture) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		textureList.emplace_back(pNewTexture, backtrace, ObjectNaming::GetNextObjectName());
	}

	struct MeshListEntry {
		MeshListEntry(struct ed_g3d_manager* mesh, std::vector<DWORD64> inBacktrace, std::string inName)
			: pMesh(mesh)
			, callstackEntry(inBacktrace)
			, name(inName)
		{}

		ed_g3d_manager* pMesh;
		CallstackPreviewerEntry callstackEntry;
		std::string name;
	};

	std::vector<MeshListEntry> meshList;

	void OnMeshLoaded(ed_g3d_manager* pNewMesh) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		meshList.emplace_back(pNewMesh, backtrace, ObjectNaming::GetNextObjectName());
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

	int selectedTextureIndex = -1;
	int selectedMaterialIndex = -1;
	std::vector<edDList_material> textureMaterials;

	int GetMaterialCountFromTexture(ed_g2d_manager* pTexture) {
		int i = 0;

		while (ed3DG2DGetMaterialFromIndex(pTexture, i)) {
			i++;
		}

		return i;
	}

	void ShowTextureList(bool* bOpen) {
		ImGui::Begin("Texture List", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		bool bOpenedMaterial = false;

		auto ClearMaterials = [&]() {
			for (auto& material : textureMaterials) {
				edDListTermMaterial(&material);
			}
			textureMaterials.clear();
			};

		for (int i = 0; i < textureList.size(); i++) {
			char buttonText[256];
			std::sprintf(buttonText, "Texture %s", textureList[i].name.c_str());

			if (ImGui::Selectable(buttonText)) {
				ClearMaterials();
				selectedTextureIndex = i;
				bOpenedMaterial = true;
				const int materialCount = GetMaterialCountFromTexture(textureList[selectedTextureIndex].pTexture);
				textureMaterials.resize(materialCount);
				DEBUG_LOG(LogLevel::Info, "Discovered %d materials for texture {}", materialCount, (uintptr_t)textureList[selectedTextureIndex].pTexture);
			}
		}

		ImGui::End();

		if (selectedTextureIndex >= 0) {
			auto& selectedTexture = textureList[selectedTextureIndex];

			static bool bOpen = true;
			ImGui::Begin("Texture Previewer", &bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			int i = 0;
			for (auto& material : textureMaterials) {
				char buttonText[256];
				std::sprintf(buttonText, "Material %d", i + 1);
				if (ImGui::Selectable(buttonText) || bOpenedMaterial) {
					DEBUG_LOG(LogLevel::Info, "Selected material %d", i + 1);
					if (material.textureInfo == nullptr) {
						DEBUG_LOG(LogLevel::Info, "Loading material %d from texture {}", i + 1, (uintptr_t)selectedTexture.pTexture);
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

	bool GetCutsceneName(void* pData, int index, const char** ppOut) {
		auto* options = static_cast<std::vector<std::string>*>(pData);
		*ppOut = ((*options)[index]).c_str();
		return true;
	}

	char* GetStateName(int state) {
		switch (state) {
		case STATE_HERO_STAND:
			return "StateHeroStand";
			break;
		case STATE_HERO_JUMP_1_1_STAND:
			return "StateHeroJump_1_1_Stand";
			break;
		case STATE_HERO_JUMP_2_3_STAND:
			return "StateHeroJump_2_3_Stand";
			break;
		case STATE_HERO_JUMP_3_3_STAND:
			return "StateHeroJump_3_3_Stand";
			break;
		case STATE_HERO_RUN:
			return "StateHeroRun";
			break;
		case STATE_HERO_SLIDE_SLIP_A:
			return "StateHeroSlideSlipA";
			break;
		case STATE_HERO_SLIDE_SLIP_B:
			return "StateHeroSlideSlipB";
			break;
		case STATE_HERO_SLIDE_SLIP_C:
			return "StateHeroSlideSlipC";
			break;
		case 0x3:
			return "Cinematic";
			break;
		default:
			assert(false);
			break;
		}
	}

	void ShowHeroMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Hero", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		CActorHeroPrivate* pActorHero = reinterpret_cast<CActorHeroPrivate*>(CActorHeroPrivate::_gThis);

		if (pActorHero) {
			ImGui::Text("State: %s", GetStateName(pActorHero->actorState));

			DebugHelpers::ImGui::TextVector4("Current Location", pActorHero->currentLocation);
			DebugHelpers::ImGui::TextVector4("Rotation Quat", pActorHero->rotationQuat);
			ImGui::InputFloat("Effort", &pActorHero->effort);

			ImGui::Spacing();

			DebugHelpers::ImGui::TextVector4("Control Direction", pActorHero->controlDirection);
			ImGui::Text("Facing control direction: %d", pActorHero->bFacingControlDirection);

			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Dynamic", ImGuiTreeNodeFlags_DefaultOpen)) {
				DebugHelpers::ImGui::TextVector4("Rotation Quat", pActorHero->dynamic.rotationQuat);
				DebugHelpers::ImGui::TextVector4("field_0x10", pActorHero->dynamic.field_0x10);
				DebugHelpers::ImGui::TextVector4("Current Location", pActorHero->dynamic.currentLocation);
				DebugHelpers::ImGui::TextVector4("Horizontal Location", pActorHero->dynamic.horizontalLocation);

				ImGui::Text("Flags: %x", pActorHero->flags);

				ImGui::InputFloat("Intensity", &pActorHero->dynamic.intensity);
				ImGui::InputFloat("field_0x40", &pActorHero->dynamic.field_0x40);
				ImGui::InputFloat("field_0x44", &pActorHero->dynamic.field_0x44);
				ImGui::InputFloat("field_0x54", &pActorHero->dynamic.field_0x54);
				ImGui::InputFloat("field_0x58", &pActorHero->dynamic.field_0x58);
			}

			ImGui::Spacing();

			if (ImGui::CollapsingHeader("DynamicExt", ImGuiTreeNodeFlags_DefaultOpen)) {
				DebugHelpers::ImGui::TextVector4("Gravity", pActorHero->dynamicExt.gForceGravity);
				DebugHelpers::ImGui::TextVector4("Translation", pActorHero->dynamicExt.normalizedTranslation);

				ImGui::InputFloat("Gravity Scale", &pActorHero->dynamicExt.gravityScale);
			}

			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Collision", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("Flags 0: %x", pActorHero->pCollisionData->flags_0x0);
				ImGui::Text("Flags 4: %x", pActorHero->pCollisionData->flags_0x4);

				if (ImGui::CollapsingHeader("Flags", ImGuiTreeNodeFlags_DefaultOpen)) {
					ImGui::Text("On Plane: %d", pActorHero->pCollisionData->flags_0x4 & 1);
				}
			}

			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen)) {
				DebugHelpers::ImGui::TextVector4("Left Analog Stick", pActorHero->pPlayerInput->lAnalogStick);
			}
		}

		// End the ImGui window
		ImGui::End();
	}

	void ShowCutsceneMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Video Player Controls", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		auto* pCinematicManager = g_CinematicManager_0048efc;

		static int selectedCutsceneId = 0;
		static bool bSelectedFirstPlaying = false;

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

			if (!bSelectedFirstPlaying) {
				for (int i = 0; i < pCinematicManager->activeCinematicCount; i++) {
					if (pCinematicManager->ppCinematicObjB_B[i]->state == CS_Playing) {
						selectedCutsceneId = i;
						bSelectedFirstPlaying = true;
					}
				}
			}

			auto* pCutscene = pCinematicManager->ppCinematicObjB_B[selectedCutsceneId];

			if (pCutscene->cineBankLoadStage_0x2b4 == 4 && pCutscene->cinFileData.pCinTag) {
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

				static bool bCutsceneStepEnabled = false;
				ImGui::Checkbox("Custom Time Control", &bCutsceneStepEnabled);

				if (bCutsceneStepEnabled) {
					static float cutsceneStepTime = 46.f;
					pCutscene->totalCutsceneDelta = cutsceneStepTime;

					if (ImGui::Button("<<")) {
						auto pTimer = Timer::GetTimer();
						cutsceneStepTime -= pTimer->cutsceneDeltaTime;
					}
					ImGui::SameLine();
					if (ImGui::Button(">>")) {
						auto pTimer = Timer::GetTimer();
						cutsceneStepTime += pTimer->cutsceneDeltaTime;
					}

				}
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

	void ShowRenderingMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Rendering", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::CollapsingHeader("VU1 Emulation", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Use Hardware Draw", &VU1Emu::GetHardwareDrawEnabled());
			ImGui::Checkbox("Use Interpreter", &VU1Emu::GetInterpreterEnabled());
			ImGui::Checkbox("Single Threaded", &VU1Emu::GetRunSingleThreaded());
			ImGui::Checkbox("Simplified Code", &VU1Emu::GetRunSimplifiedCode());
		}

		if (ImGui::Button("Enable Vertex Trace")) {
			VU1Emu::GetTraceVtx() = true;
		}

		if (ImGui::Checkbox("Complex Blending", &Renderer::GetUseComplexBlending())) {
			Renderer::ResetRenderer();
		}

		ImGui::Checkbox("Use GLSL Pipeline", &DebugMeshViewer::GetUseGlslPipeline());

		ImGui::Checkbox("Force Highest LOD", &ed3D::DebugOptions::GetForceHighestLod());

		// End the ImGui window
		ImGui::End();
	}

	void ShowTextureCache(bool* bOpen) {
		auto& texCache = PS2::GetTextureCache();
		ImGui::Begin("Texture Cache", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

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

	void ShowFramebuffers(bool* bOpen) {
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

	bool swapValue = false;

	void ShowGame() {
		if (!PS2::FrameBuffer::Exists(swapValue ? 0x100 : 0x80)) {
			return;
		}
		auto& frameBuffer = PS2::FrameBuffer::Get(swapValue ? 0x100 : 0x80);
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

	// Structure to hold category and its enabled status
	struct CategoryEntry {
		std::string name;
		bool enabled;
	};

	// Function to add a new category to the configuration file
	static void AddCategoryToConfigFile(const std::string& category, bool enabled) 
	{
		std::ofstream outFile("category_config.txt", std::ios::app); // Open file in append mode

		if (!outFile.is_open()) {
			// Handle error: unable to open file
			std::cerr << "Error: Unable to open config file for writing." << std::endl;

			// Attempt to create the file
			std::ofstream createFile("category_config.txt");
			if (!createFile.is_open()) {
				// Handle error: unable to create file
				std::cerr << "Error: Unable to create config file." << std::endl;
				return;
			}
			createFile.close();

			// Retry opening the file
			outFile.open("category_config.txt", std::ios::app);
			if (!outFile.is_open()) {
				// Handle error: unable to open file even after creation
				std::cerr << "Error: Unable to open config file for writing." << std::endl;
				return;
			}
		}

		// Write the new category and its enabled status to the file
		outFile << category << " " << std::boolalpha << enabled << std::endl;

		// Close the file
		outFile.close();
	}

	// Function to update an existing entry in the configuration file
	static void UpdateCategoryInConfigFile(const std::string& category, bool enabled) 
	{
		std::ifstream inFile("category_config.txt");
		if (!inFile.is_open()) {
			// Handle error: unable to open file
			std::cerr << "Error: Config file not found. Creating a new one." << std::endl;

			// Attempt to create the file
			std::ofstream createFile("category_config.txt");
			if (!createFile.is_open()) {
				// Handle error: unable to create file
				std::cerr << "Error: Unable to create config file." << std::endl;
				return;
			}
			createFile.close();

			// Retry opening the file
			inFile.open("category_config.txt");
			if (!inFile.is_open()) {
				// Handle error: unable to open file even after creation
				std::cerr << "Error: Unable to open config file for reading." << std::endl;
				return;
			}
		}

		bool bFound = false;

		// Read the existing entries from the file into a vector of CategoryEntry
		std::vector<CategoryEntry> entries;
		std::string line;
		while (std::getline(inFile, line)) {
			std::istringstream iss(line);
			std::string categoryName;
			bool categoryEnabled;
			if (iss >> categoryName >> std::boolalpha >> categoryEnabled) {
				if (categoryName == category) {
					// Update the enabled status for the specified category
					bFound = true;
					categoryEnabled = enabled;
				}
				entries.push_back({ categoryName, categoryEnabled });
			}
		}
		inFile.close();

		if (!bFound) {
			// Add the new category to the file
			AddCategoryToConfigFile(category, enabled);
			return;
		}

		// Write the updated entries back to the file
		std::ofstream outFile("category_config.txt");
		if (!outFile.is_open()) {
			// Handle error: unable to open file
			std::cerr << "Error: Unable to open config file for writing." << std::endl;
			return;
		}

		for (const auto& entry : entries) {
			outFile << entry.name << " " << std::boolalpha << entry.enabled << std::endl;
		}
		outFile.close();
	}

	void ShowLogWindow(bool* bOpen)
	{		
		ImGui::Begin("Log Window", bOpen);
		
		// Checkbox controls for verbose levels
		for (int level = static_cast<int>(LogLevel::VeryVerbose); level < static_cast<int>(LogLevel::Max); ++level)
		{
			// change this code to use a drop down list.
			LogLevel logLevel = static_cast<LogLevel>(level);
			if (ImGui::Selectable(LogLevelToString(logLevel).c_str())) {

			}
		}

		for (auto& [category, log] : Log::GetInstance().GetLogs())
		{
			if (ImGui::Checkbox(category.c_str(), &log.bEnabled)) {
				UpdateCategoryInConfigFile(category, log.bEnabled);
			}
		}
		
		ImGui::End();
	}

	void ShowMaterialList(bool* bOpen) {
		ImGui::Begin("Material List", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

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

	void ShowSceneMenu(bool* bOpen) {
		ImGui::Begin("Scene", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		static int selectedScene = -1;

		for (int i = 0; i < ged3DConfig.sceneCount; i++) {
			char buttonText[256];
			std::sprintf(buttonText, "gScene3D[%d]", i + 1);
			if (ImGui::Selectable(buttonText)) {
				selectedScene = i;
			}
		}

		static edNODE* pSelectedNode = nullptr;

		if (selectedScene != -1) {
			ed_3D_Scene* pSelectedScene = &gScene3D[selectedScene];

			static bool bFilterAnim = false;
			ImGui::Checkbox("Filter Anim", &bFilterAnim);

			ImGui::Text("Shadow: %d", pSelectedScene->bShadowScene);
			ImGui::Text("Flags: 0x%x", pSelectedScene->flags);
			edNODE* pCurNode;
			edLIST* pList = pSelectedScene->pHierListA;
			if (((pSelectedScene->flags & 1) != 0) && ((pSelectedScene->flags & 4) == 0)) {
				for (pCurNode = pList->pPrev; (edLIST*)pCurNode != pList; pCurNode = pCurNode->pPrev) {

					ed_3d_hierarchy* pHierarchy = (ed_3d_hierarchy*)pCurNode->pData;

					if (pHierarchy && (pHierarchy->pAnimMatrix || !bFilterAnim)) {
						char nodeText[256];
						std::sprintf(nodeText, "Node: %p", pCurNode);
						if (ImGui::Selectable(nodeText)) {
							pSelectedNode = pCurNode;
						}
					}
				}
			}
			else {
				ImGui::Text("Flags disabled nodes");
			}
		}

		ImGui::End();

		if (pSelectedNode) {
			DebugMeshViewer::ShowNodeMenu(pSelectedNode);
		}
	}

	edNODE* pNewNode = nullptr;
	ed_g3d_manager* pSelectedMesh = nullptr;

	void ShowMeshList(bool* bOpen) {
		ImGui::Begin("Mesh List", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		static edLIST* pList = ed3DHierarchyListInit();

		for (int i = 0; i < meshList.size(); i++) {
			char buttonText[256];
			std::sprintf(buttonText, "Mesh %s", meshList[i].name.c_str());

			if (ImGui::Selectable(buttonText)) {
				pNewNode = nullptr;
				pSelectedMesh = meshList[i].pMesh;

				if (!pSelectedMesh->CSTA) {
					pNewNode = ed3DHierarchyAddToList(pList, gHierarchyManagerBuffer, gHierarchyManagerFirstFreeNode, meshList[i].pMesh, NULL);
				}
			}
		}

		ImGui::End();

		if (pSelectedMesh) {
			if (pNewNode) {
				DebugMeshViewer::ShowNodeMenu(pNewNode);
			}
			else if (pSelectedMesh->CSTA) {
				DebugMeshViewer::ShowClusterMenu(pSelectedMesh);
			}
		}
	}

	struct Menu {
		void Show() {
			if (bOpen) {
				ShowFunction(&bOpen);
			}
		}

		std::string name{};
		std::function<void(bool*)> ShowFunction;
		bool bOpen{};
	};

	static std::vector<Menu> gMenus = 
	{
		{"Demo", ImGui::ShowDemoWindow },
		{"Log", ShowLogWindow },
		{"Material List", ShowMaterialList },
		{"Texture List", ShowTextureList },
		{"Mesh List", ShowMeshList },
		{"Framebuffers", ShowFramebuffers },
		{"Cutscene", ShowCutsceneMenu, true },
		{"Rendering", ShowRenderingMenu },
		{"Scene", ShowSceneMenu },
		{"Hero", ShowHeroMenu, true },

	};

	void ForEachMenu(std::function<void(Menu&)> action) {
		for (auto& currentMenu : gMenus) {
			action(currentMenu);
		}
	}

	void DrawMenu() {
		ImGui::BeginMainMenuBar();

		ForEachMenu([](Menu& menu) {
			if (ImGui::MenuItem(menu.name.c_str()))
			{
				menu.bOpen = !menu.bOpen;
			}
			});

		ImGui::EndMainMenuBar();
	}

	void DrawInternal() {
		ForEachMenu([](Menu& menu) {
			menu.Show();
			});

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
	edDListGetMaterialLoadedDelegate() += DebugMenu_Internal::OnMaterialLoaded;
	edDListGetMaterialUnloadedDelegate() += DebugMenu_Internal::OnMaterialUnloaded;
	ed3DGetTextureLoadedDelegate() += DebugMenu_Internal::OnTextureLoaded;
	ed3DGetMeshLoadedDelegate() += DebugMenu_Internal::OnMeshLoaded;
}

static Input::InputFunctions gInputFunctions;

Input::InputFunctions& DebugMenu::GetInputFunctions()
{
	gInputFunctions.pressed = GetKeyPressed;
	gInputFunctions.released = GetKeyReleased;
	gInputFunctions.analog = GetKeyAnalog;
	return gInputFunctions;
}

std::unordered_map<uint32_t, ImGuiKey> gKeyMap = {
	{ ROUTE_START,				ImGuiKey_Enter },
	{ ROUTE_X,					ImGuiKey_A },
	{ ROUTE_UP,					ImGuiKey_UpArrow },
	{ ROUTE_DOWN,				ImGuiKey_DownArrow },
	{ ROUTE_L_ANALOG_UP,		ImGuiKey_G },
	{ ROUTE_L_ANALOG_DOWN,		ImGuiKey_H },
	{ ROUTE_L_ANALOG_LEFT,		ImGuiKey_D },
	{ ROUTE_L_ANALOG_RIGHT,		ImGuiKey_T },
};

bool DebugMenu::GetKeyPressed(uint32_t routeId)
{
	assert(gKeyMap.find(routeId) != gKeyMap.end());

	return ImGui::IsKeyPressed(gKeyMap[routeId]);
}

bool DebugMenu::GetKeyReleased(uint32_t routeId)
{
	assert(gKeyMap.find(routeId) != gKeyMap.end());

	return ImGui::IsKeyReleased(gKeyMap[routeId]);
}

float DebugMenu::GetKeyAnalog(uint32_t routeId)
{
	assert(gKeyMap.find(routeId) != gKeyMap.end());

	return ImGui::IsKeyDown(gKeyMap[routeId]) ? 1.0f : 0.0f;
}

