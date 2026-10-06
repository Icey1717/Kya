#include <gtest/gtest.h>
#include "renderer.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeShadow.h"
#include "../../Windows/Renderer/Vulkan/src/Native/NativeRendererInternal.h"
#include <GLFW/glfw3.h>
#include <filesystem>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "../../ext/tracy/test/stb_image.h"

TEST(ShadowBufferDump, NoTargetReportsFailureWithoutUsingVulkan)
{
	Renderer::Native::RequestShadowBufferDump();
	Renderer::Native::Shadow::ProcessPendingDump();
	EXPECT_EQ(Renderer::Native::GetShadowBufferDumpStatus(), "No completed shadow buffers available to dump.");
}

// Opt-in GPU test; opens a hidden window and verifies actual exported coverage.
TEST(ShadowBufferDump, DISABLED_GpuReadbackPreservesPixelsAndLayouts)
{
	using namespace Renderer;
	using namespace Renderer::Native;
	ASSERT_TRUE(glfwInit());
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Renderer::SetHeadless(false);
	Renderer::Setup();
	Renderer::Native::Setup();
	struct Cleanup { ~Cleanup() { Renderer::Native::Cleanup(); } } cleanup;
	const uint32_t previousScale = GetShadowResolutionScale();
	struct RestoreScale { uint32_t scale; ~RestoreScale() { SetShadowResolutionScale(scale); } } restoreScale{previousScale};

	// Odd dimensions exercise tightly packed R8 rows. Switch 1x -> 2x -> 1x
	// to verify resizing, cached target reuse, and readback layout restoration.
	for (int iteration = 0; iteration < 3; ++iteration) {
		const uint32_t scale = iteration == 1 ? 2 : 1;
		SetShadowResolutionScale(scale);
		const int expectedWidth = 19 * scale, expectedHeight = 11 * scale;
		Shadow::BeginMask({19, 11, 0, 0, 0x30});
		EXPECT_EQ(GetShadowPassSettings().width, expectedWidth);
		EXPECT_EQ(GetShadowPassSettings().height, expectedHeight);
		RecordBeginCommandBuffer();
		RecordBeginRenderPass({ EClearMode::ColorDepth, ERenderPassKind::ShadowMask });
		const auto cmd = GetNativeRendererState().commandBuffers[GetCurrentFrame()];
		VkClearAttachment attachment{};
		attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachment.clearValue.color.float32[0] = 1.0f;
		VkClearRect rect{ {{iteration ? int32_t(10 * scale) : 0, 0}, {9 * scale, 11 * scale}}, 0, 1 };
		vkCmdClearAttachments(cmd, 1, &attachment, 1, &rect);
		RecordEndRenderPass();
		Shadow::RecordBlur(cmd);
		RecordEndCommandBuffer();
		Debug::EndLabel(cmd);
		ASSERT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
		VkSubmitInfo submit{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;
		ASSERT_EQ(vkQueueSubmit(GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE), VK_SUCCESS);
		RequestShadowBufferDump();
		Shadow::ProcessPendingDump();
		const auto status = GetShadowBufferDumpStatus();
		const std::string prefix = "Saved shadow buffers to ";
		ASSERT_EQ(status.find(prefix), 0u) << status;
		const std::filesystem::path directory(status.substr(prefix.size()));
		for (const char* name : {"caster-mask.png", "blur-output.png"}) {
			int width = 0, height = 0, channels = 0;
			auto* pixels = stbi_load((directory / name).string().c_str(), &width, &height, &channels, 1);
			ASSERT_NE(pixels, nullptr) << name;
			EXPECT_EQ(width, expectedWidth); EXPECT_EQ(height, expectedHeight); EXPECT_EQ(channels, 1);
			if (width == expectedWidth && height == expectedHeight) {
				for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
					EXPECT_EQ(pixels[y * width + x], (iteration ? x >= int(10 * scale) : x < int(9 * scale)) ? 255 : 0) << name << " at " << x << ',' << y;
			}
			stbi_image_free(pixels);
			std::filesystem::remove(directory / name);
		}
		EXPECT_TRUE(std::filesystem::exists(directory / "settings.txt"));
		std::filesystem::remove(directory / "settings.txt");
		std::filesystem::remove(directory);
	}
}
