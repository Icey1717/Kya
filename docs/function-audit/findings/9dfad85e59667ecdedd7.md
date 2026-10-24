# CActorCommander::BeginFightIntruder — `00171560`

Source: `src/b-witch/ActorCommander.cpp:469-482`; packet body matches current source. Live program `SLES_514.73`, base `00100000`; full PS2 comparison evidence: `out/function-audit-orchestrated/evidence/9dfad85e59667ecdedd7.json`.

| Signal | Verdict | Comparison |
|---|---|---|
| `call_count` — `GetTeamElt: port 1, PS2 0.` | not_real | After the fighter type check at `00171594-9c`, PS2 inlines `GetTeamElt`. It reads `nbTeams` at `001715a4`, array base at `001715b8`, compares each entry's `pEnemyActor` at `001715c0-c4`, increments the entry pointer by 0x18 at `001715e4`, and loops at `001715e0`. Source's helper at `ActorCommander.cpp:824-840` performs the same pointer walk and equality test. The selected entry reaches `AddFighter` at `001715ec`, then `SetStandAnim(0xe2)` at `001715fc`, matching source lines 477-478. |

Suggested fix: none; this is inlined helper logic.

Validation: read the full current function and helper and compared the live decompilation and loop/call instructions. No code changed; no runtime validation performed.
