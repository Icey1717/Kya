#include "DebugActorMovingPlatform.h"
#include "DebugActorBehaviour.h"
#include "DebugPathDrawing.h"
#include "ActorMovingPlatform.h"
#include <imgui.h>

namespace Debug::Actor::MovingPlatform
{
	static const char* GetStateName(int state)
	{
		switch (state) {
		case 5:    return "Static / Init";
		case 6:    return "Active / Moving";
		case 7:    return "Transition A";
		case 8:    return "At Endpoint";
		case 9:    return "Waiting";
		case 0xa:  return "Transition B";
		case 0xb:  return "Trajectory → B";
		case 0xc:  return "At Waypoint";
		case 0xd:  return "Trajectory → A";
		case 0xe:  return "Idle / Triggered";
		case 0xf:  return "Slab Off → On";
		case 0x10: return "Slab On / Slave Active";
		case 0x11: return "Destroyed";
		default:   return "Unknown";
		}
	}

	static void ShowTrajectoryBehaviour(CBehaviourPlatformTrajectory* pTraj)
	{
		if (!pTraj) { ImGui::Text("(null)"); return; }

		ImGui::Text("Segment Start Time (0x28): %.4f", pTraj->segmentStartTime);
		ImGui::Text("Loop Delay (0x2c):         %.4f", pTraj->loopDelay);
		ImGui::Text("Target Scaled Time (0x30): %.4f", pTraj->targetScaledTime);
		ImGui::Text("Path Length (0x34):        %.4f", pTraj->pathLength);

		ImGui::Separator();

		ImGui::Text("Traj Path Position:      %.4f", pTraj->trajPos.pathPosition);
		ImGui::Text("Traj Prev Segment:       %d", pTraj->trajPos.prevSegment);
		ImGui::Text("Traj Current Segment:    %d", pTraj->trajPos.currentSegment);
		ImGui::Text("Traj Last Event Segment: %d", pTraj->trajPos.lastEventSegment);

		static bool bDrawPath = false;
		ImGui::Checkbox("Draw Path", &bDrawPath);

		if (bDrawPath) {
			Path::DrawPathFollowReaderAbsolute(pTraj->pathFollowReaderAbs, true);
		}

		ImGui::Separator();
		Path::ShowPathFollowReaderAbsoluteInfo(pTraj->pathFollowReaderAbs);
	}

	static void ShowSlabBehaviour(CBehaviourPlatformSlab* pSlab)
	{
		if (!pSlab) { ImGui::Text("(null)"); return; }

		ImGui::Text("On/Off State:   %p  (pTargetStreamRef)", pSlab->switchOnOff.pTargetStreamRef);
		ImGui::Text("Speed (0x10):   %.4f", pSlab->field_0x10);
		ImGui::Text("Dist A (0x14):  %.4f  (extent A)", pSlab->field_0x14);
		ImGui::Text("Dist B (0x18):  %.4f  (extent B)", pSlab->field_0x18);
		ImGui::Text("Timer (0x1c):   %.4f  (time in slab state)", pSlab->field_0x1c);
		ImGui::Text("Sound:          %p  (field_0x20)", pSlab->field_0x20.Get());
		ImGui::Text("Cur Pos (0x24): %.4f  (current offset)", pSlab->field_0x24);
		ImGui::Text("Tgt Pos (0x28): %.4f  (target offset)", pSlab->field_0x28);
		ImGui::Text("Carried Actor:  %p  (field_0x2c)", pSlab->field_0x2c);
	}

	static void ShowWeighingMasterBehaviour(CBehaviourWeighingMachineMaster* pMaster)
	{
		if (!pMaster) { ImGui::Text("(null)"); return; }

		ImGui::Text("Slave Actor:    %p  (streamActorRef)", pMaster->streamActorRef.Get());
		ImGui::Text("Weight A (0x40): %.4f", pMaster->field_0x40);
		ImGui::Text("Weight B (0x44): %.4f", pMaster->field_0x44);

		ImGui::Spacing();
		ImGui::TextDisabled("Path Follow");
		ImGui::Text("Fill (trajPos): %.4f  (field_0x0)", pMaster->trajPos.pathPosition);
	}

	static void ShowWeighingSlaveBehaviour(CBehaviourWeighingMachineSlave* pSlave)
	{
		if (!pSlave) { ImGui::Text("(null)"); return; }

		ImGui::Text("Offset (0x34):  %.4f  (driven weight offset)", pSlave->field_0x34);
		ImGui::Text("Fill (trajPos): %.4f  (field_0x0)", pSlave->trajPos.pathPosition);
	}

