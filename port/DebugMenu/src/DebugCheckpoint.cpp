#include "DebugCheckpoint.h"
#include <imgui.h>
#include <vector>
#include "LargeObject.h"
#include "ActorCheckpointManager.h"
#include "ActorManager.h"
#include "ActorHero.h"
#include "ActorHeroPrivate.h"
#include "DebugSetting.h"

namespace Debug
{
	namespace Checkpoint
	{
		Setting<int> gCheckpointManager("CheckpointManager", 0);
		Setting<int> gCheckpointIndex("CheckpointIndex", 0);
	}
}

void Debug::Checkpoint::ShowMenu(bool* bOpen)
{
	ImGui::Begin("Checkpoint", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	std::vector<CActorCheckpointManager*> checkpointManagers;
	checkpointManagers.reserve(10);

	auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

	if (pActorManager) {
		for (int i = 0; i < pActorManager->nbActors; i++) {
			if (pActorManager->aActors[i]->typeID == CHECKPOINT_MANAGER) {
				checkpointManagers.push_back(dynamic_cast<CActorCheckpointManager*>(pActorManager->aActors[i]));
			}
		}
	}

	CActorCheckpointManager* pActiveCheckpointManager = nullptr;

	if (gCheckpointManager >= 0 && gCheckpointManager < checkpointManagers.size()) {
		pActiveCheckpointManager = checkpointManagers[gCheckpointManager];
	}

	if (ImGui::BeginCombo("##Checkpoint manager", pActiveCheckpointManager ? pActiveCheckpointManager->name : "-")) {
		for (int i = 0; i < checkpointManagers.size(); i++) {
			bool isSelected = gCheckpointManager == i;
			if (ImGui::Selectable(checkpointManagers[i]->name, isSelected)) {
				gCheckpointManager = i;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	if (pActiveCheckpointManager) {
		if (ImGui::CollapsingHeader(pActiveCheckpointManager->name, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Current checkpoint index: %d", pActiveCheckpointManager->currentCheckpointIndex);
			ImGui::Text("Checkpoint count: %d", pActiveCheckpointManager->checkpointCount);
			ImGui::Text("Checkpoint flags: 0x%x", pActiveCheckpointManager->flags);

			// Combo box
			if (ImGui::BeginCombo("##Checkpoint index", std::to_string(gCheckpointIndex).c_str())) {
				for (int i = 0; i < pActiveCheckpointManager->checkpointCount; i++) {
					char buff[256];
					std::sprintf(buff, "Checkpoint %d", i);
					bool isSelected = gCheckpointIndex == i;
					if (ImGui::Selectable(buff, isSelected)) {
						gCheckpointIndex = i;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (gCheckpointIndex >= 0 && gCheckpointIndex < pActiveCheckpointManager->checkpointCount)
			{
				auto* pCheckpoint = pActiveCheckpointManager->aCheckpoints + gCheckpointIndex;

				ImGui::Text("Sector ID: %d", pCheckpoint->sectorId);
				ImGui::Text("Flags: 0x%x", pCheckpoint->flags);
				ImGui::Text("Actor waypoints count: %d", pCheckpoint->actorWaypointsCount);

				for (int j = 0; j < pCheckpoint->actorWaypointsCount; j++) {
					auto& actorWaypoint = pCheckpoint->aActorWaypoints[j];
					ImGui::Text("Actor: %s", actorWaypoint.pActor.Get()->name);
				}

				const bool bCanActivate = CActorHero::_gThis->CanActivateCheckpoint(pCheckpoint->flags);

				ImGui::BeginDisabled(!bCanActivate);
				if (ImGui::Button("Activate")) {
					pActiveCheckpointManager->ActivateCheckpoint(gCheckpointIndex);
					CActorHeroPrivate* pHeroPrivate = reinterpret_cast<CActorHeroPrivate*>(CActorHero::_gThis);
					pHeroPrivate->ProcessDeath();
				}
				ImGui::EndDisabled();

				if (ImGui::Button("Force Activate")) {
					pActiveCheckpointManager->ActivateCheckpoint(gCheckpointIndex);
					CActorHeroPrivate* pHeroPrivate = reinterpret_cast<CActorHeroPrivate*>(CActorHero::_gThis);
					pHeroPrivate->ProcessDeath();
				}
			}
		}
	}

	ImGui::End();
}
