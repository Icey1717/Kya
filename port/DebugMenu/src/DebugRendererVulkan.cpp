#include "DebugMenu.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include "DebugMenu.h"

#include <stdexcept>

#include "VulkanRenderer.h"
#include "VulkanCommands.h"
#include "TextureCache.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace DebugMenu {
	VkRenderPass g_imguiRenderPass;
	std::vector<VkCommandBuffer> commandBuffers;
}

void DebugMenu::Setup()
{
	VkDescriptorPool imguiPool;
	g_imguiRenderPass = renderPass;

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

	if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &g_imguiRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = instance;
	initInfo.PhysicalDevice = GetPhysicalDevice();
	initInfo.Device = GetDevice();
	initInfo.QueueFamily = queueFamily;
	initInfo.Queue = graphicsQueue;
	initInfo.PipelineCache = nullptr;
	initInfo.DescriptorPool = imguiPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = minImageCount;
	initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&initInfo, g_imguiRenderPass);
	ImGui_ImplGlfw_InitForVulkan(window, true);

	// Upload Fonts
	{
		VkCommandBuffer fontCommandBuffer = BeginSingleTimeCommands();

		ImGui_ImplVulkan_CreateFontsTexture(fontCommandBuffer);
		EndSingleTimeCommands(fontCommandBuffer);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = GetCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(GetDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	GetRenderDelegate() += DebugMenu::Render;
}

void DebugMenu::Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent)
{
	const VkCommandBuffer& cmd = commandBuffers[GetCurrentFrame()];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd, &beginInfo);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = g_imguiRenderPass;
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

	BuildImguiCommands();

	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(drawData, cmd);

	vkCmdEndRenderPass(cmd);
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GetGraphicsQueue());
}
