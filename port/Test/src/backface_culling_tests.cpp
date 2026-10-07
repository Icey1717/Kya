#include <gtest/gtest.h>
#include "renderer.h"
#include "renderer_debug.h"
#include "port.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanImage.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

namespace
{
	void AddQuad(Renderer::NativeVertexBufferData& buffer, bool skipFirstTriangle)
	{
		GIFReg::GSPrim prim{};
		prim.PRIM = Renderer::GS_TRIANGLESTRIP;
		const float positions[4][3] = { { -1, -1, 0.5f }, { 1, -1, 0.5f }, { -1, 1, 0.5f }, { 1, 1, 0.5f } };
		for (int i = 0; i < 4; ++i) {
			Renderer::GSVertexUnprocessedNormal vertex{};
			memcpy(vertex.XYZFlags.fXYZ, positions[i], sizeof(positions[i]));
			for (auto& channel : vertex.RGBA) channel = 128;
			Renderer::KickVertex(vertex, prim, (i < 2 || (i == 2 && skipFirstTriangle)) ? 0x8000 : 0, buffer);
		}
	}
}

TEST(BackfaceCulling, StripWindingSurvivesAdcSkips)
{
	for (bool skip : { false, true }) {
		Renderer::NativeVertexBufferData buffer;
		buffer.Init(8, 12);
		AddQuad(buffer, skip);
		ASSERT_EQ(buffer.GetIndexTail(), skip ? 3u : 6u);
		for (size_t i = 0; i < buffer.GetIndexTail(); i += 3) {
			const auto* a = buffer.vertex.buff[buffer.index.buff[i]].XYZFlags.fXYZ;
			const auto* b = buffer.vertex.buff[buffer.index.buff[i + 1]].XYZFlags.fXYZ;
			const auto* c = buffer.vertex.buff[buffer.index.buff[i + 2]].XYZFlags.fXYZ;
			EXPECT_GT((b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]), 0);
		}
	}
}

// Vulkan fixtures run individually in their own process.
TEST(BackfaceCullingRendering, DISABLED_BothSidesReversedModeAndQueuedState)
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
	prim.PRIM = GS_TRIANGLESTRIP;
	SimpleMesh mesh("Backface culling quad", prim, 0);
	mesh.GetVertexBufferData().Init(8, 12);
	AddQuad(mesh.GetVertexBufferData(), false);
	CombinedImageData image{};
	image.bitmaps.resize(1);
	image.registers.tex.TBW = 1;
	image.registers.tex.TBP0 = 0x100;
	uint32_t pixel = 0x80808080;
	auto& bitmap = image.bitmaps[0];
	bitmap.canvasWidth = bitmap.canvasHeight = bitmap.maxMipLevel = 1;
	bitmap.bitBltBuf.DBP = 0x100; bitmap.bitBltBuf.DBW = 1;
	bitmap.trxReg.RRW = bitmap.trxReg.RRH = 1;
	bitmap.pImage = &pixel;
	SimpleTexture texture("Backface culling texture", { 0, 0, 1, 1 }, image.registers);
	texture.CreateRenderer(image);
	glm::mat4 identity(1);
	for (bool mirrored : { false, true }) {
	for (bool enabled : { false, true }) {
	for (bool reversed : { false, true }) {
		SCOPED_TRACE(testing::Message() << "mirror=" << mirrored << " enabled=" << enabled << " reversed=" << reversed);
		glm::mat4 model(1);
		if (mirrored) model[0][0] = -1;
		UpdateRenderPassKey(EClearMode::ColorDepth);
		Renderer::Native::PushGlobalMatrices(glm::value_ptr(model), glm::value_ptr(identity), glm::value_ptr(identity));
		GIFReg::GSTest test{};
		test.ZTE = 1; test.ZTST = 1;
		Renderer::SetTest(test);
		Renderer::SetZbuf(0);
		Renderer::SetBackfaceCulling(enabled, reversed);
		Renderer::Native::RenderMesh(&mesh, 0);
		Renderer::Native::BindTexture(&texture);
		Renderer::SetBackfaceCulling(false, false); // Must not change the queued draw.
		MainThreadEndCommands(state.renderThread);
		const auto cmd = state.commandBuffers[GetCurrentFrame()];
		VulkanBuffer readback(8, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VulkanImage::TransitionImageLayout(state.frameBuffer.colorImage, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		for (int side = 0; side < 2; ++side) {
			VkBufferImageCopy copy{};
			copy.bufferOffset = side * 4;
			copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copy.imageOffset = { side ? 3 * gWidth / 4 : gWidth / 4, gHeight / 2, 0 };
			copy.imageExtent = { 1, 1, 1 };
			vkCmdCopyImageToBuffer(cmd, state.frameBuffer.colorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), 1, &copy);
		}
		VulkanImage::TransitionImageLayout(state.frameBuffer.colorImage, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		Renderer::Debug::EndLabel(cmd);
		ASSERT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
		VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1; submit.pCommandBuffers = &cmd;
		ASSERT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
		ASSERT_EQ(vkQueueWaitIdle(GetGraphicsQueue()), VK_SUCCESS);
		void* mapped = nullptr;
		ASSERT_EQ(vkMapMemory(GetDevice(), readback.Memory(), 0, 8, 0, &mapped), VK_SUCCESS);
		const auto* bytes = static_cast<const uint8_t*>(mapped);
		const bool visible = !enabled || mirrored == reversed;
		for (int side = 0; side < 2; ++side) EXPECT_NEAR(bytes[side * 4], visible ? 128 : 0, 1);
		vkUnmapMemory(GetDevice(), readback.Memory());
		ResetRenderThread(state.renderThread);
		state.nativeVertexBuffer.Reset();
	}
	}
	}
	texture.GetRenderer()->DestroyImageResources();
	Renderer::Native::Cleanup();
}
