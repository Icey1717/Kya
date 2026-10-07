#include <gtest/gtest.h>

#include "FogDraw.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeFog.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include <GLFW/glfw3.h>
#include <array>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include "FogProjection.h"
#include "MathOps.h"
#include "../../../src/port/NativeProjection.h"

namespace
{
	std::array<edF32MATRIX4, 2> FogSceneProjections()
	{
		constexpr float nearClip = -0.001f, farClip = -250.0f, distance = 2.0f;
		constexpr float gsNear = float(0xffffef >> 3), gsFar = 1.0f;
		std::array<edF32MATRIX4, 2> matrices;
		matrices[0] = BuildNativeProjection(1.0f, 1.0f, distance, nearClip, farClip);
		ed3DComputeLocalToProjectionMatrix(0, 0, distance, 0, &matrices[1]);
		auto& gs = matrices[1];
		// Original ed3DViewportComputeViewMatrices depth-row setup.
		const float wNear = gs.dd + gs.cd * nearClip;
		const float wFar = gs.dd + gs.cd * farClip;
		gs.cc = (gsFar * wFar - gsNear * wNear) / (farClip - nearClip);
		gs.dc = (wNear * farClip * gsNear - wNear * nearClip * gsFar) / (farClip - nearClip);
		return matrices;
	}

	struct Pixel { std::array<uint8_t, 4> color{}; float depth = 0.0f; };
	Pixel ReadFogPixel(VkCommandBuffer cmd, int x = -1, int y = -1)
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

TEST(FogProjection, OriginalCameraWOffsetAndRoundTrip)
{
	const auto matrices = FogSceneProjections();
	const auto& native = matrices[0];
	const auto& gs = matrices[1];
	float p[4];
	Renderer::Native::BuildFogDepthProjection(p, native.raw, gs.raw);
	for (float cameraZ : { -0.01f, -1.0f, -20.0f, -150.0f, -249.0f }) {
		const float nativeDepth = (native.cc * cameraZ + native.dc) / (native.cd * cameraZ + native.dd);
		const float expectedGsDepth = (gs.cc * cameraZ + gs.dc) / (gs.cd * cameraZ + gs.dd);
		const float converted = (p[0] * nativeDepth + p[1]) / (p[2] * nativeDepth + p[3]);
		EXPECT_NEAR(converted, expectedGsDepth, 0.5f) << cameraZ;
		const float recovered = (p[1] - converted * p[3]) / (converted * p[2] - p[0]);
		EXPECT_NEAR(recovered, nativeDepth, 0.000001f);
	}
	// At camera Z=-20, the previous near/far lerp produced about 97 instead
	// of the original projection's roughly 175000, incorrectly fogging foreground.
	const float cameraZ = -20.0f;
	const float nativeDepth = (native.cc * cameraZ + native.dc) / (native.cd * cameraZ + native.dd);
	const float gsDepth = (p[0] * nativeDepth + p[1]) / (p[2] * nativeDepth + p[3]);
	EXPECT_GT(gsDepth, 0x7fff);
	EXPECT_LT(nativeDepth * float(0xffffef >> 3), 256);
}


// Opt-in Vulkan regression. Run alone with --gtest_also_run_disabled_tests --gtest_filter=FogRendering.*
TEST(FogRendering, DISABLED_DepthColorFlagsQueueViewportAndResize)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();
	auto& state = GetNativeRendererState();
	FogDraw fog;
	fog.flags = 1;
	fog.gsNear = 65536.0f;
	fog.gsFar = 0.0f;
	fog.color[0] = 128;

	auto beginScene = [&](uint32_t z) {
		RecordBeginCommandBuffer();
		RecordBeginRenderPass(RenderPassKey{ EClearMode::ColorDepth });
		VkClearAttachment attachment{};
		attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachment.clearValue.depthStencil = { float(z) / fog.gsNear, 0 };
		VkClearRect rect{ { { 0, 0 }, GetFrameBufferSize() }, 0, 1 };
		vkCmdClearAttachments(state.commandBuffers[GetCurrentFrame()], 1, &attachment, 1, &rect);
		RecordEndRenderPass();
	};
	auto render = [&](uint32_t z, int x = -1) {
		beginScene(z);
		Fog::Record(state.commandBuffers[GetCurrentFrame()], fog);
		RecordEndCommandBuffer();
		return ReadFogPixel(state.commandBuffers[GetCurrentFrame()], x);
	};
	const auto far = render(0);
	const auto middle = render(0x4000);
	const auto near = render(0x8000);
	EXPECT_EQ(far.color[0], 128);
	EXPECT_EQ(middle.color[0], 64);
	EXPECT_EQ(near.color[0], 0);
	EXPECT_EQ(far.color[3], 128);
	EXPECT_EQ(middle.color[3], 128);
	EXPECT_EQ(render(0x7fff).color[0], 1); // Last fogged GS depth.
	EXPECT_EQ(middle.depth, 0.25f);

