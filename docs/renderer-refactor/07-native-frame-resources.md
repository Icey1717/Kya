# Native Frame Resources

## Finding

The native renderer still keeps framebuffer and command-buffer resources as
fields on the broad `NativeRendererState` object:

- `frameBuffer`
- `frameBufferSampler`
- `commandPool`
- `commandBuffers`

These resources are used across setup, recording, and submission code. They are
now accessed through `GetNativeRendererState()`, which is better than exported
globals, but each module still depends on the full renderer state object.

## Why This Is A Problem

The root state object remains a broad dependency surface. Code that only needs a
framebuffer or command buffer still has access to unrelated draw queues,
pipeline state, preview state, dynamic buffers, and render-thread state.

That makes it harder to:

- See which frame resources are required by each render phase.
- Keep recording code independent from setup and submission details.
- Move toward explicit object lifetimes.
- Split more renderer modules without reintroducing global-style access.
- Test or reason about command-buffer recording in isolation.

## Suggested Fix

Extract a `NativeFrameResources` owner and store it on `NativeRendererState`.

Suggested shape:

```cpp
struct NativeFrameResources
{
    FrameBufferBase frameBuffer;
    VkSampler frameBufferSampler = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    CommandBufferVector commandBuffers;
};

struct NativeRendererState
{
    NativeFrameResources frameResources;
    NativePreviewRenderer preview;
    // Remaining state follows.
};
```

Then update callers from:

```cpp
GetNativeRendererState().frameBuffer
GetNativeRendererState().frameBufferSampler
GetNativeRendererState().commandPool
GetNativeRendererState().commandBuffers
```

to:

```cpp
GetNativeRendererState().frameResources.frameBuffer
GetNativeRendererState().frameResources.frameBufferSampler
GetNativeRendererState().frameResources.commandPool
GetNativeRendererState().frameResources.commandBuffers
```

After that mechanical move, pass the narrower object into helpers that only need
frame resources:

```cpp
void RecordBeginRenderPass(
    NativeFrameResources& frameResources,
    NativePipelineCache& pipelineCache,
    const RenderPassKey& key);
```

Do not jump straight to a full `NativeRenderer` class in this step. Keep the
change focused on frame-resource ownership and compile boundaries. A later step
can extract `NativePipelineCache`, then `NativeGpuBuffers`, then draw submission
state.
