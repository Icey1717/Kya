#include "DebugMeshViewer.h"

#include "ed3D.h"
#include "port/vu1_emu.h"
#include "ed3DScratchPadGlobalVar.h"
#include "port/pointer_conv.h"
#include "port.h"
#include "MathOps.h"
#include "imgui.h"
#include "DebugRenderer.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "DebugMaterialPreviewer.h"
#include "edDlist.h"
#include "TextureCache.h"
#include "gizmo.h"

#define MESH_PREVIEWER_LOG(level, format, ...) MY_LOG_CATEGORY("Mesh Previewer", level, format, ##__VA_ARGS__)

namespace DebugMeshViewer {
	bool gAnimate = true;
	bool gRotate = false;
	bool gUseGlslPipeline = true;
	bool gWireframe = false;
	bool gUseGizmo = false;
	glm::mat4 gGizmoMatrix;
	ImTextureID gMeshViewerTexture;

	PS2::TextureCache gTextureCache;

	ed_g2d_material* gMaterial;
	Renderer::TextureData gTextureData;

	int gIsolateStripIndex = -1;
	int gHighlightStripIndex = -1;
	int gHighlightAnimMatrixIndex = -1;
	std::vector<std::vector<int>> gStripBoneIndexes;

	void ShowStripBoneIndexes() {
		ImGui::Begin("Strip To Bone", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		// Iterate through the outer vector
		for (size_t i = 0; i < gStripBoneIndexes.size(); ++i) {
			// Display a label for the inner vector
			ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Strip %zu", i);

			// Iterate through the inner vector
			for (size_t j = 0; j < gStripBoneIndexes[i].size(); ++j) {
				// Display the value
				ImGui::Text("    [%zu][%zu]: %d", i, j, gStripBoneIndexes[i][j]);
			}
		}

		ImGui::End();
	}

	// Returns whether we prepared any indices.
	bool UpdateDrawBuffer(ed_3d_hierarchy* p3dHier) {
		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Begin");

		if (p3dHier) {
			ed3DLod* pLod = ed3DChooseGoodLOD(p3dHier);
			ed_hash_code* pObjHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);

			if (pObjHash) {
				MeshData_OBJ* pMeshOBJ = (MeshData_OBJ*)LOAD_SECTION(pObjHash->pData);

				ed_3d_strip* p3dStrip = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->body.p3DStrip);

				if (p3dStrip && p3dHier) {
					VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();
					int maxAnimIndex = 0;

					std::vector<ed_3d_strip*> strips;

					for (int stripIndex = 0; stripIndex < pMeshOBJ->body.stripCount; stripIndex++) {
						strips.push_back(p3dStrip);
						p3dStrip = (ed_3d_strip*)LOAD_SECTION(p3dStrip->pNext);
					}

					MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Found {} strips", strips.size());

					std::reverse(strips.begin(), strips.end());

					gStripBoneIndexes.clear();

					for (int stripIndex = 0; stripIndex < strips.size(); stripIndex++) {
						assert(stripIndex < gMaxStripIndex);
						auto& pCurrentStrip = strips[stripIndex];

						MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Strip: 0x{:x}", (uintptr_t)pCurrentStrip);

						if (gIsolateStripIndex != -1 && gIsolateStripIndex != stripIndex) {
							continue;
						}

						auto& currentStripBoneList = gStripBoneIndexes.emplace_back();

						char* pVifList = ((char*)pCurrentStrip) + pCurrentStrip->vifListOffset;

						short* pAnimIndexes = (short*)((char*)pCurrentStrip + pCurrentStrip->vifListOffset + -0x30);

						uint incPacketSize = ed3DFlushStripGetIncPacket(pCurrentStrip, 0, 0);
						uint partialMeshSectionCount = (uint)(ushort)pCurrentStrip->meshCount % 3;
						ushort fullMeshSectionCount = (ushort)pCurrentStrip->meshCount - partialMeshSectionCount;
						uint mode = 1;
						uint maxAnimMatrixCount = 0;

						static auto startTime = std::chrono::high_resolution_clock::now();
						static auto lastTime = std::chrono::high_resolution_clock::now();

						auto currentTime = std::chrono::high_resolution_clock::now();
						float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
						float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
						lastTime = currentTime;

						auto model = glm::rotate(glm::mat4(1.0f), gRotate ? (time * glm::radians(90.0f)) : 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

						if (gUseGizmo) {
							model = gGizmoMatrix;
						}

						static glm::vec3 cameraPos = glm::vec3(0.0f, 1.2f, 3.8f);
						static glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
						static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

						const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
						if (ImGui::IsKeyPressed(ImGuiKey_Comma))
							cameraPos += cameraSpeed * cameraFront;
						if (ImGui::IsKeyPressed(ImGuiKey_O))
							cameraPos -= cameraSpeed * cameraFront;
						//if (ImGui::IsKeyPressed(ImGuiKey_A))
						//	cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
						//if (ImGui::IsKeyPressed(ImGuiKey_E))
						//	cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

						if (ImGui::IsKeyPressed(ImGuiKey_A))
							cameraPos -= cameraSpeed * cameraUp;
						if (ImGui::IsKeyPressed(ImGuiKey_E))
							cameraPos += cameraSpeed * cameraUp;

						auto view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
						auto proj = glm::perspective(glm::radians(45.0f), (float)gWidth / (float)gHeight, 0.1f, 100.0f);
						proj[1][1] *= -1;

						memcpy(&vertexConstantBuffer.model, &model, sizeof(edF32MATRIX4));
						memcpy(&vertexConstantBuffer.view, &view, sizeof(edF32MATRIX4));
						memcpy(&vertexConstantBuffer.proj, &proj, sizeof(edF32MATRIX4));

						if ((pCurrentStrip->flags & 0x8000000) == 0) {
							maxAnimMatrixCount = 0x18;
						}
						else {
							if (mode < 2) {
								maxAnimMatrixCount = 9;
							}
							else {
								maxAnimMatrixCount = 7;
							}
						}

						bool bValidAnimIndex;
						int animMatrixIndex = 0;
						for (; (bValidAnimIndex = maxAnimMatrixCount != 0, maxAnimMatrixCount = maxAnimMatrixCount + -1, bValidAnimIndex && (-1 < *pAnimIndexes)); pAnimIndexes = pAnimIndexes + 1) {
							assert(animMatrixIndex < gMaxStripIndex);
							MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Assigning {} -> {},{}", *pAnimIndexes, stripIndex, animMatrixIndex);
							vertexConstantBuffer.animStripToIndex[stripIndex][animMatrixIndex].index = *pAnimIndexes;
							maxAnimIndex = std::max<int>(maxAnimIndex, *pAnimIndexes);

							currentStripBoneList.push_back(*pAnimIndexes);

							animMatrixIndex++;
						}

						MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Final anim index: {}", animMatrixIndex);

						auto AddVertices = [stripIndex]() {
							char* vtxStart = VU1Emu::GetVertexDataStart();

							Gif_Tag gifTag;
							gifTag.setTag((u8*)vtxStart, true);

							vtxStart += 0x10;

							for (int i = 0; i < gifTag.nLoop; i++) {
								Renderer::GSVertexUnprocessed vtx;
								memcpy(&vtx.STQ, vtxStart, sizeof(vtx.STQ));
								memcpy(&vtx.RGBA, vtxStart + 0x10, sizeof(vtx.RGBA));
								memcpy(&vtx.XYZSkip, vtxStart + 0x20, sizeof(vtx.XYZSkip));

								const uint vtxAnimMatrix = ((vtx.XYZSkip[3] & 0x7ff) - 0x3dc) / 4;

								if (gHighlightStripIndex == stripIndex || vtxAnimMatrix == gHighlightAnimMatrixIndex) {
									vtx.RGBA[0] = 0xff;
									vtx.RGBA[1] = 0xff;
									vtx.RGBA[2] = 0xff;
									vtx.RGBA[3] = 0xff;
								}

								const uint primReg = gifTag.tag.PRIM;
								const GIFReg::GSPrimPacked primPacked = *reinterpret_cast<const GIFReg::GSPrimPacked*>(&primReg);

								const uint skip = vtx.XYZSkip[3] & 0x8000;

								const uint shiftedStripIndex = stripIndex << 16;

								vtx.XYZSkip[3] |= shiftedStripIndex;

								Renderer::KickVertex(vtx, primPacked, skip, GetDrawBufferData());

								vtxStart += 0x30;
							};
							};

						if ((pCurrentStrip->flags & 4) == 0) {
							while (bValidAnimIndex = partialMeshSectionCount != 0, partialMeshSectionCount = partialMeshSectionCount - 1, bValidAnimIndex) {
								VU1Emu::ProcessVifList((edpkt_data*)pVifList, false);
								AddVertices();
								pVifList = pVifList + incPacketSize * 0x10;
							}

							for (; fullMeshSectionCount != 0; fullMeshSectionCount = fullMeshSectionCount + -3) {

								char* pVifListB = pVifList + incPacketSize * 0x10;
								char* pVifListC = pVifList + incPacketSize * 0x20;

								VU1Emu::ProcessVifList((edpkt_data*)pVifList, false);
								AddVertices();
								VU1Emu::ProcessVifList((edpkt_data*)pVifListB, false);
								AddVertices();
								VU1Emu::ProcessVifList((edpkt_data*)pVifListC, false);
								AddVertices();

								pVifList = pVifList + incPacketSize * 0x30;
							}
						}
					}

					assert(maxAnimIndex < gMaxAnimMatrices);

					for (int i = 0; i <= maxAnimIndex; i++) {
						edF32Matrix4SetIdentityHard(&vertexConstantBuffer.animMatrices[i]);

						if (p3dHier->pAnimMatrix && gAnimate) {
							vertexConstantBuffer.animMatrices[i] = p3dHier->pAnimMatrix[i];
						}
					}
				}
			}
		}

		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer End");

		return GetDrawBufferData().index.tail > 0;
	}
}

