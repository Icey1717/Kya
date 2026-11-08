# Function verification pilot

For a codebase-wide inventory and compiled survey of functions below 20 source
lines, see [the short-function batch runner](SMALL_FUNCTIONS.md). Its report
separates proved matches, inferred field correspondences, counterexamples and
unsupported functions.

**Compiled Windows comparison now covers twenty-two functions**, including
branches, direct calls, a bounded loop, bytecode readers, save state, and HUD
fields, collision reset, and fighter cleanup/state predicates. Full combat
state transitions still have [documented blockers](COMBAT.md).
See [compiled-code workflow](COMPILED.md) and
[broader validation coverage](VALIDATION.md) for the combined runner, change
watcher, CTest integration, and CI entry point. It uses production
objects, compiler-derived layout constants, and Ghidra P-code on both sides.
The thirteen source-model cases below remain a separate suite.

This pilot exports **live raw P-code** from the Totala Ghidra 10.0 bridge and
compares thirteen functions against their current C++ source bodies
with Z3. No disassembly adapter or decompiler text is used as executable semantics.

From the repository root:

```powershell
python -m pip install --target out/verification-deps -r tools/function_verify/requirements.txt
python tools/function_verify/client.py status
python tools/function_verify/verify.py
python -m unittest discover -s tools/function_verify -p test_*.py
```

Install the extension using [these build instructions](../ghidra-module/totala/README.md).
Open `SLES_514.73` in Ghidra and enable Totala. `--port` selects a different bridge
port; `--output` selects an artifact directory (default `out/verification`).
The external MCP wrapper need not advertise the command: `client.py` sends the
same read-only request directly to the bridge.

## What is checked

| Function | PS2 address | Current source mask |
| --- | --- | --- |
| CActor::IsKindOfObject | 00100460 | 1 |
| CActorMovable::IsKindOfObject | 0011e060 | 3 |
| CActorAutonomous::IsKindOfObject | 00124180 | 7 |
| CActor::IsLookingAt | 001003f0 | 0x2000 (read) |
| CActor::SetLookingAtOn | 00100400 | 0x2000 (OR write) |
| CActor::SetLookingAtOff | 00100410 | 0xffffdfff (AND write) |
| CInventoryInterface::HasChanged | 001dc7b0 | conditional clear, 2 paths |
| CActorMoney::GetType | 00362f30 | thresholds 2 and 6, 3 paths |
| CActor::GetBehaviourFlags | 001022b0 | symbolic indexed table, sentinel -1 |
| CActorAutonomous::GetBehaviourFlags | 00123c50 | direct call through thunk, private stack frame |
| CFixedTable<CActor,4>::IsInList | 00131f10 | bounded search loop with early return |
| CFixedTable<CActor,64>::Swap | 0011ad00 | symbolic writes, equal/distinct indices |
| CFixedTable<i,64>::Swap | 002000f0 | symbolic writes, equal/distinct indices |

The checker reads the source on each run, accepting only the exact unsigned
`return (kind & literal) != 0;` body grammar. It assumes `ulong` is unsigned
64-bit, as declared in `port/include/port.h`. This is a deliberately restricted
source-semantics check, **not a compiled Windows binary equivalence proof** or
a general C++ parser. Source preprocessing, declarations, and compiler behavior
are outside the model; changes there require review. The body hash is recorded.

The raw P-code executor uses explicit R5900 ABI storage: low 64-bit `a1` input,
low 64-bit `v0` result, and return via `ra`. It accepts a small integer opcode
set and one straight-line return. Ghidra includes the delay-slot operations in
the owning return instruction; the exporter excludes the separately listed slot
to avoid executing it twice. Correct lifting by Ghidra is an explicit assumption.

