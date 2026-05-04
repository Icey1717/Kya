#include "DebugMenu.h"
#include "DebugScenario.h"

#include <array>
#include <imgui.h>
#include "Types.h"

#include "ActorManager.h"
#include "ActorWolfen.h"
#include "DebugWorldNames.h"
#include "LevelScheduler.h"
#include "DebugSetting.h"

namespace Debug
{
	namespace Scenario
	{
		constexpr std::array<char*, SCN_MAX> gScenarioVariableNames = {
			"SCN_GAME_NUM_FREED_WOLFENS",
			"SCN_GAME_1",
			"SCN_GAME_2",
			"SCN_GAME_COMPANION",
			"SCN_GAME_4",
			"SCN_GAME_5",
			"SCN_GAME_CURRENT_EPISODE",
			"SCN_ABILITY_JUMP_BOUNCE",
			"SCN_ABILITY_CLIMB",
			"SCN_ABILITY_BOOMY_TYPE",
			"SCN_ABILITY_FIGHT",
			"SCN_ABILITY_MAGIC_EXORCISM",
			"SCN_ABILITY_MAGIC_REGENERATE",
			"SCN_ABILITY_JAMGUT_RIDE",
			"SCN_ABILITY_BINOCULARS",
			"SCN_ABILITY_RECEPTACLE",
			"SCN_LEVEL_MAGIC_BOARD",
			"SCN_LEVEL_MAGIC_GAUGE",
			"SCN_LEVEL_MAGIC_UPDATE",
			"SCN_LEVEL_MAGIC_MAX",
			"SCN_LEVEL_LIFE_GAUGE",
			"SCN_LEVEL_LIFE_UPDATE",
			"SCN_LEVEL_LIFE_MAX",
		};

		Debug::Setting<bool> gApplySavedScenariacVariables("Apply Saved Scenariac Variables", false);
		std::array<Debug::Setting<int>, SCN_MAX> gScenarioVariables = {
			Debug::Setting<int>(gScenarioVariableNames[0], 0),
			Debug::Setting<int>(gScenarioVariableNames[1], 0),
			Debug::Setting<int>(gScenarioVariableNames[2], 0),
			Debug::Setting<int>(gScenarioVariableNames[3], 0),
			Debug::Setting<int>(gScenarioVariableNames[4], 0),
			Debug::Setting<int>(gScenarioVariableNames[5], 0),
			Debug::Setting<int>(gScenarioVariableNames[6], 0),
			Debug::Setting<int>(gScenarioVariableNames[7], 0),
			Debug::Setting<int>(gScenarioVariableNames[8], 0),
			Debug::Setting<int>(gScenarioVariableNames[9], 0),
			Debug::Setting<int>(gScenarioVariableNames[10], 0),
			Debug::Setting<int>(gScenarioVariableNames[11], 0),
			Debug::Setting<int>(gScenarioVariableNames[12], 0),
			Debug::Setting<int>(gScenarioVariableNames[13], 0),
			Debug::Setting<int>(gScenarioVariableNames[14], 0),
			Debug::Setting<int>(gScenarioVariableNames[15], 0),
			Debug::Setting<int>(gScenarioVariableNames[16], 0),
			Debug::Setting<int>(gScenarioVariableNames[17], 0),
			Debug::Setting<int>(gScenarioVariableNames[18], 0),
			Debug::Setting<int>(gScenarioVariableNames[19], 0),
			Debug::Setting<int>(gScenarioVariableNames[20], 0),
			Debug::Setting<int>(gScenarioVariableNames[21], 0),
			Debug::Setting<int>(gScenarioVariableNames[22], 0)
		};
	}
}

namespace
{
	constexpr float gWolfenBreakdownHeight = 500.0f;

	struct WolfenStats
	{
		int maxWolfen = 0;
		int freedWolfen = 0;
	};

	struct CurrentLevelWolfenStats
	{
		int freedWolfen = 0;
		std::array<int, 12> freedWolfenPerSubsector = {};
	};

	WolfenStats GetWolfenStats(const S_LEVEL_INFO& levelInfo)
	{
		WolfenStats stats;

		for (int subsectorIndex = 0; subsectorIndex < levelInfo.maxElevatorId; ++subsectorIndex) {
			const S_SUBSECTOR_INFO& subSectorInfo = levelInfo.aSubSectorInfo[subsectorIndex];
			stats.maxWolfen += subSectorInfo.nbMaxExorcisedWolfen;
			stats.freedWolfen += subSectorInfo.nbExorcisedWolfen;
		}

		return stats;
	}

	std::string GetSubsectorDisplayName(int levelId, int subsectorIndex)
	{
		std::string sectorName = Debug::WorldNames::GetSectorName(levelId, subsectorIndex);
		if (sectorName.empty()) {
			return "Subsector " + std::to_string(subsectorIndex);
		}

		return sectorName + " (Subsector " + std::to_string(subsectorIndex) + ")";
	}

