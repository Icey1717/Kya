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
#include "Texture.h"

#define MESH_PREVIEWER_LOG(level, format, ...) MY_LOG_CATEGORY("Mesh Previewer", level, format, ##__VA_ARGS__)

namespace DebugMeshViewer {
	bool gAnimate = false;
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

	void RenderStrip(ed_3d_strip* pCurrentStrip, const int stripIndex, int& maxAnimIndex, const bool bAnimate, PreviewerVertexBufferData& vertexBufferData)
	{
		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Strip: 0x{:x}", (uintptr_t)pCurrentStrip);

		VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();

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

				animMatrixIndex++;
			}

			MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Final anim index: {}", animMatrixIndex);
		}

		const uint stripFlags = pCurrentStrip->flags;

		auto AddVertices = [stripIndex, stripFlags, &vertexBufferData]() {
			char* vtxStart = VU1Emu::GetVertexDataStart();

			int* pFlag = reinterpret_cast<int*>(vtxStart - 0x10);

			Gif_Tag gifTag;
			gifTag.setTag((u8*)vtxStart, true);

			vtxStart += 0x10;

			for (int i = 0; i < gifTag.nLoop; i++) {
				const int addr = VU1Emu::GetExecVuCodeAddr();

				int drawMode = 0;

				if ((stripFlags & 0x400) != 0) {
					drawMode = 1;
				}

				//if (addr == 0x3 /*_$DrawingStart_XYZ32 (0x0018)*/) {
				//	//VU_VTX_TRACE_LOG("_$DrawingStart_XYZ32");
				//	drawMode = 1;
				//}
				//else if (addr == 0xfc /*_$DrawingBones_Rigid (0x07e0)*/) {
				//	//VU_VTX_TRACE_LOG("_$DrawingBones_Rigid");
				//	drawMode = 2;
				//}
				//else if (addr == 0x156 /*_$DrawingBones_Rigid_noNormal (0x0ab0)*/) {
				//	//VU_VTX_TRACE_LOG("_$DrawingBones_Rigid_noNormal");
				//	drawMode = 3;
				//}
				//else if (addr == 0x19) {
				//	//_$DrawingStart_XYZ32_Normal();
				//	drawMode = 4;
				//}
				//else {
				//	IMPLEMENTATION_GUARD();
				//}

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
				//vtx.XYZSkip.Skip |= shiftedStripIndex;
				vtx.XYZSkip.Skip |= (drawMode << 16);

				Renderer::KickVertex(vtx, primPacked, skip, vertexBufferData);

				vtxStart += 0x30;
			};
		};

		if ((pCurrentStrip->flags & 4) == 0) {
			bool bCompletedPartial;

			while (bCompletedPartial = partialMeshSectionCount != 0, partialMeshSectionCount = partialMeshSectionCount - 1, bCompletedPartial) {
				VU1Emu::ProcessVifList((edpkt_data*)pVifList, true);
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
		//std::vector<ed_3d_strip*> strips;
		//
		//for (int stripIndex = 0; stripIndex < stripCount; stripIndex++) {
		//	strips.push_back(p3dStrip);
		//	p3dStrip = (ed_3d_strip*)LOAD_SECTION(p3dStrip->pNext);
		//}
		//
		//MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Found {} strips", strips.size());
		//
		//std::reverse(strips.begin(), strips.end());
		//
		//for (int stripIndex = 0; stripIndex < strips.size(); stripIndex++) {
		//	if (gIsolateStripIndex != -1 && gIsolateStripIndex != stripIndex) {
		//		continue;
		//	}
		//
		//	auto& pCurrentStrip = strips[stripIndex];
		//	RenderStrip(pCurrentStrip, stripIndex, maxAnimIndex, bAnimate);
		//}

		return maxAnimIndex;
	}

	PreviewerVertexBufferData* UpdateMaterial(ed_3d_strip* pStrip, ed_hash_code* pMBNK)
	{
		ed_g2d_material* pStripMaterial;

		if (pStrip->materialIndex == -1) {
			pStripMaterial = (ed_g2d_material*)0x0;
		}
		else {
			pStripMaterial = ed3DG2DGetG2DMaterialFromIndex(pMBNK, pStrip->materialIndex);
			if ((pStripMaterial != (ed_g2d_material*)0x0) && ((pStripMaterial->flags & 1) != 0)) {
				return nullptr;
			}
		}

		const Renderer::Kya::G2D::Material* pMaterial = Renderer::Kya::GetTextureLibrary().FindMaterial(pStripMaterial);

		if (pMaterial) {
			// Check layers and textures to make sure they are size 1
			assert(pMaterial->layers.size() == 1);
			assert(pMaterial->layers.begin()->textures.size() == 1);
			return &AddPreviewerDrawCommand(pMaterial->layers.begin()->textures.begin()->pSimpleTexture);
		}

		return nullptr;
	}

	// Returns whether we prepared any indices.
	//bool UpdateDrawBuffer(ed_3d_hierarchy* p3dHier, PreviewerVertexBufferData& vertexBufferData) {
	//	MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Begin");
	//
	//	if (p3dHier) {
	//		ed3DLod* pLod = ed3DChooseGoodLOD(p3dHier);
	//		ed_hash_code* pLodHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
	//
	//		if (pLodHash) {
	//			ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pLodHash->pData);
	//			ed_g3d_object* pMeshOBJ = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);
	//
	//			ed_3d_strip* p3dStrip = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->p3DStrip);
	//
	//			int maxAnimIndex = 0;
	//
	//			if (p3dStrip) {
	//				maxAnimIndex = RenderStripList(p3dStrip, pMeshOBJ->stripCount, true);
	//			}
	//
	//			if (p3dHier) {
	//				VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();
	//				assert(maxAnimIndex < gMaxAnimMatrices);
	//
	//				for (int i = 0; i <= maxAnimIndex; i++) {
	//					edF32Matrix4SetIdentityHard(&vertexConstantBuffer.animMatrices[i]);
	//
	//					if (p3dHier->pAnimMatrix && gAnimate) {
	//						vertexConstantBuffer.animMatrices[i] = p3dHier->pAnimMatrix[i];
	//					}
	//				}
	//			}
	//		}
	//	}
	//
	//	MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer End");
	//
	//	return GetDrawBufferData().index.tail > 0;
	//}

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

	//bool UpdateDrawBuffer(const ed_3d_octree* p3DOctree, PreviewerVertexBufferData& vertexBufferData)
	//{
	//	MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Begin");
	//
	//	MeshData_CDQU* pCDQU = (MeshData_CDQU*)p3DOctree->pCDQU;
	//
	//	uint stripCount = GetOctreeStripCount(p3DOctree);
	//
	//	MeshData_PSX2* pPSX2 = reinterpret_cast<MeshData_PSX2*>(pCDQU + 1);
	//
	//	if (stripCount != 0) {
	//		ed_3d_strip* pStrip = (ed_3d_strip*)LOAD_SECTION(pPSX2->p3DStrip);
	//		char* pMBNK = (char*)LOAD_SECTION(pPSX2->pMBNK);
	//
	//		for (; stripCount != 0; stripCount = stripCount - 1) {
	//			if (stripCount == (gOctreeStrip + 1)) {
	//				auto& vertexBuffer = UpdateMaterial(pStrip, (ed_hash_code*)(pMBNK + 0x10));
	//				int maxAnimIndex;
	//				RenderStrip(pStrip, 0, maxAnimIndex, false, vertexBuffer);
	//			}
	//
	//			pStrip = (ed_3d_strip*)LOAD_SECTION(pStrip->pNext);
	//		}
	//	}
	//
	//	MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer End");
	//
	//	return vertexBufferData.index.tail > 0;
	//}

	bool UpdateDrawBuffer(ed_3d_strip* pStrip, PreviewerVertexBufferData& vertexBufferData)
	{
		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Begin");

		int maxAnimIndex;
		RenderStrip(pStrip, 0, maxAnimIndex, false, vertexBufferData);

		VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();

		for (int i = 0; i < gMaxAnimMatrices; i++) {
			edF32Matrix4SetIdentityHard(&vertexConstantBuffer.animMatrices[i]);
		}

		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer End");

		return vertexBufferData.index.tail > 0;
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

						ed_hash_code* pLodHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
						ImGui::Text("Name: %s (0x%llx)", pLodHash->hash.name);

						ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pLodHash->pData);
						ed_g3d_object* pMeshOBJ = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);

						ImGui::Text("Strip Count: %d", pMeshOBJ->stripCount);

						ImGui::InputInt("Strip Isolate", &gIsolateStripIndex);
						gIsolateStripIndex = std::clamp<int>(gIsolateStripIndex, -1, pMeshOBJ->stripCount - 1);

						ImGui::InputInt("Strip Highlight", &gHighlightStripIndex);
						gHighlightStripIndex = std::clamp<int>(gHighlightStripIndex, -1, pMeshOBJ->stripCount - 1);

						ImGui::InputInt("Anim Matrix Highlight", &gHighlightAnimMatrixIndex);

						DebugHelpers::ImGui::TextVector4("Bounding Sphere", pMeshOBJ->boundingSphere);

						if (ImGui::CollapsingHeader("Strip", ImGuiTreeNodeFlags_DefaultOpen)) {
							ed_3d_strip* pStrip = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->p3DStrip);

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

	void ShowPreviewerWindow()
	{
		if (GetPreviewerDrawCommandCount() > 0) {
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
		}
	}

	template<typename DataType>
	void AddPreviewerData(DataType* pData)
	{
		UpdateDrawBuffer(pData);
	}

	template<typename DataType>
	void ShowPreviewer(DataType* pData)
	{
		//UpdateDrawBuffer(pData);
		ShowPreviewerWindow();
	}
} // DebugMeshViewer


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
		//MeshData_CDQU* pCDQU = reinterpret_cast<MeshData_CDQU*>(octree.pCDQU);
		//
		//DebugHelpers::ImGui::TextVector4("field_0x0", octree.field_0x0);
		//
		//DebugHelpers::ImGui::TextVector4("world Location", octree.worldLocation);
		//
		//DebugHelpers::ImGui::TextHash4("CDQU Hash:", pCDQU->header.hash);
		//ImGui::Text("CDQU Size: %d (0x%x)", pCDQU->header.size, pCDQU->header.size);
		//
		//ImGui::Text("boundingSphereTestResult: %u", octree.boundingSphereTestResult);
		//
		//ImGui::Text("field_0x2c: %f", octree.field_0x2c);
		//ImGui::Text("field_0x30: %f", octree.field_0x30);
		//
		//uint stripCount = DebugMeshViewer::GetOctreeStripCount(&octree);
		//
		//ImGui::Text("Strip Count: %u", stripCount);
		//
		//if (ImGui::CollapsingHeader("Cluster Hier", ImGuiTreeNodeFlags_DefaultOpen)) {
		//	uint clusterHierCount = pCDQU->clusterDetails.clusterHierCount;
		//
		//	if (clusterHierCount != 0) {
		//		ed_hash_code* puVar17 = (ed_hash_code*)((char*)pCDQU + 0x60);
		//		bool bVar1;
		//
		//		while (bVar1 = clusterHierCount != 0, clusterHierCount = clusterHierCount - 1, bVar1) {
		//
		//			char label[256];
		//			sprintf(label, "Cluster Hier: %s (0x%x)", puVar17->hash.name, puVar17->hash.number);
		//			if (ImGui::Selectable(label)) {
		//			}
		//			puVar17 = puVar17 + 1;
		//		}
		//	}
		//}
		//
		//
		//ImGui::InputInt("Strip", &DebugMeshViewer::gOctreeStrip);
		//DebugMeshViewer::gOctreeStrip = std::clamp<uint>(DebugMeshViewer::gOctreeStrip, 0, stripCount - 1);
		//
		//ImGui::End();
		//
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
		octreeB.pCDQU = edChunckGetFirst(pCSTA + 1, (char*)0x0);
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
			octreeA.pCDQU = edChunckGetFirst(pCSTA + 1, (char*)0x0);
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

void DebugMeshViewer::AddPreviewerStrip(ed_3d_strip* pStrip, ed_hash_code* pMBNK)
{
	auto* pVertexBufferData = UpdateMaterial(pStrip, pMBNK);

	if (pVertexBufferData) {
		UpdateDrawBuffer(pStrip, *pVertexBufferData);
	}
}

void DebugMeshViewer::ShowPreviewer()
{
	ShowPreviewerWindow();
}
