#include <gtest/gtest.h>

#include "renderer.h"
#include "port.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanBuffer.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanCommands.h"
#include "../../Windows/Renderer/Vulkan/src/Objects/VulkanImage.h"
#include <GLFW/glfw3.h>
#include <array>
#include <cmath>

namespace
{
	std::array<uint8_t, 4> ReadCenterPixel(VkCommandBuffer cmd)
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

// Run alone with --gtest_also_run_disabled_tests --gtest_filter=TextureMipmapRendering.*.
// Requires Vulkan and compiled shaders; the GLFW window remains hidden.
TEST(TextureMipmapRendering, DISABLED_AuthoredAlphaTrilinearQueuedStateAndResize)
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
	std::array<std::vector<uint32_t>, 3> pixels;
	const std::array<uint32_t, 3> colors = { 0x400000c8, 0x8000c800, 0xc0c80000 };
	CombinedImageData image{};
	image.bitmaps.resize(3);
	image.registers.tex.TW = 3;
	image.registers.tex.TH = 2;
	image.registers.tex.TBW = 1;
	image.registers.tex.TBP0 = 0x100;
	image.registers.mipTbp1.TBW1 = 1;
	image.registers.mipTbp1.TBW2 = 1;
	for (uint32_t level = 0; level < 3; ++level) {
		auto& bitmap = image.bitmaps[level];
		bitmap.canvasWidth = 8 >> level;
		bitmap.canvasHeight = 4 >> level;
		bitmap.maxMipLevel = 3;
		bitmap.bitBltBuf.DBP = 0x100 + 0x40 * level;
		bitmap.bitBltBuf.DBW = 1;
		bitmap.trxReg.RRW = bitmap.canvasWidth;
		bitmap.trxReg.RRH = bitmap.canvasHeight;
		pixels[level].assign(bitmap.canvasWidth * bitmap.canvasHeight, colors[level]);
		bitmap.pImage = pixels[level].data();
	}
	SimpleTexture texture("Mip regression", { 0, 0, 1, 1 }, image.registers);
	texture.CreateRenderer(image);
	ASSERT_EQ(texture.GetRenderer()->mipLevels, 3u);
	GIFReg::GSPrim prim{};
	prim.PRIM = 3;
	SimpleMesh mesh("Mip regression triangle", prim, 0);
	auto& vertices = mesh.GetVertexBufferData();
	vertices.Init(3, 3);
	const float positions[3][3] = { { -1, -1, 0.5f }, { 3, -1, 0.5f }, { -1, 3, 0.5f } };
	for (uint32_t i = 0; i < 3; ++i) {
		vertices.vertex.buff[i] = {};
		memcpy(vertices.vertex.buff[i].XYZFlags.fXYZ, positions[i], sizeof(positions[i]));
		for (auto& channel : vertices.vertex.buff[i].RGBA) channel = 128;
		vertices.vertex.buff[i].STQ.ST[0] = vertices.vertex.buff[i].STQ.ST[1] = 2048;
		vertices.index.buff[i] = i;
	}
	vertices.vertex.tail = vertices.vertex.next = vertices.index.tail = 3;
	float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	Renderer::SetTest(GIFReg::GSTest{});
	auto render = [&](uint32_t filter, uint32_t maxLevel, uint32_t bias, uint32_t mag = 0,
		bool fixedLod = true, float qScale = 1.0f, uint32_t lodScale = 0, float clipW = 1.0f,
		bool queueOtherSampler = false, float gsWZ = 0.0f) {
		UpdateRenderPassKey(EClearMode::ColorDepth);
		std::array<float, 16> nativeProjection;
		for (size_t i = 0; i < nativeProjection.size(); ++i) nativeProjection[i] = identity[i] * clipW;
		auto gsProjection = nativeProjection;
		gsProjection[15] = clipW / qScale;
		gsProjection[11] = gsWZ;
		Renderer::Native::PushGlobalMatrices(identity, identity, nativeProjection.data(), gsProjection.data());
		GIFReg::GSTex1 tex1{};
		tex1.CMD = SCE_GS_PACK_TEX1(fixedLod, maxLevel, mag, filter, 0, lodScale, bias);
		Renderer::SetTex1(tex1);
		Renderer::Native::RenderMesh(&mesh, 0);
		if (queueOtherSampler) {
			// Queue another sampler for this texture in the same batch. Reject its
			// fragments so readback still verifies the first instance's binding.
			tex1.CMD = SCE_GS_PACK_TEX1(1, 2, 0, 5, 0, 0, 32);
			Renderer::SetTex1(tex1);
			GIFReg::GSTest reject{};
			reject.ZTE = 1;
			reject.ZTST = 0; // NEVER
			Renderer::SetTest(reject);
			Renderer::Native::RenderMesh(&mesh, 0);
			Renderer::SetTest(GIFReg::GSTest{});
		}
		// A later option change must not overwrite the submitted instance.
		state.cachedPerDrawData.samplingParams = glm::vec2(0.0f, 1.0f);
		tex1.CMD = SCE_GS_PACK_TEX1(1, 0, 1, 1, 0, 0, 0);
		Renderer::SetTex1(tex1);
		Renderer::Native::BindTexture(&texture);
		MainThreadEndCommands(state.renderThread);
		const auto pixel = ReadCenterPixel(state.commandBuffers[GetCurrentFrame()]);
		ResetRenderThread(state.renderThread);
		return pixel;
	};
	const auto base = render(5, 0, 32, 0, true, 1.0f, 0, 1.0f, true);
	const auto last = render(5, 2, 32);
	const auto blend = render(5, 2, 8);
	// Different LOD biases reuse the same image binding. Each sampler variant
	// owns one set, while all native passes use the renderer's frame sets.
	EXPECT_EQ(texture.GetRenderer()->textureBindings.size(), 2u);
	for (const auto& [key, stage] : state.renderPass) {
		const auto& pipeline = stage.GetPipeline();
		ASSERT_EQ(pipeline.descriptorSetLayouts.size(), 2u);
		EXPECT_EQ(pipeline.descriptorSets.size(), 0u);
		EXPECT_EQ(pipeline.descriptorPool, VK_NULL_HANDLE);
		EXPECT_EQ(pipeline.descriptorSetLayoutBindings.at(0).at(EBindingStage::Vertex).size(), 5u);
	}
	EXPECT_EQ(base[0], 200);
	EXPECT_EQ(base[1], 0);
	EXPECT_EQ(base[2], 0);
	EXPECT_EQ(base[3], 64);
	EXPECT_EQ(last[0], 0);
	EXPECT_EQ(last[1], 0);
	EXPECT_EQ(last[2], 200);
	EXPECT_EQ(last[3], 192);
	EXPECT_NEAR(blend[0], 100, 1);
	EXPECT_NEAR(blend[1], 100, 1);
	EXPECT_EQ(blend[2], 0);
	EXPECT_NEAR(blend[3], 96, 1);
	const auto nearest = render(4, 2, 4);
	EXPECT_EQ(nearest, base);
	Renderer::Native::SetForceHighestMipLevel(true);
	EXPECT_EQ(render(1, 0, 0), last); // Override both base-only filtering and MXL=0.
	Renderer::Native::SetForceLowestMipLevel(true); // Replaces the highest override.
	EXPECT_EQ(render(5, 2, 32), base);
	Renderer::Native::SetForceHighestMipLevel(false);
	EXPECT_EQ(render(5, 2, 32), base); // Disabling the inactive override leaves lowest active.
	Renderer::Native::SetForceLowestMipLevel(false);
	EXPECT_EQ(render(5, 2, 32), last);
	EXPECT_EQ(render(1, 0, 0), base);
	// Constant UVs deliberately have no derivatives. Automatic LOD must use
	// GS Q, including the projection scale, signed authored K, and 2^L.
	EXPECT_EQ(render(5, 2, 0, 0, false), base);
	const auto distant = render(5, 2, 0, 0, false, 1.0f, 0, 4.0f);
	for (size_t channel = 0; channel < distant.size(); ++channel) EXPECT_NEAR(distant[channel], last[channel], 1);
	const auto automatic = render(5, 2, uint32_t(-140), 0, false, 1.0f / 1024.0f);
	EXPECT_EQ(automatic[0], 0);
	EXPECT_NEAR(automatic[1], 150, 1);
	EXPECT_NEAR(automatic[2], 50, 1);
	EXPECT_NEAR(automatic[3], 144, 1);
	EXPECT_EQ(render(5, 2, uint32_t(-140), 0, false, 1.0f / 1024.0f, 1), last);

