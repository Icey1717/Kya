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
- Generated actors are grouped in the Outliner under one transient grouping actor
  per sector, attached to the preview. Labels are `<serialized name> [class <id>]`,
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
`Kya.Editor.ExportValidation` all `Success`.

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
  size and first-record values.

## 4. Not implemented yet

These remain open in [the checklist](EDITOR_WORKFLOW_PLAN.md):

- Reference pickers (actor/waypoint/path/sector/mesh/material), named behaviour
  labels, flag widgets, unit tooltips, mixed-value multi-editing and
  reset-to-imported actions in Details.
- `UKyaLevelSet` asset and a normal Content Browser import/drag-drop flow; sector
  discovery is driven by the selected preview instead.
- Progress/cancel UI for import and export; Message Log integration with
  navigation.
- Targeted per-actor invalidation: a committed transform edit still rebuilds the
  preview once (the drag itself is live and does not rebuild).
- Waypoint/path/hierarchy migration onto the edit service, waypoint orientation
  editing, path playback.
- Outliner structural command policies (delete/duplicate/structural edits) and
  rename policy.
- Interactive viewport acceptance pass (click a mesh, W/E/R, save/reopen,
  export/reimport) in a live editor session.
