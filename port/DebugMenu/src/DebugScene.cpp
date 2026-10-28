#include "DebugMenu.h"
#include "DebugScene.h"

#include "imgui.h"

#include "Scene.h"
#include "DebugSetting.h"
#include "ed3D.h"
#include "Rendering/DisplayList.h"
#include "LevelScheduler.h"
#include <chrono>

namespace Debug {
	namespace Scene {
		Debug::Setting<int> gAutoLoadLevelId = { "Auto Load Level ID", -1 };
		static bool gSweepActive = false;
		static bool gSweepWaiting = false;
		static bool gSweepTransitionStarted = false;
		static int gSweepLevel = -1;
		static int gSweepGeneration = 0;
		static float gSweepDelay = 2.0f;
		static const char* gSweepStatus = "Idle";
		static std::chrono::steady_clock::time_point gSweepTime;

		void Startup()
		{
			gDebugLevelLoadOverride = gAutoLoadLevelId;

			// Reset, if we load successfully we will set it back.
			gAutoLoadLevelId = -1;
		}

		static void ShowMenu(bool* bOpen) {
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

			if (ImGui::CollapsingHeader("Load All Levels")) {
				ImGui::InputFloat("Seconds per level", &gSweepDelay, 1.0f);
				if (gSweepDelay < 0.0f) gSweepDelay = 0.0f;
				if (!gSweepActive) {
					if (ImGui::Button("Start Level Sweep")) {
						++gSweepGeneration;
						gSweepActive = true;
						gSweepWaiting = false;
						gSweepLevel = -1;
						gSweepStatus = "Starting";
						gSweepTime = std::chrono::steady_clock::now();
					}
				}
				else if (ImGui::Button("Stop Level Sweep")) {
					++gSweepGeneration;
					gSweepActive = false;
					gSweepStatus = "Stopped";
				}
				ImGui::Text("%s (level %d)", gSweepStatus, gSweepLevel);
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

		void Update()
		{
			auto* pScheduler = CScene::ptable.g_LevelScheduleManager_00451660;
			if (gSweepActive && pScheduler && CScene::_pinstance) {
				const auto now = std::chrono::steady_clock::now();
				const double elapsed = std::chrono::duration<double>(now - gSweepTime).count();
				if (gSweepWaiting) {
					if (gSweepTransitionStarted && pScheduler->currentLevelID == gSweepLevel && pScheduler->nextLevelID == 0x10 &&
						!CScene::_pinstance->IsFadeTermActive()) {
						gSweepWaiting = false;
						gSweepStatus = "Loaded";
						gSweepTime = now;
					}
					else if (elapsed > 60.0) {
						++gSweepGeneration;
						gSweepActive = false;
						gSweepStatus = "Timed out waiting for level";
					}
				}
				else if ((gSweepLevel == -1 || elapsed >= gSweepDelay) && pScheduler->nextLevelID == 0x10 &&
					!CScene::_pinstance->IsFadeTermActive()) {
					int nextLevel = gSweepLevel + 1;
					while (nextLevel < 16 && pScheduler->aLevelInfo[nextLevel].levelName[0] == '\0') ++nextLevel;
					if (nextLevel == 16) {
						gSweepActive = false;
						gSweepStatus = "Complete";
					}
					else {
						gSweepLevel = nextLevel;
						gSweepWaiting = true;
						gSweepTransitionStarted = false;
						gSweepStatus = "Loading";
						gSweepTime = now;
						const int generation = gSweepGeneration;
						EnqueueLevelManageTask([nextLevel, generation]() {
							if (!gSweepActive || generation != gSweepGeneration || !CLevelScheduler::gThis || !CScene::_pinstance) return;
							CLevelScheduler::gThis->Level_FillRunInfo(nextLevel, -1, -1);
							CScene::_pinstance->SetFadeStateTerm(false);
							gSweepTransitionStarted = true;
						});
					}
				}
			}
			static bool bSuccesfullyLoaded = false;

			if (!bSuccesfullyLoaded && pScheduler) {
				if (CScene::ptable.g_LevelScheduleManager_00451660->currentLevelID == gDebugLevelLoadOverride) {
					bSuccesfullyLoaded = true;
					gAutoLoadLevelId = gDebugLevelLoadOverride;
				}
			}
		}
	} // namespace Scene
} // namespace Debug

namespace Debug {
	MenuRegisterer sDebugSceneMenuReg("Scene", Debug::Scene::ShowMenu, true);
	StartupRegisterer sDebugSceneStartupReg(Debug::Scene::Startup);
	UpdateRegisterer sDebugSceneUpdateReg(Debug::Scene::Update);
}

