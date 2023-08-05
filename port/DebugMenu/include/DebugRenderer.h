#pragma once

typedef void* ImTextureID;

struct FrameBuffer;

namespace PS2 {
	struct GSTexEntry;
	struct GSTexValue;
	struct GSTexImage;
}

namespace DebugMenu {
	void SetupRenderer();
	ImTextureID AddTexture(const PS2::GSTexImage& texValue);
	ImTextureID AddFrameBuffer(const FrameBuffer& frameBuffer);
}