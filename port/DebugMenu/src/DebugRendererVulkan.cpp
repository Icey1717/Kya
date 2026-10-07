#include "DebugRenderer.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#ifdef ENABLE_GOOGLE_FONT_LOADER
#include "GoogleFontLoader.h"
#endif
#include "DebugMenu.h"
#include "DebugSetting.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <stdexcept>

#include "renderer.h"
#include "VulkanRenderer.h"
#include "Objects/VulkanCommands.h"
#include "Texture/TextureCache.h"
#include "Objects/FrameBuffer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "profiling.h"

#include "DebugMeshViewerVulkan.h"
#include "Native/NativeRenderer.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "FreeCamera.h"

// Installed at the top of the WndProc chain (after ImGui installs its own).
// Intercepts WM_NCHITTEST for client-area cursor positions and returns HTCLIENT
// immediately, preventing DefWindowProcW from triggering the uxtheme non-client
// cascade (CThemeWnd::NcHitTest / NcGetWindowDPI) that otherwise fires on every
// mouse message and consumes ~25% of frame time.
static WNDPROC gPrevKyaWndProc = nullptr;

static LRESULT CALLBACK KyaWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Fast path for WM_NCHITTEST: return HTCLIENT immediately for client-area cursor
	// positions. Without this, DefWindowProcW triggers the uxtheme non-client cascade
	// (CThemeWnd::NcHitTest, NcGetWindowDPI) on every mouse message.
	if (msg == WM_NCHITTEST)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		RECT rc;
		GetClientRect(hWnd, &rc);
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<LPPOINT>(&rc), 2);
		if (PtInRect(&rc, pt))
			return HTCLIENT;
	}

	// When free camera controls are enabled, GLFW_CURSOR_DISABLED is set and GLFW uses
	// WM_INPUT (raw mouse) for delta — it ignores WM_MOUSEMOVE entirely. ImGui
	// doesn't need WM_MOUSEMOVE or WM_SETCURSOR either when the cursor is hidden.
	// Eating them here prevents the pointless KyaWndProc→ImGui(GetPropA×2)→
	// GLFW(GetPropW) dispatch chain from running on every mouse message.
	if (CFreeCamera::IsCapturingInput())
	{
		switch (msg)
		{
		case WM_INPUT:
		{
			// Extract the raw mouse delta directly and feed it to the camera,
			// then eat the message. This bypasses ImGui's GetPropA×2 and GLFW's
			// GetPropW entirely — they add no value for WM_INPUT.
			RAWINPUT ri;
			UINT size = sizeof(ri);
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &ri, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1
				&& ri.header.dwType == RIM_TYPEMOUSE
				&& (ri.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
			{
				CFreeCamera::AccumulateRawDelta((int)ri.data.mouse.lLastX, (int)ri.data.mouse.lLastY);
			}
			return 0;
		}
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
			return 0;
		case WM_SETCURSOR:
			return TRUE;   // cursor is hidden; suppress DefWindowProcW from resetting it
		}
	}

	return CallWindowProcW(gPrevKyaWndProc, hWnd, msg, wParam, lParam);
}
#endif

bool bOther = false;

namespace DebugRendererImgui {
	VkRenderPass gImguiRenderPass;
	Renderer::CommandBufferVector gCommandBuffers;

	static Debug::Setting<float> gUiScale("Debug UI Scale", 1.25f);
	static ImGuiStyle gBaseStyle;
	static float gAppliedUiScale = 0.0f;

	static void ApplyUiScale()
	{
		const float savedScale = gUiScale.get();
		const float scale = std::isfinite(savedScale) ? std::clamp(savedScale, 1.0f, 2.0f) : 1.25f;
		if (scale == gAppliedUiScale) return;

		// Always scale from the baseline to avoid accumulating rounding errors.
		ImGuiStyle& style = ImGui::GetStyle();
		const float fontSizeBase = style.FontSizeBase;
		const float fontScaleDpi = style.FontScaleDpi;
		ImVec4 colors[ImGuiCol_COUNT];
		std::copy_n(style.Colors, ImGuiCol_COUNT, colors);
		style = gBaseStyle;
		style.ScaleAllSizes(scale);
		style.FontSizeBase = fontSizeBase;
		style.FontScaleDpi = fontScaleDpi;
		style.FontScaleMain = scale;
		std::copy_n(colors, ImGuiCol_COUNT, style.Colors);
		gAppliedUiScale = scale;
	}

