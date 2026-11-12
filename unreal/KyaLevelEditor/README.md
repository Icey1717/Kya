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
sector, with the asset path, the failure reason and any warnings. Sector
discovery shows a cancellable progress dialog and leaves a coherent partial result
when cancelled.

The handle's Details panel shows the bank record itself: one row per editable
`FKyaActorPlacement` property, grouped by the schema's own categories, including
nested fixed-size arrays, class-specific records and behaviour payloads. Edits are
applied one top-level property at a time through the shared edit service, so a
nested field or array element change writes only that property and produces one
undo step. The old hand-written mirror fields remain on the actor for script
compatibility but are hidden from Details. Non-editable fields (such as the
serialized record index) stay read-only, and the property handles evaluate the
schema's existing presence flags and edit conditions. Reference pickers, named
behaviour labels, mixed-value multi-editing and reset-to-imported actions are not
implemented yet.

Waypoints and path points have the same treatment. `Spawn Waypoint Handles` creates
one transient `AKyaSpatialRecordHandle` per waypoint (position, stored rotation and
`CWayPoint` parameter triple) and suppresses the diagnostic waypoint arrows while
enabled. `Spawn Path Point Handles` creates one handle per point of every path,
labelled `Path <n> Point <m>`, so spatial records can be found without setting
`Preview Path Index`; the spline guide still follows that index. Both handle kinds
have bank + index + generation identity, write through the same edit service as the
old arrow markers (which now call the service too, so both routes agree), and
preserve unedited values and undo grouping. Stored waypoint rotation is edited
numerically: the ported source stores an Euler triple in radians consumed by
`SetVectorFromAngles`/`InitPositionRotation` (for example `ActorMicken` and actor
spawn parameters), and orientation is deliberately not drawn because the arrow
convention is not verified. Path timing, lead-in and loop visualization are not
implemented.

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
strips. Handles are labelled with the bank name and hierarchy offset and expose
sector, hierarchy offset, strip/triangle counts and a `Visible` toggle that hides
the hierarchy in the preview and rebuilds it. The toggle is an editor inspection
state (`Hidden Hierarchies` on the preview); it does not modify bank bytes.

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
