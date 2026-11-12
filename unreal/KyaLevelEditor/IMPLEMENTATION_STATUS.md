# Unreal editor workflow — implementation status

Companion to [the workflow checklist](EDITOR_WORKFLOW_PLAN.md) and
[implementation notes](EDITOR_IMPLEMENTATION_NOTES.md). This file records what was
investigated, the design chosen for each slice, what is implemented, and how it was
verified. Reference for behaviour and layouts is this repository's own source
(`src/b-witch`, `port/`).

Last updated: 2026-11-12.

## 1. Working-tree findings (before this work)

- `AKyaLevelPreview::OnConstruction` was a single ~1,270-line builder that appended
  background, cluster, sprite and actor geometry to one procedural mesh. Actor
  identity was lost at the material-section boundary.
- `AKyaActorHandle` was a sphere marker with a hand-written mirror of 11 placement
  fields, written back wholesale from `Tick()` after a Details edit. There was no
  gizmo writeback, so moving a marker never changed the bank.
- `KyaActorTransform.h` had forward rotation rules only; no inverse existed, so no
  editor transform could be stored in a bank.
- `UKyaLevelBank` had no editor identity/generation and no change notification.
- The exporter and factory were monolithic; neither could be reused for preflight.
- The editor module only registered one console command.

## 2. What is implemented

### Actor-owned rendering and selection

- `AKyaLevelPreview` captures per-actor sections during its normal build
  (`ActorMeshSections`, keyed by bank actor index) with the resolved material,
  preview-local vertices (native hierarchy transform baked, actor translation,
  rotation and scale not baked) and relit vertex colours.
- `AKyaActorHandle` has a neutral `USceneComponent` root, a
  `UProceduralMeshComponent` that owns the actor's game mesh (one section per
  material) and the sphere marker as an icon child used only for meshless records.
  Position, rotation and scale live on the handle transform, so W/E/R feedback is
  immediate and the serialized values stay separate from display.
- `bSpawnActorHandles` and `bShowActorMeshesOnHandles` default on. When handles own
  actor geometry, actor triangles are not appended to the shared preview mesh, so
  there is exactly one draw path. `Show Actor Mesh`/`Show All Actor Meshes` keep the
  shared route and hide handle meshes and icons, so capture output is unchanged.
- Handles are spawned with `RF_Transient | RF_Transactional`, attached under the
  preview (Outliner grouping), reused by actor index on rebuild, and destroyed with
  the preview. Handles no longer poll the bank in `Tick`.

### Identity, edit service and transactions

- `KyaActorEditService.h/.cpp`: `FKyaActorRecordKey` (weak bank + actor index + bank
  edit generation), validation, `FKyaActorEditPatch` with only the changed fields,
  `Modify()` before mutation, `MarkPackageDirty()`, one change broadcast, and a weak
  registry of previews per bank.
- `UKyaLevelBank::EditGeneration` is bumped on import and reimport, so stale keys
  fail resolution instead of editing a different record.
- `EKyaEditNotify::Notify/Deferred/Silent` lets Details edits refresh immediately,
  interactive drags coalesce into one end-of-frame refresh, and bulk/scripted edits
  flush explicitly.
- `AKyaActorHandle::PostEditMove` commits once at the end of the interaction, before
  `Super` reruns construction scripts. It opens its own `FScopedTransaction` only
  when the editor does not already own one. Escape/cancel restores the transform
  before the commit point, so it does not dirty the bank.
- `AKyaActorHandle::PostEditChangeProperty` writes only the edited field, uses
  `MemberProperty`/`Property` so nested coordinate edits resolve to the containing
  field, and applies before `Super` (whose construction rerun resynchronizes fields).
- Undo/redo and asset-editor changes refresh previews through
  `FCoreUObjectDelegates::OnObjectPropertyChanged`/`OnObjectTransacted`; transaction
  notifications are coalesced through the edit service.

### Transform conversion

- `KyaActorTransform` now has `BuildRotationBasis`, `QuatFromGameBasis`,
  `GameBasisFromQuat`, `MakeUnrealTransform`, `InvertRotation` and
  `UnrealToGameTransform` alongside the existing forward rules.
- The axis permutation is applied by conjugation. Scale stays a separate component
  transform, so non-uniform scale round trips; negative/zero scale is rejected.
- Yaw-only classes expose yaw and write the native yaw-only representation (pitch
  and roll cleared, as `SV_BuildAngleWithOnlyY` does) only when the rotation
  actually changed. A translate or scale edit preserves the stored angles.