	// Scene transitions supply already-interpolated GS RGB, with independent alpha.
	fog.color[0] = 0;
	fog.color[1] = 96;
	fog.color[3] = 32;
	const auto green = render(0x4000);
	EXPECT_EQ(green.color[0], 0);
	EXPECT_EQ(green.color[1], 48);
	EXPECT_EQ(green.color[3], 32);
	fog.color[0] = 128;
	fog.color[1] = 0;
	fog.color[3] = 128;

	fog.depthOffset = 32;
	const auto shifted = render(0x4000);
	EXPECT_EQ(shifted.color[0], 32);
	EXPECT_EQ(shifted.depth, 0.375f);
	fog.flags |= 32;
	const auto repeated = render(0x4000);
	EXPECT_EQ(repeated.color, shifted.color);
	EXPECT_EQ(repeated.depth, 0.5f);
	fog.flags = 1;
	fog.depthOffset = -32;
	const auto negative = render(0x4000);
	EXPECT_EQ(negative.color[0], 96);
	EXPECT_EQ(negative.depth, 0.125f);
	fog.depthOffset = 255;
	EXPECT_EQ(render(0x4000).depth, 255.0f / 256.0f);
	fog.depthOffset = -255;
	EXPECT_EQ(render(0x4000).depth, 0.0f);
	// User capture: z=96 and offset=80 must retain 80/128 of scene RGB.
	// The previous reversed sign saturated the green byte to zero, hiding the scene.
	fog.depthOffset = 80;
	const auto captured = render(96);
	EXPECT_EQ(captured.color[0], 48); // Black scene + fog red 128 * (1 - 80/128).
	EXPECT_EQ(captured.depth, float(0x5060) / 65536.0f);
	fog.depthOffset = 127;
	EXPECT_EQ(render(96).color[0], 1);
	fog.depthOffset = 0;
	fog.flags = 1 | 4;
	const auto skipped = render(0x4000);
	EXPECT_EQ(skipped.color[0], 0);
	EXPECT_EQ(skipped.color[3], 64); // Depth-to-alpha runs even when color is disabled.
	fog.flags = 1;

	// An inset viewport affects only its scissor at both PS2 and resized resolutions.
	fog.viewport[2] = 0.25f;
	EXPECT_EQ(render(0).color[0], 0);
	EXPECT_EQ(render(0, gWidth / 8).color[0], 128);
	fog.viewport[2] = 1.0f;

	// Copied commands retain their own color and offset, and a pending clear executes first.
	UpdateRenderPassKey(EClearMode::ColorDepth);
	fog.depthOffset = 64;
	SubmitFog(fog);
	fog.depthOffset = 0;
	fog.color[0] = 0;
	fog.color[1] = 96;
	SubmitFog(fog);
	fog.color[1] = 0;
	fog.flags = 0;
	SubmitFog(fog); // Disabled fog must not enqueue a pass.
	MainThreadEndCommands(state.renderThread);
	const auto queued = ReadFogPixel(state.commandBuffers[GetCurrentFrame()]);
	EXPECT_EQ(queued.color[0], 32);
	EXPECT_EQ(queued.color[1], 48);
	ResetRenderThread(state.renderThread);
	fog.flags = 1;
	fog.color[0] = 128;
	EXPECT_EQ(render(0x4000).color, middle.color);

