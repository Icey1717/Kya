# Collision/combat support audit

Five concrete targets now compare production Windows P-code with original PS2
P-code. Full state-transition analysis still has blockers; passing this suite
does not mean the entire collision or combat subsystem is verified.

| Target | PS2 entry | Support exercised | Result/domain |
| --- | --- | --- | --- |
| CActorFighter::_EndFighterFlip | 00328300 | Symbolic pointer to a separate collision object; flag and pointer writes | Equivalent for aligned collision pointers in 0x4000..0x5000, arbitrary uint32 flags and attached pointer identities |
| CActorFighter::IsInStaggerSequence | 0031cdb0 | Signed versus unsigned arithmetic; actual compiled predicate | Equivalent for every 32-bit state after the signedness correction |
| CCollision::ClearInternalData | 0019e310 | Direct call to matrix initialization; vector and pointer fields | Equivalent for arbitrary mapped initial field bits |
| CCollision::Reset | 0019dba0 | Nested direct call followed by a tail transfer to matrix initialization | Equivalent with initial contact flags restricted to 0..255 |
| edF32Matrix4SetIdentityHard | 00267850 | Exact constant moves and PS2 VU lane rotation | Equivalent for arbitrary initial matrix bits |

Run `python tools/function_verify/compiled_extended.py --suite combat`, or use
the combined runner/watcher. Compiler-probed Windows offsets and audited PS2
offsets are in `combat_cases.py`. `compiled-combat/report.json` records domains
and assumptions. All non-private memory outside mapped fields must remain
unchanged, including the actor's collision pointer in the pointer-following case.

## Support added

`call_graph.py` instantiates a separate context for each approved direct call and
tail transfer. Real return addresses are checked before continuing at the caller.
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
records `out/verification/combat-blockers/report.json`. Audit exit 0 means the
audit ran, not that any blocked function passed equivalence.

| Candidate | Evidence | Blocking support |
| --- | --- | --- |
| CActorFighter::SetStandAnim (0031ba50) | Windows bytes lift successfully, but call expansion rejects indirect dispatch; PS2 `jalr t9` at 0031ba80 | A proved finite vtable target set and the selected SetState implementation(s) |
| CActor::SetState (00102a00) | Bundle preparation rejects the Windows LogCategories::Actor relocation; PS2 has five indirect calls, including 00102a54 and 00102b44 | Logging contract or code analysis, behavior/animation/audio callees and memory mappings, proved virtual targets, required floating-point semantics |
| CCollision::Reset with unrestricted 32-bit contact flags | A retained regression produces a mismatch with high flag bits set | Resolve the representation/invariant: PS2 masks/stores one byte, whereas Windows uint masking clears the upper 24 bits |

The 0..255 Reset restriction is a declared precondition, not a proof that every
caller maintains it. Its broader-domain mismatch is tested explicitly so the
restriction cannot quietly disappear from the reported proof scope. No logging,
virtual call, or animation/audio operation is silently skipped or stubbed as
successful. These are still blockers to claiming full combat state-transition
coverage.
