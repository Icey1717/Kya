#include "TextureUpscale.h"
#include "TextureUpdate.h"
#include "TextureCache.h"

void Renderer::Native::UpscaleTexture(SimpleTexture* pTexture)
{
	// This is a very naive nearest-neighbor upscale. It just duplicates each pixel into a 2x2 block.
	// A more sophisticated upscale (e.g. bilinear filtering) could be implemented here if desired.
	assert(pTexture && pTexture->GetRenderer());
	const uint32_t oldWidth = pTexture->GetRenderer()->width;
	const uint32_t oldHeight = pTexture->GetRenderer()->height;
	const uint32_t newWidth = oldWidth * 2;
	const uint32_t newHeight = oldHeight * 2;
	std::vector<uint8_t> newPixels(newWidth * newHeight * 4); // RGBA8
	uint8_t* pOldPixels = reinterpret_cast<uint8_t*>(pTexture->GetRenderer()->imageData.pImage);
	uint8_t* pNewPixels = newPixels.data();
	for (uint32_t y = 0; y < oldHeight; y++) {
		for (uint32_t x = 0; x < oldWidth; x++) {
			uint8_t* pOldPixel = pOldPixels + ((y * oldWidth + x) * 4);
			uint8_t* pNewPixelTopLeft = pNewPixels + (((y * 2) * newWidth + (x * 2)) * 4);
			uint8_t* pNewPixelTopRight = pNewPixelTopLeft + 4;
			uint8_t* pNewPixelBottomLeft = pNewPixelTopLeft + (newWidth * 4);
			uint8_t* pNewPixelBottomRight = pNewPixelBottomLeft + 4;
			// Copy the old pixel to the four new pixels
			memcpy(pNewPixelTopLeft, pOldPixel, 4);
			memcpy(pNewPixelTopRight, pOldPixel, 4);
			memcpy(pNewPixelBottomLeft, pOldPixel, 4);
			memcpy(pNewPixelBottomRight, pOldPixel, 4);
		}
	}

	RequestTextureUpdate(pTexture, newWidth, newHeight, std::move(newPixels));
}
