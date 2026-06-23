#include "VulkanRenderer.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <mutex>
#include <string>

#include "Objects/VulkanBuffer.h"
#include "Objects/VulkanImage.h"
#include "Objects/VulkanColorImage.h"
#include "Objects/VulkanShader.h"

#include "spirv_reflect.h"
#include "renderer.h"
#include "VulkanPS2.h"
#include "imgui.h"
#include "Objects/VulkanCommands.h"
#include "Objects/FrameBuffer.h"
#include "log.h"
#include "Native/NativeRenderer.h"
#include "Texture/TextureUpdate.h"
#include "Texture/TextureUpscale.h"

#include "profiling.h"

//constexpr uint32_t WIDTH = 640;
//constexpr uint32_t HEIGHT = 480;

constexpr uint32_t WIDTH = 3840;
constexpr uint32_t HEIGHT = 2160;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
	VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
	VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME,
};

// Optional extensions enabled when supported (e.g. for OBS Game Capture D3D11 interop)
const std::vector<const char*> optionalDeviceExtensions = {
	VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
};

const std::vector<const char*> instanceExtensions = {
	VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 
};

namespace Renderer {
	bool gHeadless = false;
}

void Renderer::CreateCommandBuffers(CommandBufferVector& commandBuffers, const char* name /*= nullptr*/) {
	CreateCommandBuffers(GetCommandPool(), commandBuffers);
}

void Renderer::CreateCommandBuffers(const VkCommandPool& pool, CommandBufferVector& commandBuffers, const char* name /*= nullptr*/)
{
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	CheckVk(vkAllocateCommandBuffers(GetDevice(), &allocInfo, commandBuffers.data()), "vkAllocateCommandBuffers");

	if (name) {
		for (size_t i = 0; i < commandBuffers.size(); i++) {
			SetObjectName(reinterpret_cast<uint64_t>(commandBuffers[i]), VK_OBJECT_TYPE_COMMAND_BUFFER, "%s %d", name, i);
		}
	}
}

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct RequiredDeviceFeatures {
	bool samplerAnisotropy = true;
	bool geometryShader = true;
	bool fillModeNonSolid = true;
	bool dualSrcBlend = true;
	bool synchronization2 = true;
	bool colorWriteEnable = true;
	bool extendedDynamicState3ColorBlendEnable = true;
	bool extendedDynamicState3ColorBlendEquation = true;
	bool extendedDynamicState3ColorWriteMask = true;
};

std::string JoinNames(const std::vector<const char*>& names)
{
	std::string result;
	for (const char* name : names) {
		if (!result.empty()) {
			result += ", ";
		}
		result += name;
	}

	return result;
}

std::string JoinStrings(const std::vector<std::string>& names)
{
	std::string result;
	for (const std::string& name : names) {
		if (!result.empty()) {
			result += ", ";
		}
		result += name;
	}

	return result;
}

std::vector<const char*> CollectMissingRequiredDeviceFeatures(VkPhysicalDevice physicalDevice, const RequiredDeviceFeatures& required = {})
{
	VkPhysicalDeviceColorWriteEnableFeaturesEXT colorWriteFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT };

	VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT };
	extendedDynamicState3.pNext = &colorWriteFeatures;

	VkPhysicalDeviceVulkan13Features deviceFeatures13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	deviceFeatures13.pNext = &extendedDynamicState3;

	VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures.pNext = &deviceFeatures13;

	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

	std::vector<const char*> missing;
	if (required.samplerAnisotropy && !deviceFeatures.features.samplerAnisotropy) missing.push_back("samplerAnisotropy");
	if (required.geometryShader && !deviceFeatures.features.geometryShader) missing.push_back("geometryShader");
	if (required.fillModeNonSolid && !deviceFeatures.features.fillModeNonSolid) missing.push_back("fillModeNonSolid");
	if (required.dualSrcBlend && !deviceFeatures.features.dualSrcBlend) missing.push_back("dualSrcBlend");
	if (required.synchronization2 && !deviceFeatures13.synchronization2) missing.push_back("synchronization2");
	if (required.colorWriteEnable && !colorWriteFeatures.colorWriteEnable) missing.push_back("colorWriteEnable");
	if (required.extendedDynamicState3ColorBlendEnable && !extendedDynamicState3.extendedDynamicState3ColorBlendEnable) missing.push_back("extendedDynamicState3ColorBlendEnable");
	if (required.extendedDynamicState3ColorBlendEquation && !extendedDynamicState3.extendedDynamicState3ColorBlendEquation) missing.push_back("extendedDynamicState3ColorBlendEquation");
	if (required.extendedDynamicState3ColorWriteMask && !extendedDynamicState3.extendedDynamicState3ColorWriteMask) missing.push_back("extendedDynamicState3ColorWriteMask");

	return missing;
}

