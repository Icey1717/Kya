#include <winsock2.h>
#include <gtest/gtest.h>

#include "renderer.h"
#include "VulkanRenderer.h"
#include "Native/NativeRenderer.h"
#include "displaylist.h"
#include "../../DebugMenu/src/DebugSaveScreenshot.h"
#include "../../DebugMenu/src/DebugSocket.h"
#include <nlohmann/json.hpp>
#include <GLFW/glfw3.h>
#include <array>
#include <chrono>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "../../ext/tracy/test/stb_image.h"

namespace
{
	VkRenderPass windowPass;
	Renderer::CommandBufferVector windowCommands;
	int windowFrames;

	void RenderWindow(const VkFramebuffer& framebuffer, const VkExtent2D& extent, Renderer::CommandBufferList& commands)
	{
		const auto cmd = windowCommands[GetCurrentFrame()];
		VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		ASSERT_EQ(vkBeginCommandBuffer(cmd, &begin), VK_SUCCESS);
		VkClearValue clear{};
		VkRenderPassBeginInfo pass{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		pass.renderPass = windowPass;
		pass.framebuffer = framebuffer;
		pass.renderArea.extent = extent;
		pass.clearValueCount = 1;
		pass.pClearValues = &clear;
		vkCmdBeginRenderPass(cmd, &pass, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(cmd);
		ASSERT_EQ(vkEndCommandBuffer(cmd), VK_SUCCESS);
		commands.push_back(cmd);
		++windowFrames;
	}

	bool WaitForMinimized(bool minimized)
	{
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
		while ((glfwGetWindowAttrib(GetGLFWWindow(), GLFW_ICONIFIED) != 0) != minimized) {
			if (std::chrono::steady_clock::now() >= deadline) return false;
			glfwWaitEventsTimeout(0.01);
		}
		return true;
	}
}

// Run alone: creates and minimizes a Vulkan window; requires an interactive desktop.
TEST(MinimizedRendering, DISABLED_FreshScreenshotsAndRestoreAcrossFrameFenceReuse)
{
	using namespace Renderer;
	ASSERT_TRUE(glfwInit());
	Renderer::SetHeadless(false);
	Renderer::Setup();
	glfwSetWindowSize(GetGLFWWindow(), 320, 240);
	Renderer::CreateCommandBuffers(windowCommands);
	VkAttachmentDescription color{};
	color.format = GetSwapchainImageFormat();
	color.samples = VK_SAMPLE_COUNT_1_BIT;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference attachment{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachment;
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstStageMask = dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	VkRenderPassCreateInfo pass{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	pass.attachmentCount = pass.subpassCount = pass.dependencyCount = 1;
	pass.pAttachments = &color;
	pass.pSubpasses = &subpass;
	pass.pDependencies = &dependency;
	ASSERT_EQ(vkCreateRenderPass(GetDevice(), &pass, GetAllocator(), &windowPass), VK_SUCCESS);
	// Match the game's ordering: window UI first, native renderer second.
	Renderer::GetRenderDelegate() += RenderWindow;
	Renderer::Native::Setup();
	Renderer::SetRenderWhenMinimized(true);
	ASSERT_TRUE(::Debug::Socket::Start(0));
	Renderer::GetFrameSubmittedDelegate() += ::Debug::Socket::Poll;
	windowFrames = 0;

	for (int cycle = 0; cycle < 2; ++cycle) {
		glfwIconifyWindow(GetGLFWWindow());
		ASSERT_TRUE(WaitForMinimized(true));
		for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT + 1; ++frame) {
			SCOPED_TRACE(frame);
			Renderer::WaitUntilReady();
			ASSERT_TRUE(Renderer::IsRenderingOffscreen());
			const auto client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			ASSERT_NE(client, INVALID_SOCKET);
			sockaddr_in address{};
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			address.sin_port = htons(::Debug::Socket::GetPort());
			ASSERT_EQ(connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
			const std::string request = "{\"command\":\"screenshot\",\"id\":123}\n";
			ASSERT_EQ(send(client, request.data(), int(request.size()), 0), request.size());
			u_long nonblocking = 1;
			ASSERT_EQ(ioctlsocket(client, FIONBIO, &nonblocking), 0);
			const uint8_t shade = frame % 2 ? 192 : 64;
			Renderer::SetTest(GIFReg::GSTest{});
			Renderer::SetAlpha(0, 1, 2, 1, 128);
			Renderer::DisplayList::BindTexture(nullptr);
			Renderer::DisplayList::Begin2D(100, 100, GS_TRIANGLELIST);
			Renderer::DisplayList::SetColor(shade, shade, shade, 128, 1);
			for (const auto& xy : std::array<std::array<float, 2>, 6>{ {
				{ 0, 0 }, { 100, 0 }, { 0, 100 }, { 100, 0 }, { 100, 100 }, { 0, 100 } } }) {
				Renderer::DisplayList::SetVertex(xy[0], xy[1], 0, 0);
			}
			Renderer::DisplayList::End2D();
			Renderer::Native::OnVideoFlip();
			Renderer::Present();
			std::string wire;
			for (int attempt = 0; attempt < 100; ++attempt) {
				char buffer[65536];
				const int count = recv(client, buffer, sizeof(buffer), 0);
				if (count > 0) wire.append(buffer, count);
				if (count == 0) break;
				::Debug::Socket::Poll();
				Sleep(1);
			}
			closesocket(client);
			const auto newline = wire.find('\n');
			ASSERT_NE(newline, std::string::npos);
			const auto response = nlohmann::json::parse(wire.substr(0, newline));
			ASSERT_EQ(response["ok"], true) << response.dump();
			EXPECT_EQ(response["offscreen"], true);
			EXPECT_EQ(response["id"], 123);
			EXPECT_EQ(response["bytes"], wire.size() - newline - 1);
			int width, height, components;
			auto* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(wire.data() + newline + 1),
				int(wire.size() - newline - 1), &width, &height, &components, 4);
			ASSERT_NE(pixels, nullptr);
			EXPECT_EQ(width, response["width"]);
			EXPECT_EQ(height, response["height"]);
			EXPECT_EQ(width, Renderer::Native::GetFrameBufferSize().width);
			for (int channel = 0; channel < 3; ++channel) EXPECT_NEAR(pixels[channel], shade, 1);
			stbi_image_free(pixels);
			const auto screenshot = ::Debug::SaveLoad::CaptureScreenshot();
			ASSERT_GT(screenshot.size(), 58u);
			// CaptureScreenshot returns a top-down BMP with a 54-byte header.
			for (int channel = 0; channel < 3; ++channel) {
				EXPECT_NEAR(static_cast<uint8_t>(screenshot[54 + channel]), shade, 1);
			}
			EXPECT_EQ(windowFrames, cycle);
		}
		glfwRestoreWindow(GetGLFWWindow());
		ASSERT_TRUE(WaitForMinimized(false));
		if (cycle == 1) Renderer::SetRenderWhenMinimized(false);
		Renderer::WaitUntilReady();
		EXPECT_FALSE(Renderer::IsRenderingOffscreen());
		Renderer::Native::OnVideoFlip();
		Renderer::Present();
		EXPECT_EQ(windowFrames, cycle + 1);
	}
	ASSERT_EQ(vkDeviceWaitIdle(GetDevice()), VK_SUCCESS);
	vkDestroyRenderPass(GetDevice(), windowPass, GetAllocator());
	::Debug::Socket::Stop();
	Renderer::Native::Cleanup();
}
