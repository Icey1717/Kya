# DoMessage parameter audit — 23 October 2026

The audit found one active incomplete native conversation payload, three dormant
instances of split message storage, and two analog-message storage discrepancies
whose extra word is not read by the currently implemented receiver. The active
incomplete payload also exists in the original PS2 binary; it is not a newly
identified port regression. The previously repaired commander attack payload is
the confirmed active port regression used as the reference case.

The findings below describe the source before the fixes. All six findings have
now been addressed; the three implementation guards remain in place.

## Fixes and verification

- Native commander replacement now sends `behaviourSpeak.field_0x24` as the
  conversation index. This is the index originally supplied by the commander in
  `field_0x8` and retained by speak behaviour. The relationship is visible in
  commander `FUN_00393620` (`0x00393620`) and the speak message handler. Supplying
  it on cancellation deliberately repairs the original PS2 omission at
  `0x00163434–0x00163450`.
- Commander cancellation reads only the index it needs. It rejects negative and
  out-of-range indices and a null conversation table, instead of copying unused,
  potentially uninitialized message members or dereferencing an invalid entry.
- The fighter's guarded hit message now uses `_msg_hit_param`, including its
  direction, force, damage, flags, and both halfwords at offset `0x50`.
- Both guarded native interaction requests now use
  `CActorNativMsgParam_0xe`, with the cinematic ID assigned to `field_0x10`.
- Both analog senders now use the eight-byte `_msg_analog_params` structure.
  The value and trailing word are contiguous; a size assertion protects this
  layout. The trailing word's meaning remains unrecovered (`field_0x4`).

Validation: `cmake --build out/build/x64-debug` succeeded and rebuilt
`bin/WIN/Kya_Debug.exe` and `KyaPortTest.exe`. All 14 tests selected by
`MessageParameters.*:NativAkasaTest.*:TimedSwitchTest.*:WolfenFight.*` passed.
The three new regression tests exercise actual commander replacement with
conversation index one, cancellation index validation, and analog delivery
including sender fallback and the existing weighing slave handler.

The three guarded sends were compared to PS2 stores but cannot be exercised
through their guarded runtime paths. No gameplay session was run for these fixes.

## Scope and method

- Inventoried **419 textual call sites in 259 functions across 67 `.cpp` files**
  under `src/b-witch`. These include guarded and commented decompilation. The
  count includes a multiline call in `Actor_Cinematic.cpp`; a single-line search
  finds only 418. The declaration and definition of `DoMessage` are excluded.
- Examined payload declarations, pointer aliases, assignments, and receiver
  dispatch for suspicious raw arrays/scalars and split PS2 stack locals.
- Used `out/function-audit/ps2.jsonl` to screen corresponding original functions,
  then checked the findings below against live Ghidra decompilation. Disassembly
  confirms the important payload stores and offsets.
- Local generated inventory: `out/domessage-audit/inventory.csv`, with a JSON
  companion containing extracted function bodies. Four free-function callbacks
  were added manually after reconciling the extraction with the text search.

This is a source audit and targeted PS2 comparison, not a proof of definite
assignment on every control-flow path. Dynamic notification IDs, opaque
`void*` forwarding, and unimplemented receivers limit what can be established
statically. The original audit was read-only; follow-up validation is recorded
above.

## Active finding: native conversation cancellation omits its index

**Priority: high; fixed, inherited from PS2.**

Sender: `CActorNativ::InterpretMessage`, `ActorNativ.cpp:648–649`.
Receiver: `CActorNativCmd::InterpretMessage`, `ActorNativCmd.cpp:229–242`.
Message: `MESSAGE_NATIV_CMD` (`0x4e`), subtype `7`.

The sender declares `CActorNativMsgParam_0xe local_20` and assigns only
`local_20.type = 7`. The receiver copies the payload and reads `field_0x8` as a
conversation index. It then dereferences the selected `CTalkParam`. The sender
has not assigned that index. Depending on the uninitialized value, the receiver
can select an unrelated conversation or dereference an invalid/null pointer.