	std::string GetLevelDisplayName(int levelId, const S_LEVEL_INFO& levelInfo)
	{
		std::string levelName = Debug::WorldNames::GetLevelName(levelId);
		if (levelName.empty()) {
			return "Level " + std::to_string(levelId) + " (" + levelInfo.levelName + ")";
		}

		return levelName + " (Level " + std::to_string(levelId) + ", " + levelInfo.levelName + ")";
	}

	CurrentLevelWolfenStats CalculateCurrentLevelWolfenStats(const S_LEVEL_INFO& levelInfo)
	{
		CurrentLevelWolfenStats stats;
		const int maxSubsectorCount = (levelInfo.maxElevatorId < 12) ? levelInfo.maxElevatorId : 12;

		CActorManager* pActorManager = CScene::ptable.g_ActorManager_004516a4;
		if (pActorManager == nullptr || maxSubsectorCount <= 0) {
			return stats;
		}

		int currentIndex = 0;
		while (currentIndex < pActorManager->nbActors) {
			CActor* pActor = pActorManager->aActors[currentIndex];
			if (pActor != nullptr) {
				const bool bIsWolfen = pActor->IsKindOfObject(OBJ_TYPE_WOLFEN);
				CActorWolfen* pWolfen = static_cast<CActorWolfen*>(pActor);
				if ((bIsWolfen != false) && (pWolfen->exorcisedState == 2)) {
					int startSectorId = pWolfen->startSectorId;
					if (startSectorId < 0 || maxSubsectorCount <= startSectorId) {
						startSectorId = 0;
					}

					stats.freedWolfenPerSubsector[startSectorId] = stats.freedWolfenPerSubsector[startSectorId] + 1;

					if (0 < pWolfen->startSectorId) {
						stats.freedWolfen = stats.freedWolfen + 1;
					}
				}
			}

			currentIndex = currentIndex + 1;
		}

		return stats;
	}

	void DrawCurrentLevelWolfenCount()
	{
		CLevelScheduler* pLevelScheduler = CLevelScheduler::gThis;
		if (pLevelScheduler == nullptr) {
			ImGui::TextDisabled("Current level wolfen count unavailable.");
			return;
		}

		const int currentLevelId = pLevelScheduler->currentLevelID;
		if (currentLevelId < 0 || 16 <= currentLevelId) {
			ImGui::TextDisabled("Current level wolfen count unavailable.");
			return;
		}

		const S_LEVEL_INFO& levelInfo = pLevelScheduler->aLevelInfo[currentLevelId];
		if (levelInfo.levelName[0] == '\0') {
			ImGui::TextDisabled("Current level info unavailable.");
			return;
		}

		const CurrentLevelWolfenStats currentLevelStats = CalculateCurrentLevelWolfenStats(levelInfo);
		const WolfenStats savedLevelStats = GetWolfenStats(levelInfo);
		const std::string levelDisplayName = GetLevelDisplayName(currentLevelId, levelInfo);

		if (!ImGui::CollapsingHeader("Current Level Live Wolfen Count")) {
			return;
		}

		ImGui::Text("%s", levelDisplayName.c_str());
		ImGui::Text("Freed Actor Count: %d / %d", currentLevelStats.freedWolfen, savedLevelStats.maxWolfen);
		ImGui::Text("Saved Freed LevelInfo Count: %d / %d", savedLevelStats.freedWolfen, savedLevelStats.maxWolfen);

		for (int subsectorIndex = 0; subsectorIndex < levelInfo.maxElevatorId; ++subsectorIndex) {
			const S_SUBSECTOR_INFO& subSectorInfo = levelInfo.aSubSectorInfo[subsectorIndex];
			const std::string subsectorName = GetSubsectorDisplayName(currentLevelId, subsectorIndex);
			ImGui::BulletText(
				"%s: freed %d / save freed %d / max %d",
				subsectorName.c_str(),
				currentLevelStats.freedWolfenPerSubsector[subsectorIndex],
				subSectorInfo.nbExorcisedWolfen,
				subSectorInfo.nbMaxExorcisedWolfen);
		}
	}

