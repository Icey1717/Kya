# Kya Level Editor

This plugin is under development. A `.bnk` file can be imported as an asset;
the original bytes are retained. The initial scene configuration, collision bank
values, animation allocation count, actor class allocations, and the scene's
initial fog, clipping, mipmap settings and basic main camera fields can be edited on
the asset and exported to an uncompressed bank. Array sizes are fixed. The export path is
experimental and has not yet been verified in the game. Assets imported before
the scene and actor Create fields were expanded must be reimported to populate
the new fixed-size arrays before export. An experimental `KyaLevelPreview` actor
displays background meshes with hierarchy transforms and
supports main scene static cluster strips (level one's main cluster is empty).
The editor generates a basic unlit texture material with vertex-colour modulation automatically. To override it,
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
the selected actor. It applies the initial class rotation rules and attempts material replacements
from the default texture bank. Animation and some geometry and texture formats are not supported,
so it should be treated as approximate.
`Show All Actor Meshes` applies the same preview to every actor placement. It can
increase editor load time and includes actors that the game may initially hide.

Enable `Log Sector Diagnostics` on the preview actor to log bank entries, cluster
and hierarchy strip counts, material coverage, and bounds. For a camera-aligned
capture, save a game screenshot with `python tools/debug_socket/client.py screenshot --metadata out/screenshots/matched-camera.json`, set `KYA_CAMERA_METADATA` to
that JSON file, then run `Tools/capture_matched_view.py` in Unreal. The script
loads the sector IDs reported by the game, importing missing sector assets
without saving them. Set `KYA_CAPTURE_ACTORS=0` to inspect level geometry alone.
The script pilots an unsaved camera with the recorded horizontal FOV and capture
aspect ratio, then takes a high-resolution screenshot. Run it from the editor's
Python console (or launch with `-ExecCmds="py <script path>"`); the
`-ExecutePythonScript` launch mode exits before its delayed screenshot. Captures
still differ in rendering, resolution and dynamic scene state.

When the scene background flag enables camera following, the preview updates
the relevant background hierarchy vertices using the current level-editing
viewport camera. A fixed-background hierarchy remains in place. This uses one
editor camera for the preview, so simultaneous viewports do not each get an
independent background transform.

## Installation

The test project at `F:\Unreal\KyaEditorTest` loads this plugin through a directory junction:

```powershell
New-Item -ItemType Directory -Force F:\Unreal\KyaEditorTest\Plugins
cmd /c mklink /J F:\Unreal\KyaEditorTest\Plugins\KyaLevelEditor G:\repos\Kya\unreal\KyaLevelEditor
```

Edits in the repository are picked up directly by the project. Remove the junction with `cmd /c rmdir F:\Unreal\KyaEditorTest\Plugins\KyaLevelEditor` (this does not delete the source).
