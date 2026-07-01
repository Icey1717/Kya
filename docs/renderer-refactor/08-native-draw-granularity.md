# Native Draw Granularity

## Finding

A RenderDoc capture of `AirPostBaseLine.rdc` shows that the native scene pass is
split into many small indexed draws:

- `846` total draw calls.
- `845` indexed draw calls.
- `821` draws in the main native render pass.
- `265` draws with `48` indices or fewer.
- `149` draws with `12` indices or fewer.
- Only `8` graphics pipelines and `3` vertex-buffer layouts are used.

The native draw event shape comes from `NativeRendererRecording.cpp`. For each
`Draw::Instance`, the capture currently shows:

- A debug label.
- Per-draw push constants.
- Possible pipeline bind for blend state.
- Dynamic depth/color write state.
- Descriptor set bind.
- One `vkCmdDrawIndexed`.

For GPU timing, that means a single native pass contains many small events whose
individual cost can be hard to read at counter precision. The useful follow-up is
to identify which markers and render states repeatedly produce nonzero GPU time.

## Why This Is A Problem

For GPU analysis, the draw granularity is useful because it exposes where native
scene work is split across many small GPU events. Very small draws often report
`0` duration at counter precision, so the useful signal is the distribution of
nonzero timed draws, repeated hotspots, and the total native pass duration rather
than the structural draw count alone.

The current structure also makes later renderer work harder:

- Sorting or grouping opportunities are hidden inside the instance loop.
- Draw state is applied at instance granularity even when adjacent instances are
  compatible.
- Debug captures become noisy because every tiny mesh piece is a separate draw.
- GPU attribution has to reason about many individual draw events instead of
  larger, easier-to-compare groups.


## GPU Timing Notes

`AirPostBaseLine_gpu_timing_summary.md` adds GPU counter data for the same capture. The native pass reports about `1233 us` of summed `EventGPUDuration` across `394` nonzero timed events, while the structural native draw count remains `821` draws. The difference is expected: many very small draws report `0` duration at counter precision.

The slowest native draws are not all the smallest draws, so the first renderer performance target should be GPU attribution: identify which assets, materials, textures, blend modes, and depth/write states repeatedly consume native pass time.

## Next GPU Timing Steps

1. Re-run the counter script across multiple captures of the same scene and compare native pass duration, slowest draw ranks, and small-draw bucket totals.
2. Extend the CSV with RenderDoc pipeline-state columns for timed draws: pipeline object, blend mode, depth test/write state, color write mask, framebuffer target, and bound texture names.
3. Group timed native draws by asset marker, material, texture, and index-count bucket to find repeatable GPU hotspots.
4. Use screenshots and RenderDoc captures for alpha-test, alpha-fail, Z-only, and FB-only scenes before changing state handling or draw merging.
5. Treat batching or multi-draw work as a GPU optimization only if a before/after capture shows lower native pass duration or lower repeated hotspot duration, not merely fewer structural draw calls.

## GPU Optimization Candidates

Use the timing data to choose renderer changes. The current capture points to
native draw attribution first, not a blind reduction of draw count.

Potential GPU-facing changes to evaluate:

1. Inspect the top native draw events and identify common material, texture,
   blend, depth, and framebuffer state.
2. Check whether slow native draws are using expensive blend/depth combinations
   or framebuffer paths that can be specialized without changing PS2 ordering.
3. Look for post/copy work inside the native pass, such as the `vkCmdCopyImage()`
   event, and verify whether it can be reduced or moved without changing output.
4. Consider adjacent draw merging only for consecutive instances with identical
   GPU-visible state:

```cpp
struct NativeBatchKey
{
    RenderPassKey renderPassKey;
    const Pipeline* pipeline;
    const VkDescriptorSet* descriptorSet;
    GIFReg::GSAlpha effectiveAlpha;
    uint32_t primCmd;
    bool alphaBlendEnabled;
    bool colorWriteEnabled;
    bool depthWriteEnabled;
    uint32_t colorWriteMask;
};
```

Batching or multi-draw work should be treated as successful only when repeated
RenderDoc captures show lower native pass GPU duration or lower duration for the
same hotspot group. Fewer draw calls alone is not enough for this GPU-timing pass.

## Validation

Use the same RenderDoc capture style to compare before and after:

- Total draw count.
- Count of draws with `<= 48` indices.
- GPU duration per pass.
- Slowest native draw events and their marker/material/texture/state attribution.
- Duration contribution from draws with `<= 48` indices and `<= 12` indices.
- Before/after duration for any changed asset, blend/depth path, post step, or draw-merging path.

Correctness should be checked in scenes with alpha-test, alpha-fail, Z-only, and
FB-only behavior because those states are PS2-order sensitive.
