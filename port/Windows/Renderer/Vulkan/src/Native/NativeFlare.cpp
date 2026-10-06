#include "NativeFlare.h"

#include "NativeRendererInternal.h"
#include "Objects/VulkanRenderPass.h"
#include "Objects/VulkanShader.h"
#include "profiling.h"
#include "profiling.h"

#include <array>
#include <stdexcept>

namespace Renderer::Native::Flare
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

		struct PushConstants
		{
			glm::vec4 rect;
			glm::vec4 marker;
			glm::vec4 params;
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
					throw std::runtime_error("failed to create native flare descriptor pool");
				framePools.push_back(pool);
			}
			VkDescriptorSetAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorPool = framePools[poolIndex];
			info.descriptorSetCount = 1;
			info.pSetLayouts = pipeline.descriptorSetLayouts.data();
			VkDescriptorSet set;
			if (vkAllocateDescriptorSets(GetDevice(), &info, &set) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate native flare descriptor");
			++descriptorIndex;
			return set;
		}

		void CreatePipeline()
		{
			pipeline.debugName = "Native Flare Pipeline";
			auto vert = Shader::ReflectedModule("shaders/flare.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			auto frag = Shader::ReflectedModule("shaders/flare.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
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
			VkPipelineColorBlendAttachmentState attachment{};
			attachment.blendEnable = VK_TRUE;
			attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.colorBlendOp = VK_BLEND_OP_ADD;
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
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
				throw std::runtime_error("failed to create native flare pipeline");
			SetObjectName(reinterpret_cast<uint64_t>(pipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, "Native Flare Pipeline");
		}
	}

	void Setup()
	{
		// No depth attachment: the preceding scene depth remains available for sampling.
		const AttachmentInfo color{ GetSwapchainImageFormat(), VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL };
		const std::array dependencies{
			VkSubpassDependency{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
				VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT, 0 },
			VkSubpassDependency{ 0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
				VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, 0 },
		};
		renderPass = CreateRenderPass2D({ &color, 1 }, std::nullopt, dependencies, "Native Flare Render Pass");
		CreatePipeline();
		VkSamplerCreateInfo info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		info.magFilter = VK_FILTER_NEAREST;
		info.minFilter = VK_FILTER_NEAREST;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		info.addressModeU = info.addressModeV = info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		if (vkCreateSampler(GetDevice(), &info, GetAllocator(), &sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create native flare sampler");
		CreateFramebuffer();
	}

	void DestroyFramebuffer()
	{
		if (framebuffer) vkDestroyFramebuffer(GetDevice(), framebuffer, GetAllocator());
		framebuffer = VK_NULL_HANDLE;
	}

	void CreateFramebuffer()
	{
		const std::array attachments{ GetNativeRendererState().frameBuffer.colorImageView };
		framebuffer = Renderer::CreateFramebuffer(renderPass, attachments, gWidth, gHeight, "Native Flare Framebuffer");
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
				throw std::runtime_error("failed to reset native flare descriptor pool");
		}
		descriptorIndex = 0;
	}

	void Record(VkCommandBuffer cmd, const FlareDraw& flare)
	{
		ZONE_SCOPED;
		const auto* texture = flare.pTexture->GetRenderer();
		if (!texture || texture->imageView == VK_NULL_HANDLE) return;
		const VkDescriptorSet set = AllocateDescriptor();
		PS2::SamplerDescription textureSampler;
		textureSampler.addressU = textureSampler.addressV = PS2::TextureAddress::Repeat;
		const std::array imageInfo{
			VkDescriptorImageInfo{ PS2::GetSampler(textureSampler), texture->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			VkDescriptorImageInfo{ sampler, GetNativeRendererState().frameBuffer.depthImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL },
		};
		std::array<VkWriteDescriptorSet, 2> writes{};
		for (uint32_t i = 0; i < writes.size(); ++i) {
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].dstSet = set;
			writes[i].dstBinding = i;
			writes[i].descriptorCount = 1;
			writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writes[i].pImageInfo = &imageInfo[i];
		}
		vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
		Debug::BeginLabel(cmd, "Flare [%s, depth %.6f, occlusion %u]", flare.pTexture->GetName().c_str(), flare.depth, flare.occlusionEnabled);
		VkRenderPassBeginInfo begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		begin.renderPass = renderPass;
		begin.framebuffer = framebuffer;
		begin.renderArea.extent = GetFrameBufferSize();
		vkCmdBeginRenderPass(cmd, &begin, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(gWidth), static_cast<float>(gHeight), 0.0f, 1.0f };
		const VkRect2D scissor{ { 0, 0 }, GetFrameBufferSize() };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &set, 0, nullptr);
		const PushConstants data{
			{ flare.center[0], flare.center[1], flare.halfSize[0], flare.halfSize[1] },
			{ flare.marker[0], flare.marker[1], flare.marker[2], flare.marker[3] },
			{ flare.depth, flare.occlusionEnabled ? 1.0f : 0.0f, 0.0f, 0.0f },
		};
		vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(data), &data);
		vkCmdDraw(cmd, 6, 1, 0, 0);
		vkCmdEndRenderPass(cmd);
		Debug::EndLabel(cmd);
	}
}
