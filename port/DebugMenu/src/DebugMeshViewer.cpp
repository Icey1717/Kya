#include "DebugMeshViewer.h"
#include "DebugMenu.h"
#include "DebugTexture.h"
#include "DrawTrace.h"
#include <optional>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>

#include "ed3D.h"
#include "ed3D/ed3DG2D.h"
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
#include "Texture/TextureCache.h"
#include "gizmo.h"
#include "FileManager3D.h"
#include "Texture.h"
#include "Mesh.h"

#define MESH_PREVIEWER_LOG(level, format, ...) MY_LOG_NAMED_CATEGORY(MeshPreviewer, level, format, ##__VA_ARGS__)

namespace DebugMeshViewer {
	bool gAnimate = false;
	bool gRotate = false;
	bool gWireframe = false;
	bool gUseGizmo = false;
	glm::mat4 gGizmoMatrix;
	ImTextureID gMeshViewerTexture;
	ed_g2d_material* gMaterial;
	Renderer::TextureData gTextureData;

	int gIsolateStripIndex = -1;
	int gHighlightStripIndex = -1;
	int gHighlightAnimMatrixIndex = -1;
	static float gZoom = 1.0f;
	static float gYaw = 0.0f;
	static bool gResetCamera = false;

	void UpdateCamera(const float* minimumXYZ, const float* maximumXYZ, bool selectionChanged)
	{
		const glm::vec3 minimum(minimumXYZ[0], minimumXYZ[1], minimumXYZ[2]);
		const glm::vec3 maximum(maximumXYZ[0], maximumXYZ[1], maximumXYZ[2]);
		if (selectionChanged || gResetCamera) {
			gZoom = 1.0f;
			gYaw = 0.0f;
			gResetCamera = false;
		}

		const auto center = (minimum + maximum) * 0.5f;
		const float radius = std::max(glm::length(maximum - minimum) * 0.5f, 0.001f);
		const float halfFov = glm::radians(30.0f) * 0.5f;
		const float distance = radius / std::sin(halfFov) * 1.1f * gZoom;
		const float yaw = glm::radians(gYaw) + (gRotate ? float(ImGui::GetTime()) * glm::radians(45.0f) : 0.0f);
		const auto position = center + glm::vec3(std::sin(yaw), 0.0f, std::cos(yaw)) * distance;
		const auto model = glm::mat4(1.0f);
		const auto view = glm::lookAt(position, center, glm::vec3(0, 1, 0));

		// Reversed Vulkan depth: near maps to 1, far maps to 0.
		auto proj = glm::perspectiveRH_ZO(halfFov * 2.0f, float(gWidth) / gHeight,
			distance + radius * 2.0f, std::max(radius * 0.001f, distance - radius * 1.5f));
		proj[1][1] *= -1;
		auto& constants = GetVertexConstantBuffer();
		const auto centeredGizmo = glm::translate(glm::mat4(1), center) * gGizmoMatrix * glm::translate(glm::mat4(1), -center);
		memcpy(&constants.model, gUseGizmo ? &centeredGizmo : &model, sizeof(edF32MATRIX4));
		memcpy(&constants.view, &view, sizeof(edF32MATRIX4));
		memcpy(&constants.proj, &proj, sizeof(edF32MATRIX4));
	}

	void FillVertexBufferData(const Renderer::NativeVertexBufferData& pCachedStripBuffer, Renderer::NativeVertexBufferData& newBuffer)
	{
		assert(newBuffer.GetVertexTail() + pCachedStripBuffer.GetVertexTail() <= newBuffer.vertex.maxcount);
		memcpy(newBuffer.vertex.buff + newBuffer.vertex.tail, pCachedStripBuffer.vertex.buff, sizeof(Renderer::GSVertexUnprocessedNormal) * pCachedStripBuffer.GetVertexTail());
		newBuffer.vertex.tail += pCachedStripBuffer.GetVertexTail();

		assert(newBuffer.GetIndexTail() + pCachedStripBuffer.GetIndexTail() <= newBuffer.index.maxcount);
		memcpy(newBuffer.index.buff + newBuffer.index.tail, pCachedStripBuffer.index.buff, sizeof(uint16_t) * pCachedStripBuffer.GetIndexTail());
		newBuffer.index.tail += pCachedStripBuffer.GetIndexTail();
	}

	void RenderStrip(ed_3d_strip* pCurrentStrip, const int stripIndex, int& maxAnimIndex, const bool bAnimate, Renderer::NativeVertexBufferData& vertexBufferData, int textureLayer = 0)
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

		auto* pMesh = Renderer::Kya::GetMeshLibrary().FindStrip(pCurrentStrip);

