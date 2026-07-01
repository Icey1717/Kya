# Native Redundant State Binds

## Finding

The same RenderDoc capture shows low state variety but high per-draw state
application:

- `8` graphics pipelines.
- `6` blend-state variants.
- `5` depth-state variants.
- `3` vertex-buffer layouts.
- `846` draw calls.

In `NativeRendererRecording.cpp`, each recorded instance sets or binds several
pieces of Vulkan state:

- `vkCmdPushConstants`
- `vkCmdBindPipeline` when the local blend key changes
- `vkCmdSetDepthWriteEnable`
- `vkCmdSetColorWriteEnableEXT`
- `vkCmdSetColorWriteMaskEXT`
- `vkCmdBindDescriptorSets`
- `vkCmdDrawIndexed`

The recorder already avoids some redundant pipeline binds inside a single
`DrawCommand` by caching `primState`, `alphaBlendState`, and
`effectiveAlphaState`. It does not yet track all command-buffer state across
draw commands, and it reapplies dynamic state and descriptor sets for every
instance.

## Why This Is A Problem

The capture suggests the renderer is not creating excessive Vulkan pipeline
variants. Instead, it is repeatedly applying a small set of state combinations.

That has several costs:

- Extra command-buffer bytes.
- Extra driver work while recording and later processing the command buffer.
- More RenderDoc noise when inspecting real state changes.
- More places where future dynamic-state or descriptor changes can accidentally
  become order-dependent.

This is separate from draw batching. Even if the draw count stays the same, the
recorder should avoid emitting commands that do not change current Vulkan state.

## Suggested Fix

Add a command-recording state cache local to `DrawCommandRecorder`.

Suggested shape:

```cpp
struct NativeBoundState
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    const VkDescriptorSet* descriptorSet = nullptr;
    VkBool32 depthWriteEnable = VK_FALSE;
    VkBool32 colorWriteEnable = VK_TRUE;
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    void Reset()
    {
        pipeline = VK_NULL_HANDLE;
        descriptorSet = nullptr;
        depthWriteEnable = VK_FALSE;
        colorWriteEnable = VK_TRUE;
        colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    }
};
```

Use it in the recorder to guard state commands:

```cpp
if (bound.pipeline != nextPipeline) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, nextPipeline);
    bound.pipeline = nextPipeline;
}

if (bound.descriptorSet != drawCommand.pDescriptorSets) {
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.layout,
        0,
        1,
        drawCommand.pDescriptorSets,
        0,
        nullptr);
    bound.descriptorSet = drawCommand.pDescriptorSets;
}
```

Do the same for depth write, color write enable, and color write mask. Reset the
cache when:

- A command buffer begins.
- A render pass begins or ends.
- `Debug::Reset(cmd)` or any helper may alter externally tracked dynamic state.
- A path records commands outside `DrawCommandRecorder`.

Keep push constants per instance at first. They contain per-instance transform,
alpha-test, render flags, and lighting indices, so they are real data changes.
They can be revisited after batching exists.

## Validation

Capture before and after with RenderDoc and compare:

- Event count around the main native pass.
- Pipeline bind count.
- Descriptor bind count.
- Dynamic depth/color state command count.
- GPU duration column and performance counters.
- CPU command-recording time in Tracy or existing renderer timers.

The expected behavior change is no visual difference and fewer redundant Vulkan
commands around the same draws.
