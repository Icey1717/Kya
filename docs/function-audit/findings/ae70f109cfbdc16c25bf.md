# CActorAutonomous::LifeIncrease — `00120cb0`

Current source `src/b-witch/ActorAutonomous.cpp:2169`; packet source matches complete current file: **true**. Live Ghidra: SLES_514.73 at `00100000`. Full decompilation and disassembly: `out/function-audit-orchestrated/evidence/ae70f109cfbdc16c25bf.json`.

| Exact packet signal | Verdict | Assessment |
|---|---|---|
| call_count — SetValue: port 2, PS2 0. | not_real | The source contains the flagged named operation; the lexical delta comes from altered spelling/expansion and is not by itself behavioral. PS2 addresses: 00120cd4, 00120ce4, 00120cf8, 00120d00, 00120d0c, 00120d1c |
| indirect_call_count — Unresolved indirect calls: port 0, PS2 2. Inspect targets before deciding equivalence. | unresolved | The original function contains Unresolved indirect calls: port 0, PS2 2. Inspect targets before deciding equivalence. but no direct target is identified; resolve the vtable slot and compare it with current source dispatch. PS2 addresses: 00120cd4, 00120ce4, 00120cf8, 00120d00, 00120d0c, 00120d1c |

Suggested fix: no change is supported yet; resolve the stated operand or target question.

Validation: audit only.
