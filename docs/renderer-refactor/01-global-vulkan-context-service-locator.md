# Global Vulkan Context Service Locator

## Finding

`VulkanRenderer.cpp` owns the core Vulkan application state through the private
`ImageRendererApp` singleton, then exposes that state through global free
functions declared in `VulkanRenderer.h`.

Examples:

- `GetDevice()`
- `GetPhysicalDevice()`
- `GetSwapchainImageFormat()`
- `GetCurrentFrame()`
- `GetGraphicsQueue()`
- `GetCommandPool()`
- `GetAllocator()`

Most renderer subsystems call these globals directly. Object wrappers, texture
code, pipeline creation, postprocessing, native rendering, and command helpers
therefore depend on `VulkanRenderer.h` instead of depending on an explicit
renderer/device context.

## Why This Is A Problem

The current shape makes `VulkanRenderer.cpp` both the bootstrapper and a service
locator. That has several costs:

- Resource creation can only work after hidden global setup has happened.
- Subsystems cannot be tested or reasoned about with a fake or alternate context.
- Lifetime ordering is implicit.
- Renderer objects look independent but actually depend on the global app.
- Swapchain-specific state and device-wide state are exposed through the same
  access pattern.

This is the source of the "Vulkan app context" smell.

## Suggested Fix

Introduce an explicit context object and migrate dependencies toward it
incrementally.

Suggested first shape:

```cpp
struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily = 0;
    VkAllocationCallbacks* allocator = nullptr;
};

struct SwapchainContext {
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = {};
    std::vector<VkImage> images;
};
```

Then split the current `ImageRendererApp` responsibilities into clearer owners:

- `VulkanDevice` or `VulkanContext`: instance, physical device, logical device,
  queues, allocator, debug messenger.
- `Swapchain`: surface, swapchain images/views/framebuffers, resize/recreate.
- `FrameScheduler`: acquire, submit, present, frame fences, semaphores.
- `RendererFrontend`: public `Renderer::Setup`, `WaitUntilReady`, `Present`,
  and render delegate integration.

Do not try to remove all global functions in one pass. A practical migration is:

1. Add `VulkanContext` and make `ImageRendererApp` own one.
2. Add `Renderer::GetVulkanContext()` as a temporary bridge.
3. Convert low-level object constructors/functions to accept `VulkanContext&`.
4. Stop including `VulkanRenderer.h` from object wrappers once they no longer
   need global accessors.
5. Delete individual globals as their call sites disappear.

## Implementation Status

Initial compatibility bridge added:

- `Renderer::VulkanContext` now stores instance, physical device, logical
  device, graphics/present queues, graphics queue family, command pool, and
  allocator.
- `Renderer::SwapchainContext` now stores swapchain handle, image format,
  extent, and swapchain image list.
- `Renderer::GetVulkanContext()` and `Renderer::GetSwapchainContext()` expose
  those contexts.
- Existing free functions such as `GetDevice()`, `GetPhysicalDevice()`,
  `GetGraphicsQueue()`, `GetCommandPool()`, `GetSwapchainImageFormat()`, and
  `GetAllocator()` now route through the context bridge.

Remaining work:

1. Convert low-level resource helpers to accept `const VulkanContext&` instead
   of calling the legacy global getters.
2. Move swapchain ownership out of `ImageRendererApp` into a dedicated
   `Swapchain` object.
3. Move acquire/submit/present state into a frame scheduler object.
4. Remove the compatibility free functions once call sites have explicit
   dependencies.
