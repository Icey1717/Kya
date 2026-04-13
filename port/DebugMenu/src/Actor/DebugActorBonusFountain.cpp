#include "DebugActorBonusFountain.h"

#include "ActorBonusFountain.h"
#include "ActorBonus.h"
#include "ActorBonusServices.h"
#include "Native/NativeDebugShapes.h"

#include <imgui.h>
#include <string>

namespace Debug::Actor::BonusFountain
{
	static constexpr uint BONUS_INSTANCE_TYPE_FLAG = 8;

	static void DrawBonusInstanceLinks(CActorBonusFountain* pBonusFountain, int totalStored)
	{
		if (pBonusFountain == nullptr || pBonusFountain->addOnGenerator.aInstances == nullptr) {
			return;
		}

		const edF32VECTOR4& fountainPos = pBonusFountain->currentLocation;
		for (int i = 0; i < totalStored; ++i) {
			CActInstance* pInstance = pBonusFountain->addOnGenerator.aInstances[i];
			if (pInstance == nullptr) {
				continue;
			}

			const uint flags = pInstance->flags;
			if ((flags & BONUS_INSTANCE_TYPE_FLAG) == 0 || (flags & ACTOR_INSTANCE_FLAG_ALIVE) == 0) {
				continue;
			}

			const edF32VECTOR4& instancePos = pInstance->currentPosition;
			Renderer::Native::DebugShapes::AddLine(
				fountainPos.x, fountainPos.y, fountainPos.z,
				instancePos.x, instancePos.y, instancePos.z,
				0.2f, 0.9f, 1.0f, 0.95f);
		}
	}

	static void ShowBonusInstance(CBnsInstance* pInstance, int slotIndex, int bonusIndex, const edF32VECTOR4& fountainPosition)
	{
		if (pInstance == nullptr) {
			return;
		}

		const bool bIsAlive = (pInstance->flags & ACTOR_INSTANCE_FLAG_ALIVE) != 0;
		const bool bIsVisible = (pInstance->flags & ACTOR_INSTANCE_FLAG_VISIBLE) != 0;

		std::string label = "BnsInstance #" + std::to_string(bonusIndex) +
			" (slot " + std::to_string(slotIndex) + ")";
		if (!ImGui::TreeNode(label.c_str())) {
			return;
		}

		const float dx = pInstance->currentPosition.x - fountainPosition.x;
		const float dy = pInstance->currentPosition.y - fountainPosition.y;
		const float dz = pInstance->currentPosition.z - fountainPosition.z;
		const float distanceToFountain = sqrtf(dx * dx + dy * dy + dz * dz);

		ImGui::Text("Ptr: %p", pInstance);
		ImGui::Text("Alive: %s", bIsAlive ? "yes" : "no");
		ImGui::Text("Visible: %s", bIsVisible ? "yes" : "no");
		ImGui::Text("Flags: 0x%08X", pInstance->flags);
		ImGui::Text("State: %d", pInstance->state);
		ImGui::Text("Instance Index: %d", pInstance->instanceIndex);
		ImGui::Text("Distance To Fountain: %.2f", distanceToFountain);
		ImGui::Text("Current Position: (%.2f, %.2f, %.2f)",
			pInstance->currentPosition.x, pInstance->currentPosition.y, pInstance->currentPosition.z);
		ImGui::Text("Path Delta: (%.2f, %.2f, %.2f)",
			pInstance->pathDelta.x, pInstance->pathDelta.y, pInstance->pathDelta.z);
		ImGui::Text("angleRotY: %.2f", pInstance->angleRotY);
		ImGui::Text("angleRotYalt: %.2f", pInstance->angleRotYalt);

		ImGui::TreePop();
	}

	void ShowBonusFountainActorDetails(CActorBonusFountain* pBonusFountain)
	{
		if (pBonusFountain == nullptr) {
			ImGui::Text("Bonus Fountain Actor: null");
			return;
		}

		CAddOnGenerator& addOnGenerator = pBonusFountain->addOnGenerator;
		const int generatedTotal = addOnGenerator.nbGeneratedBonus + addOnGenerator.nbGeneratedMoney;
		const int capacityTotal = addOnGenerator.nbBonus + addOnGenerator.nbMoney;

		int totalStored = generatedTotal;
		if (totalStored < 0) {
			totalStored = 0;
		}
		if (capacityTotal >= 0 && totalStored > capacityTotal) {
			totalStored = capacityTotal;
		}

		int bonusStored = 0;
		int bonusAlive = 0;

		if (addOnGenerator.aInstances != nullptr) {
			for (int i = 0; i < totalStored; ++i) {
				CActInstance* pInstance = addOnGenerator.aInstances[i];
				if (pInstance == nullptr) {
					continue;
				}

				if ((pInstance->flags & BONUS_INSTANCE_TYPE_FLAG) == 0) {
					continue;
				}

				++bonusStored;
				if ((pInstance->flags & ACTOR_INSTANCE_FLAG_ALIVE) != 0) {
					++bonusAlive;
				}
			}
		}

		if (ImGui::CollapsingHeader("Add-On Generator", ImGuiTreeNodeFlags_DefaultOpen)) {
			static bool bDrawBonusInstanceLinks = true;

			ImGui::Text("Flags (field_0x0): 0x%04X", addOnGenerator.field_0x0);
			ImGui::Text("Money Type: %u", addOnGenerator.moneyType);
			ImGui::Text("Configured Counts: bonus=%d, money=%d", addOnGenerator.nbBonus, addOnGenerator.nbMoney);
			ImGui::Text("Generated Counts: bonus=%d, money=%d", addOnGenerator.nbGeneratedBonus, addOnGenerator.nbGeneratedMoney);
			ImGui::Text("Stored BnsInstances: %d (alive: %d)", bonusStored, bonusAlive);
			ImGui::Text("Instance Array: %p", addOnGenerator.aInstances);
			ImGui::Checkbox("Draw Bonus Instance Links", &bDrawBonusInstanceLinks);

			if (bDrawBonusInstanceLinks) {
				DrawBonusInstanceLinks(pBonusFountain, totalStored);
			}
		}

		if (ImGui::CollapsingHeader("Bonus Instances", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (addOnGenerator.aInstances == nullptr || totalStored == 0 || bonusStored == 0) {
				ImGui::TextDisabled("No bonus instances are currently stored.");
				return;
			}

			int bonusIndex = 0;
			for (int i = 0; i < totalStored; ++i) {
				CActInstance* pBaseInstance = addOnGenerator.aInstances[i];
				if (pBaseInstance == nullptr || (pBaseInstance->flags & BONUS_INSTANCE_TYPE_FLAG) == 0) {
					continue;
				}

				ShowBonusInstance(
					static_cast<CBnsInstance*>(pBaseInstance),
					i,
					bonusIndex,
					pBonusFountain->currentLocation);
				++bonusIndex;
			}
		}
	}
}