	static void ShowAppearanceMenu(bool* pOpen)
	{
		const float fontSize = ImGui::GetFontSize();
		ImGui::SetNextWindowSize(ImVec2(fontSize * 22.0f, fontSize * 9.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(fontSize * 18.0f, fontSize * 8.0f), ImVec2(FLT_MAX, FLT_MAX));
		if (ImGui::Begin("Appearance", pOpen)) {
			float scale = gAppliedUiScale;
			ImGui::TextUnformatted("UI scale");
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::SliderFloat("##UiScale", &scale, 1.0f, 2.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp)) {
				gUiScale = scale;
			}
			ImGui::TextWrapped("Scales text, controls, and spacing. Changes apply immediately and are saved.");
			if (ImGui::Button("Reset to 125%")) {
				gUiScale = 1.25f;
			}
		}
		ImGui::End();
	}

	static Debug::MenuRegisterer gAppearanceMenu("Appearance", ShowAppearanceMenu);

	static ImTextureID ToImTextureID(VkDescriptorSet descriptorSet)
	{
		return (ImTextureID)(uintptr_t)descriptorSet;
	}

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
			if (vkCreateDescriptorPool(GetDevice(), &pool_info, GetAllocator(), &imguiPool) != VK_SUCCESS) {
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

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(GetDevice(), &renderPassInfo, GetAllocator(), &gImguiRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		SetObjectName(reinterpret_cast<uint64_t>(gImguiRenderPass), VK_OBJECT_TYPE_RENDER_PASS, "Imgui Render Pass");

		ImGui::CreateContext();
		bool fontLoaded = false;
#ifdef ENABLE_GOOGLE_FONT_LOADER
		fontLoaded = GoogleFontLoader::LoadIntoImGui("Roboto", "regular", 16.0f);
#endif
		if (!fontLoaded) {
			ImFontConfig fontConfig;
			fontConfig.SizePixels = 18.0f;
			ImGui::GetIO().Fonts->AddFontDefault(&fontConfig);
		}
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.078f, 0.098f, 0.122f, 1.0f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.078f, 0.098f, 0.122f, 1.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.110f, 0.141f, 0.176f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.62f, 0.68f, 0.74f, 1.0f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.24f, 0.30f, 0.36f, 1.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.055f, 0.075f, 0.10f, 1.0f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.21f, 0.28f, 1.0f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.29f, 0.40f, 1.0f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.15f, 0.19f, 0.24f, 1.0f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.23f, 0.33f, 0.44f, 1.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.21f, 0.39f, 0.55f, 1.0f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.21f, 0.39f, 0.55f, 0.85f);
		style.Colors[ImGuiCol_TabSelected] = ImVec4(0.21f, 0.39f, 0.55f, 1.0f);
		style.Colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.53f, 0.73f, 0.90f, 1.0f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.28f, 0.36f, 1.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.39f, 0.52f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.21f, 0.39f, 0.55f, 1.0f);
		style.FrameBorderSize = 1.0f;
		style.FrameRounding = 2.0f;
		style.GrabRounding = 2.0f;
		gBaseStyle = ImGui::GetStyle();
		gBaseStyle.WindowPadding = ImVec2(10.0f, 10.0f);
		gBaseStyle.FramePadding = ImVec2(6.0f, 4.0f);
		gBaseStyle.ItemSpacing = ImVec2(8.0f, 6.0f);
		gBaseStyle.CellPadding = ImVec2(4.0f, 3.0f);
		gAppliedUiScale = 0.0f;
		ApplyUiScale();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = GetInstance();
		initInfo.PhysicalDevice = GetPhysicalDevice();
		initInfo.Device = GetDevice();
		initInfo.QueueFamily = GetGraphicsQueueFamily();
		initInfo.Queue = GetGraphicsQueue();
		initInfo.PipelineCache = nullptr;
		initInfo.DescriptorPool = imguiPool;
		initInfo.MinImageCount = minImageCount;
		initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
		initInfo.Allocator = GetAllocator();
		initInfo.PipelineInfoMain.RenderPass = gImguiRenderPass;
		initInfo.PipelineInfoMain.Subpass = 0;
		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&initInfo);
		ImGui_ImplGlfw_InitForVulkan(GetGLFWWindow(), true);

