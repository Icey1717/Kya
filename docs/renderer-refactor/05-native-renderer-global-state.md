# Native Renderer Global State

## Finding

`NativeRenderer` keeps a large amount of mutable renderer state in namespace
globals.

Examples include:

- Render pass cache and active render pass state.
- Native framebuffer and framebuffer sampler.
- Command pool and per-frame command buffers.
- Dynamic model, animation, lighting, and fade buffers.
- Current draw state.
- Cached view/projection/per-draw data.
- Pending resize state.
- Actor preview framebuffer, command buffers, draw copies, and camera state.
- Render-thread coordination state.

## Why This Is A Problem

This makes the native renderer a hidden singleton. The public API looks like
free functions, but it is really mutating a large implicit state machine.

The main risks are:

- Lifetime ordering is hard to enforce.
- Resize/recreate behavior has to know about many globals.
- Preview rendering and main rendering share too much machinery implicitly.
- The render thread and main thread coordinate through shared state rather than
  clear ownership.
- Future render paths will likely add more globals instead of creating local
  ownership boundaries.

This is distinct from the `.inl` issue. The `.inl` files are a symptom; the
global state is the deeper design issue.

## Suggested Fix

Introduce explicit native renderer subobjects with clear ownership.

Suggested split:

- `NativeFrameResources`: command buffers, per-frame buffers, frame-local upload
  state.
- `NativeFramebuffer`: color/depth image resources, framebuffer, sampler, resize.
- `NativePipelineCache`: render pass variants, base pipeline, blend pipeline
  variants, debug line pipeline.
- `NativeDrawQueue`: current draw, saved draws, batching rules, texture binding.
- `NativeRecorder`: command buffer recording and render pass transitions.
- `NativePreviewRenderer`: actor preview framebuffer, camera, command buffers,
  and preview draw replay.
- `NativeRenderThread`: thread, queue, synchronization, timing.

Then keep the old free functions as a compatibility facade:

```cpp
namespace Renderer::Native {
    static std::unique_ptr<NativeRenderer> gRenderer;

    void Setup() {
        gRenderer = std::make_unique<NativeRenderer>();
        gRenderer->Setup(Renderer::GetVulkanContext());
    }

    void RenderMesh(SimpleMesh* mesh, uint32_t flags) {
        gRenderer->RenderMesh(mesh, flags);
    }
}
```

Migration path:

1. Create `NativeRendererState` and move globals into it.
2. Replace direct global access with `state.` access.
3. Create methods on `NativeRenderer` that own that state.
4. Extract preview into its own class first; it is relatively self-contained.
5. Extract pipeline cache and framebuffer ownership next.
6. Keep public API stable until the game-facing renderer calls can be migrated.