- Euler-matrix and default look-at paths are inverted analytically; the gimbal-lock
  branches at yaw ±90° are handled explicitly and verified.
- Every result is checked against the forward rules before it is written; an
  orientation the class cannot represent is rejected with a log message and the
  visible transform is restored from the bank.

### Export, validation and reimport

- `KyaBankExport.h/.cpp`: `KyaBuildBankExportBytes` (extracted from
  `UKyaLevelBankExporter::ExportBinary`, which is now an adapter),
  `KyaBankSupportsExport`, and `KyaWriteValidatedExportSet`, which validates and
  stages every bank first, rejects duplicate destinations, then publishes; a failed
  staging write cancels publishing and leaves existing files untouched.
- `UKyaLevelBank` carries `UAssetImportData` provenance; the factory records it.
- `FKyaLevelBankReimportHandler` implements `FReimportHandler`: parses into a
  temporary bank, only replaces the asset on success, asks before discarding dirty
  changes (refuses in unattended mode), updates provenance, bumps the edit
  generation and notifies views.
- `KyaLevelEditorActions` implements validate, export, sector discovery and save.
  Discovery parses `SECT<digits>.BNK` case-insensitively, rejects duplicate numeric
  IDs, matches existing assets by source provenance, imports missing banks into
  `/Game/KyaLevels/<Level>`, keeps existing associations and reports actor sector
  references that have no file.

### Waypoints and path points
- `FKyaLevelEditService` now also owns waypoint and path-point records:
  bank + index/path/point + generation keys, per-field patches for the full waypoint
  triple (position, stored rotation, parameters) and the path point plus its
  parallel rotation/extra-vector/delay/flags/extra-value/extra-word records.
  Validation covers index ranges, stale generations, finite values, non-negative
  delays and missing per-point arrays; edits `Modify()` before mutation, assign the
  validated candidate and share the same preview notification path as actors.
- `AKyaSpatialRecordHandle` is a selectable actor for one waypoint or path point
  with bank/index/generation identity, `Waypoint N` / `Path N Point M` labels, a full
  record in Details, gizmo and numeric writeback (including undo), and a Details
  customization that hides the fields that do not apply.
- `AKyaLevelPreview` gained `Spawn Waypoint Handles` and `Spawn Path Point Handles`.
  Path-point handles cover every path, so spatial records are discoverable without a
  numeric preview index. Waypoint arrows and handles are mutually exclusive.
- The old waypoint and path-point arrow markers now write through the edit service,
  so markers, handles, Details and scripts share one edit path.
- `Show All Paths` (`Kya.ShowAllPaths [1|0]`) draws a guide for every non-empty path
  and spawns selectable points without a numeric preview index. Guides use the game
  axis mapping with every point explicitly set to linear interpolation, each path
  gets a transient `<bank> path <n>` Outliner group that owns its points, and edits
  update both the bank and the guide. `Preview Path Index` remains the single-guide
  legacy mode.
- Stored waypoint rotation is the game's XYZ Euler triple in radians. The repository
  source assigns `CWayPoint::rotation` to `rotationEuler` and converts it with
  `SetVectorFromAngles`/`edF32Matrix4FromEulerSoft(..., "XYZ")` (ActorMicken,
  ActorTeleporter, JamGut/Hero copies), and the port's Euler basis reproduces that
  composition. Waypoint handles show a green forward arrow from that basis, rotate
  with the gizmo (inverse angles chosen nearest the stored triple), and preserve the
  angles on a pure translate. Path timing, lead-in and loop behaviour are explicit
  non-goals: path-point rotation is numeric/exported only and no playback is
  simulated.

### Editor integration
- Real `IModuleInterface` startup/shutdown; a `Kya Level` section in the Tools menu
  owned by the module, plus console commands `Kya.Validate`, `Kya.Export`,
  `Kya.DiscoverSectors`, `Kya.SaveLevel` and the existing `Kya.ViewportTonemapper`.
- `FKyaActorDetailsCustomization` (registered for `AKyaActorHandle`) exposes the
  complete editable `FKyaActorPlacement` via `FStructOnScope` and
  `AddExternalStructureProperty`. Each row applies a single top-level property patch
  through the edit service (nested array/sub-struct changes included), opens a
  transaction when the details setter does not own one, hides the old mirrored
  fields, and reloads the presentation copy on undo/redo, external edits and
  reimport (deferred, so the panel can rebuild safely).
