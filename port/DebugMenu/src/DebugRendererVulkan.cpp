#include "DebugRenderer.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "DebugMenu.h"

#include <stdexcept>

#include "renderer.h"
#include "VulkanRenderer.h"
#include "VulkanCommands.h"
#include "TextureCache.h"
#include "FrameBuffer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "DebugMeshViewerVulkan.h"

bool bOther = false;

namespace DebugRendererImgui {
	VkRenderPass gImguiRenderPass;
	Renderer::CommandBufferVector gCommandBuffers;

	void Setup()
	{
		VkDescriptorPool imguiPool;

		// Create Descriptor Pool
		{
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			if (vkCreateDescriptorPool(GetDevice(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
		}

		int minImageCount = 2;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = GetSwapchainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &gImguiRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		SetObjectName(reinterpret_cast<uint64_t>(gImguiRenderPass), VK_OBJECT_TYPE_RENDER_PASS, "Imgui Render Pass");

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = GetInstance();
		initInfo.PhysicalDevice = GetPhysicalDevice();
		initInfo.Device = GetDevice();
		initInfo.QueueFamily = GetGraphicsQueueFamily();
		initInfo.Queue = GetGraphicsQueue();
		initInfo.PipelineCache = nullptr;
		initInfo.DescriptorPool = imguiPool;
		initInfo.Subpass = 0;
		initInfo.MinImageCount = minImageCount;
		initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&initInfo, gImguiRenderPass);
		ImGui_ImplGlfw_InitForVulkan(GetGLFWWindow(), true);

		// Upload Fonts
		{
			VkCommandBuffer fontCommandBuffer = BeginSingleTimeCommands();

			ImGui_ImplVulkan_CreateFontsTexture(fontCommandBuffer);
			EndSingleTimeCommands(fontCommandBuffer);

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		Renderer::CreateCommandBuffers(gCommandBuffers);
	}

	void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent)
	{
		const VkCommandBuffer& cmd = gCommandBuffers[GetCurrentFrame()];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &beginInfo);
		Renderer::Debug::BeginLabel(cmd, "DebugMenu");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = gImguiRenderPass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DebugMenu::BuildImguiCommands();

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(drawData, cmd);

		vkCmdEndRenderPass(cmd);

		Renderer::Debug::EndLabel(cmd);
		vkEndCommandBuffer(cmd);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GetGraphicsQueue());
	}
}

void DebugMenu::SetupRenderer()
{
	DebugRendererImgui::Setup();
	DebugMeshViewer::Vulkan::Setup();

	Renderer::GetRenderDelegate() += DebugMeshViewer::Vulkan::Render;
	Renderer::GetRenderDelegate() += DebugRendererImgui::Render;
}

ImTextureID DebugMenu::AddTexture(const PS2::GSTexImage& texImage)
{
	return ImGui_ImplVulkan_AddTexture(texImage.sampler, texImage.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

struct DebugFrameBuffer {
	DebugFrameBuffer(ImTextureID inTexID, VkSampler inSampler)
		: texID(inTexID)
		, sampler(inSampler)
	{}

	DebugFrameBuffer()
		: texID(nullptr)
		, sampler(VK_NULL_HANDLE)
	{}

	ImTextureID texID;
	VkSampler sampler;
};

std::unordered_map<int, DebugFrameBuffer> gDebugFrameBuffers;

ImTextureID DebugMenu::AddFrameBuffer(const PS2::FrameBuffer& frameBuffer)
{
	if (gDebugFrameBuffers.find(frameBuffer.FBP) == gDebugFrameBuffers.end()) {
		// Create sampler
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;

		VkSampler sampler;
		VkResult result = vkCreateSampler(GetDevice(), &samplerCreateInfo, nullptr, &sampler);
		if (result != VK_SUCCESS) {
			// Handle sampler creation failure
		}

		gDebugFrameBuffers.emplace(frameBuffer.FBP, DebugFrameBuffer(ImGui_ImplVulkan_AddTexture(sampler, frameBuffer.finalImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL), sampler));
	}

	return gDebugFrameBuffers[frameBuffer.FBP].texID;
}
