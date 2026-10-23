# Fifth random function-audit sample

Reviewed on 23 October 2026 against live Ghidra program `SLES_514.73`, image base
`00100000`. Five candidates were drawn without replacement using PowerShell
`Get-Random -Count 5` from the version 4 queue of 2,168 candidates. Previously
sampled names were excluded; there were no redraws after inspecting the result.

The exact draw is in `out/function-audit-filtered-v4/random-sample-5.json`.
`random-sample-5-evidence.json` in the same directory retains the live
decompilation, sampled disassembly, relevant layouts and original cosine-table
bytes. All five current source definitions match their saved packet source after
line-ending normalization. All five live PS2 decompilations match the snapshot.
These generated artifacts are local to this checkout.

| Function | PS2 address | Assessment |
|---|---|---|
| `CBehaviourDCA::End` | `001ed110` | Equivalent inherited helper and pointer accessor |
| `edF32Matrix4FromAngAxisSoft` | `00264360` | PS2 trig macros active in the Windows game build |
| `CActorMiniGame::CActorMiniGame` | `0010cba0` | Implicit native base construction |
| `CLevelScheduler::LevelsInfo_ReadTeleporters_V7_V9` | `002e1a90` | Typed fields replace byte offsets and stride |
| `CActorAmortosCmd::Init` | `0035fe00` | Equivalent extracted loop; misleading decompiler types |

No confirmed behavioral defect was found. This small sample does not establish
equivalence or estimate the remaining defect rate. No game code, filter rules or
review exceptions were changed during this investigation.

## DCA end: helpers preserve the operations

The original calls `CActorAutonomous::SV_AUT_PathfindingEnd` at
`001ed120`–`001ed124`, using the owner at `this + 8`. It then loads the resolved
actor reference at `this + 0x8c` (`001ed134`) and calls `DoMessage` with message
`0x14` and parameter `1` at `001ed13c`–`001ed140`.

The port's `CBehaviourWolfen::End(newBehaviourId)` at `ActorWolfen.cpp:9443`
does exactly `pOwner->SV_AUT_PathfindingEnd()`; its parameter is unused.
`S_STREAM_REF<CActor>::Get()` in `Types.h` loads the stored pointer through
`LOAD_POINTER`. Ghidra's `CBehaviourDCA` layout confirms `actorRef` at `0x8c`.
The port initializes that reference in `CBehaviourDCA::Init` before use.

Thus both versions end pathfinding and send the same message to the referenced
actor. The tool sees extra named `End` and `Get` calls because it does not expand
these parameterized and value-returning helpers. Ghidra's inferred integer return
from `End` reflects the final call result; the native behavior interface is void.
No omitted operation was identified.

## Axis-angle matrix: inspect the actual macro configuration

The raw source appears to replace table lookups with `cosf(angle)` and
`sinf(angle)`. That would be a numerical change if these were libc calls. They
are function-like macros in the configured Windows game build:

- `CMakeLists.txt:851` defines `KYA_USE_PS2_TRIG` privately for `Kya`. The generated
  `out/build/x64-debug/build.ninja` also includes this definition for `MathOps.cpp`.
- `MathOps.h` conditionally includes `Ps2Trig.h`, which includes the standard
  declarations before defining the macros. Later standard includes do not remove
  these definitions.
- `cosf` indexes `edFCosinus` using
  `int(abs(float(angle) * scale) + 0.5f) & 0x1fff`.
- `sinf` uses the same formula with `float(angle) - pi/2` before scaling.

The scale literal has bits `0x44a2f983`, matching live data at `00448518`; the
half-pi literal has bits `0x3fc90fdb`. Original instructions
`00264390`–`00264424` compute these two scaled, absolute, rounded indices;
`00264408` and `00264434` apply `0x1fff`. The matrix arithmetic and translation
row then match the source.

All **8,192 used table entries** in `Ps2CosineTable.inc` were converted to float32
and compared byte-for-byte with 32,768 live PS2 bytes starting at `00472260`.
They match exactly. Entry 8192 is outside this function's masked index range and
was not part of that comparison.

