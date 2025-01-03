#include "DebugActor.h"

#include <imgui.h>

#include "DebugProjection.h"

#include "DebugSetting.h"
#include "ActorManager.h"
#include "Actor.h"
#include "LargeObject.h"
#include "DebugFrameBuffer.h"
#include "DebugActorBehaviour.h"
#include "ActorHero.h"
#include "MathOps.h"

namespace Debug {
	namespace Actor {
		static Setting<bool> gShowActorNamesOverlay("Show Actor Names Overlay", true);
		static Setting<bool> gOnlyActiveActors("Show Only Active", true);
		static Setting<float> gActorInfoDistance("Actor Info Distance", 500.0f);

		// Helper functions for ImVec2 operations
		static ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
			return ImVec2(a.x + b.x, a.y + b.y);
		}

		// Get the string version of the actor type
		const char* GetActorTypeString(int type) {
			switch (type) {
			case CINEMATIC: return "Cinematic";
			case BOOMY: return "Boomy";
			case ACTOR_HERO_PRIVATE: return "Hero";
			case MICKEN: return "Micken";
			case WOLFEN: return "Wolfen";
			case NATIV: return "Nativ";
			case MOVING_PLATFORM: return "Moving Platform";
			case BONUS: return "Bonus";
			case ROPE: return "Rope";
			case GRAVITY_AWARE: return "Gravity Aware";
			case SWITCH: return "Switch";
			case WIND: return "Wind";
			case COMMANDER: return "Commander";
			case AMBER: return "Amber";
			case FRUIT: return "Fruit";
			case BRIDGE: return "Bridge";
			case CLUSTERISER: return "Clusteriser";
			case COMPANION: return "Companion";
			case DCA: return "DCA";
			case PROJECTILE: return "Projectile";
			case CHECKPOINT_MANAGER: return "Checkpoint Manager";
			case WEAPON: return "Weapon";
			case ARAIGNOS: return "Araignos";
			case TELEPORTER: return "Teleporter";
			case AMORTOS: return "Amortos";
			case WOOF: return "Woof";
			case TRAP: return "Trap";
			case FX: return "FX";
			case MONEY: return "Money";
			case BOX: return "Box";
			case ATON: return "Aton";
			case BASIC_BOX: return "Basic Box";
			case EVENT_GENERATOR: return "Event Generator";
			case NATIV_CMD: return "Nativ Cmd";
			case FOG_MANAGER: return "Fog Manager";
			case SHOOT: return "Shoot";
			case SHOCKER: return "Shocker";
			case BLAZER: return "Blazer";
			case BUNCH: return "Bunch";
			case BRAZUL: return "Brazul";
			case SHOCKER_CMD: return "Shocker Cmd";
			case RUNE: return "Rune";
			case BONUS_FOUNTAIN: return "Bonus Fountain";
			case PUNCHING_BALL: return "Punching Ball";
			case NATIV_SHOP: return "Nativ Shop";
			case ADD_ON_PURCHASE: return "Add On Purchase";
			case ACTOR: return "Actor";
			case ELECTROLLA: return "Electrolla";
			case LIANA: return "Liana";
			case EXPLOSIVE_DISTRIBUTOR: return "Explosive Distributor";
			case HEDGEHOG: return "Hedgehog";
			case DRAGO_TERMITES: return "Drago Termites";
			case NOSE_MONSTER: return "Nose Monster";
			case WOOD_MONSTER: return "Wood Monster";
			case HELPER_SIGN: return "Helper Sign";
			case EGG: return "Egg";
			case AMORTOS_CMD: return "Amortos Cmd";
			case JAMGUT: return "Jamgut";
			case SHIP: return "Ship";
			case CARE_BOY: return "Care Boy";
			case ACCELERATOS: return "Acceleratos";
			case PATTERN_CON: return "Pattern Con";
			case PATTERN_CMD: return "Pattern Cmd";
			case MINI_GAMES_MANAGER: return "Mini Games Manager";
			case MINI_GAMES_ORGANIZER: return "Mini Games Organizer";
			case MINI_GAME: return "Mini Game";
			case MINI_GAME_TIME_ATTACK: return "Mini Game Time Attack";
			case MINI_GAME_BOX_COUNTER: return "Mini Game Box Counter";
			case HUNTER: return "Hunter";
			default: return "Unknown";
			}
		}

