# Unreal Editor workflow implementation checklist

See [implementation notes and verified UE 5.8 integration points](EDITOR_IMPLEMENTATION_NOTES.md) for source-level findings, proposed types, rendering and transaction design, sector discovery, export constraints, and reviewable implementation slices.

## Goal and review scope

This work ports the existing implementation from this repository's source into `unreal/KyaLevelEditor`. Use that source as the reference for layouts and behavior; Ghidra comparisons and recording PS2 function addresses are not required for this Unreal porting work.

Make a Kya level behave like an editable Unreal scene: import a level and its sectors, click the actual actors, inspect their complete supported properties, manipulate them with normal editor tools, save, reopen, and export the changes back to game banks.

This is a source review of the current working tree, including the in-progress actor/hierarchy handles, on 2026-11-11. It is not a live Unreal UI test. Existing local edits are the baseline. The phases below are ordered implementation work; unchecked items are proposed work, not claims of existing support.

## Findings that drive the plan

- `Source/KyaLevel/Private/KyaLevelPreview.cpp` builds shared procedural mesh sections. The displayed actor geometry does not currently provide individual actor ownership for ordinary selection. Merely enabling collision would not establish that ownership.
- `KyaLevelPreview.h` exposes a manually populated `SectorBanks` list and defaults `bSpawnActorHandles` and `bSpawnHierarchyHandles` to false. Capture scripts have some sector-import automation, but it is not a normal level-authoring workflow.
- `KyaActorHandle.h/.cpp` mirrors only placement fields, despite the much larger typed `FKyaActorPlacement` schema in `KyaLevelBank.h`. Handles synchronize location from the bank, defer Details writes until Tick, and rebuild the preview after writes. There is no actor `PostEditMove` implementation translating gizmo movement back into the placement; the older `KyaActorMarkerComponent` does have component-move writeback.
- Handle comments promise transient objects, but `RefreshActorHandles` does not explicitly set transient spawn flags. Audit actual serialization and undo behavior instead of relying on the comments. Editor-only and transient are separate lifetime requirements.
- `KyaActorTransform.h` implements class-specific rotation rules, including yaw-only classes. An arbitrary Unreal Euler conversion is insufficient for reliable round trips.
- `UKyaLevelBank` stores original bytes, typed data, a source filename and schema version. The factory is a `UFactory`, without a reimport interface in its declaration. The editor module currently provides a viewport console command rather than a broader asset/editor integration.
- Export is intentionally constrained to supported fixed-size edits. README documents rejection of count/size changes. Hierarchy visibility is preview state, not bank output; waypoint rotation and full path playback are also incomplete.

## 1. Establish ownership, identity and an edit contract — prerequisite

- [x] Make `UKyaLevelBank` typed records the authoritative editable game data. The Unreal map should store level placement, linked assets and editor presentation state; generated actor representations must not become a competing copy of game data.
- [x] Define actor identity as a bank asset plus record identity. Use the original array index while sizes remain fixed, with a generation/schema guard. Do not key by display name, mesh index, or sector alone. Specify how identities are invalidated or remapped on reimport.
- [ ] Introduce a shared editor edit service used by viewport transforms, Details, markers, scripting and bulk operations. It should resolve identity, validate, begin/participate in a transaction, modify the bank, mark it dirty, and notify dependent views.
- [x] Track which preview instances display each bank. An edit through one instance or the asset editor must refresh all others without applying the edit twice. Distinguish bank-local values from each preview's world transform.
- [x] Define generated representation lifetime explicitly: recreate from saved banks on map load, clean up on preview destruction/world teardown, avoid duplicate objects on reconstruction, and preserve selection when identity remains valid.
- [x] Add a field capability table: editable and exportable, editable presentation-only, inspect-only, or unsupported. Derive it from parser/exporter support, not just `EditAnywhere`. Include layout-controlling flags and fixed-length strings as well as arrays.
- [ ] Acceptance: editing through any supported entry point yields one authoritative value, all previews agree, and rebuilding or reopening does not introduce duplicates or dirty unchanged assets.

## 2. Import and discover a complete level

