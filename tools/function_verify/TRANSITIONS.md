# Composable verification and processor semantics

`transitions.py` executes complete exported bodies using explicit
`ExecutionState`, `ExecutionOutcome`, and `TransitionStep` interfaces. Existing
tuple-returning executor callers remain supported. A sequence carries actual
registers, heap/global memory, path conditions, and processor status between
invocations. Only external call-entry stack/link storage is reset. Argument and
`this` bindings belong in each step's architecture-specific options; PS2 state
execution requires explicit absolute memory regions.

Every stage proves returning-path coverage, successor preconditions, output
relations, and preservation of other memory. The executor rejects reachable
unresolved transfers, loop cutoffs, unknown solver results, unsupported
instructions, and exhausted exploration bounds. Branch-prefix pruning requires
an UNSAT result from execution of that exact prefix. Contracts are currently
executed through their complete bodies; no cached or handwritten call summaries
are trusted.

`call_graph.link(..., indirect_targets={site: [target1, target2]})` proves
membership of the current loaded address, branches on its value, then executes
the selected body. Singleton integer mappings remain supported. Every expanded
instruction retains `original_address` and `call_context`. Dispatch is checked
again for each call, including after writes to writable vtable storage.

## Numeric observations

`MemoryRelation.field(..., name='animationTime', numeric_policy=FloatPolicy())`
opts one binary32 field into output tolerance. Initial field relations remain
exact. Pointer fields cannot use a numeric policy. The default finite-value
condition is evaluated over exact real interpretations of the output bits:

```
abs(Windows - PS2) <= 0.000001 + 0.00001 * abs(PS2)
```

Signed zeros compare numerically. Non-finite encodings require identical bits.
The result is `equivalent-with-tolerance`, distinct from exact `equivalent`.
The report validator requires an explicit field allow-list and these exact
budgets before accepting that verdict.

Tolerant steps must classify every machine conditional branch as either a
paired gameplay decision (`BranchDecision`, optionally inverted) or an
architecture-specific auxiliary decision. Auxiliary classification is a reviewed
specification assumption, not an automatic diagnostic exemption. Paired gameplay
decisions must agree, including their number of executions. Tolerance never
constrains successor inputs: both exact machine states survive composition.
An offline negative control demonstrates a one-ULP difference that passes the
first observation and fails when a later gameplay branch distinguishes it.

## Processor support and evidence

`processor_fp.py` is opt-in (`fp_mode=True`). It validates numeric opcode shapes,
operand widths, and instruction bytes before applying processor-specific
semantics. Unsupported instructions remain failures.

* SSE: ADDSS, SUBSS, MULSS, DIVSS, COMISS and UCOMISS, explicit MXCSR, rounding,
  DAZ/FTZ, sticky exception flags, comparison flags, and preservation of upper
  scalar-register lanes. Reachable unmasked exceptions fail. Arithmetic NaN
  input payload/priority behavior remains unsupported; invalid operations with
  non-NaN inputs produce the SSE indefinite encoding.
* EE: C.EQ.S, C.LT.S and C.LE.S use integer encoding comparisons. Exponent-zero
  encodings compare as signed zero; exponent-255 encodings are finite values.
  Unconditional general EE arithmetic is still unsupported. It is **not** replaced with IEEE
  round-toward-zero: EE manual sections 8.7–8.8 explicitly distinguish them.
* VU: the existing audited identity operations remain supported. General VU
  arithmetic, accumulator/status behavior and unvalidated user operations remain
  unsupported.

### Pinned PCSX2 reference mode