bool& DebugMeshViewer::GetUseGlslPipeline()
{
	return gUseGlslPipeline;
}

bool& DebugMeshViewer::GetWireframe()
{
	return gWireframe;
}

PS2::GSTexEntry& DebugMeshViewer::GetTextureEntry()
{
	GIFReg::GSTex TEX{};
	TEX.CBP = 0x380;
	return gTextureCache.Lookup(TEX, gTextureData);
}

void DebugMeshViewer::ShowHierarchyMenu(ed_3d_hierarchy* pHierarchy)
{
	auto pTextureInfo = (ed_hash_code*)(pHierarchy->pTextureInfo + 0x10);

	if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::Text("Flags: %x", pHierarchy->flags_0x9e);
		ImGui::Text("Shadow: %d", pHierarchy->bRenderShadow);
		ImGui::Text("Mesh Count: %d", pHierarchy->subMeshParentCount_0xac);
		ImGui::Text("LOD Count: %d", pHierarchy->lodCount);
		ImGui::Text("Anim Matrix: %d", pHierarchy->pAnimMatrix != nullptr);

		if (((pHierarchy->flags_0x9e & 0x41) == 0) && (pHierarchy->lodCount != 0)) {

			if (ImGui::CollapsingHeader("LOD", ImGuiTreeNodeFlags_DefaultOpen)) {
				ed3DLod* pLod = ed3DChooseGoodLOD(pHierarchy);

				if (pLod) {

					ed_hash_code* pObjHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
					ImGui::Text("Name: %s (0x%llx)", pObjHash->hash.name);
					MeshData_OBJ* pMeshOBJ = (MeshData_OBJ*)LOAD_SECTION(pObjHash->pData);

					ImGui::Text("Strip Count: %d", pMeshOBJ->body.stripCount);

					ImGui::InputInt("Strip Isolate", &gIsolateStripIndex);
					gIsolateStripIndex = std::clamp<int>(gIsolateStripIndex, -1, pMeshOBJ->body.stripCount - 1);

					ImGui::InputInt("Strip Highlight", &gHighlightStripIndex);
					gHighlightStripIndex = std::clamp<int>(gHighlightStripIndex, -1, pMeshOBJ->body.stripCount - 1);

					ImGui::InputInt("Anim Matrix Highlight", &gHighlightAnimMatrixIndex);

					ImGui::Text("Bounding Sphere: x: %f, y: %f, z: %f, w: %f",
						pMeshOBJ->body.boundingSphere.x,
						pMeshOBJ->body.boundingSphere.y,
						pMeshOBJ->body.boundingSphere.z,
						pMeshOBJ->body.boundingSphere.w);

					if (ImGui::CollapsingHeader("Strip", ImGuiTreeNodeFlags_DefaultOpen)) {
						ed_3d_strip* pStrip = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->body.p3DStrip);

						ImGui::Text("Flags: %x", pStrip->flags);
						ImGui::Text("Bounding Sphere: x: %f, y: %f, z: %f, w: %f", pStrip->boundingSphere.x, pStrip->boundingSphere.y, pStrip->boundingSphere.z, pStrip->boundingSphere.w);
						ImGui::Text("Mesh Count: %d", pStrip->meshCount);

						auto pMaterial = ed3DG2DGetG2DMaterialFromIndex(pTextureInfo, (int)pStrip->materialIndex);

						if (pMaterial) {
							if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
								ImGui::Text("Layer Count?: %d", pMaterial->count_0x0);

								if (ImGui::Button("Open Pre")) {
									gMaterial = pMaterial;
								}

								if (pMaterial->count_0x0 != 0) {
									auto pLayer = (ed_g2d_layer_header*)LOAD_SECTION(*(int*)(pMaterial + 1));

									ed_g2d_bitmap* pBitMap = nullptr;
									ed_g2d_bitmap* pOther = nullptr;
									if (pLayer->body.field_0x1c != 0) {
										auto pTexData = (TextureData_TEX*)LOAD_SECTION(pLayer->body.pTex);


										if (pTexData->body.palette == 0) {
											assert(false);
											TextureData_HASH_Internal_PA32* pPA32 = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(pTexData->body.hashCode.pData);
											if (pPA32 != (TextureData_HASH_Internal_PA32*)0x0) {
												TextureData_PA32* pPA32Internal = (TextureData_PA32*)LOAD_SECTION(pPA32->pPA32);
												pBitMap = &pPA32Internal->body;
											}
										}
										else {
											ed_hash_code* pCode = (ed_hash_code*)(pTexData + 1);

											pCode = (ed_hash_code*)LOAD_SECTION(pCode[(pLayer->body).field_0x1e].pData);

											if (pCode->pData != 0) {
												pBitMap = (ed_g2d_bitmap*)((char*)LOAD_SECTION(pCode->pData) + 0x10);
											}

											TextureData_HASH_Internal_PA32* pPA32 = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(pTexData->body.hashCode.pData);
											if (pPA32 != (TextureData_HASH_Internal_PA32*)0x0) {
												TextureData_PA32* pPA32Internal = (TextureData_PA32*)LOAD_SECTION(pPA32->pPA32);
												pOther = &pPA32Internal->body;
											}
										}
									}

									if (pBitMap && pOther) {
										gTextureData = MakeTextureDataFromPacket(pOther, pBitMap, 0);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void DebugMeshViewer::ShowPreviewer(ed_3d_hierarchy* pHierarchy)
{
	if (UpdateDrawBuffer(pHierarchy))
	{
		ImGui::Begin("Mesh Previewer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Checkbox("Animate", &gAnimate);
		ImGui::SameLine();
		ImGui::Checkbox("Rotate", &gRotate);
		ImGui::SameLine();
		ImGui::Checkbox("Wireframe", &GetWireframe());

		ImGui::SameLine();
		ImGui::Checkbox("Gizmo", &gUseGizmo);

		if (gUseGizmo) {
			static Gizmo::Widget gizmo = Gizmo::Widget("##giz");
			gGizmoMatrix = gizmo.Show();
		}

		ImGui::Image(gMeshViewerTexture, ImVec2(gWidth, gHeight));
		ImGui::End();

		ShowStripBoneIndexes();
	}
}

void DebugMeshViewer::ShowNodeMenu(edNODE* pNode)
{
	ImGui::Begin("Node", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::CollapsingHeader("Node", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Flags: %x", pNode->header.typeField.flags);
		ImGui::Text("Type: %x", pNode->header.typeField.type);

		ed_3d_hierarchy* pHierarchy = (ed_3d_hierarchy*)pNode->pData;
		ShowHierarchyMenu(pHierarchy);
	}

	static bool bShowPreview = true;

	ImGui::Checkbox("Show Preview", &bShowPreview);

	ImGui::End();

	if (bShowPreview) {
		ed_3d_hierarchy* pHierarchy = (ed_3d_hierarchy*)pNode->pData;
		ShowPreviewer(pHierarchy);
	}

	if (gMaterial != nullptr) {
		MaterialPreviewer::Open(gMaterial);
	}
}

void DebugMeshViewer::OnFrameBufferCreated(const ImTextureID& image)
{
	gMeshViewerTexture = image;
}