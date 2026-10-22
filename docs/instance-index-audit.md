# instanceIndex rename audit

Audited 2026-10-22 against the Ghidra MCP program `SLES_514.73`, image base
`0x00100000`, language `r5900:LE:32:default`.

## Origin and correction

Commit `9bfef13` ("Updating to proper folder structure.") renamed unrelated
`field_0x60` members to `instanceIndex` alongside the valid recovery of
`CActInstance::instanceIndex`. Its diff shows the same textual substitution in
vectors, floats, bytes, and unused `undefined` members across unrelated types.
Some Ghidra structures also retain the misleading name, so their current member
names alone are not evidence of purpose.

This audit restores `field_0x60` for 15 unrelated field declarations and their
uses across 29 source/header files. It also restores the name in an existing
guarded, incomplete native tutorial path. Field types, order, allocation,
arithmetic, conditions, and function signatures are unchanged by this cleanup.
Offset names avoid assigning stronger meanings than the recovered code proves.

## Fields with executable uses

The following comparisons used both decompilation and disassembly. Addresses
identify original PS2 functions; interpretations describe their observed uses,
not recovered original source names.

| Owner | Observed purpose | PS2 evidence |
| --- | --- | --- |
| `CActInstance` | Actual bonus instance index; retained as `instanceIndex` | `Init`, `0x00398720`; `0x00398764` stores argument `t0` to offset `0x60`. PC bonus drawing uses the value to select sprite vertices/colors/UVs. |
| `CDynamicExt` | Three floating point translation-component overrides | `CActorAutonomous::ManageDyn`, `0x00122160`, substitutes x/y/z under flags `0x4000`, `0x8000`, `0x10000`. |
| `CompanionAlert` | Remaining countdown, initialized from `field_0x5c` | `CBehaviourCompanion::Manage`, `0x001e6e50`; stores at `0x001e72a8`, subtracts delta time and tests against zero around `0x001e7564`–`0x001e7580`. |
| `StaticMeshComponentAdvanced` | Byte selecting mesh handling; not an array index | `HasMesh`, `0x00406870`; `lbu` from offset `0x60` and comparison with one. PC `SetNode` also sets it to one. |
| `CBehaviourTrapStand` | Cached owner bounding-sphere radius | `Begin`, `0x003514f0`; floating point store at `0x00351520` to offset `0x60`. |
| `CNewFx` | Spatialization motion vector, scaled by frame delta for position adjustment | `SpatializeOnActor`, `0x001a2700`; float component loads/stores at offset `0x60` and adjacent offsets, and vector scaling by delta time. |
| `_ed_particle_shaper_param` | Mode 3 second UV coordinate's y component | `edPartDrawShaper`, `0x0026a860`; `0x00277aa4` loads offset `0x60` into `f13` for `edDListTexCoo2f`. Removed the incorrect "instance index" comment. |

Additional `CDynamicExt` writers were compared in the following functions before
updating their member references:

| Source | PS2 comparison |
| --- | --- |
| `ActorBrazul.cpp` | `FUN_003e87e0`, `0x003e87e0` |
| `ActorBunch.cpp` | `FUN_003de8e0`, `0x003de8e0` |
| `ActorProjectile.cpp` | `StateFlyingDirected`, `0x00205ff0` |
| `ActorHero_Private.cpp` | `StateHeroWindFly`, `0x00148f20` |
| `ActorJamGut.cpp` | `StateJamGutJump`, `0x00374280` |
| `ActorWolfen.cpp` | `StateExorcize`, `0x00176980`; inherited mesh byte references were also updated |
| `ActorWoof.cpp` | `BehaviourWoofGuard_Manage`, `0x003333c0` |
| `ActorFighter.cpp` | `_ManageFighterDyn`, `0x00316770`; mesh byte comparison above |

`ActorNativ.cpp` has two stale member references inside `IMPLEMENTATION_GUARD`.
Its original `InterpretMessage`, `0x00163320`, was inspected, but does not establish
a current member named `instanceIndex`. These references receive only the
historical offset-name restoration; this audit does not recover that incomplete
tutorial implementation.

## Unused placeholders

Nine other declarations are single `undefined` bytes between `field_0x5f` and
`field_0x61`, with no member uses found. Their names are restored from the
pre-rename representation without inferring any semantic purpose:

- `src/EdenLib/Include/edBank/edBankFiler.h`
- `src/EdenLib/Include/edFile/edFilePath.h`
- `src/EdenLib/Include/edFile/ps2/_edFileFilerCDVD.h`
- `src/EdenLib/edDev/Sources/edDev.h`
- `src/b-witch/CinematicManager.h`
- `src/b-witch/SaveManagement.h`
- `src/b-witch/Scene.h`
- `src/b-witch/TranslatedTextData.h`
- `src/b-witch/kya.cpp`

## Retained names and limitations

Bonus instance parameters, locals, debug UI, and tests remain valid uses of
`instanceIndex`. Renderer instance-loop indices in `port/Windows/Renderer` are
also valid and unchanged. A repository search found no other remaining
`instanceIndex` uses in tracked game/engine source outside the bonus family.

The earlier `CFxLightEmitter` and boomy timing fixes are separate existing
changes. This audit preserves them and the user's removal of obsolete emitter
members. It does not change Ghidra annotations or infer that every historical
`field_0x60` elsewhere in the repository requires work.

Validation: Windows debug build and full `ctest` suite. This cleanup changes
identifiers only; it adds no runtime behavior requiring new tests.
