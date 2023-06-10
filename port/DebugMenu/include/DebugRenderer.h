#pragma once

typedef void* ImTextureID;

struct FrameBuffer;

namespace PS2 {
	struct GSTexEntry;
	struct GSTexValue;
}

namespace DebugMenu {
	void SetupRenderer();
	ImTextureID AddTexture(const PS2::GSTexEntry& texEntry);
	ImTextureID AddTexture(const PS2::GSTexValue& texValue);
	ImTextureID AddFrameBuffer(const FrameBuffer& frameBuffer);
}