- `GetKyaActorFieldCapability` is the field capability table: editable-exportable,
  editable-presentation-only, inspect-only and unsupported, with a reason per field.
  Export support is an allowlist derived from the writer's own `Actor.<field>`
  references (`KyaExportedActorFields.h`), so an editable field the exporter never
  encodes is inspect-only instead of claiming round-trip support. The table also
  encodes the writer's fixed-layout constraints: the actor-name slot must keep
  `Align(length + 1, 4) == original capacity`, fixed-size arrays keep their counts
  and element sizes, raw class payload bytes keep their size, and `VisibleAnywhere`
  record data plus the layout-controlling `bHas*` presence flags are inspect-only.
  `GetKyaNestedFieldCapability("TopLevel.Field")` applies the same rules to nested
  records (for example `Models.MeshName` and `FighterBlows.NameCapacity`). Details
  only offers editable rows and shows the capability description and reason as the
  row tooltip; patch construction rejects inspect-only/unsupported fields with that
  reason.
- `KyaActorCatalog` supplies the recovered presentation data: class names/notes  (derived from the factory's class branches), class-specific behaviour names,
  bank-local mesh/texture reference resolution, sector labels, and a class-gated
  scalar reference-field table. `ValidateKyaActorReferences` reports blocking
  out-of-range mesh/texture/sector/actor/waypoint/path references and a non-blocking
  default-behaviour warning; `Apply` rejects a patched reference that does not
  resolve before mutating the bank.
- The actor Details panel adds an Identity section (class label + numeric id, notes
  tooltip, Browse bank), a References section with searchable pickers  (`SKyaReferencePicker`) for sector, mesh, texture bank, default behaviour and the
  class-specific scalar actor/waypoint/path fields (raw ids always shown, Select
  button for actor references, inline unresolved-reference warnings), and a Flags
  section (`SKyaFlagBits`) with understood lighting bits plus raw values and an
  explicit "unknown bits preserved" note. Field tooltips document units.
- Multi-selection switches Details to a `Kya Multi-edit` panel: it reports the shared
  class, checks class and optional-record compatibility (class-specific fields are
  hidden for incompatible selections), and offers shared numeric rows for Transform
  and actor settings. Mixed values show `Mixed`; each row applies one bulk patch that
  resolves and validates every record first, keeps per-record untouched vector
  components, groups the edit into one undo step, and refreshes each bank once.
  `Reset all to imported` restores a whole selection in one transaction.
- `UKyaLevelBank` stores a typed imported baseline (actors, waypoints, paths) captured
  by the factory and refreshed on reimport; it is editor metadata and never exported.
  Details shows whether the actor still matches the baseline and offers `Reset to
  imported`, per-field reset, and `Copy` from another actor. Copy validates class,
  optional-record presence, array counts, nested record ids/presence flags and
  string-slot capacity before applying, so it cannot change the destination layout.
- Generated representations validate their lifetime: handles, spatial handles,
  grouping actors and hierarchy handles are `RF_Transient | RF_Transactional` and
  editor-only; a game world creates none; destroying a preview cleans them up; a
  new preview re-creates one per record; duplicated previews get independent
  representations; and duplicated/pasted/alt-dragged copies that no preview owns
  are removed on the next build or at the end of the structural command.
- `CanDeleteSelectedActor` blocks delete for every generated class with a
  class-specific explanation, and the module explains duplicate/cut/paste/Alt-drag
  behaviour in a dialog after removing the generated copies.
- Generated representations support editor-only presentation controls: temporary
  hide and isolate use `SetIsTemporarilyHiddenInEditor` (game visibility and bank
  data are untouched) and are reapplied after each build; selection locking is a
  preview-wide toggle plus a per-representation flag exposed through
  `IsSelectable`. Menu entries and `Kya.HideSelected`/`Kya.ShowSelected`/
  `Kya.IsolateSelected`/`Kya.ClearIsolation`/`Kya.LockGeneratedSelection` commands
  operate on the current selection.
- Renaming follows an explicit policy: the Outliner label is presentation and the
  serialized actor name is the edit. Generated `[class <id>]`/duplicate suffixes are
  stripped, then the name is validated (ASCII, 63-character limit, and the writer's
  `Align(length + 1, 4)` slot capacity) before it is written through the edit
  service. Rejected renames leave the bank unchanged, restore the derived label and
  explain the limit without truncating; spatial, hierarchy and group labels are
  generated and rename attempts are reverted.
- Hierarchy handles own the rendered strips of one static cluster hierarchy (one
  procedural section per material) so clicking visible geometry selects it; the
  shared mesh skips captured strips to avoid duplicate draws. Details reports the
  owning bank/sector, source byte offset, serialized origin, strip/triangle counts,
  world bounds and material names/hashes. Handles are location-locked and reset
  transform edits because hierarchy transforms are not serialized; visibility and
  selection stay presentation-only.
- Hierarchy hide/isolate uses the same editor-only state as other generated
  representations (survives rebuilds, never touches bank bytes). A rejected transform
  sets a persistent `TransformStatus` and logs the reason; Details shows
  `Transform reset: hierarchy transforms are not serialized` so the handle cannot
  imply export support.
- `AKyaSceneVisualizer` adds selectable light, main-camera, actor-link and
  checkpoint-trigger visualizers. Lights and the camera use verified serialized
  fields and new light/camera edit-service patches; links and triggers are read-only
  derived/approximate diagnostics. Details labels verified vs approximate semantics
  and read-only transform attempts are reset with a status. Zone bounds are
  explicitly unsupported (zone references have no serialized geometry) and the
  preview's `VisualizerLimitations` says so. Light edits are exported by patching the
  THGL archive in place; layout-changing type/shape edits are rejected, and untouched
  color channels keep their exact bytes.
- `UKyaLevelSettings` is the discoverable authoring surface for environment, main
  camera, fog and scene configuration, opened from the Kya Level menu or
  `Kya.LevelSettings` as a docked details tab. Applying writes the whole settings
  block through `ApplySceneSettings` (validation, one undo step, preview refresh);
  preview properties are split into `Kya|Authoring` and `Kya|Diagnostics|*` so
  comparison/capture options are separate from authoring.
- Targeted invalidation: actor transform/property edits synchronize existing handles
  (`HandleActorEdited`) without re-decoding geometry or rebuilding the preview;
  mesh/texture edits and scene settings still take the rebuild path. Waypoint/path
  edits refresh spatial handles, guides and visualizers only, and light/camera edits
  refresh scene visualizers only. A targeted edit's own transaction event is
  suppressed per preview, while undo/redo still rebuilds. `Silent` bulk edits keep
  deferred full-refresh semantics.
- Generated actors are grouped in the Outliner under one transient grouping actor  per sector, attached to the preview. Labels are `<serialized name> [class <id>]`,
  disambiguated with the record index for duplicate names; labels are derived from
  the whole bank so sector load/unload cannot rename them, and serialized names are
  never modified.
- Validate, export and sector discovery write `FMessageLog` entries (one per bank
  or sector, with errors, warnings and asset paths). Sector discovery runs behind
  a cancellable `FScopedSlowTask` and leaves a coherent partial result when
  cancelled; `Kya.Export` reports one outcome per file and opens the log on
  failure. Imports verify the destination asset's source provenance before it is
  associated, so an occupied path from a different level is reported instead of
  being adopted.
- `UKyaLevelPreviewFactory` (auto-registered `UActorFactory`) lets a bank asset be
  dragged into the viewport to place a configured `AKyaLevelPreview`.
- `UKyaLevelSet` persists a level's main bank, sector associations (id, bank,
  source filename, manual-override flag), source root and import revision as an
  editor data asset, so reopening a map restores the level without re-running
  discovery. The preview copies the descriptor into its editable arrays on build
  and uses its ID-to-bank mapping for sector lookup, so a manual association does
  not have to match the bank's filename. `Kya.CreateLevelSet` creates/updates the
  descriptor (never overwriting another bank's descriptor) and discovery refreshes
  it while keeping manual associations authoritative.

## 3. Verification

Build (Win64 Development, UE 5.8, test project junction):

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' `
  UnrealEditor Win64 Development -Project=G:\repos\Kya\out\UnrealBankTest\BankTest.uproject
# Result: Succeeded
```

Automation tests (all pass, `KYA_TEST_BANK` set to level 1):

```powershell
$env:KYA_TEST_BANK = 'G:\repos\Kya\assets\CDEURO\LEVEL\LEVEL_1\LEVEL.BNK'
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  G:\repos\Kya\out\UnrealBankTest\BankTest.uproject -unattended -nop4 -NullRHI -nosplash `
  '-ExecCmds=Automation RunTests Kya.Editor+Kya.Bank' `
  '-TestExit=Automation Test Queue Empty' -log
```

Results: `Kya.Bank.LevelOneRoundTrip`, `Kya.Bank.SmallBankRoundTrip`,
`Kya.Editor.ActorTransformConversion`, `Kya.Editor.ActorEditService`,
`Kya.Editor.ActorHandleTransform`, `Kya.Editor.ActorOwnedGeometry`,
`Kya.Editor.ExportValidation`, `Kya.Editor.LevelPreviewPlacement`,
`Kya.Editor.LevelSet`, `Kya.Editor.SpatialRecords`, `Kya.Editor.ActorCatalog`,
`Kya.Editor.MultiEditBaseline`, `Kya.Editor.GeneratedActorLifecycle`,
`Kya.Editor.StructuralCommands`, `Kya.Editor.OutlinerPolicy`,
`Kya.Editor.PathVisualization`, `Kya.Editor.WaypointOrientation`,
`Kya.Editor.HierarchyGeometry`, `Kya.Editor.SceneVisualizers`,
`Kya.Editor.LightExport` and `Kya.Editor.LevelSettings` all `Success`.

Coverage added by the new tests:

- inverse/forward basis round trips for default, yaw-only, Euler, combined and
  gimbal-lock angle sets; angle wrapping; rejection of pitched yaw-only
  transforms and of negative/zero scale;
- edit-service resolution, generation invalidation, validation-before-mutation,
  silent/deferred/immediate notification and per-preview refresh counts;
- handle translate/scale writeback, undo/redo, one refresh per multi-actor drag,
  yaw-only rejection, nested `Position.X` Details edits and untouched-field
  preservation;
- Outliner grouping: one transient grouping actor per sector, `<name> [class <id>]`
  labels, duplicate disambiguation derived from the whole bank (stable across
  rebuilds), grouping actors that cannot move the level, and reused group labels
  when the preview is reassigned to another bank;
- real level 1: 616 handles, 472 with owned meshes and 133,758 handle-owned
  triangles, while the shared mesh contains only 6,397 background/cluster vertices
  (no actor duplication); diagnostic mesh mode moves the actor geometry back to the
  shared mesh and leaves handle meshes empty;
- export: unsupported bank rejected before writing, sentinel file preserved, no
  staging file left behind, duplicate destinations rejected, and a real bank
  build → stage → publish → reimport round trip with the same actor count, byte
  size and first-record values;
- level set: factory creation, sector add/update/manual flags, descriptor-driven
  preview bank/sector/handle setup, descriptor sector IDs overriding filenames,
  discovery-refresh preservation without filename-derived duplicates, and
  following a changed main bank on rebuild;
- placement: the actor factory only accepts bank assets, spawns a preview that
  uses the dragged bank and builds its scene and handles;
- catalog and references: recovered class/behaviour labels and unknown labels,
  bank-local mesh/texture resolution, sector labels, class-gated scalar reference
  validation, numeric field get/set, Details-style Custom patches rejected when
  they introduce an unresolved reference, same-bucket name acceptance and
  smaller-bucket rejection, negative `LightingFlags` rejection, and int32 bit
  patterns (`0x80000000`) preserved;
- generated lifetime and structural commands: transient/transactional/editor-only
  flags, ownership checks, duplicate cleanup, independent previews, destroy/reload,
  no generated actors in a game world, blocked delete with per-class reasons,
  duplicate copies rejected from writing the bank, pasted-style copies swept
  without a preselection, and owned handles surviving cleanup.

## 4. Not implemented yet

These remain open in [the checklist](EDITOR_WORKFLOW_PLAN.md):

- Nested reference arrays and nested actor/zone reference pickers (top-level scalar
  references are covered; array elements still show raw ids).
- Path timing/lead-in/loop visualization; zone-bounds visualization is not possible
  because the schema stores zone references without geometry (stated in the preview
  limitations and visualizer semantics).
- `UKyaLevelSet` creation is a command/menu action rather than an asset-type
  Content Browser action; it is not auto-created on import.
- Progress/cancel UI for export; Message Log entries do not have navigation links.
- Bulk/scripted edits and reimports still rebuild the preview; per-record targeted
  invalidation is used for single actor/waypoint/path/light/camera edits.
- Serialized hierarchy transform writeback (deliberately disabled until the writer
  milestone defines it) and path playback.
- Interactive viewport acceptance pass (click a mesh, W/E/R, save/reopen,
  export/reimport) in a live editor session, and Milestone A–E gates.

