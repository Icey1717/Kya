#include "DebugActorWind.h"
#include "ActorWind.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>

namespace Debug::Actor::Wind
{
	static const char* GetFxWindFlagName(uint flag)
	{
		switch (flag) {
		case FXWIND_FLAG_ACTIVE:
			return "Active";
		case FXWIND_FLAG_SOLID_GRAPHICS_ENABLED:
			return "Solid Graphics Enabled";
		case FXWIND_FLAG_CAMERA_ALIGNED:
			return "Camera Aligned";
		case FXWIND_FLAG_UNUSED_0x08:
			return "Unused 0x08";
		case FXWIND_FLAG_UNUSED_0x10:
			return "Unused 0x10";
		case FXWIND_FLAG_UNUSED_0x20:
			return "Unused 0x20";
		case FXWIND_FLAG_UNUSED_0x40:
			return "Unused 0x40";
		case FXWIND_FLAG_UNUSED_0x80:
			return "Unused 0x80";
		case FXWIND_FLAG_EMITTER_POOL_ACTIVE:
			return "Emitter Pool Active";
		default:
			return "Unknown";
		}
	}

	static std::string GetFxWindFlagsString(uint flags)
	{
		std::stringstream ss;
		bool first = true;
		
		if (flags & FXWIND_FLAG_ACTIVE) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_ACTIVE);
			first = false;
		}
		if (flags & FXWIND_FLAG_SOLID_GRAPHICS_ENABLED) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_SOLID_GRAPHICS_ENABLED);
			first = false;
		}
		if (flags & FXWIND_FLAG_CAMERA_ALIGNED) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_CAMERA_ALIGNED);
			first = false;
		}
		if (flags & FXWIND_FLAG_UNUSED_0x08) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_UNUSED_0x08);
			first = false;
		}
		if (flags & FXWIND_FLAG_UNUSED_0x10) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_UNUSED_0x10);
			first = false;
		}
		if (flags & FXWIND_FLAG_UNUSED_0x20) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_UNUSED_0x20);
			first = false;
		}
		if (flags & FXWIND_FLAG_UNUSED_0x40) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_UNUSED_0x40);
			first = false;
		}
		if (flags & FXWIND_FLAG_UNUSED_0x80) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_UNUSED_0x80);
			first = false;
		}
		if (flags & FXWIND_FLAG_EMITTER_POOL_ACTIVE) {
			if (!first) ss << ", ";
			ss << GetFxWindFlagName(FXWIND_FLAG_EMITTER_POOL_ACTIVE);
			first = false;
		}

		return first ? "None" : ss.str();
	}

	static std::string GetVector4String(const edF32VECTOR4& vec)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2)
		   << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return ss.str();
	}

	static void ShowFxWindSolidInfo(CFxWindSolid* pWindSolid)
	{
		if (!pWindSolid) {
			ImGui::Text("WindSolid: null");
			return;
		}

		if (ImGui::TreeNodeEx("WindSolid", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Owner: %p", pWindSolid->pOwner);
			ImGui::Text("Field 0x4: 0x%x", pWindSolid->field_0x4);
			ImGui::Text("Field 0x8: %.2f", pWindSolid->field_0x8);
			ImGui::Text("Field 0xc: %.2f", pWindSolid->field_0xc);
			ImGui::Text("Field 0x10: %.2f", pWindSolid->field_0x10);
			ImGui::Text("Field 0x14: %.2f", pWindSolid->field_0x14);
			ImGui::Text("Outer Patch ID: 0x%x", pWindSolid->outerPatchId);
			ImGui::Text("Whirl Patch ID: 0x%x", pWindSolid->whirlPatchId);
			ImGui::Text("Fake Plane Patch ID: 0x%x", pWindSolid->fakePlanePatchId);
			ImGui::Text("Alpha Data: %p", pWindSolid->pAlphaData);

			ImGui::Text("Outer State A - Index: %d, State: %d, Alpha: %.2f", 
				pWindSolid->outerStateA.index, 
				pWindSolid->outerStateA.drawState, 
				pWindSolid->outerStateA.alpha);
			ImGui::Text("Outer State B - Index: %d, State: %d, Alpha: %.2f", 
				pWindSolid->outerStateB.index, 
				pWindSolid->outerStateB.drawState, 
				pWindSolid->outerStateB.alpha);
			ImGui::Text("Whirl State A - Index: %d, State: %d, Alpha: %.2f", 
				pWindSolid->whirlStateA.index, 
				pWindSolid->whirlStateA.drawState, 
				pWindSolid->whirlStateA.alpha);
			ImGui::Text("Whirl State B - Index: %d, State: %d, Alpha: %.2f", 
				pWindSolid->whirlStateB.index, 
				pWindSolid->whirlStateB.drawState, 
				pWindSolid->whirlStateB.alpha);
			ImGui::Text("Fake Plane State - Index: %d, State: %d, Alpha: %.2f", 
				pWindSolid->fakePlaneState.index, 
				pWindSolid->fakePlaneState.drawState, 
				pWindSolid->fakePlaneState.alpha);

			ImGui::Text("Color: RGBA(%d, %d, %d, %d)", 
				pWindSolid->field_0x8c.r, 
				pWindSolid->field_0x8c.g, 
				pWindSolid->field_0x8c.b, 
				pWindSolid->field_0x8c.a);

			ImGui::TreePop();
		}
	}

	static void ShowFxWindInfo(CFxWind* pFxWind, int index)
	{
		if (!pFxWind) {
			return;
		}

		std::string label = "FX Wind #" + std::to_string(index);
		if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Address: %p", pFxWind);
			ImGui::Text("Owner: %p", pFxWind->pOwner);
			
			ImGui::Text("Flags (0x%x): %s", pFxWind->flags_0x54, GetFxWindFlagsString(pFxWind->flags_0x54).c_str());
			ImGui::Text("Field 0x58: 0x%x", pFxWind->field_0x58);
			ImGui::Text("Field 0x5c: %p", pFxWind->field_0x5c);

			ImGui::Text("Field 0x160: %.2f", pFxWind->field_0x160);
			ImGui::Text("Field 0x164: 0x%x", pFxWind->field_0x164);
			ImGui::Text("Distance to Camera Scale: %.2f", pFxWind->distanceToCameraScale);
			ImGui::Text("Field 0x16c: %.2f", pFxWind->field_0x16c);
			ImGui::Text("Field 0x170: %.2f", pFxWind->field_0x170);
			ImGui::Text("Base Alpha: %.2f", pFxWind->baseAlpha);
			ImGui::Text("Perpendicular Alpha: %.2f", pFxWind->perpendicularAlpha);
			ImGui::Text("Parallel Alpha: %.2f", pFxWind->parallelAlpha);

			ImGui::Text("Field 0x180: %p", pFxWind->field_0x180);

			ImGui::Text("Field 0x2c4: %.2f", pFxWind->field_0x2c4);
			ImGui::Text("Field 0x2c8: %.2f", pFxWind->field_0x2c8);
			ImGui::Text("Field 0x2cc: %.2f", pFxWind->field_0x2cc);
			ImGui::Text("Field 0x2d0: %.2f", pFxWind->field_0x2d0);
			ImGui::Text("Field 0x2d4: %.2f", pFxWind->field_0x2d4);

			ImGui::Text("Outer Patch ID: 0x%x", pFxWind->outerPatchId);
			ImGui::Text("Whirl Patch ID: 0x%x", pFxWind->whirlPatchId);
			ImGui::Text("Fake Plane Patch ID: 0x%x", pFxWind->fakePlanePatchId);
			ImGui::Text("Sound Actor: %p", pFxWind->field_0x374);

			ImGui::Text("Field 0x390: %.2f", pFxWind->field_0x390);

			ShowFxWindSolidInfo(&pFxWind->windSolid);

			ImGui::TreePop();
		}
	}

	void ShowWindActorDetails(CActorWind* pWindActor)
	{
		if (!pWindActor) {
			ImGui::Text("Wind Actor: null");
			return;
		}

		if (ImGui::TreeNodeEx("Wind Actor Details", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Address: %p", pWindActor);
			ImGui::Text("Name: %s", pWindActor->name);
			ImGui::Text("Type: Wind (0x%x)", WIND);
			ImGui::Text("Location: (%.2f, %.2f, %.2f)", 
				pWindActor->currentLocation.x, 
				pWindActor->currentLocation.y, 
				pWindActor->currentLocation.z);
			ImGui::Text("Sector ID: 0x%x", pWindActor->sectorId);

			ImGui::Separator();
			ImGui::Text("Wind Parameters:");
			ImGui::Text("Max Wind: %.2f", pWindActor->maxWind);
			ImGui::Text("Field 0x168: %.2f", pWindActor->field_0x168);
			ImGui::Text("Field 0x16c: %.2f", pWindActor->field_0x16c);
			ImGui::Text("Field 0x170: %.2f", pWindActor->field_0x170);
			ImGui::Text("Field 0x174: %.2f", pWindActor->field_0x174);
			ImGui::Text("Field 0x178: %.2f", pWindActor->field_0x178);
			ImGui::Text("Field 0x17c: %.2f", pWindActor->field_0x17c);
			ImGui::Text("Field 0x180: %.2f", pWindActor->field_0x180);
			ImGui::Text("Field 0x184: %.2f", pWindActor->field_0x184);
			ImGui::Text("Field 0x188: %.2f", pWindActor->field_0x188);
			ImGui::Text("Field 0x18c: %.2f", pWindActor->field_0x18c);

			ImGui::Separator();
			ImGui::Text("Wind Flags (0x160): 0x%x", pWindActor->field_0x160);
			ImGui::Text("Cur Wind Alpha: %.2f", pWindActor->curWindAlpha);
			ImGui::Text("Material ID: 0x%x", pWindActor->materialId);

			ImGui::Separator();
			ImGui::Text("Number of FX Wind: %u", pWindActor->nbFxWind);
			ImGui::Text("FX Wind Array: %p", pWindActor->aFxWind);

			if (pWindActor->nbFxWind > 0 && pWindActor->aFxWind) {
				if (ImGui::TreeNodeEx("FX Wind Children", ImGuiTreeNodeFlags_DefaultOpen)) {
					for (uint i = 0; i < pWindActor->nbFxWind; ++i) {
						ShowFxWindInfo(&pWindActor->aFxWind[i], i);
					}
					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
	}
}