void ValidateRequiredDeviceFeatures(VkPhysicalDevice physicalDevice)
{
	const std::vector<const char*> missing = CollectMissingRequiredDeviceFeatures(physicalDevice);
	if (!missing.empty()) {
		throw std::runtime_error("physical device is missing required Vulkan features: " + JoinNames(missing));
	}
}

std::vector<std::string> CollectMissingDeviceExtensions(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	CheckVk(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr), "vkEnumerateDeviceExtensionProperties(count)");

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	CheckVk(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()), "vkEnumerateDeviceExtensionProperties");

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return std::vector<std::string>(requiredExtensions.begin(), requiredExtensions.end());
}

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

//const std::vector<Vertex> vertices = {
//	{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//	{{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//	{{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//	{{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
//};

const std::vector<Vertex> vertices = {
	{{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};


const std::vector<uint16_t> indices = {
	//0, 1, 2, 2, 3, 0
	0, 3, 2, 2, 1, 0
};

static int windowPosX, windowPosY, windowWidth, windowHeight;

class ImageRendererApp {
public:

	void setup() {
		initWindow();
		initVulkan();
		Renderer::Native::InitUpscalerONNX("../../upscaling/models/4x-UltraSharpV2_fp32_op17.onnx");
	}

	void run() {
		initWindow();
		initVulkan();
		//mainLoop();
		cleanup();
	}

	VkPhysicalDevice GetPhysicalDevice() {
		return physicalDevice;
	}

	VkDevice GetDevice() {
		return device;
	}

	GLFWwindow* GetGLFWWindow() {
		return window;
	}

	VkInstance GetInstance() {
		return instance;
	}

	uint32_t GetGraphicsQueueFamily() {
		const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		return indices.graphicsFamily.value();
	}

	VkFormat GetSwapchainImageFormat() {
		return swapChainImageFormat;
	}

	std::vector<VkImage>& GetSwapchainImages() {
		return swapChainImages;
	}

	const VkCommandBuffer& GetCurrentCommandBuffer() {
		return commandBuffers[currentFrame];
	}

	uint32_t GetCurrentFrame() {
		return currentFrame;
	}

	const VkBuffer& GetUniformBuffer(int index) { return uniformBuffers[index]; }
	const VkQueue& GetGraphicsQueue() { return graphicsQueue; }
	const VkCommandPool& GetCommandPool() { return commandPool; }
	const VkExtent2D& GetSwapchainExtent() { return swapChainExtent; }
	const Renderer::VulkanContext& GetVulkanContext() const { return context; }
	const Renderer::SwapchainContext& GetSwapchainContext() const { return swapchainContext; }
	void SetAllocator(VkAllocationCallbacks* allocator) { context.allocator = allocator; }

private:
	void RefreshSwapchainContext()
	{
		swapchainContext.swapchain = swapChain;
		swapchainContext.imageFormat = swapChainImageFormat;
		swapchainContext.extent = swapChainExtent;
		swapchainContext.images = &swapChainImages;
	}

	GLFWwindow* window = nullptr;

	Renderer::VulkanContext context;
	Renderer::SwapchainContext swapchainContext;

	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;

	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapChainExtent = {};
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;

	VkCommandPool commandPool = VK_NULL_HANDLE;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;

	bool framebufferResized = false;

	VulkanColorImage* currentImage;

	static void ToggleFullscreen(GLFWwindow* window)
	{
		static bool fullscreen = false;

		if (!fullscreen)
		{
			// Save current windowed position/size
			glfwGetWindowPos(window, &windowPosX, &windowPosY);
			glfwGetWindowSize(window, &windowWidth, &windowHeight);

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			glfwSetWindowMonitor(window,
				monitor,
				0, 0,
				mode->width,
				mode->height,
				mode->refreshRate);

			fullscreen = true;
		}
		else
		{
			glfwSetWindowMonitor(window,
				NULL,
				windowPosX,
				windowPosY,
				WIDTH,
				HEIGHT,
				0);

			fullscreen = false;
		}
	}

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		windowWidth = WIDTH;
		windowHeight = HEIGHT;

		window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		
		windowPosX = 0;
		windowPosY = 0;

		glfwMaximizeWindow(window);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<ImageRendererApp*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			CheckVk(vkCreateSemaphore(device, &semaphoreInfo, GetAllocator(), &imageAvailableSemaphores[i]), "vkCreateSemaphore(imageAvailable)");
			CheckVk(vkCreateFence(device, &fenceInfo, GetAllocator(), &inFlightFences[i]), "vkCreateFence(inFlight)");
		}

		createRenderFinishedSemaphores();
	}

	void createRenderFinishedSemaphores() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		renderFinishedSemaphores.resize(swapChainImages.size());
		for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
			CheckVk(vkCreateSemaphore(device, &semaphoreInfo, GetAllocator(), &renderFinishedSemaphores[i]), "vkCreateSemaphore(renderFinished)");
		}
	}

	void destroyRenderFinishedSemaphores() {
		for (VkSemaphore semaphore : renderFinishedSemaphores) {
			vkDestroySemaphore(device, semaphore, GetAllocator());
		}

		renderFinishedSemaphores.clear();
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGlobalRenderPass();
		PS2::Setup();
		createFramebuffers();
		createSyncObjects();
		commandPool = Renderer::CreateCommandPool("Image Renderer Command Pool");
		context.commandPool = commandPool;
	}

	void cleanupSwapChain() {
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, GetAllocator());
		}

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, GetAllocator());
		}

		vkDestroySwapchainKHR(device, swapChain, GetAllocator());
	}

	void cleanup() {
		if (device != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(device);
		}

		cleanupSwapChain();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(GetDevice(), uniformBuffers[i], GetAllocator());
			vkFreeMemory(GetDevice(), uniformBuffersMemory[i], GetAllocator());
		}

		PS2::Cleanup();
		Renderer::Native::Cleanup();

		vkDestroyPipeline(device, graphicsPipeline, GetAllocator());
		vkDestroyPipelineLayout(device, pipelineLayout, GetAllocator());
		if (renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, renderPass, GetAllocator());
			renderPass = VK_NULL_HANDLE;
		}

		vkDestroyBuffer(device, indexBuffer, GetAllocator());
		vkFreeMemory(device, indexBufferMemory, GetAllocator());

		vkDestroyBuffer(device, vertexBuffer, GetAllocator());
		vkFreeMemory(device, vertexBufferMemory, GetAllocator());

		destroyRenderFinishedSemaphores();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, imageAvailableSemaphores[i], GetAllocator());
			vkDestroyFence(device, inFlightFences[i], GetAllocator());
		}

		vkDestroyCommandPool(device, commandPool, GetAllocator());

		vkDestroyDevice(device, GetAllocator());

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, GetAllocator());
		}

		vkDestroySurfaceKHR(instance, surface, GetAllocator());
		vkDestroyInstance(instance, GetAllocator());

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);

		cleanupSwapChain();
		destroyRenderFinishedSemaphores();
		if (renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, renderPass, GetAllocator());
			renderPass = VK_NULL_HANDLE;
		}

		createSwapChain();
		createImageViews();
		createGlobalRenderPass();
		createFramebuffers();
		createRenderFinishedSemaphores();
		RefreshSwapchainContext();
	}

	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Image Renderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		CheckVk(vkCreateInstance(&createInfo, GetAllocator(), &instance), "vkCreateInstance");

		context.instance = instance;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = ValidationErrorCallback;
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		CheckVk(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "vkCreateDebugUtilsMessengerEXT");
	}

	void createSurface() {
		CheckVk(glfwCreateWindowSurface(instance, window, nullptr, &surface), "glfwCreateWindowSurface");
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		CheckVk(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr), "vkEnumeratePhysicalDevices(count)");

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		CheckVk(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()), "vkEnumeratePhysicalDevices");

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			std::string message = "failed to find a suitable GPU";
			if (!devices.empty()) {
				const std::vector<std::string> missingExtensions = CollectMissingDeviceExtensions(devices[0]);
				const std::vector<const char*> missingFeatures = CollectMissingRequiredDeviceFeatures(devices[0]);

				if (!missingExtensions.empty()) {
					message += "; first device missing extensions: " + JoinStrings(missingExtensions);
				}
				if (!missingFeatures.empty()) {
					message += "; first device missing features: " + JoinNames(missingFeatures);
				}
			}
			throw std::runtime_error(message);
		}

		context.physicalDevice = physicalDevice;
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		ValidateRequiredDeviceFeatures(physicalDevice);

		VkPhysicalDeviceColorWriteEnableFeaturesEXT colorWriteFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT };
		colorWriteFeatures.colorWriteEnable = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT };
		extendedDynamicState3.extendedDynamicState3ColorBlendEnable = VK_TRUE;
		extendedDynamicState3.extendedDynamicState3ColorBlendEquation = VK_TRUE;
		extendedDynamicState3.extendedDynamicState3ColorWriteMask = VK_TRUE;
		extendedDynamicState3.pNext = &colorWriteFeatures;

		VkPhysicalDeviceVulkan13Features deviceFeatures13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		deviceFeatures13.synchronization2 = VK_TRUE;
		deviceFeatures13.pNext = &extendedDynamicState3;

		VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		deviceFeatures.features.samplerAnisotropy = VK_TRUE;
		deviceFeatures.features.geometryShader = VK_TRUE;
		deviceFeatures.features.fillModeNonSolid = VK_TRUE;
		deviceFeatures.features.dualSrcBlend = VK_TRUE;

		deviceFeatures.pNext = &deviceFeatures13;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pNext = &deviceFeatures;

		// Collect optional extensions that are actually supported
		std::vector<const char*> enabledExtensions = deviceExtensions;
		{
			uint32_t extensionCount;
			CheckVk(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr), "vkEnumerateDeviceExtensionProperties(count)");
			std::vector<VkExtensionProperties> available(extensionCount);
			CheckVk(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, available.data()), "vkEnumerateDeviceExtensionProperties");

			for (const char* optExt : optionalDeviceExtensions) {
				for (const auto& ext : available) {
					if (strcmp(ext.extensionName, optExt) == 0) {
						enabledExtensions.push_back(optExt);
						break;
					}
				}
			}
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		CheckVk(vkCreateDevice(physicalDevice, &createInfo, GetAllocator(), &device), "vkCreateDevice");

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

		context.device = device;
		context.graphicsQueue = graphicsQueue;
		context.presentQueue = presentQueue;
		context.graphicsQueueFamily = indices.graphicsFamily.value();
	}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		CheckVk(vkCreateSwapchainKHR(device, &createInfo, GetAllocator(), &swapChain), "vkCreateSwapchainKHR");

		CheckVk(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr), "vkGetSwapchainImagesKHR(count)");
		swapChainImages.resize(imageCount);
		CheckVk(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()), "vkGetSwapchainImagesKHR");

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
		RefreshSwapchainContext();
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			VulkanImage::CreateImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i]);
		}
	}

	void createGlobalRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
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

		CheckVk(vkCreateRenderPass(device, &renderPassInfo, GetAllocator(), &renderPass), "vkCreateRenderPass");

		SetObjectName(reinterpret_cast<uint64_t>(renderPass), VK_OBJECT_TYPE_RENDER_PASS, "Global Render Pass");
	}

	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			CheckVk(vkCreateFramebuffer(device, &framebufferInfo, GetAllocator(), &swapChainFramebuffers[i]), "vkCreateFramebuffer");
		}
	}