	void DrawWolfenBreakdown()
	{
		CLevelScheduler* pLevelScheduler = CLevelScheduler::gThis;
		if (pLevelScheduler == nullptr) {
			ImGui::TextDisabled("Level scheduler not available.");
			return;
		}

		int totalMaxWolfen = 0;
		int totalFreedWolfen = 0;

		for (const S_LEVEL_INFO& levelInfo : pLevelScheduler->aLevelInfo) {
			if (levelInfo.levelName[0] == '\0') {
				continue;
			}

			const WolfenStats levelStats = GetWolfenStats(levelInfo);
			totalMaxWolfen += levelStats.maxWolfen;
			totalFreedWolfen += levelStats.freedWolfen;
		}

		ImGui::Text("ScenVar Freed Wolfen: %d", CLevelScheduler::ScenVar_Get(SCN_GAME_NUM_FREED_WOLFENS));
		ImGui::Text("Loaded LevelInfo Totals: %d / %d", totalFreedWolfen, totalMaxWolfen);
		DrawCurrentLevelWolfenCount();

		if (!ImGui::CollapsingHeader("Level Wolfen Breakdown")) {
			return;
		}

		ImGui::BeginChild("LevelWolfenBreakdownScroll", ImVec2(0.0f, gWolfenBreakdownHeight), true);

		for (int levelId = 0; levelId < 16; ++levelId) {
			const S_LEVEL_INFO& levelInfo = pLevelScheduler->aLevelInfo[levelId];
			if (levelInfo.levelName[0] == '\0') {
				continue;
			}

			const WolfenStats levelStats = GetWolfenStats(levelInfo);
			const std::string levelDisplayName = GetLevelDisplayName(levelId, levelInfo);

			ImGui::PushID(levelId);

			const bool bOpen = ImGui::TreeNodeEx(
				"LevelWolfenBreakdown",
				ImGuiTreeNodeFlags_DefaultOpen,
				"%s: %d / %d",
				levelDisplayName.c_str(),
				levelStats.freedWolfen,
				levelStats.maxWolfen);

			if (bOpen) {
				ImGui::Text("LevelInfo Freed Field: %d", levelInfo.nbExorcisedWolfen);
				ImGui::Text("Subsector Total: %d / %d", levelStats.freedWolfen, levelStats.maxWolfen);

				for (int subsectorIndex = 0; subsectorIndex < levelInfo.maxElevatorId; ++subsectorIndex) {
					const S_SUBSECTOR_INFO& subSectorInfo = levelInfo.aSubSectorInfo[subsectorIndex];
					const std::string subsectorName = GetSubsectorDisplayName(levelId, subsectorIndex);
					ImGui::BulletText(
						"%s: %d / %d",
						subsectorName.c_str(),
						subSectorInfo.nbExorcisedWolfen,
						subSectorInfo.nbMaxExorcisedWolfen);
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::EndChild();
	}
}

void Debug::Scenario::ShowMenu(bool* bOpen)
{
	// Show a window with a grid of items
	if (ImGui::Begin("Debug Scenario", bOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
		gApplySavedScenariacVariables.DrawImguiControl();

		// Money manipulation section
		ImGui::Separator();
		ImGui::Text("Money Cheats");
		ImGui::Separator();
		
		if (ImGui::Button("Give 100 Money")) {
			CLevelScheduler::gThis->Money_TakeFromBank(100);
		}
		ImGui::SameLine();
		if (ImGui::Button("Give 500 Money")) {
			CLevelScheduler::gThis->Money_TakeFromBank(500);
		}
		ImGui::SameLine();
		if (ImGui::Button("Give 1000 Money")) {
			CLevelScheduler::gThis->Money_TakeFromBank(1000);
		}
		
		if (ImGui::Button("Give 5000 Money ??")) {
			CLevelScheduler::gThis->Money_TakeFromBank(5000);
		}
		ImGui::SameLine();
		if (ImGui::Button("Give 10000 Money ??")) {
			CLevelScheduler::gThis->Money_TakeFromBank(10000);
		}
		ImGui::SameLine();
		if (ImGui::Button("MAX MONEY!! ??")) {
			CLevelScheduler::gThis->Money_TakeFromBank(999999);
		}

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Scenario Variables")) {
			for (int i = 0; i < SCN_MAX; ++i) {
				ImGui::PushID(i);

				ImGui::Text("%s: %d", gScenarioVariableNames[i], CLevelScheduler::ScenVar_Get(i));

				ImGui::PopID();
			}
		}

		ImGui::Separator();
		ImGui::Text("Wolfen");
		ImGui::Separator();
		DrawWolfenBreakdown();

		ImGui::Separator();

		if (ImGui::Button("Load Scenario Variables")) {
			for (int i = 0; i < SCN_MAX; ++i) {
				gScenarioVariables[i] = CLevelScheduler::ScenVar_Get(i);
			}
		}

		ImGui::Separator();

		// Collapsable section for scenario variables
		if (ImGui::CollapsingHeader("Edit Scenario Variables")) {
			for (int i = 0; i < SCN_MAX; ++i) {
				ImGui::PushID(i);

				if (gScenarioVariables[i].DrawImguiControl()) {
					CLevelScheduler::ScenVar_Set(i, gScenarioVariables[i]);
				}

				ImGui::PopID();
			}
		}
	}

	ImGui::End();
}

void Debug::Scenario::Update()
{
	if (gApplySavedScenariacVariables) {
		for (int i = 0; i < SCN_MAX; ++i) {
			CLevelScheduler::ScenVar_Set(i, gScenarioVariables[i]);
		}
	}
}

namespace Debug {
    MenuRegisterer sDebugScenarioMenuReg("Scenario", Debug::Scenario::ShowMenu, true);
    UpdateRegisterer sDebugScenarioUpdateReg(Debug::Scenario::Update);
}

