# Unreal editor workflow — implementation status

Companion to [the workflow checklist](EDITOR_WORKFLOW_PLAN.md) and
[implementation notes](EDITOR_IMPLEMENTATION_NOTES.md). This file records what was
investigated, the design chosen for each slice, what is implemented, and how it was
verified. Reference for behaviour and layouts is this repository's own source
(`src/b-witch`, `port/`).

Last updated: 2026-11-13.

Checklist status: all 80 items are implemented and covered by the
`Kya.Editor`/`Kya.Bank` automation (100 tests), including structural actor
create/duplicate/delete with stream relocation and typed reference remapping.
Actor/behaviour family coverage beyond LEVEL_1 is tracked separately in
[ACTOR_BEHAVIOUR_AUDIT.md](ACTOR_BEHAVIOUR_AUDIT.md): all 16 level banks were
scanned. EventGenerator (0x3B) has a typed header plus raw manager tail; every other class id with a payload is now fully typed, including the minigame family (0x45/0x47/0x55 in NATIV, 0x44/0x46 source-only via fixtures). The remaining audit work is verification, not migration. Those remain the next migration targets; the 37 LEVEL_1/Hedgehog class ids are module-backed.

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
- Round-trip coverage distinguishes fixtures explicitly: the small bank is a
  synthetic `FKyaSmallBankBuilder` fixture, while the real-bank tests cover
  unedited byte identity, all-class edits with per-field offset checks, and a
  selected-actor edit-service patch whose changed bytes are confined to that
  actor's serialized record.
- Running-port validation: `Kya.Editor.RunningPortExportValidation` exports a
  selected-actor edit from a transient validation copy (mesh/texture/collision
  references cleared so no scene resources are needed) and runs the port's
  `RunningPortExport.ValidatesEditedBank` test. It checks the native `edBank`
  container and `CActorManager`/`CActor::Create` layout, and constructs the actor
  through `CActorFactory`/`CActor::Create` to inspect the resulting position and
  visibility/culling values. The harness requires matching gtest XML evidence
  (one test, no failures/errors/skips) so a stale binary cannot pass.
- Structural authoring (`Kya.Editor.StructuralWriter`): `KyaBankStructuralWriter`
  locates and rebuilds the scene actor stream, appends copied template chunks for
  create/duplicate with unique names and contiguous indices, shifts later entries
  and the entry/type tables, recomputes actor class allocation counts and preserves
  trailing scene bytes. Delete removes a chunk, shifts later actor indices, remaps
  typed actor and scene-level references recursively (respecting class presence
  flags) and rejects a delete that would leave a reference to the removed actor.
  `Kya.CreateActor`/`Kya.DuplicateActor`/`Kya.DeleteActor` and the Kya Level menu
  expose it through a transaction and preview refresh. Unknown raw payload bytes are
  preserved unchanged; class replacement, array resizing and variable-length renames
  remain blocked. `Kya.Editor.StructuralWriter` covers duplicate/append,
  export/reimport, middle-reference remapping and last-record deletion; the gate
  exercises the commands and the preflight rejection for an unapplied count change.
- Export acceptance (`Kya.Editor.ExportAcceptance`): the exporter reports its
  supported edit scope (record counts, changed actors vs the imported baseline and
  the in-place/structural-rejection policy) through `FKyaBankExportOutcome`, the
  result messages and bank-linked Message Log entries; an untouched export is
  byte-identical, a selected-actor edit reimports exact values with a bounded diff
  and an unchanged neighbour, and a structural actor-count change fails before
  staging while leaving unrelated and existing destination files untouched.
- Milestone A gate (`Kya.Editor.MilestoneAGate`): a real transactional bank is
  selected through a generated handle (headless click equivalent), moved in a
  transaction with the bank write verified, undone/redone, saved to a canonical
  package, reloaded from disk with `UPackageTools::ReloadPackages`, and the
  reloaded asset is exported and reimported with the moved position intact.
