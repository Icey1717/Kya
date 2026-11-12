# Unreal workflow implementation notes

Companion to [the workflow checklist](EDITOR_WORKFLOW_PLAN.md). Investigated against the current working tree and local UE 5.8 headers on 2026-11-11. These are implementation recommendations, not completed changes. No plugin build or interactive editor test was performed. API declarations cited below were inspected locally; selection, transaction and save behavior still require the proposed integration tests.

This work ports the existing implementation from this repository's source into `unreal/KyaLevelEditor`. Use that source as the reference for layouts and behavior; Ghidra comparisons and recording PS2 function addresses are not required for this Unreal porting work.

## 1. Split actor rendering from the shared preview

### Current path

In `Source/KyaLevel/Private/KyaLevelPreview.cpp`, `OnConstruction` clears mesh sections and several caches, reconstructs markers, then builds the scene. The actor loop currently starts around line 868:

1. Resolve a placement from `Bank->SceneEnvironment.Actors`.
2. Decode/reuse strips by mesh index within this build.
3. Resolve materials and patches using the shared section maps.
4. Apply hierarchy transform, actor scale/class rotation and actor position to every vertex.
5. Convert to Unreal coordinates and append to shared material-section arrays.

Consequently actor identity is lost at the rendering component boundary. `RefreshActorHandles` supplies separate sphere actors rather than owning those triangles. A collision switch alone cannot fix this.

### Recommended first implementation

- [ ] Give each `AKyaActorHandle` a neutral `USceneComponent` root and a `UProceduralMeshComponent` child for its game mesh. Keep an icon/marker child for meshless records. The current root is a sphere mesh with a 0.3 scale; that display scale must not become the serialized actor scale.
- [ ] Extract actor mesh assembly from `OnConstruction` into a reusable builder. Return sections containing local vertices, indices, UVs, colors and material bindings. Keep native hierarchy transforms baked into model-local vertices, but remove actor placement from those vertices.
- [ ] Preserve the existing material resolution sequence, patches, color conversion, primitive blend properties and strip debug filters. Splitting ownership must not also change rendering semantics.
- [ ] Put placement on the representation's transform, relative to the preview. Use an attachment policy that preserves the intended relative transform. Cache sections/materials across builds rather than creating them again for every actor move.
- [ ] Stop emitting actor triangles into the shared preview when editor representations render them. Preserve a shared rendering route for runtime/capture uses that do not create editor actors; choose one route explicitly to prevent duplicate draws.
- [ ] Test that ordinary component hit proxies select the owning actor and highlight its complete mesh before writing custom hit-proxy code. Investigated procedural-mesh source does not expose an actor-specific selection override; do not infer viewport success solely from component ownership.
- [ ] Test opaque, masked and translucent actors separately, including Unreal's translucent-selection setting. Keep physical collision disabled unless another feature needs it.

The simplest initial cost model is one procedural component per actor with multiple material sections. Measure it before investing in custom batched hit proxies. Background and cluster geometry can remain batched independently.

### Lighting and invalidation caveat

Actor colors currently depend on transformed normals, sector lighting and scenery ground samples (`FindSceneryGroundLight`). A translation can change ground lighting; a rotation changes normal-based lighting. Moving the component alone gives immediate geometry feedback, but a targeted lighting refresh is still needed. Cache original colors/normals separately from relit output so repeated edits do not relight already modified colors.

`OnConstruction` also returns early when the main bank lacks a valid background mesh entry, before reaching actor assembly. Decouple actor/marker construction from background availability so meshless or incomplete scene data does not suppress editing.

## 2. Define the editor model and mutation boundary

Proposed types below are new, not existing APIs:

| Proposed type | Responsibility | Suggested location |
| --- | --- | --- |
| `FKyaActorRecordKey` | Weak bank reference, actor index, import generation | Editor module |
| `FKyaLevelEditService` | Resolve records, validate patches, participate in transactions, emit changes | Editor module |
| `FKyaActorEditPatch` | Explicit changed property paths and values; never an unconditional whole-record overwrite | Editor module |
| `FKyaBankChange` | Bank, record keys, property paths, change reason and invalidation flags | Editor module |
| `UKyaLevelSet` | Persisted main bank and sector associations | Data module |
| actor mesh builder/cache | Decode and produce reusable render sections | Data/render module |

