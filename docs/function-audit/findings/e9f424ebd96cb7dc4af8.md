# CBehaviourAmortosCmdStand::Manage — `0035fb50`

Source: `src/b-witch/ActorAmortosCmd.cpp:225-235`; packet source is present in the current file (**matches**). Live Ghidra program: `SLES_514.73`, base `00100000`. Full decompilation and disassembly: `out/function-audit-orchestrated/evidence/e9f424ebd96cb7dc4af8.json`.

| Signal | Verdict | PS2 evidence and comparison |
|---|---|---|
| `call_count` — `StateAmortosCmdStand: port 1, PS2 0.` | not_real | At source line 231, `pOwner->StateAmortosCmdStand()` runs when actorState is 5. PS2 tests state 6 at `0035fb68`, state 5 at `0035fb70`, then calls `FUN_0035f890` at `0035fb80`. This is the same recovered operation under its source name. |
| `call_count` — `FUN_0035f890: port 0, PS2 1.` | not_real | The PS2 call target `0035f890` at `0035fb80` is the source method `CActorAmortosCmd::StateAmortosCmdStand()` at line 231. The predicate and receiver agree; only the function name was recovered. |

Suggested fix: no source change indicated; the signal is explained by name recovery.

Validation: reviewed the current function body, live Ghidra decompilation, and branch/call instructions. No code changed; no runtime validation performed.
