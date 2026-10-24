# CActor::ReceiveEvent

- PS2 function: `00101cd0` in `SLES_514.73` (base `00100000`).
- Current source: [src/b-witch/Actor.cpp](/G:/repos/Kya/src/b-witch/Actor.cpp:4152); full function matches packet source.
- PS2 decompilation and disassembly: live Ghidra MCP; raw responses in [evidence](</G:/repos/Kya/out/function-audit-orchestrated/evidence/fc4cfabb460aa75b2b3f.json>).
- Per-signal verdicts below retain packet kind/detail exactly.

| Signal | Verdict | Evidence and rationale |
|---|---|---|
| `call_count: InterpretEvent: port 2, PS2 1.` | **not_real** | The source contains both `pCVar3->InterpretEvent(...)` and the actor fallback `this->InterpretEvent(...)`. PS2 disassembly likewise calls the behavior vtable at 00102180 and actor vtable at 001021b0; the candidate's named-call count is a virtual-dispatch naming difference. PS2: `00102168, 0010217c, 00102198, 001021ac`. |
| `indirect_call_count: Unresolved indirect calls: port 0, PS2 1. Inspect targets before deciding equivalence.` | **not_real** | The unresolved PS2 indirect call is the behavior's InterpretEvent slot at 00102180; the actor fallback is another virtual call at 001021b0. Both calls are explicitly present in source after GetBehaviour and zero-result check. PS2: `00102158, 00102168, 00102180, 00102190, 001021b0`. |
| `constant_count: Literal 4294967295: port 2, PS2 4.` | **not_real** | The sentinel is checked for waypoint selection and repeated defensively in the original decompilation. The source uses equivalent `-1`/`0xffffffff` tests before indexing `aWaypoints`; additional source literal counts do not change the event branches. PS2 case branch/table and waypoint path are at 00101e98-00101eb4, with projectile branches at 00101f28 and 00102008. PS2: `00101e98, 00101eb4, 00101f28, 00102008`. |

**Suggested fix:** No fix indicated. Validate behavior handling and actor fallback when the first returns zero.

**Validation:** behavior handling and actor fallback when the first returns zero.
