#include <gtest/gtest.h>
#include "displaylist.h"
#include "port.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeDisplayList.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanImage.h"
#include <GLFW/glfw3.h>
#include <array>

// Run alone: requires Vulkan and compiled shaders, with no visible window.
TEST(DisplayListBlendingRendering, DISABLED_GsColorBytesAndPanelAlpha)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();

	CombinedImageData image{};
	image.bitmaps.resize(1);
	image.registers.tex.TBW = 1;
	image.registers.tex.TBP0 = 0x100;
	image.registers.alpha.CMD = SCE_GS_SET_ALPHA_1(0, 1, 0, 1, 0);
	uint32_t texel = 0x80808080; // Neutral GS modulation color, including alpha.
	auto& bitmap = image.bitmaps[0];
	bitmap.canvasWidth = bitmap.canvasHeight = bitmap.maxMipLevel = 1;
	bitmap.bitBltBuf.DBP = 0x100;
	bitmap.bitBltBuf.DBW = 1;
	bitmap.trxReg.RRW = bitmap.trxReg.RRH = 1;
	bitmap.pImage = &texel;
	SimpleTexture texture("Display-list neutral modulation", { 0, 0, 1, 1 }, image.registers);
	texture.CreateRenderer(image);
	auto draw = [](float left, float right, uint8_t color, uint8_t alpha) {
		Renderer::DisplayList::Begin2D(100, 100, GS_TRIANGLELIST);
		Renderer::DisplayList::SetColor(color, color, color, alpha, 1);
		Renderer::DisplayList::SetTexCoord(0.5f, 0.5f);
		for (const auto& xy : std::array<std::array<float, 2>, 6>{ {
			{ left, 0 }, { right, 0 }, { left, 100 },
			{ right, 0 }, { right, 100 }, { left, 100 } } }) {
			Renderer::DisplayList::SetVertex(xy[0], xy[1], 0, 0);
		}
		Renderer::DisplayList::End2D();
	};

	for (uint8_t alpha : { 0, 64, 128 }) {
		SCOPED_TRACE(int(alpha));
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

		Renderer::SetTest(GIFReg::GSTest{});
		Renderer::DisplayList::BindTexture(nullptr);
		Renderer::SetAlpha(0, 1, 2, 1, 128); // Copy background color bytes.
		draw(0, 100, 64, 128);
		Renderer::DisplayList::BindTexture(nullptr); // Flush before changing ALPHA.
		Renderer::SetAlpha(1, 2, 0, 1, 0); // Captured panel: Cd * (1 + As/128).
		draw(0, 33, 127, alpha);
		Renderer::DisplayList::BindTexture(nullptr);
		Renderer::SetAlpha(0, 1, 0, 1, 0); // Standard source-alpha interpolation.
		draw(33, 66, 192, alpha);
		Renderer::DisplayList::BindTexture(&texture);
		draw(66, 100, 192, alpha);
		auto cmd = Renderer::Native::DisplayList::FinalizeCommandBuffer(false);
		const auto framebuffer = GetFrameBuffer().colorImage;
		VulkanBuffer readback(12, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VulkanImage::TransitionImageLayout(framebuffer, GetSwapchainImageFormat(), VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		std::array<VkBufferImageCopy, 3> copies{};
		for (int i = 0; i < 3; ++i) {
			copies[i].bufferOffset = i * 4;
			copies[i].imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copies[i].imageOffset = { gWidth * (16 + 33 * i) / 100, gHeight / 2, 0 };
			copies[i].imageExtent = { 1, 1, 1 };
		}
		vkCmdCopyImageToBuffer(cmd, framebuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), copies.size(), copies.data());
		VulkanImage::TransitionImageLayout(framebuffer, GetSwapchainImageFormat(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		ASSERT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
		submit.pCommandBuffers = &cmd;
		ASSERT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
		ASSERT_EQ(vkQueueWaitIdle(GetGraphicsQueue()), VK_SUCCESS);
		void* mapped = nullptr;
		ASSERT_EQ(vkMapMemory(GetDevice(), readback.Memory(), 0, 12, 0, &mapped), VK_SUCCESS);
		std::array<uint8_t, 12> pixels{};
		memcpy(pixels.data(), mapped, pixels.size());
		vkUnmapMemory(GetDevice(), readback.Memory());
		for (int i = 0; i < 3; ++i) {
			for (int channel = 0; channel < 3; ++channel) {
				EXPECT_NEAR(pixels[i * 4 + channel], i == 0 ? 64 + alpha / 2 : 64 + alpha, 1);
			}
			EXPECT_NEAR(pixels[i * 4 + 3], alpha, 1);
		}
	}
	texture.GetRenderer()->DestroyImageResources();
	Renderer::Native::Cleanup();
}
