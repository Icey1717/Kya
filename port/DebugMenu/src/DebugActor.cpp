#include "DebugActor.h"

#include <imgui.h>

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>

#include "DebugSetting.h"
#include "ActorManager.h"
#include "Actor.h"
#include "LargeObject.h"
#include "DebugFrameBuffer.h"

namespace Renderer
{
	namespace Native
	{
		extern glm::mat4 gCachedViewMatrix;
		extern glm::mat4 gCachedProjMatrix;
	}
}

namespace Debug {
	namespace Actor {
		static Setting<bool> gShowActorNamesOverlay("Show Actor Names Overlay", true);
		static Setting<bool> gOnlyActiveActors("Show Only Active", true);
	}

	// Get the string version of the actor type
	static const char* GetActorTypeString(int type) {
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
		default: return "Unknown";
		}
	}

	static std::array<std::pair<Setting<bool>, std::function<std::string(CActor*)>>, 5> gOverlaySettings = {
		std::make_pair(Setting("Name", true), [](CActor* pActor) { return pActor->name; }),
		std::make_pair(Setting("Type", true), [](CActor* pActor) { return GetActorTypeString(pActor->typeID); }),
		std::make_pair(Setting("Location", true), [](CActor* pActor) { return pActor->currentLocation.ToString(); }),
		std::make_pair(Setting("Behaviour", true), [](CActor* pActor) { return std::to_string(pActor->curBehaviourId); }),
		std::make_pair(Setting("State", true), [](CActor* pActor) {
			std::stringstream sstream;
			sstream << std::hex << pActor->actorState;
			return sstream.str(); 
			}),
	};

	static void ForEachActiveActor(std::function<void(CActor*)> pFunc) {
		auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

		for (int i = 0; i < pActorManager->nbActiveActors; i++) {
			pFunc(pActorManager->aActiveActors[i]);
		}
	}

	static void ForEachActor(std::function<void(CActor*)> pFunc) {
		auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

		for (int i = 0; i < pActorManager->nbActors; i++) {
			pFunc(pActorManager->aActors[i]);
		}
	}

	static bool WorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewProjMatrix, ImVec2& screenPos) {
		// Step 1: Multiply the world position by the view-projection matrix to get clip space coordinates
		glm::vec4 clipSpacePos = viewProjMatrix * glm::vec4(worldPos, 1.0f);

		// Check if the object is behind the camera (z-component in clip space is negative)
		if (clipSpacePos.z < 0.0f) {
			return false; // The actor is behind the camera, don't draw it
		}

		// Step 2: Perform perspective divide (clip space -> normalized device coordinates)
		if (clipSpacePos.w != 0.0f) {
			clipSpacePos.x /= clipSpacePos.w;
			clipSpacePos.y /= clipSpacePos.w;
			clipSpacePos.z /= clipSpacePos.w;
		}

		// Step 3: Map normalized device coordinates to screen space (from [-1,1] to [0, displaySize])
		screenPos.x = ((clipSpacePos.x + 1.0f) * 0.5f) * FrameBuffer::GetGameWindowSize().x;
		screenPos.y = ((1.0f - clipSpacePos.y) * 0.5f) * FrameBuffer::GetGameWindowSize().y;

		if (screenPos.x < 0.0f || screenPos.x > FrameBuffer::GetGameWindowSize().x) {
			return false;
		}

		if (screenPos.y < 0.0f || screenPos.y > FrameBuffer::GetGameWindowSize().y) {
			return false;
		}

		return true;
	}

	static void ShowActorOverlay(CActor* pActor) {
		const glm::vec3 worldPos = glm::vec3(pActor->currentLocation.x, pActor->currentLocation.y, pActor->currentLocation.z);

		ImVec2 screenPos;
		if (WorldToScreen(worldPos, Renderer::Native::gCachedProjMatrix * Renderer::Native::gCachedViewMatrix, screenPos)) {
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