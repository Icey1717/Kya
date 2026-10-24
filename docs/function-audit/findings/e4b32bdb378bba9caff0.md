# CBehaviourCreditsDefault::Manage

- PS2 address: `00409830` (SLES_514.73, image base 00100000)
- Source: `src/b-witch/ActorCredits.cpp:463`
- Source matches current packet: **true** (full function text, normalized line endings)
- Overall verdict: **not_real**
- Live evidence: `out/function-audit-orchestrated/evidence/e4b32bdb378bba9caff0.json`

| Exact packet signal | Verdict | PS2 instruction evidence | Assessment |
|---|---|---|---|
| call_count: SwitchOn: port 0, PS2 1. | not_real | `00409830` | Source completionSwitch.Switch expands to target switching and camera SwitchOn. |
| constant_count: Literal 16: port 1, PS2 0. | not_real | `00409830` | 16 is the level id tested by the source `levelID != 0x10` branch. |
| constant_count: Literal 14: port 1, PS2 0. | not_real | `00409830` | 14 is the level id tested by the source `levelID == 0xe` branch. |
| constant_count: Literal 4: port 0, PS2 1. | not_real | `00409830` | The target switch record fields/stride are expressed through the switch helper in source. |
| constant_count: Literal 28: port 0, PS2 1. | not_real | `00409830` | 28 is the NTF target record stride in the inlined switch loop. |
| constant_count: Literal 2.242078e-44: port 0, PS2 1. | not_real | `00409830` | The scientific-notation value is the float reinterpretation of integer level id 16 or 14 (2.242078e-44=0x10, 1.961818e-44=0xe); source compares integer ids directly. |
| constant_count: Literal 1.961818e-44: port 0, PS2 1. | not_real | `00409830` | The scientific-notation value is the float reinterpretation of integer level id 16 or 14 (2.242078e-44=0x10, 1.961818e-44=0xe); source compares integer ids directly. |

## Suggested fix and validation

Fix: none indicated. Validation: exercise the corresponding state transitions and inspect identical save/credit output.

