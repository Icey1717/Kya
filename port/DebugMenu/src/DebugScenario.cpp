#include "DebugScenario.h"

#include <array>
#include <imgui.h>
#include "Types.h"

#include "LevelScheduleManager.h"
#include "DebugSetting.h"

#define SCN_GAME_NUM_FREED_WOLFENS		0x0
#define SCN_GAME_COMPANION				0x3
#define SCN_ABILITY_JUMP_BOUNCE			0x7
#define SCN_ABILITY_CLIMB				0x8
#define SCN_ABILITY_BOOMY_TYPE			0x9
#define SCN_ABILITY_FIGHT				0xa
#define SCN_ABILITY_MAGIC_EXORCISM		0xb
#define SCN_ABILITY_MAGIC_REGENERATE	0xc
#define SCN_ABILITY_JAMGUT_RIDE			0xd
#define SCN_ABILITY_BINOCULARS			0xe
#define SCN_ABILITY_RECEPTACLE			0xf
#define SCN_LEVEL_MAGIC_BOARD			0x10
#define SCN_LEVEL_MAGIC_GAUGE			0x11
#define SCN_LEVEL_MAGIC_UPDATE			0x12
#define SCN_LEVEL_MAGIC_MAX				0x13
#define SCN_LEVEL_LIFE_GAUGE			0x14
#define SCN_LEVEL_LIFE_UPDATE			0x15
#define SCN_LEVEL_LIFE_MAX				0x16

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
			"SCN_GAME_6",
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

		Debug::Setting<bool> gApplySavedScenariacVariables("Apply Saved Scenariac Variables", true);
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

void Debug::Scenario::ShowMenu(bool* bOpen)
{
	// Show a window with a grid of items
	if (ImGui::Begin("Debug Scenario", bOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
		gApplySavedScenariacVariables.DrawImguiControl();

		ImGui::Text("Scenario Variables");
		ImGui::Separator();
		for (int i = 0; i < SCN_MAX; ++i) {
			ImGui::PushID(i);

			ImGui::Text("%s: %d", gScenarioVariableNames[i], CLevelScheduler::ScenVar_Get(i));

			ImGui::PopID();
		}

		ImGui::Separator();

		if (ImGui::Button("Load Scenario Variables")) {
			for (int i = 0; i < SCN_MAX; ++i) {
				gScenarioVariables[i] = CLevelScheduler::ScenVar_Get(i);
			}
		}

		ImGui::Separator();

		// Collapsable section for scenario variables
		if (ImGui::CollapsingHeader("Scenario Variables")) {
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