	static void ShowDestroyedBehaviour(CBehaviourPlatformDestroyed* pDest)
	{
		if (!pDest) { ImGui::Text("(null)"); return; }

		ImGui::Text("Alt Model ID:   %d", pDest->altModelId);
		ImGui::Text("field_0x40:     %d", pDest->field_0x40);
		ImGui::Text("Traj Pos:       %.4f  (trajPos.field_0x0)", pDest->trajPos.pathPosition);
		ImGui::Text("FX Offset:      (%.2f, %.2f, %.2f, %.2f)  (field_0x50)",
			pDest->field_0x50.x, pDest->field_0x50.y,
			pDest->field_0x50.z, pDest->field_0x50.w);
	}

	static void ShowStandBehaviour(CBehaviourPlatformStand* pStand)
	{
		if (!pStand) { ImGui::Text("(null)"); return; }

		ImGui::Text("field_0x8:  %d  (cinematic/audio ref)", pStand->field_0x8);
		ImGui::Text("field_0xc:  %d", pStand->field_0xc);
		ImGui::Text("Cin FX:     %p  id=%d  (pCinData)", pStand->pCinData.pFx, pStand->pCinData.id);
	}

	static void ShowSelectorMasterBehaviour(CBehaviourSelectorMaster* pMaster)
	{
		if (!pMaster) { ImGui::Text("(null)"); return; }

		ImGui::Text("Active Entry:   %d  (actorEntryIndex)", pMaster->actorEntryIndex);
		ImGui::Text("Actor Stream:   %p", pMaster->pActorStreamRef);
		if (pMaster->pActorStreamRef) {
			ImGui::Text("  Entry Count: %d", pMaster->pActorStreamRef->entryCount);
		}
		ImGui::Text("Speed (0x8):    %.4f", pMaster->field_0x8);
		ImGui::Text("field_0xc:      %.4f", pMaster->field_0xc);
		ImGui::Text("field_0x10:     %.4f", pMaster->field_0x10);
		ImGui::Text("Flags (0x18):   %d", pMaster->field_0x18);
	}

	static void ShowSelectorNewBehaviour(CBehaviourSelectorNew* pNew)
	{
		if (!pNew) { ImGui::Text("(null)"); return; }

		ImGui::Text("Speed (0x8):   %.4f", pNew->field_0x8);
		ImGui::Text("field_0xc:     %.4f", pNew->field_0xc);
		ImGui::Text("field_0x10:    %.4f", pNew->field_0x10);
		ImGui::Text("Flags (0x18):  %d", pNew->field_0x18);
	}

	static void ShowTeleportRandomBehaviour(CBehaviourTeleportRandom* pTele)
	{
		if (!pTele) { ImGui::Text("(null)"); return; }

		ImGui::Text("Active Point:  %d  (activeSplinePoint)", pTele->activeSplinePoint);
		ImGui::Text("Path Follow:   %p  (pPathFollow)", pTele->pPathFollow);
		if (pTele->pPathFollow) {
			ImGui::Text("  Spline Pts:  %d", pTele->pPathFollow->splinePointCount);
		}
	}

