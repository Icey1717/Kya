# CTrackVector::Create

PS2 function at `001aee50` in `SLES_514.73` (base `00100000`). Source: `src/b-witch/Lipsync.h:58`. Full current packet source was found in the source file. Live decompilation/disassembly: `out/function-audit-orchestrated/evidence/befea83c17eebe4bb66b.json`.

Verdict: **real**.

| Exact signal | Verdict | Rationale | Supporting PS2 instruction addresses |
|---|---|---|---|
| `unfinished: Contains an implementation guard; inspect compiled behavior.` | real | Current source omits the PS2 operation represented by this signal. | 001aee74, 001aee90, 001aeec8, 001aef60, 001aef68 |
| `call_count: GetS32: port 0, PS2 2.` | real | PS2 performs GetS32 here; current source has no corresponding live operation. | 001aee74, 001aee80 |
| `call_count: operator_new_array: port 0, PS2 2.` | real | PS2 allocates both the scalar-count array and vector-data array; current body allocates neither. | 001aee90, 001aeea0 |
| `call_count: GetF32: port 0, PS2 5.` | real | PS2 performs GetF32 here; current source has no corresponding live operation. | 001aeec8, 001aeedc, 001aeef0, 001aef04, 001aef18 |
| `constant_count: Literal 12: port 0, PS2 2.` | not_real | 12 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aee7c |
| `constant_count: Literal 8: port 0, PS2 6.` | not_real | 8 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aee88 |
| `constant_count: Literal 2: port 0, PS2 1.` | not_real | 2 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aee94 |
| `constant_count: Literal 4: port 0, PS2 5.` | not_real | 4 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aef40 |
| `constant_count: Literal 32: port 0, PS2 5.` | not_real | 32 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aeea8 |
| `constant_count: Literal 16: port 0, PS2 2.` | not_real | 16 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aef28 |
| `constant_count: Literal 20: port 0, PS2 1.` | not_real | 20 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aef64 |
| `constant_count: Literal 24: port 0, PS2 1.` | not_real | 24 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aef68 |
| `constant_count: Literal 28: port 0, PS2 1.` | not_real | 28 encodes a field displacement, buffer index, or array stride in the PS2 layout; source expresses it through typed fields and is not a separate behavior value. | 001aef6c |

## Suggested fix and validation

Restore counts, arrays, five values per item and trailing vector; validate cursor/counts.

No code was changed in this audit.

