# edProfileDel — `002811f0`

Source `src/b-witch/profile.cpp:45` matches the complete current packet. Live Ghidra program `SLES_514.73`, base `00100000`. Full decompilation/disassembly: `out/function-audit-orchestrated/evidence/fcdcffab0c1176a128a1.json`.

| Exact signal | Verdict | Evidence-based rationale |
|---|---|---|
| unfinished — Contains an implementation guard; inspect compiled behavior. | not_real | `IMPLEMENTATION_GUARD_PROFILE` expands to nothing. The port also stubs edProfileNew/Begin/End, so clearing the profile slot's in-use bit is disabled profiling bookkeeping, not gameplay. PS2 instructions: 002811f0, 002811f4, 0028120c |
| constant_count — Literal 4: port 0, PS2 1. | not_real | Literal 4 participates in original profile index calculation. The profile API is explicitly disabled in the port, so this bookkeeping has no active consumer. PS2 instructions: 002811f8, 00281204, 0028120c |

Suggested fix: No game behavior fix is indicated; keep the platform/profile adaptation.

Validation: read-only audit.