	static void ShowBehaviourDetails(CActorMovingPlatform* pPlatform, int behaviourId)
	{
		switch (behaviourId) {
		case MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY:
			ShowTrajectoryBehaviour(static_cast<CBehaviourPlatformTrajectory*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_SLAB:
			ShowSlabBehaviour(static_cast<CBehaviourPlatformSlab*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SLAB)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER:
			ShowWeighingMasterBehaviour(static_cast<CBehaviourWeighingMachineMaster*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_MASTER)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE:
			ShowWeighingSlaveBehaviour(static_cast<CBehaviourWeighingMachineSlave*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_WEIGHING_MACHINE_SLAVE)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_DESTROYED:
			ShowDestroyedBehaviour(static_cast<CBehaviourPlatformDestroyed*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_DESTROYED)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_STAND:
			ShowStandBehaviour(static_cast<CBehaviourPlatformStand*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_STAND)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER:
			ShowSelectorMasterBehaviour(static_cast<CBehaviourSelectorMaster*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SELECTOR_MASTER)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW:
			ShowSelectorNewBehaviour(static_cast<CBehaviourSelectorNew*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_SELECTOR_NEW)));
			break;
		case MOVING_PLATFORM_BEHAVIOUR_TELEPORT_RANDOM:
			ShowTeleportRandomBehaviour(static_cast<CBehaviourTeleportRandom*>(pPlatform->GetBehaviour(MOVING_PLATFORM_BEHAVIOUR_TELEPORT_RANDOM)));
			break;
		default:
			ImGui::Text("No details for behaviour id=%d", pPlatform->curBehaviourId);
			break;
		}
	}

	void ShowActorDetails(CActorMovingPlatform* pPlatform)
	{
		if (!pPlatform) {
			ImGui::Text("Moving Platform Actor: null");
			return;
		}

		if (ImGui::CollapsingHeader("Platform State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Actor State:      %d  (%s)", pPlatform->actorState, GetStateName(pPlatform->actorState));
			ImGui::Text("Prev State:       %d  (%s)", pPlatform->prevActorState, GetStateName(pPlatform->prevActorState));
			ImGui::Text("Behaviour:        %d  (%s)", pPlatform->curBehaviourId, Debug::Actor::Behaviour::GetActorBehaviourName(pPlatform).c_str());
			ImGui::Text("Platform Flags:   0x%08X", pPlatform->movingPlatformFlags);
			ImGui::Text("  Has Audio:      %s  (bit 0)", (pPlatform->movingPlatformFlags & 0x1) ? "yes" : "no");
			ImGui::Text("  Has Slab Sound: %s  (bit 1)", (pPlatform->movingPlatformFlags & 0x2) ? "yes" : "no");
			ImGui::Text("  Cam Enter/Exit: %s  (bit 2)", (pPlatform->movingPlatformFlags & 0x4) ? "yes" : "no");
			ImGui::Text("  Cam Transition: %s  (bit 3)", (pPlatform->movingPlatformFlags & 0x8) ? "yes" : "no");
			ImGui::Text("  Has FX (0x10):  %s  (bit 4)", (pPlatform->movingPlatformFlags & 0x10) ? "yes" : "no");
		}

		if (ImGui::CollapsingHeader("Behaviour Details", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowBehaviourDetails(pPlatform, pPlatform->curBehaviourId);
		}

		if (ImGui::CollapsingHeader("Other Behaviours")) {
			for (int i = MOVING_PLATFORM_BEHAVIOUR_TRAJECTORY; i < MOVING_PLATFORM_BEHAVIOUR_COUNT; ++i) {
				if (pPlatform->GetBehaviour(i) && i != pPlatform->curBehaviourId) {
					ImGui::Separator();
					ShowBehaviourDetails(pPlatform, i);
				}
			}
		}

		if (ImGui::CollapsingHeader("Streams", ImGuiTreeNodeFlags_DefaultOpen)) {
			const int nbActors  = pPlatform->pActorStream  ? pPlatform->pActorStream->entryCount  : 0;
			const int nbCameras = pPlatform->pCameraStream ? pPlatform->pCameraStream->entryCount : 0;
			const int nbZones   = pPlatform->pZoneStream   ? pPlatform->pZoneStream->entryCount : 0;

			ImGui::Text("Actor Stream:   %p  (%d entries)", pPlatform->pActorStream, nbActors);
			for (int i = 0; i < nbActors; ++i) {
				S_BRIDGE_ACTOR_STREAM_ENTRY& e = pPlatform->pActorStream->aEntries[i];
				ImGui::Text("  [%d] actor=%p  carryMethod=%d", i, e.actorRef.Get(), e.carryMethod);
			}

			ImGui::Text("Camera Stream:  %p  (%d entries)", pPlatform->pCameraStream, nbCameras);
			for (int i = 0; i < nbCameras; ++i) {
				const S_BRIDGE_CAMERA_STREAM_ENTRY& e = pPlatform->pCameraStream->aEntries[i];
				ImGui::Text("  [%d] type=0x%x  seqId=%d", i, e.field_0x0, e.field_0x44);
			}

			ImGui::Text("Zone Stream:     %p  (%d zones)", pPlatform->pZoneStream, nbZones);
			ImGui::Text("No-Friction Zones: %d", pPlatform->noFrictionZoneCount);
		}

		if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (!pPlatform->pProperties) {
				ImGui::Text("(null)");
			}
			else {
				const CActorMovingPlatform_SubObj* p = pPlatform->pProperties;
				ImGui::Text("Audio A (0x0):  id0=%d  id1=%d", p->field_0x0.field_0x0, p->field_0x0.field_0x4);
				ImGui::Text("Audio B (0x8):  id0=%d  id1=%d", p->field_0x8.field_0x0, p->field_0x8.field_0x4);
				ImGui::Text("Flags (0x24):   0x%08X", p->flags_0x24);
				ImGui::Text("  Slip Surface: %s  (bit 6 / 0x40)", (p->flags_0x24 & 0x40) ? "yes" : "no");
				ImGui::Text("  Ice Surface:  %s  (bit 22 / 0x400000)", (p->flags_0x24 & 0x400000) ? "yes" : "no");
				ImGui::Text("  No Shadow:    %s  (bit 14 / 0x4000)", (p->flags_0x24 & 0x4000) ? "yes" : "no");
				ImGui::Text("field_0x28:     %d  (camera scenar idx)", p->field_0x28);
				ImGui::Text("field_0x2c:     %.4f  (tilt scale)", p->field_0x2c);
			}
		}

		if (ImGui::CollapsingHeader("FX Handles", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("FX Handle A (0x1ec): %p  id=%d", pPlatform->field_0x1ec.pFx, pPlatform->field_0x1ec.id);
			ImGui::Text("FX Handle B (0x1f4): %p  id=%d", pPlatform->field_0x1f4.pFx, pPlatform->field_0x1f4.id);
		}
	}
}
