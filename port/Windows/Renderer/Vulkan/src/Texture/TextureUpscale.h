#pragma once

#include "renderer.h"

namespace Renderer::Native
{
	// ncnn / Real-ESRGAN backend: load a .param + .bin model pair.
	// No-op when built without ENABLE_UPSCALING.
	void InitUpscaler(const char* paramPath, const char* binPath, int scaleFactor = 4);

	// ONNX Runtime backend: load a .onnx model file directly.
	// No-op when built without ENABLE_UPSCALING_ONNX.
	// ONNX is preferred over ncnn when both backends are ready.
	void InitUpscalerONNX(const char* modelPath, int scaleFactor = 4);

	// Returns true when at least one backend has been successfully initialised.
	bool IsUpscalerReady();

	// Submits a background upscale job for pTexture.
	// ONNX is used if ready, otherwise ncnn. The upscaled result is applied
	// via RequestTextureUpdate at the next frame boundary.
	// A second call for the same in-flight texture is silently ignored.
	void UpscaleTexture(SimpleTexture* pTexture);
}