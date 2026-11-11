# Kya debug socket

Open the **Debug Socket** debug menu and enable **Enable Debug Socket**. The
setting is saved in `bin/WIN/settings.json` when running from `bin/WIN`.
The default address is `127.0.0.1:18438`; change **Debug Socket Port** for
multiple game instances. A bind failure is shown in the menu.

The panel shows active connections and the most recent 128 closed connections,
newest first. Each row includes the command, request ID, result, handler time,
total elapsed time, and response transfer progress. Hover a result for the peer,
frame number, and full error message. Use the text filter, **Errors only**, or
**Clear history** to inspect activity. Session counters survive clearing history
and restarting the listener; history is kept in memory and stores no image bytes.

From the repository root:

```powershell
python tools/debug_socket/client.py status
python tools/debug_socket/client.py screenshot
python tools/debug_socket/client.py screenshot --target mesh --output out/screenshots/mesh.png
python tools/debug_socket/client.py screenshot --output out/screenshots/electrolla.png
python tools/debug_socket/client.py screenshot --output out/screenshots/reference.png --metadata out/screenshots/reference.json
```

The default screenshot destination is `out/screenshots/latest.png`. Screenshots
are full-resolution PNGs from the game's render buffer, without the debug UI.
Enable **Rendering > Pipeline > Render When Minimized** to receive fresh frames
and service requests while minimized. Requests require the game frame loop to
run; a stopped debugger or blocking load can cause a client timeout.

Use `--target mesh` to capture the open mesh preview at 1024 × 768, excluding
its controls. Select a mesh first; an inactive viewer returns an error. Mesh
captures report their own dimensions and omit the game camera metadata. The
viewer automatically frames each new selection; **Frame mesh** resets the zoom
and angle, with **Zoom** and **View angle** controls for inspection.

Mesh previews use the native scene fragment shader, GS blend equations, alpha
test/failure replay, and material texture sampling. Each strip keeps its own
blend state, and all material layers are previewed. Opening a draw from the
draw inspector also retains its captured alpha and depth settings and object
alpha. The preview camera and unposed mesh geometry remain independent of the
game scene.

To check actual Vulkan preview pixels (run alone from `bin/WIN`):

```powershell
.\KyaPortTest.exe --gtest_also_run_disabled_tests --gtest_filter=MeshViewerRendering.*
```

This checks colour scaling, translucent blending, alpha rejection and all four
alpha-failure modes, per-mesh blend isolation, and captured object alpha. It is
disabled in the default headless suite because it requires a Vulkan desktop.

The socket lives in the DebugMenu library. It polls nonblocking Winsock sockets
on the main thread after frame submission, including minimized frames. Network
work per frame is bounded; at most one request is handled per frame. Screenshot
requests perform a synchronous GPU readback and PNG encode and can briefly
increase frame time. Up to four clients are accepted; inactive connections close
after 15 seconds. The server binds only to IPv4 loopback and exposes no arbitrary
code execution or filesystem writes. Local programs can request screenshots
while it is enabled.

## Protocol version 1

One request per TCP connection: send a UTF-8 JSON object followed by a newline.
Requests are limited to 8192 bytes, including the newline. Optional `id` is echoed.

```json
{"command":"status","id":1}
{"command":"screenshot","id":2}
{"command":"screenshot","target":"mesh","id":3}
```

The response starts with a UTF-8 JSON header and newline. `bytes` gives the exact
number of binary payload bytes following the newline. The connection closes
after sending the complete response. Do not assume socket reads align with
the header or payload boundaries.

