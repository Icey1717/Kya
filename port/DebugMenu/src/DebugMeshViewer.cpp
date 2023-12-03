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

namespace DebugMeshViewer {
	edpkt_data gPkt[0x100];
	bool gUseGlslPipeline = true;
	ImTextureID gMeshViewerTexture;

	// Returns whether we prepared any indices.
	bool UpdateDrawBuffer(ed_3D_Scene* p3dScene, edNODE* pNode) {
		if (pNode) {

			ed_3d_hierarchy* p3dHier = (ed_3d_hierarchy*)pNode->pData;
			ed3DLod* pLod = ed3DChooseGoodLOD(p3dHier);
			ed_hash_code* pObjHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
			MeshData_OBJ* pMeshOBJ = (MeshData_OBJ*)LOAD_SECTION(pObjHash->pData);

			ed_3d_strip* p3dStrip = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->body.p3DStrip);

			if (p3dStrip && p3dHier) {
				ed3DInitRenderEnvironement(p3dScene, 0);

				std::vector<ed_3d_strip*> strips;

				for (int stripIndex = 0; stripIndex < pMeshOBJ->body.stripCount; stripIndex++) {
					strips.push_back(p3dStrip);
					p3dStrip = (ed_3d_strip*)LOAD_SECTION(p3dStrip->pNext);
				}

				std::reverse(strips.begin(), strips.end());

				for (auto& pCurrentStrip : strips) {

					char* pVifList = ((char*)pCurrentStrip) + pCurrentStrip->vifListOffset;

					short* pAnimIndexes = (short*)((char*)pCurrentStrip + pCurrentStrip->vifListOffset + -0x30);

					VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();

					uint incPacketSize = ed3DFlushStripGetIncPacket(pCurrentStrip, 0, 0);
					uint partialMeshSectionCount = (uint)(ushort)pCurrentStrip->meshCount % 3;
					ushort fullMeshSectionCount = (ushort)pCurrentStrip->meshCount - partialMeshSectionCount;
					uint mode = 1;
					uint maxAnimMatrixCount = 0;

					ScratchPadRenderInfo renderInfo;
					renderInfo.pSharedMeshTransform = &p3dHier->transformB;
					renderInfo.biggerScale = ed3DMatrixGetBigerScale(&p3dHier->transformB);
					auto pDma = ed3DListCreateDmaMatrixNode(&renderInfo, p3dHier);
					ed3DPKTAddMatrixPacket(gPkt, pDma);
					edF32MATRIX4* pObjToScreen = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_SCREEN_MATRIX, edF32MATRIX4*);
					edF32MATRIX4* pObjToCamera = SCRATCHPAD_ADDRESS_TYPE(OBJECT_TO_CAMERA_MATRIX_SPR, edF32MATRIX4*);

					static auto startTime = std::chrono::high_resolution_clock::now();
					static auto lastTime = std::chrono::high_resolution_clock::now();

					auto currentTime = std::chrono::high_resolution_clock::now();
					float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
					float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
					lastTime = currentTime;

					auto model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

					static float viewY = 1.0f;

					if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
   						viewY += 1.0f * deltaTime;
					}

					const float radius = 10.0f;
					float camX = sinf(time) * radius;
					float camZ = cosf(time) * radius;


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

					//vertexConstantBuffer.model = *pObjToScreen;
					//vertexConstantBuffer.worldToCamera = *WorldToCamera_Matrix;
					//vertexConstantBuffer.objectToCamera = *pObjToCamera;

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

					bool bVar1;
					int animMatrixIndex = 0;

					if (p3dHier->pAnimMatrix) {
						// Push anim matrices.
						for (; (bVar1 = maxAnimMatrixCount != 0, maxAnimMatrixCount = maxAnimMatrixCount + -1, bVar1 && (-1 < *pAnimIndexes)); pAnimIndexes = pAnimIndexes + 1)
						{
							vertexConstantBuffer.animMatrices[animMatrixIndex] = p3dHier->pAnimMatrix[*pAnimIndexes];

							// #Hack
							edF32Matrix4SetIdentityHard(&vertexConstantBuffer.animMatrices[animMatrixIndex]);
							animMatrixIndex++;
						}
					}

					auto AddVertices = []() {
						char* vtxStart = VU1Emu::GetVertexDataStart();

						Gif_Tag gifTag;
						gifTag.setTag((u8*)vtxStart, true);

						vtxStart += 0x10;

						for (int i = 0; i < gifTag.nLoop; i++) {
							Renderer::GSVertexUnprocessed vtx;
							memcpy(&vtx.STQ, vtxStart, sizeof(vtx.STQ));
							memcpy(&vtx.RGBA, vtxStart + 0x10, sizeof(vtx.RGBA));
							memcpy(&vtx.XYZSkip, vtxStart + 0x20, sizeof(vtx.XYZSkip));

							const uint primReg = gifTag.tag.PRIM;
							const GIFReg::GSPrimPacked primPacked = *reinterpret_cast<const GIFReg::GSPrimPacked*>(&primReg);

							Renderer::KickVertex(vtx, primPacked, vtx.XYZSkip[3] & 0x8000, GetDrawBufferData());

							vtxStart += 0x30;
						};
					};

					if ((pCurrentStrip->flags & 4) == 0) {
						while (bVar1 = partialMeshSectionCount != 0, partialMeshSectionCount = partialMeshSectionCount - 1, bVar1) {
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
			}
		}

		return GetDrawBufferData().index.tail > 0;
	}
}

bool& DebugMeshViewer::GetUseGlslPipeline()
{
	return gUseGlslPipeline;
}

void DebugMeshViewer::ShowNodeMenu(ed_3D_Scene* pScene, edNODE* pNode)
{
	ImGui::Begin("Node");
	if (ImGui::CollapsingHeader("Node", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Flags: %x", pNode->header.typeField.flags);
		ImGui::Text("Type: %x", pNode->header.typeField.type);

		if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen)) {
			ed_3d_hierarchy* pHierarchy = (ed_3d_hierarchy*)pNode->pData;

			ImGui::Text("Flags: %x", pHierarchy->flags_0x9e);
			ImGui::Text("Shadow: %d", pHierarchy->bRenderShadow);
			ImGui::Text("Mesh Count: %d", pHierarchy->subMeshParentCount_0xac);
			ImGui::Text("LOD Count: %d", pHierarchy->lodCount);
			ImGui::Text("Anim Matrix: %d", pHierarchy->pAnimMatrix != nullptr);

			if (((pHierarchy->flags_0x9e & 0x41) == 0) && (pHierarchy->lodCount != 0)) {

				if (ImGui::CollapsingHeader("LOD", ImGuiTreeNodeFlags_DefaultOpen)) {
					ed3DLod* pLod = ed3DChooseGoodLOD(pHierarchy);

					ed_hash_code* pObjHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
					ImGui::Text("Name: %s (0x%llx)", pObjHash->hash.name);
					MeshData_OBJ* pMeshOBJ = (MeshData_OBJ*)LOAD_SECTION(pObjHash->pData);

					ImGui::Text("Strip Count: %d", pMeshOBJ->body.stripCount);
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
					}
				}
			}
		}
	}

	ImGui::End();

	if (UpdateDrawBuffer(pScene, pNode))
	{
		ImGui::Begin("Mesh Previewer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Image(gMeshViewerTexture, ImVec2(gWidth, gHeight));
		ImGui::End();
	}
}

void DebugMeshViewer::OnFrameBufferCreated(const ImTextureID& image)
{
	gMeshViewerTexture = image;
}