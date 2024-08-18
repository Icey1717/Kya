#include "DebugMenu.h"
#include "DebugRenderer.h"

#include "FrameBuffer.h"

#include <imgui.h>

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
#include "DebugTexture.h"
#include "TimeController.h"
#include "FileManager3D.h"
#include "ActorHeroPrivate.h"

#include "input_functions.h"

#include <fstream>
#include <iostream>
#include "InputManager.h"
#include "DebugSetting.h"
#include "Rendering/DisplayList.h"
#include "SectorManager.h"
#include "DebugHero.h"
#include "../../Windows/Renderer/Vulkan/src/VulkanRenderer.h"
#include "DebugMesh.h"

#define DEBUG_LOG(level, format, ...) MY_LOG_CATEGORY("Debug", level, format, ##__VA_ARGS__)

extern bool bOther;

namespace Debug {

	static std::unordered_map<const PS2::GSTexEntry*, ImageTextureID> debugTextures;
	static std::vector<MaterialPreviewerEntry> materialList;

	static void OnMaterialLoaded(edDList_material* pNewMaterial) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		materialList.emplace_back(pNewMaterial, backtrace);
	}

	static void OnMaterialUnloaded(edDList_material* pMaterial) {
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

	static std::vector<TextureListEntry> textureList;

	static void OnTextureLoaded(ed_g2d_manager* pNewTexture, std::string name) {
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

	static std::vector<MeshListEntry> meshList;

	static void OnMeshLoaded(ed_g3d_manager* pNewMesh, std::string name) {
		std::vector<DWORD64> backtrace;
		CollectBacktrace(backtrace);
		meshList.emplace_back(pNewMesh, backtrace, ObjectNaming::GetNextObjectName());
	}

	static double deltaTime;

	static void Update() {
		// Calculate the framerate
		static double prevTime = 0.0;
		double currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;
	}

	static void ShowFrameCounter() {
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

	static int selectedTextureIndex = -1;
	static int selectedMaterialIndex = -1;
	static std::vector<edDList_material> textureMaterials;

	static int GetMaterialCountFromTexture(ed_g2d_manager* pTexture) {
		int i = 0;

		while (ed3DG2DGetMaterialFromIndex(pTexture, i)) {
			i++;
		}

		return i;
	}

	static bool GetCutsceneName(void* pData, int index, const char** ppOut) {
		auto* options = static_cast<std::vector<std::string>*>(pData);
		*ppOut = ((*options)[index]).c_str();
		return true;
	}

	static void ShowSectorMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Sector", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Current Sector: %d", CScene::ptable.g_SectorManager_00451670->baseSector.currentSectorID);

		for (int i = 0; i < 5; i++) {
			char buttonText[256]; // Buffer to hold the formatted text

			// Format the text using sprintf
			std::sprintf(buttonText, "Switch Sector %d", i);

			// Create a button for each sector
			if (ImGui::Button(buttonText)) {
				CScene::ptable.g_SectorManager_00451670->SwitchToSector(i, true);
			}
		}

		// End the ImGui window
		ImGui::End();
	}

	static Debug::Setting<bool> gSkipCutscenes = { "Skip Cutscenes", false };
	static Debug::Setting<float> gCustomCutsceneTime = { "Custom Cutscene Time", 0.0f };

	static void ShowCutsceneMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Video Player Controls", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::CollapsingHeader("Persistent Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			gSkipCutscenes.DrawImguiControl();
		}

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
				if (ImGui::Button("Jump to End") || gSkipCutscenes)
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
					gCustomCutsceneTime.DrawImguiControl();
					static float cutsceneStepTime = 0.0f;
					pCutscene->totalCutsceneDelta = gCustomCutsceneTime + cutsceneStepTime;

					pCutscene->totalCutsceneDelta = std::clamp(pCutscene->totalCutsceneDelta, 0.0f, totalTime - 1.0f);

					if (ImGui::Button("Set to current")) {
						gCustomCutsceneTime = gCustomCutsceneTime + cutsceneStepTime;
						cutsceneStepTime = 0.0f;
					}

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

	static Debug::Setting<float> gDisplyScale = { "Display Scale", 1.0f };
	static Debug::Setting<bool> gDisableClusterRendering = { "Disable Cluster Rendering", false };
	static Debug::Setting<bool> gForceAnimMatrixIdentity = { "Force animation matrix to identity", false };
	static Debug::Setting<bool> gEnableEmulatedRendering = { "Enable Emulated Rendering", false };
	static Debug::Setting<bool> gUsePreprocessedVertices = { "Use Preprocessed Vertices", false };

	static void ShowRenderingMenu(bool* bOpen) {
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

		gDisplyScale.DrawImguiControl();

		ImGui::Checkbox("Use GLSL Pipeline", &DebugMeshViewer::GetUseGlslPipeline());

		ImGui::Checkbox("Force Highest LOD", &ed3D::DebugOptions::GetForceHighestLod());

		if (gDisableClusterRendering.DrawImguiControl()) {
			ed3D::DebugOptions::GetDisableClusterRendering() = gDisableClusterRendering;
		}

		if (gForceAnimMatrixIdentity.DrawImguiControl()) {
			Renderer::GetForceAnimMatrixIdentity() = gForceAnimMatrixIdentity;
		}

		if (gEnableEmulatedRendering.DrawImguiControl()) {
			VU1Emu::GetEnableEmulatedRendering() = gEnableEmulatedRendering;
		}

		if (gUsePreprocessedVertices.DrawImguiControl()) {
			Renderer::Native::GetUsePreprocessedVertices() = gUsePreprocessedVertices;
		}

		// End the ImGui window
		ImGui::End();
	}

	static ImVec2 SetupGameFramebuffer() {
		const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		const ImVec2 windowSize(640.0f * gDisplyScale, 480.0f * gDisplyScale);
		ImGui::SetNextWindowSize(windowSize);
		ImVec2 windowPos = ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f);
		ImGui::SetNextWindowPos(windowPos);
		return windowSize;
	}

	static void ShowNativeFrameBuffer(bool* bOpen) {

		// Use ImGui::Image to display the image
		static ImTextureID gFrameBuffer = DebugMenu::AddNativeFrameBuffer();

		if (gEnableEmulatedRendering) {
			ImGui::Begin("NativeFrameBuffer", bOpen, ImGuiWindowFlags_AlwaysAutoResize);
			const ImVec2 image_size(400.0f * 4.0f, 300.0f * 4.0f);
			ImGui::Image(gFrameBuffer, image_size);
		}
		else {
			auto windowSize = SetupGameFramebuffer();
			ImGui::Begin("NativeFrameBuffer", bOpen, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
			ImGui::Image(gFrameBuffer, windowSize);
		}

		ImGui::End();
	}

	int gRenderFramebufferIndex = 0;

	static void ShowFramebuffers(bool* bOpen) {
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

	static void ShowGame() {
		if (!PS2::FrameBuffer::Exists(swapValue ? 0x100 : 0x80) || !gEnableEmulatedRendering) {
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

	static void ShowLogWindow(bool* bOpen)
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

	static void ShowSceneMenu(bool* bOpen) {
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
		static ed_g3d_manager* pMeshInfo = nullptr;

		if (selectedScene != -1) {
			ed_3D_Scene* pSelectedScene = &gScene3D[selectedScene];

			static bool bFilterAnim = false;
			ImGui::Checkbox("Filter Anim", &bFilterAnim);

			ImGui::Text("Shadow: %d", pSelectedScene->bShadowScene);
			ImGui::Text("Flags: 0x%x", pSelectedScene->flags);

			if (ImGui::CollapsingHeader("Hierarchy")) {
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

			if (ImGui::CollapsingHeader("Mesh Nodes")) {
				edNODE* pClusterNode;

				if ((pSelectedScene->bShadowScene != 1) && ((pSelectedScene->flags & 8) == 0)) {
					for (pClusterNode = (edNODE*)(pSelectedScene->meshClusterList).pPrev;
						(edLIST*)pClusterNode != &pSelectedScene->meshClusterList; pClusterNode = pClusterNode->pPrev) {
						edCluster* pCluster = (edCluster*)pClusterNode->pData;

						char nodeText[256];
						std::sprintf(nodeText, "Cluster: %p (%p)", pClusterNode, pCluster->pMeshInfo->CSTA);
						if (ImGui::Selectable(nodeText)) {
							ed_g3d_manager* pMesh;
							if (((pCluster->flags & 2) == 0) && (pMesh = pCluster->pMeshInfo, pMesh->CSTA != (ed_Chunck*)0x0)) {
								pMeshInfo = pMesh;
							}
						}
					}
				}
			}
		}

		ImGui::End();
	}

	static void ShowMemoryMenu(bool* bOpen) {
		ImGui::Begin("Memory", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Main Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_MAIN)));
		ImGui::Text("Memkit Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_MEMKIT)));
		ImGui::Text("Scratchpad Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_SCRATCHPAD)));
		ImGui::Text("Video Heap Size: 0x%x", edMemGetMemoryAvailable(TO_HEAP(H_VIDEO)));

		if (ImGui::Button("Dump Memory")) {
			std::ofstream inFile("memory_dump.bin", std::ios::binary);
			assert(inFile.is_open());

			const char* pMem = reinterpret_cast<const char*>(edmemGetMainHeader());
			inFile.write(pMem, sizeof(VirtualMemory));
			inFile.close();
		}

		if (ImGui::Button("Load Memory")) {
			std::ifstream inFile("memory_dump.bin", std::ios::binary);
			assert(inFile.is_open());

			char* pMem = reinterpret_cast<char*>(edmemGetMainHeader());
			inFile.read(pMem, sizeof(VirtualMemory));
			inFile.close();
		}

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Allocations:");
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_CACHE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_CACHE));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE));
		ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));

		ImGui::End();
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
		{"Native FrameBuffer", ShowNativeFrameBuffer, true },
		{"Framebuffers", ShowFramebuffers },
		{"Cutscene", ShowCutsceneMenu, true },
		{"Rendering", ShowRenderingMenu },
		{"Scene", ShowSceneMenu },
		{"Hero", Debug::Hero::ShowMenu, true },
		{"Sector", ShowSectorMenu, true },
		{"Memory", ShowMemoryMenu, true },
		{"Texture", Debug::Texture::ShowMenu, true },
		{"Mesh", Debug::Mesh::ShowMenu, true },

	};

	static void ForEachMenu(std::function<void(Menu&)> action) {
		for (auto& currentMenu : gMenus) {
			action(currentMenu);
		}
	}

	static void DrawMenu() {
		ImGui::BeginMainMenuBar();

		ForEachMenu([](Menu& menu) {
			if (ImGui::MenuItem(menu.name.c_str()))
			{
				menu.bOpen = !menu.bOpen;
			}
			});

		ImGui::EndMainMenuBar();
	}

	static void DrawInternal() {
		ForEachMenu([](Menu& menu) {
			menu.Show();
			});

		Texture::Update();

		ShowGame();
		DrawMenu();
		DebugCamera::ShowCamera();
		ShowFrameCounter();
	}
}

