# Native Renderer Inl Files

## Finding

`NativeRenderer.cpp` includes several `.inl` files:

- `NativeRendererRecording.inl`
- `NativeRendererSetup.inl`
- `NativeRendererSubmission.inl`

These files are not primarily template or inline implementation files. They are
being used to split one large translation unit into phase-based chunks while
sharing the same namespace-global state.

## Why This Is A Problem

The `.inl` split hides the real dependency problem. The setup, recording, and
submission code all depend on shared globals declared in `NativeRenderer.cpp`.
Moving the files to `.cpp` directly is difficult because the state has no
explicit owner.

The result is a codebase that looks physically separated but is logically one
large file. That makes it harder to:

- See which data each phase owns.
- Reuse or test recording code.
- Limit includes.
- Compile and reason about independent modules.
- Move toward cleaner object lifetimes.

## Suggested Fix

Do not start by mechanically renaming `.inl` to `.cpp`. First create explicit
state objects that can be passed between normal implementation files.

Suggested ownership shape:

```cpp
class NativeRenderer {
public:
    void Setup(VulkanContext& context);
    void ResizeFrameBuffer(int width, int height);
    void Render(const VkFramebuffer& framebuffer,
        const VkExtent2D& extent,
        Renderer::CommandBufferList& commandBufferList);

private:
    NativeFrameResources frameResources;
    NativePipelineCache pipelineCache;
    NativeDrawQueue drawQueue;
    NativeRecorder recorder;
    NativePreviewRenderer preview;
};
```

Then the files can become normal C++ files:

- `NativeRenderer.cpp`: public facade and high-level frame flow.
- `NativeRendererSetup.cpp`: resource and pipeline setup.
- `NativeRendererRecording.cpp`: command buffer recording.
- `NativeRendererSubmission.cpp`: public draw submission API.
- `NativePreviewRenderer.cpp`: actor preview render path.

Migration path:

1. Move namespace globals into a `NativeRendererState` struct without changing
   behavior.
2. Pass `NativeRendererState&` into helper functions inside the `.inl` files.
3. Convert one `.inl` at a time to `.cpp` once it no longer relies on textual
   inclusion.
4. Split `NativeRendererState` into smaller owned subobjects after compilation
   boundaries are clean.

