#include "NativeMSAA.h"
#include "NativeRendererInternal.h"
#include "Objects/VulkanImage.h"
#include "Objects/VulkanRenderPass.h"
#include "Objects/VulkanShader.h"
#include <stdexcept>

namespace Renderer::Native::MSAA
{
	namespace
	{
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		OwnedImage color, depth;
		VkRenderPass seedPass = VK_NULL_HANDLE;
		VkFramebuffer seedFramebuffer = VK_NULL_HANDLE, framebuffer = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		Pipeline seedPipeline;

		// Resolve color and reverse-Z depth at every native pass boundary so fog,
		// framebuffer captures, shadows and HUD rendering keep their single-sample inputs.
		VkRenderPass MakePass(EClearMode clearMode, bool resolve)
		{
			const bool clearColor = clearMode == EClearMode::Color || clearMode == EClearMode::ColorDepth;
			const bool clearDepth = clearMode == EClearMode::Depth || clearMode == EClearMode::ColorDepth;
			std::array<VkAttachmentDescription2, 4> attachments{};
			for (uint32_t i = 0; i < attachments.size(); ++i) {
				auto& a = attachments[i];
				a.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
				a.format = (i & 1) ? VK_FORMAT_D32_SFLOAT_S8_UINT : GetSwapchainImageFormat();
				a.samples = i < 2 ? samples : VK_SAMPLE_COUNT_1_BIT;
				const bool clear = (i & 1) ? clearDepth : clearColor;
				a.loadOp = i >= 2 ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				a.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				a.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				a.initialLayout = i >= 2 || clear ? VK_IMAGE_LAYOUT_UNDEFINED : (i & 1) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				a.finalLayout = i >= 2 ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL : (i & 1) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			const VkAttachmentReference2 colorRef{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT };
			const VkAttachmentReference2 depthRef{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT };
			const VkAttachmentReference2 colorResolve{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT };
			const VkAttachmentReference2 depthResolve{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT };
			VkSubpassDescriptionDepthStencilResolve depthInfo{ VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE };
			depthInfo.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
			depthInfo.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
			depthInfo.pDepthStencilResolveAttachment = &depthResolve;
			VkSubpassDescription2 subpass{ VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2 };
			subpass.pNext = resolve ? &depthInfo : nullptr;
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorRef;
			subpass.pDepthStencilAttachment = &depthRef;
			subpass.pResolveAttachments = resolve ? &colorResolve : nullptr;
			std::array<VkSubpassDependency2, 2> dependencies{};
			for (auto& d : dependencies) {
				d.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
				d.srcStageMask = d.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				d.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
				d.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			}
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			VkRenderPassCreateInfo2 info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };
			info.attachmentCount = resolve ? 4 : 2;
			info.pAttachments = attachments.data();
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = static_cast<uint32_t>(dependencies.size());
			info.pDependencies = dependencies.data();
			VkRenderPass pass;
			if (vkCreateRenderPass2(GetDevice(), &info, GetAllocator(), &pass) != VK_SUCCESS)
				throw std::runtime_error("failed to create native MSAA render pass");
			return pass;
		}
	}

	VkSampleCountFlagBits GetSamples() { return samples; }
	VkFramebuffer GetFramebuffer() { return framebuffer; }
	VkRenderPass CreateRenderPass(const RenderPassKey& key) { return MakePass(key.clearMode, true); }

