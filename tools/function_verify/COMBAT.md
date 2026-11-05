# Collision/combat support audit

Nine concrete targets/domains now compare production Windows P-code with original PS2
P-code. Full state-transition analysis still has blockers; passing this suite
does not mean the entire collision or combat subsystem is verified.

| Target | PS2 entry | Support exercised | Result/domain |
| --- | --- | --- | --- |
| CActorFighter::_EndFighterFlip | 00328300 | Symbolic pointer to a separate collision object; flag and pointer writes | Equivalent for aligned collision pointers in 0x4000..0x5000, arbitrary uint32 flags and attached pointer identities |
| CActorFighter::IsInStaggerSequence | 0031cdb0 | Signed versus unsigned arithmetic; actual compiled predicate | Equivalent for every 32-bit state after the signedness correction |
| CCollision::ClearInternalData | 0019e310 | Direct call to matrix initialization; vector and pointer fields | Equivalent for arbitrary mapped initial field bits |
| CCollision::Reset | 0019dba0 | Nested direct call followed by a tail transfer to matrix initialization | Equivalent for arbitrary uint32 initial contact flags |
| edF32Matrix4SetIdentityHard | 00267850 | Exact constant moves and PS2 VU lane rotation | Equivalent for arbitrary initial matrix bits |
| CAudioManager::FUN_00184470 | 00184470 | Null-pointer test and conditional decrement | Equivalent for every uint32 counter and mapped music pointer identity |
| CAudioManager::FUN_001844a0 | 001844a0 | Null-pointer test and increment | Equivalent for every uint32 counter and mapped music pointer identity |
| edAnmMetaAnimator::SetLayerTimeWarper | 002425a0 | Raw floating-register argument and indirect layer store | Equivalent for index 0, disjoint valid layer storage at 0x4000, and every binary32 bit pattern |
| CActorFighter::SetStandAnim (partial domain) | 0031ba50 | Signed integer argument, two branch guards, stack preservation | Equivalent when animation is unchanged OR state != 6; dispatching idle branch remains unsupported |

Run `python tools/function_verify/compiled_extended.py --suite combat`, or use
the combined runner/watcher. Compiler-probed Windows offsets and audited PS2
offsets are in `combat_cases.py`. `compiled-combat/report.json` records domains
and assumptions. All non-private memory outside mapped fields must remain
unchanged, including the actor's collision pointer in the pointer-following case.

## Logging-disabled verification build

Configure `cmake --preset x64-verify` in the Visual Studio x64 developer
environment, then build `cmake --build out/build/x64-verify`. This inherits Debug
settings and sets `KYA_ENABLE_GAME_LOGGING=OFF`; other presets default to ON.
Verification executables live in the build's `bin` directory. The game logging
macros omit category lookup, format/argument evaluation, and flushing. Assertions,
Tracy instrumentation, and game behavior are not stubbed.

The Python entry points default to this build and `out/verification/x64-verify`.
Use `--build-dir` and `--output-root` to select isolated alternatives. CMake's
verification targets use their own build and artifact directory. Manifests and
reports record the logging configuration and CMake cache hash; existing compiler
commands, compiler versions, source/object hashes and PS2 hashes remain recorded.
These results apply to the logging-disabled build, not the logging-enabled build.

The SetState/PlayAnim logging arguments inspected are field reads and scalar
parameters; CAnimation::PlayAnim likewise reads the actor name and scalar IDs.
There is no claim yet that every logging argument in the transitive behavior and
animation graph has been audited. Direct calls to logging APIs are still ordinary
dependencies and must not be accepted as no-op contracts.

## Support added

