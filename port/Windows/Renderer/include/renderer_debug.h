#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#include "renderer.h"

namespace Renderer
{
	using RenderDelegate = Multidelegate<const VkFramebuffer&, const VkExtent2D&, CommandBufferList&>;

	RenderDelegate& GetRenderDelegate();
	Multidelegate<>& GetCleanupDelegate();

	bool& GetForceAnimMatrixIdentity();
	bool& GetUseComplexBlending();
	void ResetRenderer();
}

namespace Renderer::Native
{
	enum class AntiAliasingMode : uint32_t;

	void SetShadowResolutionScale(uint32_t scale);
	uint32_t GetShadowResolutionScale();

	void SetFogEnabled(bool enabled);

	void SetAntiAliasingMode(AntiAliasingMode mode);
	void SetFXAAQualityMultiplier(uint32_t multiplier);
	void SetFullResolutionPS2AACapture(bool enabled);

	void SetFlareOcclusionEnabled(bool enabled);

	bool HasShadowTarget();
	VkSampler GetShadowSampler();
	VkImageView GetShadowMaskImageView();
	VkImageView GetShadowBlurImageView();
	ShadowPassSettings GetShadowPassSettings();
	void RequestShadowBufferDump();
	std::string GetShadowBufferDumpStatus();

	std::string GetShadowCasterDebugInfo();
}