This send occurs while replacing the native's commander, when the native is in
speak behaviour and `behaviourSpeak.field_0x24 != -1`. That field is a plausible
source for the missing index, but choosing it would be a recovered-intent change,
not a literal restoration of an original store. Zero-initializing the message
would silently select conversation zero and does not establish correctness.

PS2 evidence:

- Sender function `0x00163320`; instructions `0x00163434–0x00163450` pass the
  payload at `sp+0x60` and store only subtype `7` there before the send. The
  missing index would be at `sp+0x68`.
- Receiver function `0x00395200`; Ghidra confirms the copy and use of
  `field_0x8` in subtype `7`.

Resolution: send the conversation identity already retained by speak behaviour
and validate its range in the receiver. This intentionally fixes the original's
missing store rather than treating it as evidence that the field is unused.

## Dormant split-storage findings

These send sites are inside `IMPLEMENTATION_GUARD(...)`. That macro discards its
argument and asserts/logs instead of executing it. The defects below therefore
do not currently send malformed messages, but must be repaired when implementing
the guarded paths. C++ does not guarantee adjacency between independent locals.

| Sender | Message | Missing reconstructed storage | PS2 evidence |
|---|---|---|---|
| `CActorFighter::_Hold_GetPossibleExit`, `ActorFighter.cpp:1816–1827` | `MESSAGE_KICKED` (`2`) | `local_a0[2]` is only 8 bytes. Flags (`local_98`), damage (`local_94`), the `field_0x10` word (`local_90`), direction (`eStack128`), force (`local_70`), and the `field_0x50` halves (`local_50`/`local_4e`) are separate locals instead of members of the hit payload. | Function `0x00310bf0`; `0x00310d14–0x00310d78` writes into one payload beginning at `sp+0x30`, then passes that address. |
| `CBehaviourNativTakeAndPut::InterpretMessage`, `ActorNativ.cpp:3003–3007` | `0x4e`, subtype `9` | `local_20[4]` ends before offset `0x10`. `local_10 = FUN_00162a70(...)` must occupy that offset in the message. The command receiver reads it as the cinematic ID. | Function `0x003f38c0`; `0x003f3998–0x003f39c4` stores subtype at `sp+0x30` and cinematic ID at `sp+0x40`, then passes `sp+0x30`. |
| `CBehaviourNativLive::InterpretMessage`, `ActorNativ.cpp:3853–3857` | `0x4e`, subtype `9` | Same split between `local_20[4]` and `local_10`. | Function `0x003f40f0`; `0x003f41c8–0x003f41f4` confirms the same contiguous layout. |

The native receiver is `CActorNativCmd::InterpretMessage` subtype `9`, which
passes `pParams->field_0x10` to `addOnA`/`addOnB.Func_0x20`. The already implemented
`CBehaviourNativSpeak::InterpretMessage` provides a correct local example: it
assigns both `local_20.type = 9` and `local_20.field_0x10` before sending.

Resolution: `_msg_hit_param` for the fighter message and
`CActorNativMsgParam_0xe` for the two native messages. The existing guards remain
until the surrounding paths are implemented and verified.

## Analog payload discrepancies with no demonstrated current effect

| Sender | Observation | Current receiver |
|---|---|---|
| `S_STREAM_NTF_TARGET_ANALOG::NotifyAnalog`, `CinematicManager.cpp:3571–3579` | The original places `param_2` and `field_0x10` in adjacent words. The port assigns separate `float local_18` and `uint local_14` and sends only `&local_18`. Original function: `0x00119050`. | `CBehaviourWeighingMachineSlave::InterpretMessage` (`ActorMovingPlatform.cpp:3621`) reads only the first float for message `0x11`. |
| `CActorMovingPlatform::StateWeighingMaster`, `ActorMovingPlatform.cpp:2462–2465` | The original sends a float followed by a zero word. The port uses separate `local_10` and `local_c`. Original function: `0x00152530`. | Same receiver, which ignores the trailing word. |