public:
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

public:
	uint32_t presentImageIndex = 0;
	Renderer::RenderDelegate renderDelegate;
	bool frameReady = false;

	void waitUntilReady() {
		frameReady = false;
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		if (glfwWindowShouldClose(window)) {
			return;
		}

		glfwPollEvents();

		// Wait for the GPU to finish work on this frame
		CheckVk(vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX), "vkWaitForFences");

		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &presentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		frameReady = true;
		return;
	}

	void present() 
	{
		ZONE_SCOPED;

		if (!frameReady) {
			return;
		}

		Renderer::CommandBufferList commandBufferList;

		{
			ZONE_SCOPED_NAME("renderDelegate");
			renderDelegate(swapChainFramebuffers[presentImageIndex], swapChainExtent, commandBufferList);
		}

		// rotate the commandBufferList, hack so that we process the native render, then debug UI
		std::rotate(commandBufferList.begin(), commandBufferList.begin() + 1, commandBufferList.end());

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		{
			ZONE_SCOPED_NAME("vkQueueSubmit");

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = commandBufferList.size();
			submitInfo.pCommandBuffers = commandBufferList.data();
			submitInfo.signalSemaphoreCount = 1;
			// Use the renderFinished semaphore for the current swapchain image
			submitInfo.pSignalSemaphores = &renderFinishedSemaphores[presentImageIndex];

			CheckVk(vkResetFences(device, 1, &inFlightFences[currentFrame]), "vkResetFences");

			CheckVk(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "vkQueueSubmit");
		}

		{
			ZONE_SCOPED_NAME("vkQueuePresentKHR");

			// Present
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			// Wait on the renderFinished semaphore for the current swapchain image
			presentInfo.pWaitSemaphores = &renderFinishedSemaphores[presentImageIndex];
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &swapChain;
			presentInfo.pImageIndices = &presentImageIndex;

			const VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
				framebufferResized = false;
				recreateSwapChain();
			}
			else if (result != VK_SUCCESS) {
				throw std::runtime_error("failed to present swap chain image!");
			}
		}

		frameReady = false;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) { // && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		//for (const auto& availablePresentMode : availablePresentModes) {
		//	if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
		//		return availablePresentMode;
		//	}
		//}
		// Turn on Vsync
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		CheckVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

		uint32_t formatCount;
		CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR(count)");

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR");
		}

		uint32_t presentModeCount;
		CheckVk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr), "vkGetPhysicalDeviceSurfacePresentModesKHR(count)");

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			CheckVk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data()), "vkGetPhysicalDeviceSurfacePresentModesKHR");
		}

		return details;
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		const bool requiredFeaturesSupported = CollectMissingRequiredDeviceFeatures(device).empty();

		return indices.isComplete() && extensionsSupported && swapChainAdequate && requiredFeaturesSupported;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		return CollectMissingDeviceExtensions(device).empty();
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			CheckVk(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport), "vkGetPhysicalDeviceSurfaceSupportKHR");

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		if (checkShaderDebugExtensionSupport()) {
			extensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
		}

		for (auto& ext : instanceExtensions) {
			extensions.push_back(ext);
		}

		return extensions;
	}

	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		CheckVk(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "vkEnumerateInstanceLayerProperties(count)");

		std::vector<VkLayerProperties> availableLayers(layerCount);
		CheckVk(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()), "vkEnumerateInstanceLayerProperties");

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	bool checkShaderDebugExtensionSupport() {
		uint32_t extensionCount = 0;
		CheckVk(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr), "vkEnumerateInstanceExtensionProperties(count)");
		std::vector<VkExtensionProperties> extensions(extensionCount);
		CheckVk(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()), "vkEnumerateInstanceExtensionProperties");
		bool isExtensionSupported = false;
		for (const auto& ext : extensions) {
			if (strcmp(ext.extensionName, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME) == 0) {
				isExtensionSupported = true;
				break;
			}
		}

		return isExtensionSupported;
	}

	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationErrorCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT && messageType > VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			MY_LOG_CATEGORY("Validation", LogLevel::Info, "{}", pCallbackData->pMessage);
			FLUSH_LOG();
		}
		return VK_FALSE;
	}
};

