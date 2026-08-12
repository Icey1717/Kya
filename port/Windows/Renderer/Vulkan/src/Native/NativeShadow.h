#pragma once

#include "renderer.h"
#include "VulkanIncludes.h"

namespace Renderer::Native::Shadow
{
	ShadowPassSettings NormalizeSettings(const ShadowPassSettings& settings);
	void Setup();
	void Cleanup();
	void DestroyReceiverFramebuffer();
	void CreateReceiverFramebuffer();

	void BeginMask(const ShadowPassSettings& settings);
	void RecordBlur(const VkCommandBuffer& cmd);
	void BeginReceiver(const ShadowReceiverViewport& viewport);
	void End();

	VkFramebuffer GetFramebuffer(ERenderPassKind kind);
	VkExtent2D GetExtent(ERenderPassKind kind);
	VkRect2D GetReceiverScissor();
	const VkDescriptorSet& GetReceiverDescriptorSet(uint32_t frameIndex);

	VkSampler GetSampler();
	VkImageView GetMaskImageView();
	VkImageView GetBlurImageView();
	ShadowPassSettings GetSettings();
}