- Milestone B gate (`Kya.Editor.MilestoneBGate`): a uniquely named freshly imported
  main bank in an empty setup creates a level-set descriptor with no sectors, sector
  discovery performs fresh imports (`Imported > 0`, no reuse) into an isolated folder,
  associations/provenance match, the scene publishes positive vertex/triangle counts,
  repeated discovery is idempotent, and a safe sector reimport of that fresh asset
  keeps the same association and descriptor rows while bumping the edit generation.
- Milestone C gate (`Kya.Editor.MilestoneCGate`): every export-supported property is
  audited on the real bank - a distinct value is committed through the Details patch
  path for a record whose presence flags make the field meaningful, the pre-edit
  baseline must actually change, and each edited field must survive export/reimport
  (variant-specific leaves are retried so a nested edit reaches the serialized
  record). Fields with no distinct value in the bank are reported explicitly; any
  applicable field that is rejected or commits unchanged fails. The panel's own row
  filter (`KyaGetActorDetailsProperties`) must expose every editable field, and the
  shared picker commit (`KyaSetActorReferencePick`) is exercised for both a valid and
  an out-of-range choice. Nested model edits, the reference picker and a two-actor
  bulk edit are exported and reimported with all values compared.
- Milestone D gate (`Kya.Editor.MilestoneDGate`): a representative session on the
  real bank finds and selects an actor by its Outliner label (no numeric actor index,
  no capture camera or `PreviewActorIndex`), exercises hide/isolate/lock and the
  delete-refusal reason, locates waypoint/path handles by their labels, drags a
  waypoint and a path point and the actor, and verifies the bank writes, refreshed
  spatial labels and that `BuildCount`/geometry decodes never increase. Unresolved
  references produce blocking diagnostics whose message names the actor and property.
  Spatial handle labels now include the game-space position, so waypoints and path
  points are identifiable in the Outliner without knowing their indices.
- Viewport picking and highlight: `AKyaLevelPreview::HitTestActorMesh` ray-casts
  every rendered material strip (nearest hit, no proxy volumes) and skips locked,
  hidden, isolated-out and invisible-section geometry;
  `KyaHitTestRenderedActorAcrossPreviews`/`KyaSelectRenderedActorAtCursor` resolve
  the nearest hit across previews from the level viewport cursor
  (`Kya.SelectAtCursor`, **Select Actor Under Cursor**). `AKyaActorHandle` highlights
  all sections via custom depth (batching preserved) and reports whole-mesh bounds for
  Focus Selected; the preview follows editor selection changes so the highlight stays
  in sync. `Kya.Editor.ViewportClickSelection` covers per-strip picking, misses,
  nearest-strip and cross-preview occlusion, highlight/bounds, hidden and locked
  rejection; README documents the manual mouse/gizmo acceptance pass.
- Shared edit path (`Kya.Editor.SharedEditPath`): every supported entry point
  (viewport transform, Details patch, legacy waypoint/path markers, scripted
  `ApplyToBank`, bulk edits) writes the authoritative record, and all previews of the
  bank agree. The service opens one transaction for direct/scripted calls when none is
  active (actor, bulk, waypoint, path point, light, camera, settings) and treats
  unchanged candidates as no-ops, so identical patches and synchronized re-applies do
  not dirty the bank. A two-preview rebuild keeps one handle per record, does not
  duplicate spatial handles and leaves an unchanged bank clean.
- Sector discovery/reopen isolation (`Kya.Editor.SectorIsolation`): two levels with
  identically named sector files but separate source roots import fresh, keep their
  associations inside their own root with no shared bank objects, stay idempotent on
  repeated discovery, and restore the descriptor rows plus the preview's sector-id
  mapping after a forced package save/reload.
