# CActorCredits::Init

- PS2 address: `00409fe0` (SLES_514.73, image base 00100000)
- Source: `src/b-witch/ActorCredits.cpp:93`
- Source matches current packet: **true** (full function text, normalized line endings)
- Overall verdict: **not_real**
- Live evidence: `out/function-audit-orchestrated/evidence/daeedf94ef66ae96b137.json`

| Exact packet signal | Verdict | PS2 instruction evidence | Assessment |
|---|---|---|---|
| call_count: Init: port 2, PS2 3. | not_real | `00409fe0` | The two source calls are base actor Init and completionSwitch.Init; PS2 inlines switch target/camera Init and performs ClearLocalData fields directly. |
| call_count: ClearLocalData: port 1, PS2 0. | not_real | `00409fe0` | PS2 explicitly clears lineBuffer and initializes credit fields; source delegates that exact sequence to ClearLocalData(). |
| call_count: get_message: port 0, PS2 1. | not_real | `00409fe0` | Source ClearLocalData resolves the current message through get_message, matching the PS2 conditional lookup. |
| constant_count: Literal 28: port 0, PS2 1. | not_real | `00409fe0` | 28 is the target record stride in completion switch Init; source typed switch helper handles it. |
| constant_count: Literal 464: port 0, PS2 1. | not_real | `00409fe0` | 464 is a byte offset into the embedded line buffer while clearing it; source loops over the same buffer. |
| constant_count: Literal 465: port 0, PS2 1. | not_real | `00409fe0` | These are consecutive byte offsets in the lineBuffer clearing loop, represented by source ClearLocalData's loop. |
| constant_count: Literal 466: port 0, PS2 1. | not_real | `00409fe0` | These are consecutive byte offsets in the lineBuffer clearing loop, represented by source ClearLocalData's loop. |
| constant_count: Literal 467: port 0, PS2 1. | not_real | `00409fe0` | These are consecutive byte offsets in the lineBuffer clearing loop, represented by source ClearLocalData's loop. |
| constant_count: Literal 468: port 0, PS2 1. | not_real | `00409fe0` | These are consecutive byte offsets in the lineBuffer clearing loop, represented by source ClearLocalData's loop. |
| constant_count: Literal 469: port 0, PS2 1. | not_real | `00409fe0` | These are consecutive byte offsets in the lineBuffer clearing loop, represented by source ClearLocalData's loop. |
| constant_count: Literal 470: port 0, PS2 1. | not_real | `00409fe0` | These are consecutive byte offsets in the lineBuffer clearing loop, represented by source ClearLocalData's loop. |
| constant_count: Literal 471: port 0, PS2 1. | not_real | `00409fe0` | These are consecutive byte offsets in the lineBuffer clearing loop, represented by source ClearLocalData's loop. |
| constant_count: Literal 8: port 0, PS2 3. | not_real | `00409fe0` | The 8-byte loop step clears each word in the line buffer; source byte loop clears the same 64 bytes. |
| constant_count: Literal 55: port 0, PS2 1. | not_real | `00409fe0` | 55 (0x37) is the line-buffer loop boundary; source loops over all 64 bytes. |
| constant_count: Literal 9: port 0, PS2 1. | not_real | `00409fe0` | These are final line-buffer byte offsets after the 8-byte clearing loop; the source helper clears the same bytes. |
| constant_count: Literal 10: port 0, PS2 1. | not_real | `00409fe0` | These are final line-buffer byte offsets after the 8-byte clearing loop; the source helper clears the same bytes. |
| constant_count: Literal 11: port 0, PS2 1. | not_real | `00409fe0` | These are final line-buffer byte offsets after the 8-byte clearing loop; the source helper clears the same bytes. |
| constant_count: Literal 12: port 0, PS2 1. | not_real | `00409fe0` | These are final line-buffer byte offsets after the 8-byte clearing loop; the source helper clears the same bytes. |
| constant_count: Literal 13: port 0, PS2 1. | not_real | `00409fe0` | These are final line-buffer byte offsets after the 8-byte clearing loop; the source helper clears the same bytes. |
| constant_count: Literal 14: port 0, PS2 1. | not_real | `00409fe0` | These are final line-buffer byte offsets after the 8-byte clearing loop; the source helper clears the same bytes. |

## Suggested fix and validation

Fix: none indicated. Validation: exercise the corresponding state transitions and inspect identical save/credit output.

