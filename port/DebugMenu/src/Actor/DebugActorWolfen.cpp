#include "DebugActorWolfen.h"
#include "ActorWolfen.h"
#include <imgui.h>
#include <string>

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

	static const char* ActorName(CActor* pActor)
	{
		return pActor ? pActor->name : "(null)";
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

		if (ImGui::CollapsingHeader("AI / Targets", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Adversary:         %s (%p)", ActorName(pWolfenActor->pAdversary), pWolfenActor->pAdversary);
			ImGui::Text("Target:            %s (%p)", ActorName(pWolfenActor->pTargetActor_0xc80), pWolfenActor->pTargetActor_0xc80);
			ImGui::Text("Commander:         %s (%p)", ActorName(pWolfenActor->pCommander), pWolfenActor->pCommander);
			ImGui::Text("Tracked Projectile: %p", pWolfenActor->pTrackedProjectile);
			ImGui::Text("Box In Way:        %s (%p)", ActorName(pWolfenActor->pBoxInWay), pWolfenActor->pBoxInWay);
			ImGui::Separator();
			ImGui::Text("Adversary Distance: %.2f", pWolfenActor->adversaryDistance);
			ImGui::Text("Adversary Angle:    %.2f", pWolfenActor->adversaryAngleDiff);
			ImGui::Text("Start Sector:      %d", pWolfenActor->startSectorId);
		}

		if (ImGui::CollapsingHeader("Fight State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Fight Flags:          0x%08X", pWolfenActor->fightFlags);
			ImGui::Text("Stand Anim:           0x%x", pWolfenActor->standAnim);
			ImGui::Text("Active Capability:    %d / 2", pWolfenActor->activeCapabilityIndex);
			ImGui::Separator();
			ImGui::Text("Active Blow:          %p", pWolfenActor->pBlow);
			ImGui::Text("Active Combo:         %p", pWolfenActor->pFighterCombo);
			ImGui::Text("Combo Flags:          0x%08X", pWolfenActor->comboFlags);
			ImGui::Separator();
			ImGui::Text("Hit Multiplier:       %.2f", pWolfenActor->hitMultiplier);
			ImGui::Text("Hit Damage:           %.2f", pWolfenActor->hitDamage);
			ImGui::Text("Hit Flags:            0x%x", pWolfenActor->hitFlags);
			ImGui::Separator();
			ImGui::Text("Num Blows:            %u", pWolfenActor->nbBlows);
			ImGui::Text("Num Combos:           %u", pWolfenActor->nbCombos);
		}

		if (ImGui::CollapsingHeader("Perception", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Vision Range:         %.2f", pWolfenActor->vision.visionRange_0x34);
			ImGui::Text("Vision Flags:         0x%08X", pWolfenActor->vision.flags);
			ImGui::Text("Detected Actor:       %s (%p)", ActorName(pWolfenActor->vision.pActor_0x48), pWolfenActor->vision.pActor_0x48);
			ImGui::Separator();
			ImGui::Text("Vision Prop:          %.2f", pWolfenActor->visionDetectionProps.field_0x0);
			ImGui::Text("Hearing Range:        %.2f", sqrtf(pWolfenActor->hearingDetectionProps.rangeSquared));
			ImGui::Text("Hearing Max Height:   %.2f", pWolfenActor->hearingDetectionProps.maxHeightDifference);
		}

		if (ImGui::CollapsingHeader("Exorcism", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Exorcised State:   %d", pWolfenActor->exorcisedState);
			ImGui::Text("Required Magic:    %.2f", pWolfenActor->nbRequiredMagicForExorcism);
			ImGui::Text("Consumed Magic:    %.2f", pWolfenActor->nbConsumedMagicForExorcism);
		}

		if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Walk Speed:        %.2f", pWolfenActor->walkSpeed);
			ImGui::Text("Walk Acceleration: %.2f", pWolfenActor->walkAcceleration);
			ImGui::Text("Walk Rot Speed:    %.2f", pWolfenActor->walkRotSpeed);
			ImGui::Text("Default Run Speed: %.2f", pWolfenActor->defaultRunSpeed);
			ImGui::Text("Run Speed:         %.2f", pWolfenActor->runSpeed);
			ImGui::Text("Run Acceleration:  %.2f", pWolfenActor->runAcceleration);
			ImGui::Text("Run Rot Speed:     %.2f", pWolfenActor->rotRunSpeed);
			ImGui::Text("Run Speed Scale:   %.2f", pWolfenActor->runSpeedScale);
		}
	}
}