`call_graph.py` instantiates a separate context for each approved direct call and
tail transfer. Real return addresses are checked before continuing at the caller.
Exact-type callers can supply one target per indirect call site; both executors
must prove the loaded address equals that target before entering its complete
body. `fighter_dispatch.py` now recovers four concrete slots from PS2 table bytes
and actual Windows ADDR64 relocations. Wiring those slots into a complete
transition proof and proving object dynamic types remain outstanding.
It rejects unresolved indirect calls, recursion, unknown external transfers,
incompatible exports, and exhausted call-depth bounds. The PS2 executor preserves
partial register lanes and verifies the complete 128-bit saved s0 register,
stack, and return address. The Windows executor now handles XMM0 raw-bit moves,
zeroing, and explicit literal memory reads; it does not approximate SSE floating
point arithmetic.

`vu_identity.py` accepts only exact raw user-op/decoded-operand agreement for
full-lane zero moves, full-lane 32-bit rotation, and the proven +0 + +1 step in
identity initialization. Partial masks, unknown IDs, symbolic additions, NaNs,
rounding, denormals, and other VU operations remain unsupported. Literal float
data is read and checked in the actual COFF objects before relocation; no source
matrix expression supplies the expected output.

The pointer case assumes distinct actor/collision regions, aligned non-null
collision addresses, zero-extension of 32-bit pointer identities, and valid
mapped storage. Null dereferences, arbitrary native 64-bit addresses, allocation,
and general object graphs remain outside its domain.

## Signedness bug recovered and corrected

The original `0031cdb0` loads actorState at PS2 offset 0x104, subtracts 0x5a,
and performs unsigned `_sltiu ... ,5` in the return delay slot at `0031cdbc`.
Live Ghidra decompilation and disassembly were checked before changing source.
Windows actorState is compiler-probed at offset 556 in this build.

The prior Windows C++ expression used signed arithmetic, incorrectly accepting
states below 0x5a (for example state 0). The expression now casts the subtraction
constant to uint, matching the original modulo-32-bit unsigned range check.
The existing recovered name and unrelated in-progress fighter edits are preserved.
Both the actual before/fixed compiler outputs are retained in `fixtures/combat.json`.

A separately compiled wrong collision flag is also detected:

```powershell
python tools/function_verify/compiled_extended.py --suite combat --mutate flip
```

This changes only the disposable `_EndFighterFlip` copy from OR 0x1000 to
OR 0x2000; a mismatch and exit 1 are expected. Repository source is untouched
by the negative control.

## Remaining blockers, reproduced

```powershell
python tools/function_verify/audit_blockers.py
```

This rebuilds current production objects, attempts bundle/link preparation, and
records `<output-root>/combat-blockers/report.json`. Default exit is 1 while
proofs are unavailable. `--audit-only` permits exit 0 for an audit, never a proof.
The combined live runner now requires these two transition targets and fails
while they remain blocked.

| Candidate | Evidence | Blocking support |
| --- | --- | --- |
| CActorFighter::SetStandAnim (0031ba50) | Complete compiled dependency closure lifts; graph includes the movable SetState dispatch obligation; PS2 `jalr t9` at 0031ba80 | Compose dynamic-type, memory and complete callee proofs |
| CActor::SetState (00102a00) | Complete compiled dependency closure lifts; unresolved transfers remain explicit graph frontiers; PS2 has five indirect calls | Runtime object/global relations, proved virtual targets, effects and required floating-point semantics |
| CCollision::Reset with unrestricted 32-bit contact flags | Fixed production output proves equivalent; old output remains a mismatch regression | Resolved by preserving upper 24 bits |

The old 0..255 Reset restriction is retained only for historical fixtures.
Live comparisons default to arbitrary 32-bit contact flags. No logging,
virtual call, or animation/audio operation is silently skipped or stubbed as
successful. These are still blockers to claiming full combat state-transition
coverage.

## Collision correction evidence

Live Ghidra MCP decompilation and disassembly were checked for Reset (0019dba0)
and ClearInternalData (0019e310). Reset uses `lbu` at 0019dbb4, `andi 0xe8` at
0019dbb8, and `sb` at 0019dbbc. Windows now masks with 0xffffffe8, preserving
the upper three bytes without changing field layout. ClearInternalData does not
write contact flags and requires no correction. `fixtures/combat.json` retains
the old mismatch; `fixtures/collision-reset-fixed.json` retains rebuilt production
exports proving both functions over arbitrary mapped initial bits.