Use `(bank, original array index)` while counts remain fixed. Add an editor session generation that changes on reimport/replacement; stale selections and delayed edits must fail resolution rather than editing a different record. Do not persist raw pointers into `Actors`, since array replacement invalidates them.

Keep existing public handle functions as adapters during migration. Because `KyaLevelEditor` already depends on `KyaLevel`, do not make handles in `KyaLevel` call back through a new reverse module dependency. Options are editor-only delegates declared in the lower module and registered by the editor module, or moving editor-only representation classes into the editor module with a compatibility/migration decision. Prefer delegates for the first patch to reduce class relocation risk.

## 3. Transform conversion and editing lifecycle

### Position conversion is already defined

For bank position `p`, axis permutation `C(x,y,z) = (x,z,y)` and preview transform `T`:

```text
world position = T.TransformPosition(100 * C(p))
bank position  = C(T.InverseTransformPosition(world position) / 100)
```

Apply this conversion once. The older component marker uses relative coordinates; the actor representation uses world coordinates. Consolidate both routes so moving a transformed preview works consistently.

### Rotation needs an inverse implementation

`KyaActorTransform::ApplyClassRotation` supports yaw-only, Euler-matrix and default basis-building paths. Build the forward basis by transforming unit axes with the existing function, then conjugate by the axis permutation for the Unreal basis. The permutation changes handedness, so swapping Euler components is not sufficient.

- [ ] Implement and test inverse conversion separately for each supported class path. For yaw-only classes expose yaw and preserve unedited serialized components instead of zeroing them merely on selection.
- [ ] Choose equivalent angles nearest the previous serialized values to avoid gratuitous angle wrapping. Compare orientation using transformed bases, not Euler tuple equality.
- [ ] Test singular/default-basis branches near vertical forward vectors and bank rotations with roll. Do not claim unrestricted rotation support until those paths round-trip.
- [ ] Initially reject preview transforms or actor scales that produce an unrepresentable shear/degenerate basis. Document negative/nonuniform scale support as tested cases, not assumptions.

### Engine entry points verified locally

UE 5.8 `Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h` declares `PostEditMove(bool bFinished)`, `EditorApplyTranslation`, `EditorApplyRotation` and `EditorApplyScale`. These are available integration points; their declarations alone do not establish transaction order.

Recommended lifecycle:

1. At the first interactive mutation, resolve identity and capture the bank before changing it. Join the editor's transaction where it owns the operation; create an explicit transaction for plugin commands.
2. Apply interactive component feedback and validated placement updates without a full preview rebuild. Guard bank-to-view synchronization against recursive writeback.
3. On completion, validate the final transform, refresh affected lighting and issue one change notification. On cancellation, restore state through the transaction and refresh views.
4. On undo/redo, reconstruct transforms from authoritative bank data, including the currently selected representation.

Do not open a separate `FScopedTransaction` on every `PostEditMove` call. Prototype one drag with transaction logging before settling the callback sequence. Numeric Transform edits need their own property-change route into the same service.

## 4. Details without a second hand-written actor schema

The existing handle copies a dozen fields into members, waits until Tick and writes all of them back. That can clobber unrelated newer values and may place the bank modification outside the original property transaction. Replace this mechanism before exposing hundreds more fields.

### Recommended prototype

Register a class Details customization for `AKyaActorHandle`. Build supported rows from the reflected `FKyaActorPlacement` schema, with an editor-only capability registry keyed by property path/class/presence flags. Generate a temporary full typed record for presentation if needed, but compute a patch containing only the changed leaf or fixed-size subtree. The bank remains authoritative.

UE 5.8 `PropertyEditor/Public/DetailCategoryBuilder.h` declares:

- `AddExternalObjects` / `AddExternalObjectProperty` for UObject-backed properties.
- `AddExternalStructure` / `AddExternalStructureProperty` for `FStructOnScope` data.
- An external structure-property overload accepting `IStructureDataProvider`.

These are viable UI building blocks, not a guarantee that an external struct gets bank undo automatically. First prove a single nested field with pre-change bank `Modify()`, post-change patch validation and undo/redo. Avoid binding long-lived external rows directly to array element memory. On reimport or selection changes, invalidate/recreate the view safely.

