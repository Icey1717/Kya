# Bullet visuals audit

Audited 2026-10-21 against the Ghidra MCP program `SLES_514.73`, image base
`0x00100000`, language `r5900:LE:32:default`.

Scope: pistol and sniper bullet creation, movement, direct rendering, impact
effects, and lifetime transitions in `src/b-witch/ActorWeapon.cpp`. RPG shots
use separate `CActorProjectile` actors and are outside this audit. This is a
static source/PS2 comparison, not a runtime image comparison or a complete
Vulkan backend audit. Ghidra annotations were not changed.

The four confirmed port discrepancies below have now been corrected in
`ActorWeapon.cpp`. Their descriptions document the source before the fixes;
line references refer to that audited snapshot. Original PS2 behavior described
later in the report is preserved. Library trigonometry remains in use, so this
does not claim bit-exact parity with the PS2 cosine lookup table.

Status: accepted as behaviorally at parity for this audit. Temporary diagnostic
logging has been removed; the original PS2 bullet spawn behavior is retained.

## Confirmed port discrepancies

### 1. Pistol bullets after slot zero render at slot zero's position (high)

Source: `ActorWeapon.cpp:996-1004`, also `1034`.
PS2 function: `CBehaviourWeaponPistol::DrawBullets`, `0x002d2ef0`.

The loop correctly indexes state and orientation with `iVar3`, but resets
`pBVar2 = this->aBullets` each iteration. Matrix translation then uses
`pBVar2->position`, anchoring every visible shot to slot zero. Both
`edDListBegin` calls also mix slot zero's X with the current slot's Y/Z.

The PS2 advances its cursor by `0x50` per bullet (`0x002d3424`). It derives
`s1` from that cursor (`0x002d2fb0`), loads all four translation components
from `s1` (`0x002d2fd0-0x002d2ff4`), and uses that same bullet's XYZ for
`edDListBegin` (`0x002d3014-0x002d3028`). Ghidra's apparent mutation of
`this` is a cursor-recovery artifact, not evidence that all shots use slot zero.

Correction: make `pBVar2` point to `aBullets[iVar3]` and consistently use it
for position, state, and orientation.

### 2. Bullet reset loop starts from an uninitialized counter (high)

Source: `ActorWeapon.cpp:607-622` (`iVar4`).
PS2 function: `CBehaviourWeaponBullets::Begin`, `0x002d4920`.

The port increments and tests `iVar4` without assigning it first. This is C++
undefined behavior; it can leave bullet slots unreset or run beyond the array,
affecting both visual paths. The PS2 explicitly initializes its counter to zero
at `0x002d4a0c`, then performs eight iterations (`0x002d4a50-0x002d4a78`).

Correction: initialize `iVar4 = 0` before the loop.

### 3. Sniper trails always choose the same spiral handedness (medium)

Source: `ActorWeapon.cpp:1068-1076`.
PS2 function: `CSniperBullet::FUN_002d3be0`, `0x002d3be0`.

The port tests `rand2 & 0x10000`, but `CScene::Rand()` returns only 15 bits
(`scene.cpp:1369`), so this test is always false. The PS2 shifts its third
random state right by 16, masks with `0x7fff`, and tests bit zero of that
result (`0x002d3d18-0x002d3d28`). Even selects +1; odd selects -1
(`0x002d3d48-0x002d3d64`).

Correction: test `rand2 & 1`. Ghidra's high-level decompilation incorrectly
expresses this as a raw-state bit test; the assembly resolves the ambiguity.

### 4. Sniper trail X/Y trigonometry differs from the PS2 (medium)

Source: `ActorWeapon.cpp:1084-1086`.
PS2 function: `CSniperBullet::FUN_002d3be0`, `0x002d3be0`.

For `t = i/12`, polar angle `phi`, and signed azimuth `theta`, the original
computes approximately:

```text
x = sin(phi) * sin(theta)
y = sin(phi) * cos(theta)
z = 0.15 + t * 0.85
```

