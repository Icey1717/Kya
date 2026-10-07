#include <gtest/gtest.h>
#include "renderer.h"
#include "renderer_debug.h"
#include "port.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Native/Blending.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanImage.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

TEST(GsBlending, UnsupportedEquationsHaveDiagnostics)
{
	using Renderer::Native::GetUnsupportedBlendReason;
	GIFReg::GSAlpha alpha{};
	alpha.A = 0; alpha.B = 1; alpha.C = 0; alpha.D = 1;
	EXPECT_EQ(GetUnsupportedBlendReason(alpha), nullptr);
	alpha.C = 2; alpha.FIX = 128;
	EXPECT_EQ(GetUnsupportedBlendReason(alpha), nullptr);
	alpha.FIX = 192;
	EXPECT_NE(GetUnsupportedBlendReason(alpha), nullptr);
	alpha.C = 1;
	EXPECT_NE(GetUnsupportedBlendReason(alpha), nullptr);
	alpha.B = 2; alpha.C = 0; alpha.D = 0;
	EXPECT_NE(GetUnsupportedBlendReason(alpha), nullptr);
	alpha.A = alpha.B; // Factor cancels, even for Ad or FIX > 128.
	alpha.C = 1;
	EXPECT_EQ(GetUnsupportedBlendReason(alpha), nullptr);
	alpha.A = 3;
	EXPECT_NE(GetUnsupportedBlendReason(alpha), nullptr);
}

