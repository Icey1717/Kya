#include "TextureUpdate.h"

#include <cassert>
#include <mutex>
#include <stdexcept>

#include "Texture/TextureCache.h"
#include "VulkanRenderer.h"

namespace
{
	struct PendingTextureUpdate
	{
		Renderer::Native::TextureUpdateTarget target;
		uint32_t newWidth;
		uint32_t newHeight;
		std::vector<uint8_t> pixels;
	};

	std::vector<PendingTextureUpdate> gPendingTextureUpdates;
	std::mutex gPendingTextureUpdatesMutex;
}

void Renderer::Native::RequestTextureUpdate(SimpleTexture* pTexture, uint32_t newWidth, uint32_t newHeight, std::vector<uint8_t> pixels)
{
	RequestTextureUpdate(CaptureTextureUpdateTarget(pTexture), newWidth, newHeight, std::move(pixels));
}

bool Renderer::Native::TextureUpdateTarget::IsCurrent() const
{
	const auto live = state.lock();
	return pTexture && live && live->revision.load() == revision;
}

Renderer::Native::TextureUpdateTarget Renderer::Native::CaptureTextureUpdateTarget(SimpleTexture* pTexture)
{
	assert(pTexture && pTexture->GetRenderer());
	const auto& state = pTexture->GetUpdateState();
	return { pTexture->GetRenderer(), state, ++state->revision };
}

void Renderer::Native::RequestTextureUpdate(const TextureUpdateTarget& target, uint32_t newWidth, uint32_t newHeight, std::vector<uint8_t> pixels)
{
	if (!target.IsCurrent()) return;
	if (!newWidth || !newHeight || uint64_t(newWidth) * newHeight * 4 != pixels.size())
		throw std::invalid_argument("Texture update must contain tightly packed RGBA8 pixels");
	std::lock_guard<std::mutex> lock(gPendingTextureUpdatesMutex);
	gPendingTextureUpdates.push_back({ target, newWidth, newHeight, std::move(pixels) });
}

void Renderer::Native::RevertTexture(SimpleTexture* pTexture)
{
	assert(pTexture && pTexture->GetRenderer());
	PS2::GSSimpleTexture* pRenderer = pTexture->GetRenderer();

	const TextureUpload::UploadBuffer& originalData = pRenderer->GetUploadBuffer();

	const uint32_t width = pRenderer->imageData.canvasWidth;
	const uint32_t height = pRenderer->imageData.canvasHeight;
	std::vector<uint8_t> pixels(size_t(width) * height * 4);
	for (uint32_t y = 0; y < height; ++y)
		memcpy(pixels.data() + size_t(y) * width * 4, originalData.Get() + size_t(y) * originalData.Width() * 4, size_t(width) * 4);
	RequestTextureUpdate(pTexture, width, height, std::move(pixels));
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
		if (update.target.IsCurrent())
			update.target.pTexture->Resize(update.newWidth, update.newHeight, static_cast<int>(update.pixels.size()), update.pixels.data());
	}

	vkDeviceWaitIdle(GetDevice());
}
