#include "DebugHero.h"

#include <imgui.h>
#include <filesystem>

#include "DebugHelpers.h"

#include "ActorHero.h"
#include "ActorHero_Private.h"
#include "InputManager.h"
#include "SectorManager.h"
#include "DebugSetting.h"
#include "ActorManager.h"
#include "Actor/DebugActorBehaviour.h"
#include "LevelScheduleManager.h"

namespace Debug {
	namespace Hero {
		Debug::Setting<std::string> gLastCheckpoint("Last Chekpoint", "");

		constexpr int nbCheckpointMaxActors = 0x600;

		void ShowCheckpointMenu()
		{
			if (ImGui::CollapsingHeader("Checkpoints", ImGuiTreeNodeFlags_DefaultOpen)) {
				CActorHeroPrivate* pActorHero = reinterpret_cast<CActorHeroPrivate*>(CActorHeroPrivate::_gThis);

				struct Checkpoint {
					struct Actor {
						bool bActive;
						char name[64];
						edF32VECTOR4 location;
					} actors[nbCheckpointMaxActors]{};

					int version = 1;
					int sectorId;
				};

				// Find all files with the extension .chkp and display them in a dropdown
				std::vector<std::string> options;

				std::filesystem::path path = "checkpoints";

				auto* pLevelScheduleManager = CScene::ptable.g_LevelScheduleManager_00451660;
		
				// Append the level name.
				path /= pLevelScheduleManager->aLevelInfo[pLevelScheduleManager->currentLevelID].levelName;

				if (std::filesystem::exists(path)) {
					for (const auto& entry : std::filesystem::directory_iterator(path)) {
						if (entry.path().extension() == ".chkp") {
							options.emplace_back(entry.path().filename().string());
						}
					}
				}

				if (options.size() > 0) {
					auto itemGetter = [](void* data, int index, const char** out_text) {
						auto& options = *static_cast<std::vector<std::string>*>(data);
						*out_text = options[index].c_str();
						return true;
						};

					static int selectedOption = [options]() {
						const std::string lastCheckpoint = gLastCheckpoint;

						for (int i = 0; i < options.size(); i++) {
							if (std::string(options[i]) == lastCheckpoint) {
								return i;
							}
						}

						return 0;
						}();

					// Create the dropdown box
					if (ImGui::Combo("Select a Checkpoint", &selectedOption, itemGetter, &options, options.size(), -1))
					{
						gLastCheckpoint = std::string(options[selectedOption]);
					}

					if (ImGui::Button("Load Checkpoint")) {
						std::filesystem::path chkptPath = path / options[selectedOption];
						Checkpoint checkpoint;
						DebugHelpers::LoadTypeFromFile(chkptPath, checkpoint);
						
						auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

						for (int i = 0; i < pActorManager->nbActors; i++) {
							auto* pActor = pActorManager->aActors[i];
							if (checkpoint.actors[i].bActive) {
								assert(strcmp(pActor->name, checkpoint.actors[i].name) == 0);
								pActor->currentLocation = checkpoint.actors[i].location;

								if (pActor->typeID == ATON) {
									CActorMovable* pMovable = dynamic_cast<CActorMovable*>(pActor);
									pMovable->field_0x1c0 = pActor->currentLocation;;
									pActor->CheckpointReset();
								}
							}
						}

						if (CScene::ptable.g_SectorManager_00451670->baseSector.currentSectorID != checkpoint.sectorId) {
							CScene::ptable.g_SectorManager_00451670->SwitchToSector(checkpoint.sectorId, true);
						}
					}
				}

				static Checkpoint sNewCheckpoint;

				if (ImGui::Button("Save Checkpoint")) {
					auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

					assert(pActorManager->nbActors <= nbCheckpointMaxActors);

					for (int i = 0; i < pActorManager->nbActors; i++) {
						auto* pActor = pActorManager->aActors[i];
						sNewCheckpoint.actors[i].bActive = true;
						strcpy(sNewCheckpoint.actors[i].name, pActor->name);
						sNewCheckpoint.actors[i].location = pActor->currentLocation;
					}

					sNewCheckpoint.sectorId = CScene::ptable.g_SectorManager_00451670->baseSector.currentSectorID;
					ImGui::OpenPopup("Save Checkpoint");
				}

				// Create a popup to get the name of the checkpoint
				if (ImGui::BeginPopupModal("Save Checkpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					static char checkpointName[256] = "";
					ImGui::InputText("Checkpoint Name", checkpointName, IM_ARRAYSIZE(checkpointName));

					if (ImGui::Button("Save")) {
						std::filesystem::path checkpointsFolder = "checkpoints";
						if (!std::filesystem::exists(checkpointsFolder)) {
							std::filesystem::create_directory(checkpointsFolder);
						}

						if (!std::filesystem::exists(path)) {
							std::filesystem::create_directory(path);
						}

						std::filesystem::path chkptPath = path / (std::string(checkpointName) + ".chkp");
						DebugHelpers::SaveTypeToFile(chkptPath, sNewCheckpoint);
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			}
		}
	}
}

void Debug::Hero::ShowMenu(bool* bOpen) 
{
	// Create a new ImGui window
	ImGui::Begin("Hero", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	CActorHeroPrivate* pActorHero = reinterpret_cast<CActorHeroPrivate*>(CActorHeroPrivate::_gThis);

	if (pActorHero) {
		ImGui::Text("Behaviour: %s", Debug::Actor::Behaviour::GetActorBehaviourName(pActorHero).c_str());
		ImGui::Text("State: %s", Debug::Actor::State::GetActorStateName(pActorHero).c_str());
		

		if (ImGui::Button("Reset State")) {
			pActorHero->SetState(STATE_HERO_STAND, -1);
		}

		ShowCheckpointMenu();

		ImGui::Spacing();
		ImGui::Spacing();

		DebugHelpers::ImGui::TextVector4("Current Location", pActorHero->currentLocation);
		DebugHelpers::ImGui::TextVector4("Rotation Quat", pActorHero->rotationQuat);
		ImGui::InputFloat("Effort", &pActorHero->effort);

		ImGui::Spacing();
		ImGui::Spacing();

		DebugHelpers::ImGui::TextVector4("Control Direction", pActorHero->controlDirection);
		ImGui::Text("Facing control direction: %d", pActorHero->bFacingControlDirection);

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Actor", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Time in air: %.3f", pActorHero->timeInAir);
		}

		if (ImGui::CollapsingHeader("Dynamic", ImGuiTreeNodeFlags_DefaultOpen)) {
			DebugHelpers::ImGui::TextVector4("Rotation Quat", pActorHero->dynamic.rotationQuat);
			DebugHelpers::ImGui::TextVector4("field_0x10", pActorHero->dynamic.field_0x10);
			DebugHelpers::ImGui::TextVector4("Velocity Direction Euler", pActorHero->dynamic.velocityDirectionEuler);
			DebugHelpers::ImGui::TextVector4("Horizontal Velocity Direction Euler", pActorHero->dynamic.horizontalVelocityDirectionEuler);

			ImGui::Text("Flags: %x", pActorHero->flags);

			ImGui::InputFloat("Speed", &pActorHero->dynamic.speed);
			ImGui::InputFloat("Linear Jerk", &pActorHero->dynamic.horizontalLinearAcceleration);
			ImGui::InputFloat("Linear Acceleration", &pActorHero->dynamic.linearAcceleration);
			ImGui::InputFloat("Weight B", &pActorHero->dynamic.weightB);
			ImGui::InputFloat("Weight A", &pActorHero->dynamic.weightA);
		}

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("DynamicExt", ImGuiTreeNodeFlags_DefaultOpen)) {
			DebugHelpers::ImGui::TextVector4("Gravity", pActorHero->dynamicExt.gForceGravity);
			DebugHelpers::ImGui::TextVector4("Translation", pActorHero->dynamicExt.normalizedTranslation);

			ImGui::InputFloat("Gravity Scale", &pActorHero->dynamicExt.gravityScale);
		}

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Collision", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Flags 0: %x", pActorHero->pCollisionData->flags_0x0);
			ImGui::Text("Flags 4: %x", pActorHero->pCollisionData->flags_0x4);

			if (ImGui::CollapsingHeader("Flags", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("On Plane: %d", pActorHero->pCollisionData->flags_0x4 & 1);
			}
		}

		if (ImGui::CollapsingHeader("Sliding", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("field_0x10f0: %f", pActorHero->field_0x10f0);
			ImGui::Text("field_0x10f4: %f", pActorHero->field_0x10f4);
		}

		if (ImGui::CollapsingHeader("Wind", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("windBoostStrength %f", pActorHero->windBoostStrength);
			ImGui::Text("field_0x11f8 %f", pActorHero->windRotationStrength);
			ImGui::Text("field_0x11fc %f", pActorHero->field_0x11fc);
		}

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen)) {
			DebugHelpers::ImGui::TextVector4("Left Analog Stick", pActorHero->pPlayerInput->lAnalogStick);
		}
	}

	// End the ImGui window
	ImGui::End();
}