- Gizmo acceptance (`Kya.Editor.GizmoAcceptance`): on a nonidentity preview root the
  gate clicks a rendered strip, translates with W, rotates a full-basis class and a
  yaw-only class (rejecting an unrepresentable pitch), scales with R, then saves,
  reopens and exports/reimports, checking the bank values and the recovered
  placement at every stage.
- Representative-actor edits (`Kya.Editor.RepresentativeActorEdits`): a platform,
  Wolfen, hero, checkpoint-manager trigger and meshless actor are found from their
  Outliner labels and each gets a supported nested edit (carried-platform entry,
  Wolfen fight configuration, hero blow damage, checkpoint flags, meshless
  visibility distance); every edited property and the placement survive
  export/reimport.
- Save/close/reopen consistency (`Kya.Editor.SaveReopenConsistency`): two previews
  sharing a bank stay synchronized through a Details edit, undo/redo and 25 rapid
  consecutive property patches; saving, closing and reloading the package restores
  the Details values and the rendered placement in two fresh previews.
- Explicit sector assignment (`Kya.Editor.SectorAssignment`): moving an actor never
  rewrites `SectorId`; the Details panel's live **Sector candidates** row reports the
  authored sector plus loaded sector backgrounds whose approximate bounds contain
  the position, explicitly as a diagnostic that is never applied. The gate moves an
  actor into loaded SECT1 bounds and confirms the id survives the move and
  export/reimport.
- Selection and organization (`Kya.Editor.SelectionOrganisation`): Select All/Invert/
  None, multi-selection, hide/show, isolate/clear and lock/unlock work over the
  generated representations, while duplicate/paste cleanup and delete refusal keep
  the world handle count, bank records, bank actor count and `EditGeneration`
  unchanged.
