# Native shadow buffer diagnostics

Open **Native Shadow Buffers** in the debug menu. The panel previews the latest completed caster mask and blur output and shows their dimensions, blur samples/radius, and GS alpha.

**Shadow resolution** selects 1x (original), 2x, 4x, or 8x in each dimension. A 512x256 target becomes 1024x512, 2048x1024, or 4096x2048. Changes apply to the next shadow pass. The `Debug::ComboSetting` saves the selection in `settings.json` and applies it during debug-menu startup, even if the shadow panel stays closed. The renderer scales both axes together, preserving the projection and aspect ratio, and scales the blur radius to maintain its apparent softness. The actual resolution is reduced if necessary to fit the GPU's image/framebuffer limits. Previews and PNG exports use the actual target size. This affects projected shadow buffers; ordinary radial actor shadows retain their material textures.

- **Copy shadow debug info** copies the displayed settings and image availability to the clipboard. It also works when no shadow target exists.
- **Dump shadow buffers (PNG)** saves `caster-mask.png`, `blur-output.png`, and `settings.txt` in a timestamped `logs/shadows/shadow-<timestamp>/` directory under the game's working directory. The panel displays the absolute output path or an error. The button is disabled until a completed shadow target exists.

The PNGs preserve the buffers' native dimensions and R8 coverage values: black means no shadow coverage, white means full coverage, and gray means partial coverage. Readback runs after frame submission, waits for GPU completion, and restores the shader-read image layouts. A dump can briefly stall rendering. These are the latest retained buffers; they may come from an earlier frame if no shadow pass ran this frame.

To report missing shadows, share both PNGs and `settings.txt`. An empty caster mask points toward caster submission or rendering; a populated mask and empty blur output points toward the blur pass. Populated buffers with missing scene shadows suggest checking receiver submission, projection, depth, and alpha.

Expand **Traversal diagnostics** to see the latest CPU shadow traversal: render mask, hierarchy eligibility, object/strip culling and rejection counts, and caster strips linked/flushed. The clipboard report and `settings.txt` always include these details. These counts work with Draw Inspector capture disabled. They describe the latest traversal, may be retained, and do not prove that pixels were rendered. Several shadow scenes can run in one frame; these diagnostics describe the latest traversal rather than totals across scenes. The game records a fixed-size snapshot; report formatting runs in the renderer when the panel, clipboard, or dump requests it.

`Latest mask pass recorded draw calls` counts actual Vulkan indexed draw commands independently of Draw Inspector provenance and filters. Zero with linked/flushed casters points toward mesh lookup or native submission; nonzero with a black mask points toward projection, clipping, or fragment/depth state. The CPU traversal and recorded-pass snapshots are updated at different stages, so capture while the scene is stable.

Actor caster setup regression: `ed3DG3DHierarchySetStripShadowCastFlag` must pass selector `2` (cast) to `ed3DG3DHierarchyNodeSetAndClrStripFlag`; selector `4` sets receiver flags instead. `ShadowCasterFlags` tests cover normal LODs and the last-LOD actor shadow path, including preservation of existing receiver flags.

Last-LOD shadow lookup must also use the live `ed_3d_hierarchy` overload of `ed3DHierarcGetLOD`. Casting a live hierarchy to serialized `ed_g3d_hierarchy` reads the wrong offsets on Windows because live pointers are larger. This can skip the actor's shadow object entirely while leaving it counted as an eligible hierarchy. The runtime LOD test verifies the live layout's LOD addresses and bounds.

Receiver projection uses the game's PS2 STQ matrix. Its perspective divisor Q is stored in `.z` (`-camera Z`), while `.w` remains 1. The receiver shader divides `.xy` by `.z`; division by `.w` sends otherwise valid receiver UVs outside the mask. `ShadowProjection.StqDivisionMatchesNativeMaskTextureCoordinates` compares STQ UVs with the native caster viewport across aspect ratios, depths, and frustum edges.

The receiver uses the full native framebuffer scissor, matching the main scene's native viewport. `gShadowRenderViewport` holds PS2 shadow dimensions (for example 512x256), which cannot be used directly as pixel bounds on a resized native framebuffer. Doing so clips projected shadows in the lower/central scene before fragment shading.

Shadow direction comes from `CLightConfig::ComputeShadow`. Its weighted vector must accumulate light Z into Z and light W into W. Copying Y into Z and Z into W corrupts the direction and can activate `CCameraShadow`'s nonzero-W fallback. `ShadowLighting` tests preserve angled-light XYZ and the zero-W direction marker. The report includes the shadow camera position, target, and their difference for checking orientation in a live scene.

`CLightSun::Manage` must populate the selected manager light slot from `baseShape.direction` and `colorModel.color`, matching `CLightSun::DoLighting`. Reading their W components followed by components from the next fields corrupts direction and RGB intensity; zero weighted RGB leaves the shadow camera using the default vertical direction. The sun-light regression test compares both paths with distinct adjacent-field values and verifies an angled shadow direction survives validation and weighting.

The report retains the light manager shadow direction (including W), intensity, and active-light count, captured during the latest shadow traversal. Compare this direction with the camera displacement when investigating orientation. Hero lighting can replace the manager's global shadow direction.

Automatic light-slot assignment in `CLightManager::BuildActiveList` must modify only the current light's upper B nibble, preserving its other flags (PS2 function `0x00216180`, writes at `0x00216448` and `0x002165F8`). Copying A/B from the last sector light can disable the sun's per-position lighting and select slot -1, causing invalid matrix writes and zero weighted RGB. `ShadowLighting.AutomaticSlotsPreserveEachLightsFlagsAndFeedShadowDirection` covers both sun and non-sun assignment passes with an ambient light last, and checks that the sun reaches global and per-position lighting and produces an angled shadow direction.

The temporary per-object reports and full lighting-slot/sun dumps used during investigation have been removed. Aggregate rejection counts and regression tests retain coverage of the recovered failure paths without extra strip traversals or light inspection calls during rendering.

These buffers belong to the projected shadow path used by `CCameraShadow` for its target actor. `CCameraShadow::SetTarget` disables that actor's ordinary `CShadow` while it is the target, and restores it when the target changes. Ordinary actor shadows (`CShadow` and `CShadowShared`) use material-backed display-list quads in the main scene; their soft appearance comes from the material rather than this blur buffer. To inspect those draws, select the main pass rather than Shadow mask. Empty mask rows and black buffers do not diagnose missing ordinary actor shadows.

Validation: the x64-debug build and CTest pass. The opt-in `ShadowBufferDump.DISABLED_GpuReadbackPreservesPixelsAndLayouts` test renders known coverage at odd dimensions, switches resolution 1x -> 2x -> 1x, exports and decodes both PNGs, and checks every pixel to verify resizing, cached target reuse, and layout restoration. Run it alone from `bin/WIN`:

```powershell
.\KyaPortTest.exe --gtest_also_run_disabled_tests --gtest_filter=ShadowBufferDump.DISABLED_GpuReadbackPreservesPixelsAndLayouts
```
