#include "DebugRendering.h"

#include "imgui.h"
#include "DebugSetting.h"
#include "port/vu1_emu.h"
#include "Native/NativeRenderer.h"
#include "DebugMeshViewer.h"
#include "ed3D.h"
#include "edDlist.h"
#include "Rendering/DisplayList.h"

// Make sure these external variables are accessible
extern DisplayList** gDList_3D[2];
extern int gNbDList_3D[2];
extern int gCurRenderState;
extern int gCurFlushState;

namespace Debug {
	namespace Rendering {
		static Debug::Setting<bool> gDisableClusterRendering = { "Disable Cluster Rendering", false };
		static Debug::Setting<bool> gForceAnimMatrixIdentity = { "Force animation matrix to identity", false };
		static Debug::Setting<bool> gEnableEmulatedRendering = { "Enable Emulated Rendering", false };

		// In DebugRendering.cpp, add this function:
		void ShowDisplayListViewer(bool* bOpen)
		{
			ImGui::Begin("Display List Viewer", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

			// Display render state info
			ImGui::Text("Current Render State: %d", gCurRenderState);
			ImGui::Text("Current Flush State: %d", gCurFlushState);
			ImGui::Separator();

			// Iterate through both render states
			for (int state = 0; state < 2; state++) {
				if (ImGui::CollapsingHeader((state == 0) ? "Render State 0" : "Render State 1", ImGuiTreeNodeFlags_DefaultOpen)) {
					ImGui::Text("Display List Count: %d", gNbDList_3D[state]);
					ImGui::Separator();

					// Iterate through all display lists in this state
					for (int i = 0; i < gNbDList_3D[state]; i++) {
						DisplayList* pDisplayList = gDList_3D[state][i];

						if (pDisplayList == nullptr) continue;

						ImGui::PushID(state * 1000 + i);

						if (ImGui::TreeNode((void*)(intptr_t)i, "Display List %d (0x%p)", i, pDisplayList)) {
							// Display list properties
							ImGui::Text("Flags: 0x%04X", pDisplayList->flags_0x0);
							ImGui::Indent();
							if (pDisplayList->flags_0x0 & DISPLAY_LIST_FLAG_3D) ImGui::Text("- 3D");
							if (pDisplayList->flags_0x0 & DISPLAY_LIST_FLAG_RECORDING_PATCH) ImGui::Text("- 2D");
							if (pDisplayList->flags_0x0 & DISPLAY_LIST_FLAG_2D_BEFORE_3D) ImGui::Text("- 2D Before 3D");
							if (pDisplayList->flags_0x0 & DISPLAY_LIST_FLAG_SAVE_COMMANDS) ImGui::Text("- Save Commands");
							ImGui::Unindent();
							ImGui::Separator();

							ImGui::Text("Commands: %d / %d", pDisplayList->nbCommands, pDisplayList->nbMaxCommands);
							ImGui::Text("Saved Commands: %d", pDisplayList->nbSavedCommands);
							ImGui::Text("Scene: 0x%p", pDisplayList->pScene);
							ImGui::Text("Field 0x3: %d", pDisplayList->field_0x3);

							ImGui::Separator();

							// Display commands
							if (pDisplayList->nbCommands > 0 && ImGui::TreeNode("Commands")) {
								for (uint cmdIdx = 0; cmdIdx < pDisplayList->nbCommands; cmdIdx++) {
									DisplayListCommand* pCmd = &pDisplayList->aCommands[cmdIdx];

									ImGui::PushID(cmdIdx);
									if (ImGui::TreeNode((void*)(intptr_t)cmdIdx, "Command %d - Type: %d", cmdIdx, pCmd->dataType)) {
										ImGui::Text("Data Type: %d", pCmd->dataType);
										ImGui::Text("Prim Type: %d", pCmd->primType);
										ImGui::Text("Active: %s", pCmd->bActive ? "Yes" : "No");
										ImGui::Text("Vertices: %d", pCmd->nbAddedVertex);
										ImGui::Text("Matrices: %d", pCmd->nbMatrix);

										// Display matrix
										if (ImGui::TreeNode("Matrix")) {
											edF32MATRIX4* m = &pCmd->matrix;
											ImGui::Text("%.3f %.3f %.3f %.3f", m->aa, m->ab, m->ac, m->ad);
											ImGui::Text("%.3f %.3f %.3f %.3f", m->ba, m->bb, m->bc, m->bd);
											ImGui::Text("%.3f %.3f %.3f %.3f", m->ca, m->cb, m->cc, m->cd);
											ImGui::Text("%.3f %.3f %.3f %.3f", m->da, m->db, m->dc, m->dd);
											ImGui::TreePop();
										}

										// Display specific data based on type
										if (pCmd->dataType == DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST ||
											pCmd->dataType == 2 ||
											pCmd->dataType == 0) {
											ed_3d_strip* pStrip = pCmd->pRenderInput.pStrip;
											if (pStrip && ImGui::TreeNode("Strip Data")) {
												ImGui::Text("Material Index: %d", pStrip->materialIndex);
												ImGui::Text("Mesh Count: %d", pStrip->meshCount);
												ImGui::Text("Flags: 0x%08X", pStrip->flags);
												ImGui::Text("Shadow Cast: %d", pStrip->shadowCastFlags);
												ImGui::Text("Shadow Receive: %d", pStrip->shadowReceiveFlags);
												ImGui::Text("Bounding Sphere: (%.2f, %.2f, %.2f, %.2f)",
													pStrip->boundingSphere.x,
													pStrip->boundingSphere.y,
													pStrip->boundingSphere.z,
													pStrip->boundingSphere.w);
												ImGui::TreePop();
											}
										}
										else if (pCmd->dataType == DISPLAY_LIST_DATA_TYPE_SPRITE ||
											pCmd->dataType == 6) {
											ed_3d_sprite* pSprite = pCmd->pRenderInput.pSprite;
											if (pSprite && ImGui::TreeNode("Sprite Data")) {
												ImGui::Text("Material Index: %d", pSprite->materialIndex);
												ImGui::Text("Batches: %d", pSprite->nbBatches);
												ImGui::Text("Remainder Rects: %d", pSprite->nbRemainderRects);
												ImGui::Text("Flags: 0x%08X", pSprite->flags_0x0);
												ImGui::TreePop();
											}
										}

										ImGui::TreePop();
									}
									ImGui::PopID();
								}
								ImGui::TreePop();
							}

							ImGui::TreePop();
						}

						ImGui::PopID();
					}
				}
			}

			ImGui::End();
		}
	}
}

void Debug::Rendering::ShowMenu(bool* bOpen)
{
	// Create a new ImGui window
	ImGui::Begin("Rendering", bOpen, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Render Time: %.1f ms", Renderer::Native::GetRenderTime());
	ImGui::Text("Render Wait Time: %.1f ms", Renderer::Native::GetRenderWaitTime());
	ImGui::Text("Render Thread Time: %.1f ms", Renderer::Native::GetRenderThreadTime());

	if (ImGui::CollapsingHeader("VU1 Emulation", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Use Hardware Draw", &VU1Emu::GetHardwareDrawEnabled());
		ImGui::Checkbox("Use Interpreter", &VU1Emu::GetInterpreterEnabled());
		ImGui::Checkbox("Single Threaded", &VU1Emu::GetRunSingleThreaded());
		ImGui::Checkbox("Simplified Code", &VU1Emu::GetRunSimplifiedCode());
	}

	if (ImGui::Button("Enable Vertex Trace")) {
		VU1Emu::GetTraceVtx() = true;
	}

	if (ImGui::Checkbox("Complex Blending", &Renderer::GetUseComplexBlending())) {
		Renderer::ResetRenderer();
	}

	ImGui::Checkbox("Use GLSL Pipeline", &DebugMeshViewer::GetUseGlslPipeline());

	ImGui::Checkbox("Force Highest LOD", &ed3D::DebugOptions::GetForceHighestLod());

	if (gDisableClusterRendering.DrawImguiControl()) {
		ed3D::DebugOptions::GetDisableClusterRendering() = gDisableClusterRendering;
	}

	if (gForceAnimMatrixIdentity.DrawImguiControl()) {
		Renderer::GetForceAnimMatrixIdentity() = gForceAnimMatrixIdentity;
	}

	if (gEnableEmulatedRendering.DrawImguiControl()) {
		VU1Emu::GetEnableEmulatedRendering() = gEnableEmulatedRendering;
	}

	// Button to toggle the display list viewer
	static bool bDisplayListViewerOpen = false;
	if (ImGui::Button("Display List Viewer")) {
		bDisplayListViewerOpen = !bDisplayListViewerOpen;
	}

	if (bDisplayListViewerOpen) {
		ShowDisplayListViewer(&bDisplayListViewerOpen);
	}

	// End the ImGui window
	ImGui::End();
}

void Debug::Rendering::Init()
{
	ed3D::DebugOptions::GetDisableClusterRendering() = gDisableClusterRendering;
	Renderer::GetForceAnimMatrixIdentity() = gForceAnimMatrixIdentity;
	VU1Emu::GetEnableEmulatedRendering() = gEnableEmulatedRendering;
}

bool Debug::Rendering::GetEnableEmulatedRendering()
{
	return gEnableEmulatedRendering;
}
