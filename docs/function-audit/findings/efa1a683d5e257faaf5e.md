# CActor::InitDlistPatchable — `001046d0`

Source: `src/b-witch/Actor.cpp:1771-1786`; packet body is present in the current file (**matches**). Live Ghidra program `SLES_514.73`, base `00100000`; complete evidence: `out/function-audit-orchestrated/evidence/efa1a683d5e257faaf5e.json`.

| Signal | Verdict | PS2 evidence and comparison |
|---|---|---|
| `call_count` — `InitDlistPatchable: port 1, PS2 0.` | not_real | Source line 1773 calls `GetBehaviour(curBehaviourId)->InitDlistPatchable(param_2)`. PS2 obtains the behavior with `jal 00103f60` at `001046e0`, loads vtable slot `0x44` at `001046f0`, then dispatches via `jalr` at `001046f4`. Its return is tested at `001046fc` before the same display-list fallback. |
| `indirect_call_count` — `Unresolved indirect calls: port 0, PS2 1. Inspect targets before deciding equivalence.` | not_real | The PS2 indirect call at `001046f4` is precisely the virtual `CBehaviour::InitDlistPatchable` call at source line 1773; the `0x44` vtable load immediately before it identifies the dispatch slot. Source expresses the call symbolically, so it is not counted as unresolved there. |

Suggested fix: no source change indicated; the call and its return-dependent fallback correspond.

Validation: checked the full current function and live Ghidra decompilation/disassembly. No code changed; no runtime validation performed.
