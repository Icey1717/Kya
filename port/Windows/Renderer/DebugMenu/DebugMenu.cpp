#include "DebugMenu.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "Vulkan/src/VulkanRenderer.h"
#include <stdexcept>
#include "Vulkan/src/VulkanCommands.h"

namespace DebugMenu {
	VkRenderPass g_imguiRenderPass;
}

void DebugMenu::Setup(VkInstance instance, uint32_t queueFamily, VkQueue graphicsQueue, GLFWwindow* window, VkRenderPass renderPass)
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
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

	/*VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;*/

	//if (vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &g_imguiRenderPass) != VK_SUCCESS) {
	//	throw std::runtime_error("failed to create render pass!");
	//}

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
}

void DebugMenu::Render(VkCommandBuffer commandBuffer)
{
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static bool bShowDemoWindow = false;
	ImGui::ShowDemoWindow(&bShowDemoWindow);

	// Calculate the framerate
	static double prevTime = 0.0;
	double currentTime = glfwGetTime();
	double deltaTime = currentTime - prevTime;
	prevTime = currentTime;
	double fps = 1.0 / deltaTime;

	const ImVec2 window_size(100.0f, 25.0f);

	// Render the framerate counter
	ImGui::SetNextWindowSize(window_size);
	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
	ImGui::Begin("Framerate Counter", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("FPS: %.1f", fps);
	ImGui::End();

	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}