- Spatial-record acceptance (`Kya.Editor.SpatialRecordAcceptance`): waypoints and path
  points are found by Outliner label, their `Editable Scope` text describes the
  supported fields and missing per-point records, and Details edits (with rotation
  rejection when the record is absent) survive undo/redo, save/close/reopen and
  export/reimport.
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
- Actor family metadata is organized as one module per serialized class under
  `Source/KyaLevel/Private/Actors/`, with shared types in `KyaActorFamily.h` and
  a registry that the catalog, pickers and tests read. `CActorHedgehog` (class
  0x22) is the first family fully extracted end to end: typed placement and
  behaviour fields, a source-derived codec, writer support, Path reference
  registration, Details applicability and a fixture round trip. LEVEL_1 has no
  Hedgehog record, so the fixture is the round-trip evidence. Projectile (class
  0x1c) followed: its class, behaviour 3/4/5/9/10/11/12/13 and inventory codecs
  live in `Actors/KyaProjectile.h/.cpp`, with a fixture covering all eight
  payload shapes and real-bank reconciliation of the ids present in LEVEL_1.
  Moving platform (class 0x0d) followed: carried actors, zones, no-friction
  zones, cameras, properties, sound streams and behaviours 2-10 now live in
  `Actors/KyaMovingPlatform.h/.cpp`; the id 10 fixture covers the only shape
  absent from LEVEL_1, while the real bank covers ids 2-9. Switch (class 0x11)
  followed: target-switch class data and behaviours 2-8 now live in
  `Actors/KyaSwitch.h/.cpp`; ids 4/7/8 are fixture-covered, ids 2/3/5/6 come
  from the real bank. Wind (class 0x12) then moved its tuning/switch class codec
  to `Actors/KyaWind.h/.cpp`; its only behaviour (id 2) has an empty payload and
  is covered by both the fixture and the real bank. Trap (class 0x2c) then moved
  its class, trap-stream and behaviour 2/3 codecs to `Actors/KyaTrap.h/.cpp`.
  Weapon (class 0x20) then moved its class and pistol/sniper/RPG codecs to
  `Actors/KyaWeapon.h/.cpp`. Araignos (class 0x21) then moved its class/add-on
  codec to `Actors/KyaAraignos.h/.cpp`; its empty id 2 behaviour is covered by
  both the fixture and the real bank. Bonus (class 0x0e) then moved its class and
  turn/path/flock/add-on codecs to `Actors/KyaBonus.h/.cpp`. Teleporter (class
  0x24) then moved its class/switch/waypoint/condition codec to
  `Actors/KyaTeleporter.h/.cpp`; its empty id 2 behaviour is covered by both the
  fixture and the real bank. Amortos (class 0x29) then moved its class
  actor-stream/add-on/switch codec to `Actors/KyaAmortos.h/.cpp`; the module also
  corrects the serialized order of the first three tuning floats. DCA (class
  0x1b) then moved its class, fire-shot stream and controlled/trajectory codecs
  to `Actors/KyaDca.h/.cpp`; behaviour 4 remains raw because its source Create is
  an implementation guard. Fruit (class 0x16) then moved its class and
  squashed/inventory/bonus codecs to `Actors/KyaFruit.h/.cpp`. Nativ (class 0x0a)
  then moved its class and take-and-put codec to `Actors/KyaNativ.h/.cpp`. The
  shared Fighter prefix and switch-script helpers were extracted to
  `Actors/KyaFighter.h` and `Actors/KyaSwitchScript.h`, and Wolfen (class 0x09)
  moved its class/fight-parameter and twelve behaviour codecs to
  `Actors/KyaWolfen.h/.cpp`. Commander (class 0x13) then moved its class and
  behaviour 2 codec to `Actors/KyaCommander.h/.cpp`. Kim / hero private (class
  0x06) then moved its class/camera codec to `Actors/KyaKim.h/.cpp`; the shared
  actor-camera reader lives in `Actors/KyaActorCamera.h`. Boomy (class 0x05) then
  moved its class/camera/type codec to `Actors/KyaBoomy.h/.cpp`; the shared
  camera writer `KyaWriteActorCameraValues` is now used by both Kim and Boomy.
  Aton (class 0x38) then moved its class/path/add-on codec to
  `Actors/KyaAton.h/.cpp`, and its life maximum is stored as int64 with u32
  validation. Rope (class 0x0f) then moved its class/switch codec to
  `Actors/KyaRope.h/.cpp`. Stiller (class 0x4C) then moved its
  class/switch/add-on codec to `Actors/KyaStiller.h/.cpp`. Ambre (class 0x14)
  then moved its class/switch codec to `Actors/KyaAmbre.h/.cpp`. Bonus fountain
  (class 0x52) then moved its add-on codec to `Actors/KyaFountain.h/.cpp`.
  Checkpoint manager (class 0x1E) then moved its record/pair codec to
  `Actors/KyaCheckpoints.h/.cpp`. Bridge (class 0x17) then moved its class and
  basic/wind-aware behaviour codecs to `Actors/KyaBridge.h/.cpp`. Fog manager
  (class 0x3D) then moved its hierarchy/fog-zone codec to
  `Actors/KyaFog.h/.cpp`. Shoot (class 0x3E) then moved its class/add-on and
  fire/wave behaviour codecs to `Actors/KyaShoot.h/.cpp`; its life maximum is
  stored as int64 with u32 validation. Money (class 0x32) then moved its class
  and flock/add-on behaviour codecs to `Actors/KyaMoney.h/.cpp`. Box (class 0x35)
  then moved its class/switch/add-on codec to `Actors/KyaBox.h/.cpp`, and its
  life maximum is stored as int64 with u32 validation. FX (class 0x2E) then moved
  its class and single/path/unknown behaviour codecs to
  `Actors/KyaFx.h/.cpp`. Clusteriser (class 0x18) then moved its behaviour 2
  zone-list codec to `Actors/KyaClusteriser.h/.cpp`; the module also fixes
  `Flags` sign-extension by reading the serialized u32 into int64. Companion
  (class 0x19) then moved its class and alert behaviour codecs to
  `Actors/KyaCompanion.h/.cpp`; the shared conditioned/scenario readers now live
  in `Actors/KyaScenarioCondition.h`, and its life maximum is stored as int64
  with u32 validation. Gravity aware (class 0x10) then moved its class and
  fall/bell behaviour codecs to `Actors/KyaGravity.h/.cpp`, and its life maximum
  is stored as int64 with u32 validation. Rune (class 0x51) then moved its class
  codec to `Actors/KyaRune.h/.cpp`. Micken (class 0x07) then moved its class
  and eat behaviour codecs to `Actors/KyaMicken.h/.cpp`, and its life maximum is
  stored as int64 with u32 validation. Basic box (class 0x39) then moved its
  class codec to `Actors/KyaBasicBox.h/.cpp`, and its life maximum is stored as
  int64 with u32 validation. Nativ command (class 0x3C) then moved its
  class/add-on codec to `Actors/KyaNativCmd.h/.cpp`; the shared catch-by-trap and
  hero-unknown payload readers moved to `Actors/KyaCatchByTrap.h/.cpp` and
  `Actors/KyaHeroUnknown.h/.cpp`, and the Hedgehog class reader now dispatches
  through `KyaReadHedgehogActor`. Every actor family is module-backed.
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
- Authoring visibility is separate from game-like culling: `bAuthoringVisibility`
  (default on) keeps actors from unloaded sectors selectable
  (`bAuthoringShowUnloadedSectors`, with `Authoring Only Handle Count`) and
  `bAuthoringShowCulledGeometry` draws distance/LOD-culled cluster scenery. A locked
  comparison/capture camera (`SetPreviewCamera`, which refreshes membership
  immediately) or disabling the mode restores native culling; `ClearPreviewCamera`
  (`Return to Editor Camera`, `Kya.ClearPreviewCamera`) resumes authoring discovery.
  Game worlds never use the override. Enabling authoring visibility also clears the
  capture-camera lock, and the menu/`Kya.AuthoringVisibility [1|0]` toggle it.
