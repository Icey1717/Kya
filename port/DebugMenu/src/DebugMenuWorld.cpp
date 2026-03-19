#include "DebugMenuWorld.h"

#include <profiling.h>
#include <imgui.h>
#include <map>
#include <vector>

#include "DebugWorldNames.h"
#include "Actor.h"
#include "ActorManager.h"
#include "ActorCheckpointManager.h"
#include "ActorWind.h"
#include "ActorWolfen.h"
#include "ActorJamGut.h"
#include "LargeObject.h"
#include "ActorHero.h"
#include "ActorHero_Private.h"
#include "SectorManager.h"
#include "LevelScheduler.h"
#include "Types.h"
#include "DebugSetting.h"
#include "Actor/DebugActor.h"
#include "Actor/DebugActorWind.h"
#include "Actor/DebugActorWolfen.h"
#include "Actor/DebugActorBehaviour.h"
#include "Actor/DebugActorJamGut.h"

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

	static void FocusInspector() {
		ImGui::SetWindowFocus(kInspectorWindowName);
	}

	static void SelectActor(CActor* pActor) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::Actor;
		gInspectorSelection.pActor = pActor;
		FocusInspector();
	}

	static void SelectSector(int sectorIndex) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::Sector;
		gInspectorSelection.sectorIndex = sectorIndex;
		FocusInspector();
	}

	static void SelectCheckpointManager(CActorCheckpointManager* pManager) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::CheckpointManager;
		gInspectorSelection.pCheckpointManager = pManager;
		FocusInspector();
	}

	static void SelectCheckpoint(CActorCheckpointManager* pManager, int checkpointIndex) {
		gInspectorSelection = {};
		gInspectorSelection.type = InspectorSelectionType::Checkpoint;
		gInspectorSelection.pCheckpointManager = pManager;
		gInspectorSelection.checkpointIndex = checkpointIndex;
		FocusInspector();
	}

	static void DrawVector4(const char* label, const edF32VECTOR4& value) {
		ImGui::Text("%s: %.2f, %.2f, %.2f, %.2f", label, value.x, value.y, value.z, value.w);
	}

	static int GetCurrentLevelId()
	{
		if (CLevelScheduler::gThis != nullptr) {
			return CLevelScheduler::gThis->currentLevelID;
		}
		return -1;
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

		auto sortByTypeName = [](int a, int b) {
			const char* aName = Debug::Actor::GetActorTypeString(a);
			const char* bName = Debug::Actor::GetActorTypeString(b);
			return strcmp(aName, bName) < 0;
			};

		std::map<int, std::vector<CActor*>, decltype(sortByTypeName)> actorsByType(sortByTypeName);
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
			if (ImGui::TreeNodeEx((void*)(intptr_t)typeId, ImGuiTreeNodeFlags_None, "%s (%d)", pTypeName, static_cast<int>(actors.size()))) {
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

		struct CheckpointRef {
			CActorCheckpointManager* pManager;
			int index;
		};
		std::map<int, std::vector<CheckpointRef>> checkpointsBySector;
		for (auto* pManager : GatherCheckpointManagers()) {
			for (int i = 0; i < pManager->checkpointCount; ++i) {
				checkpointsBySector[pManager->aCheckpoints[i].sectorId].push_back({ pManager, i });
			}
		}

		// Build a lookup of currently loaded sectors.
		std::map<int, const CSector*> loadedSectors;
		int currentSectorId = -1;
		if (pSectorManager != nullptr) {
			currentSectorId = pSectorManager->baseSector.currentSectorID;
			loadedSectors[currentSectorId] = &pSectorManager->baseSector;
			for (int i = 0; i < pSectorManager->nbSectors; ++i) {
				const CSector& s = pSectorManager->aSectors[i];
				loadedSectors[s.currentSectorID] = &s;
			}
		}

		// Enumerate all sectors defined for this level via the level scheduler.
		std::vector<int> allSectorIds;
		if (CLevelScheduler::gThis != nullptr) {
			const int levelId = CLevelScheduler::gThis->currentLevelID;
			const S_LEVEL_INFO& levelInfo = CLevelScheduler::gThis->aLevelInfo[levelId];
			for (int i = 0; i <= levelInfo.maxSectorId; ++i) {
				if (levelInfo.aSectorSubObj[i].bankSize > 0) {
					allSectorIds.push_back(i);
				}
			}
		}

		for (auto& [id, _] : loadedSectors) {
			if (std::find(allSectorIds.begin(), allSectorIds.end(), id) == allSectorIds.end()) {
				allSectorIds.push_back(id);
			}
		}

		if (allSectorIds.empty()) {
			ImGui::TextDisabled("No sector data available.");
			return;
		}

		for (int sectorId : allSectorIds) {
			const bool selected = gInspectorSelection.type == InspectorSelectionType::Sector &&
				gInspectorSelection.sectorIndex == sectorId;

			auto cpit = checkpointsBySector.find(sectorId);
			const bool hasCheckpoints = cpit != checkpointsBySector.end();
			const bool isLoaded = loadedSectors.count(sectorId) > 0;
			const bool isCurrent = (sectorId == currentSectorId);

			ImGuiTreeNodeFlags sectorFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			if (!hasCheckpoints) sectorFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (selected) sectorFlags |= ImGuiTreeNodeFlags_Selected;

			const char* suffix = isCurrent ? " (current)" : (isLoaded ? " (loaded)" : " [inactive]");

			std::string sectorName = WorldNames::GetSectorName(GetCurrentLevelId(), sectorId);

			if (!isLoaded) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			const bool open = sectorName.empty()
				? ImGui::TreeNodeEx((void*)(intptr_t)sectorId, sectorFlags, "Sector %d%s", sectorId, suffix)
				: ImGui::TreeNodeEx((void*)(intptr_t)sectorId, sectorFlags, "%s (Sector %d)%s", sectorName.c_str(), sectorId, suffix);
			if (!isLoaded) ImGui::PopStyleColor();

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
				SelectSector(sectorId);
			}

			if (open && hasCheckpoints) {
				for (const auto& ref : cpit->second) {
					const S_CHECKPOINT* pCheckpoint = &ref.pManager->aCheckpoints[ref.index];
					const bool cpSelected = gInspectorSelection.type == InspectorSelectionType::Checkpoint &&
						gInspectorSelection.pCheckpointManager == ref.pManager &&
						gInspectorSelection.checkpointIndex == ref.index;
					ImGuiTreeNodeFlags cpFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | (cpSelected ? ImGuiTreeNodeFlags_Selected : 0);
					std::string cpName = WorldNames::GetCheckpointName(GetCurrentLevelId(), ref.pManager->name, ref.index);
					if (cpName.empty()) {
						ImGui::TreeNodeEx((void*)pCheckpoint, cpFlags, "Checkpoint %d (%s)", ref.index, ref.pManager->name);
					}
					else {
						ImGui::TreeNodeEx((void*)pCheckpoint, cpFlags, "%s (Checkpoint %d, %s)", cpName.c_str(), ref.index, ref.pManager->name);
					}
					if (ImGui::IsItemClicked()) {
						SelectCheckpoint(ref.pManager, ref.index);
					}
				}
				ImGui::TreePop();
			}
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
			ImGui::Text("Current State: %s (%d)", Debug::Actor::State::GetActorStateName(pActor).c_str(), pActor->actorState);
			ImGui::Text("Previous State: %d", pActor->prevActorState);
			ImGui::Text("Current Behaviour: %s (%d)", Debug::Actor::Behaviour::GetActorBehaviourName(pActor).c_str(), pActor->curBehaviourId);
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

		// Show jamgut-specific info if this is a jamgut actor
		if (pActor->typeID == JAMGUT) {
			CActorJamGut* pJamGutActor = static_cast<CActorJamGut*>(pActor);
			ImGui::Separator();
			Debug::Actor::JamGut::ShowJamGutActorDetails(pJamGutActor);
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
		}

		ImGui::Text("Sector %d", selectedSectorId);

		{
			const int levelId = GetCurrentLevelId();
			std::string name = WorldNames::GetSectorName(levelId, selectedSectorId);
			char nameBuf[128];
			strncpy(nameBuf, name.c_str(), sizeof(nameBuf) - 1);
			nameBuf[sizeof(nameBuf) - 1] = '\0';
			if (ImGui::InputText("Name##SectorName", nameBuf, sizeof(nameBuf))) {
				WorldNames::SetSectorName(levelId, selectedSectorId, nameBuf);
			}
		}

		if (pSelectedSector == nullptr) {
			return;
		}

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

		{
			const int levelId = GetCurrentLevelId();
			std::string name = WorldNames::GetCheckpointName(levelId, pManager->name, checkpointIndex);
			char nameBuf[128];
			strncpy(nameBuf, name.c_str(), sizeof(nameBuf) - 1);
			nameBuf[sizeof(nameBuf) - 1] = '\0';
			if (ImGui::InputText("Name##CheckpointName", nameBuf, sizeof(nameBuf))) {
				WorldNames::SetCheckpointName(levelId, pManager->name, checkpointIndex, nameBuf);
			}
		}

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

		if (ImGui::CollapsingHeader("Actions", ImGuiTreeNodeFlags_DefaultOpen)) {
			const bool bCanActivate = CActorHero::_gThis != nullptr && CActorHero::_gThis->CanActivateCheckpoint(checkpoint.flags);

			ImGui::BeginDisabled(!bCanActivate);
			if (ImGui::Button("Activate")) {
				pManager->ActivateCheckpoint(checkpointIndex);
				CActorHeroPrivate* pHeroPrivate = reinterpret_cast<CActorHeroPrivate*>(CActorHero::_gThis);
				edF32VECTOR4 pos = CActorHero::_gThis->currentLocation;
				pos.y += 100;
				pHeroPrivate->UpdatePosition(&pos, false);
				pHeroPrivate->ProcessDeath();
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Force Activate")) {
				pManager->ActivateCheckpoint(checkpointIndex);
				CActorHeroPrivate* pHeroPrivate = reinterpret_cast<CActorHeroPrivate*>(CActorHero::_gThis);
				pHeroPrivate->ProcessDeath();
			}
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