The port uses cosine for X and sine for Y, exchanging the transverse axes.
The PS2 X lookup subtracts pi/2 before indexing the cosine table
(`0x002d3e3c-0x002d3e78`); the Y lookup does not
(`0x002d3e58-0x002d3e9c`). The table at `0x00472260` begins with 1.0,
confirming it is a cosine table. The port also uses library trigonometry in
place of the original quantized table, a smaller numerical fidelity difference.

Correction: exchange the X/Y sine and cosine factors; consider lookup-table
precision separately if exact numerical parity is required.

## How the visuals work

### Shared simulation and impacts

- Each weapon owns eight bullet slots. State 0 is free, 1 is flying, and 2
  retains a post-hit lifetime. `Action` (`0x002d45d0`) stores the normalized
  original aim in `field_0x10`; `direction` and `speed` include inherited
  motion from the actor chain. Visual orientation therefore follows the
  original aim, which can differ from actual travel direction.
- `UpdateBulletsLife` (`0x002d4240`) advances by
  `cutsceneDeltaTime * speed`, ray-tests scenery and actors, and shortens the
  movement step to an accepted collision. Shared spin advances at
  `40.840706` radians/second, approximately 6.5 revolutions/second.
- `StartFx` (`0x002d3f60`) chooses the configured scenery FX (`field_0x40`)
  or actor FX (`field_0x44`). `0xffffffff` disables an FX. Rotation follows
  the original aim rather than the collision normal. The FX is positioned at
  the bullet's pre-step position, because it starts before the final position
  update. This ordering matches the PS2.
- Flying lifetime starts at three seconds. Pistol hits enter state 2 for
  0.5 seconds, but the direct pistol draw only accepts state 1, so the geometry
  disappears immediately. Separate impact FX have their own lifecycle.

### Pistol: two textured strips

`CBehaviourWeaponPistol::DrawBullets` (`0x002d2ef0`, source `970-1061`)
uses the game display list, builds an orientation from `field_0x10`, applies
the shared Z spin, and draws two materials:

| Part | Shape | Local dimensions | Vertices |
| --- | --- | --- | --- |
| Head | Triangular cross-section with pointed ends | Length 0.4, radius 0.075 | 10 |
| Tail | Two perpendicular textured quads | Length 1.5, half-width 0.15, start Z -0.1 | 8 |

The local tail extends to Z -1.6. Both parts use RGBA `0x7f7f7f7f` and
triangle strips. The fourth `edDListVertex4f` argument is a strip-skip marker,
not a homogeneous position W. Calls with `49152.0f` break strip connectivity;
the port's `edDListVertex4f_3D_DEFAULT` converts this to raw `0xc000` flags.
Dimensions and spin constants were also checked in PS2 data at
`0x00448aac-0x00448ac0`.

### Sniper: pooled model and camera-facing ribbon

- `FireBullet` (`0x002d2640`, source `1409`) acquires a model instance from
  `WeaponGlobal_0048dd60`, positions it, orients it from the original aim,
  and enables drawing. There is a shared pool of sixteen models and sixteen
  `CSniperBullet` trail records. Each weapon still owns eight simulation slots.
- Trail initialization (`0x002d3be0`) generates twelve randomized spiral
  control points once per shot. This is procedural geometry rather than a
  recorded history of prior bullet positions.
- While flying, `DrawBullets` (`0x002d2850`, source `1474`) updates the model
  matrix and draws the trail backwards along the original aim, with transverse
  scale zero, longitudinal scale 3, and ribbon width parameter 0.035. Thus the
  randomized spiral is collapsed onto the trail axis during flight.
- `CSniperBullet::Draw` (`0x002d3720`, source `1093`) prepends the current
  position to the twelve transformed points. It calculates a perpendicular in
  camera space and transforms the width offset back to world space, emitting
  a 26-vertex strip. Each pair is a centerline point and an offset point; it
  does not emit symmetric +/- half-width edges. Endpoint alpha is zero.
- `HitBullet` (`0x002d25a0`) removes the model and sets a 1.5-second trail
  lifetime, ignoring the base caller's 0.5-second argument. With normalized
  age `t = 1 - remaining/1.5`, transverse scale expands to `0.5*t`, axial scale
  to `3 + 0.5*t`, width to `0.035 + 0.05*t`, position rises by `0.15*t`, and
  direction tilts by up to `0.3141593` radians. Alpha fades with remaining life.

