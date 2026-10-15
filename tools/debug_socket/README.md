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
python tools/debug_socket/client.py screenshot --output out/screenshots/electrolla.png
```

The default screenshot destination is `out/screenshots/latest.png`. Screenshots
are full-resolution PNGs from the game's render buffer, without the debug UI.
Enable **Rendering > Pipeline > Render When Minimized** to receive fresh frames
and service requests while minimized. Requests require the game frame loop to
run; a stopped debugger or blocking load can cause a client timeout.

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
