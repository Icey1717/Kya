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
#include "CameraViewManager.h"
#include "DebugCamera.h"
#include "TextureCache.h"
#include "CinematicManager.h"
#include "port/vu1_emu.h"
#include "DebugMeshViewer.h"
#include "DebugTexture.h"
#include "Actor/DebugActor.h"
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
#include "VulkanRenderer.h"
#include "DebugMesh.h"
#include "DebugFrameBuffer.h"
#include "DebugCheckpoint.h"
#include "DebugEvent.h"
#include "DebugCinematic.h"
#include "Native/NativeRenderer.h"
#include "LevelScheduleManager.h"

#define DEBUG_LOG(level, format, ...) MY_LOG_CATEGORY("Debug", level, format, ##__VA_ARGS__)

extern bool bOther;

namespace Debug {
	static bool bShowMenus = true;

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

		const ImVec2 windowSize(100.0f, 50.0f);

		// Get the screen size
		const ImVec2 screenSize = ImGui::GetIO().DisplaySize;

		// Render the framerate counter
		ImGui::SetNextWindowSize(windowSize);
		ImGui::SetNextWindowPos(ImVec2(screenSize.x - windowSize.x - 10.0f, 10.0f), ImGuiCond_Always);
		ImGui::Begin("Framerate Counter", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("%.3f ms", deltaTime * 1000.0);
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

	static void ShowLevelSchedulerMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Level Scheduler", bOpen, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Boomy Level: %d", CLevelScheduler::GetBoomyLevel());
		ImGui::Text("Medallion Level: %d", CLevelScheduler::GetMedallionLevel());
		ImGui::Text("Fight Level: %d", CLevelScheduler::GetFightLevel());

