#pragma once
#include "renderer.h"

namespace Renderer::Native
{
	enum class TextureUpscaleModel { UltraSharp, SPAN };
	struct TextureUpscaleStatus {
		bool ready = false, loading = false;
		std::string backend, error;
	};
	struct TextureUpscaleProgress {
		size_t pending = 0, completed = 0, failed = 0;
		size_t cacheHits = 0, cacheMisses = 0, cacheErrors = 0;
		std::string lastResult;
	};
	void InitTextureUpscalers();
	void ShutdownTextureUpscalers();
	void InitUpscalerONNX(const char* modelPath, int scaleFactor = 4, TextureUpscaleModel model = TextureUpscaleModel::UltraSharp);
	void InitUpscaler(const char* paramPath, const char* binPath, int scaleFactor = 4);
	TextureUpscaleStatus GetTextureUpscaleStatus(TextureUpscaleModel model);
	TextureUpscaleProgress GetTextureUpscaleProgress();
	void SetTextureUpscaleModel(TextureUpscaleModel model);
	TextureUpscaleModel GetTextureUpscaleModel();
	bool IsUpscalerReady();
	void SetTextureUpscaleCacheLoading(bool enabled);
	// Cache-only request: missing entries leave the texture untouched.
	bool LoadTextureUpscaleFromCache(SimpleTexture* pTexture);
	// Call on the game/UI thread. Copies original pixels; the single worker
	// never dereferences SimpleTexture. Revert/unload invalidates older results.
	void UpscaleTexture(SimpleTexture* pTexture);
}
