#include "NativeFog.h"

#include "NativeRendererInternal.h"
#include "Objects/VulkanRenderPass.h"
#include "Objects/VulkanShader.h"
#include "profiling.h"
#include "Objects/VulkanImage.h"
#include <algorithm>
#include <cmath>

#include <array>
#include <stdexcept>

namespace Renderer::Native::Fog
{
	namespace
	{
		constexpr uint32_t setsPerPool = 256;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		Pipeline pipeline;
		std::array<std::vector<VkDescriptorPool>, MAX_FRAMES_IN_FLIGHT> pools;
		uint32_t descriptorIndex = 0;
		std::array<OwnedImage, MAX_FRAMES_IN_FLIGHT> depthCopies;
		std::array<bool, MAX_FRAMES_IN_FLIGHT> initialized{};

		struct PushConstants
		{
			glm::vec4 color;
			glm::vec4 params;
			glm::vec4 depthProjection;
		};
		static_assert(sizeof(PushConstants) == 48);

		VkDescriptorSet AllocateDescriptor()
		{
			auto& framePools = pools[GetCurrentFrame()];
			const uint32_t poolIndex = descriptorIndex / setsPerPool;
			if (poolIndex == framePools.size()) {
				const VkDescriptorPoolSize size{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, setsPerPool * 2 };
				VkDescriptorPoolCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				info.maxSets = setsPerPool;
				info.poolSizeCount = 1;
				info.pPoolSizes = &size;
				VkDescriptorPool pool;
				if (vkCreateDescriptorPool(GetDevice(), &info, GetAllocator(), &pool) != VK_SUCCESS)
					throw std::runtime_error("failed to create native fog descriptor pool");
				framePools.push_back(pool);
			}
			VkDescriptorSetAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorPool = framePools[poolIndex];
			info.descriptorSetCount = 1;
			info.pSetLayouts = pipeline.descriptorSetLayouts.data();
			VkDescriptorSet set;
			if (vkAllocateDescriptorSets(GetDevice(), &info, &set) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate native fog descriptor");
			++descriptorIndex;
			return set;
		}

		void CreatePipeline()
		{
			pipeline.debugName = "Native Fog Pipeline";
			auto vert = Shader::ReflectedModule("shaders/postprocess.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			auto frag = Shader::ReflectedModule("shaders/fog.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
			pipeline.AddBindings(EBindingStage::Vertex, vert.reflectData);
			pipeline.AddBindings(EBindingStage::Fragment, frag.reflectData);
			pipeline.CreateDescriptorSetLayouts();
			pipeline.CreateLayout();
			const std::array stages{ vert.shaderStageCreateInfo, frag.shaderStageCreateInfo };
			VkPipelineVertexInputStateCreateInfo vertices{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
			VkPipelineInputAssemblyStateCreateInfo assembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
			assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			VkPipelineViewportStateCreateInfo viewport{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewport.viewportCount = 1;
			viewport.scissorCount = 1;
			VkPipelineRasterizationStateCreateInfo raster{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			raster.polygonMode = VK_POLYGON_MODE_FILL;
			raster.cullMode = VK_CULL_MODE_NONE;
			raster.lineWidth = 1.0f;
			VkPipelineMultisampleStateCreateInfo samples{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			samples.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			VkPipelineDepthStencilStateCreateInfo depth{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
			depth.depthTestEnable = VK_TRUE;
			depth.depthWriteEnable = VK_TRUE;
			depth.depthCompareOp = VK_COMPARE_OP_ALWAYS;
			VkPipelineColorBlendAttachmentState attachment{};
			attachment.blendEnable = VK_TRUE;
			attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;
			attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
			attachment.colorBlendOp = VK_BLEND_OP_ADD;
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			VkPipelineColorBlendStateCreateInfo blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			blend.attachmentCount = 1;
			blend.pAttachments = &attachment;
			const std::array dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamic{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamic.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamic.pDynamicStates = dynamicStates.data();
			VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
			info.stageCount = static_cast<uint32_t>(stages.size());
			info.pStages = stages.data();
			info.pVertexInputState = &vertices;
			info.pInputAssemblyState = &assembly;
			info.pViewportState = &viewport;
			info.pRasterizationState = &raster;
			info.pMultisampleState = &samples;
			info.pDepthStencilState = &depth;
			info.pColorBlendState = &blend;
			info.pDynamicState = &dynamic;
			info.layout = pipeline.layout;
			info.renderPass = renderPass;
			if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &info, GetAllocator(), &pipeline.pipeline) != VK_SUCCESS)
				throw std::runtime_error("failed to create native fog pipeline");
			SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, "Native Fog Pipeline");
		}
	}

	void Setup()
	{
		// Sample a depth snapshot while writing the packet's green-byte adjustment.
		const AttachmentInfo color{ GetSwapchainImageFormat(), VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL };
		const AttachmentInfo depth{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL };
		const std::array dependencies{
			VkSubpassDependency{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0 },
			VkSubpassDependency{ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, 0 },
		};
		renderPass = CreateRenderPass2D({ &color, 1 }, depth, dependencies, "Native Fog Render Pass");
		CreatePipeline();
		VkSamplerCreateInfo info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		info.magFilter = VK_FILTER_NEAREST;
		info.minFilter = VK_FILTER_NEAREST;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		if (vkCreateSampler(GetDevice(), &info, GetAllocator(), &sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create native fog sampler");
		CreateFramebuffer();
	}

	void DestroyFramebuffer()
	{
		if (framebuffer) vkDestroyFramebuffer(GetDevice(), framebuffer, GetAllocator());
		framebuffer = VK_NULL_HANDLE;
		for (auto& image : depthCopies) image.Destroy();
		initialized.fill(false);
	}

	void CreateFramebuffer()
	{
		for (auto& image : depthCopies) image = VulkanImage::CreateDepth(gWidth, gHeight, VK_IMAGE_USAGE_SAMPLED_BIT);
		const std::array attachments{ GetNativeRendererState().frameBuffer.colorImageView, GetNativeRendererState().frameBuffer.depthImageView };
		framebuffer = Renderer::CreateFramebuffer(renderPass, attachments, gWidth, gHeight, "Native Fog Framebuffer");
	}

	void Cleanup()
	{
		DestroyFramebuffer();
		for (auto& framePools : pools) {
			for (auto pool : framePools) vkDestroyDescriptorPool(GetDevice(), pool, GetAllocator());
			framePools.clear();
		}
		pipeline.Destroy();
		pipeline.pushConstants.clear();
		pipeline.descriptorSetLayoutBindings.clear();
		if (sampler) vkDestroySampler(GetDevice(), sampler, GetAllocator());
		if (renderPass) vkDestroyRenderPass(GetDevice(), renderPass, GetAllocator());
		sampler = VK_NULL_HANDLE;
		renderPass = VK_NULL_HANDLE;
	}

	void BeginFrame()
	{
		// The renderer has waited for this frame slot's fence before recording starts.
		for (auto pool : pools[GetCurrentFrame()]) {
			if (vkResetDescriptorPool(GetDevice(), pool, 0) != VK_SUCCESS)
				throw std::runtime_error("failed to reset native fog descriptor pool");
		}
		descriptorIndex = 0;
	}

	void Record(VkCommandBuffer cmd, const FogDraw& fog)
	{
		ZONE_SCOPED;
		const auto frame = GetCurrentFrame();
		const auto source = GetNativeRendererState().frameBuffer.depthImage;
		const auto snapshot = depthCopies[frame].image;
		auto barrier = [&](VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkPipelineStageFlags dstStage, VkAccessFlags dstAccess) {
			VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			b.srcQueueFamilyIndex = b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			b.image = image;
			b.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
			b.oldLayout = oldLayout;
			b.newLayout = newLayout;
			b.srcAccessMask = srcAccess;
			b.dstAccessMask = dstAccess;
			vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &b);
		};
		Debug::BeginLabel(cmd, "Scene Fog [offset %d, flags 0x%x]", fog.depthOffset, fog.flags);
		barrier(source, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_MEMORY_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		barrier(snapshot, initialized[frame] ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			initialized[frame] ? VK_ACCESS_SHADER_READ_BIT : 0, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
		VkImageCopy copy{};
		copy.srcSubresource = copy.dstSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
		copy.extent = { static_cast<uint32_t>(gWidth), static_cast<uint32_t>(gHeight), 1 };
		vkCmdCopyImage(cmd, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, snapshot, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		barrier(snapshot, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
		barrier(source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		initialized[frame] = true;

		const VkDescriptorSet set = AllocateDescriptor();
		const VkDescriptorImageInfo imageInfo{ sampler, depthCopies[frame].view, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL };
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstSet = set;
		write.dstBinding = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(GetDevice(), 1, &write, 0, nullptr);
		VkRenderPassBeginInfo begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		begin.renderPass = renderPass;
		begin.framebuffer = framebuffer;
		begin.renderArea.extent = GetFrameBufferSize();
		vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(gWidth), static_cast<float>(gHeight), 0.0f, 1.0f };
		const int x0 = std::clamp(static_cast<int>(std::lround(fog.viewport[0] * gWidth)), 0, gWidth);
		const int y0 = std::clamp(static_cast<int>(std::lround(fog.viewport[1] * gHeight)), 0, gHeight);
		const int x1 = std::clamp(static_cast<int>(std::lround((fog.viewport[0] + fog.viewport[2]) * gWidth)), x0, gWidth);
		const int y1 = std::clamp(static_cast<int>(std::lround((fog.viewport[1] + fog.viewport[3]) * gHeight)), y0, gHeight);
		const VkRect2D scissor{ { x0, y0 }, { static_cast<uint32_t>(x1 - x0), static_cast<uint32_t>(y1 - y0) } };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &set, 0, nullptr);
		const PushConstants data{
			{ float(fog.color[0] & 255) / 255.0f, float(fog.color[1] & 255) / 255.0f,
			  float(fog.color[2] & 255) / 255.0f, float(fog.color[3] & 255) / 255.0f },
			{ fog.gsNear, fog.gsFar, static_cast<float>(fog.depthOffset), static_cast<float>(fog.flags) },
			glm::vec4(fog.depthProjection[0], fog.depthProjection[1], fog.depthProjection[2], fog.depthProjection[3]),
		};
		auto submitted = data;
		if (submitted.depthProjection == glm::vec4(0)) {
			submitted.depthProjection = glm::vec4(fog.gsNear - fog.gsFar, fog.gsFar, 0, 1);
		}
		vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(submitted), &submitted);
		vkCmdDraw(cmd, 3, 1, 0, 0);
		vkCmdEndRenderPass(cmd);
		Debug::EndLabel(cmd);
	}
}