	// Nonproportional W rows: native W=1, GS W=1+cameraZ. At Z=0.5,
	// Q=2/3 and LOD=log2(1.5). A constant W ratio incorrectly gives LOD=0.
	const auto gsOffset = render(5, 2, 0, 0, false, 1.0f, 0, 1.0f, false, 1.0f);
	const float gsLod = std::log2(1.5f);
	EXPECT_NEAR(gsOffset[0], 200.0f * (1.0f - gsLod), 1);
	EXPECT_NEAR(gsOffset[1], 200.0f * gsLod, 1);
	EXPECT_EQ(gsOffset[2], 0);
	EXPECT_NEAR(gsOffset[3], 64.0f + 64.0f * gsLod, 1);
	// Replacement/revert must retain the authored lower levels and descriptors.
	std::vector<uint32_t> replacement(16 * 8, 0x40000032);
	texture.GetRenderer()->Resize(16, 8, int(replacement.size() * 4), reinterpret_cast<uint8_t*>(replacement.data()));
	EXPECT_EQ(render(5, 2, 32), last);
	EXPECT_EQ(render(5, 0, 0)[0], 50);
	for (uint32_t y = 0; y < 8; ++y) {
		for (uint32_t x = 0; x < 16; ++x) replacement[y * 16 + x] = ((x + y) & 1) ? 0x40000032 : 0x400000c8;
	}
	texture.GetRenderer()->Resize(16, 8, int(replacement.size() * 4), reinterpret_cast<uint8_t*>(replacement.data()));
	// Negative fixed LOD uses MMAG, independently of point MMIN.
	EXPECT_EQ(render(0, 0, 0xfffffff0, 0)[0], 200);
	EXPECT_NEAR(render(0, 0, 0xfffffff0, 1)[0], 125, 1);
	texture.GetRenderer()->Resize(8, 4, int(pixels[0].size() * 4), reinterpret_cast<uint8_t*>(pixels[0].data()));
	EXPECT_EQ(render(5, 0, 0), base);
	EXPECT_EQ(render(5, 2, 32), last);
	texture.GetRenderer()->DestroyImageResources();
	Renderer::Native::Cleanup();
}