- [ ] Use nested/chain property events so editing `Position.X` or an array element identifies the correct leaf and owning record. The current handle's top-level whitelist is insufficient for the full schema.
- [ ] Preserve original array counts and expose no structural controls for fixed-layout arrays. Presence flags should control visibility, but are generally not user-editable record-creation switches.
- [ ] Pair the reflected UI with export capability metadata. `EditAnywhere` says that Unreal can edit a property; it does not prove the bank writer can encode every value or layout change.
- [ ] Preserve unknown bitfields and use raw-value fallbacks. Validate reference pickers against the relevant bank's reference namespace rather than assuming every integer reference is an actor-array index.
- [ ] Refresh presentations through notifications, including selected objects. Keep mixed-value handling explicit for multi-selection; commit all affected banks within one command transaction.

Prototype placement fields, one nested platform field and one fixed-size behaviour array before generating the full class-specific surface.

## 5. Change notification and generated-object lifetime

UE 5.8 `CoreUObject/Public/UObject/UObjectGlobals.h` exposes `FCoreUObjectDelegates::OnObjectPropertyChanged` and `OnObjectTransacted`. Filter notifications to known banks and use them to catch asset-editor edits and undo/redo. Unregister delegate handles during module shutdown. Plugin mutations should emit richer internal changes because a generic UObject event may not identify the precise nested actor efficiently.

Maintain a weak registry from banks to preview instances. Coalesce changes and classify them:

| Change | Minimum refresh |
| --- | --- |
| Name | Details/Outliner label |
| Position | Representation transform, bounds and ground-light update |
| Rotation/scale | Transform, bounds and actor lighting |
| Mesh/model/texture override | Actor sections/material bindings |
| Class tuning with no preview effect | Details only |
| Reimport/schema change | Invalidate keys/caches and rebuild representations |

`RefreshActorHandles` currently preserves actors by index but does not explicitly set transient spawn flags. Set the intended flags deliberately and test map serialization. Retain UObject references for generated objects where needed for garbage collection; weak cross-view registration is not ownership. Clean up on map/world teardown as well as preview destruction.

Generated editor actors and bank changes have different undo lifetimes. A bank edit must remain undoable without requiring a deleted transient representation to resurrect. Reconcile missing representations after undo, preserving selection by record key where possible.

## 6. Sector discovery and level-set data

The existing `GetSectorId` parses `SECT<number>` from `SourceFilename`, falling back to the asset name. The capture script imports explicitly listed sectors from metadata into generic `/Game/SECT<n>` paths. It does not discover the complete level and is unsafe to reuse unchanged for multiple levels.

Suggested persisted descriptor:

```text
UKyaLevelSet
  mainBank: asset reference
  sectors[]: { sectorId, bank reference, source-relative filename }
  sourceRoot/import provenance
  descriptor version
```

Use normal serialized asset references for a simple always-loaded initial implementation. Introduce soft references and async loading only if measured level size requires them. Keep visibility/load state on the preview or editor settings rather than modifying game data.

Discovery algorithm:

1. Resolve the main bank's source directory, enumerate sibling files and accept case-insensitive exact `SECT<digits>.BNK` names. Sort numerically and reject duplicate numeric IDs such as `SECT1` and `SECT01` unless explicitly resolved.
2. Validate candidate bank structure with shared import logic. Filename identity is provisional; do not invent a trustworthy embedded sector ID if none is parsed.
3. Match existing assets using normalized provenance plus level association. Asset name alone is insufficient. Use Asset Registry tags for discovery rather than loading all banks.
4. Import only missing assets under a level-specific destination. Do not replace edited assets during discovery. Report missing expected IDs separately from files found; actor sector references can suggest missing content but are not proof of the complete native sector list.
5. Update the descriptor transactionally, retain manual overrides, and refresh each dependent preview once. Canceling should leave a clearly reported subset, not orphaned associations.

Automatic sector file discovery and automatic actor sector reassignment are different features. The current scenery collision helper samples ground lighting; it is not a sector-volume containment algorithm. Keep actor sector assignment explicit until native membership rules are investigated.

## 7. Reimport, validation and export factoring

`UKyaLevelBankFactory` currently derives only from `UFactory`. UE 5.8 `UnrealEd/Public/EditorReimportHandler.h` confirms `FReimportHandler` requires `CanReimport`, `SetReimportPaths` and `Reimport`. Add that integration and editor-only import provenance to the bank.