ImageRendererApp app;

static VkAllocationCallbacks* gAllocator = nullptr;

const Renderer::VulkanContext& Renderer::GetVulkanContext()
{
	return app.GetVulkanContext();
}

const Renderer::SwapchainContext& Renderer::GetSwapchainContext()
{
	return app.GetSwapchainContext();
}

namespace Renderer
{
	// Global variable to track allocation count
	static std::array<std::vector<void*>, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE + 1> gAllocations;
	static std::mutex gAllocationMutex;
	static bool gAllocationTrackingEnabled = true;

	namespace
	{
		void RemoveTrackedAllocation_NoLock(void* pMemory)
		{
			if (!pMemory) {
				return;
			}

			for (auto& alloc : gAllocations) {
				auto it = std::find(alloc.begin(), alloc.end(), pMemory);
				if (it != alloc.end()) {
					alloc.erase(it);
					return;
				}
			}
		}
	}

	void* Alloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		// You can replace this with your custom memory allocator
		void* pNew = _aligned_malloc(size, alignment);
		if (pNew) {
			std::lock_guard<std::mutex> lock(gAllocationMutex);
			if (!gAllocationTrackingEnabled) {
				return pNew;
			}

			gAllocations[allocationScope].push_back(pNew);
		}
		return pNew;
	}

	void* ReAlloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		// You can replace this with your custom memory reallocator
		void* pNew = _aligned_realloc(pOriginal, size, alignment);

		std::lock_guard<std::mutex> lock(gAllocationMutex);
		if (gAllocationTrackingEnabled) {
			RemoveTrackedAllocation_NoLock(pOriginal);
			if (pNew) {
				gAllocations[allocationScope].push_back(pNew);
			}
		}

		return pNew;
	}

	void Free(void* pUserData, void* pMemory) {
		if (pMemory) {
			{
				std::lock_guard<std::mutex> lock(gAllocationMutex);
				if (gAllocationTrackingEnabled) {
					RemoveTrackedAllocation_NoLock(pMemory);
				}
			}

			_aligned_free(pMemory);
		}
	}

	void Setup()
	{
		gAllocator = new VkAllocationCallbacks();
		gAllocator->pUserData = nullptr;
		gAllocator->pfnAllocation = Alloc;
		gAllocator->pfnReallocation = ReAlloc;
		gAllocator->pfnFree = Free;
		app.SetAllocator(gAllocator);

		if (!gHeadless) {
			app.setup();
		}
	}

	void WaitUntilReady()
	{
		ZONE_SCOPED;

		if (!gHeadless) {
			app.waitUntilReady();
			Renderer::Native::ApplyPendingResizeIfNeeded();
			Renderer::Native::DrainPendingTextureUpdates();
		}
	}

	void Present()
	{
		if (!gHeadless) {
			app.present();
		}
	}

	RenderDelegate& GetRenderDelegate()
	{
		return app.renderDelegate;
	}

	void SetHeadless(bool bValue) {
		gHeadless = bValue;
	}
}

