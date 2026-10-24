# CActorBonus::CinematicMode_Manage

- PS2 entry: `0012ae60` (`SLES_514.73`, image base `00100000`)
- Current source: [ActorBonus.cpp](../../../src/b-witch/ActorBonus.cpp:247); complete function matches the packet.
- Live Ghidra body matches the packet. PS2 requests behavior ID 5 through `CActor::GetBehaviour` at `0012ae68`; it null-checks at `0012ae74` and calls the virtual Draw slot at `0012ae84`.

| Finding | Verdict | Evidence and rationale |
|---|---|---|
| `call_count` — `Draw: port 1, PS2 0.` | not_real | Source invokes `pBehaviourFlock->Draw()` at line 251. PS2 performs the same virtual Draw operation via `jalr` at `0012ae84`; the lexical direct call count omits virtual dispatch. |
| `indirect_call_count` — 0 vs 1 | not_real | The PS2 indirect call at `0012ae84` dispatches Draw on the behavior returned for ID 5; source obtains `BONUS_BEHAVIOUR_FLOCK` and invokes Draw under the same null check (lines 249-252). |

Suggested fix: no fix indicated. Validation: enter cinematic mode with the flock behavior present and confirm its Draw method is called once; confirm no call when absent.