void DebugMenu::BuildImguiCommands()
{
	Debug::Update();
	Debug::DrawInternal();
}

double DebugMenu::GetDeltaTime()
{
	return Debug::deltaTime;
}

void DebugMenu::Init()
{
	edDListGetMaterialLoadedDelegate() += Debug::OnMaterialLoaded;
	edDListGetMaterialUnloadedDelegate() += Debug::OnMaterialUnloaded;
	ed3DGetTextureLoadedDelegate() += Debug::OnTextureLoaded;
	ed3DGetMeshLoadedDelegate() += Debug::OnMeshLoaded;

	ed3D::DebugOptions::GetDisableClusterRendering() = Debug::gDisableClusterRendering;
	Renderer::GetForceAnimMatrixIdentity() = Debug::gForceAnimMatrixIdentity;
	VU1Emu::GetEnableEmulatedRendering() = Debug::gEnableEmulatedRendering;
	Renderer::Native::GetUsePreprocessedVertices() = Debug::gUsePreprocessedVertices;
}

static Input::InputFunctions gInputFunctions;

Input::InputFunctions& DebugMenu::GetInputFunctions()
{
	gInputFunctions.keyPressed = GetKeyPressed;
	gInputFunctions.keyReleased = GetKeyReleased;
	gInputFunctions.keyAnalog = GetKeyAnalog;

	gInputFunctions.mousePressed = GetMousePressed;
	gInputFunctions.mouseReleased = GetMouseReleased;
	gInputFunctions.mouseAnalog = GetMouseAnalog;
	return gInputFunctions;
}