The three field cases additionally accept exact getter/setter mask grammars.
They map the source `uint flags` to four PS2 bytes at `this+8`. This mapping is
an explicit reviewed assumption, not a validation of compiled Windows layout.
The object is modeled at a fixed aligned address, with 16 arbitrary symbolic
bytes. Only concrete in-bounds accesses are accepted; other pointers and pointer
aliasing are outside scope. The R5900 RAM space selector is pinned to `0x1a1`.
Loads and stores use little-endian byte semantics. Setters compare the complete
final memory array (including untouched bytes); the getter compares its low
32-bit return and checks that memory is unchanged. Scratch `v0`, `v1`, and `a1`
writes are allowed; covered register subrange reads use bit extraction.

Calls, loops, floating point, unknown inputs, unsupported
aliases, and other register writes fail closed. Thus there are no
loop bounds or sampled inputs in these checks: `unsat` covers all 2^64 arguments
for the predicates and all initial object-byte values for the memory cases,
under the documented assumptions. This narrow support is intentional.

`report.json` records function identity, source body and original executable
hashes, solver version, verdicts, assumptions, and counterexamples. Each raw
export is retained alongside it. A deliberately wrong mask (`mask ^ 1`) must
produce a mismatch for each function. The memory cases also require a wrong
field offset to produce a mismatch. These controls do not edit game source.
Exit status is nonzero if any function mismatches or is inconclusive.

The initial live run on 2026-10-29 matched all three functions and detected all
three wrong-mask controls (`kind = 1`). The expanded run matched all six functions,
with nine negative controls detected. Twelve unit tests cover equivalence,
negative controls, omitted stores, unknown memory spaces, out-of-bounds loads,
register subrange reads, and fail-closed behavior. The memory snapshots in
`fixtures/` are real exports retained for offline regression testing.
`pilot_snapshot.json` is the original MCP disassembly/decompilation reference
record; it is not used by the verifier.

## Next extensions

### Acyclic branch expansion

`paths.py` enumerates direct RAM-target branches and conditional branches, preserving
the operation order within instructions (including lifted delay-slot operations).
Each conditional edge adds a symbolic guard. Z3 proves the guards cover all inputs,
then searches for a return or memory mismatch on any guarded path. Loops, internal
P-code branches, external targets, and control transfers in the middle of an
instruction are rejected. Limits are 32 paths and 128 instructions per path;
exhausting either returns inconclusive, never a bounded equivalence claim.

`branch_cases.py` adds audited source models with exact whitespace-normalized
source checks. `HasChanged` maps signed `bDirty` to four bytes at offset 12 in a
16-byte object; `GetType` maps unsigned `moneyValue` to offset 0x1d0 in a 0x1d4-byte
object. These are explicit mappings, not compiled layout proofs. All object bytes
are symbolic. Both functions compare low 32-bit return values and complete final
memory. Negative controls omit the dirty-field clear and change threshold 2 to 3.
Both are detected. The expanded live run matched all eight cases across their
full modeled input domains. Seventeen regression tests pass, including rejection
of cycles, external branch targets, and exhausted exploration limits.

### Further work

### Symbolic indexed access

`indexed_cases.py` checks `CActor::GetBehaviourFlags` over signed states -1, 0,
and 1. The source assert supplies an upper bound; the lower bound is an explicit
valid-input assumption to exclude undefined negative indexing. The two uint32
table entries at PS2 address 0x427d80 are arbitrary symbolic bytes. This proves
the selection logic independently of the table's current contents, not agreement
between the initial global values in the two builds.

The executor now supports integer shifts and symbolic memory addresses. Before
each symbolic load/store, Z3 must prove the complete access lies in the modeled
region under the input precondition and guards accumulated before the access.
A failed or timed-out bounds proof is inconclusive. Bounds proofs do not assume
conditions encountered later on a path. Memory remains byte-addressed and final
memory is checked for unintended writes.

The ninth live case matched on both paths. A sentinel mutation was detected at
state -1, and removing the index restrictions failed the address-bounds proof.
Twenty-two regression tests include wrong strides that either produce a mismatch
or fail bounds validation. No game source changes or plugin reinstall are needed.

### Direct calls and stack frames

