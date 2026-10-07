#include <gtest/gtest.h>
#include "AntiAliasingDraw.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeAntiAliasing.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeFog.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeMSAA.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include <GLFW/glfw3.h>
#include <array>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

namespace
{
	struct Pixel { std::array<uint8_t, 4> color{}; float depth = 0.0f; };
	Pixel ReadAAPixel(VkCommandBuffer cmd, int x = -1, int y = -1)
	{
		using namespace Renderer;
		using namespace Renderer::Native;
		const auto image = GetNativeRendererState().frameBuffer.colorImage;
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
		VkBufferImageCopy copy{};
		copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy.imageOffset = { x < 0 ? gWidth / 2 : x, y < 0 ? gHeight / 2 : y, 0 };
		copy.imageExtent = { 1, 1, 1 };
		vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), 1, &copy);
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
		barrier.image = GetNativeRendererState().frameBuffer.depthImage;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
		copy.bufferOffset = 4;
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		vkCmdCopyImageToBuffer(cmd, barrier.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), 1, &copy);
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
		Debug::EndLabel(cmd);
		EXPECT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
		VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;
		EXPECT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
		EXPECT_EQ(vkQueueWaitIdle(GetGraphicsQueue()), VK_SUCCESS);
		void* mapped = nullptr;
		EXPECT_EQ(vkMapMemory(GetDevice(), readback.Memory(), 0, 8, 0, &mapped), VK_SUCCESS);
		Pixel pixel;
		if (mapped) {
			memcpy(pixel.color.data(), mapped, 4);
			memcpy(&pixel.depth, static_cast<uint8_t*>(mapped) + 4, 4);
		}
		vkUnmapMemory(GetDevice(), readback.Memory());
		if (GetSwapchainImageFormat() == VK_FORMAT_B8G8R8A8_UNORM || GetSwapchainImageFormat() == VK_FORMAT_B8G8R8A8_SRGB)
			std::swap(pixel.color[0], pixel.color[2]);
		return pixel;
	}
}

