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
		static Setting<bool> gShowActorNamesOverlay("Show Actor Names Overlay", "");
	}

	static void ForEachActiveActor(std::function<void(CActor*)> pFunc) {
		auto* pActorManager = CScene::ptable.g_ActorManager_004516a4;

		for (int i = 0; i < pActorManager->nbActiveActors; i++) {
			pFunc(pActorManager->aActiveActors[i]);
		}
	}

	static ImVec2 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewProjMatrix) {
		// Step 1: Multiply the world position by the view-projection matrix to get clip space coordinates
		glm::vec4 clipSpacePos = viewProjMatrix * glm::vec4(worldPos, 1.0f);

		// Step 2: Perform perspective divide (clip space -> normalized device coordinates)
		if (clipSpacePos.w != 0.0f) {
			clipSpacePos.x /= clipSpacePos.w;
			clipSpacePos.y /= clipSpacePos.w;
			clipSpacePos.z /= clipSpacePos.w;
		}

		// Step 3: Map normalized device coordinates to screen space (from [-1,1] to [0, displaySize])
		ImVec2 screenPos;
		screenPos.x = ((clipSpacePos.x + 1.0f) * 0.5f) * FrameBuffer::GetGameWindowSize().x;
		screenPos.y = ((1.0f - clipSpacePos.y) * 0.5f) * FrameBuffer::GetGameWindowSize().y;

		return screenPos;
	}

	static ImVec2 ScreenToImGui(const ImVec2& screenPos, const ImVec2& windowPos) {
		ImVec2 imguiPos;
		imguiPos.x = screenPos.x - windowPos.x;
		imguiPos.y = screenPos.y - windowPos.y;
		return imguiPos;
	}

	static void ShowActorOverlay(CActor* pActor) {
		glm::vec3 worldPos = glm::vec3(pActor->currentLocation.x, pActor->currentLocation.y, pActor->currentLocation.z);
		ImVec2 screenPos = WorldToScreen(worldPos, Renderer::Native::gCachedProjMatrix * Renderer::Native::gCachedViewMatrix);
		ImVec2 imguiPos = ScreenToImGui(screenPos, FrameBuffer::GetGameWindowPosition());
		imguiPos.x += ImGui::GetIO().DisplaySize.x * 0.25f;
		// Render the actor's name at the projected ImGui positionaa
		ImGui::SetCursorPos(screenPos);
		ImGui::Text("%s", pActor->name);
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

	ImGui::End();

	if (gShowActorNamesOverlay) {
		// Remove the title bar and window border and make the window transparent
		ImGui::Begin("Actor Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

		// Overlay this window on top of the game window
		ImGui::SetWindowPos(FrameBuffer::GetGameWindowPosition(), ImGuiCond_Always);
		ImGui::SetWindowSize(FrameBuffer::GetGameWindowSize(), ImGuiCond_Always);

		ForEachActiveActor(ShowActorOverlay);

		ImGui::End();
	}
}