	// Transparent surfaces with ZMSK retain background depth for the post effect.
	// When they write depth, that nearer depth suppresses fog on the composed color.
	state.lightingDynamicBuffer.AddInstanceData(LightingDynamicBufferData{});
	state.animStBuffer.AddInstanceData(glm::vec4(0));
	GIFReg::GSPrim opaquePrim{};
	opaquePrim.PRIM = 3;
	GIFReg::GSPrim transparentPrim = opaquePrim;
	transparentPrim.ABE = 1;
	SimpleMesh background("Fog background", opaquePrim, 0);
	SimpleMesh transparent("Fog transparent foreground", transparentPrim, 0);
	for (auto* mesh : { &background, &transparent }) {
		auto& vertices = mesh->GetVertexBufferData();
		vertices.Init(3, 3);
		const float z = mesh == &background ? 0.25f : 0.75f;
		const float positions[3][3] = { { -1, -1, z }, { 3, -1, z }, { -1, 3, z } };
		for (uint32_t i = 0; i < 3; ++i) {
			vertices.vertex.buff[i] = {};
			memcpy(vertices.vertex.buff[i].XYZFlags.fXYZ, positions[i], sizeof(positions[i]));
			// The native white fallback is byte 255 and GS modulation divides by 128.
			vertices.vertex.buff[i].RGBA[mesh == &background ? 2 : 1] = 64;
			vertices.vertex.buff[i].RGBA[3] = mesh == &background ? 64 : 32;
			vertices.index.buff[i] = i;
		}
		vertices.vertex.tail = vertices.vertex.next = vertices.index.tail = 3;
	}
	auto transparentScene = [&](bool writeDepth) {
		UpdateRenderPassKey(EClearMode::ColorDepth);
		glm::mat4 identity(1);
		Renderer::Native::PushGlobalMatrices(glm::value_ptr(identity), glm::value_ptr(identity), glm::value_ptr(identity));
		Renderer::SetGlobalAlpha(128);
		Renderer::SetAlpha(0, 1, 0, 1, 0);
		GIFReg::GSTest test{};
		test.ZTE = 1;
		test.ZTST = 2;
		Renderer::SetTest(test);
		Renderer::SetZbuf(0);
		Renderer::Native::RenderMesh(&background, 0x20);
		Renderer::Native::BindUntextured();
		Renderer::SetZbuf(writeDepth ? 0 : 1);
		Renderer::Native::RenderMesh(&transparent, 0x20);
		Renderer::Native::BindUntextured();
		SubmitFog(fog);
		MainThreadEndCommands(state.renderThread);
		const auto pixel = ReadFogPixel(state.commandBuffers[GetCurrentFrame()]);
		ResetRenderThread(state.renderThread);
		return pixel;
	};
	const auto behindTransparency = transparentScene(false);
	EXPECT_EQ(behindTransparency.color[0], 64);
	EXPECT_NEAR(behindTransparency.color[1], 32, 1);
	EXPECT_NEAR(behindTransparency.color[2], 32, 1);
	EXPECT_EQ(behindTransparency.depth, 0.25f);
	const auto depthWritingTransparency = transparentScene(true);
	EXPECT_EQ(depthWritingTransparency.color[0], 0);
	EXPECT_NEAR(depthWritingTransparency.color[1], 64, 1);
	EXPECT_NEAR(depthWritingTransparency.color[2], 64, 1);
	EXPECT_EQ(depthWritingTransparency.depth, 0.75f);
	Renderer::SetZbuf(0);

	// Use the game's original GS camera projection, including W's constant term.
	// The foreground is clear; sufficiently distant geometry receives fog.
	const auto projections = FogSceneProjections();
	BuildFogDepthProjection(fog.depthProjection, projections[0].raw, projections[1].raw);
	auto projectedZ = [&](float cameraZ) {
		const auto& native = projections[0];
		return uint32_t(((native.cc * cameraZ + native.dc) / (native.cd * cameraZ + native.dd)) * fog.gsNear);
	};
	// Existing render() encodes integer z/gsNear into the native depth attachment.
	// Use a large scale here to retain the small native depths in this regression.
	fog.gsNear = 16777216.0f;
	fog.depthOffset = 80;
	const auto projectedNear = render(projectedZ(-20));
	EXPECT_EQ(projectedNear.color[0], 0);
	const auto projectedFar = render(projectedZ(-150));
	EXPECT_GT(projectedFar.color[0], 0);
	EXPECT_LT(projectedFar.color[0], 48);
	std::fill(std::begin(fog.depthProjection), std::end(fog.depthProjection), 0.0f);
	fog.gsNear = 65536.0f;
	fog.depthOffset = 0;

	ResizeFrameBuffer(768, 448);
	ApplyPendingResizeInternal();
	EXPECT_EQ(render(0x4000).color, middle.color);
	fog.viewport[2] = 0.25f;
	EXPECT_EQ(render(0).color[0], 0);
	EXPECT_EQ(render(0, gWidth / 8).color[0], 128);
	Renderer::Native::Cleanup();
}