Before reimport support, extract parsing from `FactoryCreateBinary` into a function producing a temporary parsed result. Validate fully before replacing the existing UObject's fields. Preserve the asset identity; notify representations only after a successful replacement. On failure, leave the previous data intact.

Reimport must not silently discard edited typed fields. Initially provide cancel/discard choices when changes exist. A later three-way merge can parse the retained original `BankBytes` as the baseline, compare current typed values and incoming parsed values, and report conflicts by property path. Schema changes require explicit compatibility rules; equal array indices alone do not establish semantic identity across changed sources.

The exporter starts by requiring scene configuration and scene-data records and rejects unsupported schema/compressed retained data. Therefore exporting an arbitrary discovered sector through this exporter is **not yet established**. Level-set export must distinguish editable supported banks from unchanged sector dependencies; verify sector capabilities before promising round-trip support.

Extract export into two layers:

```text
BuildExportBytes(bank) -> bytes or structured diagnostics
WriteValidatedExportSet(results, destination) -> per-file outcome
```

The existing `UExporter` becomes an adapter. Reuse the in-memory builder for preflight and round-trip tests. Replace bare `return false` paths with property/record diagnostics incrementally. A group of filesystem replacements is not automatically atomic: stage files first, track which replacements succeeded, and define rollback/backups before calling whole-level export reliable.

Save and export remain separate commands. Saving writes dirty Unreal packages; exporting writes game files. Verify the normal editor Save action on a selected generated actor whose owning map is clean, and implement Save Kya Level for explicit dependency saving if needed. Generated transforms alone must never be the persisted edit.

## 8. Module integration and small implementation slices

Current `KyaLevelEditor.Build.cs` has Core, CoreUObject, Engine, KyaLevel, UnrealEd and ProceduralMeshComponent. Add dependencies only with features that use them: `PropertyEditor` for customization, Slate/SlateCore for widgets, AssetRegistry/AssetTools for import/discovery, and ToolMenus for commands. Replace the default editor module implementation with startup/shutdown registration and symmetrical cleanup. Keep editor services out of runtime dependencies.

Suggested reviewable patches:

1. **Mutation foundation:** keys, bank validation/change notifications and transaction tests. Adapt one handle field; eliminate deferred whole-record writes for that path.
2. **Actor-owned rendering prototype:** split sections for a single actor, neutral root, ordinary selection test, no duplicate mesh. Preserve capture output and lighting semantics.
3. **Translation slice:** forward/inverse position conversion, gizmo and numeric edit, one-step undo, save/reopen and export/reimport. Follow with separately tested rotation/scale.
4. **Details slice:** reflected placement plus one class-specific nested record, explicit capability filtering and reference picker pattern; then expand coverage.
5. **Level-set slice:** discovery/provenance, unique destinations and missing-sector diagnostics, followed by safe reimport.
6. **Optimization and broader objects:** targeted refresh and reusable spatial-record views for paths, waypoints and hierarchies.

Existing `Kya.Bank.SmallBankRoundTrip` and `Kya.Bank.LevelOneRoundTrip` in `KyaLevelBankTests.cpp` are useful writer regressions. Add editor integration coverage separately: a synthetic bank with two actors, duplicate display names, a nested editable property, and two preview instances at different transforms. Test real-bank cases only when the required assets are available and report skips explicitly.

Minimum manual gate: click the actual visible mesh, translate with snapping, edit a nested field, undo twice, redo twice, save, close/reopen, export and reimport. Repeat with a yaw-only actor, a meshless actor, a transformed preview root and a missing sector. Scripted selection alone does not cover viewport hit testing.

## 9. Remaining investigation gates

- [ ] Prove transaction boundaries for numeric Details edits, interactive drags and Escape cancellation in UE 5.8.
- [ ] Prove selected-object highlighting and selection through masked/translucent procedural geometry.
- [ ] Establish export support per sector bank type and whether any sector records currently have writable typed counterparts.
- [ ] Verify class rotation inverse conventions against the existing repository source before broad rotation support; no Ghidra comparison is required for this Unreal port.
- [ ] Measure actor-component cost and validate garbage collection/map reload before committing to large-scale per-actor rendering.
- [ ] Decide whether full-schema Details can safely use external UObject properties or needs a typed presentation object after the nested-field undo prototype.

These are bounded prototypes with acceptance checks, not reasons to postpone the already-understood translation, ownership and discovery work.
