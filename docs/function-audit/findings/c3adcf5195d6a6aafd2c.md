# CActor::PauseChange — `00100ca0`

Current source `src/b-witch/Actor.cpp:4933` matches packet. Live Ghidra `SLES_514.73`, base `00100000`; full capture: `out/function-audit-orchestrated/evidence/c3adcf5195d6a6aafd2c.json`.

| Exact signal | Verdict | Rationale and PS2 instructions |
|---|---|---|
| call_count — PauseChange: port 2, PS2 1. | not_real | Source invokes animation PauseChange and the current behavior's virtual PauseChange. PS2 calls animation PauseChange directly at 00100d34 and dispatches the behavior method indirectly at 00100d60. |
| indirect_call_count — Unresolved indirect calls: port 0, PS2 1. Inspect targets before deciding equivalence. | not_real | The indirect call at 00100d60 is the current behavior's PauseChange slot; source has that same virtual call at Actor.cpp:4954-4956. |

Suggested fix: None indicated; the direct and virtual calls map to the same two source operations.

Validation: source-to-packet and live Ghidra comparison; no code changes or build.
