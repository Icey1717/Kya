# Agent Notes

This repository is an ongoing Windows-focused decompilation and PC port of *Kya: Dark Lineage* for PS2. The decompiled game code lives mostly under `src/`; the Windows port, Vulkan renderer, debug menu, and tooling live under `port/`.

## Build And Test

- Main Windows build:
  - `cmake --preset x64-debug`
  - `cmake --build out/build/x64-debug`
- Useful smoke test:
  - `bin/WIN/KyaPortTest.exe`
  - or `ctest --test-dir out/build/x64-debug`
- Requirements include Visual Studio 2022 Clang tools and the Vulkan SDK, including GLM headers.
- `linux-debug` is for the PS2 GCC toolchain, not the normal PC build.
- Test builds define `HEADLESS` for the renderer.

## Capturing The Running App

- Prefer the debug socket for game screenshots, including while minimized. In the **Debug Socket** debug menu, enable **Enable Debug Socket**; for minimized capture, also enable **Rendering > Pipeline > Render When Minimized**. These settings persist in `settings.json` in the game's working directory.
- From the repository root, run `python tools/debug_socket/client.py status` to check the connection, frame number, render dimensions, and `offscreen` state.
- Run `python tools/debug_socket/client.py screenshot` to save a full-resolution game PNG to `out/screenshots/latest.png`, then inspect it with `view_image`. Use `--output out/screenshots/<name>.png` to retain a named capture.
- The default endpoint is `127.0.0.1:18438`. For multiple instances, set **Debug Socket Port** in each game's debug menu and pass `--port <port>` to the client. Requests require the game frame loop to run; a stopped debugger or blocking load can cause a timeout. See [debug socket commands and protocol](tools/debug_socket/README.md).
- Socket screenshots exclude the debug UI. To inspect the debug UI, use desktop capture:
- Run `& ./tools/capture-window.ps1` in PowerShell to capture the main Kya window, including the debug UI, to `out/screenshots/latest.png`, then inspect it with `view_image`.
- Desktop access may require running the capture command outside the sandbox. Kya must be running and not minimized.
- Use `-ProcessId <pid>` when multiple builds are running. Capture uses passive screen copying; the window must be fully visible. Avoid `-PrintWindow` for Vulkan because it can cause visible flashing. See `tools/capture-window.md`.

## Repository Shape

- `src/b-witch/`: decompiled main game code. Preserve original layout-sensitive structure, naming, and manual memory model.
- `src/EdenLib/`: decompiled Eden engine libraries, with some submodules.
- `src/port/`: PS2 hardware abstraction shims and pointer conversion helpers used by decompiled code.
- `port/Windows/Renderer/`: Vulkan backend, texture cache, framebuffers, pipelines, shaders, PCSX2-derived helpers, SPIRV-Reflect.
- `port/DebugMenu/`: Dear ImGui-based debug overlay.
- `port/KyaMesh` and `port/KyaTexture`: separate submodules; commit inside them first, then update the parent submodule pointer.
- `logs/` contains generated logs and may be deleted or regenerated during local runs.

## Coding Expectations

- Compare every change to `src/b-witch/` against the original PS2 code using the Ghidra MCP. Check the relevant decompilation and disassembly before changing behavior, and record the function addresses used for comparison.
- In decompiled `src/` code, avoid modernizing for style. Keep layout and offset-sensitive fields intact, including `field_0xNN`, `undefined*`, original-style names, and manual allocation.
- In `port/` code, prefer existing local patterns. Vulkan renderer code currently uses exceptions for hard Vulkan setup failures even though other port layers often prefer logged early returns.
- Use the pointer conversion macros (`STORE_POINTER`, `LOAD_POINTER`, related helpers) for serialized PS2 pointer fields.
- Do not replace custom PS2 math/data-layout types with GLM in decompiled code.
- Keep Tracy macros such as `ZONE_SCOPED`, `FRAME_MARK`, `TRACK_ALLOC`, and `TRACK_FREE`.
- When assigning recovered names, use PascalCase for functions and camelCase for all other symbols.
- Prefer `rg`/`rg --files` for searching.
- Use `apply_patch` for manual edits.

## Submodules And Generated Files

- Check nested repo status when parent status shows lowercase `m` on a submodule:
  - `git status --short` in the parent
  - `git -C port/KyaMesh status --short`
  - `git -C port/KyaTexture status --short`
- After committing inside a submodule, stage the submodule path in the parent to record the new commit.
- Avoid committing unrelated generated artifacts unless the user explicitly wants them.
