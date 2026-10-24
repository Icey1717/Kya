# undefined

- PS2 function: `0037ffc0` in `SLES_514.73` (base `00100000`).
- Current source: [src/b-witch/ActorAton.cpp](/G:/repos/Kya/src/b-witch/ActorAton.cpp:3066); the full function matches the current packet source.
- Live Ghidra decompilation and disassembly: [evidence](</G:/repos/Kya/out/function-audit-orchestrated/evidence/6b4edf40a507df1eed9b.json>).

| Exact packet signal | Verdict | Rationale and PS2 instruction evidence |
|---|---|---|
| `call_count: GetCurPathPlane: port 8, PS2 6.` | **not_real** | Source calls GetCurPathPlane repeatedly in member expressions; PS2 caches the current plane pointer across adjacent reads and only calls the accessor when it needs a new path-plane expression. The accessor is a pure `aPathPlanes + curIndex` calculation (PathFollow.cpp:1462), so repeated calls return the same pointer absent cursor mutation; mutations occur later at NextWayPoint. PS2: `0037ffdc, 0038003c, 00380058, 00380078, 00380158, 00380178, 003801b8, 003801c8`. |

**Suggested fix:** No fix indicated. Validate current-plane reads before and after waypoint changes.

**Validation:** current-plane reads before and after waypoint changes.
