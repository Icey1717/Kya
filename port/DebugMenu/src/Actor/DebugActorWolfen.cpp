#include "DebugActorWolfen.h"
#include "ActorWolfen.h"
#include <imgui.h>

namespace Debug::Actor::Wolfen
{
	static const char* GetCombatModeString(EEnemyCombatMode mode)
	{
		switch (mode) {
		case ECM_None:     return "None";
		case ECM_Alerted:  return "Alerted";
		case ECM_InCombat: return "In Combat";
		default:           return "Unknown";
		}
	}

	void ShowWolfenActorDetails(CActorWolfen* pWolfenActor)
	{
		if (!pWolfenActor) {
			ImGui::Text("Wolfen Actor: null");
			return;
		}

		if (ImGui::CollapsingHeader("Combat", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Combat Mode: %s", GetCombatModeString(pWolfenActor->combatMode_0xb7c));
			ImGui::Text("Combat Flags: 0x%08X", pWolfenActor->combatFlags_0xb78);

			CLifeInterface* pLife = pWolfenActor->GetLifeInterface();
			if (pLife) {
				int health = pLife->GetValue();
				int maxHealth = pLife->GetValueMax();
				int barLen = 20;
				int filled = (maxHealth > 0) ? (health * barLen) / maxHealth : 0;
				std::string bar = std::string(filled, '#') + std::string(barLen - filled, '-');
				ImGui::Text("Health: [%s] %d / %d", bar.c_str(), health, maxHealth);
			}

			if (ImGui::Button("Kill")) {
				pWolfenActor->LifeAnnihilate();
			}
		}

		if (ImGui::CollapsingHeader("Exorcism", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Exorcised State: %d", pWolfenActor->exorcisedState);
			ImGui::Text("Required Magic: %.2f", pWolfenActor->nbRequiredMagicForExorcism);
			ImGui::Text("Consumed Magic: %.2f", pWolfenActor->nbConsumedMagicForExorcism);
		}

		if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Walk Speed: %.2f", pWolfenActor->walkSpeed);
			ImGui::Text("Walk Acceleration: %.2f", pWolfenActor->walkAcceleration);
			ImGui::Text("Walk Rot Speed: %.2f", pWolfenActor->walkRotSpeed);
			ImGui::Text("Run Speed: %.2f", pWolfenActor->runSpeed);
			ImGui::Text("Run Acceleration: %.2f", pWolfenActor->runAcceleration);
			ImGui::Text("Run Rot Speed: %.2f", pWolfenActor->rotRunSpeed);
			ImGui::Text("Run Speed Scale: %.2f", pWolfenActor->runSpeedScale);
		}
	}
}
