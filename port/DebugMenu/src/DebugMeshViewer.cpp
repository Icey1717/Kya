#include "DebugMeshViewer.h"

#include "ed3D.h"
#include "port/vu1_emu.h"
#include "ed3DScratchPadGlobalVar.h"
#include "port/pointer_conv.h"
#include "port.h"
#include "MathOps.h"
#include "imgui.h"
#include "DebugRenderer.h"
#include "DebugHelpers.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "DebugMaterialPreviewer.h"
#include "edDlist.h"
#include "TextureCache.h"
#include "gizmo.h"
#include "FileManager3D.h"

#define MESH_PREVIEWER_LOG(level, format, ...) MY_LOG_CATEGORY("Mesh Previewer", level, format, ##__VA_ARGS__)

namespace DebugMeshViewer {
	bool gAnimate = true;
	bool gRotate = false;
	bool gUseGlslPipeline = true;
	bool gWireframe = false;
	bool gUseGizmo = false;
	glm::mat4 gGizmoMatrix;
	ImTextureID gMeshViewerTexture;

	ed_g2d_material* gMaterial;
	Renderer::TextureData gTextureData;

	int gIsolateStripIndex = -1;
	int gHighlightStripIndex = -1;
	int gHighlightAnimMatrixIndex = -1;
	std::vector<std::vector<int>> gStripBoneIndexes;

	void ShowStripBoneIndexes() 
	{
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

	void RenderStrip(ed_3d_strip* pCurrentStrip, const int stripIndex, int& maxAnimIndex, const bool bAnimate)
	{
		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Strip: 0x{:x}", (uintptr_t)pCurrentStrip);

		VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();

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

		if (bAnimate) {
			assert(stripIndex < gMaxStripIndex);

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
		}

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

				const uint vtxAnimMatrix = ((vtx.XYZSkip.Skip & 0x7ff) - 0x3dc) / 4;

				if (gHighlightStripIndex == stripIndex || vtxAnimMatrix == gHighlightAnimMatrixIndex) {
					vtx.RGBA[0] = 0xff;
					vtx.RGBA[1] = 0xff;
					vtx.RGBA[2] = 0xff;
					vtx.RGBA[3] = 0xff;
				}

				const uint primReg = gifTag.tag.PRIM;
				const GIFReg::GSPrim primPacked = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

				const uint skip = vtx.XYZSkip.Skip & 0x8000;

				const uint shiftedStripIndex = stripIndex << 16;

				vtx.XYZSkip.Skip |= shiftedStripIndex;

				Renderer::KickVertex(vtx, primPacked, skip, GetDrawBufferData());

				vtxStart += 0x30;
			};
		};

		if ((pCurrentStrip->flags & 4) == 0) {
			bool bCompletedPartial;

			while (bCompletedPartial = partialMeshSectionCount != 0, partialMeshSectionCount = partialMeshSectionCount - 1, bCompletedPartial) {
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

	int RenderStripList(ed_3d_strip* p3dStrip, int stripCount, const bool bAnimate) 
	{
		int maxAnimIndex = 0;
		std::vector<ed_3d_strip*> strips;

		for (int stripIndex = 0; stripIndex < stripCount; stripIndex++) {
			strips.push_back(p3dStrip);
			p3dStrip = (ed_3d_strip*)LOAD_SECTION(p3dStrip->pNext);
		}

		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Found {} strips", strips.size());

		std::reverse(strips.begin(), strips.end());

		for (int stripIndex = 0; stripIndex < strips.size(); stripIndex++) {
			if (gIsolateStripIndex != -1 && gIsolateStripIndex != stripIndex) {
				continue;
			}

			auto& pCurrentStrip = strips[stripIndex];
			RenderStrip(pCurrentStrip, stripIndex, maxAnimIndex, bAnimate);
		}

		return maxAnimIndex;
	}

	void UpdateMaterial(ed_3d_strip* pStrip, ed_hash_code* pMBNK)
	{
		ed_g2d_material* pStripMaterial;

		if (pStrip->materialIndex == -1) {
			pStripMaterial = (ed_g2d_material*)0x0;
		}
		else {
			pStripMaterial = ed3DG2DGetG2DMaterialFromIndex(pMBNK, (int)pStrip->materialIndex);
			if ((pStripMaterial != (ed_g2d_material*)0x0) && ((pStripMaterial->flags & 1) != 0)) {
				return;
			}
		}

		if ((pStripMaterial != (ed_g2d_material*)0x0) && (pStripMaterial->nbLayers != 0)) {
			ed_Chunck* pLAY = LOAD_SECTION_CAST(ed_Chunck*, pStripMaterial->aLayers[0]);
			ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
			ed_g2d_bitmap* pBitmap = (ed_g2d_bitmap*)0x0;
			ed_g2d_bitmap* pOther = (ed_g2d_bitmap*)0x0;

			if (pLayer->bHasTexture != 0) {
				ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
				ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);

				if (pTexture->bHasPalette == 0) {
					ed_hash_code* pTVar5 = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
					if (pTVar5 != (ed_hash_code*)0x0) {
						ed_Chunck* pPA32 = LOAD_SECTION_CAST(ed_Chunck*, pTVar5->pData);
						pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pPA32 + 1);
					}
				}
				else {
					ed_hash_code* pAfterHash = (ed_hash_code*)(pTexture + 1);
					int iVar4 = pAfterHash[(uint)pLayer->paletteId].pData;
					if (iVar4 != 0) {
						ed_hash_code* pHash = (ed_hash_code*)LOAD_SECTION(iVar4);
						ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pHash->pData);
						pOther = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
					}

					ed_hash_code* pPA32Hash = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
					if (pPA32Hash != (ed_hash_code*)0x0) {
						ed_hash_code* pPaletteHashCode = (ed_hash_code*)LOAD_SECTION(iVar4);
						ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);
						pOther = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
					}
				}
			}