std::unordered_map<uint32_t, ImGuiKey> gKeyMap = {
	{ ROUTE_START,				ImGuiKey_Enter },
	{ ROUTE_TRIANGLE,			ImGuiKey_Comma },	// 0x4
	{ ROUTE_CIRCLE,				ImGuiKey_E },		// 0x5
	{ ROUTE_CROSS,				ImGuiKey_A },		// 0x6
	{ ROUTE_SQUARE,				ImGuiKey_O },		// 0x7
	{ ROUTE_L1,					ImGuiKey_C },
	{ ROUTE_SELECT,				ImGuiKey_Backspace },
	{ ROUTE_UP,					ImGuiKey_UpArrow },
	{ ROUTE_DOWN,				ImGuiKey_DownArrow },
	{ ROUTE_L_ANALOG_UP,		ImGuiKey_G },
	{ ROUTE_L_ANALOG_DOWN,		ImGuiKey_H },
	{ ROUTE_L_ANALOG_LEFT,		ImGuiKey_D },
	{ ROUTE_L_ANALOG_RIGHT,		ImGuiKey_T },
};

bool DebugMenu::GetKeyPressed(uint32_t routeId)
{
	// Check if we are typing in a text box
	if (ImGui::GetIO().WantCaptureKeyboard) {
		return false;
	}

	assert(gKeyMap.find(routeId) != gKeyMap.end());

	return ImGui::IsKeyPressed(gKeyMap[routeId]);
}

