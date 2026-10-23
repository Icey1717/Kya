# Level and elevator field audit

2026-10-23. Compared against the open Ghidra PS2 program `SLES_514.73`.

The audit follows the destination-record layout through all its readers in `src/b-witch/ActorTeleporter.cpp`, the scheduler's level/elevator APIs, event forwarding, cinematic forwarding and map route checks. A repository search found no other `S_DESTINATION_ENTRY` or `pDestinationList` consumers in `src/`. Save-slot descriptions, localization records and debug labels also use names such as `levelId`, but are separate record types; this is not a full behavior audit of every function containing that spelling.

## Layout

Each `S_DESTINATION_ENTRY` is three 32-bit words, with a 12-byte stride:

| Offset | Current member | Meaning |
|---|---|---|
| 0 | `levelId` | Destination level; 0x10 is the no-destination sentinel |
| 4 | `elevatorId` | Destination elevator/area |
| 8 | `field_0x8` | Destination cutscene ID |

Several Ghidra expressions point four bytes before an entry, or at the final word of the previous entry. Their `+4`/`+8` accesses must be translated relative to that pointer, rather than treated as entry offsets 4/8. This explains the confirmed translation mistakes.

## Confirmed mismatches

| Function | PS2 entry / instruction evidence | Effect | Correction |
|---|---|---|---|
| `LevelHasTeleporters` | 0x002ee790; 0x002ee7ec, 0x002ee800 | Checks elevator/cutscene as level/elevator, preventing the hub door switch from replaying its open notification | Previously fixed to `levelId`, `elevatorId` |
| `UpdateCurTeleporterState` | 0x002ed7a0; 0x002ed9e0, 0x002ed9ec; selected index store at 0x002ed9f8 | Fails to select the matching return destination on arrival, or accidentally selects an unrelated entry | Compare `levelId` and `elevatorId` with the source level/material arguments |
| `Draw` | 0x002edda0; 0x002ee084 loads offset 4 from the entry in `s1` | Uses cutscene ID to decide whether the remaining-Wolfen count should be zero. An elevator with cutscene -1 can incorrectly display zero; an invalid elevator with a positive cutscene can incorrectly display a count | Test `elevatorId < 1` |

The two newly found mismatches were corrected. The PS2 decompilation and disassembly were checked before editing. Original structure layout, names, state transitions and memory model were preserved.

## Other reviewed paths

These comparisons found no additional level/elevator field or argument-order discrepancy. This does not establish full behavioral equivalence of each function.

| Function | PS2 address | Field/argument check |
|---|---|---|
| `StateTeleporterActive` | 0x002ed090 | Reads level, elevator and cutscene in order; maps elevator <1 to -1 before transition |
| `Draw` destination lookup | 0x002edda0 | Material and subsector queries already pass level/elevator correctly |
| `DetectDisabledDestinations` | 0x002edbd0 | Both initialization and live refresh use level/elevator correctly |
| `FUN_002edaa0` | 0x002edaa0 | First valid destination level comes from offset 0 |
| `FUN_002edb10` | 0x002edb10 | Counts enabled destinations using offset-0 level and the destination-index mask |
| `GetDestinationMaterial` | 0x002ee2d0 | Uses the level for linked-hub classification and elevator minus one for material selection |
| `GetMySubSectorMaterial` | 0x002ee3b0 | Reads destination level from offset 0 and forwards the requested area |
| `Level_Run` | 0x002db9d0 | Preserves level/elevator order when filling run info |
| `Level_FillRunInfo` | 0x002dba90 | Destination level indexes level info; elevator selects subsector/start sector; source level/material are retained |
| `Level_Teleport` | 0x002dc3a0 | Both inter-level and same-level paths preserve level/elevator/cutscene order |
| `Level_GetSubSectorInfo` | 0x002dbec0 | Indexes level first, then elevator; invalid elevator returns cleared info |
| `ManageLoadElevator` | 0x002dc200 | First argument selects the destination elevator; remaining arguments describe the source |
| `LevelsInfo_ReadTeleporters_V7_V9` | 0x002e1a90 | File offset 8 indexes the elevator slot in its containing level |
| `GetNbAreas` | 0x002d9c00 | Current-level hub classification versus requested-level area lookup is also present in PS2; left intact |
| `GetAreaMaterial` | 0x002d9890 | Correct level/area indexing and forwarding |
| `GetAreaWolfenCounts` | 0x002d9980 | Correct level/area indexing and linked-level accumulation |
| `RunSectorLoadingCinematic` | 0x001c48e0 | Stores elevator and source-material arguments in manager fields 0x20/0x24 |
| `CActorHeroPrivate::InterpretEvent` teleport branch | 0x00341c70 | Event words 1/2/3 forward level/elevator/cutscene |
| `CMapManager::HasRouteToLevel` | 0x003f6ae0 | Route comparisons use level IDs; the oddly named `markerType` parameter is also used as a level in PS2 |

`S_DESTINATION_LIST::Create` advances by the count times `sizeof(S_DESTINATION_ENTRY)`, with no field remapping. The debug teleporter table displays the three members in the same order.

## Validation and limits

The Windows debug build and the five `TeleporterReturnTest` checks pass. These cover hub door eligibility and destination-mask initialization/refresh using actual level-10 destination values `(10, 1..3, 123)`, including independence from the cutscene ID. The two new arrival/display corrections are validated against PS2 instructions; their complete runtime paths were not exercised by these unit tests.

An unrelated pre-existing defect in `UpdateCurTeleporterState` still tests the uninitialized `pfVar1` instead of the resolved waypoint pointer during companion spawning. It was already recorded in the return investigation and is outside this field audit; no change was made to it.

The earlier function-audit report classified `LevelHasTeleporters` as `parse_error`, so it did not identify that mismatch. This manual audit does not add reviewed exceptions to the heuristic audit ledger.
