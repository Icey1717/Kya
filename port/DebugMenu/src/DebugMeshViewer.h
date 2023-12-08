#pragma once
#include "renderer.h"
#include "Types.h"
#include "imgui.h"

struct ed_3D_Scene;
struct edNODE;
struct ed_3d_hierarchy;

namespace DebugMeshViewer {

	constexpr int gWidth = 0x400;
	constexpr int gHeight = 0x300;

	struct VertexConstantBuffer {
		edF32MATRIX4 model;
		edF32MATRIX4 view;
		edF32MATRIX4 proj;
		edF32MATRIX4 animMatrices[0x20][0x18];
	};

	// Implented via renderer backend.
	PS2::DrawBufferData<Renderer::GSVertexUnprocessed, uint16_t>& GetDrawBufferData();
	VertexConstantBuffer& GetVertexConstantBuffer();

	bool& GetUseGlslPipeline();

	void ShowHierarchyMenu(ed_3d_hierarchy* pHierarchy);
	void ShowPreviewer(ed_3d_hierarchy* pHierarchy);
	void ShowNodeMenu(edNODE* pNode);

	void OnFrameBufferCreated(const ImTextureID& image);
}