Both storage representations are now corrected. The extra word is not a
demonstrated required input of the current receiver, so these changes should
not be presented as fixes for confirmed gameplay bugs.

## Reference case and false positives checked

- `CActorCommander::UpdateAttack` (`0x00170720`) previously sent an integer array
  while assigning the hero pointer to a separate local. The existing repair now
  sends `InitPathfindingClientMsgParams` and assigns `msgId` and `pActor` within
  it. Its configured-target and hero-fallback regression test remains present.
- Commander subtype `0xf` in `CBehaviourCommander::UpdateTeamAnim` needs only
  `msgId`: the Wolfen receiver does not read `pActor` for that subtype. The raw
  array is not evidence of another lost target pointer.
- Native shop message `0x60` subtypes `0`, `1`, `2`, and `4` use their command
  word. Subtype `3`, which does read `purchaseId`, already uses
  `_msg_params_0x60` and assigns that field in `TermState`.
- Shocker command `0x69` subtypes `0` and `1` use only the first command word.
- Native hold/exorcism notifications `0x1b` use the subtype and the sender
  identity; the command-only arrays do not omit a required actor pointer.
- `MESSAGE_GET_ACTION` and inventory state queries include output fields. A
  sender need not preassign fields that a successful receiver fills before the
  sender reads them.
- Native organizer `ManageZone` uses a three-word array containing both the
  command and its in/out index at word two. That index is part of the array,
  unlike the dormant native subtype `9` cases above.
- Numeric `MSG_PARAM` casts such as bone IDs, boomy levels, enable flags, and
  inventory actions are immediate values where the receiver expects immediate
  values; they must not be replaced indiscriminately with addresses.
- Several hit messages initialize only fields used by their projectile subtype.
  Partial initialization alone was not classified as a port regression where
  the PS2 original makes the same assignments. Broader undefined-value cleanup
  requires a separate receiver/path analysis.

## Remaining verification

Gameplay verification of native commander replacement remains useful. The three
dormant paths need runtime coverage when their surrounding implementation guards
are removed. The inventory remains a review queue for deeper branch-by-branch
payload analysis; absence of a finding here does not certify every call site.

## Bell follow-up: reinforcement sequence was never initialized

The original commander message repair was necessary but did not repair the
earlier loading stage. `CActorCommander::Create` consumed each wave's two stream
values without saving them. Its `ACommanderComponent_10` also lacked the raw
fields at offsets `0x0` and `0x8`, so the original 16-byte record was represented
as only eight bytes. Both cumulative fields used by `UpdateKilled` and
`UpdateAttack` were uninitialized.

The loading stage now restores all four fields and accumulates the death
threshold and number of released fighters, matching Ghidra function
`0x00172910` and stores at `0x00172a70–0x00172ad0`. The restored loader is extracted
as `CreateSequence` so the serialized sequence can be tested without rendering
and model setup. A size assertion protects the original 16-byte layout.
`UpdateKilled` checks the sequence bound before reading the next entry; the
original check at `0x001709b0–0x001709e0` read past the array after the final
death threshold.

The level archive confirms `COMMANDER_01_L1` (actor `0x18`) has eight team
members, including `GRUNT_01_BASIC_02` (`0x140`), and sequence words
`{ 2, 0, 5, 5, 0 }` at archive offsets `0x00a6e95c–0x00a6e96c`:
release five fighters immediately, then finish after five deaths.

The new `BellSequenceLoadsTheLevelWaveAndAlertsFiveWolfen` regression parses
these words, latches the bell/detection flags, and runs the commander's sequence
through the real Wolfen combat-message handler. Exactly the first five of eight
Wolfen enter combat against the hero. Repeated updates do not release duplicates,
and all five deaths advance safely to the final sequence entry. Another test
checks cumulative values across multiple waves and the empty-sequence case.

The Windows build and all seven tests in `WolfenFight.*:MessageParameters.*`
pass. `Kya_Debug.exe` is rebuilt. Live bell verification remains outstanding:
the debug socket at `127.0.0.1:18438` refused the connection during this session.