VkDevice GetDevice()
{
	return Renderer::GetVulkanContext().device;
}

VkFormat GetSwapchainImageFormat()
{
	return Renderer::GetSwapchainContext().imageFormat;
}

std::vector<VkImage>& GetSwapchainImages() {
	if (!Renderer::GetSwapchainContext().images) {
		return app.GetSwapchainImages();
	}

	return *Renderer::GetSwapchainContext().images;
}

VkPhysicalDevice GetPhysicalDevice()
{
	return Renderer::GetVulkanContext().physicalDevice;
}

const VkCommandBuffer& GetCurrentCommandBuffer()
{
	return app.GetCurrentCommandBuffer();
}

uint32_t GetCurrentFrame()
{
	return app.GetCurrentFrame();
}

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	return app.FindMemoryType(typeFilter, properties);
}

const VkBuffer& GetUniformBuffer(int index)
{
	return app.GetUniformBuffer(index);
}

const VkQueue& GetGraphicsQueue()
{
	return Renderer::GetVulkanContext().graphicsQueue;
}

const VkCommandPool& GetCommandPool()
{
	return Renderer::GetVulkanContext().commandPool;
}

const VkExtent2D& GetSwapchainExtent()
{
	return Renderer::GetSwapchainContext().extent;
}

void SetObjectName(const uint64_t objHandle, const VkObjectType objType, const char* format, ...)
{
#ifdef _DEBUG
	// Use a buffer and vsnprintf to handle printf-style arguments
	va_list args;
	va_start(args, format);

	// Determine the size of the buffer needed
	size_t size = std::vsnprintf(nullptr, 0, format, args) + 1;

	// Allocate a buffer
	std::unique_ptr<char[]> buffer(new char[size]);

	// Format the string into the buffer
	std::vsnprintf(buffer.get(), size, format, args);

	va_end(args);

	// Set the debug name using vkSetDebugUtilsObjectNameEXT
	VkDebugUtilsObjectNameInfoEXT objectNameInfo{};
	objectNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	objectNameInfo.objectType = objType;
	objectNameInfo.objectHandle = objHandle;
	objectNameInfo.pObjectName = buffer.get();

	static auto pvkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(GetInstance(), "vkSetDebugUtilsObjectNameEXT");
	if (pvkSetDebugUtilsObjectNameEXT == nullptr) {
		throw std::runtime_error("vkSetDebugUtilsObjectNameEXT is not available");
	}
	CheckVk(pvkSetDebugUtilsObjectNameEXT(GetDevice(), &objectNameInfo), "vkSetDebugUtilsObjectNameEXT");
#endif
}

