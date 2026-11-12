# Kya Level Editor

This plugin is under development. A `.bnk` file can be imported as an asset;
the original bytes are retained. The initial scene configuration, collision bank
values, animation allocation count, actor class allocations, and the scene's
initial fog, clipping, mipmap settings and basic main camera fields can be edited on
the asset and exported to an uncompressed bank. Array sizes are fixed. The export path is
experimental; a fixed-size actor scale edit has been verified in the running game,
while count- or size-changing edits are rejected. Assets imported before
the scene and actor Create fields were expanded must be reimported to populate
the new fixed-size arrays before export. Moving-platform actors expose typed
weight, carried-actor, zone, no-friction, camera/notification, sound and
configuration fields. Basic-box actors expose typed life, vibration, switch and
add-on generators. FX actors expose their two Create-stage floats and their
single/path behaviour payloads. Araignos actors expose their movable weight,
path-follow index, animation/damage floats, sound references, and add-on
generator. Projectile actors expose their autonomous movement values, 72-byte
tuning sub-object, and pattern-part settings. Wind actors expose their tuning,
zones, actor/material references, sector hierarchy, and switch records. Trap
actors expose their movement/path settings, escape state, trap switch stream,
and both behaviour payloads. Weapon actors expose their class values and the
burst/bullets/pistol/sniper/RPG behaviour payloads. Switch actors expose their
target list and camera event. Bonus actors expose their sound/material values
and turn/flock/add-on behaviours. Micken actors expose their autonomous
values, waypoint/actor references, and eat behaviour. Wolfen actors expose
their vision/detection and movement tail, the fixed fighter block, the
blow/grab/reaction/combo databases, and the FightParam capabilities/lists/
knowledge block. Fruit actors expose their autonomous/switch data and
squashed/bonus/inventory behaviours. Commander actors expose their squad, team,
sequence and camera-figure data plus behaviour 2. Nativ actors expose their
autonomous/movement fields and take-and-put trajectories. Amortos actors expose
their actor lists, tuning floats, add-on generator, switch/camera records, and
cinematic behaviour. Teleporter actors expose destinations, three switch/camera
records, zone/material references, waypoints, and conditioned operations. DCA
actors expose their aim/shot tuning and controlled/trajectory behaviours. Box
actors expose their autonomous tuning, actor list, switch and add-on. Money
actors expose their weight, value, sound and material. Fog managers expose their
hierarchy list and streamed fog zones. Shoot actors expose static-mesh/aim tuning
and fire/wave behaviours. Ambre actors expose their mesh/material and switch
data. Stiller actors expose vision, path, life, switches and add-on. Rune,
bonus-fountain and rope actors expose their serialized tuning, references and
switch data. Gravity-aware/bell actors expose autonomous/behaviour data, command
actors expose add-on lists, and checkpoint managers expose their checkpoint
records. Bridge actors expose their actor/waypoint/config data and behaviours;
companion actors expose their autonomous values and ids. Aton actors expose their
autonomous values, effect/mesh/material ids, path list, add-on sub-objects and
Cinematic behaviour. Boomy actors expose their tail/particle fields, embedded
camera, and per-type vision/damage table. Kim/HeroPrivate actors expose the full
shared CActorFighter prefix (autonomous block, fixed block, blow/grab/reaction/
combo databases), plus bone keys, climb/tutorial/projectile/boomy zone lists,
eight embedded cameras and FX handles. Companion alert behaviours expose their
scenario gate, target/camera switch and conditioned operations. Moving-platform
behaviours expose their trajectory/stand/slave/destroyed/selector fields, and the
shared 4-byte cinematic record is typed for every class. Projectile
stand/straight/portable/excuse behaviours and switch lever/magic/multi-condition/
target behaviours expose their fields. Wolfen track/DCA/avoid and the track-weapon
behaviours expose their flags, tuning, sub-object lists and the lost behaviour's
switch-script records, and the watchdog/snipe behaviours expose their tuning,
switch scripts and on/off camera. Moving-platform slab/master/selector-master
behaviours expose their switches, analog targets and selector lists. Wolfen
guard-area/escape/exorcism behaviours expose their path readers, switch scripts,
on/off camera and mesh/texture pairs. Kim hero-unknown behaviours, Micken/Nativ
catch-by-trap and the projectile inventory behaviour expose their payloads. Import
schema 50 requires reimporting older assets. An experimental `KyaLevelPreview` actor
displays background meshes with hierarchy transforms and
supports main scene static cluster strips (level one's main cluster is empty).
Enable `Show All Actor Meshes` to preview level-placed actor models; they decode from the
bank's `.G3D` mesh entries and use each actor's recorded placement. Actors without a texture
index resolve their materials by hash across every texture bank in the bank. Actor materials
are resolved in native order: the model's own texture bank first, then matching hashes from
the shared actor-material bank (`Alternate Texture Index`) replace them, then explicit
`Actor.Models` texture patches. Editing any preview
property (bank, sectors, colour space, actor meshes, markers) rebuilds the mesh immediately.
A preview warns with `KYA_PREVIEW_STALE_SCHEMA` when its bank predates the current import
schema; reimport the bank to refresh typed data. The bank also exposes the level's light
archive (sun, spot and torch entries) and `Apply Actor Lighting` relights actor models from
the sun entries using each strip's stored model normals, transformed by the hierarchy and
actor placement. Strips without stored normals keep their baked vertex colours. `Show Background Meshes`,
`Show Cluster Meshes` and `Show Sprites` isolate those layers for inspection. Sprites use the prepared
packet centers, UVs, colours and width/height data stored in the cluster, expanded with
`Sprite Camera Forward` and `Sprite Camera Up`. Level 1's sector 2 and 4 sprite packets
use alpha-tested opaque materials (`ABE=0`); no additive or blended sprite materials are
present in those sectors. Hierarchy type-3 background objects are also inspected:
the fixed sun flare configured by native `SetupBackground` is projected at the
current camera and composited with the native tint and 16-by-16 depth-visibility
marker. Such sprites need not have a `SPR.` chunk. Additive mesh materials use
the strip GIF primitive's ABE bit (bit 6) and GS `ALPHA` register (`0x42`), including
the source-alpha or fixed blend selector. `ALPHA=0x48` uses source alpha; its
stored FIX value is unused. Level 1's sector 1 background contains three strips
using an additive material; these are not ordinary opaque surfaces.
Sector scene instances are submitted separately from CSTA references: the
`DYNAMIC` root and its children, plus roots referenced by the ANHR entry. Sector
1 has eleven ray strips and ten tree strips on this path. These scene instances
use their hierarchy transforms; animated trees currently show their stored pose.
The editor generates a basic unlit texture material with vertex-colour modulation automatically. By default it
combines texture and vertex colours in the game's encoded colour space and then decodes the product for Unreal's
linear output; this reproduces the game's vertex shading much more closely than Unreal's normal sRGB sampling.
Disable `Game Colour Space` only to compare against Unreal's own conversion. To override the generated material,
assign `Texture Material` a material with a Texture2D parameter named `KyaTexture`.
Supported
indexed and direct-colour textures on main and sector backgrounds are decoded automatically when supported; unsupported
materials and unsupported cluster geometry remain untextured and use their vertex colours. Supported cluster strips resolve sector and common textures. General sprite lighting/blending and some scene geometry
are missing, and the preview is not a faithful level render. The runtime module is separated from editor tooling so that level
data and rendering can also be used by a future playable Unreal project.

The editor goal is to select individual Kya actors by clicking them in the
Unreal viewport and edit their properties through the World Outliner and
Details panel or a custom editor panel. World geometry should support the same
workflow at the smallest practical piece granularity.

The preview creates editor-only waypoint markers. Moving a marker updates its
position in the referenced bank asset; save the asset and export it to write the
change to a bank. Marker orientation does not yet represent waypoint rotation.
Set `Preview Path Index` on the preview actor to inspect a path's points as a
linear guide. Moving a point marker updates the bank asset. The game's absolute
path reader also interpolates positions linearly between its chosen points;
the guide does not yet model reader timing, lead-in, or loop behavior.
Set `Preview Actor Index` to show a position marker for one actor; moving it
updates the bank asset. Actor meshes and orientation are not shown by this marker.
The optional `Show Actor Mesh` setting also attempts a static model preview for
the selected actor. It applies the initial class rotation rules, resolves the model's
materials against its own and the shared actor-material banks, and applies explicit
`Actor.Models` overrides. Animation and some geometry and texture formats are not supported,
so it should be treated as approximate. Set `Preview Actor Strip Index` to isolate one
strip of the selected model for material or shape debugging.
`Show All Actor Meshes` applies the same preview to every actor placement. It can
increase editor load time and includes actors that the game may initially hide.
`Tools/capture_actor_mesh.py` captures one isolated actor with an off-screen scene
capture (independent of editor focus). Set `KYA_ACTOR_NAME` to choose the actor and
`KYA_ACTOR_LIGHTING=0` to compare baked vertex colours; `KYA_ACTOR_STRIP_SCAN=1`
writes one image per strip using `KYA_ACTOR_STRIP_COUNT`.
Set `KYA_ACTOR_DISABLE_TONEMAPPER=1` to export directly in sRGB for native
color comparisons. Set `KYA_ACTOR_TEXTURE_ONLY=1` to bypass vertex RGB
modulation, or supply `KYA_ACTOR_SECTORS=/Game/SECT1,/Game/SECT7` to include
the corresponding ground-light data in a lit capture.
Enter `Kya.ViewportTonemapper 0` in the editor console to inspect the preview
without Unreal's tone curve, or `Kya.ViewportTonemapper 1` to enable it again.
Both camera-aligned capture scripts disable the tone curve by default so their
output matches native colour; set `KYA_CAPTURE_DISABLE_TONEMAPPER=0` to keep it.

For an isolated bind-pose comparison, [capture the native actor and draw metadata](../../tools/debug_socket/README.md#how-the-bind-pose-works)
and set `KYA_ACTOR_NATIVE_CAPTURE` to the combined screenshot JSON before
running `Tools/capture_actor_mesh.py`. The preview already renders the stored
mesh with its hierarchy transforms. In capture mode it uses the native
object-to-world matrix in place of the actor's archive placement and uses the
recorded camera position, orientation, and horizontal FOV. This mode requires
the captured submissions to share one model matrix. Set
`KYA_ACTOR_NATIVE_VIEW_HEIGHT` to the picture height excluding letterbox bars
if present.

For a lighting comparison, set `KYA_ACTOR_LIGHTING=1`,
`KYA_ACTOR_DISABLE_TONEMAPPER=1`, and assign the relevant sector assets through
`KYA_ACTOR_SECTORS`. Unreal computes lighting from its imported bank and sector
data at the captured placement. The native light packet in the JSON is
available for diagnosis.

Enable `Spawn Actor Handles` on the preview to create one selectable
`AKyaActorHandle` per actor placement in the loaded sectors. Each handle is
labelled with the bank actor's name, can be clicked in the viewport or picked in
the World Outliner, and mirrors the placement fields in its Details panel: name,
mesh/texture index, position, rotation, scale, sector, default behaviour,
visibility distance, lighting value and lighting flags. Edits write back to the
bank asset with undo support and rebuild the preview; handles are editor-only and
are destroyed with the preview or when the option is cleared. The preview's
`Build Count` confirms a rebuild after an edit. `PreviewActorIndex`,
`PreviewActorStripIndex`, `TextureOnlyActorIndex` and the capture scripts continue
to work unchanged.

**Authoring visibility.** By default the preview runs in authoring discovery mode:
`Authoring Visibility` keeps actors whose sector bank is not loaded selectable
(`Authoring Show Unloaded Sectors`, reported by the preview's `Authoring Only
Handle Count`) and `Authoring Show Culled Geometry` draws cluster scenery that the
native distance/LOD test would hide. This is separate from the game-like
distance/sector culling used for comparison: setting a preview camera
(`SetPreviewCamera`, used by the capture scripts) or disabling authoring visibility
restores native culling, and game/PIE worlds never use the override. Setting that
camera refreshes actor membership immediately, and `ClearPreviewCamera`
(`Return to Editor Camera`, `Kya.ClearPreviewCamera`) restores authoring discovery.
Toggle visibility from the Kya Level menu or with `Kya.AuthoringVisibility [1|0]`.

`Spawn Actor Handles` and `Show Actor Meshes On Handles` are enabled by default:
each handle owns that actor's game mesh (one procedural mesh section per resolved
material), so clicking any strip selects the owning actor and shows its bounds for
Focus Selected. Actor triangles are then not added to the shared preview mesh, so
there is exactly one draw path. Diagnostic mesh modes (`Show Actor Mesh`,
`Show All Actor Meshes`) keep the shared route and hide the handle meshes, so
capture output is unchanged. Meshless actors keep a small icon marker instead of a
mesh; that display scale is never written to the bank.

Interactive moves (W/E/R) update the handle immediately. The bank placement is
written once when the interaction ends, through the shared edit service
(`KyaActorEditService.h`), which validates the record, calls `Modify()` before
mutating, writes only the changed fields, marks the package dirty and refreshes
every preview that displays the bank. A canceled drag restores the original
transform before that point, so it does not dirty the bank. Rotation conversion
uses the class rotation rules: yaw-only classes write yaw and clear pitch/roll
(their native representation), and orientations that a class cannot represent are
rejected with a log message and the visible transform is restored from the bank.
Unreal axis edits are converted with the game's Y-up permutation; non-uniform
scale is supported, while negative or zero scale is rejected instead of being
stored as a misleading transform. Numeric Details edits use the same conversion
and patch path.

Actor records are identified by bank asset, actor index and a bank edit
generation. Reimport or replacement bumps the generation, so a stale handle fails
to resolve instead of editing a different record. Handles and other views
subscribed to a bank refresh once per edit, including for undo/redo and for
changes made in the bank's own asset editor.

Handles are grouped in the World Outliner under one transient grouping actor per
sector (attached to the level preview, labelled `<bank> sector <n>` or
`<bank> global actors`). Their labels are the serialized actor name plus the game
class (`<name> [class <id>]`); duplicate names are disambiguated with the record
index so the Outliner never shows two identical entries, without modifying the
serialized names. Labels are derived from the whole bank, so loading or unloading
a sector does not rename existing handles.

Validation, export and sector discovery report through the Message Log
(`KyaLevelEditor` listing) as well as the dialog summary: one entry per bank or
sector, with the asset path, the failure reason and any warnings. Entries are
tokenized: the bank is a clickable asset link and actor-specific problems add a
**Select …** action that selects and focuses the generated representation (or the
level preview for sector/discovery problems). Each actor issue names the index, the
serialized name, the recovered class and the property path (for example
`Actor 3 "BRIDGE" (Bridge).TrapActorRef`), so selecting it opens that record in
Details. `Export Kya Level` validates the linked set first and logs the same
findings, plus an actionable entry for unsupported dependency banks, before any
output is replaced. A blocking issue (including a writer rejection attributed to an
actor record and property, such as a name that no longer fits its serialized slot)
stops the export before staging, so no destination is replaced.

`Kya.Editor.RunningPortExportValidation` closes the loop with the Windows port: it
edits one real actor through the shared service in a transient validation copy,
exports the bank, then runs the port's `RunningPortExport.ValidatesEditedBank` gtest.
That test reads the file through the port's own `edBank` container structures,
parses the actor layout in `CActorManager`/`CActor::Create` order, and constructs the
actor through the native `CActorFactory`/`CActor::Create` path to inspect the
resulting position and visibility/culling values. The validation copy clears that
actor's mesh/texture/collision references (meshless native create needs no scene
resources); the edit under test is position/visibility/culling and the source bank
is untouched. The Unreal test requires proof from a gtest XML report (`tests=1`, no
failures/errors/skips) in addition to a zero exit code, and skips with a warning when
`bin/WIN/KyaPortTest.exe` is not present. Missing sectors name the expected
`SECT<id>.BNK` file; stale schemas, missing references, unsupported/missing geometry
and export restrictions are reported the same way, so a problem can be inspected
without searching raw logs. Sector discovery shows a cancellable progress dialog and
leaves a coherent partial result when cancelled.

The handle's Details panel shows the bank record itself: one row per editable
`FKyaActorPlacement` property, grouped by the schema's own categories, including
nested fixed-size arrays, class-specific records and behaviour payloads. Edits are
applied one top-level property at a time through the shared edit service, so a
nested field or array element change writes only that property and produces one
undo step. A field capability table (`GetKyaActorFieldCapability`) classifies every
property as editable-exportable, editable-presentation-only, inspect-only or
unsupported, with a reason. Export support comes from the writer's own field
references (`KyaExportedActorFields.h`), so a field the exporter does not encode is
inspect-only even when it is `EditAnywhere`. The table handles the serialized name
slot (`Align(length + 1, 4)` must match the original capacity), fixed-size arrays,
raw class-payload size, `VisibleAnywhere` record data and the layout-controlling
`bHas*` presence flags; `GetKyaNestedFieldCapability` applies the same rules to
nested records such as `Models.MeshName`. The panel only offers editable fields and
shows the capability reason as the row tooltip. The old hand-written mirror fields
remain on the actor for script compatibility but are hidden from Details. The
selected-actor gate (`Kya.Editor.MilestoneCGate`) verifies that surface end to end:
every export-supported field is committed through the Details patch path with a
distinct value, must actually change the record, must be exposed by the panel's own
row filter (`KyaGetActorDetailsProperties`) and must survive export/reimport;
nested, reference-picker (through `KyaSetActorReferencePick`, including its
out-of-range rejection) and multi-actor bulk edits round trip as well.

The identity section shows the recovered game class name and numeric id (forexample `Moving platform (13)`, `Unknown class 127`), with the recovered notes as a
tooltip and a Browse button for the bank asset. Recovered class-specific behaviour
names appear in the default-behaviour picker and in a behaviour summary; ids without
a recovered name are labelled `Behaviour N (unknown)` instead of being guessed. The
References section provides searchable pickers for the sector (loaded sector ids),
model mesh ordinals, texture banks, default behaviour, and the class-specific scalar
actor/waypoint/path fields, always showing the raw serialized id. Actor references
have a Select button that selects the referenced actor handle. Unresolved references
are listed inline with the exact reason, and updates to mesh, texture or sector
references are rejected before the bank changes when they would not resolve. Flag
widgets show understood bits (for example lighting bit 1 = ignore ground lighting)
and always display the raw value; unknown bits are preserved exactly. Fixed-layout
arrays and nested reference arrays still expose raw element ids only.

Selecting several actor handles switches Details to the multi-edit panel. It reports
the shared class and whether the selection is compatible (same class and the same
optional records); class-specific fields are hidden for incompatible selections
while shared fields stay editable. Shared Transform and actor settings rows can be
applied to the whole selection, show `Mixed` when values differ, and each bulk edit
is one undo step and one preview refresh per bank. Rejected values leave every
record unchanged and report the failing record inline.

Each bank also keeps a typed **imported baseline** captured by the bank factory (and
refreshed on reimport); it is editor metadata and never exported. Details shows
whether the selected actor still matches the imported bank and offers `Reset to
imported` for the whole record, `Reset field` for one property, and `Copy` to copy a
chosen field from another actor in the same bank. `Reset all to imported` restores a
whole multi-selection in one transaction. Reset and copy preserve identity and
fixed layout; the baseline is separate from Unreal class defaults.

Generated representations are editor-only and transient, so they are never saved
into a map or cooked into a packaged game, and opening the level in PIE or a game
world creates none of them. Deleting a generated actor is blocked with an
explanation from `CanDeleteSelectedActor`. Duplicate, cut, paste and Alt-drag are
not silently allowed to create free-floating copies: at the end of the command any
generated copies that no preview owns are removed and a dialog explains that these
actors mirror bank records and must be edited through the original handle or the
bank asset. Rebuilding or reloading the map recreates exactly one representation per
bank record.

Generated representations support the usual Outliner organization: labels and
per-sector groups give multi-selection and search, and focusing works on the handle
bounds. `Hide Selected Generated Actors` temporarily hides the selection in the
editor only (`SetIsTemporarilyHiddenInEditor`, never game visibility), `Isolate
Selected Generated Actors` hides every other representation, and `Clear Generated
Isolation` restores them; the state survives rebuilds. The same actions are
available as `Kya.HideSelected`, `Kya.ShowSelected`, `Kya.IsolateSelected` and
`Kya.ClearIsolation`. Selection locking (`Kya.LockGeneratedSelection`, or the
preview's `Lock Generated Actor Selection` / a handle's `Selection Locked`) keeps
representations visible but stops viewport picking.

Transform and property edits are applied in place: the affected handles are
resynchronized, labels/grouping and light/camera/link/trigger visualizers update,
and the level geometry is **not** decoded or rebuilt. Mesh/texture changes and level
settings still rebuild the capture; undo/redo and reimports keep the full-refresh
path. A multi-actor drag or waypoint edit therefore does not rebuild the level.
`Kya.Editor.TargetedWorkflowAcceptance` exercises this acceptance path end to end:
a drag writes the bank without rebuilding or decoding, a selected handle keeps its
identity and selection through targeted and full refreshes, and stale/missing data
produces navigable Message Log entries.

Imports, exports and preview rebuilds show a cancellable progress dialog in
interactive sessions. Cancelling an import discards it; cancelling an export writes
nothing; cancelling a rebuild stops at the next sector/actor boundary and reports
`RebuildStatus = Cancelled`. Actor mesh strips are also decoded on the thread pool
into a data-only cache keyed by bank revision: UObject/material/mesh creation stays
on the game thread, only the newest result is kept, and stale results after a
reimport or newer request are discarded (counters are exposed for diagnostics).

Renaming follows an explicit policy: the Outliner label is presentation and the
serialized actor name is the edit. A rename strips the generated `[class <id>]` and
duplicate `[<index>]` suffixes, then validates the result: single-byte ASCII, at
most 63 characters, and the writer's `Align(length + 1, 4)` must still fit this
record's fixed name slot. Accepted renames write through the shared edit service
(one undo step) and reapply the derived label; rejected renames leave the bank
untouched, restore the label and explain the limit — names are never silently
truncated. Waypoint, path, hierarchy and sector-group labels are generated from
their records and a rename attempt is reverted with an explanation.

Waypoints and path points have the same treatment. `Spawn Waypoint Handles` creates
one transient `AKyaSpatialRecordHandle` per waypoint (position, stored rotation and
`CWayPoint` parameter triple) and suppresses the diagnostic waypoint arrows while
enabled. `Spawn Path Point Handles` creates one handle per point of every path,
labelled `Path <n> Point <m> (<x>, <y>, <z>)`, so spatial records can be found
without setting `Preview Path Index`; waypoint labels carry their game-space position
too and refresh after edits, so a record is identifiable in the Outliner without
knowing its index. The spline guide still follows that index.

`Kya.Editor.MilestoneDGate` exercises this as a session on the real bank: it selects
an actor by its Outliner label with no capture camera or `PreviewActorIndex`, uses
hide/isolate/lock and the delete-refusal reason, drags waypoint/path/actor handles
found by label, and asserts the bank writes while the level never rebuilds or
re-decodes geometry; unresolved references surface as blocking diagnostics naming the
actor and property.

`Show All Paths` (`Kya.ShowAllPaths [1|0]`, or the `Show All Paths` menu entries)
draws a guide for every non-empty path and spawns its selectable points, so no
numeric preview index is required. Guides use the game→Unreal axis mapping and
explicitly set every point to linear interpolation — the native point-to-point
semantics, not Unreal's smooth spline defaults. Each path gets a transient
`<bank> path <n>` Outliner group that owns its point handles, so ownership is
visible and hide/isolate/cleanup treat the group and points together. Editing a
point updates both the bank and its guide. The legacy `Preview Path Index` still
draws one guide when all-path mode is off.

Both handle kinds
have bank + index + generation identity, write through the same edit service as the
old arrow markers (which now call the service too, so both routes agree), and
preserve unedited values and undo grouping. Stored waypoint rotation is an XYZ Euler
triple in radians: the ported source assigns it to `rotationEuler` and converts it
with `SetVectorFromAngles`/`edF32Matrix4FromEulerSoft(..., "XYZ")` (for example
`ActorMicken`, `ActorTeleporter`). The handle therefore shows a green forward arrow
built from that exact basis, rotates with the gizmo, and writes the inverse triple
back through the edit service; a pure translate preserves the stored angles. Path
timing, lead-in and loop behaviour remain explicit non-goals: path-point rotation is
edited numerically and exported, but no playback is simulated.

## Editor commands

Import a `.bnk` file with the normal Content Browser import (the bank factory is a
standard binary import), then drag the bank asset into the level viewport. The
`Kya Level Preview` actor factory places an `AKyaLevelPreview` that points at the
dragged bank and builds its scene immediately, so editing does not require a
capture script or a manually configured preview. Dragging an actor class from the
Place Actors panel still works for an empty preview.

The plugin registers a `Kya Level` section in the editor's Tools menu and the
following console commands. They operate on the selected `KyaLevelPreview`
actors, or on every preview in the level when nothing is selected.

| Command | Action |
| --- | --- |
| `Kya.Validate` | Validate every bank of the level set and report which banks the writer cannot export, without writing anything. |
| `Kya.Export [dir]` | Validate, stage and publish the level's banks to `Saved/KyaExport/<BankName>.bnk` (or `dir`). Existing files are only replaced after every staged output validated. |
| `Kya.DiscoverSectors [0]` | Find sibling `SECT<digits>.BNK` files next to the main bank's source, reuse already-imported assets by source provenance, and import missing ones into `/Game/KyaLevels/<Level>`. `0` disables importing. Duplicate numeric IDs are reported as ambiguous, actor sector references with no file are reported as missing, destinations already occupied by a bank from a different source are rejected, and cancelling leaves a coherent partial result. |
| `Kya.CreateLevelSet` | Create or update the persisted `UKyaLevelSet` asset for the selected preview (`/Game/KyaLevels/<Bank>/<Bank>LevelSet`). It records the main bank, sector associations with their source filenames, the source root and an import revision; the preview and `Refresh Sector Discovery` keep it in sync. Descriptor sector IDs are authoritative (a manually associated bank does not have to match its `SECT<number>` filename), and an existing descriptor that belongs to a different main bank is never overwritten. |
| `Kya.SaveLevel` | Save the bank packages used by the preview (saving Unreal assets is separate from exporting game banks). |

The bank assets record `UAssetImportData` provenance, so they can be reimported
from the Content Browser. Reimport parses the new source into a temporary bank
first and only replaces the asset when parsing succeeds; a dirty asset asks before
discarding changes. `Kya.Export` writes `.bnk` files and never overwrites the
original source bank.


Actor and hierarchy handles can also be selected from scripts through the editor
selection API (`set_selected_level_actors` / `get_selected_level_actors`), which is
how the selection state is verified.

Path-point markers write their position back to the bank when moved (the editor move
callback and the `SetMarkerLocation`/`WriteBackToBank` functions share the same
code, including `Modify` for undo). `GetPathMarkerCount`/`GetPathMarker` expose the
markers for scripted checks.

Enable `Spawn Hierarchy Handles` to create one selectable handle per static
cluster node (a hierarchy's strip group) instead of the thousands of individual
strips. Each handle owns the hierarchy's rendered strips (one procedural section
per material, captured with the shared geometry removed so only one draw path
exists), so clicking any visible strip selects that hierarchy. Handles are
labelled with the bank name and hierarchy offset; Details reports the owning
bank/sector (or global scene mesh), source byte offset, serialized origin,
strip/triangle counts, world bounds and material names/hashes. A `Visible` toggle
hides the hierarchy in the preview and rebuilds it; hidden hierarchies show only
the fallback marker. Visibility and selection are editor inspection state
(`Hidden Hierarchies` on the preview) and do not modify bank bytes. The handle is
location-locked and resets translate/rotate/scale edits because the game stores no
independent hierarchy transform, so a movable-looking handle would imply export
support that does not exist.

The same handle participates in the editor-only Hide/Isolate actions: hiding or
isolating a hierarchy changes only the preview (and survives rebuilds), never bank
bytes. A transform attempt is reset immediately and the Details panel reports
`Transform reset: hierarchy transforms are not serialized`, so the handle cannot be
mistaken for a supported export edit.

Enable `Spawn Light Visualizers`, `Spawn Camera Visualizer`,
`Spawn Actor Link Visualizers` and `Spawn Trigger Visualizers` to add selectable
scene visualizers. Lights mirror the verified serialized light records and are
editable through the shared edit service (position, direction, colors, multiplier,
falloff/FOV and type); the colored sphere and arrow are explicitly approximate
shapes. The main camera edits the verified serialized position/look-at/up/FOV, with
an approximate frustum marker. Light edits are exported by patching the THGL
archive in place (record counts are fixed); the exporter rejects a count mismatch.
Actor links are read-only derived lines between an
actor and a resolved actor/waypoint reference, and trigger bounds are approximate
checkpoint-waypoint spheres. Zone bounds are **not** visualized: the schema stores
zone references without serialized geometry, and the preview's
`VisualizerLimitations` field states this. Each visualizer's Details panel labels
what is verified versus approximate, and read-only kinds reject transforms with a
status message.

**Level settings.** `Edit Level Settings` in the Kya Level menu (or
`Kya.LevelSettings`) opens a docked **Kya Level Settings** tab for the selected
preview. It edits the bank's environment, main camera, fog and scene configuration
as one authoring surface; applying writes the whole block through the shared edit
service (one undo step) and refreshes every preview. Invalid values (non-finite
camera/fog data, FOV outside 0–180, flags outside the serialized 32-bit range) are
rejected with the bank unchanged. Preview properties are categorised separately:
`Kya|Authoring` holds the bank/level-set association, while render comparison,
capture, selection and visualizer toggles live under `Kya|Diagnostics|*`.

Enable `Log Sector Diagnostics` on the preview actor to log bank entries, cluster
and hierarchy strip counts, material coverage, and bounds. For a camera-aligned
capture, save a game screenshot with `python tools/debug_socket/client.py screenshot --metadata out/screenshots/matched-camera.json`, set `KYA_CAMERA_METADATA` to
that JSON file, then run `Tools/capture_matched_view.py` in Unreal. The script
loads the sector IDs reported by the game, importing missing sector assets
without saving them. Set `KYA_CAPTURE_ACTORS=0` to inspect level geometry alone.
The script also copies the recorded camera's forward and up vectors into
`Sprite Camera Forward`/`Sprite Camera Up` so captured sprites face the camera.
`Tools/capture_scene_offscreen.py` renders the same metadata through an
off-screen `SceneCapture2D` and works while the editor is backgrounded. It sets
manual exposure and disables tone mapping, eye adaptation, colour grading and
bloom by default for colour comparisons; set
`KYA_CAPTURE_DISABLE_TONEMAPPER=0` to keep Unreal's tone curve. Both capture
scripts explicitly set the preview camera and render dimensions for background
following and fixed-size flares. Set `KYA_CAPTURE_SPRITES=0` in the off-screen
script to isolate the scene without sprites or the background flare.
`KYA_CAPTURE_VERIFY_OMISSIONS=1` additionally writes captures with flares disabled,
additive coefficients zeroed, an explicit 1,000,000-metre draw range, and forced
visible/occluded flare markers. These are diagnostic controls, not saved asset changes.
For a scenery-only reference, capture the game with
`python tools/debug_socket/client.py screenshot --hide-actors ...` after
`skip_cutscene`, then run the off-screen capture with `KYA_CAPTURE_ACTORS=0` and the
screenshot's own JSON as `KYA_CAMERA_METADATA`. The free camera keeps the active
game camera's FOV, so skip the intro cutscene first; otherwise the reference is
letterboxed and zoomed. Native actor shadows and the HUD are drawn even with actor
draws hidden, so they remain in that reference.
If the loaded bank has no imported actors (a stale asset), the script reimports `LEVEL.BNK`
into `/Game/__KyaCapture` for the capture.
The script removes existing `KyaLevelPreview` actors from the loaded map before it spawns the
new preview, so previews saved in a map cannot render on top and hide material changes. It pilots
an unsaved camera with the recorded horizontal FOV and capture aspect ratio, then takes a
high-resolution screenshot. Run it from the editor's
Python console (or launch with `-ExecCmds="py <script path>"`); the
`-ExecutePythonScript` launch mode exits before its delayed screenshot. Captures
still differ in rendering, resolution and dynamic scene state. Run
`python Tools/compare_capture_colors.py <game.png> <capture.png> [...]` to print
whole-frame mean RGB, mean absolute error and RMS against the game capture.

Each sector background uses its own CSBS background flags; the common background
uses the scene flags. Sector 1 is fixed even though Level 1's common flag is set.
When a background flag enables camera following, the preview updates
the relevant background hierarchy vertices using the current level-editing
viewport camera. A fixed-background hierarchy remains in place. This uses one
editor camera for the preview, so simultaneous viewports do not each get an
independent background transform.
`SetPreviewCamera` supplies an explicit camera for captures and prevents the
editor viewport from overwriting it; spawn a new preview to resume viewport following.

`Preview Base Sector` selects the sector's native clipping distance (or defaults
to the first sector bank). Cluster objects and strips use native sphere tests,
including the far-only parent intersection exception, and distance-based hierarchy
LOD selection. This differs from setting an Unreal per-pixel far plane. Background
geometry keeps its independent native range. Generated textures preserve the
material's clamp/repeat addressing.

Additive and source-alpha meshes match encoded-colour blending against Unreal's opaque scene
colour. Overlapping translucent surfaces still use Unreal's translucency ordering
and scene-colour copy, so arbitrary GS blend equations and layered translucency
are not fully reproduced. The fixed flare uses the native main scene's default
depth projection; custom native near/far or depth-range settings are not imported.
`Native Surface Size` controls GS flare sizing independently of framebuffer
resolution: 512 by 512 for PAL, or 512 by 448 for NTSC.

## Validation

Set `KYA_TEST_BANK` to a bank path before running `Kya.Bank.LevelOneRoundTrip`;
without it the test only runs synthetic fixtures and logs a skip warning:

```powershell
$env:KYA_TEST_BANK = 'G:\repos\Kya\assets\CDEURO\LEVEL\LEVEL_1\LEVEL.BNK'
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'G:\repos\Kya\out\UnrealBankTest\BankTest.uproject' -unattended -nop4 -NullRHI `
  -nosplash -ExecCmds='Automation RunTests Kya.Bank.LevelOneRoundTrip' `
  '-TestExit=Automation Test Queue Empty' -log
```

The test project's `Plugins/KyaLevelEditor` must be a junction to this plugin
directory, not a copied plugin. A stale copy can run old test binaries while the
repository builds successfully. Verify the link before trusting test results:

```powershell
Get-Item G:\repos\Kya\out\UnrealBankTest\Plugins\KyaLevelEditor |
  Format-List FullName,LinkType,Target
```

`Kya.Bank.SmallBankRoundTrip` builds a small but complete bank in memory
(`FKyaSmallBankBuilder` in `KyaLevelBankTests.cpp`): header and entry/type
tables, a scene entry with one `INST` moving-platform actor chunk, and a scene
configuration entry. The actor carries a full class-0x0d prefix with one
two-actor no-friction zone plus a platform details block, so optional layouts can
be exercised without LEVEL_1. The test checks that the unedited export is
byte-identical, that no-friction ref/actor, weight and flag edits reimport with
exactly those bytes changed, and that the exporter rejects a changed no-friction
zone count. The test also writes the constructed asset to
`<project>/Saved/KyaSmallBank.BNK` (864 bytes) so it can be imported for manual
inspection; importing it yields four entries and one class-0x0d actor named
`PLATFORM` with the no-friction zone and full typed class data. Run it with
`-ExecCmds="Automation RunTests Kya.Bank.SmallBankRoundTrip"`.

`Tools/inventory_actor_classes.py` lists remaining raw Create payloads by actor
class. `Tools/inventory_platform_details.py` reports moving-platform coverage:
the carried-actor (bridge) stream with its target placements, zone refs,
no-friction zones, camera/notification records, sounds and the remaining raw
class-data size. Level 1 currently reports 166 platform actors with 9 carried-actor
streams (19 refs, all resolving to placements), 3 zone refs, 54 camera records and
no raw class-data suffix.

## Installation

The test project at `F:\Unreal\KyaEditorTest` loads this plugin through a directory junction:

```powershell
New-Item -ItemType Directory -Force F:\Unreal\KyaEditorTest\Plugins
cmd /c mklink /J F:\Unreal\KyaEditorTest\Plugins\KyaLevelEditor G:\repos\Kya\unreal\KyaLevelEditor
```

Edits in the repository are picked up directly by the project. Remove the junction with `cmd /c rmdir F:\Unreal\KyaEditorTest\Plugins\KyaLevelEditor` (this does not delete the source).
