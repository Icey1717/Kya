#pragma once

#include "renderer.h"

typedef void* ImTextureID;

namespace PS2 {
	struct FrameBuffer;
}
struct edDList_material;
struct ed_g2d_material;

namespace PS2 {
	struct GSTexEntry;
	struct GSTexValue;
	struct GSTexImage;
}

namespace DebugMenu {
	void SetupRenderer();
	ImTextureID AddTexture(const PS2::GSTexImage& texValue);
	ImTextureID AddFrameBuffer(const PS2::FrameBuffer& frameBuffer);

	ImTextureID AddNativeFrameBuffer();

	Renderer::TextureData LoadTextureData(edDList_material* pMaterial);
	Renderer::TextureData LoadTextureData(ed_g2d_material* pMaterial);
}