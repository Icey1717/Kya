#include "TextureUpdate.h"

#include <cassert>
#include <mutex>

#include "Texture/TextureCache.h"
#include "VulkanRenderer.h"

namespace
{
	struct PendingTextureUpdate
	{
		PS2::GSSimpleTexture* pTexture;
		uint32_t newWidth;
		uint32_t newHeight;
		std::vector<uint8_t> pixels;
	};

	std::vector<PendingTextureUpdate> gPendingTextureUpdates;
	std::mutex gPendingTextureUpdatesMutex;
}

void Renderer::Native::RequestTextureUpdate(SimpleTexture* pTexture, uint32_t newWidth, uint32_t newHeight, std::vector<uint8_t> pixels)
{
	assert(pTexture && pTexture->GetRenderer());
	std::lock_guard<std::mutex> lock(gPendingTextureUpdatesMutex);
	gPendingTextureUpdates.push_back({ pTexture->GetRenderer(), newWidth, newHeight, std::move(pixels) });
}

void Renderer::Native::TurnTextureWhite(SimpleTexture* pTexture)
{
	const uint32_t whitePixel = 0xFFFFFFFF; // RGBA8 white

	const uint32_t width = pTexture->GetRenderer()->width;
	const uint32_t height = pTexture->GetRenderer()->height;

	std::vector<uint8_t> whiteData(width * height * 4); // RGBA8

	for (size_t i = 0; i < width * height; i++)
	{
		*reinterpret_cast<uint32_t*>(whiteData.data() + (i * 4)) = whitePixel;
	}

	RequestTextureUpdate(pTexture, width, height, std::move(whiteData));
}

void Renderer::Native::DrainPendingTextureUpdates()
{
	std::vector<PendingTextureUpdate> updates;
	{
		std::lock_guard<std::mutex> lock(gPendingTextureUpdatesMutex);
		updates = std::move(gPendingTextureUpdates);
	}

	if (updates.empty()) 
		return;

	// Ensure no frame is still referencing any of these textures before
	// DestroyImageResources() is called inside Resize().
	vkDeviceWaitIdle(GetDevice());

	for (auto& update : updates)
	{
		update.pTexture->Resize(update.newWidth, update.newHeight, static_cast<int>(update.pixels.size()), update.pixels.data());
	}
}
