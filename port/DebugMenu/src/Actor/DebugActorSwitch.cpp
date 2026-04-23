#include "DebugActorSwitch.h"
#include "DebugActorBehaviour.h"
#include "ActorSwitch.h"
#include <imgui.h>

namespace Debug::Actor::Switch
{
	static const char* GetBehaviourName(int behaviourId)
	{
		switch (behaviourId) {
		case SWITCH_BEHAVIOUR_LEVER:            return "Lever (2)";
		case SWITCH_BEHAVIOUR_MAGIC:            return "Magic (3)";
		case SWITCH_BEHAVIOUR_WOLFEN_COUNTER:   return "WolfenCounter (4)";
		case SWITCH_BEHAVIOUR_MULTI_CONDITION:  return "MultiCondition (5)";
		case SWITCH_BEHAVIOUR_TARGET:           return "Target (6)";
		case SWITCH_BEHAVIOUR_SEQUENCE:         return "Sequence (7)";
		case SWITCH_BEHAVIOUR_NEW:              return "New/Target (8)";
		default:                                return "Unknown";
		}
	}

	static const char* GetSwitchStateName(int state)
	{
		switch (state) {
		case 5: return "Off / Waiting";
		case 6: return "Off → On Transition";
		case 7: return "Lever Push (returning)";
		case 8: return "On / Activated";
		case 9: return "Return Animation";
		default: return "Unknown";
		}
	}

	static void ShowLeverBehaviour(CBehaviourSwitchLever* pLever)
	{
		if (!pLever) { ImGui::Text("(null)"); return; }

		ImGui::Text("Interacting Actor:  %p", pLever->pActor);
		ImGui::Text("Target Rot (0x10):  (%.3f, %.3f, %.3f, %.3f)",
			pLever->field_0x10.x, pLever->field_0x10.y,
			pLever->field_0x10.z, pLever->field_0x10.w);
		ImGui::Text("Osc Quat (0x20):    (%.3f, %.3f, %.3f, %.3f)",
			pLever->field_0x20.quat.x, pLever->field_0x20.quat.y,
			pLever->field_0x20.quat.z, pLever->field_0x20.quat.w);
		ImGui::Text("Sound On  (0x40):   idx=%d", pLever->field_0x40.index);
		ImGui::Text("Sound Done (0x44):  idx=%d", pLever->field_0x44.index);
	}

	static void ShowMagicBehaviour(CBehaviourSwitchMagic* pMagic)
	{
		if (!pMagic) { ImGui::Text("(null)"); return; }

		const float pct = (pMagic->field_0x8 > 0.0f) ? (pMagic->field_0xc / pMagic->field_0x8) * 100.0f : 0.0f;
		ImGui::Text("Magic Required:     %.2f  (field_0x8)", pMagic->field_0x8);
		ImGui::Text("Magic Consumed:     %.2f  (%.1f%%)  (field_0xc)", pMagic->field_0xc, pct);
		ImGui::Text("Drain Rate:         %.4f / tick  (field_0x10)", pMagic->field_0x10);
		ImGui::Text("FX Height Offset:   %.2f  (field_0x14)", pMagic->field_0x14);

		ImGui::Separator();
		ImGui::TextDisabled("FX Handles");
		ImGui::Text("Idle Glow:  %p  id=%-4d  bank=%d  (field_0x1c / 0x18)",
			pMagic->idleGlowFx.pFx, pMagic->idleGlowFx.id, pMagic->field_0x18);
		ImGui::Text("Activated:  %p  id=%-4d  bank=%d  (field_0x34 / 0x30)",
			pMagic->activatedFx.pFx, pMagic->activatedFx.id, pMagic->field_0x30);
		ImGui::Text("Proximity:  %p  id=%-4d  bank=%d  (field_0x50 / 0x4c)",
			pMagic->proximityFx.pFx, pMagic->proximityFx.id, pMagic->field_0x4c);

		ImGui::Separator();
		ImGui::TextDisabled("Scene Nodes");
		ImGui::Text("Off Hierarchy:  %p  (mesh=%d tex=%d)  (0x24/0x28)",
			pMagic->pHierarchy, pMagic->field_0x24, pMagic->field_0x28);
		ImGui::Text("Open Model:     %p  (mesh=%d tex=%d)",
			pMagic->pOpenModelNode, pMagic->openMeshId, pMagic->openTextureId);
	}

	static void ShowWolfenCounterBehaviour(CBehaviourSwitchWolfenCounter* pWC)
	{
		if (!pWC) { ImGui::Text("(null)"); return; }

		ImGui::Text("Required ScenVar:  %d  (field_0x14)", pWC->field_0x14);
		ImGui::Text("Trigger Radius:    %.2f  (field_0x18)", pWC->field_0x18);
		ImGui::Text("Zone Ref:          idx=%d  ptr=%p", pWC->streamRefZone.index, pWC->streamRefZone.Get());
		ImGui::Text("Actor Ref:         idx=%d  ptr=%p", pWC->streamRefActor.index, pWC->streamRefActor.Get());
	}