- [x] Add a normal Content Browser action to import/open a Kya level, and a drag-and-drop placement path that creates a configured level preview. Do not require a capture script or manually constructed preview to start editing.
- [x] Introduce a persisted level-set asset or equivalent descriptor linking the main bank, sector IDs, sector bank assets, source root and import revision. Keep these associations out of game bytes.
- [x] Discover sibling sector banks from the selected source level directory and available bank metadata. Inspect the existing capture scripts and `GetSectorId` logic first; factor reusable discovery into editor code rather than invoking capture scripts.
- [x] Resolve dependencies deterministically by level identity and sector ID. Reuse already imported matching assets through the Asset Registry; never match solely on a generic name such as `SECT1` across different levels.
- [x] Import missing sector assets into a predictable level-specific folder, with progress/cancel support and a summary of imported, reused, missing and ambiguous files. Leave a coherent partial result if canceled or a bank fails.
- [x] Distinguish discovered sectors from currently visible sectors. Default to an editable complete level where practical; retain explicit load/unload, visibility and manual association overrides for large levels and unusual directory layouts.
- [x] Add Refresh Sector Discovery and missing-source resolution actions. Refresh must preserve existing edits and manual overrides rather than silently reimporting banks.
- [x] Add standard import provenance (`UAssetImportData` or the appropriate supported equivalent) and a reimport handler. Support source-file relocation and Content Browser asset moves without breaking associations.
- [x] Before reimport, detect local edits and offer explicit discard/reapply/cancel behavior. Initially refuse unsafe merges; later compare original imported values, edited values and new source values per stable record/property.
- [ ] Acceptance: importing one level discovers its sectors, reopening restores associations, repeated discovery creates no duplicates, and two levels with identically named sector files remain isolated.

## 3. Select actual actors and use normal viewport tools

- [x] Promote actor representations to the default editing experience. Retain diagnostic marker toggles, but remove the requirement to enable stand-ins before selecting game actors.
- [x] Prototype rendering each actor's geometry on components owned by its editor representation. Keep world geometry batching separate. If per-actor components are too expensive, implement a tested hit-proxy/selection mapping that still selects a stable actor representation.
- [ ] Ensure clicking any strip of an actor selects that actor, highlights its full mesh, exposes its Details and gives useful bounds for Focus Selected. Preserve material batching within an actor and avoid invisible broad selection volumes masking neighbors.
- [x] Give meshless actors meaningful selectable icons or bounds with class/name tooltips. Support overlapping actor selection and temporarily hidden actors through the Outliner.
- [x] Implement transform writeback for translate, rotate and scale using the editor's interactive movement lifecycle. Update the visible actor during drag and commit the supported bank values at the appropriate transaction boundary.
- [x] Centralize forward and inverse position/rotation/scale conversion: game Y-up coordinates, meters versus centimeters, preview placement and class rotation semantics. Preserve original values when no actual edit occurs.
- [x] Restrict or explain axes that cannot be represented by a class, especially yaw-only actors. Define handling for negative/nonuniform scale and unsupported shear; never silently store a misleading transform.
- [x] Make numeric Transform edits and gizmo edits use the same conversion and validation. Keep raw game-space transform fields in an advanced section, with radians/degrees and units clearly labeled.
- [x] Implement snapping, world/local gizmos, multi-selection transforms and focus behavior. Moving the preview root should reposition the whole displayed level without rewriting every bank placement.
- [ ] Acceptance: click a visible mesh, move it with W, rotate with E where supported, scale with R where supported, save/reopen, export/reimport, and recover the intended placement. Include nonidentity preview transforms and yaw-only classes.

## 4. Show the selected actor's complete supported properties

- [x] Replace the small manually mirrored handle field list with a Details customization backed by the selected `FKyaActorPlacement` and shared edit service. Avoid maintaining a second hand-written copy of every class-specific field.
- [x] Organize Details into identity, transform, rendering/materials, sector, class configuration, behaviours, references and advanced/raw data. Show applicable class/behaviour fields using existing presence flags; hide irrelevant structures.
- [x] Make every currently supported exportable actor property reachable from selection, including nested fixed-size arrays, class tuning, models, switch/camera records and behaviour payloads. Audit against the schema and exporter and document any exceptions.
- [x] Handle nested property notifications and array-element edits correctly. Preserve indices and serialized sizes; remove add/remove/reorder controls where the exporter cannot support them.
- [x] Show class name plus numeric ID, named behaviours where recovered, unit tooltips, flag widgets for understood bits, and explicit unknown labels for unrecovered fields. Preserve unknown bits and do not invent semantics.
- [x] Provide actor, waypoint, path, sector, mesh and material reference pickers with searchable labels, current raw IDs and unresolved-reference warnings. Add Browse/Select Referenced Object actions and retain raw ID access for advanced work.
- [x] Validate ranges, reference scope, serialized widths, string byte lengths and layout-changing flags before committing. Explain the exact property and constraint inline instead of waiting for an export failure.
- [x] Support multi-edit of compatible shared fields, with mixed-value indicators. Prevent class-specific fields from being applied to incompatible selections and group each bulk edit into one undo action.
- [x] Add reset-to-imported-value and property-copy actions that preserve identity and layout. Define the imported baseline separately from Unreal class defaults.
- [ ] Acceptance: choose representative platform, Wolfen, hero, trigger and meshless actors and edit supported nested fields without navigating the bank's global actor array; verify corresponding exported values.