// Run alone: the renderer's other GPU fixtures also own a device and cannot
// currently reinitialize all renderer globals safely within the same process.
TEST(AntiAliasingRendering, DISABLED_ModesFogMaskQueueViewportAndResize)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();
	auto& state = GetNativeRendererState();
	AntiAliasingDraw aa;
	EXPECT_EQ(GetAntiAliasingMode(), AntiAliasingMode::PS2Approximation);
	const auto cmd = state.commandBuffers[GetCurrentFrame()];
	auto beginPattern = [&](uint32_t z, bool dot = true) {
		RecordBeginCommandBuffer();
		RecordBeginRenderPass(RenderPassKey{ EClearMode::ColorDepth });
		VkClearAttachment attachment{};
		attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachment.clearValue.depthStencil = { float(z) / 65536.0f, 0 };
		VkClearRect rect{ { { 0, 0 }, GetFrameBufferSize() }, 0, 1 };
		vkCmdClearAttachments(cmd, 1, &attachment, 1, &rect);
		attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 0.25f } };
		vkCmdClearAttachments(cmd, 1, &attachment, 1, &rect);
		attachment.clearValue.color = { { 1.0f, 1.0f, 1.0f, 0.25f } };
		if (dot) rect = { { { gWidth / 2, gHeight / 2 }, { 1, 1 } }, 0, 1 };
		vkCmdClearAttachments(cmd, 1, &attachment, 1, &rect);
		RecordEndRenderPass();
	};
	auto finish = [&] {
		RecordEndCommandBuffer();
		return ReadAAPixel(cmd);
	};
	auto render = [&](uint32_t z, bool dot = true) {
		beginPattern(z, dot);
		AntiAliasing::Record(cmd, aa);
		return finish();
	};
	aa.mode = AntiAliasingMode::Off;
	const auto off = render(0);
	EXPECT_EQ(off.color[0], 255);
	aa.mode = AntiAliasingMode::PS2Approximation;
	const auto ps2 = render(0);
	EXPECT_GT(ps2.color[0], 0);
	EXPECT_LT(ps2.color[0], 255);
	EXPECT_EQ(ps2.color[3], off.color[3]);
	EXPECT_EQ(ps2.depth, off.depth);
	aa.fullResolutionPS2Capture = true;
	const auto fullResolution = render(0);
	EXPECT_GT(fullResolution.color[0], ps2.color[0]);
	EXPECT_LT(fullResolution.color[0], 255);
	EXPECT_EQ(fullResolution.color[3], off.color[3]);
	EXPECT_EQ(fullResolution.depth, off.depth);
	aa.fullResolutionPS2Capture = false;
	EXPECT_EQ(render(0xe000).color[0], 255); // Foreground survives the original -82 offset.
	const auto partial = render(0x9200); // Adjusted green byte 64 => half original, half blur.
	EXPECT_GT(partial.color[0], ps2.color[0]);
	EXPECT_LT(partial.color[0], 255);
	EXPECT_EQ(render(0, false).color[0], 255); // Flat regions stay flat.

	// Fog's positive depth-byte shift is undone only for the AA mask.
	beginPattern(0);
	FogDraw fog;
	fog.flags = 1 | 4;
	fog.depthOffset = 80;
	fog.gsNear = 65536.0f;
	fog.gsFar = 0.0f;
	Fog::Record(cmd, fog);
	aa.fogDepthOffset = 80;
	AntiAliasing::Record(cmd, aa);
	const auto withFog = finish();
	EXPECT_EQ(withFog.color[0], ps2.color[0]);
	EXPECT_EQ(withFog.depth, 80.0f / 256.0f);
	aa.fogDepthOffset = 0;

	aa.mode = AntiAliasingMode::FXAA;
	const auto fxaa = render(0);
	EXPECT_GT(fxaa.color[0], 0);
	EXPECT_LT(fxaa.color[0], 255);
	EXPECT_EQ(fxaa.color[3], off.color[3]);
	EXPECT_EQ(fxaa.depth, 0.0f);
	EXPECT_EQ(render(0, false).color[0], 255);
	EXPECT_EQ(render(0xe000).color[0], fxaa.color[0]); // Modern AA ignores depth.
	// A long finite edge needs more than the original twelve search steps.
	// Increasing the budget must reach its far endpoint and change the subpixel shift.
	auto renderLongEdge = [&](uint32_t multiplier) {
		beginPattern(0);
		RecordBeginRenderPass(RenderPassKey{ EClearMode::None });
		VkClearAttachment attachment{};
		attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachment.clearValue.color = { { 1.0f, 1.0f, 1.0f, 0.25f } };
		VkClearRect rect{ { { gWidth / 2 - 4, gHeight / 2 }, { 40, 80 } }, 0, 1 };
		vkCmdClearAttachments(cmd, 1, &attachment, 1, &rect);
		RecordEndRenderPass();
		aa.fxaaQualityMultiplier = multiplier;
		AntiAliasing::Record(cmd, aa);
		return finish();
	};
	const auto edge1x = renderLongEdge(1);
	const auto edge2x = renderLongEdge(2);
	const auto edge4x = renderLongEdge(4);
	EXPECT_LT(edge2x.color[0], edge1x.color[0]);
	EXPECT_LT(edge4x.color[0], edge2x.color[0]);
	EXPECT_EQ(edge4x.color[3], off.color[3]);
	EXPECT_EQ(edge4x.depth, 0.0f);
	EXPECT_EQ(render(0, false).color[0], 255);
	aa.fxaaQualityMultiplier = 1;
	// Two scene AA boundaries can reuse the slot's snapshots in one command buffer.
	beginPattern(0);
	aa.mode = AntiAliasingMode::PS2Approximation;
	AntiAliasing::Record(cmd, aa);
	aa.mode = AntiAliasingMode::FXAA;
	AntiAliasing::Record(cmd, aa);
	const auto repeated = finish();
	EXPECT_LT(repeated.color[0], 255);
	EXPECT_EQ(repeated.color[3], off.color[3]);
	EXPECT_EQ(repeated.depth, 0.0f);
	aa.viewport[2] = 0.25f;
	EXPECT_EQ(render(0).color[0], 255); // Center outside inset viewport.
	aa.viewport[2] = 1.0f;

	// Queue geometry and AA together. Later settings/parameter changes must not
	// turn the submitted PS2 filter into Off or change its captured viewport.
	state.lightingDynamicBuffer.AddInstanceData(LightingDynamicBufferData{});
	state.animStBuffer.AddInstanceData(glm::vec4(0));
	GIFReg::GSPrim prim{};
	prim.PRIM = 3;
	SimpleMesh point("AA queued white square", prim, 0);
	auto& vertices = point.GetVertexBufferData();
	vertices.Init(6, 6);
	const float dx = 2.0f / float(gWidth), dy = 2.0f / float(gHeight);
	const float positions[6][3] = { {0,0,0.25f}, {dx,0,0.25f}, {0,dy,0.25f},
		{0,dy,0.25f}, {dx,0,0.25f}, {dx,dy,0.25f} };
	for (uint32_t i = 0; i < 6; ++i) {
		vertices.vertex.buff[i] = {};
		memcpy(vertices.vertex.buff[i].XYZFlags.fXYZ, positions[i], sizeof(positions[i]));
		std::fill_n(vertices.vertex.buff[i].RGBA, 4, 128);
		vertices.index.buff[i] = i;
	}
	vertices.vertex.tail = vertices.vertex.next = vertices.index.tail = 6;
	auto queuedScene = [&](AntiAliasingMode mode, bool fullResolutionCapture = false) {
		UpdateRenderPassKey(EClearMode::ColorDepth);
		glm::mat4 identity(1);
		Renderer::Native::PushGlobalMatrices(glm::value_ptr(identity), glm::value_ptr(identity), glm::value_ptr(identity));
		Renderer::SetGlobalAlpha(128);
		GIFReg::GSTest test{};
		test.ZTE = 1;
		test.ZTST = 2;
		Renderer::SetTest(test);
		Renderer::SetZbuf(0);
		Renderer::Native::RenderMesh(&point, 0x20);
		Renderer::Native::BindUntextured();
		aa.mode = mode;
		SetFullResolutionPS2AACapture(fullResolutionCapture);
		SubmitAntiAliasing(aa);
		SetFullResolutionPS2AACapture(false);
		aa.mode = AntiAliasingMode::Off;
		aa.viewport[2] = 0;
		SetAntiAliasingMode(AntiAliasingMode::Off);
		MainThreadEndCommands(state.renderThread);
		const auto pixel = ReadAAPixel(cmd);
		ResetRenderThread(state.renderThread);
		aa.viewport[2] = 1;
		return pixel;
	};
	EXPECT_EQ(queuedScene(AntiAliasingMode::Off).color[0], 255);
	const auto queuedPS2 = queuedScene(AntiAliasingMode::PS2Approximation);
	EXPECT_LT(queuedPS2.color[0], 255);
	const auto queuedFullResolution = queuedScene(AntiAliasingMode::PS2Approximation, true);
	EXPECT_GT(queuedFullResolution.color[0], queuedPS2.color[0]);
	EXPECT_LT(queuedFullResolution.color[0], 255);
	EXPECT_EQ(queuedFullResolution.color[3], queuedPS2.color[3]);
	EXPECT_EQ(queuedFullResolution.depth, queuedPS2.depth);
	SetAntiAliasingMode(AntiAliasingMode::PS2Approximation);

	ResizeFrameBuffer(768, 448);
	ApplyPendingResizeInternal();
	aa.mode = AntiAliasingMode::FXAA;
	EXPECT_LT(render(0).color[0], 255);
	aa.mode = AntiAliasingMode::PS2Approximation;
	const auto resizedPS2 = render(0);
	EXPECT_LT(resizedPS2.color[0], 255);
	aa.fullResolutionPS2Capture = true;
	const auto resizedFullResolution = render(0);
	EXPECT_GT(resizedFullResolution.color[0], resizedPS2.color[0]);
	EXPECT_LT(resizedFullResolution.color[0], 255);
	EXPECT_EQ(resizedFullResolution.color[3], resizedPS2.color[3]);
	EXPECT_EQ(resizedFullResolution.depth, resizedPS2.depth);
	Renderer::Native::Cleanup();
}

