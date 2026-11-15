# Animated texture investigation

## Finding

UV scrolling can be added to the existing preview material pipeline without
rebuilding meshes or uploading texture pixels every frame. The current G2D
decoder follows MAT -> first LAY -> TEX -> bitmap, but does not expose the
animation vector. Generated preview materials sample unmodified mesh UVs.

UV scrolling is now implemented against the checked-in source, as requested.
`KyaG2D::ReadMaterialScroll` validates first-layer metadata, and the preview applies
the offsets through `KyaUVOffset` on each generated material. Fixed source steps
(configurable, default 60 Hz), pause/resume and reset are available. The Unreal
editor build and `Kya.TextureAnimation.Scroll` automation test pass. The notes
below describe the source behavior and remaining fidelity considerations.

## Source behavior

- `src/b-witch/ed3D.h`: `ed_g2d_texture::pAnimSpeedNormalExtruder` points to four
  floats: x/y are per-step U/V increments; z/w are accumulated U/V offsets.
- `src/b-witch/ed3D.cpp`, `ed3DFlushMaterialAnimST` (line 6256): layer
  `flags_0x4 & 0x10` selects scrolling. When `gStepTime != 0` and the layer has
  not already updated this frame, z += x and w += y. Each component subtracts
  one at >= 1, or adds one at <= -1. This is a single adjustment, not general
  modulo. The routine sends z/w as the U/V offset.
- The same routine has a separate atlas-frame path when scrolling is off and
  `flags_0x4 & 2` is set. This requires the animation chunk and frame controller;
  a scrolling-only implementation must not treat it as continuous panning.
- `port/Windows/Renderer/Shaders/src/native.vert.glsl` adds the offset to ST.
  Unreal preview geometry currently copies decoded UVs directly, so no V sign
  reversal is indicated by this path.
- Rendering also gates scrolling by `animSTMaxDist` in `ed3DLinkStripToViewport`.
  Some actors modify speed at runtime. `StaticMeshComponentAdvanced::Create`
  and `ResetInternal` multiply x/y by 0.8333333 for the non-NTSC configuration.
  Consequently, a universal conversion of raw file increments to UV/second
  needs validation against a known material and game configuration.

## Serialized fields to read

Offsets below are relative to chunk starts, including the 16-byte chunk header,
and follow the existing `KyaG2D::ReadMaterialBitmap` addressing convention.

| Field | Offset |
| --- | --- |
| First LAY reference | MAT + 32 |
| Layer animation flags | LAY + 20 |
| TEX reference | LAY + 48 |
| Animation vector reference | TEX + 36 |
| Animation distance | TEX + 40 |
| Animation chunk reference | TEX + 44 |

Resolve references as file offsets with bounds checks, not host pointers. Require
a full 16-byte vector, reject non-finite values, and distinguish missing animation
from malformed data. A vector alone does not enable scrolling: check layer flags.

## Proposed implementation

1. Add a layer-aware animation metadata reader to `KyaG2D.h/.cpp`. Initially
   consume layer zero, matching `DecodeMaterialTexture`, while keeping scroll
   and atlas modes distinct. Leave imported bytes untouched for export.
2. In `AKyaLevelPreview::EnsurePreviewMaterials`, connect texture coordinates
   plus a `KyaUVOffset` vector parameter to the texture sample. Apply this to
   opaque, masked, additive, and translucent generated parents. Default to zero.
   User-supplied material parents must expose the same parameter to participate.
3. In `RebuildPreview`'s `GetTexturedSection`, read animation metadata and retain
   the dynamic material, initial offset, speed, and source TEX identity. Different
   materials can share a TEX: use a source-bank-plus-TEX key to keep their phase
   synchronized. Do not key state by bitmap alone.
4. Advance animation in the existing editor-enabled `Tick` and update material
   parameters. Use a defined source-step rate or fixed-step accumulator; adding
   the raw increments once per Unreal tick makes speed depend on viewport FPS.
   Provide pause/reset and deterministic time for screenshot comparisons.
5. Treat distance gating and actor-controlled speed changes as separate fidelity
   work. A first preview can animate all decoded scrolling materials, but should
   document that difference from game rendering. Atlas animation needs its own
   frame-controller investigation before implementation.

## Validation before shipping

- Decoder cases: valid scroll vector, negative speed, nonzero initial phase,
  missing vector, invalid/truncated offset, non-finite floats, static layer with
  a vector, and atlas-only layer.
- Animation cases: zero speed, pause/reset, wrap boundaries, matching phase across
  shared TEX references, and matching elapsed-time results at different viewport
  frame rates.
- Compare a known animated surface against debug-socket captures from the game,
  including direction and measured period. Verify all generated blend modes and
  a static surface. Editor viewport realtime must be enabled for visible motion.
- Verification targets the checked-in `src` behavior; PS2 verification is outside
  the requested scope.