bool DebugMenu::GetKeyReleased(uint32_t routeId)
{
	// Check if we are typing in a text box
	if (ImGui::GetIO().WantCaptureKeyboard) {
		return false;
	}

	assert(gKeyMap.find(routeId) != gKeyMap.end());

	return ImGui::IsKeyReleased(gKeyMap[routeId]);
}

float DebugMenu::GetKeyAnalog(uint32_t routeId)
{
	// Check if we are typing in a text box
	if (ImGui::GetIO().WantCaptureKeyboard) {
		return false;
	}

	assert(gKeyMap.find(routeId) != gKeyMap.end());

	return ImGui::IsKeyDown(gKeyMap[routeId]) ? 1.0f : 0.0f;
}

bool DebugMenu::GetMousePressed(uint32_t routeId)
{
	if (routeId == MOUSE_INPUT_BUTTON1) {
		return ImGui::IsMouseDown(ImGuiMouseButton_Left);
	}
	else if (routeId == MOUSE_INPUT_BUTTON2) {
		return ImGui::IsMouseDown(ImGuiMouseButton_Middle);
	}
	else if (routeId == MOUSE_INPUT_BUTTON3) {
		return ImGui::IsMouseDown(ImGuiMouseButton_Right);
	}

	return false;
}

bool DebugMenu::GetMouseReleased(uint32_t routeId)
{
	return false;
}

namespace DebugCamera {
	extern float gMouseDeltaX;
	extern float gMouseDeltaY;
}

float DebugMenu::GetMouseAnalog(uint32_t routeId)
{
	float delta = 0.0f;

	if (routeId == MOUSE_INPUT_DX) {
		delta = DebugCamera::gMouseDeltaX;
	}
	else if (routeId == MOUSE_INPUT_DY) {
		delta = DebugCamera::gMouseDeltaY;
	}
	else if (routeId == MOUSE_INPUT_WHEEL) {
		delta = ImGui::GetIO().MouseWheel;
	}

	return delta;
}