#ifdef _WIN32
		{
			// ImGui has just installed its WndProc hook via SetWindowLongPtrW.
			// We install ours on top so we're first in the chain.
			HWND hwnd = (HWND)ImGui::GetMainViewport()->PlatformHandleRaw;
			gPrevKyaWndProc = (WNDPROC)::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)KyaWndProc);
		}
#endif

		Renderer::CreateCommandBuffers(gCommandBuffers);
	}

	void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent, Renderer::CommandBufferList& commandBufferList)
	{
		ZONE_SCOPED;

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
		ApplyUiScale();
		ImGui::NewFrame();

		DebugMenu::BuildImguiCommands();

		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(drawData, cmd);

		vkCmdEndRenderPass(cmd);

		Renderer::Debug::EndLabel(cmd);
		vkEndCommandBuffer(cmd);

		commandBufferList.push_back(cmd);
	}
}

void DebugMenu::SetupRenderer()
{
	DebugRendererImgui::Setup();
	DebugMeshViewer::Vulkan::Setup();

	Renderer::GetRenderDelegate() += DebugMeshViewer::Vulkan::Render;
	Renderer::GetRenderDelegate() += DebugRendererImgui::Render;
}

struct DebugFrameBuffer {
	DebugFrameBuffer(ImTextureID inTexID, VkSampler inSampler)
		: texID(inTexID)
		, sampler(inSampler)
	{}

	DebugFrameBuffer()
		: texID(0)
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
		VkResult result = vkCreateSampler(GetDevice(), &samplerCreateInfo, GetAllocator(), &sampler);
		if (result != VK_SUCCESS) {
			// Handle sampler creation failure
		}

		gDebugFrameBuffers.emplace(frameBuffer.FBP, DebugFrameBuffer(DebugRendererImgui::ToImTextureID(ImGui_ImplVulkan_AddTexture(sampler, frameBuffer.finalImageView, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL)), sampler));
	}

	return gDebugFrameBuffers[frameBuffer.FBP].texID;
}

	ImTextureID DebugMenu::AddNativeFrameBuffer()
	{
	return DebugRendererImgui::ToImTextureID(ImGui_ImplVulkan_AddTexture(Renderer::Native::GetSampler(), Renderer::Native::GetColorImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	}

	void DebugMenu::RefreshNativeFrameBuffer(ImTextureID& existingTextureId)
	{
		static VkExtent2D sLastSize = Renderer::Native::GetFrameBufferSize();
		const VkExtent2D currentSize = Renderer::Native::GetFrameBufferSize();
		if (currentSize.width == sLastSize.width && currentSize.height == sLastSize.height)
		{
			return;
		}
		sLastSize = currentSize;
		ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)(uintptr_t)existingTextureId);
		existingTextureId = (ImTextureID)(uintptr_t)ImGui_ImplVulkan_AddTexture(
			Renderer::Native::GetSampler(),
			Renderer::Native::GetColorImageView(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	ImTextureID DebugMenu::GetNativeShadowBuffer(bool blurred)
	{
		static VkImageView cachedViews[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		static ImTextureID textureIds[2] = { 0, 0 };
		const int index = blurred ? 1 : 0;
		const VkImageView imageView = blurred
			? Renderer::Native::GetShadowBlurImageView()
			: Renderer::Native::GetShadowMaskImageView();
		if (imageView == VK_NULL_HANDLE) return 0;
		if (cachedViews[index] != imageView) {
			if (textureIds[index] != 0) {
				ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)(uintptr_t)textureIds[index]);
			}
			cachedViews[index] = imageView;
			textureIds[index] = DebugRendererImgui::ToImTextureID(ImGui_ImplVulkan_AddTexture(
				Renderer::Native::GetShadowSampler(), imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		}
		return textureIds[index];
	}

static ImTextureID gActorPreviewTextureID = 0;

ImTextureID DebugMenu::GetActorPreviewTextureID()
{
	if (gActorPreviewTextureID == 0) {
		gActorPreviewTextureID = DebugRendererImgui::ToImTextureID(
			ImGui_ImplVulkan_AddTexture(
				Renderer::Native::GetPreviewSampler(),
				Renderer::Native::GetPreviewColorImageView(),
				VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL));
	}
	return gActorPreviewTextureID;
}
