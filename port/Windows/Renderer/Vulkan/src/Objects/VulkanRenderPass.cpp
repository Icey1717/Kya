#include "VulkanRenderPass.h"

#include <stdexcept>
#include <vector>

#include "VulkanRenderer.h"

namespace Renderer
{
	VkRenderPass CreateRenderPass2D(
		std::span<const AttachmentInfo>      colorAttachments,
		std::optional<AttachmentInfo>        depthAttachment,
		std::span<const VkSubpassDependency> dependencies,
		const char*                          debugName)
	{
		std::vector<VkAttachmentDescription> attachmentDescs;
		std::vector<VkAttachmentReference>   colorRefs;

		for (uint32_t i = 0; i < colorAttachments.size(); ++i)
		{
			const AttachmentInfo& info = colorAttachments[i];

			VkAttachmentDescription desc{};
			desc.format         = info.format;
			desc.samples        = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp         = info.loadOp;
			desc.storeOp        = info.storeOp;
			desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout  = info.initialLayout;
			desc.finalLayout    = info.finalLayout;
			attachmentDescs.push_back(desc);

			VkAttachmentReference ref{};
			ref.attachment = i;
			ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorRefs.push_back(ref);
		}

		VkAttachmentReference depthRef{};
		if (depthAttachment)
		{
			const AttachmentInfo& info = *depthAttachment;

			VkAttachmentDescription desc{};
			desc.format         = info.format;
			desc.samples        = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp         = info.loadOp;
			desc.storeOp        = info.storeOp;
			desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout  = info.initialLayout;
			desc.finalLayout    = info.finalLayout;
			attachmentDescs.push_back(desc);

			depthRef.attachment = static_cast<uint32_t>(colorAttachments.size());
			depthRef.layout     = (info.finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
				? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
				: VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount    = static_cast<uint32_t>(colorRefs.size());
		subpass.pColorAttachments       = colorRefs.data();
		subpass.pDepthStencilAttachment = depthAttachment ? &depthRef : nullptr;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.pAttachments    = attachmentDescs.data();
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies   = dependencies.empty() ? nullptr : dependencies.data();

		VkRenderPass renderPass = VK_NULL_HANDLE;
		if (vkCreateRenderPass(GetDevice(), &renderPassInfo, GetAllocator(), &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		if (debugName) {
			SetObjectName(reinterpret_cast<uint64_t>(renderPass), VK_OBJECT_TYPE_RENDER_PASS, "%s", debugName);
		}

		return renderPass;
	}

	VkFramebuffer CreateFramebuffer(
		VkRenderPass                 renderPass,
		std::span<const VkImageView> attachments,
		uint32_t                     width,
		uint32_t                     height,
		const char*                  debugName)
	{
		VkFramebufferCreateInfo info{};
		info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass      = renderPass;
		info.attachmentCount = static_cast<uint32_t>(attachments.size());
		info.pAttachments    = attachments.data();
		info.width           = width;
		info.height          = height;
		info.layers          = 1;

		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		if (vkCreateFramebuffer(GetDevice(), &info, GetAllocator(), &framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

		if (debugName) {
			SetObjectName(reinterpret_cast<uint64_t>(framebuffer), VK_OBJECT_TYPE_FRAMEBUFFER, "%s", debugName);
		}

		return framebuffer;
	}

} // namespace Renderer
