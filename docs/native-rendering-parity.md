# Native rendering parity checklist

Track the remaining Windows renderer features identified by the PS2 packet submission audit. Check off implementation and validation separately: a native hook alone does not establish visual parity.

## Overview

| Order | Feature | Current finding | Complete |
| --- | --- | --- | --- |
| 1 | GS depth-test modes | Implemented; GPU and PS2 capture validation pending | [ ] |
| 2 | Animated vertex colors / alpha | Implemented; GPU and PS2 capture validation pending | [ ] |
| 3 | Normal extrusion | Implemented; GPU and PS2 capture validation pending | [ ] |
| 4 | Mipmaps / trilinear filtering | Material settings exist; texture images have one mip level | [ ] |
| 5 | Environment mapping | Setup exists; VU emulation body still needs recovery | [ ] |
| 6 | Fog | Scene settings exist; fog effect body still needs recovery | [ ] |
| 7 | PS2 AA effect | Scene dispatch exists; effect body still needs recovery | [ ] |

This order is a starting point. Runtime captures should establish which levels and materials exercise each feature before implementation decisions are finalized.

## Packet submission audit

Follow each feature from producer to consumer:

`scene/material/strip -> DMA/VIF/GIF packet -> Windows macro or explicit hook -> native draw state -> RenderThread command -> shader/pipeline`

- [ ] Identify real callers, flags, packet contents, and at least one reproducible scene for each feature.
- [ ] Check Windows macro side effects in [port.h](../port/include/port.h) before declaring a packet unsupported.
- [ ] Trace referenced option packets through `ed3DFlushOptionState`, not just inline packet builders.
- [ ] Check whether state comes from the current GS state, cached texture registers, or per-draw data, and when it is captured.
- [ ] Verify that queued draws retain their state across later changes, material switches, and render-pass boundaries.
- [ ] Add RenderThread commands only where ordered work or pass transitions require them; snapshot ordinary draw state in the draw command.

`VU1Emu::UpdateMemory` in [vu1_emu.cpp](../src/port/vu1_emu.cpp) handles VIF unpacking rather than providing a general GS packet replay path. Some `SCE_GS_SET_*` macros call renderer setters directly on Windows. In particular, full-alpha `ZBUF.ZMSK` has a bridge through `SetZbufWin` and must not be classified as wholly missing. Its batching/state capture still belongs in the audit.

## 1. GS depth-test modes

Sources: `SCE_GS_SET_TEST` / `SetTestWin` in [port.h](../port/include/port.h), `SetTest` in [VulkanPS2.cpp](../port/Windows/Renderer/Vulkan/src/VulkanPS2.cpp), and `SetColorDepthDynamicState` in [NativeRendererRecording.cpp](../port/Windows/Renderer/Vulkan/src/Native/NativeRendererRecording.cpp).

- [x] Trace `TEST.ZTE` and `TEST.ZTST` into immutable native draw state.
- [x] Implement the GS comparison modes with the native reversed-Z convention, including disabled depth testing.
- [x] Preserve depth-write masking and alpha-fail behavior; audit full-alpha batch boundaries.
- [ ] Validate never, always, greater-or-equal, and greater with equal-depth and overlapping geometry.
- [x] Audit destination-alpha testing (`DATE` / `DATM`) separately and record whether game packets use it; implement if exercised.

Implementation notes (2026-10-06):

