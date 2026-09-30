# Mini-games organizer management fidelity pass

Compared against fresh Ghidra MCP decompilation of `SLES_514.73` (image base
`00100000`). Repository style reference: `CActorAcceleratos`. This pass covers
music, stand management, its menu paths, and the state callbacks and mesh setup
used by their state transitions. Drawing and actor creation/lifecycle code are
outside this pass. The pre-existing `ManageFade` edit is preserved.

## Function comparisons

| Function | PS2 entry | Result |
| --- | --- | --- |
| `ManageMusic` | `003b70b0` | Restored separate previous/current state branches, repeated virtual configuration calls, branch-local music lookup, locals, and handle assignments. The earlier version looked up music before any transition and cached it across both checks. Normal transitions are equivalent when lookups have no side effects. |
| `BehaviourMiniGamesOrganizerStand_Manage` | `003b0600` | Restored explicit state configuration lookup and expanded each sound lookup/play sequence. Preserved fade/music/disconnection ordering, independent confirm/cancel checks in state 8, and the final zone call. |
| `ManageMenuChoose` | `003b6bb0` | Restored negative nested input tests, separate button masks, null-aware repeated entry-count reads, and sound locals. Both consecutive `ComputeCurPlayMode` calls remain. Input priority is unchanged. |
| `ManageMenuMulti` | `003b6680` | Restored availability calculation before arrow management, repeated behaviour lookups, original candidate loops and bounds exits, and the shared positive-count/availability gate around navigation and confirm. Previously confirm could use stale positive availability when the bet count was nonpositive. Previously candidate loops had neither the original bounds exit nor return-to-start termination. |
| `ManageMenuTrain` | `003b6430` | Restored locals, separate input masks, sound sequences, and one solo-behaviour lookup per adjustment. The earlier version performed a second lookup for the assignment. Player-count limits and independent input checks are unchanged. |
| `ManageMenuResult` | `003b5fb0` | Restored nested final-action branches, three separate name copies, repeated mini-game reads after messages, explicit name-byte writes, and message locals. Previously mini-game lookup was cached across the callbacks. |
| `ManageMenuEnterName` | `003b5a60` | Restored fresh input reads after each sound callback, sound locals, signed modulo correction, explicit assignments, and the do/while row-count walk. Previously all input tests used one cached bitfield. Retained the port's cancel bounds guard. |
| `ManageZone` | `003b72f0` | Restored manager/hero capture before the loop, null-aware count lookup, separate loop counters, and message construction order. The earlier loop dereferenced a null stream even when it contained no entries. Retained initialization of the unused message word. |
| `BehaviourMiniGamesOrganizerStand_InitState` | `003b09b0` | Restored repeated configuration lookups, the original fade condition expression, separate state cases, message locals, and repeated width/height calculations and conversion branches. |
| `BehaviourMiniGamesOrganizerStand_TermState` | `003b0380` | Restored the original condition grouping, explicit configuration lookup, separate result/name cases, and ordered name-byte assignments. |
| `InitMenuMulti` | `003afe80` | Restored repeated behaviour lookups, availability calculation before selection assignment, and the original search from selection zero. For stable behaviour data the selected result matches the earlier implementation; callback ordering now follows the original. |
| `InitMenuMeshes` | `003b1070` | Restored the nested state branches and all repeated per-component validity checks and initialization calls. The original passes null mesh managers so each component resolves its own configured indices. The earlier cached lookup used the organizer's indices for every component. State 6 now assigns the arrow context even if the validity checks fail, as the original does. |
| `TermMenuMeshes` | `003b0220` | Restored eight separate virtual mesh checks and termination calls in order instead of the array loop. |
| `ComputeCurPlayMode` | `003b2110` | Retrieved and checked; existing nested behaviour checks and assignments already correspond. |
| `PrevPlayMode` / `NextPlayMode` | `003b2280` / `003b23b0` | Retrieved and checked; existing branches already correspond. |
| `CMenuWheel::Manage` | `002efc80` | Restored timer locals, the inline arrow-timer update, and the original boolean return. Existing callers discard that return. |
| `CMenuWheel::MoveWheel(bool)` | `002efda0` | Restored inline arrow assignments and the float local instead of the extracted arrow call. |
| `CMenuWheel::MoveWheel()` | `002efc70` | Retrieved and checked; both assignments already correspond. |
| Arrow movement (`MoveMenuArrow`) | `002ef920` | Restored the explicit direction branch and float local; this maps to an original standalone function, rather than an extracted sound/size helper. |
| `Fade` | `001a0180` | Restored the standalone original function locally: NTSC branches, unsigned conversion, repeated fade-color calls, and optional wait loop. Organizer calls still pass zero for waiting. |
| `CActorMiniGamesManager::IsBetAvailable` | `003adb20` | Restored the native manager call and original repeated episode lookup when the cap applies. Removed the organizer's extracted availability helper. |
| `CActorMiniGamesManager::PlaceBet` | `003adad0` | Restored the original native manager call, money comparison, conditional transfer, and return. |

## Type corrections and port adaptations

- Disassembly at `003b66ec` calls vtable offset `0xf8`; its result uses the
  multi-behaviour bet count at `+0xc` and bet array at `+0x10`. Ghidra's
  `ManageDyn` prototype and apparent float/register arguments are incorrect.
  Use `GetMultiBehaviour()` with native typed bets and element strides.
- Disassembly at `003b6518` calls offset `0x100`, then `003aa1a0` increments the
  result's integer at `+8`. This is solo player count, not a wind-state object.
  The increment/decrement helper bodies are represented directly with typed
  `nbPlayers` accesses.
- Ghidra's `StaticMeshComponent` layout identifies texture index at `+4` and
  mesh index at `+8`. Native member accesses retain that comparison order even
  though the port's C++ layout differs.
- Actor-stream reads retain typed `GetMiniGame()`/stream-entry `Get()` access;
  the existing stream reference implementation performs serialized pointer
  conversion. No raw PS2 integer-to-pointer arithmetic is introduced.
- Name-array and row-count accesses use their real members rather than Ghidra's
  indexing beyond `field_0x958` or its fabricated organizer-pointer walk.
  Restart positions use waypoint location/rotation members and native message
  pointers. Sound playback uses `CSoundInstance` members and native virtual
  `Play`; disassembly at `003b6500` supplies the handle output pointer that
  Ghidra omitted from one call.
- Data at `00448d2c`, `00448d30`, and `00448d38` is `2d 2d 2d 00`; score-name
  locals therefore use three explicit letter writes and a zero terminator.
  Disassembly at `003b04f0`/`003b0500` stores integer `-1` into `field_0x9b4`,
  correcting Ghidra's `-NAN` interpretation.
- Discarded return-register artifacts in void state/menu functions are not
  modeled as integer casts of native pointers. Floating constants retain `f`
  suffixes. PS2 division traps become debug assertions at valid-divisor checks;
  zero-count navigation remains an invalid configuration.

## Deliberate deviations

- Cancel in name entry retains `field_0x9b4 < 3`. The PS2 can write the next
  byte when the index is 3, overlapping `field_0x9b4`; reproducing that with a
  native C++ array would be undefined behavior.
- The zone message's middle word remains zero-initialized. Disassembly
  `003b73b4`–`003b73fc` initializes words at stack offsets `0x40` and `0x48`,
  leaving `0x44` uninitialized. The port retains deterministic message data.

## Validation

`cmake --build out/build/x64-debug`, `ctest --test-dir out/build/x64-debug
--output-on-failure`, and `git diff --check`. The headless smoke test does not
exercise every interactive mini-game menu or audio transition.