		ImGui::InputInt("Current Level", &CLevelScheduler::gThis->currentLevelID);
		// End the ImGui window
		ImGui::End();
	}

	static Debug::Setting<bool> gDisableClusterRendering = { "Disable Cluster Rendering", false };
	static Debug::Setting<bool> gForceAnimMatrixIdentity = { "Force animation matrix to identity", false };
	static Debug::Setting<bool> gEnableEmulatedRendering = { "Enable Emulated Rendering", false };

	static void ShowRenderingMenu(bool* bOpen) {
		// Create a new ImGui window
		ImGui::Begin("Rendering", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Render Time: %.1f ms", Renderer::Native::GetRenderTime());
		ImGui::Text("Render Wait Time: %.1f ms", Renderer::Native::GetRenderWaitTime());
		ImGui::Text("Render Thread Time: %.1f ms", Renderer::Native::GetRenderThreadTime());

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

		if (gDisableClusterRendering.DrawImguiControl()) {
			ed3D::DebugOptions::GetDisableClusterRendering() = gDisableClusterRendering;
		}

		if (gForceAnimMatrixIdentity.DrawImguiControl()) {
			Renderer::GetForceAnimMatrixIdentity() = gForceAnimMatrixIdentity;
		}

		if (gEnableEmulatedRendering.DrawImguiControl()) {
			VU1Emu::GetEnableEmulatedRendering() = gEnableEmulatedRendering;
		}

		// End the ImGui window
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

		if (ImGui::Button("Term Scene")) {
			GameFlags = GameFlags | GAME_REQUEST_TERM;
		}

		if (ImGui::CollapsingHeader("Teleport", ImGuiTreeNodeFlags_DefaultOpen)) {
			static Debug::Setting<int> gTeleportLevelId = { "Teleport Level ID", 0 };
			static Debug::Setting<int> gTeleportElevatorId = { "Teleport Elevator ID", 0 };
			static Debug::Setting<int> gTeleportCutsceneId = { "Teleport Cutscene ID", 0 };

			gTeleportLevelId.DrawImguiControl();
			gTeleportElevatorId.DrawImguiControl();
			gTeleportCutsceneId.DrawImguiControl();

			if (ImGui::Button("Level Teleport")) {
				bShowMenus = false;
				CScene::ptable.g_LevelScheduleManager_00451660->Level_Teleport(nullptr, gTeleportLevelId, gTeleportElevatorId, gTeleportCutsceneId, -1);
			}
		}

		if (ImGui::CollapsingHeader("Scene List")) {

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

			if (selectedScene != -1 && ImGui::CollapsingHeader("Scene Details", ImGuiTreeNodeFlags_DefaultOpen)) {
				ed_3D_Scene* pSelectedScene = &gScene3D[selectedScene];

				static bool bFilterAnim = false;
				ImGui::Checkbox("Filter Anim", &bFilterAnim);

				ImGui::Text("Shadow: %d", pSelectedScene->bShadowScene);
				ImGui::Text("Flags: 0x%x", pSelectedScene->flags);

				if (ImGui::CollapsingHeader("Hierarchy")) {
					edNODE* pCurNode;
					edLIST* pList = pSelectedScene->pHierListA;
					if (((pSelectedScene->flags & SCENE_FLAG_IN_USE) != 0) && ((pSelectedScene->flags & 4) == 0)) {
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

		if (ImGui::CollapsingHeader("Vulkan")) {
			ImGui::Text("Allocations:");
			ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
			ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
			ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_CACHE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_CACHE));
			ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE));
			ImGui::Text("VkAllocations VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: %u", GetAllocationCount(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));
		}

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
		{"Framebuffer", Debug::FrameBuffer::ShowMenu },
		{"Framebuffers", Debug::FrameBuffer::ShowFramebuffers },
		{"Cutscene", Debug::Cinematic::ShowMenu, true },
		{"Rendering", ShowRenderingMenu },
		{"Hero", Debug::Hero::ShowMenu, true },
		{"Sector", ShowSectorMenu, true },
		{"Level Scheduler", ShowLevelSchedulerMenu, true },
		{"Memory", ShowMemoryMenu, false },
		{"Texture", Debug::Texture::ShowMenu, false },
		{"Mesh", Debug::Mesh::ShowMenu, false },
		{"Actor", Debug::Actor::ShowMenu, true },
		{"Checkpoint", Debug::Checkpoint::ShowMenu, true },
		{"Event", Debug::Event::ShowMenu, false },
		{"Scene", ShowSceneMenu, true },
	};

	static void ForEachMenu(std::function<void(Menu&)> action) {
		for (auto& currentMenu : gMenus) {
			action(currentMenu);
		}
	}

	static void DrawMenuBar() {
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
		if (ImGui::IsKeyPressed(ImGuiKey_F10)) {
			bShowMenus = !bShowMenus;
		}

		static bool bRunningTerm = false;

		if (bShowMenus && !bRunningTerm) {
			Texture::Update();
			Debug::Camera::ShowCamera();

			ForEachMenu([](Menu& menu) {
				menu.Show();
				});

			DrawMenuBar();
		}

		if (bRunningTerm) {
			bRunningTerm = false;
		}

		if ((GameFlags & 0x2) != 0 && !bRunningTerm) {
			bRunningTerm = true;
		}

		//Debug::FrameBuffer::ShowGame();
		Debug::FrameBuffer::ShowNativeFrameBuffer(nullptr);
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
	{ ROUTE_L2,					ImGuiKey_Q },		// 0x0
	{ ROUTE_R2,					ImGuiKey_W },		// 0x1
	{ ROUTE_L1,					ImGuiKey_C },		// 0x2
	{ ROUTE_R1,					ImGuiKey_R },		// 0x3
	{ ROUTE_TRIANGLE,			ImGuiKey_Comma },	// 0x4
	{ ROUTE_CIRCLE,				ImGuiKey_E },		// 0x5
	{ ROUTE_CROSS,				ImGuiKey_A },		// 0x6
	{ ROUTE_SQUARE,				ImGuiKey_O },		// 0x7
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

bool DebugMenu::GetEnableEmulatedRendering()
{
	return Debug::gEnableEmulatedRendering;
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

namespace Debug::Camera {
	extern float gMouseDeltaX;
	extern float gMouseDeltaY;
}

float DebugMenu::GetMouseAnalog(uint32_t routeId)
{
	float delta = 0.0f;

	if (routeId == MOUSE_INPUT_DX) {
		delta = Debug::Camera::gMouseDeltaX;
	}
	else if (routeId == MOUSE_INPUT_DY) {
		delta = Debug::Camera::gMouseDeltaY;
	}
	else if (routeId == MOUSE_INPUT_WHEEL) {
		delta = ImGui::GetIO().MouseWheel;
	}

	return delta;
}