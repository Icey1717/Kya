#pragma once

#include "VulkanIncludes.h"
#include <optional>
#include <span>

namespace Renderer
{
	struct AttachmentInfo
	{
		VkFormat            format;
		VkAttachmentLoadOp  loadOp;
		VkImageLayout       initialLayout;
		VkImageLayout       finalLayout;
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		bool                readOnlyDepth = false;
	};

	// Creates a single-subpass 2D render pass.
	// Color attachments are bound at indices 0..n-1; depth (if present) at index n.
	// Set AttachmentInfo::readOnlyDepth when a depth attachment must be tested but not written.
	VkRenderPass CreateRenderPass2D(
		std::span<const AttachmentInfo>      colorAttachments,
		std::optional<AttachmentInfo>        depthAttachment,
		std::span<const VkSubpassDependency> dependencies,
		const char*                          debugName = nullptr);

	// Creates a 2D framebuffer. attachments must outlive this call.
	VkFramebuffer CreateFramebuffer(
		VkRenderPass                  renderPass,
		std::span<const VkImageView>  attachments,
		uint32_t                      width,
		uint32_t                      height,
		const char*                   debugName = nullptr);

} // namespace Renderer