	void Setup()
	{
		VkImageFormatProperties colorProps{}, depthProps{};
		const auto colorResult = vkGetPhysicalDeviceImageFormatProperties(GetPhysicalDevice(), GetSwapchainImageFormat(), VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &colorProps);
		const auto depthResult = vkGetPhysicalDeviceImageFormatProperties(GetPhysicalDevice(), VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, &depthProps);
		if (colorResult != VK_SUCCESS || depthResult != VK_SUCCESS) return;
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
		const auto supported = colorProps.sampleCounts & depthProps.sampleCounts
			& properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
		samples = supported & VK_SAMPLE_COUNT_4_BIT ? VK_SAMPLE_COUNT_4_BIT : supported & VK_SAMPLE_COUNT_2_BIT ? VK_SAMPLE_COUNT_2_BIT : VK_SAMPLE_COUNT_1_BIT;
		if (samples == VK_SAMPLE_COUNT_1_BIT) return;
		seedPass = MakePass(EClearMode::ColorDepth, false);
		auto vert = Shader::ReflectedModule("shaders/postprocess.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		auto frag = Shader::ReflectedModule("shaders/msaa_seed.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		seedPipeline.debugName = "Native MSAA Seed";
		seedPipeline.AddBindings(EBindingStage::Vertex, vert.reflectData);
		seedPipeline.AddBindings(EBindingStage::Fragment, frag.reflectData);
		seedPipeline.CreateDescriptorSetLayouts();
		seedPipeline.CreateLayout();
		seedPipeline.CreateDescriptorPool();
		seedPipeline.CreateDescriptorSets();
		const std::array stages{ vert.shaderStageCreateInfo, frag.shaderStageCreateInfo };
		VkPipelineVertexInputStateCreateInfo vertices{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VkPipelineInputAssemblyStateCreateInfo assembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPipelineViewportStateCreateInfo viewport{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewport.viewportCount = viewport.scissorCount = 1;
		VkPipelineRasterizationStateCreateInfo raster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.lineWidth = 1.0f;
		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.rasterizationSamples = samples;
		VkPipelineDepthStencilStateCreateInfo depthState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthState.depthTestEnable = depthState.depthWriteEnable = VK_TRUE;
		depthState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		VkPipelineColorBlendAttachmentState attachment{};
		attachment.colorWriteMask = 15;
		VkPipelineColorBlendStateCreateInfo blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blend.attachmentCount = 1;
		blend.pAttachments = &attachment;
		const std::array states{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamic.dynamicStateCount = static_cast<uint32_t>(states.size());
		dynamic.pDynamicStates = states.data();
		VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		info.stageCount = static_cast<uint32_t>(stages.size());
		info.pStages = stages.data();
		info.pVertexInputState = &vertices;
		info.pInputAssemblyState = &assembly;
		info.pViewportState = &viewport;
		info.pRasterizationState = &raster;
		info.pMultisampleState = &multisampling;
		info.pDepthStencilState = &depthState;
		info.pColorBlendState = &blend;
		info.pDynamicState = &dynamic;
		info.layout = seedPipeline.layout;
		info.renderPass = seedPass;
		if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &info, GetAllocator(), &seedPipeline.pipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create native MSAA seed pipeline");
		VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		if (vkCreateSampler(GetDevice(), &samplerInfo, GetAllocator(), &sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create native MSAA sampler");
	}

	void CreateFramebuffer()
	{
		if (samples == VK_SAMPLE_COUNT_1_BIT) return;
		VulkanImage::CreateImage(gWidth, gHeight, GetSwapchainImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, color.image, color.memory, 1, samples);
		VulkanImage::CreateImageView(color.image, GetSwapchainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, color.view);
		VulkanImage::CreateImage(gWidth, gHeight, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth.image, depth.memory, 1, samples);
		VulkanImage::CreateImageView(depth.image, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT, depth.view);
		const auto& target = GetNativeRendererState().frameBuffer;
		const std::array seedAttachments{ color.view, depth.view };
		seedFramebuffer = Renderer::CreateFramebuffer(seedPass, seedAttachments, gWidth, gHeight, "MSAA Seed Framebuffer");
		const std::array attachments{ color.view, depth.view, target.colorImageView, target.depthImageView };
		RenderPassKey key{ EClearMode::None, ERenderPassKind::Main, true };
		framebuffer = Renderer::CreateFramebuffer(GetNativeRendererState().renderPass.at(key).gRenderPass, attachments, gWidth, gHeight, "MSAA Framebuffer");
		for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; ++frame) {
			const std::array images{
				VkDescriptorImageInfo{ sampler, target.colorImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL },
				VkDescriptorImageInfo{ sampler, target.depthImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL },
			};
			DescriptorWriteList writes;
			for (int binding = 0; binding < 2; ++binding)
				writes.EmplaceWrite({ binding, EBindingStage::Fragment, nullptr, &images[binding], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
			const auto updates = writes.CreateWriteDescriptorSetList(seedPipeline.descriptorSets[frame], seedPipeline.descriptorSetLayoutBindings);
			vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(updates.size()), updates.data(), 0, nullptr);
		}
	}

	void Seed(VkCommandBuffer cmd, const RenderPassKey& key)
	{
		if (key.clearMode == EClearMode::ColorDepth) return;
		// Expand the resolved scene before resuming geometry after a single-sample
		// effect. The following scene pass applies any requested color/depth clears.
		VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
		VkRenderPassBeginInfo begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		begin.renderPass = seedPass;
		begin.framebuffer = seedFramebuffer;
		begin.renderArea.extent = { static_cast<uint32_t>(gWidth), static_cast<uint32_t>(gHeight) };
		const std::array<VkClearValue, 2> clears{};
		begin.clearValueCount = static_cast<uint32_t>(clears.size());
		begin.pClearValues = clears.data();
		vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
		const VkViewport viewport{ 0, 0, static_cast<float>(gWidth), static_cast<float>(gHeight), 0, 1 };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &begin.renderArea);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, seedPipeline.pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, seedPipeline.layout, 0, 1, &seedPipeline.descriptorSets[GetCurrentFrame()], 0, nullptr);
		vkCmdDraw(cmd, 3, 1, 0, 0);
		vkCmdEndRenderPass(cmd);
	}

	void DestroyFramebuffer()
	{
		if (framebuffer) vkDestroyFramebuffer(GetDevice(), framebuffer, GetAllocator());
		if (seedFramebuffer) vkDestroyFramebuffer(GetDevice(), seedFramebuffer, GetAllocator());
		framebuffer = seedFramebuffer = VK_NULL_HANDLE;
		color.Destroy(); depth.Destroy();
	}
	void Cleanup()
	{
		DestroyFramebuffer();
		auto& stages = GetNativeRendererState().renderPass;
		for (auto it = stages.begin(); it != stages.end();) {
			if (!it->first.multisampled) { ++it; continue; }
			auto& stage = it->second;
			for (const auto& [key, pipeline] : stage.gBlendPipelines)
				if (pipeline != stage.gPipeline.pipeline) vkDestroyPipeline(GetDevice(), pipeline, GetAllocator());
			stage.gPipeline.Destroy();
			vkDestroyRenderPass(GetDevice(), stage.gRenderPass, GetAllocator());
			it = stages.erase(it);
		}
		seedPipeline.Destroy();
		seedPipeline.descriptorSetLayoutBindings.clear();
		seedPipeline.pushConstants.clear();
		if (seedPass) vkDestroyRenderPass(GetDevice(), seedPass, GetAllocator());
		if (sampler) vkDestroySampler(GetDevice(), sampler, GetAllocator());
		seedPass = VK_NULL_HANDLE; sampler = VK_NULL_HANDLE;
		samples = VK_SAMPLE_COUNT_1_BIT;
	}
}
