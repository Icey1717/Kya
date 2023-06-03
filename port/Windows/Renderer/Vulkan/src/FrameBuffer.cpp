#include "FrameBuffer.h"
#include <unordered_map>
#include "VulkanRenderer.h"
#include "stdexcept"

std::unordered_map<int, FrameBuffer> passes;

// Custom res settings.
const Vector2i rtsize = { 0x500, 0x400 };
const Vector2f rtscale = { 1.0f, 1.0f };

Vector2i GetRTSize()
{
	return rtsize;
}

Vector2f GetRTScale()
{
	return rtscale;
}

FrameBuffer FrameBuffer::Create(Vector2i size, int FBP) {
	FrameBuffer newBuffer = {};

	newBuffer.FBP = FBP;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = GetSwapchainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription colorSubpass{};
	colorSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	colorSubpass.colorAttachmentCount = 1;
	colorSubpass.pColorAttachments = &colorAttachmentRef;
	colorSubpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency colorDependency{};
	colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	colorDependency.dstSubpass = 0;
	colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorDependency.srcAccessMask = 0;
	colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depthDependency{};
	depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depthDependency.dstSubpass = 0;
	depthDependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depthDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	depthDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	depthDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkSubpassDependency selfDependency{};
	selfDependency.srcSubpass = 0;
	selfDependency.dstSubpass = 0;
	selfDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	selfDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	selfDependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	selfDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	selfDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
	VkSubpassDescription subpasses[] = { colorSubpass };
	VkSubpassDependency dependencies[] = { colorDependency, depthDependency };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = subpasses;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &newBuffer.renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	{
		const VkFormat format = GetSwapchainImageFormat();
		const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// Create the images that will be used as render targets
		VkImageCreateInfo colorImageCreateInfo = {};
		colorImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		colorImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		colorImageCreateInfo.extent.width = size.x;
		colorImageCreateInfo.extent.height = size.y;
		colorImageCreateInfo.extent.depth = 1;
		colorImageCreateInfo.mipLevels = 1;
		colorImageCreateInfo.arrayLayers = 1;
		colorImageCreateInfo.format = format;
		colorImageCreateInfo.tiling = tiling;
		colorImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorImageCreateInfo.usage = usage;
		colorImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		colorImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkCreateImage(GetDevice(), &colorImageCreateInfo, nullptr, &newBuffer.colorImage);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(GetDevice(), newBuffer.colorImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, &newBuffer.imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(GetDevice(), newBuffer.colorImage, newBuffer.imageMemory, 0);

		// Create an image view for the color image
		VkImageViewCreateInfo colorImageViewCreateInfo = {};

		colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageViewCreateInfo.image = newBuffer.colorImage;
		colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageViewCreateInfo.format = format;
		colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		colorImageViewCreateInfo.subresourceRange.levelCount = 1;
		colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		colorImageViewCreateInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(GetDevice(), &colorImageViewCreateInfo, nullptr, &newBuffer.colorImageView);
	}

	{
		VkFormat format = VK_FORMAT_D32_SFLOAT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		// Create the images that will be used as render targets
		VkImageCreateInfo depthImageCreateInfo = {};
		depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		depthImageCreateInfo.extent.width = size.x;
		depthImageCreateInfo.extent.height = size.y;
		depthImageCreateInfo.extent.depth = 1;
		depthImageCreateInfo.mipLevels = 1;
		depthImageCreateInfo.arrayLayers = 1;
		depthImageCreateInfo.format = format;
		depthImageCreateInfo.tiling = tiling;
		depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthImageCreateInfo.usage = usage;
		depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkCreateImage(GetDevice(), &depthImageCreateInfo, nullptr, &newBuffer.depthImage);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(GetDevice(), newBuffer.depthImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(GetDevice(), &allocInfo, nullptr, &newBuffer.depthImageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(GetDevice(), newBuffer.depthImage, newBuffer.depthImageMemory, 0);

		// Create an image view for the color image
		VkImageViewCreateInfo depthImageViewCreateInfo = {};

		depthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthImageViewCreateInfo.image = newBuffer.depthImage;
		depthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthImageViewCreateInfo.format = format;
		depthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		depthImageViewCreateInfo.subresourceRange.levelCount = 1;
		depthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		depthImageViewCreateInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(GetDevice(), &depthImageViewCreateInfo, nullptr, &newBuffer.depthImageView);
	}

	VkImageView imageAttachments[] = { newBuffer.colorImageView , newBuffer.depthImageView };

	// Create the framebuffer that will use the color image as a render target
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = newBuffer.renderPass;
	framebufferCreateInfo.attachmentCount = 2;
	framebufferCreateInfo.pAttachments = imageAttachments;
	framebufferCreateInfo.width = size.x;
	framebufferCreateInfo.height = size.y;
	framebufferCreateInfo.layers = 1;
	vkCreateFramebuffer(GetDevice(), &framebufferCreateInfo, nullptr, &newBuffer.framebuffer);

	return newBuffer;
}

FrameBuffer& FrameBuffer::Get(int FBP)
{
	if (passes.find(FBP) == passes.end()) {
		passes.emplace(FBP, FrameBuffer::Create(rtsize, FBP));
	}

	return passes[FBP]; // Return the FrameBuffer object
}
