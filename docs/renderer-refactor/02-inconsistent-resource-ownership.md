# Inconsistent Resource Ownership

## Finding

Vulkan resource ownership is inconsistent across the renderer.

Examples:

- `VulkanImage` destroys its image, view, sampler, and memory in its destructor.
- `OwnedImage` requires a manual `Destroy()` call.
- `Renderer::Pipeline` requires a manual `Destroy()` call.
- `DynamicUniformBuffer`, `StorageBuffer`, and `UniformBuffer` allocate Vulkan
  buffers and memory but do not consistently own/destroy those Vulkan resources.
- Helpers like `CreateBuffer` and `VulkanImage::CreateImage` return raw handles
  through out parameters.

## Why This Is A Problem

The renderer already throws exceptions for many hard Vulkan setup failures.
That is a reasonable policy, but it only works well if partial construction is
safe. Manual cleanup and raw out-parameter resources make exception paths risky.

The current ownership style also makes lifetime ordering hard to see:

- Some resources clean themselves up.
- Some need explicit teardown.
- Some are embedded in namespace-global state.
- Some are raw handles passed through several layers.

That makes leaks and use-after-destroy bugs more likely during refactors,
swapchain recreation, and feature additions.

## Suggested Fix

Standardize on small, non-copyable, movable RAII wrappers for Vulkan resources.

Suggested wrappers:

- `VulkanBuffer`: owns `VkBuffer` and `VkDeviceMemory`.
- `VulkanImageResource`: owns `VkImage`, `VkDeviceMemory`, and optional
  `VkImageView`.
- `VulkanSampler`: owns `VkSampler`.
- `VulkanPipeline`: owns `VkPipeline` and `VkPipelineLayout`.
- `VulkanDescriptorPool`: owns `VkDescriptorPool`.
- `VulkanCommandPool`: owns `VkCommandPool`.
- `VulkanFramebuffer`: owns `VkFramebuffer`.
- `VulkanRenderPass`: owns `VkRenderPass`.

Each wrapper should:

- Store the `VkDevice` and allocator pointer needed for destruction.
- Be non-copyable.
- Be movable.
- Destroy in the destructor.
- Provide `Reset()` for explicit early teardown.
- Allow access to the raw handle through `Get()` or an implicit lightweight
  accessor if local style prefers that.

Example direction:

```cpp
class VulkanBuffer {
public:
    VulkanBuffer() = default;
    VulkanBuffer(VulkanContext& context, VkDeviceSize size,
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

    VkBuffer Get() const { return buffer; }
    VkDeviceMemory Memory() const { return memory; }
    void Reset();

private:
    VkDevice device = VK_NULL_HANDLE;
    VkAllocationCallbacks* allocator = nullptr;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};
```

Migration path:

1. Start with buffers and images because they are the most common.
2. Convert `OwnedImage` into a real RAII type or replace it with
   `VulkanImageResource`.
3. Convert `Pipeline::Destroy()` into destructor-backed ownership.
4. Remove raw `Vk*`/memory pairs from framebuffer and native renderer state.
5. Keep explicit `Reset()` where swapchain recreation needs deterministic
   teardown before rebuilding.

