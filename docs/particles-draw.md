# Particle Drawing — `edPartDrawShaper`

`edPartDrawShaper` is the single function responsible for rendering all particles in the game.
It receives a pre-simulated batch of particles and writes them into the display list.

---

## Call signature

```cpp
void edPartDrawShaper(
    float alpha,                          // global opacity multiplier
    _ed_particle_group*        pGroup,    // particle group (bounding sphere, count)
    _ed_particle_shaper_param* pDrawData, // per-emitter render settings
    _ed_particle*              pParticle, // array of particle state
    _ed_particle_vectors*      pVector,   // array of velocity vectors
    edF32VECTOR4*              pRawVectors, // array of world-space positions
    int startIndex, int endIndex          // slice of the arrays to draw
);
```

---

## Overview

```
edPartDrawShaper
 ├── 1. Coarse frustum cull  (group bounding sphere vs camera frustum)
 │       → sets pCurParticle->visible per particle
 ├── 2. Sorting              (optional, depth or age order)
 ├── 3. Dispatch on drawMode
 │       ├── Mode 1  Billboard     (camera-facing quads)   ──┐
 │       ├── Mode 4  Ground-plane  (world-XZ quads)        ──┤ per-particle loop
 │       ├── Mode 2  Patchable display list                   │
 │       └── Mode 3  Hierarchy (3D mesh per particle)         │
 └── 4. Per-particle loop  (Modes 1 & 4)
         ├── UV transform  (affine Mat2x3 pipeline)
         ├── Colour        (ParticleColorA / B / C)
         └── Vertex emit   (edDListVertex4f)
```

---

## Step 1 — Frustum culling

The system checks the group's bounding sphere against the camera frustum first. The result
is one of three states:

| `iVar15` | Meaning |
|---|---|
| `0` | Fully outside — skip everything |
| `1` | Partially inside — run per-particle frustum test |
| `2` | Fully inside — mark all live particles visible |

For the partial case, each particle is tested individually:
- Transform position into camera space via `particleToCameraMatrix`.
- Check the screen-space half-size (`yScale * sizeByte * gParticleSizeScale`) against the
  four view-frustum plane normals stored in `aCameraPoints[4]`.
- `pCurParticle->visible` is set to `1` if the particle passes.

---

## Step 2 — Sorting

Controlled by `pDrawData->sortFlags`:

| Bits | Effect |
|---|---|
| `0x01 / 0x02` | Age sort (ascending or descending) |
| `0x04 / 0x08` | Depth sort (nearest-first or farthest-first) |
| `0x10` | Screen-coverage budget cap (`maxScreenCoverage`) |

Sorted particles are stored in two parallel static arrays:
- `table_sorted_indices$1918[256]` — particle indices in draw order
- `table_sorted_values$1919[256]` — sort keys (age or camera-Z)

When sorting is active the main draw loop iterates `table_sorted_indices` instead of the
raw particle array.

---

## Step 3 — Draw modes

### Mode 1 — Billboard

Camera-facing quads. `cameraRightAxis` and `cameraUpAxis` are taken from
`particleToCameraMatrix.rowX / rowY`. The display list format tag is `0xb` (quad).
One draw call per particle (hardware expands width/height into billboard corners).

### Mode 4 — Ground-plane

World-XZ quads (lying flat on the ground). `cameraRightAxis = gF32Vector4UnitX`,
`cameraUpAxis = {0,0,-1,0}`. Each particle emits 4 explicit vertices rather than a
width/height pair. Display list format tag is `0x8`.

### Mode 2 — Patchable display list

A pre-built display list is modified in-place. `edDListPatchableInfo` returns base pointers
to embedded position, colour, and size buffers. The loop patches those buffers:
- `field_0x6c` → patch world positions
- `field_0x6e` → patch colours
- `field_0x6f` → patch scale values

### Mode 3 — Hierarchy

Each particle drives a 3D mesh node (`edNODE` / `ed_g3d_hierarchy`). The particle's world
position and uniform scale (`yScale * sizeByte * gParticleSizeScale`) are written directly
into the node's `transformA` matrix. The node is then submitted through the normal 3D path.

---

## Step 4 — Per-particle loop (Modes 1 & 4)

For each visible particle:

### 4a — UV transform pipeline

Controlled by `pDrawData->uvFlags` (`PARTICLE_SHAPER_FLAG_*`):

```
UV_TRANSFORM set?
 └── SPRITE_SHEET set?
      NO  → uvQuad = gDefaultQuad   (identity: maps (0,0)–(1,1))
      YES → build rotation quad:
             RAND_ROT  → baseRot = (seed & 0xffff) * baseAngle / 65535
             else      → baseRot = pDrawData->baseAngle

             VELOCITY_ROT → ParticleA()   (velocity-aligned rotation)
             else         → ParticleC()   (angle = baseRot + angleRate * lifeRatio)
                            RANDOM_DIR + seed bit 5 → negate the angle delta

 ├── SCALE_MASK   → set s_uvScaleMat from uvScaleX/Y, ApplyAffineC(s_uvScaleMat, uvQuad)
 ├── HFLIP + seed bit 6 → ApplyAffineC(s_uvHFlipMat, uvQuad)
 ├── VFLIP + seed bit 7 → ApplyAffineC(s_uvVFlipMat, uvQuad)
 └── FRAME_ANIM   → ParticleB()  (writes cell offset into s_uvFrameMat, ApplyAffineC)
     → TransformUVs()  maps uvQuad onto uv0..uv3
```