// Run separately from the other GPU fixtures, which own renderer globals.
TEST(AntiAliasingRendering, DISABLED_MSAAEdgeCoverageEffectsHudAndResize)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();
	ASSERT_NE(MSAA::GetSamples(), VK_SAMPLE_COUNT_1_BIT);
	auto& state = GetNativeRendererState();
	state.lightingDynamicBuffer.AddInstanceData(LightingDynamicBufferData{});
	state.animStBuffer.AddInstanceData(glm::vec4(0));
	GIFReg::GSPrim prim{};
	prim.PRIM = 3;
	SimpleMesh triangle("MSAA diagonal edge", prim, 0);
	auto& vertices = triangle.GetVertexBufferData();
	vertices.Init(3, 3);
	const auto cmd = state.commandBuffers[GetCurrentFrame()];
	auto queueTriangle = [&] {
		glm::mat4 identity(1);
		Renderer::Native::PushGlobalMatrices(glm::value_ptr(identity), glm::value_ptr(identity), glm::value_ptr(identity));
		Renderer::SetGlobalAlpha(128);
		GIFReg::GSTest test{};
		test.ZTE = 1;
		test.ZTST = 2;
		Renderer::SetTest(test);
		Renderer::SetZbuf(0);
		Renderer::Native::RenderMesh(&triangle, 0x20);
		Renderer::Native::BindUntextured();
	};
	auto render = [&](AntiAliasingMode mode, bool hud = false, int effect = 0) {
		state.msaaSceneEnded = false;
		SetAntiAliasingMode(mode);
		UpdateRenderPassKey(EClearMode::ColorDepth);
		const float dx = 2.0f / float(gWidth), dy = 2.0f / float(gHeight);
		const float positions[3][3] = { { 0, 0, 0.25f }, { dx, 0, 0.25f }, { 0, dy, 0.25f } };
		for (uint32_t i = 0; i < 3; ++i) {
			vertices.vertex.buff[i] = {};
			memcpy(vertices.vertex.buff[i].XYZFlags.fXYZ, positions[i], sizeof(positions[i]));
			std::fill_n(vertices.vertex.buff[i].RGBA, 4, 128);
			vertices.index.buff[i] = i;
		}
		vertices.vertex.tail = vertices.vertex.next = vertices.index.tail = 3;
		queueTriangle();
		AntiAliasingDraw aa;
		aa.mode = mode;
		SubmitAntiAliasing(aa);
		if (hud) {
			UpdateRenderPassKey(EClearMode::ColorDepth);
			queueTriangle();
		}
		// Submitted geometry must retain its sample count despite later setting changes.
		SetAntiAliasingMode(AntiAliasingMode::Off);
		MainThreadEndCommands(state.renderThread);
		if (effect != 0) {
			FogDraw fog;
			fog.flags = 1 | 4;
			fog.depthOffset = 32;
			fog.gsNear = 65536.0f;
			fog.gsFar = 0;
			Fog::Record(cmd, fog);
			if (effect == 2) {
				RecordBeginRenderPass(RenderPassKey{ EClearMode::None, ERenderPassKind::Main, true });
				RecordEndRenderPass();
			}
		}
		const auto pixel = ReadAAPixel(cmd);
		ResetRenderThread(state.renderThread);
		return pixel;
	};
	const auto off = render(AntiAliasingMode::Off);
	EXPECT_TRUE(off.color[0] == 0 || off.color[0] == 255);
	const auto msaa = render(AntiAliasingMode::MSAA);
	EXPECT_GT(msaa.color[0], 0);
	EXPECT_LT(msaa.color[0], 255);
	EXPECT_TRUE(msaa.depth == 0.0f || msaa.depth == 0.25f);

	// Resume multisampled geometry after a single-sample effect. Preserve both
	// the resolved edge color and the effect's modified depth.
	const auto fogOnly = render(AntiAliasingMode::MSAA, false, 1);
	const auto resumed = render(AntiAliasingMode::MSAA, false, 2);
	EXPECT_EQ(resumed.color, fogOnly.color);
	EXPECT_EQ(resumed.depth, fogOnly.depth);
	EXPECT_NEAR(resumed.depth, msaa.depth + 32.0f / 256.0f, 0.0001f);
	EXPECT_EQ(render(AntiAliasingMode::MSAA, true).color, off.color);
	EXPECT_EQ(render(AntiAliasingMode::Off).color, off.color);
	ResizeFrameBuffer(768, 448);
	ApplyPendingResizeInternal();
	const auto resized = render(AntiAliasingMode::MSAA);
	EXPECT_GT(resized.color[0], 0);
	EXPECT_LT(resized.color[0], 255);
	Renderer::Native::Cleanup();
}
