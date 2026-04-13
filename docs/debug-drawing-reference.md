# Debug Drawing Reference

This reference covers the debug shape APIs currently available for drawing world-space debug information on screen.

---

## Primary API: `Renderer::Native::DebugShapes`

All shape calls **queue geometry for the current frame**. The renderer consumes that queue later in the debug pass.

### Wireframe / line primitives

| Function | Use |
|---|---|
| `AddLine(start, end, color)` / float overload | Draw a single world-space line segment |
| `AddOBB(mat, hw, hh, hd, r, g, b, a)` | Draw oriented box edges |
| `AddCylinder(mat, r, g, b, a)` | Draw unit cylinder transformed by `mat` |
| `AddCone(mat, r, g, b, a)` | Draw unit cone transformed by `mat` |
| `AddOpenTopBox(mat, r, g, b, a)` | Draw open-top box / plane+walls primitive |
| `AddHalfSpace(mat, r, g, b, a)` | Draw half-space marker (plane cross + up arrow) |
| `AddSphere(center, radius, color)` / float overload | Draw sphere wireframe (3 great circles) |

### Filled primitives (alpha blended triangles)

| Function | Use |
|---|---|
| `AddFilledOBB(mat, hw, hh, hd, r, g, b, a)` | Filled oriented box |
| `AddFilledCylinder(mat, r, g, b, a)` | Filled cylinder |
| `AddFilledCone(mat, r, g, b, a)` | Filled cone |
| `AddFilledSphere(cx, cy, cz, radius, r, g, b, a)` | Filled sphere |

### Utility / lifecycle

| Function | Use |
|---|---|
| `GetLineCount()` | Number of debug lines queued this frame |
| `Setup()`, `ResetFrame()` | Initialize buffers and clear per-frame queues |
| `SetInitialViewProjection(view, proj)` | Sets matrices used when rendering debug geometry |
| `SetupDedicatedPass(...)`, `SaveDepth(...)`, `RecordDedicatedPass(...)`, `DestroyDedicatedPass()` | Dedicated end-of-frame debug pass with depth testing |

---

## Matrix convention for shape functions

Functions taking `const float* mat` expect an **`edF32MATRIX4 raw[]` local-to-world transform** in this layout:

- row X: `raw[0..3]`
- row Y: `raw[4..7]`
- row Z: `raw[8..11]`
- translation row: `raw[12..15]`

If you have a world-to-local matrix, invert it before passing to debug shapes (see `DebugEvent.cpp` pattern).

---

## Path helpers: `Debug::Path`

`port\DebugMenu\include\DebugPathDrawing.h` provides convenience wrappers for spline/path debug:

| Function | What it does |
|---|---|
| `DrawPathFollow(const CPathFollow*, bool bIsActive)` | Draws segment lines + node spheres |
| `DrawPathFollowReaderAbsolute(const CPathFollowReaderAbsolute&, bool)` | Draws `reader.pPathFollow` |
| `ShowPathFollowReaderAbsoluteInfo(...)` | ImGui data display (no shape drawing) |
| `ShowPathPlaneInfo(...)` | ImGui data display (no shape drawing) |

Active paths use a brighter yellow-ish color; inactive paths use a dimmer blue-ish color.

---

## Existing usage examples

### 1. Moving platform trajectory debug

`DebugActorMovingPlatform.cpp` uses a checkbox (`Draw Path`) and calls:

```cpp
Path::DrawPathFollowReaderAbsolute(pTraj->pathFollowReaderAbs, true);
```

### 2. Event zone primitive visualization

`DebugEvent.cpp` maps zone primitive types to debug shapes:

| Zone type | Draw call |
|---|---|
| `0` | `AddHalfSpace` |
| `1` | `AddOpenTopBox` |
| `2` | `AddOBB` / `AddFilledOBB` |
| `3` | `AddCylinder` / `AddFilledCylinder` |
| `4` | `AddCone` / `AddFilledCone` |
| `5` | `AddSphere` / `AddFilledSphere` |

### 3. Wolfen come-back target debug

`DebugActorWolfen.cpp` now exposes behaviour-specific sections for Wolfen comeback logic, plus two draw toggles:

- `Draw Come Back Position` — draws the current behaviour's come-back target as a sphere.
- `Draw Come Back Link` — draws a line from the Wolfen's current location to that target.

Each loaded comeback-capable behaviour has its own section with position/angle/state details. This is useful for verifying patrol/return behaviour when switching between watchdog, track, guard-area, and DCA variants.

---

## Recommended pattern for adding new on-screen debug info

1. Gate drawing behind a debug toggle (ImGui checkbox / debug setting).
2. Build world-space points or a local-to-world matrix for the thing you want to visualize.
3. Queue one or more `Renderer::Native::DebugShapes::*` calls per frame while the toggle is enabled.
4. Optionally expose `GetLineCount()` in your panel when tuning/debugging volume.

Minimal example:

```cpp
if (gShowMyDebug) {
    Renderer::Native::DebugShapes::AddLine(x1, y1, z1, x2, y2, z2, 1.0f, 0.2f, 0.2f, 1.0f);
    Renderer::Native::DebugShapes::AddSphere(cx, cy, cz, 0.25f, 0.2f, 1.0f, 0.2f, 1.0f);
}
```