**UV flag constants** (`_ed_particle_shaper_param::uvFlags`):

| Constant | Bit | Description |
|---|---|---|
| `UV_TRANSFORM` | `0x0001` | Enable any UV processing |
| `HFLIP` | `0x0002` | Random horizontal flip (seed bit 6) |
| `VFLIP` | `0x0004` | Random vertical flip (seed bit 7) |
| `RANDOM_DIR` | `0x0008` | Random rotation direction (seed bit 5) |
| `FRAME_SEED` | `0x0010` | Frame index randomised by seed |
| `SPRITE_SHEET` | `0x0020` | Enable sprite-sheet / rotation mode |
| `FRAME_ANIM` | `0x0040` | Animate frame by age or rate |
| `AGE_FRAME` | `0x0080` | Age-based frame advance |
| `SCALE_MASK` | `0x0300` | UV scale (2-bit field) |
| `RAND_ROT` | `0x0400` | Per-particle random base rotation |
| `VELOCITY_ROT` | `0x0800` | Velocity-aligned rotation |

### 4b — UV helper functions

| Function | Description |
|---|---|
| `ParticleA(quad, …)` | Aligns uvQuad to the particle's velocity projected onto the camera plane |
| `ParticleB(quad, …)` | Computes sprite-sheet frame → writes cell offset into `s_uvFrameMat` → `ApplyAffineC` |
| `ParticleC(quad, angle)` | Fills uvQuad with a 2D rotation matrix |
| `ApplyAffineC(mat, quad)` | Right-multiplies `quad` by the 2×3 affine matrix `mat` in-place |
| `TransformUVs(base, quad, uv0..3)` | Transforms the 4 corner UVs through uvQuad |

**Static UV matrices:**

| Static | Value | Purpose |
|---|---|---|
| `s_uvHFlipMat` | `{-1,0,0, 1,1,0}` | Flips U axis (const) |
| `s_uvVFlipMat` | `{1,0,0, -1,0,1}` | Flips V axis (const) |
| `s_uvScaleMat` | writable | Written from `uvScaleX/Y` each draw call |
| `s_uvFrameMat` | writable | Written from frame index in `ParticleB` |

### 4c — Colour

```
blendAlpha == 0  → ParticleColorA()   simple per-channel scale from particle->baseColor
blendAlpha != 0  → ParticleColorB()   palette-blended colour from pDrawData->palette[]
cornerColor != -1 → ParticleColorC()  derives rgbaColor[1..3] by scaling with per-corner
                                       multipliers (non-uniform quad lighting)
```

- `ParticleColorA` applies `alphaScale`, `redScale`, `greenScale`, `blueScale` per channel,
  then the global `alphaMultiplier` (= `alpha * 255 * pDrawData->globalAlpha`).
- `ParticleColorB` blends between two palette entries using `blendAlpha` as the key.
- `ParticleColorC` derives three additional per-corner colours from `rgbaColor[0]`, used only
  when `pDrawData->cornerColor != -1`.

### 4d — Size

```cpp
halfHeight = gParticleSizeScale * particle->yScale * (float)particle->sizeByte * 0.5f;
halfWidth  = halfHeight * pDrawData->aspectRatio;
```

### 4e — Vertex output

**Mode 1 (billboard):** one call per particle — `edDListWidthHeight2f(width, height)` then
`edDListVertex4f(x, y, z, 0)`. The display list hardware expands this into four corners
using the camera axes set at `edDListBegin` time.

**Mode 4 (ground-plane):** four explicit vertices per particle at world positions:

```
(-halfWidth, y,  halfHeight)   top-left
(+halfWidth, y,  halfHeight)   top-right
(-halfWidth, y, -halfHeight)   bottom-left
(+halfWidth, y, -halfHeight)   bottom-right
```

Each vertex gets its own UV and (if `bSeparateVertexColors`) its own colour.

---

## Key data structures

| Type | Role |
|---|---|
| `_ed_particle` | Per-particle: `sizeByte`, `yScale`, `age`, `lifetime`, `baseColor`, `seed`, `visible`, `state` |
| `_ed_particle_shaper_param` | Per-emitter config: `drawMode`, `uvFlags`, `sortFlags`, world matrix, material, UV params, palette |
| `_ed_particle_group` | Bounding sphere (`field_0x58` radius, `field_0x84` centre) and particle count |
| `AffineQuad2D` | 6-float 2D affine transform (xAxis, yAxis, origin) — identical memory layout to `Mat2x3` |
| `Mat2x3` | 2×3 row-major UV matrix: `m00 m01 m02 / m10 m11 m12` |
