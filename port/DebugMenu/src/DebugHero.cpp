#include "DebugHero.h"

#include <imgui.h>
#include <filesystem>

#include "DebugHelpers.h"

#include "ActorHero.h"
#include "ActorHeroPrivate.h"
#include "InputManager.h"
#include "SectorManager.h"
#include "DebugSetting.h"
#include "ActorManager.h"

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
			case STATE_HERO_JOKE:
				return "StateHeroJoke";
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
			case STATE_HERO_STAND_TO_CROUCH_A:
				return "StateHeroStandToCrouchA";
				break;
			case STATE_HERO_STAND_TO_CROUCH_B:
				return "StateHeroStandToCrouchB";
				break;
			case STATE_HERO_CROUCH_A:
				return "StateHeroCrouchA";
				break;
			case STATE_HERO_CROUCH_WALK_A:
				return "StateHeroCrouchWalkA";
				break;
			case STATE_HERO_CROUCH_B:
				return "StateHeroCrouchB";
				break;
			case STATE_HERO_JUMP_TO_CROUCH:
				return "StateHeroJumpToCrouch";
				break;
			case STATE_HERO_ROLL:
				return "StateHeroRoll";
				break;
			case STATE_HERO_CROUCH_C:
				return "StateHeroCrouchC";
				break;
			case STATE_HERO_ROLL_2_CROUCH:
				return "StateHeroRoll2Crouch";
				break;
			case STATE_HERO_TOBOGGAN_JUMP_HURT:
				return "StateHeroTobogganJumpHurt";
				break;
			case STATE_HERO_HURT_A:
				return "StateHeroHurtA";
				break;
			case STATE_HERO_FALL_DEATH:
				return "StateHeroFallDeath";
				break;
			case STATE_HERO_KICK_A:
				return "StateHeroKickA";
				break;
			case STATE_HERO_KICK_B:
				return "StateHeroKickB";
				break;
			case STATE_HERO_KICK_C:
				return "StateHeroKickC";
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
			case STATE_HERO_WIND_CANON:
				return "StateHeroWindCanon";
				break;
			case STATE_HERO_WIND_CANON_B:
				return "StateHeroWindCanonB";
				break;
			case STATE_HERO_WIND_FLY:
				return "StateHeroWindFly";
				break;
			case STATE_HERO_WIND_SLIDE:
				return "StateHeroWindSlide";
				break;
			case STATE_HERO_WIND_WALL_MOVE_A:
				return "StateHeroWindSlideMoveA";
				break;
			case STATE_HERO_WIND_WALL_MOVE_B:
				return "StateHeroWindSlideMoveB";
				break;
			case STATE_HERO_WIND_WALL_MOVE_E:
				return "StateHeroWindSlideMoveE";
				break;
			case STATE_HERO_WIND_WALL_MOVE_F:
				return "StateHeroWindSlideMoveF";
				break;
			case STATE_HERO_WIND_WALL_MOVE_C:
				return "StateHeroWindSlideMoveC";
				break;
			case STATE_HERO_WIND_WALL_MOVE_D:
				return "StateHeroWindSlideMoveD";
				break;
			case STATE_HERO_GRIP_UP_A:
				return "StateHeroGripUpA";
				break;
			case STATE_HERO_GRIP_UP_B:
				return "StateHeroGripUpB";
				break;
			case STATE_HERO_WIND_WALL_MOVE_JUMP:
				return "StateHeroWindWallMoveJump";
				break;
			case STATE_HERO_WIND_FLY_B:
				return "StateHeroWindFlyB";
				break;
			case STATE_HERO_TRAMPOLINE_JUMP_1_2_A:
				return "StateHeroTrampolineJump_1_2_A";
				break;
			case STATE_HERO_TRAMPOLINE_JUMP_2_3:
				return "StateHeroTrampolineJump_2_3";
				break;
			case STATE_HERO_TRAMPOLINE_JUMP_1_2_B:
				return "StateHeroTrampolineJump_1_2_B";
				break;
			case STATE_HERO_TRAMPOLINE_STOMACH_TO_FALL:
				return "StateHeroTrampolineJump_1_2_C";
				break;
			case STATE_HERO_CAUGHT_TRAP_1:
				return "StateHeroCaughtTrap1";
				break;
			case STATE_HERO_CAUGHT_TRAP_2:
				return "StateHeroCaughtTrap2";
				break;
			case 0x3:
				return "Cinematic";
				break;
			default:
				assert(false);
				return "UKNOWN ERROR";
				break;
			}
		}

		Debug::Setting<std::string> gLastCheckpoint("Last Chekpoint", "");

		constexpr int nbCheckpointMaxActors = 0x300;

		void ShowCheckpointMenu()
		{
			if (ImGui::CollapsingHeader("Checkpoints", ImGuiTreeNodeFlags_DefaultOpen)) {
				CActorHeroPrivate* pActorHero = reinterpret_cast<CActorHeroPrivate*>(CActorHeroPrivate::_gThis);

				struct Checkpoint {
					struct Actor {
						bool bActive;
						char name[nbCheckpointMaxActors];
						edF32VECTOR4 location;
					} actors[0x300]{};

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

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen)) {
			DebugHelpers::ImGui::TextVector4("Left Analog Stick", pActorHero->pPlayerInput->lAnalogStick);
		}
	}

	// End the ImGui window
	ImGui::End();
}