void Renderer::Debug::BeginLabel(const VkCommandBuffer& cmdBuffer, const char* szLabel)
{
#ifdef _DEBUG
	VkDebugUtilsLabelEXT labelInfo{};
	labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	labelInfo.pLabelName = szLabel;
	static auto pvkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdBeginDebugUtilsLabelEXT");
	if (pvkCmdBeginDebugUtilsLabelEXT == nullptr) {
		throw std::runtime_error("vkCmdBeginDebugUtilsLabelEXT is not available");
	}
	pvkCmdBeginDebugUtilsLabelEXT(cmdBuffer, &labelInfo);
#endif
}

void Renderer::Debug::EndLabel(const VkCommandBuffer& cmdBuffer)
{
#ifdef _DEBUG
	static auto pvkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(GetInstance(), "vkCmdEndDebugUtilsLabelEXT");
	if (pvkCmdEndDebugUtilsLabelEXT == nullptr) {
		throw std::runtime_error("vkCmdEndDebugUtilsLabelEXT is not available");
	}
	pvkCmdEndDebugUtilsLabelEXT(cmdBuffer);
#endif
}


void Renderer::Debug::BeginLabel(const char* szLabel)
{
#ifdef _DEBUG
	//BeginLabel(GetCurrentCommandBuffer(), szLabel);
#endif
}