The PCSX2 source audit is pinned to commit
`1f67c53b6723fcd6a90041a5b296f0e34a6a3222`; source URLs and SHA-256 hashes are
retained in `fixtures/pcsx2-reference.json`. Its extended-FPU implementation
documents [multiply/divide/square-root inaccuracies](https://github.com/PCSX2/pcsx2/blob/1f67c53b6723fcd6a90041a5b296f0e34a6a3222/pcsx2/x86/iFPUd.cpp),
and notes limited testing of add/sub underflow. Emulator agreement therefore
cannot automatically discharge hardware-semantic obligations.

`ee_reference.py` implements the corrected add/sub algorithm symbolically,
including operand alignment truncation, the extended exponent, underflow
fraction retention, saturation, and cause/sticky flags. Enable it explicitly
with `ExecutionState(..., processor={'ee_arithmetic_model': ee_reference.MODEL})`.
The assumption survives execution and composition; the verdict becomes
`conditional-equivalent` (or `conditional-equivalent-with-tolerance`). The
combined proof validator rejects those reference-dependent results as ordinary
PS2 equivalence, even if a caller relabels the result string.

The 274 vectors in `fixtures/ee-add-sub-reference.json` compare the symbolic
encoding against an independent concrete implementation of that pinned
algorithm. They are not captures from PS2 hardware or from a complete PCSX2
executable. Regenerate them with:

```powershell
clang tools/function_verify/ee_reference_oracle.cpp -O0 -o out/verification/ee-reference-oracle.exe
python tools/function_verify/capture_ee_vectors.py out/verification/ee-reference-oracle.exe --output tools/function_verify/fixtures/ee-add-sub-reference.json
```

`fixtures/sse-scalar.json` contains 1,064 native x64 oracle observations of raw
results, EFLAGS and MXCSR across all rounding modes and separate/combined DAZ/FTZ
settings, including exceptional inputs and cancellation/underflow boundaries.
These are regression evidence, not an exhaustive proof of the numeric model.
Regenerate with a separately compiled oracle in a Visual Studio x64 environment:

```powershell
clang tools/function_verify/fp_oracle.cpp -O0 -o out/verification/fp-oracle.exe
python tools/function_verify/capture_fp_vectors.py out/verification/fp-oracle.exe --output tools/function_verify/fixtures/sse-scalar.json
```

The updated Java exporter adds byte/operand metadata, operation sequence
identities, delay-slot provenance and exporter/Ghidra versions without breaking
the existing raw-pcode schema. Rebuild Totala using its documented builder before
headless lifting. `client.export_pcode` can obtain byte provenance read-only from
the legacy live EE bridge without reinstalling that bridge.

## Recovered effect callees

The following functions were checked through live Ghidra MCP decompilation and
disassembly before changing `FxLod`:

| Function | Original address | Recovered behavior |
| --- | --- | --- |
| CLodFx::Kill | 0036a150 | Validate child handle, kill/reset only when valid, then base cleanup |
| CLodFx::Stop | 0036a1f0 | Set flags/timing and forward the incoming float unchanged to a valid child |
| CLodFx::IsLooped | 0036a5d0 | Load byte at original offset 0x88 |
| CLodFx constructor | 001a5460 | Child handle storage at 0x80/0x84; installs vtable 00443b50 |
| CNewFx::Kill | 001a2b00 | Base cleanup retained without substitution |

`effects.py` rebuilds production objects, probes actual Windows layouts, lifts
complete callees, and proves IsLooped plus four Stop handle partitions (null,
zero ID, stale ID, valid ID). Stop permits all finite normal binary32 values and
signed zeros, with checked child dispatch to the complete CNewFx::Stop body.
It explicitly excludes subnormal/extended EE encodings whose comparisons can
differ from SSE. This is an independent callee domain, not evidence that a
specific fighter configuration satisfies it. Kill is recovered and compiled;
its complete manager/pool/animation cleanup graph is not yet proved.

```powershell
python tools/function_verify/effects.py
python tools/function_verify/check.py --offline
```

The combined live runner requires the effect callee suite. Retained real exports
in `fixtures/effect-callees.json` supply offline proofs and negative controls for
incorrect returns, stale image identities and incorrect child dispatch.

## Fighter runtime inputs and configuration initialization

`fighter_runtime.py` connects the actor's behaviour list to complete Windows
registry resolution and original PS2 behaviour pointers. Compiler probes recover
the actor/list/entry fields; native object addresses deliberately differ from PS2
addresses. The domain partitions list counts 0, 1 and 2 across both registries.
In each live partition entry i uses slot i and all entries use the same selected
registry, with symbolic matching generations/reference counts and arbitrary
signed IDs/query. A separate two-entry partition has both handles null. Mixed
registries and mixed null/live entries remain outside this domain. IDs must be
strictly increasing. Live Ghidra decompilation/disassembly at `00103f60` confirms
the PS2 binary search; the Windows linear scan is equivalent in this domain.
An unsorted-list negative control fails, so sorting remains an explicit caller
obligation rather than an inferred runtime invariant.

The same input mapping executes complete `CActor::SetState` (`00102a00`) bodies
when state is unchanged idle (6) or run (9), and an explicit animation ID other
than -1 equals the current animation. Both lookup and the compiled resolver run
at their actual call sites. Every memory write must lie in the private stack;
all eight 128-bit PS2 saved registers must be preserved. Animation selection,
playback, virtual behaviour calls and state-changing paths remain reachable
frontiers outside this domain. This is a partial-domain proof, not a passing
whole-transition target. Windows private stack is `0x7c00..0x7fff`; PS2 private
stack is `0x7f00..0x800f`.

`fighter_configuration.py` executes the production Windows dynamic initializer
and all 108 `StateConfig` constructors, proving the complete resulting table
equals original PS2 bytes at `00434460` for arbitrary initial table memory.
It permits only table/stack writes. Live Ghidra `GetStateCfg` at `0031dd50`,
especially `0031dd64..0031dd74`, establishes the table address and eight-byte
stride. Compiler probes recover table size and member offsets. This establishes
initial configuration values (idle animation 12, run animation 21), rather than
assuming COFF BSS zeros are constructor postconditions. The runtime suite also
proves the complete accessor across all 108 states and virtual GetStateCfg
execution in configured unchanged idle/run transitions. Later table
immutability, animation-bank contents and configured effect-type closure
remain open.

The live entry domain reserves PS2 private stack `0x7e00..0x800f`: SetState's
`0x70`-byte frame nests InitState's `0x110`-byte frame and the accessor's
`0x10`-byte frame. The earlier unchanged-state domain retains its smaller
reservation. The stack extension grants no actor/global write permission.
InitState saves/restores f20 at `00313da0` / `00314d60`. The live domain seeds
its caller value with arbitrary 32-bit bits, and the executor checks explicitly
supplied callee-saved FPR storage at return alongside the eight saved GPRs.
A negative regression rejects an f20 clobber; an identity write passes.

```powershell
python tools/function_verify/fighter_runtime.py
python tools/function_verify/fighter_configuration.py
```

Both accept `--replay <artifact-folder>` for retained exports and are included
as separate results in the live blocker audit. `fixtures/fighter-runtime.json`
and `fixtures/fighter-configuration.json` retain actual production exports and
negative controls. Unreachable omitted fixture dependencies remain explicit
frontiers; no successful call summaries replace them. The initializer follows
only an acyclic unconditional graph and rejects conditionals or missing bodies.
Neither result increases whole-transition coverage in the combined validator.
The complete symbolic regression suite takes several minutes; CTest permits
15 minutes for offline checks and 30 minutes for the full live build/audit run.

## Outstanding whole-transition obligations

The required SetStandAnim/SetState fighter targets remain nonpassing in the
combined runner. Their full configured idle/run domains still require runtime
actor/behaviour/animation/global relations, configuration-derived effect type
closure, all reachable concrete effect implementations, and validated general
EE/VU arithmetic where reachable. Registry resolution is now composed at the
unchanged-state call site in the bounded domain above; its invariants still need
establishing for the remaining domains. Debug-validator proofs remain independent
until their entry invariants are established at transition call sites.
No full fighter-transition equivalence, full Kill proof, hardware-accurate
general EE arithmetic, or automatic reusable contract cache is claimed.

## All-state implementation progress (2026-11-04)

`fighter_cases.py` enumerates states 6 through 113 from the checked 864-byte
configuration table. The SetState inventory contains all 11,664 old/new pairs,
including 108 unchanged pairs. Three animation modes and two idle-loop
partitions produce 69,984 SetState cases; 432 SetStandAnim cases bring the
inventory to 70,416. Stable case hashes include configuration bytes; pair,
flag-family and deterministic shard selectors do not change the full inventory.
An inventory is an obligation list, not a passing proof.

Configured GetStateCfg execution now passes across all 108 states. Restricted
state-changing proofs pass for idle (6) to run (9) and the reverse, with an empty
behaviour list, null animation resources, three animation modes and two loop
partitions (12 cases). These execute complete selected bodies, compare mapped
fields and reject writes outside those fields and private stack storage. The
signed original byte representing idle-loop count is related to its Windows
32-bit field by sign extension. These results remain partial domains and do not
discharge whole fighter contracts. Live exact CBehaviourFighter execution now
passes idle (6) to run (9), and run (9) to idle (6), for configured animation,
zero idle loops and one registered behaviour in slot 0. All observed fields,
write limits, saved GPRs and incoming f20 are checked. Neither proof needs
numeric tolerance or a reference arithmetic profile. The domains still require
null animation controller/macro table and current animation from the old-state
configuration. Animation resources, effects and other entry domains remain open.

The first live runs took 734.1 seconds (four paired outcomes, idle to run) and
676.5 seconds (one paired outcome, run to idle). Register-expression
simplification reduced the subsequent idle-to-run run to 271.2 seconds; the
retained run-to-idle regression passes in 251.7 seconds. These are partial-domain
measurements, not a full-matrix estimate. `fixtures/fighter-state-entry.json.gz`
retains the actual complete Windows/PS2 exports and compiled image. The
`test_fighter_state_entry.py` replay checks live run-to-idle execution and still
requires `full_fighter_contract=False`.

```powershell
python -m unittest discover -s tools/function_verify -p test_fighter_state_entry.py
```

`fighter_behaviour.py` prepares the compiled InitState/TermState closure. Audited
Clang switch tables are relocated from COFF and every target must be an actual
instruction boundary inside its owning function. The exporter includes each
checked case body. Checked internal indirect branches retain destination
membership checks. Deferred function bodies remain explicit frontiers if
execution reaches them; they are never successful call summaries.

Original TermState at `00313290` was compared through live Ghidra decompilation
and disassembly to recover these source behaviours:

- Defend-fall terminates its case; defend-land writes cutscene delta time to
  owner offset `0x6bc` (`00313500..00313510`).
- Hit-sway clears fight flag bit 0, enables looking-at and resets the layer
  warper (`00313560..00313594`).
- Hold-hit calls the return-only helper at `00310620` (`00313548..00313558`).
- Grab-action-miss-end resets the warper, sets flag `0x36c` bit 0 and stores
  turn-speed bits `0x41bc7edd` (`00313950..00313980`).
- Ride-exit clears both collision links through the helper at `00309190`;
  the adversary pointer is at original actor offset `0x350`.
- Flip cleanup compares destination-state flags for ordinary destination
  states as well as the sentinel (`00313ae8..00313b34`).

`fighter_transitions.py` writes per-case results and progress, and runs each
proof in an isolated worker with a default 600-second wall-clock limit.
Timeout, unsupported execution, missing cases and restricted contracts keep the
report incomplete.

The 600-second limit is a resource bound, not a measured completion estimate.
A live idle-to-run benchmark with a 180-second bound timed out. After the memory
optimization a longer attempt completed exploration in 274.5 seconds and
correctly rejected a missing PlayAnim body (`00103db0`); preparation now includes
its actual original export. The bounded live proofs above now provide initial
measurements; no full-matrix finish estimate has been established. The latest offline regression
run passed 203 tests in 338.3 seconds while live cases ran concurrently; the
additional retained state-entry regression also passed independently.
The earlier memory-optimization regression passed 199 tests in 260.6 seconds,
versus 454.8 seconds for the preceding 195-test run. This is regression timing,
not time to discharge all-state contracts.

For example, using previously prepared exports:

```powershell
python tools/function_verify/fighter_transitions.py --artifacts out/verification/fighter-live-behaviour/fighter-behaviour --inventory-only
python tools/function_verify/fighter_transitions.py --artifacts out/verification/fighter-live-behaviour/fighter-behaviour --state-pair 6:9 --live-behaviour --case-timeout 600
```

## Pinned COP1 interpreter reference

`pcsx2_oracle.py` builds the actual PCSX2 COP1 interpreter from revision
`1f67c53b6723fcd6a90041a5b296f0e34a6a3222`. Its manifest records canonical
upstream source/header hashes, driver and boundary hashes, compiler/settings,
binary hash and limitations. `capture_pcsx2_vectors.py` records 6,741 actual
observations for 21 operations, including destination, accumulator and FCR31.
`ee_interpreter.py` matches these observations, including exceptional encodings
and guest status bits. This interpreter profile is distinct from EE hardware,
the PCSX2 recompiler and VU arithmetic. Only explicitly integrated instruction
handlers can use it; unsupported instructions continue to fail closed.

Strict policy remains the default and rejects conditional arithmetic results.
The optional `pcsx2-reference` policy requires the pinned manifest, verifies its
artifacts and replays the captured vectors against the executable. Reference
dependencies propagate to parent results; missing tags and attempts to promote
the interpreter profile to another backend or hardware are rejected. Reference
results cannot establish full contracts while animation/runtime obligations
remain open. General VU arithmetic and remaining EE instruction integration
still require implementation and validation.

Production instruction regressions now retain all nine floating-point
operations from InitState at `00313d80`, including three ABS.S instructions in
JAL delay slots (`00314824`, `0031484c`, `00314a40`). Each is checked against
captured upstream results and FCR31. Call delay slots require the actual owner
and delay bytes, one adjacent four-byte slot and matching p-code/operands;
unsupported composite forms remain rejected. ABS.S/NEG.S handlers preserve
payload bits and clear the interpreter's cause flags under explicit reference
opt-in. These checks establish conditional instruction semantics, not callee or
whole-function equivalence.

Actual R5900 raw p-code exposed a non-linear floating-register layout (`f1` at
`0x1000`, `f0` at `0x1004`). Register checks now use only the recovered mappings
for f0, f1, f2, f12 and f14. Other encodings remain unsupported until their
storage is recovered; a synthetic linear mapping is not treated as evidence.

## Explicit coverage domains and resumable jobs

Transition jobs now distinguish `null`, `controller-only`, `macro-table-only`,
`populated-reset`, and `populated` resource profiles. The first three retain
opaque non-null pointer identities where appropriate; a reachable dereference
still needs a bounded memory region. `populated-reset` supplies an empty macro
lookup table and one stopped, real animation layer. It executes original and
compiled reset bodies and restores callback/user pointers from actual callback
table evidence. It does not cover descriptor playback, morphing, event tracks or
callback invocation. `populated` remains explicitly unsupported until its full
bank/controller/layer input relation is established.

`SetStandAnim` cases now execute their full bodies. Idle dispatch uses the actual
`CActorMovable::SetState` target recovered from the fighter vtable, rather than
substituting base dispatch. Layout probes include stand animation and movable
weight. Original evidence includes SetStandAnim `0031ba50`, movable SetState
`0011e7c0`, macro lookup `00103e70`, animation reset `00180450`, layer reset
`00241460`, and callback/user setters `002426f0` / `00242680`. Callback storage is
read from `0040e760`. Compiler float/vector constants are checked against the
bundled image before they enter runtime memory.

Use `--function`, `--animation`, and `--idle-loops` to select narrower proof
partitions without changing the original 70,416-case inventory. Repeat
`--resources` to compare resource domains. For example:

```powershell
python tools/function_verify/fighter_behaviour.py --output-root out/verification/coverage-animation
python tools/function_verify/fighter_transitions.py --artifacts out/verification/coverage-animation/fighter-behaviour --state-pair 6:9 --animation configured --idle-loops zero --dormant-actor --resources populated-reset --output-root out/verification/coverage-animation --resume
python tools/function_verify/fighter_transitions.py --artifacts out/verification/coverage-animation/fighter-behaviour --state-pair 6:6 --function SetStandAnim --dormant-actor --output-root out/verification/coverage-animation
python tools/function_verify/fighter_transitions.py --artifacts out/verification/coverage-animation/fighter-behaviour --sequence 6:9:6 --dormant-actor --output-root out/verification/coverage-animation
```

Sequences accept 2..8 states, use configured animation and zero initial idle
loops, and retain actual successor heap, globals, path conditions and processor
state. Later entry states are proved from their predecessors. Only external ABI
call-entry storage is reset. Fighter composition checks every recorded store
against mapped fields/private storage, using the same write-frame proof as
single-transition execution; generic composition retains its full-array check.

Job identities include the case, resource/behaviour profile, complete artifact
and tool hashes, arithmetic policy/reference approval, sequence and timeout.
`--resume` reuses only completed, policy-valid results with matching identities;
mismatches, unsupported results and timeouts are rerun. Inventory-only runs do
not overwrite retained executable results. Inputs are rechecked around each
worker, and changing them requires restarting the run. Watcher inputs now include
Animation and compressed retained fixtures.

`--registry transient` selects a single live fighter in the transient registry.
`--behaviour-profile fighter-general-one` and `fighter-general-two` use arbitrary
queries and valid symbolic handles across both two-slot registries. Handles can
be null, share either of two concrete fighter objects, or mix registries. Both
objects have checked fighter dispatch and the same mapped owner. IDs remain
strictly increasing; this does not establish registry allocation/release or
arbitrary list-capacity invariants. The independent runtime suite includes these
general binding partitions for lookup and unchanged SetState.

Reports separate raw execution verdicts from whole-contract completion for each
domain. A legacy `full_fighter_contract=True` flag cannot discharge a contract:
unrestricted domain evidence and all runtime, resource, configuration, dispatch,
effect, processor, return, observation, memory-frame and preservation obligations
are required. Current bounded profiles remain incomplete, including when every
selected proof returns `equivalent`. Successful restricted runs therefore still
exit nonzero; `--inventory-only` is the explicit audit-only exception.

Fresh production exports passed configured idle-to-run and run-to-idle with one
registered live fighter, zero idle loops, and the `populated-reset` resource
profile (four paired outcomes and one paired outcome respectively). Both are
exact, with no reference arithmetic or tolerance. A full unchanged-state run
also passed the two-entry mixed symbolic behaviour domain. The retained
`fighter-animation-reset.json.gz` fixture covers reset, stand dispatch and a
persistent dormant idle/run/idle cycle, with a missing-layer-write negative
control. Full animation playback/morphing, effect cleanup, state-family closure,
hardware EE/VU semantics and complete matrix proofs remain outstanding.

The complete 218-test regression run passed in 994.7 seconds while live proofs
ran alongside it. This includes the formerly separate state-entry replay and
the expanded mixed-registry partitions, so it is not comparable to the earlier
203-test timing. The runner now separates fast checks from 15 expensive retained
fighter tests; two partition-accounting tests brought the inventory to
220 tests (205 fast, 15 retained). Default offline checking still runs all tests.

The next playback prerequisite is now independently executable through
`animation_descriptor.py`. Fresh production exports prove the complete
`edAnmStateParser::BuildDesc` body against original `00241490` (decompilation and
all 24 instructions inspected). Same-TU compiler probes recover the parser and
descriptor layouts. Four partitions cover one through four offset-table words
in a disjoint 128-byte bank, arbitrary animation IDs, arbitrary header bytes,
and a symbolic aligned final header offset. The header pointers may alias.
Initial descriptor header pointers are null; scalar outputs start arbitrary.
Each proof checks all descriptor fields, relocated pointer identities, return
identity, unchanged parser/bank storage and every store's frame. No callee
summary, arithmetic reference or numeric tolerance is involved.

```powershell
python tools/function_verify/fighter_behaviour.py --output-root out/verification/coverage-playback-prerequisites
python tools/function_verify/animation_descriptor.py --artifacts out/verification/coverage-playback-prerequisites/fighter-behaviour --output out/verification/coverage-playback-prerequisites/descriptor-report.json
python -m unittest discover -s tools/function_verify -p test_animation_descriptor.py -v
```

The retained `animation-descriptor.json.gz` contains the complete production
image and original exports with checked executable identities. Negative controls
detect a missing output store, a wrong header pointer and a wrong returned
descriptor pointer. Missing compiler layout evidence fails closed.

The compiled Windows debug-validator contract now also covers three spaced
track records at bank offsets 12, 32 and 52. Intervening bytes are arbitrary but
equal in the live bank and snapshot; track IDs range from -1 through 2. All three
lookups prove their returned pointer, exact history entry and wrapping counter
update while preserving other memory. Preparation runs both the existing
two-entry domain and this three-entry domain. These diagnostics are observable
Windows-only effects and still need to be included when composing playback.

These are prerequisite proofs, not populated fighter playback proofs.
`resources=populated` deliberately remains unsupported. Layer SetAnim
`00240530` and macro Initialize `00241ce0` were inspected for the next closure:
initialization introduces duration subtraction, comparisons and potentially
division, followed by morphing/event-track obligations for broader inputs.
That increment brought the regression inventory to 227 tests: 212 fast and 15 retained.

Validation for this increment: all 212 fast tests passed through CTest in
295.3 seconds; all four fresh descriptor partitions passed; the fresh live
idle-to-run and run-to-idle populated-reset proofs passed with four and one
paired outcomes respectively. Both live proofs remain strict with no arithmetic
reference or numeric tolerance, and both retain their bounded-domain limits.

Stopped-layer playback now has a composed proof in `animation_playback.py`.
It executes the complete original and production-compiled descriptor parser,
then layer SetAnim with the actual macro Initialize body, then repeats SetAnim
for the same animation. Heap and processor state survive between invocations;
the descriptor is never reconstructed as a successor input. Original evidence
is BuildDesc `00241490`, SetAnim `00240530` (all 74 instructions inspected), and
Initialize `00241ce0` (decompilation and disassembly inspected). Same-TU compiler
probes recover every named layer member, including nested macro state.

The bounded domain has one stopped layer, a one-entry macro header, and a
one-entry key-data pointer array. Animation IDs, header flags and initial scalar
layer fields are arbitrary. The five `(start, end)` timing partitions are
`(0, 0)`, `(0, 2)`, `(1, 2)`, `(2, 1)` and `(-1, 1)`. These exercise both header
flag paths, duration subtraction, negative-time/duration clamping and conditional
division. Opaque nonnull actor and callback pointers are copied without access
or invocation. All named layer outputs and inactive-member preservation are
checked against explicit expected values; parser/bank/key inputs remain
unchanged. Every write must stay within mapped fields or the ABI stack.

These proofs require explicit opt-in to the pinned PCSX2 COP1 interpreter model
and report `conditional-equivalent`, with its revision recorded. They use exact
output equality and no numeric tolerance. This is not a hardware EE theorem.
Initialize's division at `00241d94` supplies byte-backed evidence for `f20`
storage at `0x1054`, extending the strict operand validator only for that
observed register. A changed register operand is still rejected.

```powershell
python tools/function_verify/fighter_behaviour.py --output-root out/verification/coverage-stopped-playback
python tools/function_verify/animation_playback.py --artifacts out/verification/coverage-stopped-playback/fighter-behaviour --output out/verification/coverage-stopped-playback/playback-report.json --reference-model pcsx2-cop1-interpreter:1f67c53b6723fcd6a90041a5b296f0e34a6a3222
python -m unittest discover -s tools/function_verify -p test_animation_playback.py -v
```

The retained `animation-playback.json.gz` contains the validated complete
production bundle and original exports. Eight regression tests cover the five
partitions, explicit reference policy, missing layouts/callees, a lost compiled
playback store, a lost original callback pointer, a corrupted original duration,
and operand validation against the original `f20` division instruction.
That increment brought the regression inventory to 235 tests: 220 fast and 15 retained.

Full fighter `resources=populated` remains unsupported. This increment closes
descriptor-to-stopped-layer initialization and same-animation replay for the
stated domain. Nonzero key-array indices, multi-key UpdateAnimParams, direct/raw
and normalized initialization, morphing, frame advance, event tracks, callbacks,
and composition of Windows debug-validator effects remain open. Effect cleanup,
state-family closure, broader EE/VU semantics and full matrix proofs also remain
open; these bounded conditional proofs do not discharge their obligations.

Validation for this increment: fresh production preparation and all five
composed timing proofs passed. The aggregate report passes the shared reference
policy validator and binds the complete artifacts, layouts and Python verifier
implementation with `inputs_sha256`. All 220 fast regressions passed through
CTest (311.0 seconds in the Python suite). The 15 retained fighter regressions
were not rerun in this increment. No game source code changed.

The next increment extends `animation_playback.py` through the real multi-key
`edAnmMacroAnimator::UpdateAnimParams` body at `00241b40`. Its decompilation and
disassembly were inspected before adding the original entry to the production
export closure. There are still no callee summaries or synthesized playback
outputs between invocations.

All playback domains now contain three distinct key records and a native
pointer array (8-byte elements on Windows, 4-byte elements on PS2). Single-key
partitions select indices 0, 1 or 2; unselected records have different times,
so accidentally forcing index zero cannot satisfy the expected outputs. Blend
domains contain an empty list, two terms, three permuted terms, two references
to the same record, zero weights, negative weights or cancelling weights. The
actual loop reads both packed index and weight arrays and executes its original
reverse traversal, accumulation, conditional normalization and clamping.

The blend inputs require header bits `0x80000004` clear: the high bypass flag
and callback-enable flag remain outside this weighted-data domain. Other header
bits and both animation IDs remain arbitrary. Actor and callback pointers are
nonnull opaque identities. A mutation that reaches callback dispatch must fail
as an unresolved frontier. The single-key domain retains arbitrary header flags.
The numeric inputs are fixed small dyadic partitions whose intermediate and
normalized values are exactly representable in binary32. Independent expected
output checks use exact rational constants; the real bodies still execute all
arithmetic under their respective processor semantics. Results retain the pinned
COP1 reference dependency, exact equality and `full_fighter_contract=false`.

The fresh runner selects 28 cases: every combination of the five prior timings
and three single-key indices, seven blend profiles at timing `(1, 2)`, and six
additional blend/timing partitions. Repeated-first-record blends with `(-1, 1)`
and `(2, 1)` exercise negative blended start and duration clamps respectively.
This is a selected finite domain, not all possible weights or key counts.

```powershell
python tools/function_verify/fighter_behaviour.py --output-root out/verification/coverage-blended-playback
python tools/function_verify/animation_playback.py --artifacts out/verification/coverage-blended-playback/fighter-behaviour --output out/verification/coverage-blended-playback/playback-report.json --reference-model pcsx2-cop1-interpreter:1f67c53b6723fcd6a90041a5b296f0e34a6a3222
python -m unittest discover -s tools/function_verify -p test_animation_blended_playback.py -v
```

`animation-blended-playback.json.gz` retains the validated production image and
original bodies. Ten new regression tests cover the new domains, missing
UpdateAnimParams, forced callback dispatch, lost nonzero indexing, lost duration
accumulation, lost normalization, invalid inputs and strict operand checks.
Original instructions `00241c1c` (`mul.S f0,f0,f3`) and `00241c38`
(`add.S f4,f4,f3`) establish `f3=0x1008` and `f4=0x1014` storage; changing their
p-code operands remains unsupported. The regression inventory is now 245 tests:
230 fast and 15 retained.

Full fighter populated resources remain unsupported. General blend weights and
key counts, callback invocation, raw/normalized initialization, morphing, frame
advance, event tracks and Windows debug-validator composition remain open,
alongside effects, state-family closure, broader EE/VU semantics and full matrix
proofs. The bounded blend proof does not discharge those obligations.

The next closure was audited read-only at MorphingInitDT `002411d0` and
MorphingStartDT `002410e0`. A different-animation request first queues the next
descriptor using transition-record/default-stage settings. Morph startup then
introduces normalized initialization and integer milliseconds converted to
float: original `cvt.s.W` instructions at `00241108` and `00241178`, followed by
division by 1000. The scalar reference interpreter has conversion operations,
but the strict instruction adapters still need byte-validated EE and SSE
conversion handlers before these paths can execute. Stage configuration and
transition-record observations must also be established rather than using
uninitialized globals. Queueing, startup and interrupted-morph sequences are
the next composition steps, before integrating the Windows diagnostic effects
and the full actor animation path.

Validation for the blend increment: fresh production preparation passed, all
28 fresh composed proofs returned `conditional-equivalent`, and the aggregate
passed the shared reference-policy validator with no tolerance. All ten new
targeted tests passed in 417.9 seconds. The full fast CTest tier then passed all
230 tests in 749.0 seconds in the Python suite (750.1 seconds total). The 15
retained fighter tests were not rerun. No game source code changed.

The morph/callback increment executes MorphingInitDT `002411d0` and
MorphingStartDT `002410e0` through real queue, startup, same-next-animation
replay and interruption calls. Both descriptors share one single-key header;
animation IDs are distinct symbolic values. The current transition-record count
is zero. The first three default-stage settings are explicit runtime inputs,
not inferred from BSS or constructor assumptions: `(-6, -1)`, `(-4, 0)`,
`(500, 500)` and `(1000, 1000)`, with duration `0.25`. These cover retaining
current timing, raw restart, normalized startup and millisecond conversion.
Header flags remain arbitrary. All mapped layer fields, read-only inputs,
state/time outputs and writes are checked after each invocation. No memory or
processor state is reconstructed between calls.

The strict adapters now accept the observed signed 32-bit `cvt.s.W` and
`CVTSI2SS` shapes. EE instructions at `00241108` and `00241178` validate the
format/function bits and actual f1 source/destination storage. Windows memory
operands validate base register, signed displacement, width, load linkage and
destination against the bytes; unimplemented SIB/RIP-relative/64-bit conversion
forms fail closed. Conversion instructions are routed through processor
semantics in both executors. SSE conversion uses MXCSR rounding and sticky
precision with unmasked exceptions rejected. The EE conversion requires the
explicit pinned COP1 interpreter model and preserves its status behavior.
`sse-conversions.json` retains 66 independent native oracle vectors, including
signed boundaries, ties, all four rounding modes and pre-existing precision.
The EE adapter is also checked against the retained pinned interpreter vectors.

Callback coverage now includes actual invocation, not just copying opaque
pointers. The populated callback domain executes UpdateAnimParams `00241b40`,
layer-zero callback `0017f470`, and CActorFighter::AnimEvaluate `0031baa0`.
Concrete vtable slot evidence comes from the real COFF table relocation and PS2
table at `00441a90`, slot `0xa0`; runtime dispatch is checked before entry.
For original animation `0x27`, count two and fighter blend factor `0.25`, the
fighter body changes the live bank weights from `(1, 1)` to `(0.75, 0.25)`
before the real blend loop executes. Both direction flags are covered, with
other permitted header bits arbitrary. The complete actor, controller, vtable
and constant storage are preserved. Other callbacks and special fighter
animation branches remain outside this selected domain. The bundled acosf
import slot has no runtime memory region or callable implementation: reaching
it is still a frontier.

Actor playback integration additionally executes CActor::PlayAnim `00103db0`,
GetIdMacroAnim `00103e70`, CAnimation::PlayAnim `001801b0`, meta SetAnim
`00242230`, descriptor construction and layer initialization. One sorted macro
lookup entry maps an arbitrary original ID other than `-1` to macro zero;
the layer starts stopped, the controller has one layer, and the bank track ID
is `-1`. Repeating the actor request retains the actual started state. The
Windows Debug validator and its vector helpers execute in this call graph.
Its history counter is arbitrary, including wraparound. Its exact ring entry,
counter increment, other history bytes and read-only diagnostic storage are
observed separately, without inventing PS2 diagnostic writes. Controller track
data reset and actor current/previous IDs have independent expected-value checks.

The retained production fixture is `animation-morphing-callback.json.gz`.
`test_fp_conversion.py` belongs to the fast tier; the longer composed proofs
and negative controls in `test_animation_morphing.py` have a separate exhaustive
`animation` regression tier and `FunctionVerifyAnimation` CTest entry. Existing
fast and retained tiers keep their own limits. All-test discovery includes all
three disjoint tiers.

```powershell
python tools/function_verify/fighter_behaviour.py --output-root out/verification/coverage-actor-playback
python tools/function_verify/animation_morphing.py --artifacts out/verification/coverage-actor-playback/fighter-behaviour --output out/verification/coverage-actor-playback/composition-report.json --reference-model pcsx2-cop1-interpreter:1f67c53b6723fcd6a90041a5b296f0e34a6a3222
ctest --test-dir out/build/x64-verify -R '^FunctionVerifyAnimation$' --output-on-failure
```

These are bounded conditional proofs with exact output equality and no numeric
tolerance. They do not establish the full fighter contract. In particular,
`resources=populated` is still rejected: the state-transition verifier has not
yet composed its live behaviour/registry domain with these playback inputs.
Nonempty transition records, different banks/headers during morphing, frame
advance, event-track stop/reset/playback, remaining layer callbacks, other
fighter AnimEvaluate branches, effects cleanup and broader EE/VU coverage
remain open. Next connect the live idle/run SetState closure to the populated
actor/controller domain and preserve the now-observed Windows diagnostics;
then extend the bank and event-track domains. No game source changed.

Diagnostic observations use proved region views to keep symbolic ring addresses
from inflating every gameplay observation. `regional_initial_stores` opts into
dropping an initial store from a region view only after proving, under the entry
domain alone, that its address lies outside that region. Unknown ownership
retains the store. Later branch conditions cannot license a cached omission.
Gameplay observations separately omit only stores certified wholly inside
private regions; actual carried memory is unchanged. Diagnostic fields and the
complete write frame are still checked against actual memory/writes. This
avoids comparing a 32-entry symbolic ring as one large byte concatenation.
Two aliasing regressions check exact agreement with the full array and prevent
branch-condition leakage. The inventory is now 264 tests: 237 fast, 15 retained
fighter and 12 animation compositions/fault controls.

The next live state input can use the now-probed macro-entry layout: configured
idle/run IDs are 12 and 21, so a sorted two-entry lookup can map both to the
bounded bank. Start with one stopped layer and no event tracks, then compose
live SetState in both directions before adding cycles or an initially playing
layer. Export the original GetAnimEventTrackID body at `00242100` when that
path becomes reachable. Preserve the Windows lookup-counter/history contract
across Reset as well as PlayAnim; its extra diagnostics must not be hidden.

Validation for this increment: fresh production preparation and all six
compositions passed (four morph settings, fighter callback blending and actor
playback). The aggregate passes the shared reference-policy validator and binds
the current verifier and complete artifacts with `inputs_sha256`
`cac0e326d374cf1bb8687b4d48ac539f78e54060303f839f50acf830f8aa4625`.
The retained fixture was checked equal to the fresh production artifacts.
All 237 fast tests passed through CTest (850.4 seconds in Python, 852.0 total),
including five conversion checks and two new aliasing checks. All 12 animation
tests then passed through `FunctionVerifyAnimation` (1142.3 seconds in Python,
1143.1 total), including lost actor, morph, callback-weight and diagnostic
counter writes, missing bodies and wrong callback targets. Actor observations
are proved as individual conjuncts under the same feasible input domain; no
extra input constraint or numeric tolerance is introduced. All six results stay
conditional on the explicitly approved pinned COP1 interpreter, with the full
fighter contract false. The older 15 retained fighter tests were not rerun.
