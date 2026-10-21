# Draw Inspector

Open **Debug → Rendering Debug → Renderer → Draw Inspector**. Click **Capture next frame** and wait for the native render thread to finish that frame. The game continues running; the inspector retains a copied snapshot until the next capture or **Clear snapshot**.

Paste mesh and texture names into the separate filters, for example `SECT5.g3d_15_0_1` and `SECT5.g2d (m: 74 l:0)`. Filters use ImGui's substring/include/exclude syntax. There is no RenderDoc dependency or capture integration; identical names in different captures need not identify the same frame or instance.

The table follows native draw-recording order. Sort columns or filter by type, pass and owner. Repeated PS2 nodes, material layers, sprite batches and copied depth-only draws retain separate identities. Enable **Include linked/submitted sources without recorded draws** to find incomplete submission chains. Objects rejected before list linking are not captured.

Click **Normal extrusion (0x100)** to highlight matching strip rows in amber. **Highlight strips with flags** toggles the list highlight; **Flag mask (hex)** selects other flags. A strip matches when every mask bit is set in its submitted render flags (or linked node flags for unsubmitted sources); a zero mask matches nothing. The count covers currently visible rows. This list highlight does not change scene rendering or draw overrides.

Select a row to inspect:

Use **Open mesh viewer** to preview the selected strip and its requested texture layer, or **Open texture viewer** to select the bound material/layer/texture in the existing texture preview. These show current live assets, not frozen geometry or pixels. The mesh preview currently supports strips, not sprite batches. Missing/ambiguous texture assets and framebuffer captures cannot be opened through these shortcuts; unloaded source selections are disabled. Opened previews recheck the source lifetime each frame.

In **Mesh Previewer**, **Export OBJ** saves the currently queued preview meshes to
`logs/mesh-exports/preview_<timestamp>.obj` under the game's working directory
(normally `bin/WIN/logs/mesh-exports`). A matching `.txt` records mesh names,
vertex/triangle counts, local XYZ bounds, and animation-reference counts. Multiple
preview meshes become separate OBJ groups. Coordinates retain the game's raw
object-space axes and units; actor placement, animation, gizmo rotation, camera,
and projection are not baked in. Export is geometry-only, without textures.
The resulting absolute path or an error is shown beside the preview controls.

- **Source:** copied PS2 addresses, flags, bounds, counts, material bank, scene and available actor or scenery-cluster ownership. Right-click an address to copy it. Unknown ownership is not inferred from asset names.
- **Material/state:** actual texture binding, requested and bound layers, native indices, effective alpha/blend/depth/color-write state and raw GS registers.
- **Transforms:** model at linking, submitted model, and recorded view/projection matrices, displayed in storage order.
- **Stacks:** optional link/flush CPU stacks, resolved lazily using the existing symbol previewer. Enable **Collect call stacks** before capturing; unavailable symbols depend on the executable's debug information.

**Hide**, **Isolate** and **Highlight bounds** apply at the next frame boundary. The default match groups the source object, material, hierarchy and owner within its source scene, across layers/passes/viewports. Repeated submissions sharing that identity are grouped explicitly; this is not a persistent ID for an individual particle. **All uses of asset** matches the full mesh name within the current scene lifetime. The floating **Draw overrides** window reports native draw matches and provides **Clear overrides**, even when the inspector is closed.

Isolation affects traced strip/sprite instances only. Clears, framebuffer copies, postprocessing, independent native display-list draws and the ImGui overlay continue. Bounds highlighting is an approximate transformed source sphere, not a pixel outline or an animated silhouette, and uses the existing debug pass/depth behavior.

Game data is never edited by these controls. Source destruction, hierarchy/scene removal and level termination invalidate affected selections; a reused address cannot reactivate an old selection. Snapshot addresses are informational and are never dereferenced. Recapture if the source is unavailable or its LOD/source identity has changed.

Saved display-list commands construct two physical strip/sprite copies. The inspector registers that exact pair at construction and assigns it a shared lifetime token. Source availability and source overrides use this token across alternating buffers, including while **Draw up to this point** keeps per-frame tracing active. Availability does not depend on which copy was linked last frame. Different commands and unrelated sprites are not paired by texture, list-node address, or frame proximity. Resetting/rebinding/deleting the list or rebuilding either primitive retires the token, even with tracing off. Mesh preview resolves a recently linked physical instance and validates its lifetime before its cache lookup; it does not access the captured primitive address.

The Windows-only hooks preserve the PS2 display-list behavior. Ghidra decompilation and disassembly comparisons: `edDListBegin` at `0x002ca170` (the `0x100` branch recursively constructs the other buffer), `edDListSetBuffer` at `0x002cad90` (binds both buffers), `edDListPatchableReset` at `0x002d0dd0` (rewinds both buffers), and `edDListDelete` at `0x002cb160` (unlinks and frees the list).

Collection has a conservative 64 MiB per-frame budget; a truncated capture is marked and may omit draws or stages. Names are displayed up to 255 bytes; asset matching hashes the full name. Detailed collection is off outside requested captures, except while a visual override needs source tracking. Capture metadata crosses the render-thread queue through `Draw::Instance::traceSubmission`; completion is published only after the existing render-thread handoff.

Automated coverage lives in `port/Test/src/draw_trace_tests.cpp`. For an interactive smoke check, capture a scene containing scenery, actors and sprites; check layered/shadow rows, select an instance shared by several actors, exercise each override and reset, then unload the scene and confirm the frozen snapshot remains readable.