## 5. Reliable undo, saving and synchronization

- [x] Replace next-tick full-record writeback with transaction-safe edits to the actual changed properties. Verify whether a deferred callback falls outside the originating Details transaction; do not assume `Modify()` alone guarantees correct undo grouping.
- [x] Capture bank state before mutation. One drag, numeric commit or bulk operation should produce one meaningful undo step. Escape/cancel should restore both bank and preview values.
- [x] Implement undo/redo refresh for selected and unselected objects, asset-editor changes, nested edits and sector association changes. Avoid Tick-based stale mirrors overwriting newer values.
- [x] Save changed bank packages through normal Save/Save All workflows and expose unsaved bank changes clearly when the map itself is clean. Verify toolbar Save behavior; if it only saves the map, provide an obvious Save Kya Level action that saves its dirty dependencies.
- [x] Separate Save Unreal Assets from Export Game Banks. Saving must not silently overwrite source `.bnk` files. Show which bank packages changed and which have changes not yet exported.
- [x] Validate transient/editor-only flags, transactional flags, object references, map duplication, PIE and map reload. Generated editing actors should not accidentally appear in packaged runtime worlds.
- [ ] Acceptance: edit, undo, redo, save, close and reopen; both Details and rendered placement agree. Repeat with multiple previews sharing a bank and with rapid consecutive property edits.

## 6. Outliner and normal level organization

- [x] Group generated actors under a level root and sector folders, with stable human-readable names and class labels. Disambiguate duplicate names without changing serialized actor names.
- [x] Support Outliner selection, multi-selection, search, focus, temporary hide/isolate and selection locking. Keep editor visibility distinct from the game's visibility fields.
- [x] Define Outliner rename behavior explicitly: presentation label versus serialized actor name, including encoded-length limits. Do not silently truncate a game name.
- [x] Define Delete, Duplicate, Alt-drag, copy/paste and cut behavior. While the bank writer requires fixed counts, block unsupported structural commands with a clear explanation rather than allowing a generated actor to disappear and respawn mysteriously.
- [ ] Keep assigning an actor's sector explicit until reliable spatial sector membership exists. Add candidate-sector diagnostics for moved actors; do not infer membership from arbitrary mesh bounds or change it silently.
- [ ] Acceptance: familiar selection and organization commands work, and unsupported structural operations cannot leave map and bank state inconsistent.

## 7. Extend editing beyond actor placements

- [x] Give waypoints and path points stable identities, useful labels and the same selection/transaction/save behavior as actors. Expose the selected record's full supported properties.
- [x] Show all relevant paths on demand with selectable points and clear path ownership, rather than requiring a numeric preview index. Preserve native linear interpolation; do not silently convert to Unreal smooth spline semantics.
- [x] Add waypoint orientation visualization/editing only after verifying stored rotation conventions. Keep path timing, lead-in and loop visualization explicit about what is implemented.
- [x] Make hierarchy selection operate on rendered geometry at the existing practical hierarchy/strip-group granularity. Show bounds, owning sector, material information and source offsets.
- [x] Keep hierarchy hide/isolate actions clearly presentation-only. Enable hierarchy transforms only after implementing and validating serialized hierarchy writeback; a movable-looking handle must not imply export support.
- [x] Add selectable visualizers for supported lights, cameras, zones, trigger bounds and actor links. Use the same Details/edit service and distinguish approximate preview shapes from verified native semantics.
- [x] Put level environment, camera, fog and scene configuration on a discoverable level settings surface. Separate authoring properties from render comparison and capture diagnostics.
- [ ] Acceptance: users can find spatial records through the viewport/Outliner, understand their editable scope, and save/export supported changes without global-array hunting.

## 8. Make editing responsive and diagnosable

