#include "DebugActorDCA.h"

#include "ActorDCA.h"
#include "ActorProjectile.h"
#include "camera.h"
#include "Native/NativeDebugShapes.h"

#include <imgui.h>

namespace Debug::Actor::DCA
{
	static const char* GetBehaviourName(int behaviourId)
	{
		switch (behaviourId) {
		case DCA_BEHAVIOUR_DEFAULT:       return "Default";
		case DCA_BEHAVIOUR_CONTROLLED:    return "Controlled";
		case DCA_BEHAVIOUR_AI_CONTROLLED: return "AI Controlled";
		case DCA_BEHAVIOUR_TRAJECTORY:    return "Trajectory";
		default:                          return "Unknown";
		}
	}

	static const char* GetStateName(int state)
	{
		switch (state) {
		case 5: return "Idle";
		case 6: return "Recovery";
		case 7: return "Firing";
		default: return "Unknown";
		}
	}

	static const char* GetActorName(CActor* pActor)
	{
		return pActor ? pActor->name : "(null)";
	}

	static void ShowVector3(const char* pLabel, const edF32VECTOR3& value)
	{
		ImGui::Text("%s: (%.3f, %.3f, %.3f)", pLabel, value.x, value.y, value.z);
	}

	static void ShowVector4(const char* pLabel, const edF32VECTOR4& value)
	{
		ImGui::Text("%s: (%.3f, %.3f, %.3f, %.3f)", pLabel, value.x, value.y, value.z, value.w);
	}

	static void ShowControlledParams(const char* pLabel, const CONTROLLED_PARAMS& params)
	{
		if (!ImGui::TreeNodeEx(pLabel, ImGuiTreeNodeFlags_DefaultOpen)) {
			return;
		}

		ImGui::Text("Min Velocity (0x0):  %.3f", params.field_0x0);
		ImGui::Text("Max Velocity (0x4):  %.3f", params.field_0x4);
		ImGui::Text("Charge Rate (0x8):   %.3f", params.field_0x8);
		ImGui::Text("FOV:                 %.3f rad", params.fov);
		ShowVector4("Camera Offset (0x10)", params.field_0x10);
		ShowVector4("LookAt Offset (0x20)", params.field_0x20);

		ImGui::TreePop();
	}

	static void ShowShotPool(CActorDCA* pDCA)
	{
		CFireShot& fireShot = pDCA->fireShot;
		ImGui::Text("Projectile Stream: %p", fireShot.pActorStreamRef);
		ImGui::Text("Selection Mode (0x290): %d", fireShot.field_0x290);
		ImGui::Text("field_0x294: %u", fireShot.field_0x294);
		ImGui::Text("Active Mask (0x298): 0x%08X", fireShot.field_0x298);
		ImGui::Text("field_0x29c: %u", fireShot.field_0x29c);

		if (fireShot.pActorStreamRef == nullptr) {
			ImGui::TextDisabled("No projectile stream.");
			return;
		}

		ImGui::Text("Projectile Slots: %d", fireShot.pActorStreamRef->entryCount);
		if (!ImGui::BeginTable("DCAShotPool", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
			return;
		}

		ImGui::TableSetupColumn("Slot");
		ImGui::TableSetupColumn("Active");
		ImGui::TableSetupColumn("Projectile");
		ImGui::TableSetupColumn("State");
		ImGui::TableSetupColumn("TimeToExplode");
		ImGui::TableSetupColumn("Life");
		ImGui::TableHeadersRow();

		for (int i = 0; i < fireShot.pActorStreamRef->entryCount && i < 20; ++i) {
			CActorProjectile* pProjectile = static_cast<CActorProjectile*>(fireShot.pActorStreamRef->aEntries[i].Get());
			const bool bActive = (fireShot.field_0x298 & (1u << i)) != 0;
			const S_SHOT_DATA& shot = fireShot.aShots[i];

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%d", i);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%s", bActive ? "yes" : "no");
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%p", pProjectile);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%d", pProjectile ? pProjectile->actorState : -1);
			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%.3f", shot.timeToExplode);
			ImGui::TableSetColumnIndex(5);
			ImGui::Text("%.3f", shot.field_0x18);
		}

