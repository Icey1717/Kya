#include "DebugScene.h"

#include "imgui.h"

#include "LargeObject.h"
#include "DebugSetting.h"
#include "ed3D.h"
#include "Rendering/DisplayList.h"
#include "LevelScheduler.h"

namespace Debug {
	namespace Scene {
		Debug::Setting<int> gAutoLoadLevelId = { "Auto Load Level ID", -1 };
	}
}

void Debug::Scene::Startup()
{
	gDebugLevelLoadOverride = gAutoLoadLevelId;
}

static void Debug::Scene::ShowMenu(bool* bOpen) {
	ImGui::Begin("Scene", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::PushID("Scene");

	if (ImGui::Button("Term Scene")) {
		GameFlags = GameFlags | GAME_REQUEST_TERM;
	}

	gAutoLoadLevelId.DrawImguiControl();

	ImGui::SameLine();

	if (ImGui::Button("Reset")) {
		gAutoLoadLevelId = -1;
	}

	if (ImGui::CollapsingHeader("Teleport", ImGuiTreeNodeFlags_DefaultOpen)) {
		static Debug::Setting<int> gTeleportLevelId = { "Teleport Level ID", 0 };
		static Debug::Setting<int> gTeleportElevatorId = { "Teleport Elevator ID", 0 };
		static Debug::Setting<int> gTeleportCutsceneId = { "Teleport Cutscene ID", 0 };

		gTeleportLevelId.DrawImguiControl();
		gTeleportElevatorId.DrawImguiControl();
		gTeleportCutsceneId.DrawImguiControl();

		if (ImGui::Button("Level Teleport")) {
			EnqueueLevelManageTask([=]() {
				CScene::ptable.g_LevelScheduleManager_00451660->Level_Teleport(nullptr, gTeleportLevelId, gTeleportElevatorId, gTeleportCutsceneId, -1);
				});
		}
	}

	if (ImGui::CollapsingHeader("Scene List")) {

		static int selectedScene = -1;

		for (int i = 0; i < ged3DConfig.sceneCount; i++) {
			char buttonText[256];
			std::sprintf(buttonText, "gScene3D[%d]", i + 1);
			if (ImGui::Selectable(buttonText)) {
				selectedScene = i;
			}
		}

		static edNODE* pSelectedNode = nullptr;
		static ed_g3d_manager* pMeshInfo = nullptr;

		if (selectedScene != -1 && ImGui::CollapsingHeader("Scene Details", ImGuiTreeNodeFlags_DefaultOpen)) {
			ed_3D_Scene* pSelectedScene = &gScene3D[selectedScene];

			static bool bFilterAnim = false;
			ImGui::Checkbox("Filter Anim", &bFilterAnim);

			ImGui::Text("Shadow: %d", pSelectedScene->bShadowScene);
			ImGui::Text("Flags: 0x%x", pSelectedScene->flags);

			if (ImGui::CollapsingHeader("Hierarchy")) {
				edNODE* pCurNode;
				edLIST* pList = pSelectedScene->pHierListA;
				if (((pSelectedScene->flags & SCENE_FLAG_IN_USE) != 0) && ((pSelectedScene->flags & 4) == 0)) {
					for (pCurNode = pList->pPrev; (edLIST*)pCurNode != pList; pCurNode = pCurNode->pPrev) {

						ed_3d_hierarchy* pHierarchy = (ed_3d_hierarchy*)pCurNode->pData;

						if (pHierarchy && (pHierarchy->pAnimMatrix || !bFilterAnim)) {
							char nodeText[256];
							std::sprintf(nodeText, "Node: %p", pCurNode);
							if (ImGui::Selectable(nodeText)) {
								pSelectedNode = pCurNode;
							}
						}
					}
				}
				else {
					ImGui::Text("Flags disabled nodes");
				}
			}

			if (ImGui::CollapsingHeader("Mesh Nodes")) {
				edNODE* pClusterNode;

				if ((pSelectedScene->bShadowScene != 1) && ((pSelectedScene->flags & 8) == 0)) {
					for (pClusterNode = (edNODE*)(pSelectedScene->meshClusterList).pPrev;
						(edLIST*)pClusterNode != &pSelectedScene->meshClusterList; pClusterNode = pClusterNode->pPrev) {
						edCluster* pCluster = (edCluster*)pClusterNode->pData;

						char nodeText[256];
						std::sprintf(nodeText, "Cluster: %p (%p)", pClusterNode, pCluster->pMeshInfo->CSTA);
						if (ImGui::Selectable(nodeText)) {
							ed_g3d_manager* pMesh;
							if (((pCluster->flags & 2) == 0) && (pMesh = pCluster->pMeshInfo, pMesh->CSTA != (ed_Chunck*)0x0)) {
								pMeshInfo = pMesh;
							}
						}
					}
				}
			}
		}
	}

	ImGui::PopID();
	ImGui::End();
}