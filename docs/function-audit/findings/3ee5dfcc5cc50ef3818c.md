# CBehaviourMiniGamesOrganizerStand::Begin — `003b2020`

Source: `src/b-witch/ActorMiniGamesOrganizer.cpp:4273-4287`; packet body matches current source. Live Ghidra `SLES_514.73`, base `00100000`; evidence: `out/function-audit-orchestrated/evidence/3ee5dfcc5cc50ef3818c.json`.

| Signal | Verdict | Comparison |
|---|---|---|
| `call_count` — `Begin: port 1, PS2 0.` | not_real | Source line 4275 calls base `CBehaviourMiniGamesOrganizer::Begin`, whose full body at lines 4237-4242 only assigns `pOwner`. PS2 inlines this write at `003b2030`. It then makes the same branch calls: `SetState(5,-1)` at `003b2044` with state in delay slot `003b2048`, or forwarded `SetState(newState,newAnimationType)` at `003b2068-6c`. |

Suggested fix: none; the call delta is explained by the base field assignment being inlined.

Validation: compared both current method bodies with live Ghidra decompilation and the store/SetState instructions. No code changed; no runtime validation performed.
