#include "FrameBuffer.h"
#include "VulkanRenderer.h"
#include "stdexcept"
#include "VulkanCommands.h"
#include "VulkanShader.h"
#include "VulkanReflect.h"
#include <array>
#include "VulkanImage.h"

FrameBuffer::FrameBufferMap passes;

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

static void CreateRenderPass(VkRenderPass& renderPass, bool bIsClear) {
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
	colorAttachment.loadOp = bIsClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = bIsClear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
#ifdef BLIT_TO_BACKBUFFER
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
#else
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
#endif

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

	if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void FrameBuffer::ExecuteClearPass()
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = clearPass;
	renderPassInfo.framebuffer = framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { (unsigned int)GetRTSize().x, (unsigned int)GetRTSize().y };

	VkClearValue clearColors[] = { {{0.0f, 0.0f, 0.0f, 1.0f}}, {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearColors;

	VkImageSubresourceLayers subresourceLayers = {};
	subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceLayers.mipLevel = 0;
	subresourceLayers.baseArrayLayer = 0;
	subresourceLayers.layerCount = 1;

	vkCmdBeginRenderPass(GetCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(GetCurrentCommandBuffer());
}

void FrameBuffer::ExecuteFinalPass()
{
	// Begin the grayscale render pass
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = finalPass;
	renderPassBeginInfo.framebuffer = finalFramebuffer; // Replace with your actual framebuffer
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = { (unsigned int)GetRTSize().x, (unsigned int)GetRTSize().y };

	std::vector<VkClearValue> clearColors = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } }; // Clear color to black
	renderPassBeginInfo.clearValueCount = clearColors.size();
	renderPassBeginInfo.pClearValues = clearColors.data();

	vkCmdBeginRenderPass(GetCurrentCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind the grayscale pipeline
	vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, finalPipeline.pipeline);

	vkCmdBindDescriptorSets(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, finalPipeline.layout, 0, 1, &finalPipeline.descriptorSets[GetCurrentFrame()], 0, nullptr);


	// Draw the full-screen quad
	vkCmdDraw(GetCurrentCommandBuffer(), 4, 1, 0, 0);

	// End the render pass
	vkCmdEndRenderPass(GetCurrentCommandBuffer());
}

void FrameBuffer::CreateFinalPassPipeline()
{
	{
		const VkFormat format = GetSwapchainImageFormat();
		const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		VulkanImage::CreateImage(GetRTSize().x, GetRTSize().y, format, tiling, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, finalImage, finalImageMemory);
		VulkanImage::CreateImageView(finalImage, format, VK_IMAGE_ASPECT_COLOR_BIT, finalImageView);
	}

	// Grayscale Render Pass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = GetSwapchainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Load the previous content
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store the result
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &finalPass);

	VkImageView imageAttachments[] = { finalImageView };

	// Create the framebuffer that will use the color image as a render target
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = finalPass;
	framebufferCreateInfo.attachmentCount = 1;
	framebufferCreateInfo.pAttachments = imageAttachments;
	framebufferCreateInfo.width = GetRTSize().x;
	framebufferCreateInfo.height = GetRTSize().y;
	framebufferCreateInfo.layers = 1;
	vkCreateFramebuffer(GetDevice(), &framebufferCreateInfo, nullptr, &finalFramebuffer);

	auto vertShader = Shader::ReflectedModule("shaders/fullscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, false);
	auto fragShader = Shader::ReflectedModule("shaders/gray.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, false);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)GetRTSize().x;
	viewport.height = (float)GetRTSize().y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { (unsigned int)GetRTSize().x, (unsigned int)GetRTSize().y };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	finalPipeline.AddBindings(Renderer::EBindingStage::Fragment, fragShader.reflectData);
	finalPipeline.CreateDescriptorSetLayouts();
	finalPipeline.CreateLayout();

	std::vector<VkPipelineShaderStageCreateInfo> stages = { vertShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = stages.size();
	pipelineInfo.pStages = stages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = finalPipeline.layout;
	pipelineInfo.renderPass = finalPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &finalPipeline.pipeline);

	finalPipeline.CreateDescriptorPool();
	finalPipeline.CreateDescriptorSets();

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 0.0f;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = -FLT_MAX;
	samplerInfo.maxLod = FLT_MAX;

	if (vkCreateSampler(GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = colorImageView;
		imageInfo.sampler = sampler;

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = finalPipeline.descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

FrameBuffer FrameBuffer::Create(Vector2i size, int FBP) {
	FrameBuffer newBuffer = {};

	newBuffer.FBP = FBP;

	CreateRenderPass(newBuffer.clearPass, true);
	SetObjectName("Framebuffer Clear Pass", (uint64_t)newBuffer.clearPass, VK_OBJECT_TYPE_RENDER_PASS);

	CreateRenderPass(newBuffer.renderPass, false);
	SetObjectName("Framebuffer Render Pass", (uint64_t)newBuffer.renderPass, VK_OBJECT_TYPE_RENDER_PASS);

	{
		const VkFormat format = GetSwapchainImageFormat();
		const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		VulkanImage::CreateImage(size.x, size.y, format, tiling, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newBuffer.colorImage, newBuffer.imageMemory);
		VulkanImage::CreateImageView(newBuffer.colorImage, format, VK_IMAGE_ASPECT_COLOR_BIT, newBuffer.colorImageView);
	}

	{
		VkFormat format = VK_FORMAT_D32_SFLOAT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VulkanImage::CreateImage(size.x, size.y, format, tiling, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newBuffer.depthImage, newBuffer.depthImageMemory);
		VulkanImage::CreateImageView(newBuffer.depthImage, format, VK_IMAGE_ASPECT_DEPTH_BIT, newBuffer.depthImageView);
	}

	newBuffer.CreateFinalPassPipeline();

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

	{
		char buff[256];
		sprintf_s(buff, 256, "Framebuffer Object (%d, %d) FBP: %d", size.x, size.y, FBP);
		SetObjectName(buff, (uint64_t)newBuffer.framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER);
	}

	newBuffer.ExecuteClearPass();

	return newBuffer;
}

bool FrameBuffer::Exists(int FBP)
{
	const auto it = passes.find(FBP);
	return it != passes.end();
}

FrameBuffer& FrameBuffer::Get(int FBP)
{
	auto it = passes.find(FBP);
	if (it == passes.end()) {
		passes.emplace(FBP, FrameBuffer::Create(rtsize, FBP));
		return passes.at(FBP);
	}

	return it->second;
}

FrameBuffer::FrameBufferMap& FrameBuffer::GetAll()
{
	return passes;
}