`call_cases.py` links the exported caller at 00123c50, the tail-jump thunk at
0011e790, and the base implementation at 001022b0. Only this explicit call graph
is accepted. The graph must use the same executable hash and compatible exports.
It has one direct call site; indirect calls, additional nested calls, and recursive
control flow remain unsupported. This is limited inlining of real exported
instructions, not a return-value-only call summary.

The source model is guarded by exact whitespace-normalized checks of both source
bodies. Valid signed states are -1 through 2. ACT's two entries at 0x427d80 and
AUT's one entry at 0x428e20 are arbitrary symbolic uint32 values. The explicit
mapping includes the original compiler's bias for `state - 2` indexing.

The caller starts with modeled sp=0x8010 and ra=0xdead0000. A private 16-byte
frame at 0x8000 may change. Every other memory address must be preserved.
Each load/store must be proven within an approved table or stack region.
The callee's return link is checked before returning to the caller; final sp and
ra restoration are also solver-checked under each path guard. Fixed modeled
stack addresses and the audited ABI are assumptions; arbitrary stack relocation
and faults are not modeled.

The tenth live case matched on all three paths; a wrong AUT table mapping was
detected at state 2. Twenty-seven regression tests pass, including broken stack
restoration, incorrect return links, unknown calls, and mixed-executable exports.

### Bounded loops with a cutoff proof

`loop_cases.py` verifies the four-entry `CFixedTable::IsInList` specialization
against its current template body in `Types.h`. The explicit mapping is a
20-byte PS2 object: signed count at offset 0, four 32-bit pointer values at
offset 4. The input domain is count 0..4 with arbitrary entries and an arbitrary
32-bit query sign-extended into a1. This models pointer identity; it does not
dereference actors or establish host pointer representation equivalence.

`paths.expand` accepts an optional instruction-visit bound. At that bound it
retains the prefix as a marked cutoff path rather than silently discarding it.
The loop verifier must prove every cutoff guard unsatisfiable within the input
domain, prove returning-path coverage, then compare return values and all memory.
This establishes completeness for the stated domain; it is not a proof for
unbounded table sizes. A reachable cutoff or solver timeout is inconclusive.
Other verifier cases still reject cycles by default. Path/instruction limits
remain enforced; loop visit bounds are restricted to 1..16.

The eleventh live case has nine syntactic returning paths and one cutoff path
(some returning paths are infeasible). Four visits suffice and the cutoff is
proven unreachable. Three visits produce an inconclusive result. Omitting the
last entry from the source model produces a counterexample with count 4.
All 33 regression tests pass. No plugin reinstall is needed.

### Aliasing and ordered symbolic stores

`alias_cases.py` verifies two swap specializations with two symbolic 32-bit
index arguments (a1 and a2), arbitrary four-byte entry values, and a 260-byte
PS2 object. The 64 entries start at offset 4. Z3 proves bounds for every symbolic
load and store and compares complete final memory against a swap using saved
initial values. Equal indices are allowed; the expected effect in that case is
no change. Unrelated bytes, including the count, must be preserved.

The domain is indices **1..63**, rather than 0..63. The current source template
uses `(this->aEntries + a + -1)[1]`; index zero would form an intermediate pointer
before the array. The source model does not claim defined C++ semantics there.
The recovered pointer specialization also has index-parameter type ambiguity;
this check explicitly models the machine arguments as integer indices and entry
values as pointer-identity bits. It does not establish host pointer layout or
template-instantiation correctness. No source repair is performed by this pilot.

Both live functions matched. Losing the temporary value or omitting a store
produces mismatches for distinct indices; the lost-temporary mutation correctly
has no effect when both indices are equal. All 37 regression tests pass.

Before expanding to general functions, add compiled-PC input (with explicit ABI
and object mapping), instruction model validation, and symbolic pointer support.
Then add loop reasoning and call summaries. Unsupported
operations must remain inconclusive rather than becoming unconstrained no-ops.