		ImGui::EndTable();
	}

	void ShowActorDetails(CActorDCA* pDCA)
	{
		if (pDCA == nullptr) {
			ImGui::Text("DCA Actor: null");
			return;
		}

		if (ImGui::CollapsingHeader("DCA State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("State:          %d  (%s)", pDCA->actorState, GetStateName(pDCA->actorState));
			ImGui::Text("Previous State: %d  (%s)", pDCA->prevActorState, GetStateName(pDCA->prevActorState));
			ImGui::Text("Behaviour:      %d  (%s)", pDCA->curBehaviourId, GetBehaviourName(pDCA->curBehaviourId));
			ImGui::Text("Controller:     %s (%p)", GetActorName(pDCA->pControlledByActor), pDCA->pControlledByActor);
			ImGui::Text("Tied Actor:     %s (%p)", GetActorName(pDCA->pTiedActor), pDCA->pTiedActor);
			ImGui::Text("Control Camera Pushed: %s", pDCA->bCameraPushed ? "yes" : "no");
			ImGui::Text("Charging:            %s", pDCA->bCharging ? "yes" : "no");
			ImGui::Text("Move Sound Active:   %s", pDCA->bMoveSoundActive ? "yes" : "no");
			ImGui::Text("Square Released:     %s", pDCA->bSquareReleased ? "yes" : "no");
			ImGui::Text("Pending Fire:        %s  (bIsFiring)", pDCA->bIsFiring ? "yes" : "no");
		}

		if (ImGui::CollapsingHeader("Aiming", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Aim Adjust Speed: %.4f rad/s", pDCA->aimAdjustSpeed);
			ImGui::Text("Charge Start Time (0x460): %.4f", pDCA->field_0x460);
			ImGui::Text("Current Shot Velocity (0x45c): %.4f", pDCA->field_0x45c);
			ImGui::Text("Fallback Velocity (0x4f0): %.4f", pDCA->field_0x4f0);
			ImGui::Text("field_0x464: %.4f", pDCA->field_0x464);
			ShowVector3("Aim Input", pDCA->aimInputDirection);
			ShowVector3("Aim Direction", pDCA->aimDirection);
			ShowVector4("Projected Aim Location", pDCA->projectAimLocation);
			ShowVector4("Projected Aim Direction", pDCA->projectAimDirection);

			static bool bDrawAimRay = false;
			ImGui::Checkbox("Draw Aim Ray", &bDrawAimRay);
			if (bDrawAimRay) {
				const edF32VECTOR4& origin = pDCA->projectAimLocation;
				const edF32VECTOR4& direction = pDCA->projectAimDirection;
				Renderer::Native::DebugShapes::AddLine(
					origin.x, origin.y, origin.z,
					origin.x + direction.x * 10.0f,
					origin.y + direction.y * 10.0f,
					origin.z + direction.z * 10.0f,
					1.0f, 0.5f, 0.1f, 1.0f);
			}
		}

		if (ImGui::CollapsingHeader("Controlled Behaviour", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowControlledParams("Hero Controlled Params", pDCA->behaviourControlled.controlledParams);
			ShowControlledParams("AI Controlled Params", pDCA->behaviourAiControlled.controlledParams);
		}

		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Camera: %p", pDCA->pCamera);
			if (pDCA->pCamera == nullptr) {
				ImGui::TextDisabled("(null)");
			}
			else {
				ImGui::Text("FOV: %.4f rad", pDCA->pCamera->fov);
				ShowVector4("Camera Position (field_0xb0)", pDCA->pCamera->field_0xb0);
				ShowVector4("Camera LookAt", pDCA->pCamera->lookAt);
			}
		}

		if (ImGui::CollapsingHeader("Projectile / FireShot", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowShotPool(pDCA);
		}

		if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Aim Bone (0x450): %u", pDCA->field_0x450);
			ImGui::Text("field_0x454:      %u", pDCA->field_0x454);
			ImGui::Text("Actor Sound (0x4fc): %p", pDCA->field_0x4fc);
			ImGui::Text("Sound 0 (0x500): idx=%d ptr=%p", pDCA->field_0x500.index, pDCA->field_0x500.Get());
			ImGui::Text("Sound 1 (0x504): idx=%d ptr=%p", pDCA->field_0x504.index, pDCA->field_0x504.Get());
			ImGui::Text("Sound 2 (0x508): idx=%d ptr=%p", pDCA->field_0x508.index, pDCA->field_0x508.Get());
			ImGui::Text("Sound 3 (0x50c): idx=%d ptr=%p", pDCA->field_0x50c.index, pDCA->field_0x50c.Get());
		}

		if (ImGui::CollapsingHeader("Trajectory Behaviour")) {
			CBehaviour* pTrajectory = pDCA->GetBehaviour(DCA_BEHAVIOUR_TRAJECTORY);
			ImGui::Text("Behaviour Ptr: %p", pTrajectory);
			ImGui::Text("Loaded: %s", pTrajectory ? "yes" : "no");
			ImGui::TextDisabled("The trajectory behaviour keeps most of its state in private members, so only actor-level data is exposed here.");
		}
	}
}
