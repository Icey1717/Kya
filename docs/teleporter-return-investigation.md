# Nativ City return teleporter investigation

Investigation: 2026-10-23. Returning from level 0x0a to Nativ City leaves the teleporter door visually closed on PC. The user clarified that this was based on appearance, rather than observing `bOpen = 0`.

## Cause and correction

`CActorTeleporter::LevelHasTeleporters` read destination offsets 4 and 8 (`elevatorId`, cutscene ID) as the level and elevator. PS2 uses offsets 0 and 4 (`levelId`, `elevatorId`). Corrected these two field accesses, preserving the original control flow.

For NATIV's `TELEPORTER_L7` (actor index 1156, placement hash 0x262, hub elevator 7), destination `(10, 2, 123)` was incorrectly checked as level 2 / elevator 123. The elevator is out of range, so availability always returned false. The correct check is level 10 / elevator 2.

`CActorSwitch::LoadContext` calls `ManageNativElevators` (PS2 0x0016b370). This checks linked teleporter availability and replays its target notifications when a destination is open. In NATIV, `WOLFENSWITCH_L7` (index 1146) sends activation message 0x0f to `TELEPORTER_NC_DOOR_L7` (index 699). The incorrect availability lookup prevented the door-opening notifications on return, independently of the teleporter's own open flag.

PS2 decompilation and disassembly comparison: `LevelHasTeleporters` 0x002ee790. Instructions 0x002ee7ec and 0x002ee800 load the first and second destination words; the call to `Level_GetSubSectorInfo` is at 0x002ee80c. Array data begins four bytes after the destination count.

Runtime trace supplied by the user confirms `TELEPORTER_L7` has `open=1` both after Init and after LoadContext on entering level 0. `slot_2.dat` and `slot_2.dat.bak.1` also store its mode-3, 16-byte actor payload with `bOpen=1`. Level 10 has opened source teleporters. The save-overwrite hypothesis below was not supported. Temporary lifecycle logging was removed after collecting this evidence.

Validation: Windows debug build and three `TeleporterReturnTest` regression tests pass. Tests use the actual `(10, 1..3, 123)` destination pattern, verify an opened source elevator makes the hub check true, and verify unrelated levels and a closed hub actor do not unlock it. Visual confirmation with the corrected executable is pending.

## Confirmed control flow

- `CActorTeleporter::Init` initializes `bOpen` from `field_0x16c & 1`.
- `CActorTeleporter::LoadContext` restores `bOpen` from the first 32-bit word of its 16-byte save payload, only for save mode 3. This matches PS2 decompilation and disassembly at `0x002ee880`; `0x002ee8cc` writes the saved word to actor offset `0x2a0`.
- `CBehaviourTeleporterDefault::InterpretMessage` opens a closed teleporter for messages `0x0e` (toggle) and `0x0f` (activate). It sets state 6, writes `bOpen = 1`, updates target streams, and calls `Level_TeleporterChanged`. Compared against PS2 decompilation at `0x002ed440` and the message dispatch disassembly there. No opening-logic discrepancy identified.
- `CLevelScheduler::ManageLoadElevator` requires `bOpen != 0` before calling `UpdateCurTeleporterState`. PS2 decompilation at `0x002dc200` has the same requirement. Arrival itself does not open the teleporter; removing this check would diverge from PS2.
- `UpdateCurTeleporterState` spawns the hero and switches to state 7, but does not write `bOpen`. Compared against PS2 decompilation at `0x002ed7a0`.
- `Level_UpdateCurLiveLevelInfo` clears subsector flags, seeds bit 0 from `field_0x10 & 1`, and also sets bit 0 when the resolved teleporter actor has `bOpen != 0`. Distinguish the actor's `bOpen` from subsector availability flags and `disabledDestinationMask` during reproduction.
- `Level_FillRunInfo` preserves the originating level/material and destination elevator; its relevant flow matches PS2 decompilation at `0x002dba90`.

Save restoration runs in `CLevelScheduler::Level_PostInit` -> `SaveGame_LoadLevelState` -> `CScene::Level_LoadContext` -> `CActorManager::Level_LoadContext`, after all manager `Level_Init` calls in `CScene::Level_Init`. The initial hypothesis that this overwrote an activation with a closed save payload was ruled out for the supplied reproduction: the trace shows the hub actor remains open.

## Separate discrepancies noticed

These are not established causes of the closed flag and were left untouched:

- `UpdateCurTeleporterState` tests uninitialized `pfVar1` after resolving `pWayPoint`; PS2 resolves and tests the waypoint pointer. This affects companion spawn processing after the open gate has passed.
- Its destination-selection loop compared `pEntry->elevatorId` with `levelId` and `pEntry->field_0x8` with the source material argument. This was subsequently corrected in the [level/elevator field audit](level-elevator-field-audit.md), along with a related display-count check.

Existing working-tree modifications in ActorHero, DebugActorBehaviour, KyaMesh and KyaTexture were left untouched.
