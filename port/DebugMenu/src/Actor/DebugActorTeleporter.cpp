#include "DebugActorTeleporter.h"
#include "DebugActorBehaviour.h"
#include "ActorTeleporter.h"
#include "Actor.h"
#include "ActorManager.h"
#include "ActorMovingPlatform.h"
#include "ActorSwitch.h"
#include "WayPoint.h"
#include "Native/NativeDebugShapes.h"
#include <imgui.h>

namespace Debug::Actor::Teleporter
{
	static const char* GetStateName(int state)
	{
		switch (state) {
		case TELEPORTER_STATE_DEFAULT_INACTIVE: return "Inactive";
		case TELEPORTER_STATE_DEFAULT_ACTIVE:   return "Active";
		case 7:                                 return "Arrival / Spawn";
		default:                                return "Unknown";
		}
	}

	static bool IsDestinationDisabled(const CActorTeleporter* pTeleporter, int index)
	{
		return (pTeleporter->disabledDestinationMask & (1u << (index & 0x1f))) != 0;
	}

	static void DrawVector4(const char* label, const edF32VECTOR4& value)
	{
		ImGui::Text("%s: %.2f, %.2f, %.2f, %.2f", label, value.x, value.y, value.z, value.w);
	}

	static void ShowMatrix(const char* label, const edF32MATRIX4& matrix)
	{
		if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_None)) {
			DrawVector4("rowX", matrix.rowX);
			DrawVector4("rowY", matrix.rowY);
			DrawVector4("rowZ", matrix.rowZ);
			DrawVector4("rowT", matrix.rowT);
			ImGui::TreePop();
		}
	}

	static void ShowNtfSwitch(const char* label, const S_NTF_SWITCH_ONOFF& ntfSwitch)
	{
		const int targetCount = ntfSwitch.pTargetStreamRef ? ntfSwitch.pTargetStreamRef->entryCount : 0;
		ImGui::Text("%s: targets=%p (%d entries) camera=%p",
			label,
			ntfSwitch.pTargetStreamRef,
			targetCount,
			ntfSwitch.pStreamEventCamera);
	}

	static void ShowDestinations(CActorTeleporter* pTeleporter)
	{
		S_DESTINATION_LIST* pList = pTeleporter->pDestinationList;
		const int nbEntries = pList ? pList->nbEntries : 0;

		ImGui::Text("Destination List: %p (%d entries)", pList, nbEntries);
		ImGui::Text("Active Button:    %d", pTeleporter->activeButtonIndex);
		ImGui::Text("Selected Index:   %d  (field_0x298)", pTeleporter->field_0x298);
		ImGui::Text("Disabled Mask:    0x%08X", pTeleporter->disabledDestinationMask);
		ImGui::Text("Can Teleport:     %s  (field_0x2a4)", pTeleporter->field_0x2a4 ? "yes" : "no");

		if (nbEntries <= 0) {
			return;
		}

		if (ImGui::BeginTable("##TeleporterDestinations", 6, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("#");
			ImGui::TableSetupColumn("Level");
			ImGui::TableSetupColumn("Elevator");
			ImGui::TableSetupColumn("Cutscene");
			ImGui::TableSetupColumn("Disabled");
			ImGui::TableSetupColumn("Active");
			ImGui::TableHeadersRow();

			for (int i = 0; i < nbEntries; ++i) {
				const S_DESTINATION_ENTRY& entry = pList->aEntries[i];
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%d", i);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entry.levelId);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entry.elevatorId);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entry.field_0x8);
				ImGui::TableNextColumn();
				ImGui::Text("%s", IsDestinationDisabled(pTeleporter, i) ? "yes" : "no");
				ImGui::TableNextColumn();
				ImGui::Text("%s", i == pTeleporter->activeButtonIndex ? "yes" : "");
			}

			ImGui::EndTable();
		}
	}

	static void ShowActorWaypoints(CActorTeleporter* pTeleporter)
	{
		ACTOR_WAYPOINT_LIST* pList = pTeleporter->pActorWaypointList;
		const int nbEntries = pList ? pList->nbEntries : 0;

		ImGui::Text("Actor Waypoint List: %p (%d entries)", pList, nbEntries);

		for (int i = 0; i < nbEntries; ++i) {
			ACTOR_WAYPOINT& entry = pList->aEntries[i];
			CActor* pActor = entry.actorRef.Get();
			CWayPoint* pWayPoint = entry.wayPointRef.Get();

			ImGui::PushID(i);
			if (ImGui::TreeNodeEx("Waypoint Entry", ImGuiTreeNodeFlags_None, "[%d]", i)) {
				ImGui::Text("Actor Ref:    idx=%d ptr=%p", entry.actorRef.index, pActor);
				if (pActor) {
					ImGui::Text("  Actor:      %s", pActor->name);
				}
				ImGui::Text("Waypoint Ref: idx=%d ptr=%p", entry.wayPointRef.index, pWayPoint);
				if (pWayPoint) {
					ImGui::Text("  Location:   %.2f, %.2f, %.2f", pWayPoint->location.x, pWayPoint->location.y, pWayPoint->location.z);
					ImGui::Text("  Rotation:   %.2f, %.2f, %.2f", pWayPoint->rotation.x, pWayPoint->rotation.y, pWayPoint->rotation.z);
					Renderer::Native::DebugShapes::AddFilledSphere(
						pWayPoint->location.x, pWayPoint->location.y, pWayPoint->location.z,
						0.2f,
						0.2f, 0.7f, 1.0f, 0.8f);
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}


	static CActor* GetTargetActor(const S_STREAM_NTF_TARGET_BASE& target)
	{
		return LOAD_POINTER_CAST(CActor*, target.pRef);
	}

	static void ShowSelectorControlledActors(CBehaviourSelectorMaster* pSelector, uint mask)
	{
		S_ACTOR_STREAM_REF* pActorStream = pSelector->pActorStreamRef;
		const int actorCount = pActorStream ? pActorStream->entryCount : 0;

		if (ImGui::BeginTable("##SelectorControlledActors", 5, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Bit");
			ImGui::TableSetupColumn("Selection");
			ImGui::TableSetupColumn("Actor");
			ImGui::TableSetupColumn("State");
			ImGui::TableSetupColumn("Masked");
			ImGui::TableHeadersRow();

			const bool selfMasked = (mask & 1u) != 0;
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("0");
			ImGui::TableNextColumn();
			ImGui::Text("-1");
			ImGui::TableNextColumn();
			ImGui::Text("selector platform");
			ImGui::TableNextColumn();
			ImGui::Text("%s", selfMasked ? "inactive" : "eligible");
			ImGui::TableNextColumn();
			ImGui::Text("%s", selfMasked ? "yes" : "no");

			for (int i = 0; i < actorCount; ++i) {
				CActor* pActor = pActorStream->aEntries[i].Get();
				const uint bit = 1u << ((i + 1) & 0x1f);
				const bool masked = (mask & bit) != 0;

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("%d", i + 1);
				ImGui::TableNextColumn();
				ImGui::Text("%d", i);
				ImGui::TableNextColumn();
				ImGui::Text("%s  %p", pActor ? pActor->name : "(null)", pActor);
				ImGui::TableNextColumn();
				ImGui::Text("%s", masked ? "inactive" : "eligible");
				ImGui::TableNextColumn();
				ImGui::Text("%s", masked ? "yes" : "no");
			}

			ImGui::EndTable();
		}
	}

	static bool ShowMovingPlatformSelectorLink(CActorTeleporter* pTeleporter, CActorMovingPlatform* pPlatform)
	{
		CBehaviourSelectorMaster* pSelector = static_cast<CBehaviourSelectorMaster*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER));
		if (!pSelector || !pSelector->switchExList.pTargetStreamRef) {
			return false;
		}

		S_STREAM_NTF_TARGET_SWITCH_EX_LIST* pTargets = pSelector->switchExList.pTargetStreamRef;
		int targetIndex = -1;
		for (int i = 0; i < pTargets->entryCount; ++i) {
			if (GetTargetActor(pTargets->aEntries[i]) == pTeleporter) {
				targetIndex = i;
				break;
			}
		}

		if (targetIndex == -1) {
			return false;
		}

		const uint queryMask = pTeleporter->bOpen ? pTeleporter->disabledDestinationMask : 0xffffffffu;
		const bool allBlocked = queryMask == 0xffffffffu;

		ImGui::PushID(pPlatform);
		if (ImGui::TreeNodeEx("Moving Platform Selector", ImGuiTreeNodeFlags_DefaultOpen, "%s  %p", pPlatform->name, pPlatform)) {
			ImGui::Text("Actor State:        %d", pPlatform->actorState);
			ImGui::Text("Target Entry:       %d / %d", targetIndex, pTargets->entryCount);
			ImGui::Text("Selected Actor:     %d  (actorEntryIndex)", pSelector->actorEntryIndex);
			ImGui::Text("Teleporter Mask:    0x%08X", queryMask);
			ImGui::Text("Teleporter Pick:    %d  (field_0x298)", pTeleporter->field_0x298);
			if (allBlocked) {
				ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.2f, 1.0f), "Selector sees all destinations blocked.");
			}
			ShowSelectorControlledActors(pSelector, queryMask);
			ImGui::TreePop();
		}
		ImGui::PopID();
		return true;
	}

	static bool ShowSwitchLink(CActorTeleporter* pTeleporter, CActorSwitch* pSwitch)
	{
		S_NTF_TARGET_STREAM_REF* pTargets = pSwitch->targetSwitch.pTargetStreamRef;
		if (!pTargets) {
			return false;
		}

		int targetIndex = -1;
		for (int i = 0; i < pTargets->entryCount; ++i) {
			if (GetTargetActor(pTargets->aEntries[i]) == pTeleporter) {
				targetIndex = i;
				break;
			}
		}

		if (targetIndex == -1) {
			return false;
		}

		ImGui::PushID(pSwitch);
		if (ImGui::TreeNodeEx("Nativ Elevator Switch", ImGuiTreeNodeFlags_None, "%s  %p", pSwitch->name, pSwitch)) {
			ImGui::Text("Actor State:      %d", pSwitch->actorState);
			ImGui::Text("Target Entry:     %d / %d", targetIndex, pTargets->entryCount);
			ImGui::Text("Has Level Dests:  %s  (LevelHasTeleporters)", pTeleporter->LevelHasTeleporters() ? "yes" : "no");
			ImGui::TreePop();
		}
		ImGui::PopID();
		return true;
	}

	static void ShowActivationLinks(CActorTeleporter* pTeleporter)
	{
		CActorManager* pActorManager = CScene::ptable.g_ActorManager_004516a4;
		if (!pActorManager) {
			ImGui::TextDisabled("Actor manager unavailable.");
			return;
		}

		int selectorCount = 0;
		int switchCount = 0;

		ImGui::TextDisabled("Moving-platform selectors query this teleporter with message 0x2e and use the returned mask to choose which linked actor is active.");
		for (int i = 0; i < pActorManager->nbActors; ++i) {
			CActor* pActor = pActorManager->aActors[i];
			if (pActor && pActor->typeID == MOVING_PLATFORM) {
				if (ShowMovingPlatformSelectorLink(pTeleporter, static_cast<CActorMovingPlatform*>(pActor))) {
					++selectorCount;
				}
			}
		}

		if (selectorCount == 0) {
			ImGui::TextDisabled("No loaded moving-platform selector targets this teleporter.");
		}

		ImGui::Separator();
		ImGui::TextDisabled("Nativ elevator switches only fire when at least one targeted teleporter reports available level destinations.");
		for (int i = 0; i < pActorManager->nbActors; ++i) {
			CActor* pActor = pActorManager->aActors[i];
			if (pActor && pActor->typeID == SWITCH) {
				if (ShowSwitchLink(pTeleporter, static_cast<CActorSwitch*>(pActor))) {
					++switchCount;
				}
			}
		}

		if (switchCount == 0) {
			ImGui::TextDisabled("No loaded switch targets this teleporter.");
		}
	}

	void ShowActorDetails(CActorTeleporter* pTeleporter)
	{
		if (!pTeleporter) {
			ImGui::Text("Teleporter Actor: null");
			return;
		}

		Renderer::Native::DebugShapes::AddSphere(
			pTeleporter->field_0x270.x, pTeleporter->field_0x270.y, pTeleporter->field_0x270.z,
			0.5f,
			pTeleporter->bOpen ? 0.1f : 0.8f,
			pTeleporter->field_0x2a4 ? 1.0f : 0.2f,
			0.9f,
			0.9f);

		if (ImGui::CollapsingHeader("Teleporter State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Actor State:   %d  (%s)", pTeleporter->actorState, GetStateName(pTeleporter->actorState));
			ImGui::Text("Behaviour:     %d  (%s)", pTeleporter->curBehaviourId, Debug::Actor::Behaviour::GetActorBehaviourName(pTeleporter).c_str());
			ImGui::Text("Open:          %s  (bOpen)", pTeleporter->bOpen ? "yes" : "no");
			ImGui::Text("Init Flags:    0x%08X  (field_0x16c)", pTeleporter->field_0x16c);
			ImGui::Text("Hash:          0x%016llX", pTeleporter->hash);
			ImGui::Text("Subsector Mat: %d", pTeleporter->subsectorMaterialId);
			ImGui::Text("Cinematic ID:  %d", pTeleporter->cinematicId);
			ImGui::Text("Audio A:       %d  (field_0x2ac)", pTeleporter->field_0x2ac);
			ImGui::Text("Audio B:       %d  (field_0x2b0)", pTeleporter->field_0x2b0);
			DrawVector4("Map Position", pTeleporter->field_0x270);
		}

		if (ImGui::CollapsingHeader("Destinations", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowDestinations(pTeleporter);
		}

		if (ImGui::CollapsingHeader("Actor Waypoints", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowActorWaypoints(pTeleporter);
		}

		if (ImGui::CollapsingHeader("Activation Links", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowActivationLinks(pTeleporter);
		}

		if (ImGui::CollapsingHeader("Switch Streams", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowNtfSwitch("Available", pTeleporter->field_0x170);
			ShowNtfSwitch("Arrival", pTeleporter->field_0x178);
			ShowNtfSwitch("Disabled", pTeleporter->field_0x180);
		}

		if (ImGui::CollapsingHeader("Zones And Refs", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Entrance Zone:    idx=%d ptr=%p  (field_0x188)", pTeleporter->field_0x188.index, pTeleporter->field_0x188.Get());
			ImGui::Text("Trigger Zone:     idx=%d ptr=%p  (field_0x18c)", pTeleporter->field_0x18c.index, pTeleporter->field_0x18c.Get());
			ImGui::Text("Snapshot Zone:    idx=%d ptr=%p  (field_0x1d4)", pTeleporter->field_0x1d4.index, pTeleporter->field_0x1d4.Get());
			ImGui::Text("Snapshot Actor:   idx=%d ptr=%p  (field_0x1d8)", pTeleporter->field_0x1d8.index, pTeleporter->field_0x1d8.Get());
			ImGui::Text("Level 0 Zone:     idx=%d ptr=%p  (field_0x1e0)", pTeleporter->field_0x1e0.index, pTeleporter->field_0x1e0.Get());
			ImGui::Text("Snapshot Hash:    %p 0x%08X", pTeleporter->pSnapshotHashCode, pTeleporter->snapshotHashCode.hash.number);
			ShowMatrix("Snapshot Matrix (field_0x1f0)", pTeleporter->field_0x1f0);
			ShowMatrix("Level 0 Matrix (field_0x230)", pTeleporter->field_0x230);
		}

		if (ImGui::CollapsingHeader("Materials And FX", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Particle Info A: %p  (field_0x190)", pTeleporter->field_0x190);
			ImGui::Text("Particle Info B: %p  (field_0x194)", pTeleporter->field_0x194);
			ImGui::Text("Particle Info C: %p  (field_0x1dc)", pTeleporter->field_0x1dc);
			ImGui::Text("G2D Manager:     %p", &pTeleporter->g2dManager);
			ImGui::Text("Materials:       %p (%d)", pTeleporter->aMaterials, pTeleporter->nbMaterials);
		}
	}
}



