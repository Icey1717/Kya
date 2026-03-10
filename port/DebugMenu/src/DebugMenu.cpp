#include "DebugMenu.h"
#include "DebugRenderer.h"

#include "FrameBuffer.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "log.h"

#include "DebugHelpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <map>
#include <sstream>
#include <unordered_map>
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
#include "Actor/DebugActorCollision.h"
#include "DebugCollision.h"
#include "TimeController.h"
#include "FileManager3D.h"
#include "ActorHero_Private.h"
#include "Actor.h"
#include "ActorManager.h"
#include "ActorCheckpointManager.h"
#include "SectorManager.h"

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
#include "DebugSaveLoad.h"
#include "DebugCinematic.h"
#include "Native/NativeRenderer.h"
#include "LevelScheduler.h"
#include "DebugShop.h"
#include "DebugWolfen.h"
#include "DebugScenario.h"
#include "DebugMemory.h"
#include "DebugScene.h"
#include "DebugRendering.h"
#include "DebugTutorial.h"
#include "DebugInput.h"
#include "DebugHeroReplay.h"
#include "ed3D/ed3DG2D.h"

#define DEBUG_LOG(level, format, ...) MY_LOG_CATEGORY("Debug", level, format, ##__VA_ARGS__)

std::unique_ptr<std::vector<Debug::Menu>> Debug::MenuRegisterer::menus;

void Debug::Menu::Show()
{
	bool bOpen = GetOpen();
	if (bOpen) {
		ShowFunction(&bOpen);
		SetOpen(bOpen);
	}
}

bool Debug::Menu::GetOpen() const
{
	return pOpenSetting ? pOpenSetting->get() : false;
}

void Debug::Menu::SetOpen(bool bNewOpen)
{
	if (pOpenSetting && pOpenSetting->get() != bNewOpen) {
		*pOpenSetting = bNewOpen;
	}
}

Debug::MenuRegisterer::MenuRegisterer(const std::string& name, std::function<void(bool*)> showFunction, bool bDefaultOpen)
{
	if (!menus) {
		menus = std::make_unique<std::vector<Menu>>();
	}

	std::string settingName = "MenuOpen." + name;
	menus->push_back({
		name,
		showFunction,
		std::make_shared<Debug::Setting<bool>>(settingName, bDefaultOpen)
		});
}

