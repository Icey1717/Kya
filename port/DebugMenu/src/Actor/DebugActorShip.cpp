#include "DebugActorShip.h"
#include "DebugActorBehaviour.h"
#include "ActorShip.h"
#include "ActorHero.h"
#include "ActorDCA.h"
#include "Components/DebugVision.h"
#include "Native/NativeDebugShapes.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>

namespace Debug::Actor::Ship
{
	static std::string GetVector4String(const edF32VECTOR4& vec)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2)
		   << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return ss.str();
	}

	static void DrawFlameThrowerShapes(CActorShip* pShip, bool bFilled)
	{
		// Vision cone — represents the ScanForTarget detection volume.
		if (bFilled) {
			Debug::Components::Vision::DrawVisionShapesFilled(&pShip->vision);
		}
		else {
			Debug::Components::Vision::DrawVisionShapesWireframe(&pShip->vision);
		}

		// Mirror CheckForKimTouch: iterate every active FX particle and draw a
		// sphere at its world position.  Colour indicates whether that particle
		// is currently within kimTouchRange of Kya (red = touching, orange = not).
		const CFxArrayManager& mgr = pShip->fxArrayManager;
		const edF32VECTOR4& kimPos = CActorHero::_gThis->currentLocation;
		const float rangeSq = pShip->kimTouchRange * pShip->kimTouchRange;

		int curNb = mgr.nb;
		while (curNb != 0) {
			--curNb;
			const ACTIVE_PATHS_CONTAINER& container = mgr.aActivePathContainers[curNb];
			int iVar9 = container.field_0x4;
			while (iVar9 != 0) {
				--iVar9;
				const CFxHandle& handle = container.aActivePaths[iVar9];
				if (!const_cast<CFxHandle&>(handle).IsValid()) {
					continue;
				}

				const edF32VECTOR4& fxPos = handle.pFx->position;
				const float dx = fxPos.x - kimPos.x;
				const float dy = fxPos.y - kimPos.y;
				const float dz = fxPos.z - kimPos.z;
				const bool bTouching = (dx * dx + dy * dy + dz * dz) < rangeSq;

				if (bFilled) {
					Renderer::Native::DebugShapes::AddFilledSphere(
						fxPos.x, fxPos.y, fxPos.z,
						0.15f,
						bTouching ? 1.0f : 1.0f,
						bTouching ? 0.1f : 0.5f,
						bTouching ? 0.1f : 0.1f,
						0.8f);
				}
				else {
					Renderer::Native::DebugShapes::AddSphere(
						fxPos.x, fxPos.y, fxPos.z,
						0.15f,
						bTouching ? 1.0f : 1.0f,
						bTouching ? 0.1f : 0.5f,
						bTouching ? 0.1f : 0.1f,
						0.8f);
				}
			}
		}
	}

	static void ShowFxArrayManagerInfo(const CFxArrayManager& mgr)
	{
		if (ImGui::TreeNodeEx("FX Array Manager", ImGuiTreeNodeFlags_None)) {
			ImGui::Text("Nb Ints (total paths):     %d", mgr.nbInts);
			ImGui::Text("Nb Active:                 %d", mgr.nb);
			ImGui::Text("Max Simultaneous (0x8):    %d", mgr.field_0x8);
			ImGui::Text("Active Flags:              0x%08x", mgr.flags);
			ImGui::Text("FX Type ID (0x18):         %d", mgr.field_0x18);
			ImGui::Text("Paths Array:               %p", mgr.aInts);
			ImGui::Text("Active Containers:         %p", mgr.aActivePathContainers);

			if (mgr.nb > 0 && mgr.aActivePathContainers) {
				ImGui::Separator();
				for (int i = 0; i < mgr.nb; ++i) {
					const ACTIVE_PATHS_CONTAINER& c = mgr.aActivePathContainers[i];
					ImGui::Text("  [%d] pathIdx=%u  activeFx=%d", i, c.field_0x0, c.field_0x4);
				}
			}

			ImGui::TreePop();
		}
	}

	void ShowShipActorDetails(CActorShip* pShip)
	{
		if (!pShip) {
			ImGui::Text("Ship Actor: null");
			return;
		}

		if (ImGui::Button("Proceed to thrower")) {
			for (int i = 0; i < pShip->nbCareBoys; i++) {
				pShip->DoMessage(pShip, MESSAGE_TRAP_CAUGHT, 0);
			}
		}

		static bool shapesFilled = false;
		ImGui::Checkbox("Flamethrower Shapes Filled", &shapesFilled);
		DrawFlameThrowerShapes(pShip, shapesFilled);

		const float red = pShip->vision.ScanForTarget(CActorHero::_gThis, SCAN_MODE_AMORTISED) ? 1.0f : 0.2f;

		// Draw some debug spheres at the hero's vision points.
		const int numPoints = CActorHero::_gThis->GetNumVisualDetectionPoints();

		for (int i = 0; i < numPoints; ++i) {
			edF32VECTOR4 point;
			CActorHero::_gThis->GetVisualDetectionPoint(&point, i);
			Renderer::Native::DebugShapes::AddFilledSphere(
				point.x, point.y, point.z,
				0.1f,
				red, 0.2f, 0.2f, 0.8f);
		}

		if (ImGui::CollapsingHeader("Ship State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Actor State:      %d  (%s)", pShip->actorState, Debug::Actor::State::GetActorStateName(pShip).c_str());
			ImGui::Text("Anim State (0x30c): %d", pShip->field_0x30c);
			ImGui::Text("Save State (0x308): %d", pShip->field_0x308);
			ImGui::Text("Care Boys Alive:  %d", pShip->nbCareBoys);
			ImGui::Text("Flag 0x304:       %d  (FX spawned this throw)", pShip->field_0x304);
			ImGui::Text("Flag 0x305:       %d  (death pending)", pShip->field_0x305);
		}

		if (ImGui::CollapsingHeader("FlameThrower Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Separator();
			ImGui::TextDisabled("Vision scan (ScanForTarget)");
			ImGui::Text("Kim Touch Range:  %.2f  (CheckForKimTouch radius)", pShip->kimTouchRange);
			ImGui::Text("Scan Damage:      %.2f  (field_0x230, on vision hit)", pShip->field_0x230);
			ImGui::Text("Touch Damage:     %.2f  (field_0x22c, on kim-touch hit)", pShip->field_0x22c);

			ImGui::Separator();
			ImGui::TextDisabled("Timing (compared against timeInAir)");
			ImGui::Text("Jaw Open Time:    %.2f  (field_0x210, vision expand end)", pShip->field_0x210);
			ImGui::Text("Throw Duration:   %.2f  (field_0x214, THROWING → CLOSING)", pShip->field_0x214);
			ImGui::Text("Close Duration:   %.2f  (field_0x218, CLOSING → FLAME_THROWER)", pShip->field_0x218);
			ImGui::Text("Time In Air:      %.2f  (current)", pShip->timeInAir);

			ImGui::Separator();
			ImGui::TextDisabled("Jaw rotation (during FLAME_THROWER)");
			ImGui::Text("Jaw Target Angle: %.4f rad  (field_0x224)", pShip->field_0x224);
			ImGui::Text("Jaw Open Speed:   %.4f  (field_0x25c)", pShip->field_0x25c);
			ImGui::Text("Jaw Close Speed:  %.4f  (field_0x260)", pShip->field_0x260);
		}

		if (ImGui::CollapsingHeader("Vision Cone", ImGuiTreeNodeFlags_DefaultOpen)) {
			Debug::Components::Vision::ShowVisionDetails(&pShip->vision);
			ImGui::Spacing();
			ImGui::Text("Initial Vision Range: %.2f  (field_0x258, reset on Init)", pShip->field_0x258);
		}

		if (ImGui::CollapsingHeader("Gobing State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Gob Target Actor: %p  (field_0x310)", pShip->field_0x310);
			ImGui::Text("Gob Start Pos:    %s  (field_0x320)", GetVector4String(pShip->field_0x320).c_str());
			ImGui::Text("Gob Speed (0x300):%.2f", pShip->field_0x300);
		}

		if (ImGui::CollapsingHeader("Actor References", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Hunter:       %p", pShip->hunterStreamRef.Get());
			ImGui::Text("Air Cannon:   %p", pShip->airCannonStreamRef.Get());
			ImGui::Text("Jaw CareBoy:  %p", pShip->jawCareBoyStreamRef.Get());
			ImGui::Text("Pending Gob:  %p  (field_0x24c)", pShip->field_0x24c);
			ImGui::Text("Gob Score:    %.2f  (field_0x250)", pShip->field_0x250);
			if (pShip->airCannonStreamRef.Get()) {
				CActorDCA* pDCA = static_cast<CActorDCA*>(pShip->airCannonStreamRef.Get());
				int nbShots = pDCA->fireShot.pActorStreamRef ? pDCA->fireShot.pActorStreamRef->entryCount : 0;
				ImGui::Text("Active Shots: %d", nbShots);
			}
		}

		if (ImGui::CollapsingHeader("FX Array Manager", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowFxArrayManagerInfo(pShip->fxArrayManager);
		}
	}
}
