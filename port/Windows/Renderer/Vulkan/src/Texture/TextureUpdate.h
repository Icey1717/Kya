#pragma once

#include <cstdint>
#include <vector>

#include "renderer.h"

/*
	RequestTextureUpdate()  →  push to gPendingTextureUpdates (mutex-protected)
										↓  (next frame boundary)
	DrainPendingTextureUpdates()  →   swap queue out, release mutex
								   →  for each update:
										  GSSimpleTexture::Resize()
											├─ DestroyImageResources()
											├─ CreateResources()     (new VkImage/View)
											├─ UploadData()          (staging → GPU)
											└─ RefreshDescriptors()  (re-point all descriptor sets)
*/

namespace Renderer::Native
{
	// Queue a texture resize + data upload to be applied at the next safe frame boundary
	// (after vkWaitForFences, before any rendering). Safe to call from any thread at any time.
	// Pixel data must be RGBA8, row-major. It is copied into the queue immediately.
	void RequestTextureUpdate(SimpleTexture* pTexture, uint32_t newWidth, uint32_t newHeight, std::vector<uint8_t> pixels);

	void TurnTextureWhite(SimpleTexture* pTexture);

	// Apply all queued texture updates. Called once per frame from WaitUntilReady().
	void DrainPendingTextureUpdates();
}
