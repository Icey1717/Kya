#include "DebugMenuWorld.h"

#include <profiling.h>
#include <imgui.h>
#include <map>
#include <vector>

#include "Actor.h"
#include "ActorManager.h"
#include "ActorCheckpointManager.h"
#include "ActorWind.h"
#include "ActorWolfen.h"
#include "SectorManager.h"
#include "LevelScheduler.h"
#include "Types.h"
#include "DebugSetting.h"
#include "Actor/DebugActor.h"
#include "Actor/DebugActorWind.h"
#include "Actor/DebugActorWolfen.h"

namespace Debug {

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
	static Debug::Setting<bool> gShowWorldPanel("Show World Panel", true);
	static Debug::Setting<bool> gShowInspectorPanel("Show Inspector Panel", true);
	static int gSectorFilter = -1;

	static constexpr const char* kWorldWindowName = "World";
	static constexpr const char* kInspectorWindowName = "Inspector";

	bool GetShowWorldPanel() { return gShowWorldPanel; }
	void SetShowWorldPanel(bool bShow) { gShowWorldPanel = bShow; }

	bool GetShowInspectorPanel() { return gShowInspectorPanel; }
	void SetShowInspectorPanel(bool bShow) { gShowInspectorPanel = bShow; }

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

	// ---- World panel tabs ----

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

		// Sector filter controls
		ImGui::Text("Sector Filter:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::InputInt("##SectorFilter", &gSectorFilter);
		ImGui::SameLine();
		if (ImGui::Button("Clear Filter")) {
			gSectorFilter = -1;
		}
		ImGui::SameLine();
		auto* pSectorManager = CScene::ptable.g_SectorManager_00451670;
		if (pSectorManager != nullptr && ImGui::Button("Current")) {
			gSectorFilter = pSectorManager->baseSector.currentSectorID;
		}

		ImGui::Separator();

		std::map<int, std::vector<CActor*>> actorsByType;
		for (int i = 0; i < pActorManager->nbActors; ++i) {
			CActor* pActor = pActorManager->aActors[i];
			if (pActor != nullptr) {
				// Apply sector filter
				if (gSectorFilter >= 0 && pActor->sectorId != gSectorFilter) {
					continue;
				}
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

	void DrawWorldPanel() {
		if (!gShowWorldPanel) {
			return;
		}

		ZONE_SCOPED;

		bool bOpen = gShowWorldPanel;

		ImGui::Begin(kWorldWindowName, &bOpen);
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

		if (!bOpen) {
			gShowWorldPanel = false;
		}

		ImGui::End();
	}

	// ---- Inspector panel ----

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

		// Show wind-specific info if this is a wind actor
		if (pActor->typeID == WIND) {
			CActorWind* pWindActor = static_cast<CActorWind*>(pActor);
			ImGui::Separator();
			Debug::Actor::Wind::ShowWindActorDetails(pWindActor);
		}

		// Show wolfen-specific info if this is a wolfen actor
		if (pActor->typeID == WOLFEN) {
			CActorWolfen* pWolfenActor = static_cast<CActorWolfen*>(pActor);
			ImGui::Separator();
			Debug::Actor::Wolfen::ShowWolfenActorDetails(pWolfenActor);
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

	void DrawInspectorPanel() {
		if (!gShowInspectorPanel) {
			return;
		}

		ZONE_SCOPED;

		bool bOpen = gShowInspectorPanel;

		ImGui::Begin(kInspectorWindowName, &bOpen);
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

		if (!bOpen) {
			gShowInspectorPanel = false;
		}
	}

} // namespace Debug
