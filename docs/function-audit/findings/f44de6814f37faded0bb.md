# undefined

- PS2 function: `003fca30` in `SLES_514.73` (base `00100000`).
- Current source: [src/b-witch/ActorBombLauncher.cpp](/G:/repos/Kya/src/b-witch/ActorBombLauncher.cpp:132); the full function matches the current packet source.
- Live Ghidra decompilation and disassembly: [evidence](</G:/repos/Kya/out/function-audit-orchestrated/evidence/f44de6814f37faded0bb.json>).

| Exact packet signal | Verdict | Rationale and PS2 instruction evidence |
|---|---|---|
| `call_count: ZEXT14: port 0, PS2 1.` | **not_real** | PS2 converts the boolean result of CFireShot::Project to integer 0/1 with `sltu v0,zero,v0` at 003fca6c. Source's `bVar1 != false` assigned to int has the same zero/one result; ZEXT14 is a decompiler integer-width annotation. PS2: `003fca60, 003fca6c, 003fca7c`. |

**Suggested fix:** No fix indicated. Validate Project returning both false and true.

**Validation:** No fix indicated. Validate Project returning both false and true.