## Outstanding transition work

The intended initial domain is exact CActorFighter/CBehaviourFighter objects:
SetStandAnim's unchanged, non-idle, and idle paths, plus SetState idle-to-idle,
idle-to-run, and run-to-idle with explicit/configuration-selected animation IDs
and zero/nonzero idle-loop counts. Full cases are not yet proved. The unchanged
idle/run subdomain with an explicit unchanged animation now composes complete
behaviour lookup and registry resolution for bounded sorted behaviour lists;
production initialization of all 108 fighter configuration entries is also
proved independently. See [runtime domains and initialization](TRANSITIONS.md#fighter-runtime-inputs-and-configuration-initialization).

Checked dispatch supports finite target sets. Complete reachable
object/global mappings, behavior/animation analysis, and general floating-point
support remain outstanding. The audio helpers are proved independently; their
composition into state transitions is not proved. The time-warper proof covers
raw copies, including signed zero and NaN payloads, not FP arithmetic or index -1.
Loop cutoffs must be proven unreachable, and PS2 path pruning only occurs after
an unsatisfiability result. Unknown solver results remain failures.

The graph linker can retain unresolved direct/indirect transfers as explicit
terminal frontiers with `defer_unresolved=True`. Both executors reject any
reachable frontier; the partial SetStandAnim proof discharges them through its
documented domain. Raw graphs cannot inject this synthetic operation. This
support does not discharge the idle dispatch branch.

The blocker audit computes a bounded closure of complete compiled direct
callees. Missing imports/globals and conflicting function definitions remain an
explicit frontier. Duplicate COMDAT functions are coalesced only after complete
bytes and named relocations match, with object-local non-COMDAT bindings rejected.
The current SetState closure includes 63 functions from actor, movable actor,
animation, animation manager, event track, audio and pointer-conversion objects.
Track handling reaches CFxHandle::IsValid/Stop/Kill/Reset. Their compiled wrappers
are included, but their virtual effect targets are not proved.
Compiler probes recover registry, animation stage/callback, scene and Debug
validation storage. Runtime mappings still need composition into transitions.
In particular,
BehaviourEntry's Windows field is a registry handle, not a zero-extended native
pointer: valid slot bounds, nonzero reference count and matching generation must
be established before relating ResolvePointerChecked to the original pointer.

## Concrete dispatch evidence

Run `python tools/function_verify/fighter_dispatch.py` after building. The blocker
audit also regenerates this evidence. It is explicitly not an equivalence result.

| Type / method | PS2 table + offset | PS2 entry | Windows slot byte offset |
| --- | --- | --- | --- |
| CActorFighter / SetState | 00441a90 + 0x7c | 0011e7c0 (CActorMovable::SetState) | 0xe8 |
| CActorFighter / GetStateCfg | 00441a90 + 0x54 | 0031dd50 | 0x98 |
| CBehaviourFighter / InitState | 00441a30 + 0x34 | 00313d80 | 0x58 |
| CBehaviourFighter / TermState | 00441a30 + 0x38 | 00313290 | 0x60 |

Live PS2 decompilation/disassembly of 0011e7c0 shows a conditional store of
0x3f800000 to actor offset 0x1b8 before tail-calling 00102a00. A concrete fighter
proof must include this movable-actor weight reset; assuming direct base dispatch
would omit it. The fighter destructor at 0031f0a0 installs table 00441a90.
Tables reside in writable PS2 memory: table identity is an explicit input
condition, not an inferred read-only invariant. The checker must recheck loaded
targets after any intervening writes.

`fixtures/transition-leaves.json` retains actual compiled audio/time-warper
exports. Negative controls detect missing audio writes, zeroed float stores,
unsupported operations, and invalid vtable relocation kinds/addends/holes.

## Compiled registry and Debug-validator contracts

`registry_memory.py` executes the complete compiled `ResolvePointerChecked`
closure, including vector and optional helpers. It proves null resolution and
resolution through either of two slots in each registry, with symbolic generation
bits, reference counts, returned 64-bit pointer values and unrelated memory.
Nonzero handles must select an in-bounds live slot with the matching generation.
The proof checks every write is confined to private stack storage. This establishes
the Windows resolver contract; callers must still establish valid registry inputs
and relate returned pointer identities to PS2 objects. It does not establish the
invariant for every game caller or prove registry allocation/release operations.

Run it from the Visual Studio x64 environment:

```powershell
python tools/function_verify/registry_memory.py --output-root out/verification/registry-check
```

The transition audit also runs `animation_memory.py` against the complete compiled
Debug animation validator and its vector helpers. Its domain has one layer, two
animation entries, structurally valid matching live/snapshot banks, track IDs
-1..1, and an arbitrary 32-bit history counter and initial history bytes. Both
entries prove the returned track-data pointer, complete history-entry writes,
counter increment including wraparound, and preservation of all other non-stack
memory. Diagnostic failures remain explicit nonreturning frontiers and must be
unreachable. This is a Windows-only contract; diagnostics are not silently
discarded and no playback/morphing equivalence follows from this result.

`transition_data.py` compiles disposable same-translation-unit layout probes for
the actual globals and slot fields. `coff_data.py` extracts exact storage bytes,
permissions and ADDR64 fixups. BSS bytes describe link-time storage only, not
post-constructor runtime state. Reports retain source/object/probe hashes and
compiler commands. The candidate MSVC vector layout is checked by executing its
actual compiled helpers, rather than being accepted as a helper summary.

`MemoryRelation` describes field and pointer-identity relations without assuming
equal addresses across platforms. `RegionMemory` keeps views of disjoint storage
to avoid irrelevant symbolic aliases. Each optimized access needs a solver proof
of region ownership; an uncertain classification falls back to the full byte
array. Write permissions also apply to direct RIP-relative P-code outputs.

The audit saves `semantic-frontier.json` for each compiled transition closure.
It records floating-point instructions, indirect transfers, processor-control
instructions and user operations with addresses and symbols. This is a syntactic
overapproximation, not a feasibility result. Scalar SSE support requires explicit
processor control state and `fp_mode`; general EE/VU arithmetic remains unsupported
apart from the audited operations and conditional reference mode described in
[processor support](TRANSITIONS.md#processor-support-and-evidence).
The [EE Core User's Manual, section 8.8](https://docs.alexrp.com/mips/ee.pdf) specifies
non-IEEE behavior, including missing guard/round/sticky bits. Intel's
[architecture manuals](https://cdrdv2-public.intel.com/868137/325462-089-sdm-vol-1-2abcd-3abcd-4.pdf)
describe SSE control/status behavior. Substituting one IEEE rounding mode for
both processors is not an approved implementation.

One concrete effects implementation gap was checked live against Ghidra:
`CLodFx::IsLooped` at PS2 `0036a5d0` returns the byte at `this+0x88`
(`jr ra`; delay-slot `lbu v0,0x88(a0)`). The Windows override in `FxLod.h`
previously executed `IMPLEMENTATION_GUARD()` and returned false; Kill/Stop were
also guards. These three methods are now recovered from live PS2 evidence.
The independent effect suite proves IsLooped and four Stop handle partitions;
Kill's complete cleanup graph and configured whole-transition composition remain
outstanding. See [composition and processor support](TRANSITIONS.md) for exact
domains, numeric policies, proof interfaces and remaining blockers.

`fixtures/registry-resolver.json` and `fixtures/animation-validator.json` retain
actual compiled exports. Negative controls check incorrect slot layout, registry
writes, image identity, missing diagnostic counter updates and reachable validator
failure calls. These contracts are recorded separately from the still-nonpassing
whole-transition targets and are not added to the count of PS2 equivalence proofs.