	static void ShowMultiConditionBehaviour(CBehaviourSwitchMultiCondition* pMC)
	{
		if (!pMC) { ImGui::Text("(null)"); return; }

		const bool bSatisfied = (pMC->field_0x10 == pMC->field_0xc);
		const bool bRetrigger = (pMC->field_0x14 & 1) != 0;
		ImGui::Text("Initial Mask:    0x%08X  (field_0x8)", pMC->field_0x8);
		ImGui::Text("Target Mask:     0x%08X  (field_0xc)", pMC->field_0xc);
		ImGui::Text("Current Mask:    0x%08X  (field_0x10)", pMC->field_0x10);
		ImGui::Text("Flags:           0x%08X  (field_0x14)", pMC->field_0x14);
		ImGui::Text("Retrigger:       %s  (bit 0)", bRetrigger ? "yes" : "no");
		ImGui::Text("Condition Met:   %s", bSatisfied ? "YES" : "no");
	}

	static void ShowTargetBehaviour(CBehaviourSwitchTarget* pTarget)
	{
		if (!pTarget) { ImGui::Text("(null)"); return; }

		ImGui::Text("Target Disp:     %.4", pTarget->targetDisplacement);

		ImGui::Spacing();

		ImGui::Text("Osc Speed:       %.2f", pTarget->oscConfig.springStrength);
		ImGui::Text("Osc Damp:        %.2f", pTarget->oscConfig.damping);

		ImGui::Spacing();

		ImGui::Text("Osc Value:       %.4f", pTarget->oscValue.value);
		ImGui::Text("Osc Velocity:    %.4f", pTarget->oscValue.velocity);
	}

	static void ShowSequenceBehaviour(CBehaviourSwitchSequence* pSeq)
	{
		if (!pSeq) { ImGui::Text("(null)"); return; }

		const int totalSteps = (pSeq->field_0x10 != nullptr) ? *pSeq->field_0x10 : 0;
		const bool bComplete = (pSeq->field_0x14 == totalSteps);
		ImGui::Text("Reset Value:     %u  (field_0x8)", pSeq->field_0x8);
		ImGui::Text("Target Count:    %u  (field_0xc)", pSeq->field_0xc);
		ImGui::Text("Total Steps:     %d  (*field_0x10)", totalSteps);
		ImGui::Text("Current Step:    %d  (field_0x14)", pSeq->field_0x14);
		ImGui::Text("Remaining:       %u  (field_0x18)", pSeq->field_0x18);
		ImGui::Text("Complete:        %s", bComplete ? "YES" : "no");
	}

	void ShowActorDetails(CActorSwitch* pSwitch)
	{
		if (!pSwitch) {
			ImGui::Text("Switch Actor: null");
			return;
		}

		if (ImGui::CollapsingHeader("Switch State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Actor State:  %d  (%s)", pSwitch->actorState, GetSwitchStateName(pSwitch->actorState));
			ImGui::Text("Time In Air:  %.3f", pSwitch->timeInAir);
			ImGui::Text("Behaviour:    %d  (%s)", pSwitch->curBehaviourId, GetBehaviourName(pSwitch->curBehaviourId));
			ImGui::Text("Sound:        %p", pSwitch->pActorSound);
			ImGui::Text("Tied Actor:   %p", pSwitch->pTiedActor);
		}

		if (ImGui::CollapsingHeader("Behaviour Details", ImGuiTreeNodeFlags_DefaultOpen)) {
			switch (pSwitch->curBehaviourId) {
			case SWITCH_BEHAVIOUR_LEVER:
				ShowLeverBehaviour(static_cast<CBehaviourSwitchLever*>(pSwitch->GetBehaviour(SWITCH_BEHAVIOUR_LEVER)));
				break;
			case SWITCH_BEHAVIOUR_MAGIC:
				ShowMagicBehaviour(static_cast<CBehaviourSwitchMagic*>(pSwitch->GetBehaviour(SWITCH_BEHAVIOUR_MAGIC)));
				break;
			case SWITCH_BEHAVIOUR_WOLFEN_COUNTER:
				ShowWolfenCounterBehaviour(static_cast<CBehaviourSwitchWolfenCounter*>(pSwitch->GetBehaviour(SWITCH_BEHAVIOUR_WOLFEN_COUNTER)));
				break;
			case SWITCH_BEHAVIOUR_MULTI_CONDITION:
				ShowMultiConditionBehaviour(&pSwitch->behaviourSwitchMultiCondition);
				break;
			case SWITCH_BEHAVIOUR_TARGET:
			case SWITCH_BEHAVIOUR_NEW:
				ShowTargetBehaviour(static_cast<CBehaviourSwitchTarget*>(pSwitch->GetBehaviour(pSwitch->curBehaviourId)));
				break;
			case SWITCH_BEHAVIOUR_SEQUENCE:
				ShowSequenceBehaviour(&pSwitch->behaviourSwitchSequence);
				break;
			default:
				ImGui::Text("No details for behaviour id=%d", pSwitch->curBehaviourId);
				break;
			}
		}

		if (ImGui::CollapsingHeader("Target Switch", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Camera:        %p", pSwitch->targetSwitch.pStreamEventCamera);
			ImGui::Text("Target Refs:   %p", pSwitch->targetSwitch.pTargetStreamRef);
			if (pSwitch->targetSwitch.pTargetStreamRef) {
				ImGui::Text("  Entry Count: %d", pSwitch->targetSwitch.pTargetStreamRef->entryCount);
			}
		}
	}
}