- [x] Replace full preview rebuilds for simple transform/property changes with targeted invalidation. Cache decoded meshes/textures by source revision and refresh only affected geometry, materials, lighting or visualizers.
- [x] Preserve selection and actor identity through refresh. A property edit should not collapse the Details panel, reset Outliner state or destroy the selected object.
- [x] Remove per-actor polling where change notifications suffice. Measure editor frame time, drag latency, rebuild time, memory and component count on a representative large level before choosing further batching work.
- [x] Support progress and cancellation for expensive import/decode/rebuild operations. Keep UObject/editor mutations on the appropriate thread and discard stale background work after asset changes.
- [x] Add actionable Message Log entries for missing sectors, stale schemas, missing references, unsupported geometry and export restrictions, with links to affected assets/actors.
- [x] Provide authoring visibility controls separate from game-like distance/sector culling, so an actor remains discoverable even when the native preview would hide it. Preserve comparison/capture modes.
- [x] Acceptance: transform dragging does not decode the whole level, selection survives refresh, and missing content can be diagnosed without searching raw logs.

## 9. Export validation and structural editing roadmap

- [x] Add Validate Kya Level and Export Kya Level actions for the linked bank set. Report errors by bank, actor identity and property path, with navigation back to Details.
- [x] Preflight all affected banks before writing, stage output, and publish only validated results with clear partial-failure handling. Export unchanged banks only when requested or required by the chosen output layout.
- [x] Retain original unknown bytes and existing fixed-size restrictions. Surface incompatible schema and unsupported edit failures before output files are replaced.
- [x] Extend existing round-trip tests to selected-actor edits: export, reimport and compare typed values; compare unchanged byte regions too. Clearly distinguish synthetic fixtures from real-bank coverage.
- [x] Validate representative exported changes in the running port. Check layout and behavior against the existing repository source when porting into `unreal/KyaLevelEditor`; no Ghidra comparison is required.
- [x] Treat actor creation/deletion/duplication, variable-length names, array resizing and class replacement as a later writer milestone. Implement relocation, allocation counts, reference remapping and unknown-record preservation before enabling these editor commands.
- [x] Acceptance: a successful export accurately describes its supported edits, passes reimport checks and preserves unrelated data; unsupported structural edits fail before any output replacement.

## 10. Delivery order and end-to-end gates

- [ ] **Milestone A — direct actor editing:** complete ownership/edit service, default mesh selection, transform conversion, transactions and saving for existing fixed-size placements. Gate: click → move → undo/redo → save/reopen → export/reimport.
- [ ] **Milestone B — complete level setup:** implement level-set discovery, sector associations, import provenance and safe reimport. Gate: import a main bank into an empty project and reach a usable scene without manually filling sector arrays.
- [ ] **Milestone C — full selected-actor Details:** cover the existing typed schema, nested edits, reference pickers and multi-edit. Gate: audit every export-supported actor field for accessibility and persistence.
- [ ] **Milestone D — broader authoring:** complete Outliner policies, spatial record editing, diagnostics and incremental updates. Gate: a representative editing session requires neither numeric actor indices nor capture-script setup.
- [ ] **Milestone E — structural authoring:** enable new/deleted/duplicated records only after the bank writer and reference-remapping tests support them.
- [x] Add focused automation alongside each milestone in `KyaLevelBankTests.cpp` and editor-focused tests where needed. Cover undo grouping, map reload, sector identity collisions, selected-object refresh, unsupported commands and interrupted export.
- [ ] Include real viewport click/gizmo tests or a documented manual acceptance pass. Existing scripted selection proves selection APIs work; it does not prove clicking visible actor geometry works.
- [x] Update README and COVERAGE after each milestone to state actual behavior, remaining restrictions and the standard user workflow. Keep capture/render-comparison instructions available as diagnostics.

## Implementation boundaries

- Put asset actions, reimport, Details customizations, transaction coordination and editor commands in `Source/KyaLevelEditor`. Keep reusable bank data and transform/render helpers in `Source/KyaLevel`; avoid leaking editor module dependencies into runtime code.
- Start from the existing actor, hierarchy, waypoint and path implementations rather than introducing unrelated parallel editing systems. Migrate their callers onto one consistent edit path.
- Confirm exact editor extension APIs against the installed Unreal version during implementation. This plan specifies behavior and integration points, not a verified version-specific API recipe.
- No implementation or build was performed for this planning task. The first implementation milestone should establish an editor test fixture and baseline the current working-tree behavior.
