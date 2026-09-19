#include "NativeFrameBufferCopy.h"

#include "NativeRendererInternal.h"
#include "Objects/VulkanImage.h"

#include <array>
#include <stdexcept>

namespace Renderer::Native::FrameBufferCopy
{
	namespace
	{
		constexpr uint32_t captureSize = 512;
		std::array<OwnedImage, MAX_FRAMES_IN_FLIGHT> images;
		std::array<bool, MAX_FRAMES_IN_FLIGHT> initialized{};
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptors{};
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;

		void Barrier(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkPipelineStageFlags dstStage, VkAccessFlags dstAccess)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcAccessMask = srcAccess;
			barrier.dstAccessMask = dstAccess;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}
	}

	void Setup()
	{
		auto& state = GetNativeRendererState();
		const auto& pipeline = state.renderPass.at(RenderPassKey{ EClearMode::None }).GetPipeline();
		VkFormatProperties properties{};
		vkGetPhysicalDeviceFormatProperties(GetPhysicalDevice(), GetSwapchainImageFormat(), &properties);
		const VkFormatFeatureFlags required = VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
			VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
		if ((properties.optimalTilingFeatures & required) != required) {
			throw std::runtime_error("native framebuffer capture requires linear color blit support");
		}

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		if (vkCreateSampler(GetDevice(), &samplerInfo, GetAllocator(), &sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer capture sampler");
		}
		CreateDescriptorPool(pipeline.descriptorSetLayoutBindings, descriptorPool);
		std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts;
		layouts.fill(pipeline.descriptorSetLayouts.at(0));
		VkDescriptorSetAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocation.descriptorPool = descriptorPool;
		allocation.descriptorSetCount = static_cast<uint32_t>(descriptors.size());
		allocation.pSetLayouts = layouts.data();
		if (vkAllocateDescriptorSets(GetDevice(), &allocation, descriptors.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate framebuffer capture descriptors");
		}

		for (uint32_t i = 0; i < images.size(); ++i) {
			images[i] = VulkanImage::CreateColor(captureSize, captureSize);
			SetObjectName(reinterpret_cast<uint64_t>(images[i].image), VK_OBJECT_TYPE_IMAGE, "Native Framebuffer Capture %u", i);
			SetObjectName(reinterpret_cast<uint64_t>(descriptors[i]), VK_OBJECT_TYPE_DESCRIPTOR_SET, "Native Framebuffer Material %u", i);
			const VkDescriptorImageInfo imageInfo{ sampler, images[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			const auto modelInfo = state.modelBuffer.GetDescBufferInfo(i);
			const auto animationInfo = state.animationBuffer.GetDescBufferInfo(i);
			const auto lightingInfo = state.lightingDynamicBuffer.GetDescBufferInfo(i);
			const auto animStInfo = state.animStBuffer.GetDescBufferInfo(i);
			DescriptorWriteList writes;
			writes.EmplaceWrite({ 1, EBindingStage::Fragment, nullptr, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });
			writes.EmplaceWrite({ 2, EBindingStage::Vertex, &modelInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
			writes.EmplaceWrite({ 3, EBindingStage::Vertex, &animationInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
			writes.EmplaceWrite({ 4, EBindingStage::Vertex, &lightingInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
			writes.EmplaceWrite({ 5, EBindingStage::Vertex, &animStInfo, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
			auto descriptorWrites = writes.CreateWriteDescriptorSetList(descriptors[i], pipeline.descriptorSetLayoutBindings);
			vkUpdateDescriptorSets(GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void Cleanup()
	{
		if (descriptorPool) vkDestroyDescriptorPool(GetDevice(), descriptorPool, GetAllocator());
		if (sampler) vkDestroySampler(GetDevice(), sampler, GetAllocator());
		descriptorPool = VK_NULL_HANDLE;
		sampler = VK_NULL_HANDLE;
		for (auto& image : images) image.Destroy();
		initialized.fill(false);
		descriptors.fill(VK_NULL_HANDLE);
	}

	void Record(VkCommandBuffer cmd)
	{
		const auto frame = GetCurrentFrame();
		const auto source = GetNativeRendererState().frameBuffer.colorImage;
		const auto destination = images[frame].image;
		Debug::BeginLabel(cmd, "Framebuffer Capture (GS base 0)");
		// EndRenderPass leaves the native color attachment in READ_ONLY_OPTIMAL.
		// ALL_COMMANDS also covers the frame-start clear if there were no preceding draws.
		Barrier(cmd, source, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_MEMORY_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		// A later list may replace the same snapshot after earlier effects sampled it.
		Barrier(cmd, destination, initialized[frame] ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			initialized[frame] ? VK_ACCESS_SHADER_READ_BIT : 0,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

		VkImageBlit blit{};
		blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		blit.dstSubresource = blit.srcSubresource;
		blit.srcOffsets[1] = { gWidth, gHeight, 1 };
		blit.dstOffsets[1] = { captureSize, captureSize, 1 };
		// Native approximation of the PS2 strip resample. Exact half-texel and copy-alpha
		// behavior can be compared against the GS packet using this labelled capture.
		vkCmdBlitImage(cmd, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		Barrier(cmd, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
		Barrier(cmd, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		// The continuation pass LOADs depth from the pass that just ended.
		VkMemoryBarrier depthBarrier{};
		depthBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		depthBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			0, 1, &depthBarrier, 0, nullptr, 0, nullptr);
		initialized[frame] = true;
		Debug::EndLabel(cmd);
	}

	const VkDescriptorSet& GetDescriptorSet(int frame)
	{
		return descriptors[frame];
	}
}
