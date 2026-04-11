#include "DebugActorWolfen.h"
#include "ActorWolfen.h"
#include "Native/NativeDebugShapes.h"
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

	static bool IsComeBackBehaviour(int behaviourId)
	{
		switch (behaviourId) {
		case WOLFEN_BEHAVIOUR_WATCH_DOG:
		case WOLFEN_BEHAVIOUR_GUARD_AREA:
		case WOLFEN_BEHAVIOUR_SLEEP:
		case WOLFEN_BEHAVIOUR_ESCAPE:
		case WOLFEN_BEHAVIOUR_TRACK:
		case WOLFEN_BEHAVIOUR_TRACK_STAND:
		case WOLFEN_BEHAVIOUR_TRACK_WEAPON:
		case WOLFEN_BEHAVIOUR_TRACK_WEAPON_STAND:
		case WOLFEN_BEHAVIOUR_TRACK_WEAPON_SNIPE:
		case WOLFEN_BEHAVIOUR_SNIPE:
		case WOLFEN_BEHAVIOUR_LOST:
		case WOLFEN_BEHAVIOUR_WOLFEN_DCA:
		case WOLFEN_BEHAVIOUR_AVOID:
			return true;
		default:
			return false;
		}
	}

	static CBehaviourWolfen* GetCurrentWolfenBehaviour(CActorWolfen* pWolfenActor)
	{
		if (!pWolfenActor || !IsComeBackBehaviour(pWolfenActor->curBehaviourId)) {
			return nullptr;
		}

		return static_cast<CBehaviourWolfen*>(pWolfenActor->GetBehaviour(pWolfenActor->curBehaviourId));
	}

	static bool TryGetComeBackPosition(CActorWolfen* pWolfenActor, CBehaviourWolfen* pBehaviour, edF32VECTOR4* pOutComeBackPosition)
	{
		if (!pWolfenActor || !pBehaviour || !pOutComeBackPosition) {
			return false;
		}

		if (pWolfenActor->curBehaviourId == WOLFEN_BEHAVIOUR_GUARD_AREA) {
			CBehaviourGuardArea* pGuardArea = static_cast<CBehaviourGuardArea*>(pBehaviour);
			if (pGuardArea->pathFollowReader.pPathFollow == nullptr) {
				return false;
			}
		}
		else if (pWolfenActor->curBehaviourId == WOLFEN_BEHAVIOUR_WOLFEN_DCA) {
			CBehaviourDCA* pDCA = static_cast<CBehaviourDCA*>(pBehaviour);
			if (pDCA->actorRef.Get() == nullptr) {
				return false;
			}
		}

		edF32VECTOR4* pComeBackPosition = pBehaviour->GetComeBackPosition();
		if (!pComeBackPosition) {
			return false;
		}

		*pOutComeBackPosition = *pComeBackPosition;
		return true;
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
			ImGui::Text("Vision Range:         %.2f", pWolfenActor->vision.visionRange);
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

		if (ImGui::CollapsingHeader("Debug Draw", ImGuiTreeNodeFlags_DefaultOpen)) {
			static bool bDrawComeBackPosition = true;
			static bool bDrawComeBackLink = true;

			ImGui::Checkbox("Draw Come Back Position", &bDrawComeBackPosition);
			ImGui::Checkbox("Draw Come Back Link", &bDrawComeBackLink);

			CBehaviourWolfen* pBehaviour = GetCurrentWolfenBehaviour(pWolfenActor);
			edF32VECTOR4 comeBackPosition;

			if (!TryGetComeBackPosition(pWolfenActor, pBehaviour, &comeBackPosition)) {
				if (pBehaviour == nullptr) {
					ImGui::TextDisabled("Come back position is not available for behaviour %d.", pWolfenActor->curBehaviourId);
				}
				else {
					ImGui::TextDisabled("Come back position is unavailable (missing behaviour data).");
				}
			}
			else {
				ImGui::Text("Come Back Position: (%.2f, %.2f, %.2f)", comeBackPosition.x, comeBackPosition.y, comeBackPosition.z);

				if (bDrawComeBackPosition) {
					Renderer::Native::DebugShapes::AddSphere(
						comeBackPosition.x, comeBackPosition.y, comeBackPosition.z,
						0.3f,
						0.15f, 1.0f, 0.35f, 1.0f);
				}

				if (bDrawComeBackLink) {
					Renderer::Native::DebugShapes::AddLine(
						pWolfenActor->currentLocation.x, pWolfenActor->currentLocation.y, pWolfenActor->currentLocation.z,
						comeBackPosition.x, comeBackPosition.y, comeBackPosition.z,
						0.15f, 1.0f, 0.35f, 0.95f);
				}
			}
		}
	}
}