// Run in its own process, like the other Vulkan fixtures.
TEST(GsBlendingRendering, DISABLED_AlphaFailMasksGlobalAlphaAndFixedBlend)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();
	auto& state = GetNativeRendererState();
	state.lightingDynamicBuffer.AddInstanceData(LightingDynamicBufferData{});
	state.animStBuffer.AddInstanceData(glm::vec4(0));
	GIFReg::GSPrim prim{};
	prim.PRIM = 3; // ABE deliberately clear: render flag 0x20 enables blending.
	SimpleMesh mesh("GS alpha-test regression", prim, 0);
	auto& vertices = mesh.GetVertexBufferData();
	vertices.Init(3, 3);
	const float positions[3][3] = { { -1, -1, 0.5f }, { 3, -1, 0.5f }, { -1, 3, 0.5f } };
	for (uint32_t i = 0; i < 3; ++i) {
		vertices.vertex.buff[i] = {};
		memcpy(vertices.vertex.buff[i].XYZFlags.fXYZ, positions[i], sizeof(positions[i]));
		for (auto& channel : vertices.vertex.buff[i].RGBA) channel = 128;
		vertices.vertex.buff[i].STQ.ST[0] = i == 1 ? 8192 : 0; // ST=2 at rightmost vertex.
		vertices.index.buff[i] = i;
	}
	vertices.vertex.tail = vertices.vertex.next = vertices.index.tail = 3;
	glm::mat4 identity(1);
	for (const auto testMode : { ATST_GEQUAL, ATST_NEVER }) {
	for (bool depthMask : { false, true }) {
	for (uint32_t vertexAlpha : { 128u, 64u }) {
		for (uint32_t globalAlpha : { 128u, 64u }) {
			for (uint32_t factor : { 0u, 2u }) {
			for (uint32_t fail = 0; fail < 4; ++fail) {
				SCOPED_TRACE(::testing::Message() << "vertex=" << vertexAlpha << " global=" << globalAlpha << " C=" << factor << " AFAIL=" << fail << " ATST=" << testMode << " ZMSK=" << depthMask);
				CombinedImageData image{};
				image.bitmaps.resize(1);
				image.registers.tex.TW = 1;
				image.registers.tex.TBW = 1;
				image.registers.tex.TBP0 = 0x100;
				image.registers.test.ATE = 1;
				image.registers.test.ATST = testMode;
				image.registers.test.AREF = 96 * vertexAlpha * globalAlpha / (128 * 128);
				image.registers.test.AFAIL = fail;
				image.registers.alpha.CMD = SCE_GS_SET_ALPHA_1(0, 1, 2, 1, 64);
				uint32_t pixels[2] = { 0x40808080, 0x80808080 };
				auto& bitmap = image.bitmaps[0];
				bitmap.canvasWidth = 2; bitmap.canvasHeight = 1; bitmap.maxMipLevel = 1;
				bitmap.bitBltBuf.DBP = 0x100; bitmap.bitBltBuf.DBW = 1;
				bitmap.trxReg.RRW = 2; bitmap.trxReg.RRH = 1;
				bitmap.pImage = pixels;
				SimpleTexture texture("GS alpha-test texture", { 0, 0, 1, 1 }, image.registers);
				texture.CreateRenderer(image);
				for (uint32_t i = 0; i < 3; ++i) vertices.vertex.buff[i].RGBA[3] = vertexAlpha;
				UpdateRenderPassKey(EClearMode::ColorDepth);
				Renderer::Native::PushGlobalMatrices(glm::value_ptr(identity), glm::value_ptr(identity), glm::value_ptr(identity));
				GIFReg::GSTest test{};
				test.ZTE = 1; test.ZTST = 1;
				Renderer::SetTest(test);
				Renderer::SetZbuf(depthMask ? 1 : 0);
				Renderer::SetAlpha(0, 1, factor, 1, 64);
				Renderer::SetGlobalAlpha(globalAlpha);
				const uint32_t color = (vertexAlpha << 24) | 0x00808080;
				const uint32_t colors[3] = { color, color, color };
				Renderer::Native::RenderMesh(&mesh, 0x20, colors);
				Renderer::Native::BindTexture(&texture);
				Renderer::SetGlobalAlpha(0); // Queued state must survive later changes.
				MainThreadEndCommands(state.renderThread);
				const bool slowPath = testMode != ATST_NEVER && fail != AFAIL_KEEP &&
					(fail == AFAIL_RGB_ONLY || !depthMask);
				if (slowPath) EXPECT_GT(state.accumulatedAlphaTestSlowPathTime, 0.0);
				else EXPECT_DOUBLE_EQ(state.accumulatedAlphaTestSlowPathTime, 0.0);
				const auto cmd = state.commandBuffers[GetCurrentFrame()];
				VulkanBuffer readback(16, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				for (bool depth : { false, true }) {
					const auto aspect = depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
					const auto handle = depth ? state.frameBuffer.depthImage : state.frameBuffer.colorImage;
					const auto format = depth ? VK_FORMAT_D32_SFLOAT_S8_UINT : VK_FORMAT_R8G8B8A8_UNORM;
					const VkImageAspectFlags transitionAspect = depth ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
					VulkanImage::TransitionImageLayout(handle, format, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, transitionAspect, cmd);
					for (uint32_t side = 0; side < 2; ++side) {
						VkBufferImageCopy copy{};
						copy.bufferOffset = (depth ? 8 : 0) + side * 4;
						copy.imageSubresource = { VkImageAspectFlags(aspect), 0, 0, 1 };
						copy.imageOffset = { side ? 3 * gWidth / 4 : gWidth / 4, gHeight / 2, 0 };
						copy.imageExtent = { 1, 1, 1 };
						vkCmdCopyImageToBuffer(cmd, handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), 1, &copy);
					}
					VulkanImage::TransitionImageLayout(handle, format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, transitionAspect, cmd);
				}
				Renderer::Debug::EndLabel(cmd);
				ASSERT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
				VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
				submit.commandBufferCount = 1; submit.pCommandBuffers = &cmd;
				ASSERT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
				ASSERT_EQ(vkQueueWaitIdle(GetGraphicsQueue()), VK_SUCCESS);
				void* mapped = nullptr;
				ASSERT_EQ(vkMapMemory(GetDevice(), readback.Memory(), 0, 16, 0, &mapped), VK_SUCCESS);
				const auto* bytes = static_cast<const uint8_t*>(mapped);
				const uint32_t alphaScale = vertexAlpha * globalAlpha;
				const uint32_t passAlpha = 128 * alphaScale / (128 * 128);
				const uint32_t failAlpha = 64 * alphaScale / (128 * 128);
				float depths[2]; memcpy(depths, bytes + 8, sizeof(depths));
				for (uint32_t side = 0; side < 2; ++side) {
					const bool passes = side == 1 && testMode != ATST_NEVER;
					const bool colorWrites = passes || fail == AFAIL_FB_ONLY || fail == AFAIL_RGB_ONLY;
					const bool alphaWrites = passes || fail == AFAIL_FB_ONLY;
					const auto alpha = side ? passAlpha : failAlpha;
					EXPECT_NEAR(bytes[side * 4], colorWrites ? (factor == 2 ? 64 : alpha) : 0, 1);
					EXPECT_NEAR(bytes[side * 4 + 3], alphaWrites ? alpha : 255, 1);
					EXPECT_FLOAT_EQ(depths[side], !depthMask && (passes || fail == AFAIL_ZB_ONLY) ? 0.5f : 0.0f);
				}
				vkUnmapMemory(GetDevice(), readback.Memory());
				ResetRenderThread(state.renderThread);
				state.accumulatedAlphaTestSlowPathTime = 0.0;
				state.nativeVertexBuffer.Reset();
				texture.GetRenderer()->DestroyImageResources();
			}
			}
		}
	}
	}
	}
	Renderer::Native::Cleanup();
}