		static std::string GetAnimationInfo(CActor* pActor) {
			std::stringstream sstream;
			if (pActor) {
				CAnimation* pAnim = pActor->pAnimationController;

				if (pAnim) {
					sstream << "Anim: 0x" << std::hex << pAnim->currentAnimType_0x30;
					sstream << " - " << pAnim->anmBinMetaAnimator.aAnimData[0].currentAnimDesc.animType;
				}
			}

			return sstream.str();
		}

		static std::array<std::pair<Setting<bool>, std::function<std::string(CActor*)>>, 7> gOverlaySettings = {
			std::make_pair(Setting("Name", true), [](CActor* pActor) { return pActor->name; }),
			std::make_pair(Setting("Type", true), [](CActor* pActor) { return GetActorTypeString(pActor->typeID); }),
			std::make_pair(Setting("Location", true), [](CActor* pActor) { return pActor->currentLocation.ToString(); }),
			std::make_pair(Setting("Behaviour", true), [](CActor* pActor) { return Actor::Behaviour::GetActorBehaviourName(pActor); }),
			std::make_pair(Setting("State", true), [](CActor* pActor) { return Actor::State::GetActorStateName(pActor); }),
			std::make_pair(Setting("Collision flags", true), [](CActor* pActor) {
				std::stringstream sstream;
				sstream << "0x" << std::hex << (pActor->pCollisionData ? pActor->pCollisionData->flags_0x0 : 0);
				sstream << " - 0x";
				sstream << std::hex << (pActor->pCollisionData ? pActor->pCollisionData->flags_0x4 : 0);
				return sstream.str();
				}),
			std::make_pair(Setting("Animation", false), GetAnimationInfo),
		};

		void ForEachActiveActor(PerActorFunc pFunc) {
			auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

			for (int i = 0; i < pActorManager->nbActiveActors; i++) {
				pFunc(pActorManager->aActiveActors[i]);
			}
		}

		void ForEachSectorActor(PerActorFunc pFunc) {
			auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

			for (int i = 0; i < pActorManager->nbSectorActors; i++) {
				pFunc(pActorManager->aSectorActors[i]);
			}
		}

		void ForEachActor(PerActorFunc pFunc) {
			auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

			for (int i = 0; i < pActorManager->nbActors; i++) {
				pFunc(pActorManager->aActors[i]);
			}
		}

		static void ShowActorOverlay(CActor* pActor) {
			const edF32VECTOR4 heroLocation = CActorHero::_gThis->currentLocation;
			const edF32VECTOR4 worldPos = pActor->currentLocation;

			// Calculate the distance between the hero and the actor the long way
			float distance = sqrtf((worldPos.x - heroLocation.x) * (worldPos.x - heroLocation.x) +
				(worldPos.y - heroLocation.y) * (worldPos.y - heroLocation.y) +
				(worldPos.z - heroLocation.z) * (worldPos.z - heroLocation.z));

			if (fabs(distance) > Actor::gActorInfoDistance) {
				return;
			}

			ImVec2 screenPos;
			if (Projection::WorldToScreen(worldPos, screenPos)) {
				std::string buff;

				for (auto& [setting, func] : gOverlaySettings) {
					if (!setting) {
						continue;
					}

					buff += func(pActor);
					buff += "\n";
				}

				ImGui::SetCursorPos(screenPos);
				ImGui::Text("%s", buff.c_str());
			}
		}
	}
}

void Debug::Actor::ShowMenu(bool* bOpen)
{
	ImGui::Begin("Actor", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::CollapsingHeader("Active Actors")) {
		ForEachActiveActor([](CActor* pActor) {
			ImGui::Text(pActor->name);
			});
	}

	gShowActorNamesOverlay.DrawImguiControl();
	gOnlyActiveActors.DrawImguiControl();

	if (gShowActorNamesOverlay) {
		if (ImGui::CollapsingHeader("Overlay Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto& [setting, _] : gOverlaySettings) {
				setting.DrawImguiControl();
			}
		}
	}

	gActorInfoDistance.DrawImguiControl();

	ImGui::End();

	if (gShowActorNamesOverlay) {
		// Remove the title bar and window border and make the window transparent
		ImGui::Begin("Actor Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

		// Overlay this window on top of the game window
		ImGui::SetWindowPos(FrameBuffer::GetGameWindowPosition(), ImGuiCond_Always);
		ImGui::SetWindowSize(FrameBuffer::GetGameWindowSize(), ImGuiCond_Always);

		if (gOnlyActiveActors) {
			ForEachActiveActor(ShowActorOverlay);
		}
		else {
			ForEachActor(ShowActorOverlay);
		}

		ImGui::End();
	}
}