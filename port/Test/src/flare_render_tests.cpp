#include <gtest/gtest.h>

#include "FlareDraw.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeFlare.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include <GLFW/glfw3.h>
#include <array>

namespace
{
	std::array<uint8_t, 4> ReadFlarePixel(VkCommandBuffer cmd)
	{
		using namespace Renderer;
		using namespace Renderer::Native;
		const auto image = GetNativeRendererState().frameBuffer.colorImage;
		VulkanBuffer readback(4, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		barrier.oldLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
		VkBufferImageCopy copy{};
		copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy.imageOffset = { gWidth / 2, gHeight / 2, 0 };
		copy.imageExtent = { 1, 1, 1 };
		vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), 1, &copy);
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
		Debug::EndLabel(cmd);
		EXPECT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
		VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;
		EXPECT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
		EXPECT_EQ(vkQueueWaitIdle(GetGraphicsQueue()), VK_SUCCESS);
		void* mapped = nullptr;
		EXPECT_EQ(vkMapMemory(GetDevice(), readback.Memory(), 0, 4, 0, &mapped), VK_SUCCESS);
		std::array<uint8_t, 4> pixel{};
		if (mapped) memcpy(pixel.data(), mapped, 4);
		vkUnmapMemory(GetDevice(), readback.Memory());
		return pixel;
	}
}

// Opt-in: requires a Vulkan device and the compiled shaders, but opens no visible window.
// Run alone with --gtest_also_run_disabled_tests --gtest_filter=FlareRendering.*
TEST(FlareRendering, DISABLED_VisibilityQueueAndResize)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();
	auto& state = GetNativeRendererState();
	FlareDraw flare;
	flare.pTexture = state.whiteTexture;
	flare.center[0] = flare.center[1] = 0.5f;
	flare.halfSize[0] = flare.halfSize[1] = 0.1f;
	flare.marker[0] = flare.marker[1] = 0.5f;
	flare.marker[2] = flare.marker[3] = 8.0f / 512.0f;
	flare.depth = 0.5f;

	auto render = [&](float sceneDepth, bool partial, bool occlusion) {
		RecordBeginCommandBuffer();
		RecordBeginRenderPass(RenderPassKey{ EClearMode::ColorDepth });
		VkClearAttachment attachment{};
		attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachment.clearValue.depthStencil = { sceneDepth, 0 };
		VkClearRect rect{ { { 0, 0 }, GetFrameBufferSize() }, 0, 1 };
		if (partial) rect.rect.extent.width /= 2;
		vkCmdClearAttachments(state.commandBuffers[GetCurrentFrame()], 1, &attachment, 1, &rect);
		RecordEndRenderPass();
		flare.occlusionEnabled = occlusion;
		Flare::Record(state.commandBuffers[GetCurrentFrame()], flare);
		RecordEndCommandBuffer();
		return ReadFlarePixel(state.commandBuffers[GetCurrentFrame()]);
	};
	const auto visible = render(0.0f, false, true);
	const auto hidden = render(0.75f, false, true);
	const auto partial = render(0.75f, true, true);
	const auto bypass = render(0.75f, false, false);
	for (int i = 0; i < 3; ++i) {
		EXPECT_GT(visible[i], 0);
		EXPECT_EQ(hidden[i], 0);
		EXPECT_GT(partial[i], hidden[i]);
		EXPECT_LT(partial[i], visible[i]);
		EXPECT_EQ(bypass[i], visible[i]);
	}
	EXPECT_EQ(visible[3], 255); // Additive flares preserve framebuffer alpha.

	// Exercise the copied RenderThread command, including a pending scene clear.
	UpdateRenderPassKey(EClearMode::ColorDepth);
	SubmitFlare(flare);
	MainThreadEndCommands(state.renderThread);
	const auto queued = ReadFlarePixel(state.commandBuffers[GetCurrentFrame()]);
	EXPECT_EQ(queued, visible);
	ResetRenderThread(state.renderThread);

	// Exceed one descriptor-pool block and verify consecutive flare commands LOAD color.
	UpdateRenderPassKey(EClearMode::ColorDepth);
	for (int i = 0; i < 260; ++i) SubmitFlare(flare);
	MainThreadEndCommands(state.renderThread);
	const auto accumulated = ReadFlarePixel(state.commandBuffers[GetCurrentFrame()]);
	for (int i = 0; i < 3; ++i) EXPECT_EQ(accumulated[i], 255);
	EXPECT_EQ(accumulated[3], 255);
	ResetRenderThread(state.renderThread);

	ResizeFrameBuffer(768, 448);
	ApplyPendingResizeInternal();
	EXPECT_EQ(render(0.0f, false, true), visible);
	Renderer::Native::Cleanup();
}