Status returns `ok`, `id`, `protocol`, `pid`, `frame`, `offscreen`, `width`,
`height`, and `bytes: 0`. `frame` identifies the frame-submission callback count
(not the game's simulation tick). Screenshots return the same metadata plus
`encoding: "png"` and a PNG payload. Errors return `ok: false`, `error`, and
`bytes: 0` (and `id` if it could be parsed).

When the camera manager is available, responses also include `camera` with the
game-space position, forward and up vectors, and horizontal FOV in radians. The camera
metadata and screenshot are collected in the same frame callback. Consumers
should still verify projection, aspect ratio, and roll when matching views.
When a scene is active, `base_sector` and `loaded_sectors` identify the game
sector indices, load stages, and installed state at capture time. Sector index N uses `SECTN.BNK`.

The Python client has no external dependencies and can also be imported through
its `request()` function. This protocol can be wrapped in MCP later.

## Verification

Normal socket regression tests run in `ctest`. The standalone Vulkan test checks
real socket screenshots while minimized, decodes their PNG pixels, and restores
window presentation:

```powershell
Push-Location bin/WIN
./KyaPortTest.exe --gtest_filter=MinimizedRendering.* --gtest_also_run_disabled_tests
Pop-Location
```

Run that test alone; it requires an interactive desktop and Vulkan.

## Matched actor scene captures

Once the listener is enabled, the native workflow can run through the socket.
Pause and request a draw capture, then poll `status` until
`draw_capture_pending` is false before listing actors:

```powershell
python tools/debug_socket/client.py pause
python tools/debug_socket/client.py capture_draws
python tools/debug_socket/client.py status
python tools/debug_socket/client.py actors
```

Use the actor's returned `index` in `focus_actor --actor <index>` if you want the
debug camera to frame it. Apply `actor_override --actor <index> --mode bind_pose`
or `--mode texture_only`. Poll `status` until `actor_override_matches` is greater
than zero before requesting a screenshot. Run `step` while paused to advance one
frame and poll until `simulation_paused` is true again. To restore gameplay,
run `actor_override --mode none`, `camera_reset` if you used focus, and `resume`.
Use `--mode isolate_bind_pose` to suppress other scene draws while preserving
the selected actor's native lighting.

The JSON protocol uses the same command names, with integer `actor` and string
`mode` fields where applicable. The index returned by `actors` is a native
runtime index. Unreal's texture-only setting takes the index in the imported bank.

In the game's debug toolbar, use **Pause** to set the simulation time scale to
zero. **Step Frame** advances one frame at the previous time scale and pauses
again. The game keeps rendering and servicing screenshot requests. Systems
that update independently of the simulation time scale can still advance.

In **Draw Inspector**, capture a frame, select one of the actor's draws, and
choose **Actor bind pose**. This bypasses rigid skinning for that actor's scene
draws. Use **Actor bind pose + texture only** to also bypass RGB modulation on
its textured draws. Both modes retain the scene camera, material bindings, and
alpha handling. The **Draw overrides** window shows the number of matching
draws and lets you clear the override.

After the matching draw count is visible, save the scene and its camera data
from the repository root:

```powershell
python tools/debug_socket/client.py screenshot --output out/screenshots/actor-native.png --metadata out/screenshots/actor-native.json
```

Use that metadata with the Unreal plugin's `Tools/capture_matched_view.py`
via `KYA_CAMERA_METADATA`. Set `KYA_CAPTURE_TEXTURE_ONLY_ACTOR` to the Unreal
bank actor index for the texture-only comparison. The default is `-1`, which
keeps normal vertex modulation on all actors.

### How the bind pose works

The native mesh already contains positions and normals in its stored rest pose.
The vertex shader normally applies the current bone matrix to both when the
strip has rigid animation enabled (`stripFlags & 0x10000`). The draw override
matches the runtime actor that owns each submission and clears that bit on the
matching draw's copy of the flags. The shader then uses the stored positions
and normals directly. The draw keeps its object-to-world matrix, material,
alpha handling, and resolved scene light packet.

`isolate_bind_pose` also suppresses other scene draws. `texture_only` uses the
same rest pose and samples texture RGB directly on textured draws, which helps
separate texture and display color differences from vertex lighting.

For an isolated Unreal comparison, apply `isolate_bind_pose`, wait for
`actor_override_matches` to become positive, and request a fresh `capture_draws`.
Once it finishes, save the actor draw data and screenshot as close together as
possible. Replace `439` with the runtime index returned by `actors`.

```powershell
python tools/debug_socket/client.py actor_lighting --actor 439 --metadata out/screenshots/actor-draws.json
python tools/debug_socket/client.py screenshot --output out/screenshots/actor-native.png --metadata out/screenshots/actor-native.json
python -c "import json; from pathlib import Path; p = Path('out/screenshots/actor-native.json'); m = json.loads(p.read_text(encoding='utf-8')); m['actor_capture'] = json.loads(Path('out/screenshots/actor-draws.json').read_text(encoding='utf-8')); p.write_text(json.dumps(m, indent=2), encoding='utf-8')"
```

The screenshot and draw trace are separate requests. Check the screenshot's
`frame` against `actor_capture.draw_capture_frame` when the actor or camera can
move during the capture, such as in a cutscene.
