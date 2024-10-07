#include "DebugCheckpoint.h"
#include <imgui.h>
#include <vector>
#include "LargeObject.h"
#include "ActorCheckpointManager.h"
#include "ActorManager.h"
#include "ActorHero.h"
#include "ActorHeroPrivate.h"

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

	for (auto* pCheckpointManager : checkpointManagers) {
		if (ImGui::CollapsingHeader(pCheckpointManager->name)) {
			ImGui::Text("Current checkpoint index: %d", pCheckpointManager->currentCheckpointIndex);
			ImGui::Text("Checkpoint count: %d", pCheckpointManager->checkpointCount);
			ImGui::Text("Checkpoint flags: 0x%x", pCheckpointManager->flags);

			for (int i = 0; i < pCheckpointManager->checkpointCount; i++) {
				S_CHECKPOINT* pCheckpoint = pCheckpointManager->aCheckpoints + i;
				char buff[256];
				std::sprintf(buff, "Checkpoint %d", i);

				if (ImGui::CollapsingHeader(buff)) {
					ImGui::Text("Sector ID: %d", pCheckpoint->sectorId);
					ImGui::Text("Flags: 0x%x", pCheckpoint->flags);
					ImGui::Text("Actor waypoints count: %d", pCheckpoint->actorWaypointsCount);

					for (int j = 0; j < pCheckpoint->actorWaypointsCount; j++) {
						auto& actorWaypoint = pCheckpoint->aActorWaypoints[j];
						ImGui::Text("Actor: %s", actorWaypoint.pActor.Get()->name);
					}

					if (CActorHero::_gThis->CanActivateCheckpoint(pCheckpoint->flags)) {
						if (ImGui::Button("Activate")) {
							pCheckpointManager->ActivateCheckpoint(i);
							CActorHeroPrivate* pHeroPrivate = reinterpret_cast<CActorHeroPrivate*>(CActorHero::_gThis);
							pHeroPrivate->ProcessDeath();
						}
					}
				}
			}
		}
	}

	ImGui::End();
}