## Original behavior that should not be mistaken for port regressions

- The sniper material ID (`field_0x344`) is loaded/installed and passed to
  `CSniperBullet::Draw`, but that function does not use it. Both original and
  port bind a null material, enable blending, and set GS ALPHA to `0x48`
  (`edDListBlendFunc_002ca830`), i.e. source-alpha additive blending.
- Sniper `DrawBullets` initializes its color once outside the eight-slot loop.
  Each fading slot multiplies the current alpha, so later slots inherit prior
  attenuation, including flying slots. This order-dependent fade is present
  in the original `0x002d2850` decompilation.
- On natural flight expiry, `UpdateBulletsLife` calls `HitBullet` and then
  independently checks whether the resulting state is 2 and resets it in the
  same iteration. Consequently natural expiry does not retain the impact-style
  trail fade. Actual collisions occur later in the update and do retain it.
- Pool exhaustion can leave `WeaponGlobal::Add` returning null while sniper
  `FireBullet` dereferences the result. The original does the same; a defensive
  fix would be a separate behavior decision, not restoration of PS2 parity.

## Parity disposition

Closed as accepted PS2 parity for the audited bullet behavior after the four
corrections above. The original launch offset, attachment update order, impact
handling, and fade behavior are preserved. No barrel-alignment compensation was
introduced. This is a source/PS2 behavior comparison, not a claim of bit-exact
rendered pixels or cosine-table arithmetic.

The follow-up report of pistol shots appearing above and behind the barrel did
not reveal another transcription discrepancy in the checked paths:

| Path | PS2 address | Result |
| --- | --- | --- |
| Bullet launch offset | `0x002d45d0` | Original also adds stored weapon position and forward |
| Vector addition helper | `0x00266b40` | Assembly confirms `vadd.xyzw` |
| Weapon attachment | `0x002d59c0` | Same linked actor and bone |
| Actor management order | `0x001024a0` | Behavior runs before animation |
| Linked-actor update order | `0x00106b80` | Links update after managing active actors |
| Attachment transform | `0x00106720` | Bone times parent world matrix; normalized Z copied to forward |
| Actor matrix update | `0x00103540` | Same position and mesh-transform assignment |
| Bone registration / lookup | `0x0017fb00`, `0x0017ff60` | Same initialization and registered-matrix lookup |
| Registered bone recovery | `0x00180580`, `0x00242840` | Same unskin step and bind-matrix multiplication order |
| Orthogonal matrix inverse | `0x00267920` | Same transpose and translation calculation |
| Aim callback dispatch / blending | `0x0017cbe0`, `0x0017c930` | Same animation IDs, aim direction, and vertical blend selection |
| Global bone construction | `0x00242bf0` | Reviewed quaternion signs, parent composition, and bind application agree |
| Strip matrix submission | `0x002994e0` | Same object-to-view matrix composition |

Temporary captures showed launch positions agreeing with the attachment-derived
positions, and first-draw/submitted tip positions agreeing with launch positions
at logged precision. The later 25-shot capture contained 49 aim snapshots and
50 matching strip submissions across two animation LODs.

One existing port safeguard remains in aim blending: the next pose weight is
written only when its index is inside the active weight array. The PS2 writes
that next weight unconditionally at `0x0017cbac`, including at the final pose.
The guard does not change valid pose weights and never triggered in the 49
captured aim snapshots. It was not changed during this audit.

The pistol mesh export placed the raw opening center approximately at
`(-0.00237, -0.05357, -0.14893)`, facing negative Z, while the logged launch
offset was `(0,0,1)`. The export omits animation transforms, so those raw mesh
coordinates do not establish a port behavior discrepancy. The matching PS2
launch calculation is retained.

All temporary pistol logging hooks, trace side tables, the `Weapon` log category,
and the trace header were removed when closing this investigation. The Mesh
Previewer's reusable **Export OBJ** tool is retained; see `draw-inspector.md`.
Generated logs, exports, and plots remain local investigation artifacts.

## Validation

The final cleanup is checked with the Windows build, existing smoke suite, and
`git diff --check`. Runtime captures informed the parity decision; no new spawn
or rendering adjustment was made during cleanup.
