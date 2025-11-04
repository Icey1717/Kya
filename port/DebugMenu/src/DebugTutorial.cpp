#include "DebugTutorial.h"
#include "DebugMenu.h"
#include "Actor/DebugActor.h"
#include <vector>
#include <imgui.h>
#include "ActorNativ.h"
#include "ActorWolfen.h"
#include "ActorManager.h"

namespace Debug
{
	MenuRegisterer tutorialMenuRegisterer("Tutorial", Debug::Tutorial::ShowMenu);
}

void Debug::Tutorial::ShowMenu(bool* bOpen)
{
	if (ImGui::Begin("Debug Tutorial", bOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
		// Find the Akasa actor (CBehaviourNativAkasa contains ComboTutorialManager)
		CActorNativ* pAkasaActor = nullptr;

		Debug::Actor::ForEachActor([&](CActor* pActor) {
			if (pAkasaActor == nullptr && pActor->IsKindOfObject(NATIV)) {
				if (pActor->GetBehaviour(NATIVE_BEHAVIOUR_AKASA) != nullptr) {
					pAkasaActor = dynamic_cast<CActorNativ*>(pActor);
				}
			}
			});

		// Get the actor from the actor manager (you may need to adjust this based on your actor manager implementation)
		// This is a placeholder - you'll need to find the correct way to get the Akasa actor
		if (pAkasaActor != nullptr) {
			CBehaviourNativAkasa* pAkasaBehaviour = dynamic_cast<CBehaviourNativAkasa*>(pAkasaActor->GetBehaviour(NATIVE_BEHAVIOUR_AKASA));
			if (pAkasaBehaviour != nullptr) {
				ComboTutorialManager* pComboTutorialMgr = &pAkasaBehaviour->comboTutorialManager;

				ImGui::Text("Total Tutorials: %d", pComboTutorialMgr->nbTutorials);
				ImGui::Separator();

				// Display and allow modification of active tutorial index
				int currentIndex = pComboTutorialMgr->activeTutorialIndex;
				if (ImGui::SliderInt("Active Tutorial Index", &currentIndex, 0, pComboTutorialMgr->nbTutorials - 1)) {
					pComboTutorialMgr->activeTutorialIndex = currentIndex;
				}

				// Display current tutorial information
				if (pComboTutorialMgr->aTutorials != nullptr && currentIndex >= 0 && currentIndex < pComboTutorialMgr->nbTutorials) {
					ArenaTutorial* pCurrentTutorial = &pComboTutorialMgr->aTutorials[currentIndex];
					ImGui::Text("Required Moves: %d", pCurrentTutorial->nbRequiredMoves);
				}

				ImGui::Separator();

				// Button to advance to next tutorial
				if (ImGui::Button("Next Tutorial")) {
					if (currentIndex < pComboTutorialMgr->nbTutorials - 1) {
						pComboTutorialMgr->activeTutorialIndex = currentIndex + 1;
					}
				}
				ImGui::SameLine();

				// Button to go to previous tutorial
				if (ImGui::Button("Previous Tutorial")) {
					if (currentIndex > 0) {
						pComboTutorialMgr->activeTutorialIndex = currentIndex - 1;
					}
				}
			}
		}
		else {
			ImGui::Text("Akasa actor not found");
		}
	}
	ImGui::End();
}