void Renderer::Debug::EndLabel()
{
#ifdef _DEBUG
	//EndLabel(GetCurrentCommandBuffer());
#endif
}

GLFWwindow* GetGLFWWindow()
{
	return app.GetGLFWWindow();
}

VkInstance GetInstance()
{
	return Renderer::GetVulkanContext().instance;
}

uint32_t GetGraphicsQueueFamily()
{
	return Renderer::GetVulkanContext().graphicsQueueFamily;
}

VkAllocationCallbacks* GetAllocator()
{
	return Renderer::GetVulkanContext().allocator;
}

void SetAllocationTrackingEnabled(bool enabled)
{
	std::lock_guard<std::mutex> lock(Renderer::gAllocationMutex);
	Renderer::gAllocationTrackingEnabled = enabled;

	if (!enabled) {
		for (auto& alloc : Renderer::gAllocations) {
			alloc.clear();
		}
	}
}

uint32_t GetAllocationCount(VkSystemAllocationScope scope)
{
	std::lock_guard<std::mutex> lock(Renderer::gAllocationMutex);
	if (!Renderer::gAllocationTrackingEnabled) {
		return 0;
	}

	return Renderer::gAllocations[scope].size();
}

VkCommandPool Renderer::CreateCommandPool(const char* name /*= nullptr*/)
{
	VkCommandPool commandPool = VK_NULL_HANDLE;

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = GetGraphicsQueueFamily();

	CheckVk(vkCreateCommandPool(GetDevice(), &poolInfo, GetAllocator(), &commandPool), "vkCreateCommandPool");

	if (name != nullptr) {
		SetObjectName(reinterpret_cast<uint64_t>(commandPool), VK_OBJECT_TYPE_COMMAND_POOL, name);
	}

	return commandPool;
}