- `RenderMesh` snapshots the current GS `TEST` and `ZBUF.ZMSK` in each native instance before material binding queues its batch. Later option changes, full-alpha init/term, and render-thread or preview replay cannot change that instance's depth state. Existing material alpha-test/alpha-fail handling is retained; this change does not establish full alpha-fail parity.
- With `ZTE=1`, `ZTST=0/1/2/3` maps to Vulkan `NEVER` / `ALWAYS` / `GREATER_OR_EQUAL` / `GREATER`. GS depth and native reversed-Z both increase toward the camera. `ZTE=0` disables testing and depth writes, consistent with [PCSX2's GS depth implementation](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/GS/Renderers/HW/GSRendererHW.cpp). `ZMSK` still suppresses writes, including the existing alpha-fail depth-only replay. Framebuffer materials use the captured comparison instead of a special hardcoded comparison.
- `ed3DFlushOptionState` calls `Renderer::ApplyOptionDepthState` in `port/src/rendering_helpers.cpp` to apply `TEST_1` and `ZBUF_1` from the referenced packed A+D option packet produced by `edDListPatchGifTag3D`. The reference now stores the packet pointer, rather than the address of its pointer variable. Multilayer native submission replays each layer's material register packet before capturing instance state.
- Source audit: the literal `SCE_GS_SET_TEST` calls in `ed3D.cpp`, viewport/video setup, and the found `edDListAlphaTestAndZTest` callers disable `DATE`. Material packets can carry authored `DATE`/`DATM`; runtime asset usage remains unverified. Native mesh submission asserts whenever `DATE=1`, for either `DATM` value. Destination-alpha testing is not implemented.
- Validation: the x64-debug incremental build and existing `KyaPortTest` CTest target pass. Equal-depth/overlap GPU checks, Vulkan validation, and representative PS2/native capture comparisons remain pending; the overview stays incomplete.

## 2. Animated vertex colors / alpha

Sources: strip flag `0x4` and the animation branch in `ed3DFlushStrip` in [ed3D.cpp](../src/b-witch/ed3D.cpp); fixed `pColorBuf` loading in `Strip::PreProcessVertices` in [Mesh.cpp](../port/KyaMesh/src/Mesh.cpp).

- [x] Recover frame selection, interpolation, looping/end behavior, and color layout from the existing packet code and Ghidra.
- [x] Supply animated RGBA to native draws without altering serialized strip layout.
- [x] Support independent animation state for instances sharing a cached mesh.
- [x] Validate frame boundaries, intermediate colors, animated alpha, looping, and multilayer materials against PS2 output.

Implementation notes (2026-10-06):

- Ghidra's `ed3DFlushStrip` (`002a5630`) confirms a four-word color header: base stride in quadwords, serialized animation-controller pointer, base count, and a fourth header word. Bases contain packed RGBA bytes, with each mesh section occupying 72 colors and the final section padded to a multiple of four. Frame addresses are `header + 16 + frame * stride * 16`; section addresses advance by 72 packed colors. The native loader skips the header and records original color indices through `KickVertex` compaction instead of assuming cached vertices remain contiguous in the source.
- `ed3DManageAnim` (`0029eca0`) already produces `field_0x10` from authored integer time/frame keys. Flags `0x1`, `0x2`, `0x4`, `0x8`, and `0x10` control looping, direction changes, forward playback, reverse playback, and integer-frame stepping. Native submission consumes that value without advancing the shared controller again. Packet selection interpolates adjacent bases with the fractional frame value; both playback directions use the same selection. The final base holds, including the controller's base-count endpoint. Native selection also clamps invalid/out-of-range frame values safely.
- `MeshLibrary::RenderNode` interpolates all four channels in GS byte units and supplies one packed RGBA value per compacted vertex. `RenderMesh` copies those values into each draw instance before queueing. The render thread applies them to the uploaded vertex copy, preserving cached geometry and colors, later submissions, alpha-fail replay, and preview replay. Material layers retain strip flags and their own source-index mapping. Serialized strip and GPU vertex layouts are unchanged; the existing native shaders consume the resulting RGBA through their lighting, global-alpha, material-alpha-test, and blend paths.
- Validation: `cmake --preset x64-debug`, the x64-debug build, four targeted `VertexColorAnimation` tests, and the existing CTest target pass. Tests cover frame boundaries/end holding, intermediate RGB/alpha, existing controller looping/non-looping/reverse playback, and padded section indexing through compaction and multilayer mesh creation. GPU validation, runtime scene/asset identification, shared-instance visual checks, lighting/global-alpha ordering, and PS2/native capture comparisons remain pending. CPU interpolation currently truncates each channel to a byte; exact VU interpolation/rounding still needs microcode or capture confirmation. The overview stays incomplete.
- Follow-up: a RenderDoc draw for `parchemin_medaillon.g3d_0_0_0_layer_1` exposed incorrect multilayer ST section indexing. Later layers used a 20-entry UV stride, confusing the PS2 0x50-byte VIF command-block stride with the packed ST payload. Ghidra's `ed3DStripPreparePacket` (`002a16f0`) advances each layer's payload by `gNbVertexDMA` packed ST entries, matching the section vertex count (72 with normals, 96 without). All layers now use the cumulative section vertex index. The regression test uses distinct UVs across a full 72-vertex section and a final short section; it fails with the old stride and passes with the corrected indexing. Vertex initialization also clears the unused Q padding seen as an indeterminate `inQ.y` in the CSV. A fresh capture is still needed to confirm the parchment's appearance.
- The follow-up vertex CSVs (2026-10-06, 14:50) contain 441 indexed vertices, no conflicting ST values at matching positions, and ST-to-UV conversion matching within CSV rounding. The user reports changed but still incorrect visuals. A separate texture-loading audit found that `G2D::Layer::ProcessTexture` decoded layer 0's material registers for every layer. It now selects the requested layer's command block before texture upload and caching, preserving that layer's TEX0, CLAMP, TEST, and ALPHA. A new `MaterialLayers` test verifies distinct texture/palette, sampler, alpha-test, and blend registers across two passes; the build and CTest pass. Runtime confirmation of this second fix is pending.

## 3. Normal extrusion

Sources: strip flag `0x100`, `ANIM_ST_NORMAL_EXTRUDER_SPR`, and `ed3DPKTCopyMatrixPacket` in [ed3D.cpp](../src/b-witch/ed3D.cpp); embedded VU microcode in [OneTimeCommands.h](../src/Rendering/OneTimeCommands.h); [native vertex shader](../port/Windows/Renderer/Shaders/src/native.vert.glsl). The simplified VU emulation extrusion branch remains a guard.

- [x] Recover the extrusion operation, coordinate space, flag gating, and position relative to skinning/model transforms.
- [x] Consume the per-draw extrusion amount in the native vertex shader.
- [x] Preserve UV scrolling when both features are enabled.
- [ ] Validate zero, positive, and negative extrusion on static and animated geometry, including shared-mesh instances.

Implementation notes (2026-10-06):

- The embedded VU program gates extrusion on `vi01 & 0x100` at instruction `0x3b1`. Instructions `0x3b6`–`0x3c9` iterate vertex XYZ at `vi15 + 3`, with normals starting at `vi15 + 1 + 0xd8`, and load the animation/extrusion vector from VU address `0x21`. `MULz.xyzw vf04, vf02, vf01` (`0x3bf`) followed by `ADD.xyz vf05, vf03, vf04` (`0x3c3`) and `SQ.xyz` (`0x3c5`) implements `position.xyz += normal.xyz * animST.z`. There is no normalization, inverse-transpose transform, clamping, or displacement of position W. Signed decoded normals retain their magnitude; positive and negative amounts move along and against them.
- Rigid bone conversion first stores the transformed XYZ and normal XYZ in those buffers (`0x138`–`0x139` for the first vertex). Extrusion therefore uses the bone matrix's upper 3x3 for normals, then offsets the skinned position in object space before object-to-culling/clipping/screen transforms. Static vertices use their decoded object-space normal. The native mesh, shadow-mask, and shadow-receiver vertex shaders follow this ordering. Shadow shaders also use the captured animation base offset instead of assuming `0x3dc`; receiver descriptors now bind the existing animation/extrusion buffer at binding 5.
- `ed3DPKTCopyMatrixPacket` selects the hierarchy setup's `field_0x10` float when supplied, otherwise `FLOAT_00448a04` (default `0.01f`), and stores it in vector Z. Existing `PushMatrixPacket` / `PushAnimST` append per-draw vector data, and `RenderMesh` snapshots its buffer index and render flags for each instance. Cached mesh vertices remain unchanged, so shared meshes, later state changes, queued draws, alpha-fail replay, and preview replay retain each submission's amount. UV scrolling independently consumes XY under flag `0x200`, including shadow-mask texture sampling.
- Validation: `cmake --preset x64-debug`, the x64-debug build, existing `KyaPortTest` CTest target, and `spirv-val` for all three modified shaders pass. These checks establish build and shader validity, not visual parity. Runtime level/asset identification, zero/positive/negative static and rigid-animation GPU checks, shared-mesh instance comparisons, Vulkan validation, and representative PS2/native captures remain pending. The overview stays incomplete.
- Runtime diagnostic: debug builds log the first emitted main-pass draw with render flag `0x100`, including mesh name, extrusion amount, buffer index, and index count. It reports flag activation even when the amount is zero; recording a draw does not prove its pixels survive clipping, depth, or alpha tests.

## 4. Mipmaps / trilinear filtering

Sources: `ed3DFlushMaterial`, `TEX1`, `MIPTBP1`, and `MIPTBP2` in [ed3D.cpp](../src/b-witch/ed3D.cpp); texture upload/samplers in [TextureCache.cpp](../port/Windows/Renderer/Vulkan/src/Texture/TextureCache.cpp); one-level image allocation in [VulkanImage.cpp](../port/Windows/Renderer/Vulkan/src/Objects/VulkanImage.cpp).

- [ ] Trace all stored mip levels through bitmap decoding, upload, image allocation, and image views.
- [ ] Preserve authored mip levels where available; decide how textures without a chain behave from PS2 evidence.
- [ ] Carry TEX1 filtering, maximum level, and LOD settings into sampler selection, accounting for differences from GS LOD calculation.
- [ ] Support mip enable/disable and nearest/linear mip selection without regressing point-versus-linear base filtering.
- [ ] Validate distant and oblique surfaces, mip transitions, alpha textures, and material switches against PS2 captures.

## 5. Environment mapping

Sources: `gbEnv`, node flag `0x40`, and `ed3DFlushStripMultiTexture` in [ed3D.cpp](../src/b-witch/ed3D.cpp); `_$Env_Mapping` in [vu1_emu.cpp](../src/port/vu1_emu.cpp) is guarded.

- [ ] Recover the original VU mapping operation, input vectors, coordinate space, and layer selection from Ghidra/microcode.
- [ ] Trace mapping parameters and flags into native per-draw data.
- [ ] Implement generated texture coordinates and the required multilayer composition.
- [ ] Validate static and skinned geometry under camera/object movement against PS2 output.

## 6. Fog

Sources: `CameraToFog_Matrix`, `FOGCOL`, `g3DFXFog`, and `ed3DFlushFogFX` in [ed3D.cpp](../src/b-witch/ed3D.cpp); fog transitions in [LargeObject.cpp](../src/b-witch/LargeObject.cpp). `ed3DFlushFogFX` currently contains a guard, and native mesh shaders have no fog calculation.

- [ ] Fix the disconnected fog gate when implementing the effect: Ghidra's `ed3DFlushFogFX` at `002aeb40` checks `g3DFXFog.field_0x0 & 1` (the structure at `00425000`), while the port checks a separate `UINT_00425000`. The scene code in `LargeObject.cpp` already updates the real structure through `ed3DGetFxFogProp`. The separate zero-initialized variable hides the missing effect body.
- [ ] Recover `ed3DFlushFogFX` and establish whether the active game path uses GS vertex fog, a depth-based post effect, or both.
- [ ] Recover distance/color parameters, flags, transitions, and scene/viewport scope.
- [ ] Implement the corresponding native shader or ordered effect command, including depth access if required.
- [ ] Validate near/far behavior, color transitions, transparency, child scenes, and viewport changes against PS2 output.

## 7. PS2 AA effect

Sources: `ed3DFlushAAEffect` and its scene dispatch in [ed3D.cpp](../src/b-witch/ed3D.cpp). The effect body is guarded and its original resolution/scene gating remains present.

- [ ] Establish whether AA is enabled at runtime: Ghidra's `ed3DFlushAAEffect` at `002b8640` checks `gAALiasON` (`00449418`, zero in the loaded image; `BYTE_00449418` in the port). Its only reported direct write is `FUN_002ab480`, which clears it when enabling blur. No direct enabling write was found; indirect writes remain unverified. The body also requires `BYTE_00448a70`, a viewport, width 512, and height 512 or 448; the caller requires scene flag `0x80` and excludes `0x800`.
- [ ] Recover the packet sequence, source/destination buffers, sampling offsets, blend rules, and execution order.
- [ ] Establish the intended behavior at native resolutions from the recovered operation.
- [ ] Implement the effect with explicit resource transitions and ordered RenderThread work where needed.
- [ ] Validate silhouettes, transparency, scene gating, resizing, and interaction with fog and framebuffer effects against PS2 output.

## Validation and completion

For each feature, record the tested level, camera position, enabling flags/materials, and PS2/native capture references below its checklist. Mark the overview complete only after implementation and validation are finished.

- [ ] Build with `cmake --preset x64-debug` and `cmake --build out/build/x64-debug`.
- [ ] Run `ctest --test-dir out/build/x64-debug` plus targeted GPU checks for changed rendering behavior.
- [ ] Check Vulkan validation output, queued state changes, resource lifetime, and resize behavior where affected.
- [ ] Compare representative PS2/native captures and document any deliberate approximation or remaining mismatch.

Flares already have native submission and rendering. Final PS2 visual parity comparison remains useful as a reference for this process. Existing native shadows, framebuffer distortion, UV scrolling, and multilayer rendering are not classified as missing by this checklist.

Sprite follow-up (2026-10-06): fountain sparks exposed two pre-existing native sprite issues in `port/Sprite/src/Sprite.cpp`. Width/height lookup now advances by 20 packed entries per full 18-quad batch, matching the PS2 packet's 0x50-byte stride, and honors the shared-size flag. `RenderNode` submits one combined mesh because `ProcessVertices` already expands all batches; previously it submitted that whole mesh once per batch. Sprite primitive state and vertex padding are initialized. The x64-debug build, three `SpriteBatches` regression tests (full/partial batches with poisoned WH padding, shared sizes, and a single batch), and CTest pass. Runtime confirmation of the fountain's intermittent jumping/glitching remains pending.
