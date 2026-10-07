#include <gtest/gtest.h>
#include "renderer.h"
#include "renderer_debug.h"
#include "port.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanCommands.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanImage.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <array>

namespace
{
	std::array<uint8_t, 4> ReadEnvironmentPixel(VkCommandBuffer cmd)
	{
		using namespace Renderer::Native;
		const auto image = GetNativeRendererState().frameBuffer.colorImage;
		VulkanBuffer readback(4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VulkanImage::TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		VkBufferImageCopy copy{};
		copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copy.imageOffset = { gWidth / 2, gHeight / 2, 0 };
		copy.imageExtent = { 1, 1, 1 };
		vkCmdCopyImageToBuffer(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.Get(), 1, &copy);
		VulkanImage::TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, cmd);
		Renderer::Debug::EndLabel(cmd);
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
		if (GetSwapchainImageFormat() == VK_FORMAT_B8G8R8A8_UNORM || GetSwapchainImageFormat() == VK_FORMAT_B8G8R8A8_SRGB) std::swap(pixel[0], pixel[2]);
		vkUnmapMemory(GetDevice(), readback.Memory());
		return pixel;
	}
}

// Run alone with --gtest_also_run_disabled_tests --gtest_filter=EnvironmentMappingRendering.*.
TEST(EnvironmentMappingRendering, DISABLED_StaticRigidCameraScrollingLayersAndQueuedState)
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
	state.animStBuffer.AddInstanceData(glm::vec4(0.0f));
	CombinedImageData image{};
	image.bitmaps.resize(1);
	image.registers.tex.TW = image.registers.tex.TH = 2;
	image.registers.tex.TBW = 1;
	image.registers.tex.TBP0 = 0x100;
	std::array<uint32_t, 16> pixels;
	const std::array<uint32_t, 4> colors = { 0x800000c8, 0x8000c800, 0x80c80000, 0x80c8c8c8 };
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) pixels[y * 4 + x] = colors[(y / 2) * 2 + x / 2];
	}
	auto& bitmap = image.bitmaps[0];
	bitmap.canvasWidth = bitmap.canvasHeight = 4;
	bitmap.maxMipLevel = 1;
	bitmap.bitBltBuf.DBP = 0x100;
	bitmap.bitBltBuf.DBW = 1;
	bitmap.trxReg.RRW = bitmap.trxReg.RRH = 4;
	bitmap.pImage = pixels.data();
	SimpleTexture texture("Environment regression", { 0, 0, 1, 1 }, image.registers);
	texture.CreateRenderer(image);
	GIFReg::GSPrim prim{};
	prim.PRIM = 3;
	SimpleMesh mesh("Environment static", prim, 0x8000000);
	SimpleMesh rigid("Environment rigid", prim, 0x8010000);
	SimpleMesh noNormals("Environment absent normals", prim, 0);
	const float positions[3][3] = { { -1, -1, 0.5f }, { 3, -1, 0.5f }, { -1, 3, 0.5f } };
	for (auto* current : { &mesh, &rigid, &noNormals }) {
		auto& vertices = current->GetVertexBufferData();
		vertices.Init(3, 3);
		for (uint32_t i = 0; i < 3; ++i) {
			vertices.vertex.buff[i] = {};
			memcpy(vertices.vertex.buff[i].XYZFlags.fXYZ, positions[i], sizeof(positions[i]));
			vertices.vertex.buff[i].XYZFlags.flags = 0x3dc;
			for (auto& channel : vertices.vertex.buff[i].RGBA) channel = 128;
			vertices.vertex.buff[i].STQ.ST[0] = vertices.vertex.buff[i].STQ.ST[1] = 512;
			vertices.vertex.buff[i].normal.fNormal[0] = vertices.vertex.buff[i].normal.fNormal[1] = 0.5f;
			vertices.index.buff[i] = i;
		}
		vertices.vertex.tail = vertices.vertex.next = vertices.index.tail = 3;
	}
	glm::mat4 identity(1.0f);
	glm::mat4 rotation(1.0f);
	rotation[0] = glm::vec4(0, 1, 0, 0);
	rotation[1] = glm::vec4(-1, 0, 0, 0);
	glm::vec4 cameraX(1, 0, 0, 0), cameraY(0, 1, 0, 0);
	auto render = [&](SimpleMesh& current, uint32_t flags, const glm::mat4& transform,
		const glm::vec4& axisY, glm::vec4 scroll, bool layers = false,
		bool fixedBlend = false, bool additive = false, uint8_t fixed = 64) {
		UpdateRenderPassKey(EClearMode::ColorDepth);
		Renderer::Native::PushGlobalMatrices(glm::value_ptr(identity), glm::value_ptr(identity), glm::value_ptr(identity));
		Renderer::SetTest(GIFReg::GSTest{});
		Renderer::SetGlobalAlpha(128);
		Renderer::SetAlpha(0, 1, 2, 1, 128);
		GIFReg::GSTex1 tex1{};
		tex1.CMD = SCE_GS_PACK_TEX1(1, 0, 0, 0, 0, 0, 0);
		Renderer::SetTex1(tex1);
		Renderer::Native::PushAnimST(glm::value_ptr(scroll));
		Renderer::Native::PushEnvironmentMapping(glm::value_ptr(cameraX), glm::value_ptr(axisY), glm::value_ptr(transform));
		if (layers) {
			Renderer::Native::RenderMesh(&current, 0);
			Renderer::Native::BindTexture(&texture);
			Renderer::SetAlpha(0, additive ? 2 : 1, fixedBlend ? 2 : 0, 1, fixed);
			Renderer::SetGlobalAlpha(fixedBlend ? 128 : 64);
		}
		Renderer::Native::RenderMesh(&current, flags | (layers ? 0x20 : 0));
		// Change mapping state after submission and queue a rejected instance of
		// the same mesh. The first instance must keep its own flags and vectors.
		Renderer::Native::PushEnvironmentMapping(glm::value_ptr(cameraY), glm::value_ptr(cameraX), glm::value_ptr(rotation));
		GIFReg::GSTest reject{};
		reject.ZTE = 1;
		reject.ZTST = 0;
		Renderer::SetTest(reject);
		Renderer::Native::RenderMesh(&current, 0x40);
		Renderer::Native::RenderMesh(&current, 0);
		Renderer::Native::BindTexture(&texture);
		MainThreadEndCommands(state.renderThread);
		const auto pixel = ReadEnvironmentPixel(state.commandBuffers[GetCurrentFrame()]);
		ResetRenderThread(state.renderThread);
		return pixel;
	};
	const std::array<uint8_t, 4> red = { 200, 0, 0, 128 }, green = { 0, 200, 0, 128 }, white = { 200, 200, 200, 128 };
	EXPECT_EQ(render(mesh, 0, identity, cameraY, glm::vec4(0)), red);
	EXPECT_EQ(render(mesh, 0x40, identity, cameraY, glm::vec4(0)), green);
	EXPECT_EQ(render(mesh, 0x40, rotation, cameraY, glm::vec4(0)), red);
	EXPECT_EQ(render(mesh, 0x40, identity, -cameraY, glm::vec4(0)), white);
	// The VU preserves magnitude and leaves wrapping to the GS sampler.
	glm::mat4 scaled(3.0f);
	scaled[3][3] = 1.0f;
	const std::array<uint8_t, 4> blue = { 0, 0, 200, 128 };
	EXPECT_EQ(render(mesh, 0x40, scaled, cameraY, glm::vec4(0)), blue);
	EXPECT_EQ(render(mesh, 0x240, identity, cameraY, glm::vec4(0, 0.5f, 0, 0)), white);
	EXPECT_EQ(render(noNormals, 0x40, identity, cameraY, glm::vec4(0)), red);
	Renderer::Native::StartAnimMatrix();
	Renderer::Native::PushAnimMatrix(glm::value_ptr(rotation));
	EXPECT_EQ(render(rigid, 0x40, identity, cameraY, glm::vec4(0)), red);
	const auto composed = render(mesh, 0x40, identity, cameraY, glm::vec4(0), true);
	EXPECT_NEAR(composed[0], 100, 1);
	EXPECT_NEAR(composed[1], 100, 1);
	EXPECT_EQ(composed[2], 0);
	const auto fixedComposed = render(mesh, 0x40, identity, cameraY, glm::vec4(0), true, true);
	EXPECT_NEAR(fixedComposed[0], 100, 1);
	EXPECT_NEAR(fixedComposed[1], 100, 1);
	// FIX changes reuse the same pipeline and must update its dynamic constants.
	const auto quarter = render(mesh, 0x40, identity, cameraY, glm::vec4(0), true, true, false, 32);
	EXPECT_NEAR(quarter[0], 150, 1);
	EXPECT_NEAR(quarter[1], 50, 1);
	// Match the reported Blend 281 overlay: Cs * FIX/128 + Cd, even when As=0.
	for (auto& pixel : pixels) pixel &= 0x00ffffff;
	texture.GetRenderer()->Resize(4, 4, sizeof(pixels), reinterpret_cast<uint8_t*>(pixels.data()));
	const auto fixedAdd = render(mesh, 0x40, identity, cameraY, glm::vec4(0), true, true, true);
	EXPECT_EQ(fixedAdd[0], 200);
	EXPECT_NEAR(fixedAdd[1], 100, 1);
	EXPECT_EQ(fixedAdd[2], 0);
	EXPECT_EQ(fixedAdd[3], 0);
	texture.GetRenderer()->DestroyImageResources();
	Renderer::Native::Cleanup();
}
