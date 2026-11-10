# Kya Level Editor

This plugin is under development. A `.bnk` file can be imported as an asset;
the original bytes are retained. The initial scene configuration, collision bank
values, animation allocation count, actor class allocations, and the scene's
initial fog, clipping, mipmap settings and basic main camera fields can be edited on
the asset and exported to an uncompressed bank. Array sizes are fixed. The export path is
experimental and has not yet been verified in the game. Assets imported before
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
actor placement. Strips without stored normals keep their baked vertex colours. `Show Background Meshes` and
`Show Cluster Meshes` isolate those layers for inspection. Level geometry never alpha-blends: all
level 1 cluster and background strips use disabled blend (`ABE=0`), so opaque/masked
materials are faithful for this content.
The editor generates a basic unlit texture material with vertex-colour modulation automatically. By default it
combines texture and vertex colours in the game's encoded colour space and then decodes the product for Unreal's
linear output; this reproduces the game's vertex shading much more closely than Unreal's normal sRGB sampling.
Disable `Game Colour Space` only to compare against Unreal's own conversion. To override the generated material,
assign `Texture Material` a material with a Texture2D parameter named `KyaTexture`.
Supported
indexed and direct-colour textures on main and sector backgrounds are decoded automatically when supported; unsupported
materials and unsupported cluster geometry remain untextured and use their vertex colours. Supported cluster strips resolve sector and common textures. Sprites and some scene geometry
are missing, and the preview is not a faithful level render. The runtime module is separated from editor tooling so that level
data and rendering can also be used by a future playable Unreal project.

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

Enable `Log Sector Diagnostics` on the preview actor to log bank entries, cluster
and hierarchy strip counts, material coverage, and bounds. For a camera-aligned
capture, save a game screenshot with `python tools/debug_socket/client.py screenshot --metadata out/screenshots/matched-camera.json`, set `KYA_CAMERA_METADATA` to
that JSON file, then run `Tools/capture_matched_view.py` in Unreal. The script
loads the sector IDs reported by the game, importing missing sector assets
without saving them. Set `KYA_CAPTURE_ACTORS=0` to inspect level geometry alone.
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

When the scene background flag enables camera following, the preview updates
the relevant background hierarchy vertices using the current level-editing
viewport camera. A fixed-background hierarchy remains in place. This uses one
editor camera for the preview, so simultaneous viewports do not each get an
independent background transform.

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

`Tools/inventory_actor_classes.py` lists remaining raw Create payloads by actor
class. `Tools/inventory_platform_details.py` reports moving-platform camera,
sound, and class-data coverage.

## Installation

The test project at `F:\Unreal\KyaEditorTest` loads this plugin through a directory junction:

```powershell
New-Item -ItemType Directory -Force F:\Unreal\KyaEditorTest\Plugins
cmd /c mklink /J F:\Unreal\KyaEditorTest\Plugins\KyaLevelEditor G:\repos\Kya\unreal\KyaLevelEditor
```

Edits in the repository are picked up directly by the project. Remove the junction with `cmd /c rmdir F:\Unreal\KyaEditorTest\Plugins\KyaLevelEditor` (this does not delete the source).
