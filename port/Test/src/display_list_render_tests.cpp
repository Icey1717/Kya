#include <gtest/gtest.h>

#include "displaylist.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeDisplayList.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include <GLFW/glfw3.h>
#include <array>

// Run alone: requires Vulkan and compiled shaders, with no visible window.
TEST(DisplayListRendering, DISABLED_QueuedSpritesRetainTopology)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();

	for (int scenario = 0; scenario < 4; ++scenario) {
		const bool mixedTopology = (scenario & 1) != 0;
		const bool cutoff = scenario >= 2;
		SCOPED_TRACE(mixedTopology);
		SCOPED_TRACE(cutoff);
		DrawTrace::InvalidateSources();
		DrawTrace::AdvanceFrame();
		if (cutoff) {
			DrawTrace::RequestCapture(false);
			DrawTrace::AdvanceFrame();
			const auto submission = DrawTrace::Submit({}, 700);
			DrawTrace::Draw draw;
			draw.recorded = true;
			DrawTrace::Record(submission, draw);
			DrawTrace::AdvanceFrame();
			const auto snapshot = DrawTrace::GetSnapshot();
			ASSERT_EQ(snapshot->draws.size(), 1u);
			DrawTrace::SetDrawLimit(*snapshot, snapshot->draws[0]);
			DrawTrace::AdvanceFrame();
		}
		RecordBeginCommandBuffer();
		RecordBeginRenderPass(RenderPassKey{ EClearMode::ColorDepth });
		RecordEndRenderPass();
		RecordEndCommandBuffer();
		auto sceneCmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];
		Debug::EndLabel(sceneCmd);
		ASSERT_EQ(vkEndCommandBuffer(sceneCmd), VK_SUCCESS);
		VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &sceneCmd;
		ASSERT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
		ASSERT_EQ(vkQueueWaitIdle(GetGraphicsQueue()), VK_SUCCESS);

		GIFReg::GSTest test{};
		Renderer::SetTest(test);
		Renderer::DisplayList::BindTexture(GetNativeRendererState().whiteTexture);
		Renderer::DisplayList::Begin2D(100, 100, GS_SPRITE);
		Renderer::DisplayList::SetColor(128, 0, 0, 128, 1);
		Renderer::DisplayList::SetTexCoord(0.5f, 0.5f);
		Renderer::DisplayList::SetVertex(10, 10, 0, 0);
		Renderer::DisplayList::SetVertex(40, 90, 0, 0);
		Renderer::DisplayList::End2D();
		if (mixedTopology) {
			Renderer::DisplayList::Begin2D(100, 100, GS_TRIANGLELIST);
			Renderer::DisplayList::SetColor(0, 128, 0, 128, 1);
			Renderer::DisplayList::SetVertex(50, 10, 0, 0);
			Renderer::DisplayList::SetVertex(90, 10, 0, 0);
			Renderer::DisplayList::SetVertex(70, 90, 0, 0);
			Renderer::DisplayList::End2D();
		}
		// Native scene recording changes this shared state before display-list submission.
		PS2::GetGSState().PRIM.PRIM = GS_TRIANGLELIST;
		auto cmd = Renderer::Native::DisplayList::FinalizeCommandBuffer(false);
		const auto image = GetFrameBuffer().colorImage;
		VulkanBuffer readback(8, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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
		std::array<VkBufferImageCopy, 2> copies{};
		for (int i = 0; i < 2; ++i) {
			copies[i].bufferOffset = i * 4;
			copies[i].imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copies[i].imageOffset = { gWidth * (i == 0 ? 25 : 70) / 100, gHeight / 2, 0 };
			copies[i].imageExtent = { 1, 1, 1 };
		}
		vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), copies.size(), copies.data());
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
		ASSERT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
		submit.pCommandBuffers = &cmd;
		ASSERT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
		ASSERT_EQ(vkQueueWaitIdle(GetGraphicsQueue()), VK_SUCCESS);
		void* mapped = nullptr;
		ASSERT_EQ(vkMapMemory(GetDevice(), readback.Memory(), 0, 8, 0, &mapped), VK_SUCCESS);
		std::array<uint8_t, 8> pixels{};
		memcpy(pixels.data(), mapped, pixels.size());
		vkUnmapMemory(GetDevice(), readback.Memory());
		// The framebuffer stores BGRA bytes.
		EXPECT_EQ(pixels[0], 0);
		EXPECT_EQ(pixels[1], 0);
		if (cutoff) EXPECT_EQ(pixels[2], 0); // Later game overlays must not cover the partial scene.
		else EXPECT_GT(pixels[2], 240); // Red sprite survives the later GS primitive change.
		EXPECT_EQ(pixels[4], 0);
		EXPECT_EQ(pixels[5], mixedTopology && !cutoff ? 255 : 0);
		EXPECT_EQ(pixels[6], 0);
	}
	DrawTrace::InvalidateSources();
	DrawTrace::AdvanceFrame();
	DrawTrace::ClearSnapshot();
	Renderer::Native::Cleanup();
}