- Imports, exports and preview rebuilds show cancellable progress in interactive
  sessions; cancellation discards an import, writes nothing on export and stops a
  rebuild at the next phase boundary (`RebuildStatus`) while restoring the previous
  mesh/presentation state (geometry, materials, flares, ranges, hierarchy metadata).
  Actor mesh strips are decoded on the thread pool into a data-only cache keyed by
  bank revision; requests use a bank-global id so a newer request from any preview
  supersedes older data, stale results after a reimport/newer request are discarded,
  older workers cancel cooperatively, and UObject/material/mesh creation stays on the
  game thread. Counters expose async completions, stale discards, cache hits and
  synchronous decodes.
- Targeted invalidation: actor transform/property edits synchronize existing handles
  (`HandleActorEdited`) without re-decoding geometry or rebuilding the preview;
  mesh/texture edits and scene settings still take the rebuild path. Waypoint/path
  edits refresh spatial handles, guides and visualizers only, and light/camera edits
  refresh scene visualizers only. A targeted edit's own transaction event is
  suppressed per preview, while undo/redo still rebuilds. `Silent` bulk edits keep
  deferred full-refresh semantics.
- Rebuilds reuse decoded background, sector-background and cluster strips from a
  per-bank, generation-validated cache, so rotation/scale/lighting fallbacks and
  ground-lit translations re-decode only the affected actor
  (`GetGeometryDecodeCount` vs `GetCachedGeometryDecodeCount`).
- Generated actors are grouped in the Outliner under one transient grouping actor  per sector, attached to the preview. Labels are `<serialized name> [class <id>]`,
  disambiguated with the record index for duplicate names; labels are derived from
  the whole bank so sector load/unload cannot rename them, and serialized names are
  never modified.