This is a macro-expansion false positive for the configured Windows game build.
The audit does not currently evaluate function-like macros or build definitions.
The assessment does not extend to a build that omits `KYA_USE_PS2_TRIG`, nor does
it establish bit-identical results across all floating-point arithmetic inputs.
Do not globally equate libc trig calls with table lookups: the source also has
real libc call sites deliberately using `std::sin` and `std::cos`.

## Minigame constructor: implicit base initialization

Original `0010cbac` calls `CActor::CActor` at `00105b00`, then installs the derived
vtable and zeroes four members at `0010cbc4`–`0010cbd0`.

`ActorMiniGame.h:117` declares `CActorMiniGame : public CActor`. The native
constructor implicitly invokes the default `CActor` constructor and installs the
native derived vtable. Its body zeroes the same four members. The absent explicit
base call is normal C++ construction, not missing initialization. A generic
filter needs inheritance and constructor-initializer evidence; dropping all
constructor calls would hide genuinely missing member construction.

## Teleporter import: named fields and native indexing

Ghidra's `S_LEVEL_INFO` places `levelName` at `0x24` and `aSubSectorInfo` at
`0x58`. `S_SUBSECTOR_INFO` has PS2 size `0x28`, with fields at `0`, `8`, `0xc`,
`0x10` and `0x14` matching the five source assignments.

The misleading decompiled base expression
`levelName + index * 0x28 - 0x24` cancels the `levelName` offset. Original
`002e1ac0`–`002e1ae4` computes `levelInfo + index * 40` and stores the eight-byte
field at `+0x58`. Stores at `002e1aec`, `002e1af4`, `002e1afc` and `002e1b04`
write the four following fields at `0x60`, `0x64`, `0x68` and `0x6c`.
These are the flagged literals 40, 88, 96, 100, 104 and 108.

The source uses `aSubSectorInfo[index]` and named fields. The packed input record
is 28 bytes, enforced by its source static assertion, matching the original
`+0x1c` advance at `002e1b0c`. Negative elevator IDs clamp to zero, the maximum ID
update matches, and both versions accumulate wolfen counts, propagate bit 0 into
flags and adjust the first entry and level total (`002e1b28`–`002e1b84`). Native
indexing accommodates the port's pointer widths. No field or operation mismatch
was found; this is not validation of malformed record counts or array bounds.

## Amortos commander init: count mistaken for an actor pointer

The original loads the reference list at `this + 0x160`, reads its first word
into `s1` as a count, and starts entries four bytes later
(`0035fe1c`–`0035fe28`). It calls `S_STREAM_REF<CActor>::Init` at `00119a00`
once per entry, decrements the count at `0035fe40`, advances four bytes at
`0035fe48`, then clears `this + 0x164` at `0035fe50`.

The decompiler instead types that count as a `CActor*` and renders decrement as
`&pCVar1[-1].field_0x15c + 3`, producing the spurious literal `3`. The instruction
is simply `addiu s1,s1,-1`.

The source calls `aAmortos->Init()`. `S_ACTOR_STREAM_REF::Init` at
`Actor.cpp:5743` loops over `entryCount` and calls the same entry initializer.
The specialization at `CinematicManager.cpp:864` maps index `-1` to null and
other indices to `aActors[index]`, then stores the native pointer handle.
Original `00119a00`–`00119a30` performs exactly that index resolution. The extra
register arguments visible at the call site are not consumed by this callee.
Both versions call base `CActor::Init` first and clear the final member afterward.

## General filtering opportunities

This sample reinforces three reusable directions, without per-function exceptions:

1. Build-aware, bounded expansion of supported function-like macros. Preserve
   arguments and values, and distinguish table macros from actual libc calls.
2. Binding-aware helper and constructor modeling, including implicit base calls,
   simple return accessors and extracted loops. Preserve receiver identity,
   arguments and side effects.
3. Layout-aware canonicalization of named fields versus raw offsets, plus
   instruction evidence when misleading decompiler types manufacture literals.

These are opportunities for future tooling work, not rules enabled by this
review. All five remain candidates in the current report. Validation comprised
source/snapshot consistency, live instruction and datatype review, build-flag
inspection and the complete used-table comparison. No game build or runtime
smoke test was run.
