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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "DebugMaterialPreviewer.h"
#include "edDlist.h"
#include "TextureCache.h"
#include "gizmo.h"
#include "FileManager3D.h"
#include "Texture.h"
#include "Mesh.h"

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

		uint incPacketSize = ed3DFlushStripGetIncPacket(pCurrentStrip, false, false);
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

		const float cameraSpeed = 30.f * deltaTime; // adjust accordingly
		if (ImGui::IsKeyPressed(ImGuiKey_Comma)) {
			cameraPos += cameraSpeed * cameraFront;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_O)) {
			cameraPos -= cameraSpeed * cameraFront;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_A)) {
			cameraPos -= cameraSpeed * cameraUp;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_E)) {
			cameraPos += cameraSpeed * cameraUp;
		}

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

				Renderer::GSVertexUnprocessed vtx;
				memcpy(&vtx.STQ, vtxStart, sizeof(vtx.STQ));
				memcpy(&vtx.RGBA, vtxStart + 0x10, sizeof(vtx.RGBA));
				memcpy(&vtx.XYZFlags, vtxStart + 0x20, sizeof(vtx.XYZFlags));

				if (drawMode == 1) {
					vtx.XYZFlags.fXYZ[0] = int12_to_float(vtx.XYZFlags.iXYZ[0]);
					vtx.XYZFlags.fXYZ[1] = int12_to_float(vtx.XYZFlags.iXYZ[1]);
					vtx.XYZFlags.fXYZ[2] = int12_to_float(vtx.XYZFlags.iXYZ[2]);
				}

				const uint vtxAnimMatrix = ((vtx.XYZFlags.flags & 0x7ff) - 0x3dc) / 4;

				if (gHighlightStripIndex == stripIndex || vtxAnimMatrix == gHighlightAnimMatrixIndex) {
					vtx.RGBA[0] = 0xff;
					vtx.RGBA[1] = 0xff;
					vtx.RGBA[2] = 0xff;
					vtx.RGBA[3] = 0xff;
				}

				const uint primReg = gifTag.tag.PRIM;
				const GIFReg::GSPrim primPacked = *reinterpret_cast<const GIFReg::GSPrim*>(&primReg);

				const uint skip = vtx.XYZFlags.flags & 0x8000;

				const uint shiftedStripIndex = stripIndex << 16;
				//vtx.XYZSkip.Skip |= shiftedStripIndex;
				vtx.XYZFlags.flags |= (drawMode << 16);

				Renderer::KickVertex(vtx, primPacked, skip, vertexBufferData);

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
		const Renderer::Kya::G3D::Strip* pMeshStrip = Renderer::Kya::GetMeshLibrary().FindStrip(pStrip);

		assert(pMeshStrip);

		if (pMaterial) {
			// Check layers and textures to make sure they are size 1
			assert(pMaterial->layers.size() > 0);
			assert(pMaterial->layers.begin()->textures.size() > 0);
			return &AddPreviewerDrawCommand(pMaterial->layers.begin()->textures.begin()->pSimpleTexture, pMeshStrip->pSimpleMesh);
		}

		return nullptr;
	}

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
			if (ImGui::BeginCombo("Isolate Strip", gIsolateStripIndex == -1 ? "None" : std::to_string(gIsolateStripIndex).c_str())) {
				for (int i = -1; i < gMaxStripIndex; i++) {
					if (ImGui::Selectable(std::to_string(i).c_str())) {
						gIsolateStripIndex = i;
					}
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::Text("Isolate Anim:");
			ImGui::SameLine();
			if (ImGui::ArrowButton("IsolateAnimLeft", ImGuiDir_Left)) {
				gHighlightAnimMatrixIndex--;
			}
			ImGui::SameLine();
			ImGui::Text(std::to_string(gHighlightAnimMatrixIndex).c_str());
			ImGui::SameLine();
			if (ImGui::ArrowButton("IsolateAnimRight", ImGuiDir_Right)) {
				gHighlightAnimMatrixIndex++;
			}
			

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
