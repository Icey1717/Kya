#include "DebugActorFogManager.h"

#include "ActorFogManager.h"
#include "ActorHero.h"
#include "CameraViewManager.h"
#include "EventManager.h"

#include <imgui.h>

namespace Debug::Actor::FogManager
{
	static void ShowFogDef(const S_STREAM_FOG_DEF& fogDef)
	{
		ImGui::Text("Clip Value 0x0: %.3f", fogDef.clipValue_0x0);
		ImGui::Text("Field 0x4:      %.3f", fogDef.field_0x4);
		ImGui::Text("Fog RGBA:       (%u, %u, %u, %u)",
			fogDef.fogRGBA.r,
			fogDef.fogRGBA.g,
			fogDef.fogRGBA.b,
			fogDef.fogRGBA.a);
		ImGui::Text("Fog Flags:      0x%08X", fogDef.flags);
	}

	static void ShowZoneState(CStreamFogZone& fogZone, int index)
	{
		ed_zone_3d* pZone = fogZone.zoneRef.Get();
		const bool bHeroInside = CActorHero::_gThis != nullptr &&
			pZone != nullptr &&
			CScene::ptable.g_EventManager_006f5080 != nullptr &&
			edEventComputeZoneAgainstVertex(
				CScene::ptable.g_EventManager_006f5080->activeChunkId,
				pZone,
				&CActorHero::_gThis->currentLocation,
				0) != 2;

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("%d", index);
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("0x%08X", fogZone.flags);
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%s", (fogZone.flags & 1) ? "yes" : "no");
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%s", (fogZone.flags & 2) ? "yes" : "no");
		ImGui::TableSetColumnIndex(4);
		ImGui::Text("%s", bHeroInside ? "yes" : "no");
		ImGui::TableSetColumnIndex(5);
		ImGui::Text("%d", fogZone.zoneRef.index);
		ImGui::TableSetColumnIndex(6);
		ImGui::Text("%p", pZone);
	}

	static void ShowFogZones(CActorFogManager* pFogManager)
	{
		CStreamFogZoneArray& zoneArray = pFogManager->behaviourFogManagerZones.streamFogZoneArray;
		CStreamFogZoneInternal* pInternal = zoneArray.pInternal;
		if (pInternal == nullptr) {
			ImGui::TextDisabled("No fog zone stream.");
			return;
		}

		ImGui::Text("Zone Count: %d", pInternal->nbFogZones);
		ImGui::Text("Internal:   %p", pInternal);

		if (ImGui::BeginTable("FogZones", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableSetupColumn("#");
			ImGui::TableSetupColumn("Flags");
			ImGui::TableSetupColumn("Tests Hero");
			ImGui::TableSetupColumn("Pushed");
			ImGui::TableSetupColumn("Hero Inside");
			ImGui::TableSetupColumn("Zone Ref");
			ImGui::TableSetupColumn("Zone");
			ImGui::TableHeadersRow();

			for (int i = 0; i < pInternal->nbFogZones; ++i) {
				ShowZoneState(pInternal->aFogZones[i], i);
			}

			ImGui::EndTable();
		}

		for (int i = 0; i < pInternal->nbFogZones; ++i) {
			CStreamFogZone& fogZone = pInternal->aFogZones[i];
			ImGui::PushID(i);

			if (ImGui::TreeNodeEx("Fog Zone", ImGuiTreeNodeFlags_None, "Fog Zone %d", i)) {
				ed_zone_3d* pZone = fogZone.zoneRef.Get();
				ImGui::Text("Zone Ref Index: %d", fogZone.zoneRef.index);
				ImGui::Text("Zone Ptr:       %p", pZone);
				if (pZone != nullptr) {
					ImGui::Text("Zone Sphere:    %s", pZone->boundSphere.ToString().c_str());
					ImGui::Text("Zone Flags:     0x%08X", pZone->flags);
					ImGui::Text("Zone Matrix:    %p", LOAD_POINTER(pZone->pMatrix));
				}

				ImGui::Separator();
				ImGui::Text("Blend Time / Field 0x18: %.3f", fogZone.field_0x18);
				ShowFogDef(fogZone.fogDef);

				ImGui::TreePop();
			}

			ImGui::PopID();
		}
	}

	static void ShowFogHierarchies(CActorFogManager* pFogManager)
	{
		CFogHierarchiesArray& hierarchyArray = pFogManager->fogHierarchiesArray;
		ImGui::Text("Hierarchy Count: %d", hierarchyArray.nbFogHierarchies);
		ImGui::Text("Hierarchies:     %p", hierarchyArray.aFogHierarchies);

		if (hierarchyArray.aFogHierarchies == nullptr) {
			return;
		}

		if (ImGui::BeginTable("FogHierarchies", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableSetupColumn("#");
			ImGui::TableSetupColumn("Hash");
			ImGui::TableSetupColumn("Node");
			ImGui::TableSetupColumn("Hier");
			ImGui::TableSetupColumn("Flags");
			ImGui::TableSetupColumn("Alpha");
			ImGui::TableSetupColumn("Near");
			ImGui::TableSetupColumn("Far");
			ImGui::TableHeadersRow();

			for (int i = 0; i < hierarchyArray.nbFogHierarchies; ++i) {
				CFogHierarchy& fogHierarchy = hierarchyArray.aFogHierarchies[i];
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08llX", fogHierarchy.hash);
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%p", fogHierarchy.pNode);
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%p", fogHierarchy.pHier);
				ImGui::TableSetColumnIndex(4);
				ImGui::Text("0x%08X", fogHierarchy.flags);
				ImGui::TableSetColumnIndex(5);
				ImGui::Text("%u", fogHierarchy.alpha);
				ImGui::TableSetColumnIndex(6);
				ImGui::Text("%.3f", fogHierarchy.field_0x20);
				ImGui::TableSetColumnIndex(7);
				ImGui::Text("%.3f", fogHierarchy.field_0x24);
			}

			ImGui::EndTable();
		}
	}

	void ShowActorDetails(CActorFogManager* pFogManager)
	{
		if (pFogManager == nullptr) {
			ImGui::Text("Fog Manager Actor: null");
			return;
		}

		if (ImGui::CollapsingHeader("Fog Manager State", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Actor State:      %d", pFogManager->actorState);
			ImGui::Text("Behaviour:        %d", pFogManager->curBehaviourId);
			ImGui::Text("Actor Flags:      0x%08X", pFogManager->flags);
			ImGui::Text("Sector ID:        %d", pFogManager->sectorId);
			ImGui::Text("Behaviour Owner:  %p", pFogManager->behaviourFogManagerZones.pOwner);
		}

		if (ImGui::CollapsingHeader("Fog Zones", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowFogZones(pFogManager);
		}

		if (ImGui::CollapsingHeader("Fog Hierarchies", ImGuiTreeNodeFlags_DefaultOpen)) {
			ShowFogHierarchies(pFogManager);
		}

		if (CScene::_pinstance != nullptr && ImGui::CollapsingHeader("Scene Fog Stack", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Current Stream: %p", CScene::_pinstance->pFogClipStream);
			ImGui::Text("Stack Size:     %d", CScene::_pinstance->fogClipSettingStackSize);
			ImGui::Text("Current Fog RGBA: (%u, %u, %u, %u)",
				CScene::_pinstance->fogRGBA.r,
				CScene::_pinstance->fogRGBA.g,
				CScene::_pinstance->fogRGBA.b,
				CScene::_pinstance->fogRGBA.a);
			ImGui::Text("Current Fog Flags: 0x%08X", CScene::_pinstance->fogFlags);
		}
	}
}