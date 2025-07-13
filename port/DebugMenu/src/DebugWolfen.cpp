#include "DebugWolfen.h"

#include "imgui.h"

#include "Actor/DebugActor.h"
#include "ActorWolfen.h"

void Debug::Wolfen::ShowMenu(bool* bOpen)
{
	if (ImGui::Begin("Debug Wofen", bOpen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
		std::vector<CActorWolfen*> aWolfen;
		aWolfen.reserve(64);

		Debug::Actor::ForEachActiveActor([&aWolfen](CActor* pActor) {
			if (pActor->typeID == WOLFEN) {
				aWolfen.push_back(static_cast<CActorWolfen*>(pActor));
			}
			});

		for (CActorWolfen* pWolfen : aWolfen) {
			if (ImGui::CollapsingHeader(pWolfen->name)) {
				ImGui::Text("Position: %s", pWolfen->currentLocation.ToString().c_str());
				ImGui::Text("Rotation: %s", pWolfen->rotationEuler.ToString().c_str());
				ImGui::Text("Health: %d", pWolfen->lifeInterface.GetValue());

				// Show an ascii art representation of the health bar.

				{
					CLifeInterface* pLifeInterface = pWolfen->GetLifeInterface();

					int health = pLifeInterface->GetValue();
					int maxHealth = pLifeInterface->GetValueMax();

					int healthBarLength = 20; // Length of the health bar in characters
					int filledLength = (health * healthBarLength) / maxHealth; // Calculate filled length based on health percentage
					std::string healthBar = std::string(filledLength, '#') + std::string(healthBarLength - filledLength, '-');
					ImGui::Text("Health Bar: %s (%d/%d)", healthBar.c_str(), health, maxHealth);
				}

				std::string label = "Kill###btn" + std::string(pWolfen->name);

				if (ImGui::Button(label.c_str())) {
					pWolfen->LifeAnnihilate();
				}
			}
		}
	}

	ImGui::End();
}