			gTextureData = MakeTextureDataFromPacket(pStripMaterial, pOther, pBitmap, 0);
		}
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

				int maxAnimIndex = 0;

				if (p3dStrip) {
					maxAnimIndex = RenderStripList(p3dStrip, pMeshOBJ->body.stripCount, true);
				}

				if (p3dHier) {
					VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();
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

	int gOctreeStrip = 0;

	uint GetOctreeStripCount(const ed_3d_octree* p3DOctree)
	{
		int stripCountArrayEntryIndex = 0;

		MeshData_CDQU* pCDQU = (MeshData_CDQU*)p3DOctree->pCDQU;
		ushort* pStripCounts = pCDQU->aClusterStripCounts;
		if (p3DOctree->field_0x30 == 1.0f) {
			stripCountArrayEntryIndex = 4;
		}
		else {
			stripCountArrayEntryIndex = 0xc;
		}

		return (uint)pStripCounts[stripCountArrayEntryIndex];
	}

	bool UpdateDrawBuffer(const ed_3d_octree* p3DOctree) 
	{
		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Begin");

		MeshData_CDQU* pCDQU = (MeshData_CDQU*)p3DOctree->pCDQU;

		uint stripCount = GetOctreeStripCount(p3DOctree);

		if (stripCount != 0) {
			ed_3d_strip* pStrip = (ed_3d_strip*)LOAD_SECTION(pCDQU->p3DStrip);
			char* pMBNK = (char*)LOAD_SECTION(pCDQU->pMBNK);

			for (; stripCount != 0; stripCount = stripCount - 1) {
				if (stripCount == (gOctreeStrip + 1)) {
					UpdateMaterial(pStrip, (ed_hash_code*)(pMBNK + 0x10));
					int maxAnimIndex;
					RenderStrip(pStrip, 0, maxAnimIndex, false);
				}

				pStrip = (ed_3d_strip*)LOAD_SECTION(pStrip->pNext);
			}
		}

		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer End");

		return GetDrawBufferData().index.tail > 0;
	}

	void ShowHierarchyMenu(ed_3d_hierarchy* pHierarchy)
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

						DebugHelpers::ImGui::TextVector4("Bounding Sphere", pMeshOBJ->body.boundingSphere);

						if (ImGui::CollapsingHeader("Strip", ImGuiTreeNodeFlags_DefaultOpen)) {
							ed_3d_strip* pStrip = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->body.p3DStrip);

							ImGui::Text("Flags: %x", pStrip->flags);
							DebugHelpers::ImGui::TextVector4("Bounding Sphere", pStrip->boundingSphere);
							ImGui::Text("Mesh Count: %d", pStrip->meshCount);

							auto pMaterial = ed3DG2DGetG2DMaterialFromIndex(pTextureInfo, (int)pStrip->materialIndex);

							UpdateMaterial(pStrip, pTextureInfo);

							if (pMaterial) {
								if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
									ImGui::Text("Layer Count?: %d", pMaterial->nbLayers);

									if (ImGui::Button("Open Pre")) {
										gMaterial = pMaterial;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	template<typename DataType>
	void ShowPreviewer(DataType* pData)
	{
		gStripBoneIndexes.clear();

		if (UpdateDrawBuffer(pData))
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
}

bool& DebugMeshViewer::GetUseGlslPipeline()
{
	return gUseGlslPipeline;
}

bool& DebugMeshViewer::GetWireframe()
{
	return gWireframe;
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
}

void ShowOctreeMenu(const ed_3d_octree& octree) 
{
	if (ImGui::CollapsingHeader("Octree", ImGuiTreeNodeFlags_DefaultOpen)) {
		MeshData_CDQU* pCDQU = reinterpret_cast<MeshData_CDQU*>(octree.pCDQU);

		DebugHelpers::ImGui::TextVector4("field_0x0", octree.field_0x0);

		DebugHelpers::ImGui::TextVector4("world Location", octree.worldLocation);

		DebugHelpers::ImGui::TextHash4("CDQU Hash:", pCDQU->header.hash);
		ImGui::Text("CDQU Size: %d (0x%x)", pCDQU->header.size, pCDQU->header.size);

		ImGui::Text("boundingSphereTestResult: %u", octree.boundingSphereTestResult);

		ImGui::Text("field_0x2c: %f", octree.field_0x2c);
		ImGui::Text("field_0x30: %f", octree.field_0x30);

		uint stripCount = DebugMeshViewer::GetOctreeStripCount(&octree);

		ImGui::Text("Strip Count: %u", stripCount);

		if (ImGui::CollapsingHeader("Cluster Hier", ImGuiTreeNodeFlags_DefaultOpen)) {
			uint clusterHierCount = pCDQU->clusterDetails.clusterHierCount;

			if (clusterHierCount != 0) {
				ed_hash_code* puVar17 = (ed_hash_code*)((char*)pCDQU + 0x60);
				bool bVar1;

				while (bVar1 = clusterHierCount != 0, clusterHierCount = clusterHierCount - 1, bVar1) {

					char label[256];
					sprintf(label, "Cluster Hier: %s (0x%x)", puVar17->hash.name, puVar17->hash.number);
					if (ImGui::Selectable(label)) {
					}
					puVar17 = puVar17 + 1;
				}
			}
		}


		ImGui::InputInt("Strip", &DebugMeshViewer::gOctreeStrip);
		DebugMeshViewer::gOctreeStrip = std::clamp<uint>(DebugMeshViewer::gOctreeStrip, 0, stripCount - 1);

		ImGui::End();

		//DebugMeshViewer::ShowPreviewer(&octree);
	}
	else {
		ImGui::End();
	}
}

void DebugMeshViewer::ShowClusterMenu(ed_g3d_manager* pManager)
{
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	ed_3d_octree octreeA;
	ed_3d_octree octreeB;
	MeshData_CSTA* pCSTA;

	ImGui::Begin("Cluster", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	pCSTA = (MeshData_CSTA*)pManager->CSTA;

	DebugHelpers::ImGui::TextHash4("Hash:", pCSTA->chunk.hash);

	if (pCSTA->chunk.hash == 0x414f4443) {
		local_90.xyz = pCSTA->field_0x20;
		octreeB.field_0x0.w = 0.0f;
		octreeB.worldLocation.xyz = pCSTA->worldLocation.xyz;
		octreeB.worldLocation.w = 1.0f;
		local_90.w = 0.0f;
		octreeB.field_0x0.xyz = local_90.xyz;
		edF32Vector4SquareHard(&local_90, &local_90);
		octreeB.boundingSphereTestResult = 2;
		octreeB.field_0x30 = 0.0f;
		local_90.x = local_90.x + local_90.y + local_90.z;
		octreeB.field_0x2c = sqrtf(local_90.x) * 0.5f;
		octreeB.pCDQU = edChunckGetFirst((char*)(pCSTA + 1), (char*)0x0);
		octreeB.pCDQU_End = reinterpret_cast<char*>(octreeA.pCDQU) + octreeA.pCDQU->size;

		ShowOctreeMenu(octreeB);
	}
	else {
		if (pCSTA->chunk.hash == 0x41514443) {
			local_a0.xyz = pCSTA->field_0x20;
			octreeA.field_0x0.w = 0.0f;
			octreeA.worldLocation.xyz = pCSTA->worldLocation.xyz;
			octreeA.worldLocation.w = 1.0f;
			local_a0.w = 0.0f;
			octreeA.field_0x0.xyz = local_a0.xyz;
			edF32Vector4SquareHard(&local_a0, &local_a0);
			octreeA.field_0x30 = 1.0f;
			octreeA.boundingSphereTestResult = 2;
			local_a0.x = local_a0.x + local_a0.y + local_a0.z;
			octreeA.field_0x2c = sqrtf(local_a0.x) * 0.5;
			octreeA.pCDQU = edChunckGetFirst((char*)(pCSTA + 1), (char*)0x0);
			octreeA.pCDQU_End = reinterpret_cast<char*>(octreeA.pCDQU) + octreeA.pCDQU->size;

			ShowOctreeMenu(octreeA);
		}
		else {
			// No valid octree.
			ImGui::End();
		}
	}
}

void DebugMeshViewer::OnFrameBufferCreated(const ImTextureID& image)
{
	gMeshViewerTexture = image;
}