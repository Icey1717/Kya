#include "DebugHero.h"

#include <imgui.h>
#include <filesystem>

#include "DebugHelpers.h"

#include "ActorHero.h"
#include "ActorHeroPrivate.h"
#include "InputManager.h"
#include "SectorManager.h"
#include "DebugSetting.h"

namespace Debug {
	namespace Hero {

		char* GetStateName(int state) {
			switch (state) {
			case STATE_HERO_STAND:
				return "StateHeroStand";
				break;
			case STATE_HERO_JUMP_1_1_STAND:
				return "StateHeroJump_1_1_Stand";
				break;
			case STATE_HERO_JUMP_2_3_STAND:
				return "StateHeroJump_2_3_Stand";
				break;
			case STATE_HERO_JUMP_3_3_STAND:
				return "StateHeroJump_3_3_Stand";
				break;
			case STATE_HERO_RUN:
				return "StateHeroRun";
				break;
			case STATE_HERO_SLIDE_SLIP_A:
				return "StateHeroSlideSlipA";
				break;
			case STATE_HERO_SLIDE_SLIP_B:
				return "StateHeroSlideSlipB";
				break;
			case STATE_HERO_SLIDE_SLIP_C:
				return "StateHeroSlideSlipC";
				break;
			case STATE_HERO_SLIDE_A:
				return "StateHeroSlideA";
				break;
			case STATE_HERO_SLIDE_B:
				return "StateHeroSlideB";
				break;
			case STATE_HERO_U_TURN:
				return "StateHeroUTurn";
				break;
			case STATE_HERO_JUMP_1_1_RUN:
				return "StateHeroJump_1_1_Run";
				break;
			case STATE_HERO_JUMP_2_3_RUN:
				return "StateHeroJump_2_3_Run";
				break;
			case STATE_HERO_JUMP_3_3_RUN:
				return "StateHeroJump_3_3_Run";
				break;
			case STATE_HERO_FALL_A:
				return "StateHeroFallA";
				break;
			case STATE_HERO_FALL_B:
				return "StateHeroFallB";
				break;
			case STATE_HERO_ROLL:
				return "StateHeroRoll";
				break;
			case STATE_HERO_GRIP_B:
				return "StateHeroGripB";
				break;
			case STATE_HERO_GRIP_C:
				return "StateHeroGripC";
				break;
			case STATE_HERO_GRIP_HANG_IDLE:
				return "StateHeroGripHangIdle";
				break;
			case STATE_HERO_GRIP_UP:
				return "StateHeroGripUp";
				break;
			case STATE_HERO_JUMP_2_3_GRIP:
				return "StateHeroJump_2_3_Grip";
				break;
			case STATE_HERO_GRIP_GRAB:
				return "StateHeroGripGrab";
				break;
			case STATE_HERO_TOBOGGAN_3:
				return "StateHeroToboggan3";
				break;
			case STATE_HERO_TOBOGGAN_JUMP_1:
				return "StateHeroTobogganJump1";
				break;
			case STATE_HERO_TOBOGGAN_JUMP_2:
				return "StateHeroTobogganJump2";
				break;
			case STATE_HERO_TOBOGGAN_2:
				return "StateHeroToboggan2";
				break;
			case STATE_HERO_TOBOGGAN:
				return "StateHeroToboggan";
				break;
			case STATE_HERO_GLIDE_1:
				return "StateHeroGlide1";
				break;
			case STATE_HERO_GLIDE_2:
				return "StateHeroGlide2";
				break;
			case STATE_HERO_GLIDE_3:
				return "StateHeroGlide3";
				break;
			case STATE_HERO_CAUGHT_TRAP_1:
				return "StateHeroCaughtTrap1";
				break;
			case 0x3:
				return "Cinematic";
				break;
			default:
				assert(false);
				break;
			}
		}

		Debug::Setting<std::string> gLastCheckpoint("Last Chekpoint", "");

		void ShowCheckpointMenu()
		{
			if (ImGui::CollapsingHeader("Checkpoints", ImGuiTreeNodeFlags_DefaultOpen)) {
				CActorHeroPrivate* pActorHero = reinterpret_cast<CActorHeroPrivate*>(CActorHeroPrivate::_gThis);

				struct Checkpoint {
					edF32VECTOR4 location;
					int sectorId;
				};

				// Find all files with the extension .chkp and display them in a dropdown
				std::vector<std::string> options;

				std::filesystem::path path = "checkpoints";

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
						std::string path = "checkpoints/" + options[selectedOption];
						Checkpoint checkpoint;
						DebugHelpers::LoadTypeFromFile(path.c_str(), checkpoint);
						pActorHero->currentLocation = checkpoint.location;

						if (CScene::ptable.g_SectorManager_00451670->baseSector.currentSectorID != checkpoint.sectorId) {
							CScene::ptable.g_SectorManager_00451670->SwitchToSector(checkpoint.sectorId, true);
						}
					}
				}

				static Checkpoint sNewCheckpoint;

				if (ImGui::Button("Save Checkpoint")) {
					sNewCheckpoint.location = pActorHero->currentLocation;
					sNewCheckpoint.sectorId = CScene::ptable.g_SectorManager_00451670->baseSector.currentSectorID;
					ImGui::OpenPopup("Save Checkpoint");
				}

				// Create a popup to get the name of the checkpoint
				if (ImGui::BeginPopupModal("Save Checkpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					static char checkpointName[256] = "";
					ImGui::InputText("Checkpoint Name", checkpointName, IM_ARRAYSIZE(checkpointName));

					if (ImGui::Button("Save")) {
						if (!std::filesystem::exists(path)) {
							std::filesystem::create_directory(path);
						}

						std::string path = "checkpoints/" + std::string(checkpointName) + ".chkp";
						DebugHelpers::SaveTypeToFile(path.c_str(), sNewCheckpoint);
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
		ImGui::Text("State: %s", GetStateName(pActorHero->actorState));

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
			ImGui::InputFloat("Linear Jerk", &pActorHero->dynamic.linearJerk);
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

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen)) {
			DebugHelpers::ImGui::TextVector4("Left Analog Stick", pActorHero->pPlayerInput->lAnalogStick);
		}
	}

	// End the ImGui window
	ImGui::End();
}