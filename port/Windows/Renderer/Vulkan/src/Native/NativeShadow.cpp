#include "NativeShadow.h"

#include "NativeDebug.h"
#include "NativeRendererInternal.h"
#include "Objects/VulkanImage.h"
#include "Objects/VulkanRenderPass.h"
#include "Objects/VulkanShader.h"
#include "Objects/VulkanBuffer.h"
#include "Objects/VulkanCommands.h"
#include "VulkanRenderer.h"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../ext/glfw/deps/stb_image_write.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace Renderer::Native::Shadow
{
	ShadowPassSettings NormalizeSettings(const ShadowPassSettings& settings, uint32_t resolutionScale = 1)
	{
		ShadowPassSettings normalized = settings;
		resolutionScale = std::clamp(resolutionScale, 1u, 8u);
		auto scale = [resolutionScale](uint32_t value) {
			return static_cast<uint32_t>(std::min(uint64_t(value) * resolutionScale,
				uint64_t(std::numeric_limits<uint32_t>::max())));
		};
		normalized.width = scale(std::max(normalized.width, 1u));
		normalized.height = scale(std::max(normalized.height, 1u));
		normalized.blurRadius = scale(normalized.blurRadius);
		normalized.blurSamples = std::min(normalized.blurSamples, 32u);
		return normalized;
	}

	namespace
	{
		constexpr VkFormat kShadowFormat = VK_FORMAT_R8_UNORM;

		struct BlurPushConstants
		{
			glm::vec2 texelSize;
			uint32_t sampleCount;
			float radius;
		};

		struct ShadowTarget
		{
			VkExtent2D extent{};
			OwnedImage mask;
			OwnedImage depth;
			OwnedImage blur;
			VkFramebuffer maskFramebuffer = VK_NULL_HANDLE;
			VkFramebuffer blurFramebuffer = VK_NULL_HANDLE;
			VkSampler sampler = VK_NULL_HANDLE;
			VkDescriptorPool blurDescriptorPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet> blurDescriptorSets;
			VkDescriptorPool receiverDescriptorPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet> receiverDescriptorSets;
			bool blurOutputValid = false;

			void Destroy()
			{
				if (maskFramebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(GetDevice(), maskFramebuffer, GetAllocator());
				if (blurFramebuffer != VK_NULL_HANDLE) vkDestroyFramebuffer(GetDevice(), blurFramebuffer, GetAllocator());
				if (sampler != VK_NULL_HANDLE) vkDestroySampler(GetDevice(), sampler, GetAllocator());
				if (blurDescriptorPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(GetDevice(), blurDescriptorPool, GetAllocator());
				if (receiverDescriptorPool != VK_NULL_HANDLE) vkDestroyDescriptorPool(GetDevice(), receiverDescriptorPool, GetAllocator());
				maskFramebuffer = VK_NULL_HANDLE;
				blurFramebuffer = VK_NULL_HANDLE;
				sampler = VK_NULL_HANDLE;
				blurDescriptorPool = VK_NULL_HANDLE;
				receiverDescriptorPool = VK_NULL_HANDLE;
				mask.Destroy();
				depth.Destroy();
				blur.Destroy();
			}
		};

		VkRenderPass gBlurRenderPass = VK_NULL_HANDLE;
		Pipeline gBlurPipeline;
		VkFramebuffer gReceiverFramebuffer = VK_NULL_HANDLE;
		std::unordered_map<uint64_t, ShadowTarget> gTargets;
		ShadowTarget* gpActiveTarget = nullptr;
		ShadowTarget* gpDebugTarget = nullptr;
		ShadowPassSettings gSettings{};
		ShadowReceiverViewport gReceiverViewport{};
		std::mutex gDebugTargetMutex;
		bool gDumpRequested = false;
		std::string gDumpStatus;
		ShadowCasterDiagnostics gCasterDiagnostics{};
		bool gHasCasterDiagnostics = false;
		uint32_t gMaskDrawCount = 0;
		std::atomic<uint32_t> gResolutionScale{1};

		// Caller holds gDebugTargetMutex. Format only for UI/export requests.
		std::string FormatCasterDebugInfo()
		{
			std::ostringstream report;
			if (gHasCasterDiagnostics) {
				const auto& counts = gCasterDiagnostics;
				report << "Latest shadow traversal (CPU; counts do not prove visible pixels):\n"
					<< "Render mask: 0x" << std::hex << counts.renderMask << std::dec
					<< "\nHierarchies visited: " << counts.hierarchyVisited << "; eligible: " << counts.hierarchyEligible
					<< "; missing LOD: " << counts.missingLod
					<< "\nObjects culled: " << counts.objectCulled << "; strips visited: " << counts.stripVisited
					<< "\nStrips rejected: mask " << counts.maskRejected << "; material " << counts.materialRejected
					<< "; animation " << counts.animationRejected << "; culling " << counts.stripCulled
					<< "\nCaster strips linked: " << counts.casterLinked << "; flushed: " << counts.casterFlushed
					<< std::fixed << std::setprecision(3)
					<< "\nShadow camera position: (" << counts.cameraPosition[0] << ", " << counts.cameraPosition[1] << ", " << counts.cameraPosition[2] << ')'
					<< "\nShadow camera target: (" << counts.cameraTarget[0] << ", " << counts.cameraTarget[1] << ", " << counts.cameraTarget[2] << ')'
					<< "\nShadow camera direction: (" << counts.cameraTarget[0] - counts.cameraPosition[0] << ", "
					<< counts.cameraTarget[1] - counts.cameraPosition[1] << ", " << counts.cameraTarget[2] - counts.cameraPosition[2] << ')'
					<< "\nLight manager shadow direction: (" << counts.lightDirection[0] << ", " << counts.lightDirection[1] << ", "
					<< counts.lightDirection[2] << ", " << counts.lightDirection[3] << "); intensity " << counts.lightIntensity
					<< "; active lights " << counts.activeLights << '\n';
			}
			report << "Latest mask pass recorded draw calls: " << gMaskDrawCount << '\n';
			return report.str();
		}

		uint64_t GetTargetKey(uint32_t width, uint32_t height)
		{
			return (static_cast<uint64_t>(width) << 32) | height;
		}

		std::vector<VkDescriptorSet> AllocateDescriptorSets(
			const Pipeline& pipeline,
			VkDescriptorPool& descriptorPool,
			const char* debugName, int setIndex = 0)
		{
			CreateDescriptorPool(pipeline.descriptorSetLayoutBindings, descriptorPool, setIndex);
			std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline.descriptorSetLayouts.at(setIndex));
			std::vector<VkDescriptorSet> sets(MAX_FRAMES_IN_FLIGHT);

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(sets.size());
			allocInfo.pSetLayouts = layouts.data();
			if (vkAllocateDescriptorSets(GetDevice(), &allocInfo, sets.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate native shadow descriptor sets");
			}

			for (uint32_t i = 0; i < sets.size(); ++i) {
				SetObjectName(reinterpret_cast<uint64_t>(sets[i]), VK_OBJECT_TYPE_DESCRIPTOR_SET, "%s %u", debugName, i);
			}
			return sets;
		}

		void CreateBlurRenderPass()
		{
			const AttachmentInfo colorInfo{
				kShadowFormat,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
			const std::array dependencies{
				VkSubpassDependency{
					VK_SUBPASS_EXTERNAL, 0,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
				},
				VkSubpassDependency{
					0, VK_SUBPASS_EXTERNAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
				},
			};
			gBlurRenderPass = CreateRenderPass2D({ &colorInfo, 1 }, std::nullopt, dependencies, "Native Shadow Blur Render Pass");
		}

		void CreateBlurPipeline()
		{
			gBlurPipeline.debugName = "Native Shadow Blur Pipeline";
			auto vertShader = Shader::ReflectedModule("shaders/postprocess.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			auto fragShader = Shader::ReflectedModule("shaders/shadow_blur.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
			gBlurPipeline.AddBindings(EBindingStage::Vertex, vertShader.reflectData);
			gBlurPipeline.AddBindings(EBindingStage::Fragment, fragShader.reflectData);
			gBlurPipeline.CreateDescriptorSetLayouts();
			gBlurPipeline.CreateLayout();

			const std::array shaderStages{ vertShader.shaderStageCreateInfo, fragShader.shaderStageCreateInfo };
			VkPipelineVertexInputStateCreateInfo vertexInput{};
			vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;
			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.cullMode = VK_CULL_MODE_NONE;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.lineWidth = 1.0f;
			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			VkPipelineColorBlendAttachmentState blendAttachment{};
			blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
			VkPipelineColorBlendStateCreateInfo blending{};
			blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blending.attachmentCount = 1;
			blending.pAttachments = &blendAttachment;
			const std::array dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();
			VkPipelineDepthStencilStateCreateInfo depthState{};
			depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

			VkGraphicsPipelineCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			info.stageCount = static_cast<uint32_t>(shaderStages.size());
			info.pStages = shaderStages.data();
			info.pVertexInputState = &vertexInput;
			info.pInputAssemblyState = &inputAssembly;
			info.pViewportState = &viewportState;
			info.pRasterizationState = &rasterizer;
			info.pMultisampleState = &multisampling;
			info.pColorBlendState = &blending;
			info.pDepthStencilState = &depthState;
			info.pDynamicState = &dynamicState;
			info.layout = gBlurPipeline.layout;
			info.renderPass = gBlurRenderPass;
			if (vkCreateGraphicsPipelines(GetDevice(), VK_NULL_HANDLE, 1, &info, GetAllocator(), &gBlurPipeline.pipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create native shadow blur pipeline");
			}
			SetObjectName(reinterpret_cast<uint64_t>(gBlurPipeline.pipeline), VK_OBJECT_TYPE_PIPELINE, "Native Shadow Blur Pipeline");
		}

		void CreateSampler(VkSampler& sampler)
		{
			VkSamplerCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			if (vkCreateSampler(GetDevice(), &info, GetAllocator(), &sampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create native shadow sampler");
			}
		}

		void UpdateTargetDescriptors(ShadowTarget& target)
		{
			const Pipeline& receiverPipeline = GetNativeRendererState().renderPass.at(RenderPassKey{ EClearMode::None, ERenderPassKind::ShadowReceiver }).GetPipeline();
			target.blurDescriptorSets = AllocateDescriptorSets(gBlurPipeline, target.blurDescriptorPool, "Shadow Blur Descriptor");
			target.receiverDescriptorSets = AllocateDescriptorSets(receiverPipeline, target.receiverDescriptorPool, "Shadow Receiver Descriptor", 1);

			for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
				VkDescriptorImageInfo maskInfo{ target.sampler, target.mask.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
				DescriptorWriteList blurWrites;
				blurWrites.EmplaceWrite({ 0, EBindingStage::Fragment, nullptr, &maskInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
				auto blurDescriptorWrites = blurWrites.CreateWriteDescriptorSetList(target.blurDescriptorSets[i], gBlurPipeline.descriptorSetLayoutBindings);
				vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(blurDescriptorWrites.size()), blurDescriptorWrites.data(), 0, nullptr);

				VkDescriptorImageInfo shadowInfo{ target.sampler, target.blur.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
				DescriptorWriteList receiverWrites;
				receiverWrites.EmplaceWrite({ 0, EBindingStage::Fragment, nullptr, &shadowInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
				auto receiverDescriptorWrites = receiverWrites.CreateWriteDescriptorSetList(target.receiverDescriptorSets[i], receiverPipeline.descriptorSetLayoutBindings, 1);
				vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(receiverDescriptorWrites.size()), receiverDescriptorWrites.data(), 0, nullptr);
			}
		}

		ShadowTarget& GetOrCreateTarget(uint32_t width, uint32_t height)
		{
			width = std::max(width, 1u);
			height = std::max(height, 1u);
			const uint64_t key = GetTargetKey(width, height);
			auto [it, inserted] = gTargets.try_emplace(key);
			if (!inserted) return it->second;

			ShadowTarget& target = it->second;
			target.extent = { width, height };
			target.mask = VulkanImage::CreateColor(width, height, kShadowFormat, 0);
			target.depth = VulkanImage::CreateDepth(width, height);
			target.blur = VulkanImage::CreateColor(width, height, kShadowFormat, 0);
			const std::array maskAttachments{ target.mask.view, target.depth.view };
			target.maskFramebuffer = CreateFramebuffer(
				GetNativeRendererState().renderPass.at(RenderPassKey{ EClearMode::ColorDepth, ERenderPassKind::ShadowMask }).gRenderPass,
				maskAttachments, width, height, "Native Shadow Mask Framebuffer");
			const std::array blurAttachments{ target.blur.view };
			target.blurFramebuffer = CreateFramebuffer(gBlurRenderPass, blurAttachments, width, height, "Native Shadow Blur Framebuffer");
			CreateSampler(target.sampler);
			UpdateTargetDescriptors(target);
			return target;
		}
	}

	void Setup()
	{
		CreateBlurRenderPass();
		CreateBlurPipeline();
		CreateReceiverFramebuffer();
	}

	void Cleanup()
	{
		std::lock_guard lock(gDebugTargetMutex);
		DestroyReceiverFramebuffer();
		for (auto& [key, target] : gTargets) target.Destroy();
		gTargets.clear();
		gpActiveTarget = nullptr;
		gpDebugTarget = nullptr;
		gDumpRequested = false;
		gDumpStatus.clear();
		gCasterDiagnostics = {};
		gHasCasterDiagnostics = false;
		gMaskDrawCount = 0;
		gBlurPipeline.Destroy();
		if (gBlurRenderPass != VK_NULL_HANDLE) vkDestroyRenderPass(GetDevice(), gBlurRenderPass, GetAllocator());
		gBlurRenderPass = VK_NULL_HANDLE;
	}

	void DestroyReceiverFramebuffer()
	{
		if (gReceiverFramebuffer != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(GetDevice(), gReceiverFramebuffer, GetAllocator());
			gReceiverFramebuffer = VK_NULL_HANDLE;
		}
	}

	void CreateReceiverFramebuffer()
	{
		const std::array attachments{
			GetNativeRendererState().frameBuffer.colorImageView,
			GetNativeRendererState().frameBuffer.depthImageView,
		};
		gReceiverFramebuffer = CreateFramebuffer(
			GetNativeRendererState().renderPass.at(RenderPassKey{ EClearMode::None, ERenderPassKind::ShadowReceiver }).gRenderPass,
			attachments,
			static_cast<uint32_t>(gWidth),
			static_cast<uint32_t>(gHeight),
			"Native Shadow Receiver Framebuffer");
	}

	void BeginMask(const ShadowPassSettings& settings)
	{
		uint32_t scale = gResolutionScale.load(std::memory_order_relaxed);
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(GetPhysicalDevice(), &properties);
		const uint32_t maxDimension = std::min({ properties.limits.maxImageDimension2D,
			properties.limits.maxFramebufferWidth, properties.limits.maxFramebufferHeight });
		// Reduce both axes together to preserve the authored aspect ratio.
		while (scale > 1 && (uint64_t(std::max(settings.width, 1u)) * scale > maxDimension ||
			uint64_t(std::max(settings.height, 1u)) * scale > maxDimension)) scale /= 2;
		const ShadowPassSettings normalized = NormalizeSettings(settings, scale);
		std::lock_guard lock(gDebugTargetMutex);
		gpActiveTarget = &GetOrCreateTarget(normalized.width, normalized.height);
		gSettings = normalized;
		gMaskDrawCount = 0;
		gpDebugTarget = gpActiveTarget;
	}

	void RecordBlur(const VkCommandBuffer& cmd)
	{
		if (!gpActiveTarget) return;
		Renderer::Debug::BeginLabel(cmd, "Native Shadow Blur [%ux%u, samples: %u, radius: %u]",
			gpActiveTarget->extent.width, gpActiveTarget->extent.height, gSettings.blurSamples, gSettings.blurRadius);

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.renderPass = gBlurRenderPass;
		beginInfo.framebuffer = gpActiveTarget->blurFramebuffer;
		beginInfo.renderArea.extent = gpActiveTarget->extent;
		vkCmdBeginRenderPass(cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(gpActiveTarget->extent.width), static_cast<float>(gpActiveTarget->extent.height), 0.0f, 1.0f };
		VkRect2D scissor{ { 0, 0 }, gpActiveTarget->extent };
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gBlurPipeline.pipeline);
		const VkDescriptorSet& set = gpActiveTarget->blurDescriptorSets[GetCurrentFrame()];
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gBlurPipeline.layout, 0, 1, &set, 0, nullptr);
		const BlurPushConstants constants{
			{ 1.0f / gpActiveTarget->extent.width, 1.0f / gpActiveTarget->extent.height },
			gSettings.blurSamples,
			static_cast<float>(gSettings.blurRadius),
		};
		vkCmdPushConstants(cmd, gBlurPipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), &constants);
		vkCmdDraw(cmd, 3, 1, 0, 0);
		vkCmdEndRenderPass(cmd);
		{
			std::lock_guard lock(gDebugTargetMutex);
			gpActiveTarget->blurOutputValid = true;
		}
		Renderer::Debug::EndLabel(cmd);
	}

	void BeginReceiver(const ShadowReceiverViewport& viewport)
	{
		gReceiverViewport = viewport;
	}

	void End()
	{
		gpActiveTarget = nullptr;
		gReceiverViewport = {};
	}

	VkFramebuffer GetFramebuffer(ERenderPassKind kind)
	{
		if (kind == ERenderPassKind::ShadowMask) {
			if (!gpActiveTarget) throw std::runtime_error("shadow mask pass has no active target");
			return gpActiveTarget->maskFramebuffer;
		}
		if (kind == ERenderPassKind::ShadowReceiver) return gReceiverFramebuffer;
		return GetNativeRendererState().frameBuffer.framebuffer;
	}

	VkExtent2D GetExtent(ERenderPassKind kind)
	{
		if (kind == ERenderPassKind::ShadowMask && gpActiveTarget) return gpActiveTarget->extent;
		return { static_cast<uint32_t>(gWidth), static_cast<uint32_t>(gHeight) };
	}

	VkRect2D GetReceiverScissor()
	{
		const int32_t x = std::clamp(gReceiverViewport.x, 0, gWidth);
		const int32_t y = std::clamp(gReceiverViewport.y, 0, gHeight);
		const uint32_t availableWidth = static_cast<uint32_t>(gWidth - x);
		const uint32_t availableHeight = static_cast<uint32_t>(gHeight - y);
		const uint32_t width = gReceiverViewport.width ? std::min(gReceiverViewport.width, availableWidth) : availableWidth;
		const uint32_t height = gReceiverViewport.height ? std::min(gReceiverViewport.height, availableHeight) : availableHeight;
		return { { x, y }, { width, height } };
	}

	const VkDescriptorSet& GetReceiverDescriptorSet(uint32_t frameIndex)
	{
		if (!gpActiveTarget) throw std::runtime_error("shadow receiver pass has no active target");
		return gpActiveTarget->receiverDescriptorSets.at(frameIndex);
	}

	VkSampler GetSampler()
	{
		std::lock_guard lock(gDebugTargetMutex);
		return gpDebugTarget ? gpDebugTarget->sampler : VK_NULL_HANDLE;
	}

	VkImageView GetMaskImageView()
	{
		std::lock_guard lock(gDebugTargetMutex);
		return gpDebugTarget && gpDebugTarget->blurOutputValid ? gpDebugTarget->mask.view : VK_NULL_HANDLE;
	}

	VkImageView GetBlurImageView()
	{
		std::lock_guard lock(gDebugTargetMutex);
		return gpDebugTarget && gpDebugTarget->blurOutputValid ? gpDebugTarget->blur.view : VK_NULL_HANDLE;
	}

	ShadowPassSettings GetSettings()
	{
		std::lock_guard lock(gDebugTargetMutex);
		return gSettings;
	}

	void ProcessPendingDump()
	{
		std::unique_lock lock(gDebugTargetMutex);
		if (!gDumpRequested) return;
		gDumpRequested = false;
		if (!gpDebugTarget || !gpDebugTarget->blurOutputValid) {
			gDumpStatus = "No completed shadow buffers available to dump.";
			return;
		}
		try {
			const auto settings = gSettings;
			const auto extent = gpDebugTarget->extent;
			const VkDeviceSize byteCount = VkDeviceSize(extent.width) * extent.height;
			std::array<VulkanBuffer, 2> readback;
			for (auto& buffer : readback) buffer.Create(byteCount, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			const std::array images{ gpDebugTarget->mask.image, gpDebugTarget->blur.image };
			const auto cmd = BeginSingleTimeCommands();
			for (size_t i = 0; i < images.size(); ++i) {
				VulkanImage::TransitionImageLayout(images[i], kShadowFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
				VkBufferImageCopy copy{};
				copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
				copy.imageExtent = { extent.width, extent.height, 1 };
				vkCmdCopyImageToBuffer(cmd, images[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback[i].Get(), 1, &copy);
				VulkanImage::TransitionImageLayout(images[i], kShadowFormat, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
			}
			EndSingleTimeCommands(cmd); // Queue order and idle wait complete the frame and readback.
			std::array<std::vector<unsigned char>, 2> pixels;
			for (size_t i = 0; i < pixels.size(); ++i) {
				pixels[i].resize(static_cast<size_t>(byteCount));
				void* mapped = nullptr;
				if (vkMapMemory(GetDevice(), readback[i].Memory(), 0, byteCount, 0, &mapped) != VK_SUCCESS)
					throw std::runtime_error("Could not map shadow buffer pixels");
				std::memcpy(pixels[i].data(), mapped, pixels[i].size());
				vkUnmapMemory(GetDevice(), readback[i].Memory());
			}
			const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count();
			const auto directory = std::filesystem::absolute(std::filesystem::path("logs") / "shadows" / ("shadow-" + std::to_string(timestamp)));
			std::filesystem::create_directories(directory);
			const std::array names{ "caster-mask.png", "blur-output.png" };
			for (size_t i = 0; i < pixels.size(); ++i) {
				if (!stbi_write_png((directory / names[i]).string().c_str(), static_cast<int>(extent.width),
					static_cast<int>(extent.height), 1, pixels[i].data(), static_cast<int>(extent.width)))
					throw std::runtime_error("Could not write shadow PNG");
			}
			std::ofstream report(directory / "settings.txt");
			report << "Native Shadow Buffers\nSize: " << extent.width << 'x' << extent.height
				<< "\nRequested resolution scale: " << gResolutionScale.load(std::memory_order_relaxed) << 'x'
				<< "\nBlur samples: " << settings.blurSamples << "\nBlur radius: " << settings.blurRadius
				<< "\nAlpha (GS units): " << settings.alpha
				<< "\nFormat: R8_UNORM; black = no coverage, white = full coverage.\n"
				<< "Latest completed shadow target; may be retained from an earlier frame.\n";
			report << FormatCasterDebugInfo();
			report.close();
			if (!report) throw std::runtime_error("Could not write shadow settings");
			gDumpStatus = "Saved shadow buffers to " + directory.string();
		}
		catch (const std::exception& error) {
			gDumpStatus = std::string("Shadow buffer dump failed: ") + error.what();
		}
	}
}

void Renderer::Native::RequestShadowBufferDump()
{
	std::lock_guard lock(Shadow::gDebugTargetMutex);
	Shadow::gDumpRequested = true;
	Shadow::gDumpStatus = "Shadow buffer dump queued for frame submission.";
}

std::string Renderer::Native::GetShadowBufferDumpStatus()
{
	std::lock_guard lock(Shadow::gDebugTargetMutex);
	return Shadow::gDumpStatus;
}

void Renderer::Native::SetShadowCasterDiagnostics(const ShadowCasterDiagnostics& diagnostics)
{
	std::lock_guard lock(Shadow::gDebugTargetMutex);
	Shadow::gCasterDiagnostics = diagnostics;
	Shadow::gHasCasterDiagnostics = true;
}

std::string Renderer::Native::GetShadowCasterDebugInfo()
{
	std::lock_guard lock(Shadow::gDebugTargetMutex);
	return Shadow::FormatCasterDebugInfo();
}

void Renderer::Native::RecordShadowMaskDraw()
{
	std::lock_guard lock(Shadow::gDebugTargetMutex);
	++Shadow::gMaskDrawCount;
}

void Renderer::Native::SetShadowResolutionScale(uint32_t scale)
{
	// The menu exposes powers of two, keeping the target cache bounded.
	scale = scale >= 8 ? 8 : scale >= 4 ? 4 : scale >= 2 ? 2 : 1;
	Shadow::gResolutionScale.store(scale, std::memory_order_relaxed);
}

uint32_t Renderer::Native::GetShadowResolutionScale()
{
	return Shadow::gResolutionScale.load(std::memory_order_relaxed);
}

Renderer::Native::ShadowPassSettings Renderer::Native::NormalizeShadowPassSettings(const ShadowPassSettings& settings, uint32_t resolutionScale)
{
	return Shadow::NormalizeSettings(settings, resolutionScale);
}

bool Renderer::Native::HasShadowTarget()
{
	return Shadow::GetMaskImageView() != VK_NULL_HANDLE;
}

VkSampler Renderer::Native::GetShadowSampler()
{
	return Shadow::GetSampler();
}

VkImageView Renderer::Native::GetShadowMaskImageView()
{
	return Shadow::GetMaskImageView();
}

VkImageView Renderer::Native::GetShadowBlurImageView()
{
	return Shadow::GetBlurImageView();
}

Renderer::Native::ShadowPassSettings Renderer::Native::GetShadowPassSettings()
{
	return Shadow::GetSettings();
}
