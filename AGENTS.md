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

## Repository Shape

- `src/b-witch/`: decompiled main game code. Preserve original layout-sensitive structure, naming, and manual memory model.
- `src/EdenLib/`: decompiled Eden engine libraries, with some submodules.
- `src/port/`: PS2 hardware abstraction shims and pointer conversion helpers used by decompiled code.
- `port/Windows/Renderer/`: Vulkan backend, texture cache, framebuffers, pipelines, shaders, PCSX2-derived helpers, SPIRV-Reflect.
- `port/DebugMenu/`: Dear ImGui-based debug overlay.
- `port/KyaMesh` and `port/KyaTexture`: separate submodules; commit inside them first, then update the parent submodule pointer.
- `logs/` contains generated logs and may be deleted or regenerated during local runs.

## Coding Expectations

- In decompiled `src/` code, avoid modernizing for style. Keep layout and offset-sensitive fields intact, including `field_0xNN`, `undefined*`, original-style names, and manual allocation.
- In `port/` code, prefer existing local patterns. Vulkan renderer code currently uses exceptions for hard Vulkan setup failures even though other port layers often prefer logged early returns.
- Use the pointer conversion macros (`STORE_POINTER`, `LOAD_POINTER`, related helpers) for serialized PS2 pointer fields.
- Do not replace custom PS2 math/data-layout types with GLM in decompiled code.
- Keep Tracy macros such as `ZONE_SCOPED`, `FRAME_MARK`, `TRACK_ALLOC`, and `TRACK_FREE`.
- Prefer `rg`/`rg --files` for searching.
- Use `apply_patch` for manual edits.

## Submodules And Generated Files

- Check nested repo status when parent status shows lowercase `m` on a submodule:
  - `git status --short` in the parent
  - `git -C port/KyaMesh status --short`
  - `git -C port/KyaTexture status --short`
- After committing inside a submodule, stage the submodule path in the parent to record the new commit.
- Avoid committing unrelated generated artifacts unless the user explicitly wants them.