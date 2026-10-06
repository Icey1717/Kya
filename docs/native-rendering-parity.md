# Native rendering parity checklist

Track the remaining Windows renderer features identified by the PS2 packet submission audit. Check off implementation and validation separately: a native hook alone does not establish visual parity.

## Overview

| Order | Feature | Current finding | Complete |
| --- | --- | --- | --- |
| 1 | GS depth-test modes | Native draws hardcode the depth comparison | [ ] |
| 2 | Animated vertex colors / alpha | Native mesh submission bypasses the animation packet path | [ ] |
| 3 | Normal extrusion | Extrusion amount is supplied but unused by the native shader | [ ] |
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

- [ ] Trace `TEST.ZTE` and `TEST.ZTST` into immutable native draw state.
- [ ] Implement the GS comparison modes with the native reversed-Z convention, including disabled depth testing.
- [ ] Preserve depth-write masking and alpha-fail behavior; audit full-alpha batch boundaries.
- [ ] Validate never, always, greater-or-equal, and greater with equal-depth and overlapping geometry.
- [ ] Audit destination-alpha testing (`DATE` / `DATM`) separately and record whether game packets use it; implement if exercised.

## 2. Animated vertex colors / alpha

Sources: strip flag `0x4` and the animation branch in `ed3DFlushStrip` in [ed3D.cpp](../src/b-witch/ed3D.cpp); fixed `pColorBuf` loading in `Strip::PreProcessVertices` in [Mesh.cpp](../port/KyaMesh/src/Mesh.cpp).

- [ ] Recover frame selection, interpolation, looping/end behavior, and color layout from the existing packet code and Ghidra.
- [ ] Supply animated RGBA to native draws without altering serialized strip layout.
- [ ] Support independent animation state for instances sharing a cached mesh.
- [ ] Validate frame boundaries, intermediate colors, animated alpha, looping, and multilayer materials against PS2 output.

## 3. Normal extrusion

Sources: strip flag `0x100`, `ANIM_ST_NORMAL_EXTRUDER_SPR`, and `ed3DPKTCopyMatrixPacket` in [ed3D.cpp](../src/b-witch/ed3D.cpp). The current [native vertex shader](../port/Windows/Renderer/Shaders/src/native.vert.glsl) only uses `animST.xy` for UV scrolling. The VU emulation extrusion branch is a guard.

- [ ] Recover the extrusion operation, coordinate space, flag gating, and position relative to skinning/model transforms.
- [ ] Consume the per-draw extrusion amount in the native vertex shader.
- [ ] Preserve UV scrolling when both features are enabled.
- [ ] Validate zero, positive, and negative extrusion on static and animated geometry, including shared-mesh instances.

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