- Validate, export and sector discovery write tokenized `FMessageLog` entries (one
  per bank or sector) with a clickable bank asset link and, for actor-specific
  issues, a **Select …** action that selects/focuses the generated representation
  (or the level preview for discovery problems) so Details shows the record. Each
  actor issue names the index, serialized name, recovered class and property path
  (`Actor 3 "BRIDGE" (Bridge).TrapActorRef`). Validation reports stale schemas,
  missing/invalid geometry, missing references per actor and export restrictions as
  structured issues; `Export Kya Level` validates the linked set first and logs the
  same findings plus an actionable entry for unsupported dependency banks before
  replacing any output. Sector discovery runs behind a cancellable `FScopedSlowTask`
  and leaves a coherent partial result when cancelled; `Kya.Export` reports one
  outcome per file and opens the log on failure. Imports verify the destination
  asset's source provenance before it is
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
`Kya.Bank.HedgehogFixtureRoundTrip`, `Kya.Bank.ProjectileFixtureRoundTrip`,
`Kya.Bank.MovingPlatformFixtureRoundTrip`,
`Kya.Bank.SwitchFixtureRoundTrip`,
`Kya.Bank.WindFixtureRoundTrip`,
`Kya.Bank.TrapFixtureRoundTrip`,
`Kya.Bank.WeaponFixtureRoundTrip`,
`Kya.Bank.AraignosFixtureRoundTrip`,
`Kya.Bank.BonusFixtureRoundTrip`,
`Kya.Bank.TeleporterFixtureRoundTrip`,
`Kya.Bank.AmortosFixtureRoundTrip`,
`Kya.Bank.DcaFixtureRoundTrip`,
`Kya.Bank.FruitFixtureRoundTrip`,
`Kya.Bank.NativFixtureRoundTrip`,
`Kya.Bank.WolfenFixtureRoundTrip`,
`Kya.Bank.CommanderFixtureRoundTrip`,
`Kya.Bank.KimFixtureRoundTrip`,
`Kya.Bank.BoomyFixtureRoundTrip`,
`Kya.Bank.AtonFixtureRoundTrip`,
`Kya.Bank.RopeFixtureRoundTrip`,
`Kya.Bank.StillerFixtureRoundTrip`,
`Kya.Bank.AmbreFixtureRoundTrip`,
`Kya.Bank.FountainFixtureRoundTrip`,
`Kya.Bank.CheckpointsFixtureRoundTrip`,
`Kya.Bank.BridgeFixtureRoundTrip`,
`Kya.Bank.FogFixtureRoundTrip`,
`Kya.Bank.ShootFixtureRoundTrip`,
`Kya.Bank.MoneyFixtureRoundTrip`,
`Kya.Bank.BoxFixtureRoundTrip`,
`Kya.Bank.FxFixtureRoundTrip`,
`Kya.Bank.ClusteriserFixtureRoundTrip`,
`Kya.Bank.CompanionFixtureRoundTrip`,
`Kya.Bank.GravityFixtureRoundTrip`,
`Kya.Bank.RuneFixtureRoundTrip`,
`Kya.Bank.MickenFixtureRoundTrip`,
`Kya.Bank.BasicBoxFixtureRoundTrip`,
`Kya.Bank.NativCmdFixtureRoundTrip`,
`Kya.Bank.EventGeneratorFixtureRoundTrip`,
`Kya.Bank.ExplosiveDistributorFixtureRoundTrip`,
`Kya.Bank.ShockerFixtureRoundTrip`,
`Kya.Bank.NativShopFixtureRoundTrip`,
`Kya.Bank.PunchingBallFixtureRoundTrip`,
`Kya.Bank.NoseMonsterFixtureRoundTrip`,
`Kya.Bank.PatternFixtureRoundTrip`,
`Kya.Bank.ShipFixtureRoundTrip`,
`Kya.Bank.BunchFixtureRoundTrip`,
`Kya.Bank.BrazulFixtureRoundTrip`,
`Kya.Bank.MiniGameTimeAttackFixtureRoundTrip`,
`Kya.Bank.MiniGameBoomyFixtureRoundTrip`,
`Kya.Bank.MiniGameVariantsFixtureRoundTrip`,
`Kya.Editor.ActorTransformConversion`, `Kya.Editor.ActorEditService`,
`Kya.Editor.ActorHandleTransform`, `Kya.Editor.ActorOwnedGeometry`,
`Kya.Editor.ExportValidation`, `Kya.Editor.LevelPreviewPlacement`,
`Kya.Editor.LevelSet`, `Kya.Editor.SpatialRecords`, `Kya.Editor.ActorCatalog`,
`Kya.Editor.MultiEditBaseline`, `Kya.Editor.GeneratedActorLifecycle`,
`Kya.Editor.StructuralCommands`, `Kya.Editor.OutlinerPolicy`,
`Kya.Editor.PathVisualization`, `Kya.Editor.WaypointOrientation`,
`Kya.Editor.HierarchyGeometry`, `Kya.Editor.SceneVisualizers`,
`Kya.Editor.LightExport`, `Kya.Editor.LevelSettings`, `Kya.Editor.AsyncDecode` and
`Kya.Editor.MessageLogLinks`, `Kya.Editor.AuthoringVisibility`,
`Kya.Editor.TargetedWorkflowAcceptance`, `Kya.Editor.LevelActionsReporting`,
`Kya.Bank.SelectedActorRoundTrip`, `Kya.Editor.RunningPortExportValidation`,
`Kya.Editor.StructuralEditGate`, `Kya.Editor.StructuralWriter`, `Kya.Editor.ExportAcceptance`,
`Kya.Editor.MilestoneAGate`, `Kya.Editor.MilestoneBGate`,
`Kya.Editor.MilestoneCGate`, `Kya.Editor.MilestoneDGate`,
`Kya.Editor.ViewportClickSelection`, `Kya.Editor.SharedEditPath`,
`Kya.Editor.SectorIsolation`, `Kya.Editor.GizmoAcceptance`,
`Kya.Editor.RepresentativeActorEdits`, `Kya.Editor.SaveReopenConsistency`,
`Kya.Editor.SectorAssignment`, `Kya.Editor.SelectionOrganisation` and
`Kya.Editor.SpatialRecordAcceptance` all `Success`.

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
- Acceptance flow (`Kya.Editor.TargetedWorkflowAcceptance`): a committed drag leaves
  `BuildCount` and synchronous actor decodes unchanged, a selected handle keeps its
  object identity and selection through a targeted edit and a full rebuild, and a
  stale-schema/missing-reference validation writes Message Log entries with asset and
  actor navigation tokens. On a real bank, translation, rotation and scale reuse the
  cached background/sector/cluster strips (`GetGeometryDecodeCount` stays unchanged)
  and synchronously re-decode at most the affected actor.

## 4. Not implemented yet

These remain open in [the checklist](EDITOR_WORKFLOW_PLAN.md):

- Nested reference arrays and nested actor/zone reference pickers (top-level scalar
  references are covered; array elements still show raw ids).
- Path timing/lead-in/loop visualization; zone-bounds visualization is not possible
  because the schema stores zone references without geometry (stated in the preview
  limitations and visualizer semantics).
- `UKyaLevelSet` creation is a command/menu action rather than an asset-type
  Content Browser action; it is not auto-created on import.
- Several failures are still only logged to the output log rather than the Message
  Log (for example per-actor decode/diagnostic warnings).
- Bulk/scripted edits and reimports still rebuild the preview; per-record targeted
  invalidation is used for single actor/waypoint/path/light/camera edits.
- Serialized hierarchy transform writeback (deliberately disabled until the writer
  milestone defines it) and path playback.
- Interactive viewport acceptance pass (click a mesh, W/E/R, save/reopen,
  export/reimport) in a live editor session, and Milestone A–E gates.