		if (auto* mesh = pMesh->GetSimpleMesh(textureLayer)) FillVertexBufferData(mesh->GetVertexBufferData(), vertexBufferData);
	}


	bool UpdateDrawBuffer(ed_3d_strip* pStrip, Renderer::NativeVertexBufferData& vertexBufferData, int textureLayer = 0)
	{
		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer Begin");

		int maxAnimIndex;
		RenderStrip(pStrip, 0, maxAnimIndex, false, vertexBufferData, textureLayer);

		VertexConstantBuffer& vertexConstantBuffer = GetVertexConstantBuffer();

		for (int i = 0; i < gMaxAnimMatrices; i++) {
			edF32Matrix4SetIdentityHard(&vertexConstantBuffer.animMatrices[i]);
		}

		MESH_PREVIEWER_LOG(LogLevel::Verbose, "UpdateDrawBuffer End");

		return vertexBufferData.index.tail > 0;
	}

	void ShowPreviewerWindow(bool* pOpen = nullptr)
	{
		if (GetPreviewerDrawCommandCount() > 0) {
			ImGui::Begin("Mesh Previewer", pOpen, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Checkbox("Animate", &gAnimate);
			ImGui::SameLine();
			ImGui::Checkbox("Rotate", &gRotate);
			ImGui::SameLine();
			ImGui::Checkbox("Wireframe", &GetWireframe());

			if (ImGui::Button("Frame mesh")) {
				gResetCamera = true; 
			}

			ImGui::SliderFloat("Zoom", &gZoom, 0.5f, 5.0f);
			ImGui::SliderFloat("View angle", &gYaw, -180.0f, 180.0f, "%.0f degrees");

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
			ImGui::SameLine();
			static std::string exportResult;
			if (ImGui::Button("Export OBJ")) {
				exportResult = ExportPreviewerMeshes();
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Export queued preview meshes in raw object coordinates, without animation, gizmo, or camera transforms. Includes mesh names and bounds.");
			}
			if (!exportResult.empty()) {
				ImGui::TextWrapped("%s", exportResult.c_str());
			}

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

	struct DrawSelection {
		Renderer::DrawTrace::Source source;
		Renderer::DrawTrace::Submission submission;
		Renderer::DrawTrace::Draw draw;
	};
	static std::optional<DrawSelection> gDrawSelection;
	static bool gFocusDraw = false;

	bool OpenDraw(const Renderer::DrawTrace::Source& source, const Renderer::DrawTrace::Submission& submission,
		const Renderer::DrawTrace::Draw& draw)
	{
		if (source.kind != Renderer::DrawTrace::Kind::Strip || !Renderer::DrawTrace::IsSourceLive(source) ||
			!Debug::Texture::FindDrawTexture(source, draw)) return false;
		gDrawSelection = DrawSelection{ source, submission, draw };
		gFocusDraw = true;
		gIsolateStripIndex = -1;
		return true;
	}

	static void UpdateDrawPreview()
	{
		if (!gDrawSelection) return;
		const auto& selection = *gDrawSelection;
		Renderer::DrawTrace::Source currentSource;
		if (!Renderer::DrawTrace::ResolveSource(selection.source, currentSource)) { gDrawSelection.reset(); return; }
		auto* texture = Debug::Texture::FindDrawTexture(selection.source, selection.draw);
		if (!texture) { gDrawSelection.reset(); return; }
		// The lifetime check above precedes the cache lookup and any source access.
		auto* strip = Renderer::Kya::GetMeshLibrary().FindStrip(reinterpret_cast<const ed_3d_strip*>(currentSource.object));
		auto* mesh = strip ? strip->GetSimpleMesh(selection.submission.layer) : nullptr;
		if (!mesh || Renderer::DrawTrace::AssetKey(mesh->GetName().c_str()) != selection.submission.assetKey) {
			gDrawSelection.reset(); return;
		}
		auto& vertices = AddPreviewerDrawCommand(texture, mesh);
		SetPreviewerDrawState(selection.submission, selection.draw);
		UpdateDrawBuffer(strip->pStrip, vertices, selection.submission.layer);
		bool open = true;
		if (gFocusDraw) { ImGui::SetNextWindowFocus(); gFocusDraw = false; }
		ShowPreviewerWindow(&open);
		if (!open) gDrawSelection.reset();
	}
	static Debug::UpdateRegisterer drawPreviewUpdate(UpdateDrawPreview);
} // DebugMeshViewer


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
	if (pStrip->materialIndex == -1) {
		return;
	}

	auto* material = ed3DG2DGetG2DMaterialFromIndex(pMBNK, pStrip->materialIndex);
	if (!material || (material->flags & 1) != 0) {
		return;
	}

	const auto* cachedMaterial = Renderer::Kya::GetTextureLibrary().FindMaterial(material);
	const auto* strip = Renderer::Kya::GetMeshLibrary().FindStrip(pStrip);
	if (!cachedMaterial || !strip) {
		return;
	}

	for (int layer = 0; layer < static_cast<int>(cachedMaterial->layers.size()); ++layer) {
		auto* texture = cachedMaterial->FindRenderTextureFromLayer(layer);
		auto* mesh = strip->GetSimpleMesh(layer);
		if (!texture || !mesh) {
			continue;
		}

		auto& vertices = AddPreviewerDrawCommand(texture, mesh);
		UpdateDrawBuffer(pStrip, vertices, layer);
	}
}

void DebugMeshViewer::ShowPreviewer()
{
	ShowPreviewerWindow();
}
