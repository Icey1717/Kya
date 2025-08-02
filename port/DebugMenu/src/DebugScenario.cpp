#include "DebugScenario.h"

#include <imgui.h>
#include "Types.h"

#include "LevelScheduleManager.h"

namespace Debug
{
	namespace Scenario
	{
		//Debug::Setting<bool> gAutoBuyBoomy("Auto Buy Boomy", false);
	}
}

void Debug::Scenario::ShowMenu(bool* bOpen)
{
	// Show a window with a grid of items
	if (ImGui::Begin("Debug Scenario", bOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
		
	}

	ImGui::End();
}

void Debug::Scenario::Update()
{

}