std::vector<Debug::Menu>& Debug::MenuRegisterer::GetMenus()
{
	return *menus;
}

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

	struct MenuDefinition {
		const char* name;
		std::function<void(bool*)> ShowFunction;
		bool bOpen = false;
	};

	std::vector<MenuDefinition> gOldMenus = 
	{
		{"Demo", ImGui::ShowDemoWindow },
		{"Log", ShowLogWindow },
		{"Framebuffer", Debug::FrameBuffer::ShowMenu },
		{"Framebuffers", Debug::FrameBuffer::ShowFramebuffers },
		{"Cutscene", Debug::Cinematic::ShowMenu, true },
		{"Rendering", Debug::Rendering::ShowMenu, true },
		{"Hero", Debug::Hero::ShowMenu, true },
		{"Sector", ShowSectorMenu, true },
		{"Level Scheduler", ShowLevelSchedulerMenu, true },
		{"Memory", Debug::Memory::ShowMenu, false },
		{"Texture", Debug::Texture::ShowMenu, false },
		{"Mesh", Debug::Mesh::ShowMenu, false },
		{"Actor", Debug::Actor::ShowMenu, true },
		{"Checkpoint", Debug::Checkpoint::ShowMenu, true },
		{"Event", Debug::Event::ShowMenu, false },
		{"Shop", Debug::Shop::ShowMenu, true },
		{"Wolfen", Debug::Wolfen::ShowMenu, true },
		{"Scenario", Debug::Scenario::ShowMenu, true },
		{"Save/Load", Debug::SaveLoad::ShowMenu, true },
		{"Scene", Debug::Scene::ShowMenu, true },
		{"Collision", Debug::Collision::ShowMenu, false },
		{"Tutorial", Debug::Tutorial::ShowMenu, true },
		{"Input", Debug::Input::ShowMenu, true },
		{"Hero Replay", Debug::HeroReplay::ShowMenu, true },
	};

	enum class InspectorSelectionType {
		None,
		Actor,
		Sector,
		CheckpointManager,
		Checkpoint,
	};

	struct InspectorSelection {
		InspectorSelectionType type = InspectorSelectionType::None;
		CActor* pActor = nullptr;
		CActorCheckpointManager* pCheckpointManager = nullptr;
		int sectorIndex = -1;
		int checkpointIndex = -1;
	};

	static InspectorSelection gInspectorSelection;
	static bool gShowWorldPanel = false;
	static bool gShowInspectorPanel = false;
	static bool gShowDebugPanel = false;
	static bool gShowCameraWindow = false;
	static bool gResetDockLayout = false;
	static bool gDockLayoutInitialized = false;
	static int gToolbarLevelId = 0;
	static int gToolbarSectorId = 0;
	static int gStepFramesPending = 0;
	static float gResumeTimeScale = 1.0f;
	static ImGuiID gLeftDockId = 0;
	static ImGuiID gRightDockId = 0;
	static ImGuiID gBottomDockId = 0;
	static ImGuiID gCenterDockId = 0;

	static constexpr float kGameAspectRatio = 640.0f / 480.0f;
	static constexpr const char* kWorldWindowName = "World";
	static constexpr const char* kInspectorWindowName = "Inspector";
	static constexpr const char* kGameViewportWindowName = "GameViewport";
	static constexpr const char* kDebugWindowName = "Debug";

	static void ForEachMenu(std::function<void(Menu&)> action);

	static std::vector<CActorCheckpointManager*> GatherCheckpointManagers() {
		std::vector<CActorCheckpointManager*> checkpointManagers;

		auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;
		if (pActorManager == nullptr) {
			return checkpointManagers;
		}

		for (int i = 0; i < pActorManager->nbActors; ++i) {
			auto* pActor = pActorManager->aActors[i];
			if (pActor != nullptr && pActor->typeID == CHECKPOINT_MANAGER) {
				auto* pManager = dynamic_cast<CActorCheckpointManager*>(pActor);
				if (pManager != nullptr) {
					checkpointManagers.push_back(pManager);
				}
			}
		}

		return checkpointManagers;
	}

	static void SelectActor(CActor* pActor) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::Actor;
		gInspectorSelection.pActor = pActor;
	}

	static void SelectSector(int sectorIndex) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::Sector;
		gInspectorSelection.sectorIndex = sectorIndex;
	}

	static void SelectCheckpointManager(CActorCheckpointManager* pManager) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::CheckpointManager;
		gInspectorSelection.pCheckpointManager = pManager;
	}

	static void SelectCheckpoint(CActorCheckpointManager* pManager, int checkpointIndex) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::Checkpoint;
		gInspectorSelection.pCheckpointManager = pManager;
		gInspectorSelection.checkpointIndex = checkpointIndex;
	}

	static void DrawVector4(const char* label, const edF32VECTOR4& value) {
		ImGui::Text("%s: %.2f, %.2f, %.2f, %.2f", label, value.x, value.y, value.z, value.w);
	}

	static void DrawGameViewportImage() {
		static ImTextureID gFrameBuffer = DebugMenu::AddNativeFrameBuffer();

		const ImVec2 available = ImGui::GetContentRegionAvail();
		if (available.x <= 0.0f || available.y <= 0.0f) {
			return;
		}

		ImVec2 imageSize = available;
		if ((imageSize.x / imageSize.y) > kGameAspectRatio) {
			imageSize.x = imageSize.y * kGameAspectRatio;
		}
		else {
			imageSize.y = imageSize.x / kGameAspectRatio;
		}

		const ImVec2 cursorPos = ImGui::GetCursorPos();
		const ImVec2 centeredCursor(
			cursorPos.x + (available.x - imageSize.x) * 0.5f,
			cursorPos.y + (available.y - imageSize.y) * 0.5f);
		ImGui::SetCursorPos(centeredCursor);
		ImGui::Image(gFrameBuffer, imageSize);
	}

	static void BuildDefaultDockLayout(ImGuiID dockspaceId) {
		ImGuiDockNode* pExistingNode = ImGui::DockBuilderGetNode(dockspaceId);
		if (!gResetDockLayout) {
			if (gDockLayoutInitialized) {
				return;
			}

			if (pExistingNode != nullptr && (pExistingNode->IsSplitNode() || pExistingNode->Windows.Size > 0)) {
				gDockLayoutInitialized = true;
				gCenterDockId = dockspaceId;
				return;
			}
		}

		gResetDockLayout = false;

		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, pViewport->WorkSize);

		ImGuiID centerId = dockspaceId;
		ImGuiID leftId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Left, 0.20f, nullptr, &centerId);
		ImGuiID rightId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Right, 0.25f, nullptr, &centerId);
		ImGuiID bottomId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Down, 0.25f, nullptr, &centerId);
		gLeftDockId = leftId;
		gRightDockId = rightId;
		gBottomDockId = bottomId;
		gCenterDockId = centerId;

		ImGui::DockBuilderDockWindow(kWorldWindowName, leftId);
		ImGui::DockBuilderDockWindow(kInspectorWindowName, rightId);
		ImGui::DockBuilderDockWindow(kGameViewportWindowName, centerId);
		ImGui::DockBuilderDockWindow(kDebugWindowName, bottomId);
		ImGui::DockBuilderDockWindow("Camera", rightId);
		ImGui::DockBuilderDockWindow("Scene", bottomId);

		ImGui::DockBuilderFinish(dockspaceId);
		gDockLayoutInitialized = true;
	}

	static ImGuiID GetPreferredDockIdForMenu(const std::string& menuName) {
		if (menuName == "Log" || menuName == "Framebuffer" || menuName == "Framebuffers" ||
			menuName == "Rendering" || menuName == "Memory" || menuName == "Texture" ||
			menuName == "Mesh" || menuName == "Collision" || menuName == "Hero Replay" ||
			menuName == "Cutscene" || menuName == "Demo") {
			return gBottomDockId;
		}

		if (menuName == "Actor" || menuName == "Hero" || menuName == "Checkpoint" ||
			menuName == "Event" || menuName == "Shop" || menuName == "Wolfen" ||
			menuName == "Scenario" || menuName == "Save/Load" || menuName == "Scene" ||
			menuName == "Tutorial" || menuName == "Input" || menuName == "Sector" ||
			menuName == "Level Scheduler") {
			return gRightDockId;
		}

		return gBottomDockId;
	}

	static bool HasSavedWindowSettings(const char* pWindowName) {
		return ImGui::FindWindowSettingsByID(ImHashStr(pWindowName)) != nullptr;
	}

	static void DrawLegacyMenus() {
		ForEachMenu([](Menu& menu) {
			if (!menu.GetOpen()) {
				return;
			}

			const ImGuiID preferredDockId = GetPreferredDockIdForMenu(menu.name);
			if (preferredDockId != 0 && !HasSavedWindowSettings(menu.name.c_str())) {
				ImGui::SetNextWindowDockID(preferredDockId, ImGuiCond_FirstUseEver);
			}

			menu.Show();
		});
	}

	static void DrawLogContents() {
		ImGui::Text("Log Categories");
		ImGui::Separator();

		for (int level = static_cast<int>(LogLevel::VeryVerbose); level < static_cast<int>(LogLevel::Max); ++level) {
			LogLevel logLevel = static_cast<LogLevel>(level);
			ImGui::BulletText("%s", LogLevelToString(logLevel).c_str());
		}

		ImGui::Separator();

		for (auto& [category, log] : Log::GetInstance().GetLogs()) {
			if (ImGui::Checkbox(category.c_str(), &log.bEnabled)) {
				UpdateCategoryInConfigFile(category, log.bEnabled);
			}
		}
	}

	static void DrawAudioContents() {
		auto* pAudioManager = CScene::ptable.g_AudioManager_00451698;
		if (pAudioManager == nullptr) {
			ImGui::TextDisabled("Audio manager unavailable.");
			return;
		}

		ImGui::Text("Audio Manager: 0x%p", pAudioManager);
		ImGui::Text("field_0xcc: %.3f", pAudioManager->field_0xcc);
		ImGui::Separator();
		ImGui::TextWrapped("Audio debugging has not been split into a dedicated panel yet. This tab is the new home for audio-specific tools.");
	}

	static void DrawPerformanceContents() {
		const double fps = deltaTime > 0.0 ? (1.0 / deltaTime) : 0.0;
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0);
		ImGui::Separator();
		ImGui::Text("Render Time: %.1f ms", Renderer::Native::GetRenderTime());
		ImGui::Text("Render Wait Time: %.1f ms", Renderer::Native::GetRenderWaitTime());
		ImGui::Text("Render Thread Time: %.1f ms", Renderer::Native::GetRenderThreadTime());

		if (auto* pTimer = GetTimer(); pTimer != nullptr) {
			ImGui::Separator();
			ImGui::Text("Timer Scale: %.3f", pTimer->timeScale);
			ImGui::Text("Scaled Total Time: %.3f", pTimer->scaledTotalTime);
			ImGui::Text("Total Play Time: %.3f", pTimer->totalPlayTime);
		}
	}

	static void DrawWorldOverviewTab() {
		auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;
		auto* pSectorManager = CScene::ptable.g_SectorManager_00451670;

		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (pSectorManager != nullptr) {
				ImGui::BulletText("Current Sector: %d", pSectorManager->baseSector.currentSectorID);
				ImGui::BulletText("Loaded Sectors: %d", pSectorManager->nbSectors);
			}

			if (pActorManager != nullptr) {
				ImGui::BulletText("Actors: %d total / %d active / %d in sector", pActorManager->nbActors, pActorManager->nbActiveActors, pActorManager->nbSectorActors);
			}

			if (CLevelScheduler::gThis != nullptr) {
				ImGui::BulletText("Level: %d", CLevelScheduler::gThis->currentLevelID);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Managers", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::Selectable("Actor Manager", false) && pActorManager != nullptr) {
				gInspectorSelection = {};
			}
			if (ImGui::Selectable("Sector Manager", false) && pSectorManager != nullptr) {
				SelectSector(pSectorManager->baseSector.currentSectorID);
			}

			auto checkpointManagers = GatherCheckpointManagers();
			for (auto* pManager : checkpointManagers) {
				if (ImGui::Selectable(pManager->name, false)) {
					SelectCheckpointManager(pManager);
				}
			}

			ImGui::TreePop();
		}
	}

	static void DrawActorsTab() {
		auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;
		if (pActorManager == nullptr) {
			ImGui::TextDisabled("Actor manager unavailable.");
			return;
		}

		std::map<int, std::vector<CActor*>> actorsByType;
		for (int i = 0; i < pActorManager->nbActors; ++i) {
			CActor* pActor = pActorManager->aActors[i];
			if (pActor != nullptr) {
				actorsByType[pActor->typeID].push_back(pActor);
			}
		}

		for (auto& [typeId, actors] : actorsByType) {
			const char* pTypeName = Debug::Actor::GetActorTypeString(typeId);
			if (ImGui::TreeNodeEx((void*)(intptr_t)typeId, ImGuiTreeNodeFlags_DefaultOpen, "%s (%d)", pTypeName, static_cast<int>(actors.size()))) {
				for (CActor* pActor : actors) {
					const bool isSelected = gInspectorSelection.type == InspectorSelectionType::Actor && gInspectorSelection.pActor == pActor;
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);
					ImGui::TreeNodeEx((void*)pActor, flags, "%s [sector %d]", pActor->name, pActor->sectorId);
					if (ImGui::IsItemClicked()) {
						SelectActor(pActor);
					}
				}
				ImGui::TreePop();
			}
		}
	}

	static void DrawSectorsTab() {
		auto* pSectorManager = CScene::ptable.g_SectorManager_00451670;
		if (pSectorManager == nullptr) {
			ImGui::TextDisabled("Sector manager unavailable.");
			return;
		}

		if (ImGui::TreeNodeEx("Base Sector", ImGuiTreeNodeFlags_DefaultOpen)) {
			const bool selected = gInspectorSelection.type == InspectorSelectionType::Sector &&
				gInspectorSelection.sectorIndex == pSectorManager->baseSector.currentSectorID;
			if (ImGui::Selectable("Current Sector", selected)) {
				SelectSector(pSectorManager->baseSector.currentSectorID);
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Loaded Sectors", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (int i = 0; i < pSectorManager->nbSectors; ++i) {
				const CSector& sector = pSectorManager->aSectors[i];
				const bool selected = gInspectorSelection.type == InspectorSelectionType::Sector &&
					gInspectorSelection.sectorIndex == sector.currentSectorID;
				char sectorLabel[64] = {};
				sprintf_s(sectorLabel, "Sector %d##%d", sector.currentSectorID, i);
				if (ImGui::Selectable(sectorLabel, selected)) {
					SelectSector(sector.currentSectorID);
				}
			}
			ImGui::TreePop();
		}
	}

	static void DrawCheckpointsTab() {
		auto checkpointManagers = GatherCheckpointManagers();
		if (checkpointManagers.empty()) {
			ImGui::TextDisabled("No checkpoint managers found.");
			return;
		}

		for (auto* pManager : checkpointManagers) {
			ImGuiTreeNodeFlags managerFlags = ImGuiTreeNodeFlags_DefaultOpen;
			if (gInspectorSelection.type == InspectorSelectionType::CheckpointManager &&
				gInspectorSelection.pCheckpointManager == pManager) {
				managerFlags |= ImGuiTreeNodeFlags_Selected;
			}

			const bool open = ImGui::TreeNodeEx((void*)pManager, managerFlags, "%s (%d)", pManager->name, pManager->checkpointCount);
			if (ImGui::IsItemClicked()) {
				SelectCheckpointManager(pManager);
			}

			if (!open) {
				continue;
			}

			for (int checkpointIndex = 0; checkpointIndex < pManager->checkpointCount; ++checkpointIndex) {
				const S_CHECKPOINT& checkpoint = pManager->aCheckpoints[checkpointIndex];
				const bool selected = gInspectorSelection.type == InspectorSelectionType::Checkpoint &&
					gInspectorSelection.pCheckpointManager == pManager &&
					gInspectorSelection.checkpointIndex == checkpointIndex;
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | (selected ? ImGuiTreeNodeFlags_Selected : 0);
				ImGui::TreeNodeEx((void*)(intptr_t)checkpointIndex, flags, "Checkpoint %d [sector %d]", checkpointIndex, checkpoint.sectorId);
				if (ImGui::IsItemClicked()) {
					SelectCheckpoint(pManager, checkpointIndex);
				}
			}

			ImGui::TreePop();
		}
	}

	static void DrawWorldPanel() {
		if (!gShowWorldPanel) {
			return;
		}

		ImGui::Begin(kWorldWindowName, &gShowWorldPanel);
		if (ImGui::BeginTabBar("WorldTabs")) {
			if (ImGui::BeginTabItem("World")) {
				DrawWorldOverviewTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Actors")) {
				DrawActorsTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Sectors")) {
				DrawSectorsTab();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Checkpoints")) {
				DrawCheckpointsTab();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	static void DrawActorInspector(CActor* pActor) {
		if (pActor == nullptr) {
			ImGui::TextDisabled("Actor selection is no longer valid.");
			return;
		}

		ImGui::Text("%s", pActor->name);
		ImGui::TextDisabled("%s", Debug::Actor::GetActorTypeString(pActor->typeID));

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
			DrawVector4("Position", pActor->currentLocation);
			DrawVector4("Rotation Euler", pActor->rotationEuler);
			DrawVector4("Rotation Quaternion", pActor->rotationQuat);
			DrawVector4("Scale", pActor->scale);
		}

		if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Current Animation: %d", pActor->currentAnimType);
			ImGui::Text("Current Behaviour: %d", pActor->curBehaviourId);
			ImGui::Text("Previous Behaviour: %d", pActor->prevBehaviourId);
			ImGui::Text("Distance To Camera: %.2f", pActor->distanceToCamera);
			ImGui::Text("Distance To Ground: %.2f", pActor->distanceToGround);
		}

		if (ImGui::CollapsingHeader("Debug Info", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Sector ID: %d", pActor->sectorId);
			ImGui::Text("Actor Manager Index: %d", pActor->actorManagerIndex);
			ImGui::Text("Flags: 0x%08X", pActor->flags);
			ImGui::Text("Actor FieldS: 0x%08X", pActor->actorFieldS);
			ImGui::Text("Mesh Node: 0x%p", pActor->pMeshNode);
		}

		if (ImGui::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("state_0x10: %u", pActor->state_0x10);
			ImGui::Text("field_0x11: %u", pActor->field_0x11);
			ImGui::Text("Macro Anim Table: 0x%p", pActor->pMacroAnimTable);
			ImGui::Text("Hierarchy: 0x%p", pActor->pHier);
		}
	}

	static void DrawSectorInspector(int selectedSectorId) {
		auto* pSectorManager = CScene::ptable.g_SectorManager_00451670;
		if (pSectorManager == nullptr) {
			ImGui::TextDisabled("Sector manager unavailable.");
			return;
		}

		const CSector* pSelectedSector = nullptr;
		if (pSectorManager->baseSector.currentSectorID == selectedSectorId) {
			pSelectedSector = &pSectorManager->baseSector;
		}
		else {
			for (int i = 0; i < pSectorManager->nbSectors; ++i) {
				if (pSectorManager->aSectors[i].currentSectorID == selectedSectorId) {
					pSelectedSector = &pSectorManager->aSectors[i];
					break;
				}
			}
		}

		if (pSelectedSector == nullptr) {
			ImGui::TextDisabled("Sector %d is not currently loaded.", selectedSectorId);
			return;
		}

		ImGui::Text("Sector %d", selectedSectorId);
		if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Desired Sector ID: %d", pSelectedSector->desiredSectorID);
			ImGui::Text("Current Sector ID: %d", pSelectedSector->currentSectorID);
			ImGui::Text("Sector Index: %d", pSelectedSector->sectorIndex);
			ImGui::Text("Load Stage: %d", pSelectedSector->loadStage_0x8);
		}

		if (ImGui::CollapsingHeader("Debug Info", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Mesh: 0x%p", &pSelectedSector->sectorMesh);
			ImGui::Text("Texture: 0x%p", &pSelectedSector->sectorTexture);
			ImGui::Text("OBB Tree: 0x%p", pSelectedSector->pObbTree);
		}

		if (ImGui::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Sector Root: %s", pSectorManager->szSectorFileRoot);
			ImGui::Text("Total Loaded Sectors: %d", pSectorManager->nbSectors);
		}
	}

	static void DrawCheckpointManagerInspector(CActorCheckpointManager* pManager) {
		if (pManager == nullptr) {
			ImGui::TextDisabled("Checkpoint manager selection is no longer valid.");
			return;
		}

		ImGui::Text("%s", pManager->name);
		if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Checkpoint Count: %d", pManager->checkpointCount);
			ImGui::Text("Current Checkpoint Index: %d", pManager->currentCheckpointIndex);
		}

		if (ImGui::CollapsingHeader("Debug Info", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Sector ID: %d", pManager->sectorId);
			ImGui::Text("Type: %s", Debug::Actor::GetActorTypeString(pManager->typeID));
			ImGui::Text("Flags: 0x%08X", pManager->flags);
		}
	}

	static void DrawCheckpointInspector(CActorCheckpointManager* pManager, int checkpointIndex) {
		if (pManager == nullptr || checkpointIndex < 0 || checkpointIndex >= pManager->checkpointCount) {
			ImGui::TextDisabled("Checkpoint selection is no longer valid.");
			return;
		}

		S_CHECKPOINT& checkpoint = pManager->aCheckpoints[checkpointIndex];
		ImGui::Text("%s / Checkpoint %d", pManager->name, checkpointIndex);

		if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Sector ID: %d", checkpoint.sectorId);
			ImGui::Text("Flags: 0x%08X", checkpoint.flags);
			ImGui::Text("Actor Waypoints: %d", checkpoint.actorWaypointsCount);
		}

		if (ImGui::CollapsingHeader("Debug Info", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Zone: 0x%p", checkpoint.pZone.Get());
			ImGui::Text("Waypoint A: 0x%p", checkpoint.pWayPointA.Get());
			ImGui::Text("Waypoint B: 0x%p", checkpoint.pWayPointB.Get());
		}

		if (ImGui::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Manager Current Checkpoint: %d", pManager->currentCheckpointIndex);
		}
	}

	static void DrawInspectorPanel() {
		if (!gShowInspectorPanel) {
			return;
		}

		ImGui::Begin(kInspectorWindowName, &gShowInspectorPanel);
		switch (gInspectorSelection.type) {
		case InspectorSelectionType::Actor:
			DrawActorInspector(gInspectorSelection.pActor);
			break;

		case InspectorSelectionType::Sector:
			DrawSectorInspector(gInspectorSelection.sectorIndex);
			break;

		case InspectorSelectionType::CheckpointManager:
			DrawCheckpointManagerInspector(gInspectorSelection.pCheckpointManager);
			break;

		case InspectorSelectionType::Checkpoint:
			DrawCheckpointInspector(gInspectorSelection.pCheckpointManager, gInspectorSelection.checkpointIndex);
			break;

		default:
			ImGui::TextDisabled("Select an actor, sector, or checkpoint from the World panel.");
			break;
		}
		ImGui::End();
	}

	static void DrawGameViewportWindow() {
		ImGui::Begin(kGameViewportWindowName, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		DrawGameViewportImage();
		ImGui::End();
	}

	static void DrawFullscreenGameViewportWindow() {
		ImGuiViewport* pViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(pViewport->Pos);
		ImGui::SetNextWindowSize(pViewport->Size);
		ImGui::SetNextWindowViewport(pViewport->ID);
		ImGui::Begin(
			"GameViewportFullscreen",
			nullptr,
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoInputs);
		DrawGameViewportImage();
		ImGui::End();
	}

	static void DrawDebugPanel() {
		if (!gShowDebugPanel) {
			return;
		}

		ImGui::Begin(kDebugWindowName, &gShowDebugPanel);
		if (ImGui::BeginTabBar("DebugTabs")) {
			if (ImGui::BeginTabItem("Logs")) {
				DrawLogContents();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Performance")) {
				DrawPerformanceContents();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Rendering Debug")) {
				if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
					Debug::Rendering::DrawContents();
				}
				if (ImGui::CollapsingHeader("Collision", ImGuiTreeNodeFlags_DefaultOpen)) {
					Debug::Collision::DrawContents();
				}
				if (ImGui::CollapsingHeader("Framebuffer", ImGuiTreeNodeFlags_DefaultOpen)) {
					Debug::FrameBuffer::DrawContents();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Replay")) {
				Debug::HeroReplay::DrawContents();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Audio")) {
				DrawAudioContents();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	static void UpdateSingleStepState() {
		if (gStepFramesPending <= 0) {
			return;
		}

		--gStepFramesPending;
		if (gStepFramesPending == 0) {
			if (auto* pTimer = GetTimer(); pTimer != nullptr) {
				pTimer->timeScale = 0.0f;
			}
		}
	}

	static void DrawToolbar() {
		auto* pTimer = GetTimer();
		auto* pSectorManager = CScene::ptable.g_SectorManager_00451670;
		const double fps = deltaTime > 0.0 ? (1.0 / deltaTime) : 0.0;

		if (CLevelScheduler::gThis != nullptr && gToolbarLevelId == 0) {
			gToolbarLevelId = CLevelScheduler::gThis->currentLevelID;
		}
		if (pSectorManager != nullptr && gToolbarSectorId == 0) {
			gToolbarSectorId = pSectorManager->baseSector.currentSectorID;
		}

		if (ImGui::BeginMainMenuBar()) {
			const bool bPaused = pTimer != nullptr && pTimer->timeScale == 0.0f;
			//
			//if (ImGui::SmallButton("Pause") && pTimer != nullptr) {
			//	if (pTimer->timeScale > 0.0f) {
			//		gResumeTimeScale = pTimer->timeScale;
			//	}
			//	pTimer->timeScale = 0.0f;
			//}
			//ImGui::SameLine();
			//if (ImGui::SmallButton("Step") && pTimer != nullptr) {
			//	if (gResumeTimeScale <= 0.0f) {
			//		gResumeTimeScale = 1.0f;
			//	}
			//	pTimer->timeScale = gResumeTimeScale;
			//	gStepFramesPending = 1;
			//}
			//ImGui::SameLine();
			//if (ImGui::SmallButton("Resume") && pTimer != nullptr) {
			//	if (gResumeTimeScale <= 0.0f) {
			//		gResumeTimeScale = 1.0f;
			//	}
			//	pTimer->timeScale = gResumeTimeScale;
			//	gStepFramesPending = 0;
			//}
			//
			//ImGui::SameLine();
			//ImGui::Separator();
			//ImGui::SameLine();
			//
			//ImGui::TextUnformatted("Level");
			//ImGui::SameLine();
			//ImGui::SetNextItemWidth(80.0f);
			//ImGui::InputInt("##ToolbarLevel", &gToolbarLevelId, 1, 1, ImGuiInputTextFlags_CharsDecimal);
			//if (CLevelScheduler::gThis != nullptr) {
			//	CLevelScheduler::gThis->currentLevelID = gToolbarLevelId;
			//}
			//
			//ImGui::SameLine();
			//ImGui::TextUnformatted("Sector");
			//ImGui::SameLine();
			//ImGui::SetNextItemWidth(70.0f);
			//ImGui::InputInt("##ToolbarSector", &gToolbarSectorId, 1, 1, ImGuiInputTextFlags_CharsDecimal);
			//ImGui::SameLine();
			//if (ImGui::SmallButton("Apply Sector") && pSectorManager != nullptr) {
			//	pSectorManager->SwitchToSector(gToolbarSectorId, true);
			//}
			//
			//ImGui::SameLine();
			//ImGui::Separator();
			//ImGui::SameLine();
			ImGui::Checkbox("World", &gShowWorldPanel);
			ImGui::SameLine();
			ImGui::Checkbox("Inspector", &gShowInspectorPanel);
			ImGui::SameLine();
			ImGui::Checkbox("Debug", &gShowDebugPanel);
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset Layout")) {
				gResetDockLayout = true;
			}
			
			ImGui::SameLine();
			ImGui::Separator();
			ImGui::SameLine();
			if (ImGui::BeginMenu("Tools")) {
				ImGui::MenuItem("Camera", nullptr, &gShowCameraWindow);
				ImGui::Separator();
			
				ForEachMenu([](Menu& menu) {
					bool bOpen = menu.GetOpen();
					if (ImGui::MenuItem(menu.name.c_str(), nullptr, &bOpen)) {
						menu.SetOpen(bOpen);
					}
				});
			
				ImGui::EndMenu();
			}
			
			ImGui::SameLine();
			ImGui::Separator();
			ImGui::SameLine();
			ImGui::Text("FPS %.1f | %s", fps, bPaused ? "Paused" : "Running");
			//
			ImGui::EndMainMenuBar();
		}
	}

	static void ForEachMenu(std::function<void(Menu&)> action) {
		for (auto& currentMenu : MenuRegisterer::GetMenus()) {
			action(currentMenu);
		}
	}

	static void DrawInternal() {
		if (ImGui::IsKeyPressed(ImGuiKey_F10)) {
			bShowMenus = !bShowMenus;
		}

		static bool bRunningTerm = false;
		UpdateSingleStepState();

		Debug::Collision::DrawDebugShapes();
		Texture::Update();
		Debug::Shop::Update();
		Debug::Scenario::Update();
		Debug::SaveLoad::Update();
		Debug::HeroReplay::Update();

		if (bRunningTerm) {
			bRunningTerm = false;
		}

		if ((GameFlags & 0x2) != 0 && !bRunningTerm) {
			bRunningTerm = true;
		}

		if (!bShowMenus || bRunningTerm) {
			DrawFullscreenGameViewportWindow();
			return;
		}

		DrawToolbar();

		const ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
		BuildDefaultDockLayout(dockspaceId);

		DrawWorldPanel();
		DrawInspectorPanel();
		DrawGameViewportWindow();
		DrawDebugPanel();
		DrawLegacyMenus();

		if (gShowCameraWindow) {
			if (gRightDockId != 0 && !HasSavedWindowSettings("Camera")) {
				ImGui::SetNextWindowDockID(gRightDockId, ImGuiCond_FirstUseEver);
			}
			Debug::Camera::ShowCamera();
		}
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

	Debug::Scene::Startup();
	Debug::Rendering::Init();

	// Register old menus
	for (const auto& menu : Debug::gOldMenus) {
		Debug::MenuRegisterer registerer(menu.name, menu.ShowFunction, menu.bOpen);
	}
}

void DebugMenu::AddKeyboardMouseSupport()
{
	Input::gInputFunctions.keyPressed = GetKeyPressed;
	Input::gInputFunctions.keyReleased = GetKeyReleased;
	Input::gInputFunctions.keyAnalog = GetKeyAnalog;

	Input::gInputFunctions.mousePressed = GetMousePressed;
	Input::gInputFunctions.mouseReleased = GetMouseReleased;
	Input::gInputFunctions.mouseAnalog = GetMouseAnalog;
}

static std::unordered_map<uint32_t, ImGuiKey> gKeyMap = {
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
	{ ROUTE_R_ANALOG_DOWN,		ImGuiKey_K },
	{ ROUTE_R_ANALOG_UP,		ImGuiKey_I },
	{ ROUTE_R_ANALOG_LEFT,		ImGuiKey_J },
	{ ROUTE_R_ANALOG_RIGHT,		ImGuiKey_L },
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
