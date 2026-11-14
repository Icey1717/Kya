# Actor and behaviour coverage audit

This is the project-wide checklist for every source type deriving directly or
indirectly from `CActor` or `CBehaviour`. It includes roots, shared/abstract bases,
nested classes, classes absent from LEVEL_1, and behaviours with no serialized
payload. [COVERAGE.md](COVERAGE.md) records the previous LEVEL_1 milestone; its
completion statements do not establish coverage of this inventory.

## Checklist status legend

- `[ ]` **Incomplete**: missing implementation, partial coverage, or an inherited
  contract/applicability map that still needs reconciliation.
- `[~]` **Awaiting verification**: an implementation or empty-payload disposition
  is recorded in the findings, but validation or audit requirements remain open.
  This includes fixture/round-trip execution, UI inspection, PS2 comparison and
  documented inspector/reference limitations. It is not a completion claim.
- `[x]` **Verified**: every completion criterion below is satisfied with recorded
  evidence, or an explicit not-applicable disposition covers it.

The inventory status describes the row's own serialized contract, not completion
of every descendant or behaviour in its family. Shared bases still need an
explicit inherited contract. Runtime AI/animation completeness is separate from
bank import, editing, export, and inspector coverage.

During reconciliation, existing checked rows with outstanding verification or
documented exclusions were changed to `[~]`; newly implemented rows were also
marked `[~]`. Missing/partial implementations remain `[ ]`. No builds, tests or UI
validation were performed by this reconciliation. Read each family's findings
for its evidence and remaining limitations; earlier fixture names describe
available coverage and do not establish that the current worktree was tested.

## Completion criteria for each row

- [ ] Identify the full inheritance chain, actor factory class ID, and every
  applicable `BuildBehaviour` ID, including inherited/default/fallback cases.
  Behaviour IDs are actor-specific; names alone do not establish the mapping.
- [ ] Trace `Create` and every base/helper reader in byte order. Record version
  gates, optional blocks, counts, references, and field units. Distinguish runtime
  state from serialized data. Explicitly record genuinely empty payloads.
- [ ] Locate the existing editor reader/model or mark it missing. Preserve raw
  unsupported payloads; do not equate zero raw bytes with a complete inspector.
- [ ] Check export/writer and capability metadata for every serialized field,
  including inherited fields. Record fixed-size/variable-size editing limits and
  explicit reasons for read-only or unsupported data.
- [ ] Check inspector grouping, field applicability, empty-group hiding, names,
  reference pickers, and refresh/undo behaviour. Check individual behaviour IDs
  within a family, not just its broad `bHasTyped...` flag.
- [ ] Validate source-derived fixtures for each distinct layout and round trips,
  plus real-bank instances where available. Inspect relevant actor/behaviour UI.
  Empty behaviours still require ID/catalog coverage and an accurate empty view.
- [ ] Record implementation file paths, evidence/test names, remaining exclusions,
  and original PS2 function addresses when verified. Source stubs/guards or uncertain
  readers require Ghidra verification; do not infer an empty payload from a stub.

Use `[~]` when implementation is recorded but any criterion remains outstanding.
Use `[x]` only when all criteria are satisfied, or a documented not-applicable
disposition covers them. For shared bases, record the inherited
contract and reference it from descendants. Use the finding template below;
add further per-family findings as the audit proceeds.

## Implementation progress: Hunter

Class 0x48 now has a typed 40-byte reader/writer in `Actors/KyaHunter.h/.cpp`,
placement properties in `KyaLevelBank.h`, factory/export dispatch, export capability
metadata and a registered family with an actor-reference picker. Source reads are
`CActorHunter::Create`, `CActorAutonomous::Create` and `CActorMovable::Create`.
Offsets: weight f32 0, life maximum u32 4, gravity f32 8, dynamic fields f32 12/16,
bone u32 20, field 354 u32 24, actor s32 28, fields 3A4/350 f32 32/36.
Unsigned fields use int64 properties and writer range validation. Trailing bytes
remain raw, and export accounts for the decoded prefix. Watch id 3 has an empty
Create; ids 0/1/2 inherit Stand/Cinematic/CatchByTrap. No source files were changed.
CatchByTrap id 2 now uses the shared conditioned-operation codec for Hunter and
Projectile as well as Micken/Nativ. Watch id 3 is explicitly recognized as empty
and its inspector explains that no serialized payload exists. Rows are
marked `[~]`: binary addresses, fixtures and UI inspection still require
reconciliation. No builds or tests run.

## Implementation progress: Helper Sign, Wood Monster, Shocker Command, Acceleratos

These implementations are source-derived and unbuilt/untested, per the current
instruction. Their inventory rows are marked `[~]` pending the validation gates.
Each has a dedicated `Public/Actors/Kya<Name>.h`, `Private/Actors/Kya<Name>.cpp`
and `Private/Actors/Kya<Name>ActorFamily.cpp`, with placement models in
`KyaLevelBank.h`, factory and exporter dispatch, and capability allowlist entries
in `KyaExportedActorFields.h`. Decoded class prefixes are removed from `ClassData`;
both export passes retain the remaining raw tail. Shared placement transactions
handle refresh and undo. No decompiled source was modified for these families.

| Family | Factory / inheritance | Class bytes in source order | Owned behaviour IDs |
| --- | --- | --- | --- |
| Helper Sign | 0x2b, CActor -> CObject | s32 field 168, u64 field 160 (12 bytes, unaligned hash) | 2 Stand empty; 3 HighScore s32 organizer actor reference |
| Wood Monster | 0x27, CActor -> CObject | four f32 components of field 160 (16 bytes) | 2 Get: f32 field 8, s32 waypoint ref, s32 field 10 |
| Shocker Command | 0x50, CActor -> CObject | u32 field 168, s32 sequence count, each s32 actor ref + s32 float count + floats | 2 Stand empty |
| Acceleratos | 0x3a, CActor -> CObject | packed S_ACCELERATOS_CONFIG: three f32 fields, two s32 sound refs (20 bytes) | 2 Stand empty |

All four inherit generic 0 Stand and 1 Cinematic; other IDs fall back to CActor.
Factory mappings were reconciled with `Types.h` and `ActorFactory.cpp`. Reader
evidence is each family's `Create` and `BuildBehaviour`, plus
`CBehaviourHelperSignHighScore::Create`, `CBehaviourWoodMonsterGet::Create`,
`CBehaviourShockerCmdStand::Create` and `CBehaviourAcceleratosStand::Create`.
The Acceleratos packed config has a source size assertion of 0x14.

Helper Sign's hash uses a signed int64 container retaining all 64 bits. Shocker
Command's unsigned field uses an int64 property with writer range checks; signed
negative counts and truncated nested lists are rejected. Both sequence count and
each sequence's float count are fixed on export; the inspector uses EditFixedSize.
Other fields have fixed-size writers. Empty behaviour inspectors hide raw and
irrelevant groups and show an explicit no-payload message. Unsupported/malformed
behaviour layouts retain the raw inspector. Helper Sign HighScore and Wood Monster
Get have typed fields gated by their decoder flags and exact behaviour IDs.

Remaining: source-derived fixtures and real-bank/UI validation have not been run;
PS2 addresses are unverified. Nested organizer/waypoint/sequence references remain
numeric fields without pickers. Acceleratos sound references are numeric because
the shared reference catalog does not yet provide a sound namespace. No version
gates occur in these class or owned-behaviour readers.

## Implementation progress: Add-On Purchase

Class 0x49 (`Types.h`, `ActorFactory.cpp`), CActorAddOnPurchase -> CActor ->
CObject. `ActorAddOnPurchase.cpp::Create` calls `CInventoryInfo::Create`
(`InventoryInfo.cpp:123`), then runtime-only Init. BuildBehaviour maps 2 to
CBhvAddOnPurchase (empty Create), otherwise delegates to CActor (0 Stand,
1 Cinematic). No trailing initial-behaviour word belongs to this class payload.

`Actors/KyaAddOnPurchase.h/.cpp` handles exact 4/36/40/52-byte records: signed
presence, if nonzero two u64 fields and four s32 fields; from scene version 2.27,
an object-present s32 and, when nonzero, s32 object ID plus u64 field 18. Exact
sizes disambiguate the version because scene version is not available to this
reader. Unknown sizes remain entirely raw. The signed int64 hash containers
preserve all 64 bits. Original presence/object flag words are retained and
validated before writing; field visibility follows both discriminators.

Model: FKyaPurchaseInventory and AddOnPurchaseInventory in KyaLevelBank.h.
Family: KyaAddOnPurchaseActorFamily.cpp. Factory, both exporter raw-tail paths,
typed writer dispatch, and KyaExportedActorFields.h are integrated. Behaviour 2
uses the explicit empty inspector; edits use the shared transaction path.
No arrays or editable layout switches occur. Rows are marked `[~]` pending
fixtures, UI inspection and PS2 address verification. No builds or tests run.

## Implementation progress: Amortos Command and legacy inventory

Amortos Command class 0x31 (`Types.h`, `ActorFactory.cpp`) derives from CActor ->
CObject. `CActorAmortosCmd::Create` reads only an S_ACTOR_STREAM_REF: count u32,
then count x s32 actor indices (`Actor.cpp::S_ACTOR_STREAM_REF::Create`). Runtime
Init/Reset add no bytes. BuildBehaviour maps 2 to CBehaviourAmortosCmdStand, whose
Create is empty; 0/1 inherit Stand/Cinematic and other IDs delegate to CActor.
`Actors/KyaAmortosCmd.h/.cpp` preserves trailing bytes, rejects out-of-range or
truncated counts, and requires the original list length on export. The
AmortosCmdActorRefs placement array is EditFixedSize. Dedicated family metadata,
factory, both exporter tail paths, writer and capability allowlist are wired.
Nested actor refs remain numeric. The empty inspector covers id 2. No version
gates occur in this class reader. Fixtures/UI/PS2 addresses remain unverified.

The shared CBehaviourInventory codec in KyaProjectile.cpp now also handles the
exact 40-byte pre-2.27 record (36 bytes CInventoryInfo + initial behaviour).
Absent records remain 8 bytes, modern records remain 44/56. This follows
InventoryInfo.cpp::Create and BehaviourInventory.cpp::Create. FKyaInventoryInfo
now hides purchase fields when Field0 is zero and object fields when their block
is absent. No builds or tests were run for these changes.

## Implementation progress: Pattern Command

Class 0x40 (`Types.h`, `ActorFactory.cpp`), CActorPatternCmd -> CActor -> CObject.
Source: ActorPatternCmd.cpp::Create/BuildBehaviour and
ActorPatternService.cpp::CPatternPart::Create. Class reads two u32 fields (164,
160), actor stream (u32 count + s32 references), two f32 fields (1F0/1F4), then
CPatternPart. The pattern reads six f32 values (4/0/C/8/10/14), s32 material,
f32 point count, f32 field 20, optional f32 field 24 from version 2.21, and u32
field 28. Runtime clamping/defaults and point-pattern allocations are not bytes.
Total size is 60 + count*4 before 2.21, or 64 + count*4 from 2.21. Exact remaining
size selects the version; other shapes remain wholly raw. BuildBehaviour maps
2 to the empty CBehaviourPatternCmdStand, delegates other IDs to CActor (0/1).

Actors/KyaPatternCmd.h/.cpp owns offsets/read/write; FKyaActorPlacement stores
typed fields and the existing FKyaPatternPart model. The actor-reference list
is fixed-size, both unsigned values are int64 with explicit writer bounds, and
changing pattern-field presence is rejected. The registry, factory, both export
tail paths, writer dispatch, export capabilities and empty inspector are wired.
Numeric actor references have no nested picker. Source-derived fixtures, UI
inspection and PS2 addresses remain unverified; rows are marked `[~]`. No builds
or tests were run.

## Implementation progress: Egg and Care Boy

Egg: class 0x2f (`Types.h`, `ActorFactory.cpp`), CActorEgg -> CActorMovable ->
CActor -> CObject. Source readers: ActorEgg.cpp::Create,
ActorMovable.cpp::Create, ActorAton.cpp::CPathPlaneArray::Create. Payload is
weightB f32, field 1D8 f32, u32 path count, count x s32 path index, u32 field 1DC,
u32 field 1E0 (20 + count*4 bytes). Path-plane runtime objects/defaults are not
serialized. BuildBehaviour maps 2 to CBehaviourEggMove, whose Create is empty;
other IDs delegate directly to CActor (0 Stand, 1 Cinematic).

Care Boy: class 0x37 (`Types.h`, `ActorFactory.cpp`), CActorCareBoy ->
CActorMovable -> CActor -> CObject. ActorCareBoy.cpp::Create reads weightB f32
through CActorMovable, then signed fields 1DC/1E0 (12 bytes). BuildBehaviour maps
2 to CBehaviourCareBoySteady, whose Create is empty, otherwise CActor fallback.
Neither family has a version gate in its readers.

Dedicated Actors/KyaEgg.h/.cpp and Actors/KyaCareBoy.h/.cpp implement codecs;
corresponding ActorFamily modules own catalog entries. Placement models,
factory dispatch, both export raw-tail paths, writer dispatch and export
capabilities are integrated. Unknown trailing bytes are retained. Egg's list
uses EditFixedSize and export validates the original count; u32 fields use int64
properties and writer bounds. Care Boy's signed fields preserve all bits.
Egg path refs remain numeric with no per-element picker. Shared transactions
provide edit/undo refresh. KyaActorFamilies::HasEmptyPayload centralizes the
explicitly source-reconciled empty ID map for import and export; empty Move and
Steady payloads show the no-payload inspector. Unknown empty data is not promoted
to known emptiness. Rows are marked `[~]` pending fixtures, UI and PS2 address
verification. No builds or tests were run.

## Implementation progress: Electrolla

Class 0x0b (`Types.h`, `ActorFactory.cpp`), CActorElectrolla -> CActor -> CObject.
ActorElectrolla.cpp::Create reads s32 spark count, six f32 fields
(178/17C/180/194/198/19C), s32 material, s32 spark material, then
S_NTF_SWITCH_ONOFF. CinematicManager.cpp::S_NTF_SWITCH_ONOFF::Create reads a
count, count x 28-byte S_STREAM_NTF_TARGET_ONOFF and a 32-byte camera record.
The ONOFF target derives from S_STREAM_NTF_TARGET_SWITCH and has a source size
assertion of 0x1c; runtime on/off semantics add no bytes. Material installation,
spark allocations, light initialization and spark parameters are runtime state.
BuildBehaviour maps 2 to CBehaviourElectrolla, whose Create is empty, and
delegates other IDs to CActor (0 Stand / 1 Cinematic).

Actors/KyaElectrolla.h/.cpp implements a 36-byte scalar prefix and reuses the
existing switch target/camera byte codec; minimum payload is 72 bytes. Trailing
bytes remain raw. FKyaActorPlacement exposes all scalars and ElectrollaSwitch.
The dedicated family module, factory, both export tail paths, typed writer and
export capability allowlist are wired. Switch target count is fixed on export;
negative spark counts are rejected because the game allocates that many sparks.
The generic empty-payload map covers behaviour 2. Switch refs remain numeric
without nested pickers. No version gate occurs in these readers. Fixtures,
UI inspection and PS2 address verification remain outstanding; rows are marked `[~]`.
No builds or tests were run.

## Implementation progress: Bomb Launcher

Class 0x53 (`Types.h`, `ActorFactory.cpp`), CActorBombLauncher -> CActor -> CObject.
ActorBombLauncher.cpp::Create reads field 168 f32 and calls CFireShot::Create.
FireShot.cpp::Create only reads an S_ACTOR_STREAM_REF (count + actor indices);
Actor.cpp::S_ACTOR_STREAM_REF::Create advances four bytes per entry. The class
payload is therefore 8 + count*4 bytes. Runtime shot state adds no bytes.
BuildBehaviour maps 2 to CBehaviourBombLauncherStand, whose Create is empty;
all other IDs delegate to CActor, including 0 Stand and 1 Cinematic.

Actors/KyaBombLauncher.h/.cpp implements bounded offsets, typed read/write and
trailing-byte preservation. BombLauncherField168 and BombLauncherActorRefs are
placement fields with capability allowlist entries; the array is EditFixedSize
and its original count is validated by both exporter passes. The family module,
registry, factory, typed writer and raw-tail paths are integrated. The shared
empty map covers behaviour 2. No version gates occur in these readers. Actor
indices remain numeric without per-element pickers. Rows are marked `[~]`:
fixtures, UI inspection and PS2 addresses are not verified. No builds or tests
were run.

## Implementation progress: Blazer

Class 0x4d (`Types.h`, `ActorFactory.cpp`), CActorBlazer -> CActorAutonomous ->
CActorMovable -> CActor -> CObject. Source: ActorBlazer.cpp::Create/BuildBehaviour,
the autonomous/movable readers, FireShot.cpp::Create and S_TILT_CONFIG in
ActorMovable.h. Base payload is weight f32, life u32 and three f32 dynamic values
(20 bytes). Then fields 358/35C u32; 368/364/36C f32; 370 u32; 374/378/37C,
stunned duration, 384 and authored 388 angle f32. The editor keeps angle 388 in
degrees, before the game's radians conversion. Version 2.24 adds two s32 waypoint
refs. Suffix: 398/394 u32, path s32, 3AC/3B0 f32, 3B4 u32, 3B8 f32, CFireShot
actor list, and 32-byte tilt config (f32 field0, s32 tilt mode, tilt value/spring/
damping f32, push value/spring/damping f32). Runtime collision flag changes are
not bytes. Minimum class sizes are 132 legacy and 140 modern, plus count*4.

Actors/KyaBlazer.h/.cpp validates both candidate layouts against the entire
record and decodes only a unique match. Ambiguous or unsupported bytes remain
wholly raw, because scene version is not available to the reader. FKyaBlazerData
stores all fields, with int64 u32 containers and writer bounds. Array count and
waypoint presence cannot change on export. Model, family, registry, factory,
both exporter tail paths, typed writer and capability allowlist are integrated.
BuildBehaviour maps 3 to empty BlazerStand; 0/1/2 inherit Stand/Cinematic/
CatchByTrap. The existing conditioned-operation codec now applies to Blazer id 2,
and id 3 uses the explicit empty inspector. Nested reference fields are numeric,
without pickers. Source-derived fixtures, UI and PS2 addresses remain unverified;
rows are marked `[~]`. No builds or tests were run.

## Implementation progress: Woof

Class 0x2a (`Types.h`, `ActorFactory.cpp`), CActorWoof -> CActorAutonomous ->
CActorMovable -> CActor -> CObject. Source readers: ActorWoof.cpp::Create,
autonomous/movable Create, S_ACTOR_STREAM_REF::Create and Vision.cpp::CVision::Create.
Payload: 20-byte autonomous prefix; u32 fields 420/424/428; f32 fields 390/394;
u32 fields 398/39C; s32 path index and pathfinding ID 41C; actor stream count and
s32 indices; f32 field 418; u32 fields 414/410; s32 waypoint 3A4; vision (four
f32s and u32 flags). Minimum size 96 + actorCount*4 bytes. Vision angle is stored
as authored degrees, and the two authored extents remain full values before the
runtime half-size conversion. Runtime lookups, collision state and vision
trigonometric calculations consume no additional bytes. No version gates occur.

Actors/KyaWoof.h/.cpp owns the codec. FKyaWoofData exposes every field and uses
int64 containers with writer bounds for the nine u32 values. The actor list is
EditFixedSize and export validates its original count. Trailing bytes stay raw.
Family metadata, registry, factory, both export tail paths, typed writer and
capability allowlist are integrated. BuildBehaviour maps 3 Guard and 4 VerticalJump,
both empty Create, and otherwise delegates to CActorAutonomous (0 Stand, 1
Cinematic, 2 CatchByTrap). Empty inspectors cover 3/4; the shared conditioned
operation codec now covers id 2. Nested path/waypoint/actor refs remain numeric
without pickers. Fixtures, UI inspection and PS2 address verification remain
outstanding; inventory rows are marked `[~]`. No builds or tests were run.

## Implementation progress: Nativ Speak, Live, Seller and Exorcism

Class 0x0a mapping was reconciled with ActorNativ.cpp::BuildBehaviour and
ActorNativ.h's NATIVE_BEHAVIOUR constants. The existing Nativ codec now decodes
3 Speak, 5 Live and 7 Seller alongside 6 TakeAndPut. Speak Create reads a signed
count and count x two f32 values (4 + count*8 bytes). Live Create reads f32 fields
8/C (8 bytes). Seller Create reads f32 field 8 then CAddOnNativ::Create
(ActorNativCmd.cpp:529): signed sub-object count, each CAddOnSubObj::Create
(Actor.cpp:5374) reads u32 field 0, signed cinematic count and s32 cinematic IDs.
Minimum Seller payload is 8 bytes, each item adds 8 + cinematicCount*4 bytes.
Runtime state/defaults do not add bytes. No version gates occur in these readers.

KyaNativ.cpp readers require exact record sizes and bounded nonnegative counts;
writers validate original list lengths, including nested cinematic lists.
KyaLevelBank.h exposes NativSpeakValues, NativLiveField8/C and NativSellerField8/
SubObjs. Every group field is gated by the individual behaviour ID; TakeAndPut
fields are now limited to id 6. Factory dispatch and export preflight/write paths
cover the added IDs through the existing shared transaction path. Raw data stays
available when decoding fails. Exorcism id 4 has no Create override in
ActorNativ.h and inherits the empty CBehaviourNativ::Create; its zero-byte layout
is now in the shared explicit-empty map. Cinematic IDs remain numeric without
nested pickers. Akasa id 8 is implemented in the following progress section;
ambiguous or unsupported layouts still fall back to raw bytes.
Fixtures, UI inspection and PS2 addresses remain unverified; rows are marked `[~]`.
No builds or tests were run.

## Implementation progress: Nativ Akasa

Nativ class 0x0a behaviour 8 now dispatches to Actors/KyaNativAkasa.h/.cpp.
Source: CBehaviourNativAkasa::Create, NativComboSequenceStep::Create,
CAddOnNativ::Create, CAddOnSubObj::Create, S_NTF_SWITCH::Create and
ByteCode::GetString (MemoryStream.cpp:30). A material s32 is present from scene
version 2.26; then field 8 u32, tutorial count s32, per tutorial step count s32,
per step combo count s32 (maximum two), null-terminated combo names aligned to
four bytes, an add-on block, and four switch target/camera records. Counts and
string terminators/alignment are bounded before model use. Minimum payload is
156 legacy or 160 modern bytes. Both version candidates must consume the entire
record and only a unique match is decoded; unsupported or ambiguous payloads
remain wholly raw. Unsupported non-ASCII names also remain raw.

FKyaAkasaBehaviour exposes material presence, material, full-range u32 field 8,
nested tutorials/steps/names, add-on sub-objects and all four switches. All list
lengths and material presence are fixed on export. Name edits must fit the same
aligned byte span; unchanged padding remains untouched. Writers validate every
count before writing and re-decode output to reject edits that make version
selection ambiguous. Factory/read/write dispatch uses the Nativ codec and shared
transaction path; inspector applicability is limited to id 8. References remain
numeric. Fixtures, UI inspection and PS2 address verification remain outstanding;
rows are marked `[~]`. No builds or tests were run.

## Implementation progress: Minigames Manager and Organizer

Classes 0x42/0x43 (`Types.h`, `ActorFactory.cpp`), CActorMiniGamesManager and
CActorMiniGamesOrganizer each derive from CActor -> CObject. Source Create and
BuildBehaviour functions are in their corresponding .cpp files. Manager reads
S_ACTOR_STREAM_REF (count + s32 organizer indices), then one s32 actor reference;
minimum payload is 8 bytes. Organizer reads ByteCode::GetString (null-terminated,
four-byte aligned), four s32 fields (16C, texture, mesh, material), a minigame
actor stream, two u32 fields (180/184), three s32 fields (188/18C/190), and f32
field 194. Runtime mesh resets consume no bytes. No version gates occur.
Both BuildBehaviour functions map 2 to their empty Stand Create; other IDs
delegate to CActor (0 Stand and 1 Cinematic).

Actors/KyaMiniGamesSupport.h/.cpp shares the bounded codec dispatch. Manager
placement properties and FKyaMiniGamesOrganizerData expose every field. Export
fixes actor-list counts and Organizer's aligned string span, rejects non-ASCII
name edits/unsupported encodings, validates u32 ranges, preserves padding and
retains unknown trailing bytes. Family metadata, registry, factory, both export
tail paths, typed writer, capability metadata and the shared empty map are wired.
Manager's scalar actor ref has a picker; nested actor/mesh/texture references
remain numeric. Edits use shared placement transactions. Fixtures, UI inspection
and PS2 address verification remain outstanding; inventory rows are marked `[~]`.
No builds or tests were run.

## Implementation progress: Credits

Class 0x56 (`Types.h`, `ActorFactory.cpp`), CActorCredits -> CActor -> CObject.
ActorCredits.cpp::Create reads signed message count + count x u64 message hashes,
then nine f32 display settings (scroll speed, X/Y, width/height, fill/border
opacity, border width/height), music u32, three f32 music settings, three s32
next-level fields and S_NTF_SWITCH. Minimum payload is 104 bytes, plus eight
bytes per message and 28 per completion target. X/width/border-width remain in
authored 512-based units and Y/height/border-height in authored 448-based units,
before runtime screen scaling. Audio lookup, scenario lookup and runtime state
do not add reads. No version gates occur. BuildBehaviour maps 2 to
CBehaviourCreditsDefault -> CBehaviourCredits -> CBehaviour; neither derived
class overrides Create, so Actor.cpp::CBehaviour::Create establishes an empty
payload. Other IDs delegate to CActor (0 Stand / 1 Cinematic).

Actors/KyaCredits.h/.cpp owns bounded offsets and typed read/write. FKyaCreditsData
exposes every serialized field; signed int64 containers retain all message-hash
bits and a bounded int64 music index retains the 0xffffffff sentinel. Message
and completion-target counts remain fixed on export, with both export passes
checking them. Unknown trailing bytes remain raw. Family metadata, registry,
factory, writer dispatch, capability allowlist and empty-default inspector are
integrated. Nested switch references and music index remain numeric. Fixtures,
UI inspection and PS2 addresses remain unverified; rows are marked `[~]`. No builds
or tests were run.

## Implementation progress: Wanted Zoo

Class 0x54 (`Types.h`, `ActorFactory.cpp`), CActorWantedZoo -> CActor -> CObject.
ActorWantedZoo.cpp::Create reads two signed counts with count x f32 XY positions,
then a signed entry count. Each entry reads CScenaricCondition (count + count x
16-byte combiner/variable/comparison/value), a signed message-key count with
count x u64 keys, and a signed numeric-label count with count x s32 values.
ScenaricCondition.cpp::Create normalizes the first combiner at runtime; the editor
retains its authored bytes. Stream order is display priority; an empty condition
list is an unconditional fallback. Position values remain normalized screen
coordinates, before runtime multiplication. No version gate or custom behaviour
factory exists; 0 Stand and 1 Cinematic are inherited.

Actors/KyaWantedZoo.h/.cpp owns bounded offsets/read/write. FKyaWantedZooData and
FKyaWantedZooEntry expose both position lists, every gate, message key and numeric
label. Negative/truncated counts are rejected; every list is EditFixedSize and
both export passes validate all outer and nested counts. Hash containers retain
all 64 bits, authored condition combiners remain unchanged and trailing raw bytes
are preserved. Family metadata, registry, factory, typed writer and capability
allowlist are integrated. Conditions use the existing property customization and
edits use shared placement transactions. The source contains prior PS2 comparison
notes for Create 0x00404c20 and Draw 0x00404810; these were not re-verified here.
Fixtures and UI inspection remain outstanding; rows are marked `[~]`. No builds
or tests were run.

## Implementation progress: JamGut behaviours

Class 0x33 (`Types.h`, `ActorFactory.cpp`), CActorJamGut -> CActorAutonomous ->
CActorMovable -> CActor -> CObject. ActorJamGut.cpp::BuildBehaviour maps 3 Stand,
4 Ridden, otherwise delegates to CActorAutonomous (0 Stand, 1 Cinematic,
2 CatchByTrap). CBehaviourJamGutStand::Create is empty and now has the explicit
empty inspector. CBehaviourJamGutRidden::Create reads music u32 then fields
14/18/1C f32 (16 bytes); the audio pointer lookup and field C reset are runtime
state. Actors/KyaJamGut.h/.cpp implements the behaviour codec, preserving the
0xffffffff music sentinel in a bounded int64 property. Factory, writer and
export validation are integrated; raw bytes remain available on decode failure.
The existing conditioned-operation decoder now applies to JamGut id 2. All edits
use shared behaviour/placement transactions.

The class codec is now implemented in Actors/KyaJamGutActor.cpp. After the
20-byte autonomous prefix, Create reads six f32 fields 400/404/408/40C/410/414,
optional f32 field 41C from scene version 2.28, f32 field 4A4, u32 field 374 and
flags 358, signed checkpoint count + pairs of s32 zone/waypoint refs, signed path
count + 24-byte records (s32 zone/path/actor refs and f32 fields 28/2C/30), then
s32 field 368 type. Minimum sizes are 68 legacy and 72 modern, plus checkpoint
count*8 and path count*24. Runtime default field 41C=50 is not serialized in the
legacy layout. Both candidate layouts must fit the whole record; only a unique
match becomes typed, otherwise all bytes remain raw.

FKyaJamGutData and nested checkpoint/path models expose all fields. Factory,
both export tail paths, typed writer and capability metadata are integrated.
Counts and optional presence remain fixed, unsigned fields have writer bounds,
and post-write decoding rejects newly ambiguous layouts. Nested references and
Ridden music index remain numeric. Source-derived fixtures, UI inspection and PS2
addresses are unverified; rows are marked `[~]`. No builds or tests were run.

## Implementation progress: inherited Stand applicability

KyaActorFamilies::HasEmptyPayload now recognizes id 0 for the source-traced
Nativ, Electrolla, WoodMonster, Woof, HelperSign, Egg, AmortosCmd, JamGut,
CareBoy, Acceleratos, PatternCmd, MiniGamesManager/Organizer, Hunter,
AddOnPurchase, Blazer, ShockerCmd, BombLauncher, WantedZoo and Credits chains.
Their BuildBehaviour functions delegate id 0 to CActor (directly or through
CActorAutonomous); CActor::BuildBehaviour selects standBehaviour for id 0 and
Actor.h::CBehaviourStand::Create is explicitly empty. The factory still requires
exactly zero payload bytes before promoting a record to the empty inspector;
export validates class/ID applicability through the same map. Unknown classes
and unknown IDs stay raw. This closes the inherited empty-inspector gap for
those families without inferring emptiness from payload length alone. UI and
fixture validation remain outstanding. No builds or tests were run.

## Implementation progress: inherited CatchByTrap dispatch

KyaActorFamilies::UsesCatchByTrap centralizes verified actor-specific id-2
applicability for import and export. It covers Micken, Nativ, Projectile,
Hedgehog, Woof, JamGut, Aton, Shoot, Hunter and Blazer. Hedgehog, Aton and Shoot
were added after tracing their BuildBehaviour fallbacks to CActorAutonomous;
none overrides id 2. The existing conditioned-operation reader/writer now applies
to those three consumers as well. Unknown classes/IDs are excluded. Array-count
validation, raw fallback and shared undo/refresh follow the existing Catch codec.
Fixtures, UI and PS2 address verification remain outstanding. No builds or tests
were run.

## Findings: Shared base classes

Status: **shared, non-instantiated bases documented. They have no class id of
their own; their serialized records are consumed by the family modules.**
Source traces are from `CActor::Create` (`Actor.cpp:693`), `CActorMovable::Create`
(`ActorMovable.cpp`), `CActorAutonomous::Create` (`ActorAutonomous.cpp`),
`CActorCinematic::Create` (`Actor_Cinematic.cpp:220`), `CActorFighter::Create`
and `CActorHeroPrivate::Create` (`ActorHero_Private.cpp`).

| Base | Serialized contract | Editor path | Evidence | Exclusions |
| --- | --- | --- | --- | --- |
| `CActor` | Name string + 4-byte align, placement, macro-animation list, model strings, common block (default behaviour, bounding sphere, clipping, visibility/culling distances, field 20, sector, hash, lod bias, actor flags, animation layers, lighting, scenario conditions) | `KyaReadActorOffsets` (`KyaCameraStream.h`), `FKyaActorPlacement` (`KyaLevelBank.h`), factory common block | Every real-bank round trip (`Kya.Bank.LevelOneRoundTrip`, `Kya.Bank.SelectedActorRoundTrip`) and every fixture test | Objective/condition/map chunks are scene-level, not actor records |
| `CActorMovable` | Weight f32 at class payload +0 (4 bytes) | Consumed by every Movable/Autonomous family codec (e.g. `KyaHedgehog.cpp` weight at offset 0) | Family fixtures assert weight; `LevelOne`/`SelectedActor` round trips | None |
| `CActorAutonomous` | Life maximum u32 at +4, gravity scale f32 at +8, dynamic-extension f32 at +12 and +16 (16 bytes) | Consumed by every Autonomous family codec (`KyaHedgehog.cpp`, `KyaAton.cpp`, `KyaShoot.cpp`, ...), writers range-check the u32 | Fixtures assert exact u32 values (e.g. 16777217) and gravity; `LevelOne` | None |
| `CActorFighter` | `Actors/KyaFighter.h` prefix: weight, autonomous prefix, 156-byte fixed block, blow/grab/react/combo arrays, weapon index | `KyaFighter.h` shared reader; Wolfen and Kim class modules | `Kya.Bank.WolfenFixtureRoundTrip`, `Kya.Bank.KimFixtureRoundTrip` | Fight-parameter block remains Wolfen-specific |
| `CActorHero` | No `CActorHero::Create` body exists in the recovered source; class 0x06 instantiates `CActorHeroPrivate` | `Actors/KyaKim.h/.cpp` (Kim finding) | `Kya.Bank.KimFixtureRoundTrip` | Inherited record is audited as part of `CActorHeroPrivate` |
| `CActorCinematic` | `Create(ByteCode*)` returns immediately: empty class payload | Generic zero-length class-data fallback; no typed fields | Cross-bank scan: class 0x00 (`WOLFENFLAG`, `ATEA_CIN`) reports `withRaw=0` in 6 banks (LEVEL_2/3/9/10/12/NATIV) | None |

## Findings: Shared base behaviours

Status: **shared behaviour records documented; typed where the plugin already has
a codec, explicit disposition otherwise.** Source traces are from `Actor.cpp`
and the named actor sources.

| Behaviour | Serialized contract | Editor path / disposition | Evidence |
| --- | --- | --- | --- |
| `CBehaviour` | `Create` returns immediately: empty payload | Empty payload is only promoted through the verified `KyaActorFamilies::HasEmptyPayload` applicability map; factory still requires size 0 | Inherited Stand applicability note; every zero-size behaviour row |
| `CBehaviourStand` | `Create` is not overridden: empty payload; id 0 via `CActor::BuildBehaviour` | `KyaActorFamilies::HasEmptyPayload` traces id 0 to `CActor`; factory promotes only on zero bytes | Bank inventories show id 0 size 0 (e.g. DCA `AIRCANON_L1`) |
| `CBehaviourInactive` | 12 bytes: activate message s32, flags u32, active behaviour id s32 (`Actor.cpp:5655`) | Typed for Projectile id 12 (`InactiveActivateMessageId`, `InactiveFlags`, `InactiveActiveBehaviourId`); other classes keep the raw payload | `Actors/KyaProjectile.*`; source layout above |
| `CBehaviourCinematic` | 4 bytes: leave-behaviour id s32 (`Actor_Cinematic.cpp:272`) | Typed for id 1 with size 4 (`bHasTypedCinematicBehaviour`, `CinematicLeaveBehaviourId`); writer validates size 4 | Bank inventories: id 1 size 4 for Wolfen, Kim, Aton and others |
| `CBehaviourAutonomous` | No `Create` override: empty payload | Empty disposition through the applicability map | `ActorAutonomous.cpp`; zero-size id records |
| `CBehaviourCatchByTrap` | Conditioned-operation block array (`condOpArray.Create`, `ActorAutonomous.cpp:2187`) | `Actors/KyaCatchByTrap.h/.cpp` + `KyaActorFamilies::UsesCatchByTrap`; readers/writers shared | Micken/Nativ/Projectile/Hedgehog/Aton/Shoot/Hunter/Blazer inventories: id 2 (4/24 bytes) |

## Findings: Implemented behaviours without a dedicated payload codec

Status: **rows that are implemented through shared/family code or are empty by
source contract; no new class ids.**

| Behaviour | Disposition | Evidence |
| --- | --- | --- |
| `CActorBridge::CBhvBasic` | 32-byte basic payload handled by `Actors/KyaBridge.*` (direction + five floats) | `Kya.Bank.BridgeFixtureRoundTrip`; LEVEL_2 class 0x17 id 2 size 32 |
| `CActorBridge::CBhvWindAware` | 20-byte wind payload handled by `Actors/KyaBridge.*` (five floats) | `Kya.Bank.BridgeFixtureRoundTrip`; source-derived (absent from LEVEL_2) |
| `CActorDCA::CBhvIAControlled` | id 4 source `Create` is `IMPLEMENTATION_GUARD()`; payload retained raw, not typed | `ActorDCA.cpp:998`; no id-4 instance in LEVEL_1/2 inventories |
| `CBehaviourCommanderBeatUp` | No `Create` override; id 3 empty on top of `CBehaviourCommander` | `ActorCommander.h`; LEVEL_2 inventory shows only id 2 |
| `CBehaviourInventory` | `inventoryInfo.Create` + initial behaviour id s32 (`BehaviourInventory.cpp:6`); typed for Projectile id 9 | `Actors/KyaProjectile.*`, `Kya.Bank.ProjectileFixtureRoundTrip` |
| `CBehaviourRideJamGut` / `CBehaviourHeroRideJamGut` | `CBehaviourRideJamGut::Create` reads the mount/command record (`ActorHero.cpp:1901`); `HeroRideJamGut` adds no `Create`. Retained raw pending JamGut bank evidence | Source layout above; no mounted-JamGut instance in the scanned banks |
| `CBehaviourFighter` / `...Slave` / `...Ridden` / `...Projected` | No `Create` overrides: empty payloads; Wolfen ids 3/4/5/6 dispatch here | LEVEL_2 inventory: Wolfen ids 3/4/5/6 size 0 |
| `CBehaviourFighterWolfen` / `CBehaviourWolfenFighterRidden` / `...Slave` / `...Projected` | `CBehaviourFighterWolfen::Create` returns immediately (`ActorWolfen.cpp:10813`); derived classes add no `Create` | Source above; Wolfen ids 3/4/5/6 size 0 |
| `CBehaviourTrackStand` | Inherits `CBehaviourWolfen::Create` (flags u32, 4 bytes); no own `Create` | `ActorWolfen.cpp:9461`; no id-16 instance in LEVEL_2 |
| `CBehaviourSleep` | Inherits `CBehaviourWatchDog::Create`; same layout as Wolfen id 10 (88 bytes in LEVEL_2) | `ActorWolfen.cpp:9332`; LEVEL_2 id 12 size 88. Wolfen codec currently handles id 10 but not 12, so id 12 stays raw |
| `CBehaviourWolfenGrindDeath` | Extends `CBehaviour`; no `Create` override: empty payload | `ActorWolfen.h:899`; no id-25 instance in LEVEL_2 |
| `S_MINI_GAME_NAME_BEHAVIOUR` | Not a `CBehaviour`: a name/tag struct used by the minigame tables | `ActorMiniGame.cpp` declaration; not a serialized behaviour row |

## Findings: Event generator

Status: **class header and behaviour header typed; manager payloads stay raw**.

- Source classes: `CActorEventGenerator` (`CActor` -> `CObject`) and
  `CBehaviourEventGen`.
- Factory registration: class 0x3B (`Types.h`; `CActorEventGenerator::Create`
  at `ActorEventGenerator.cpp:359`).
- `BuildBehaviour` only adds id 2 (`behaviourEventGen`); ids 0/1 are inherited.
- Class payload: `u32` global-event flag, `s32` zone reference, then twelve
  `PTMF_Ext` manager `Create` calls. Six of the twelve registrations are
  `DummyTempFunc` placeholders in the decompiled source
  (`ActorEventGenerator.cpp:389-404`), so the manager byte order cannot be
  reconstructed exactly; the leading 8 bytes are typed and the rest is preserved
  as `ClassData`.
- Behaviour 2 payload: `s32` field 0xC, `f32` field 0x10, then nine manager
  `Create` calls. Source layouts: camera s32, cinematic s32, message count +
  12-byte entries, slow-down count + 16-byte entries, quake-A count + 16-byte
  entries, quake-B count + 24-byte entries (one read is inside a commented
  `CAM_QUAKE` call, so its original width is not provable), quake-C count +
  28-byte entries, draw-A fixed 8 bytes, draw-B count + 20-byte entries. Only
  the 8-byte header is typed; manager bytes stay raw.
- Editor model/reader/writer: `Public/Actors/KyaEventGenerator.h` and
  `Private/Actors/KyaEventGenerator.cpp`; factory class/behaviour dispatch and
  exporter writer wired; catalog family `KyaEventGeneratorActorFamily.cpp`
  registers `CActorEventGenerator` and ids 0/1/2. Model fields:
  `EventGeneratorEnabled`, `EventGeneratorZoneRef`,
  `EventGeneratorFieldC`, `EventGeneratorField10`.
- Capability/applicability: typed header fields are editable; the raw manager
  bytes are preserved byte-for-byte. The generic behaviour `Data` gate is left
  unchanged because the meta string is already at the engine's 1024-character
  limit, so the raw block remains visible for this family.
- Tests: `Kya.Bank.EventGeneratorFixtureRoundTrip` (class flag/zone, behaviour
  header, raw tails, byte-identical unedited export, four edited fields through
  export/reimport). Real-bank evidence: `Kya.Bank.SelectedActorRoundTrip` run
  against `LEVEL_2/LEVEL.BNK` exports the bank byte-identically with the new
  reader/writer (LEVEL_2 has five class-0x3b actors, ids 2 sizes 56/60, raw
  class payloads 60-68 bytes).
- Remaining exclusions: manager payloads are not typed; the six
  `DummyTempFunc` registrations and the commented `CAM_QUAKE` read need Ghidra
  or a matching original binary before their byte order can be proven.

## Findings: Explosive distributor

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorExplosiveDistributor` (`CActor` -> `CObject`) and
  `CBehaviourExplosiveDistributor`.
- Factory registration: class 0x1D; `CActorExplosiveDistributor::Create` at
  `ActorExplosiveDistributor.cpp:19`.
- `BuildBehaviour` only adds id 2; ids 0/1 are inherited.
  `CBehaviourExplosiveDistributor::Create` returns immediately (empty payload).
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | radius | f32 (the game squares it at runtime) |
  | angle | f32 (the game takes its cosine at runtime) |
  | projectiles | count u32 + count x 20-byte record |
  | projectile | actor reference s32, direction xyz f32, eject force f32 |
  | camera | 32-byte `S_STREAM_EVENT_CAMERA` (index, flags, three f32, actor, zone, one f32) |

  Bytes after the camera remain raw in `ClassData`. The 20-byte stride and
  44-byte fixed part explain the observed bank sizes: 64 (1 projectile), 84 (2)
  and 124 (4).
- Editor model/reader/writer: `Public/Actors/KyaExplosiveDistributor.h` and
  `Private/Actors/KyaExplosiveDistributor.cpp`; model fields
  `ExplosiveDistributorRadius`, `ExplosiveDistributorAngle`,
  `ExplosiveDistributorProjectiles`, `ExplosiveDistributorCamera`. Factory class
  dispatch and exporter writer wired; `ExplosiveDistributor.Tail` is in both
  `RawOffset` chains. Catalog family
  `KyaExplosiveDistributorActorFamily.cpp` registers `CActorExplosiveDistributor`
  and ids 0/1/2.
- Capability/applicability: every serialized field is editable; the projectile
  count is validated before writing so the optional list cannot change size.
  Runtime transformations (squaring the radius, cosine of the angle, direction
  normalisation) are not applied to the stored values.
- Tests: `Kya.Bank.ExplosiveDistributorFixtureRoundTrip` (two projectiles, full
  camera, byte-identical unedited export, seven edited fields through
  export/reimport, projectile-count rejection). Real-bank evidence:
  `Kya.Bank.SelectedActorRoundTrip` against `LEVEL_2/LEVEL.BNK` exports
  byte-identically; LEVEL_2 has class-0x1d actors with 64-byte class payloads.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Direction/eject-force units are the stored values; runtime scaling
  is out of scope.

## Findings: Shocker

Status: **class and fire-wave behaviour codecs extracted**.

- Source classes: `CActorShocker` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor`), `CBehaviourShocker` and `CBehaviourShockerFireWave`.
- Factory registration: class 0x4B; `CActorShocker::Create` at
  `ActorShocker.cpp:54`.
- `BuildBehaviour` only adds id 3 (`SHOCKER_BEHAVIOUR_FIRE_WAVE`); ids 0/1/2 are
  inherited (`CBehaviourShocker::Create` is empty).
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | autonomous prefix | weight f32, life u32, gravity f32, two dynamic-ext f32 (20 bytes) |
  | tuning | fields 350/354/358/35C/364/368/360/F0/36C f32 (nine) |
  | zones | inner zone s32 (0x374), outer zone s32 (0x370) |
  | switch | `S_NTF_SWITCH_ONOFF`: count u32 + count x 28-byte target + 32-byte camera |
  | fields | field 380 u32, fields 384/388/38C/390/394/398 s32, bone id u32 |
  | add-on | 20-byte `CAddOnGenerator`; sub-object mode u32, then five f32 when 1 |

  Bytes after the add-on remain raw in `ClassData`. With no switch targets the
  record is 156 bytes (mode 0) or 176 (mode 1); each switch target adds 28,
  matching the observed 156/184 bank payloads.
- Behaviour 3 payload (`CBehaviourShockerFireWave::Create` ->
  `CCircularWaveShoot::Create`): six f32, material id s32 and field 24 s32
  (32 bytes, matching every LEVEL_2 shocker).
- Editor model/reader/writer: `Public/Actors/KyaShocker.h` and
  `Private/Actors/KyaShocker.cpp`; model fields `ShockerField350`...
  `ShockerBoneId`, `ShockerSwitch`, `ShockerAddOn`, `ShockerWave`. Factory class
  and behaviour dispatch wired; `Shocker.Tail` is in both `RawOffset` chains.
  Catalog family `KyaShockerActorFamily.cpp` registers `CActorShocker` and ids
  0/1/2/3.
- Capability/applicability: every serialized field is editable; switch target
  count and add-on mode are validated before writing. Runtime animation lookups
  (`FUN_001156e0`) and the derived spark parameters are not serialized.
- Tests: `Kya.Bank.ShockerFixtureRoundTrip` (full class payload with one switch
  target, mode-1 add-on, wave behaviour; byte-identical unedited export, eight
  edited fields through export/reimport, switch-count rejection). Real-bank
  evidence: `Kya.Bank.SelectedActorRoundTrip` against `LEVEL_2/LEVEL.BNK`
  exports byte-identically; LEVEL_2 has four class-0x4b actors with behaviour 3
  (32 bytes).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Material/bone/zone ids are scalar references without per-entry
  pickers.

## Findings: Nativ shop

Status: **class codec and behaviour id 2 typed**.

- Source classes: `CActorNativShop` (`CActor` -> `CObject`) and
  `CBehaviourNativShopSell`.
- Factory registration: class 0x34; `CActorNativShop::Create` at
  `ActorNativShop.cpp:33`.
- `BuildBehaviour` only adds id 2; ids 0/1 are inherited from `CActor`
  (`CBehaviourStand::Create` empty, `CBehaviourCinematic` 4 bytes).
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | radius | f32 (the game squares it at runtime) |
  | class name | NUL-terminated string (`ByteCode::GetString`), seek aligned up to 4 |
  | actors | `S_ACTOR_STREAM_REF`: count u32 + count x 4-byte actor references |
  | episode link | episode threshold link u32 (`episodeThresholdLink`) |
  | field | field 0x174 u32 |
  | actor | actor reference s32 |

  `field_0x17C` is set to zero at runtime and is not serialized. The observed
  payload sizes match 4 + aligned string + 4 + 4n + 12: LEVEL_4's 64-byte
  payloads use 13-character names (16-byte aligned string) with seven actor
  entries, and NATIV's 76-byte payloads are consistent with ten.
- Behaviour 2 payload (`CBehaviourNativShopSell::Create`, 52 bytes):

  | Section | Layout |
  | --- | --- |
  | camera | camera stream reference s32 |
  | sub-objects | five actor stream references s32 (purchasable slots) |
  | path | `CPathFollowReader` index s32 |
  | material | material id s32 |
  | music | music index u32 |
  | zone | zone reference s32 |
  | fields | fields 0xC4/0xC8/0xCC f32 |

  The per-slot `field_0x4/8/C/10`, `bCanPurchase` and
  `selectedPurchaseItemIndex` are derived in memory and are not serialized.
- Editor model/reader/writer: `Public/Actors/KyaNativShop.h` and
  `Private/Actors/KyaNativShop.cpp`; model fields `NativShopRadius`,
  `NativShopName`, `NativShopActorRefs`, `NativShopEpisodeThresholdLink`,
  `NativShopField174`, `NativShopActorRef`, `NativShopCameraRef`,
  `NativShopSubObjActorRefs`, `NativShopPathIndex`, `NativShopMaterialId`,
  `NativShopMusicIndex`, `NativShopZoneRef`, `NativShopFieldC4/C8/CC`. Factory
  class/behaviour dispatch, exporter writer and `NativShop.Tail` in both
  `RawOffset` chains. Catalog family `KyaNativShopActorFamily.cpp` registers
  `CActorNativShop` and ids 0/1/2; 0x34 was added to
  `KyaActorFamilies::HasEmptyPayload` for id 0.
- Capability/applicability: every serialized field is editable. The class name
  is written inside its original aligned span and is rejected if longer (the
  game stores an in-place pointer); the actor list count is validated before
  writing.
- Tests: `Kya.Bank.NativShopFixtureRoundTrip` (class name and actor list, all
  52 behaviour bytes, byte-identical unedited export, nine edited fields through
  export/reimport, actor-count rejection, over-long name rejection).
  Real-bank evidence: `Kya.Bank.SelectedActorRoundTrip` against both
  `LEVEL_4/LEVEL.BNK` (5 class-0x34 actors, previously raw up to 64 bytes) and
  `NATIV/LEVEL.BNK` (9 class-0x34 actors) exports byte-identically; a
  post-change class inventory reports `withRaw=0` for 0x34 in NATIV.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The music index is kept numeric; runtime `CAudioManager` pointer
  resolution is out of scope.

## Findings: Punching ball

Status: **class codec extracted; all five BuildBehaviour ids audited as empty**.

- Source classes: `CActorPunchingBall` (`CActorFighter` -> `CActorAutonomous` ->
  `CActorMovable` -> `CActor`), `CBehaviourPunchingBall`,
  `CBehaviourPunchingBallStand`, `CBehaviourPunchingBallSlave`,
  `CBehaviourPunchingBallRidden` and `CBehaviourPunchingBallDefault`.
- Factory registration: class 0x30; `CActorPunchingBall::Create` at
  `ActorPunchingBall.cpp:9`.
- `BuildBehaviour` adds id 3 (fighter default), 4 (projected), 5 (ridden),
  6 (slave) and 7 (`PUNCHING_BALL_BEHAVIOUR_STAND`); every one of those
  `Create` bodies returns immediately, so all five payloads are empty
  (`ActorPunchingBall.cpp:530`, `:552`). Ids 0/1 are inherited from `CActor`.
- Class payload layout (after the shared `CActorFighter` prefix):

  | Section | Layout |
  | --- | --- |
  | tuning | fields 0xA80/0xA84/0xA88/0xA8C f32 |
  | trap stream | `S_TRAP_STREAM_REF`: count u32 + count x 60-byte entry |
  | trap entry | `S_STREAM_NTF_TARGET_SWITCH` (target ref, cutscene id, flags, message id, message flags, fields 0x14/0x18) + 32-byte `S_STREAM_EVENT_CAMERA` |
  | actors | actor references 0xA94 and 0xA98 s32 |
  | camera | `CCamFigData::Create`: exactly fourteen f32 |

  The fighter prefix is parsed with the shared `Actors/KyaFighter.h` reader
  (`KyaReadFighterPrefixEnd`) but its bytes are left untouched; only the
  PunchingBall block is typed and written. The 60-byte trap stride and 56-byte
  camera block explain the observed payloads (84 bytes plus 60 per target;
  cross-bank maximum 728 including the fighter prefix).
- Editor model/reader/writer: `Public/Actors/KyaPunchingBall.h` and
  `Private/Actors/KyaPunchingBall.cpp`; model fields `PunchingBallFieldA80`
  ... `PunchingBallFieldA8C`, `PunchingBallTrapEntries` (typed
  `FKyaPunchingBallTrapEntry` incl. camera), `PunchingBallActorRefA94/A98` and
  `PunchingBallCamFigFloats`. Factory class dispatch, exporter writer and
  `PunchingBall.Tail` in both `RawOffset` chains. Catalog family
  `KyaPunchingBallActorFamily.cpp` registers `CActorPunchingBall` and ids
  0/1/3/4/5/6/7; `0x30` was added to `KyaActorFamilies::HasEmptyPayload` for
  id 0 and ids 3-7.
- Capability/applicability: the trap list count and the fourteen-float camera
  list are validated before writing, so neither can change size. The fighter
  prefix remains raw and is not exposed for editing through this module.
- Tests: `Kya.Bank.PunchingBallFixtureRoundTrip` (fighter prefix, four tuning
  floats, one trap entry with a full camera, two actor refs, fourteen camera
  floats, two empty behaviours, byte-identical unedited export, five edited
  fields through export/reimport, trap-count and float-count rejection).
  Real-bank evidence: `Kya.Bank.SelectedActorRoundTrip` against both
  `LEVEL_4/LEVEL.BNK` and `NATIV/LEVEL.BNK` exports byte-identically; a
  post-change class inventory reports `withRaw=0` for 0x30 in both banks
  (previously raw up to 728 bytes).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. `CCamFigData`'s derived runtime angles/scales and the fighter
  prefix's fight-parameter records are not re-exposed here.

## Findings: Nose monster

Status: **class codec extracted; all three BuildBehaviour ids audited as empty**.

- Source classes: `CActorNoseMonster` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor`), `CBehaviourNoseMonster`, `CBehaviourNoseMonsterSleep`,
  `CBehaviourNoseMonsterAttack` and `CBehaviourNoseMonsterDeath`.
- Factory registration: class 0x26; `CActorNoseMonster::Create` at
  `ActorNoseMonster.cpp:17`.
- `BuildBehaviour` adds id 2 (sleep), 3 (attack) and 4 (death); all three
  inherit the empty `CBehaviourNoseMonster::Create`
  (`ActorNoseMonster.cpp:1124`). Ids 0/1 are inherited.
- Class payload layout (after the 20-byte autonomous prefix):

  | Section | Layout |
  | --- | --- |
  | zone | zone reference s32 |
  | tuning | fields 0x384/0x354/0x358/0x35C/0x360 f32 |
  | version block | fields 0x378/0x37C f32, present only when `CScene::field_0x1C >= 2.21` |
  | fields | field 0x364 u32, fields 0x370/0x374 f32 |
  | zone paths | `ZonePathEntryList`: count u32 + count x 8-byte `ZonePathEntry` (zone ref + path-follow ref) |
  | field | field 0x36C u32 |
  | fire shot | `CFireShot`: `S_ACTOR_STREAM_REF` count u32 + count x 4-byte actor refs |
  | fields | field 0x630 s32, field 0x380 u32, fields 0x690/0x694/0x698/0x69C f32, fields 0x6A0/0x6A4/0x6A8 u32 |
  | notification | `S_NTF_SWITCH_ONOFF`: count u32 + count x 28-byte target + 32-byte camera |

  The scene-version gate changes the record by eight bytes, so the reader parses
  both shapes and selects the one that consumes the payload exactly (falling
  back to the versioned shape). The writer rejects any attempt to change the
  detected layout. The LEVEL_2 actor resolves to the versioned shape.
- Editor model/reader/writer: `Public/Actors/KyaNoseMonster.h` and
  `Private/Actors/KyaNoseMonster.cpp`; model fields `NoseMonsterZoneRef`,
  `NoseMonsterField384/354/358/35C/360`, `NoseMonsterField378/37C`,
  `NoseMonsterField364/370/374`, `NoseMonsterZonePathEntries`,
  `NoseMonsterField36C`, `NoseMonsterFireShotActorRefs`, `NoseMonsterField630`,
  `NoseMonsterField380`, `NoseMonsterField690/694/698/69C`,
  `NoseMonsterField6A0/6A4/6A8` and `NoseMonsterSwitch`, plus the read-only
  `bNoseMonsterHasVersionFloats` shape flag. Factory class dispatch, exporter
  writer and `NoseMonster.Tail` in both `RawOffset` chains. Catalog family
  `KyaNoseMonsterActorFamily.cpp` registers `CActorNoseMonster` and ids
  0/1/2/3/4; `0x26` was added to `KyaActorFamilies::HasEmptyPayload` for id 0
  and ids 2-4.
- Capability/applicability: every serialized field is editable except the
  layout-shape flag; zone-path, fire-shot and switch counts are validated before
  writing. Runtime substitutions (radius default 5.0, angle defaults) are not
  applied to the stored values.
- Tests: `Kya.Bank.NoseMonsterFixtureRoundTrip` (versioned layout with one zone
  path, two fire-shot refs and one switch target, three empty behaviours,
  byte-identical unedited export, seven edited fields through export/reimport,
  layout-version and zone-path-count rejection). Real-bank evidence:
  `Kya.Bank.SelectedActorRoundTrip` against `LEVEL_2/LEVEL.BNK` exports
  byte-identically, the post-change class inventory reports `withRaw=0`
  (previously raw up to 304 bytes), and a direct probe shows the real actor uses
  the versioned shape (`versionFloats=True`, zone 109, field 0x36C 5).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The scene-version value used by the game is inferred from record
  length, not from a bank-level version field (the plugin does not expose one).

## Findings: Pattern controller

Status: **class and behaviour 3 codecs extracted; behaviour 4 audited as empty**.

- Source classes: `CActorPattern` (class 0x3F, `PATTERN_CON`; `CActorAutonomous`
  -> `CActorMovable` -> `CActor`), `CBehaviourPattern`,
  `CBehaviourPatternAlone` and `CBehaviourPatternMultiple`.
- Factory registration: class 0x3F (`ActorFactory.cpp:387`); bank actors are
  named `LINER`/`LINER_L1`, which is why the cross-bank table lists 0x3F as
  LINER. `CActorPattern::Create` is at `ActorPattern.cpp:15`.
- `BuildBehaviour` adds id 3 (alone) and 4 (multiple); `CBehaviourPattern` and
  `CBehaviourPatternMultiple::Create` are empty (`ActorPattern.cpp:241`,
  `:342`). Ids 0/1 are inherited and id 2 is the shared catch-by-trap
  behaviour (`CActorAutonomous::BuildBehaviour`).
- Class payload layout (after the 20-byte autonomous prefix):

  | Section | Layout |
  | --- | --- |
  | tuning | fields 0x350/0x354/0x358/0x35C/0x364/0x360/0x368 f32 |
  | fields | field 0x36C s32, field 0x370 s32 |
  | bone | bone id u32 |
  | add-on | `CAddOnGenerator`: field 0, money type, money count, bonus count, field 0xC (five words) |
  | add-on sub-object | mode u32; when 1, five f32 (scene angles/scale) |

  The bank records are 84 bytes with mode 0; mode 1 adds 20 bytes.
- Behaviour 3 payload (`CBehaviourPatternAlone::Create` -> `CPatternPart`):

  | Section | Layout |
  | --- | --- |
  | tuning | fields 0x90/0x94/0x98 f32 |
  | pattern part | six f32 (fields 0x4/0x0/0xC/0x8/0x10/0x14) |
  | material | material id s32 |
  | fields | field 0x1C f32, field 0x20 f32 |
  | version field | field 0x24 f32, present only when `CScene::field_0x1C >= 2.21` |
  | field | field 0x28 u32 |

  Observed payloads are 56 bytes (versioned); the reader also accepts the
  52-byte older shape and rejects a layout change on write.
- Editor model/reader/writer: `Public/Actors/KyaPattern.h` and
  `Private/Actors/KyaPattern.cpp`; model fields `PatternField350` ...
  `PatternField368`, `PatternField36C/370`, `PatternBoneId`, `PatternAddOn`,
  and the behaviour fields `PatternField90/94/98`,
  `PatternPartField4/0/C/8/10/14`, `PatternPartMaterialId`,
  `PatternPartField1C/20/24/28`, plus the read-only
  `bPatternPartHasField24`. Factory class/behaviour dispatch, exporter writer
  and `Pattern.Tail` in both `RawOffset` chains. Catalog family
  `KyaPatternActorFamily.cpp` registers `CActorPattern` and ids 0/1/2/3/4;
  0x3F was added to `KyaActorFamilies::HasEmptyPayload` for id 0 and id 4 and
  to `UsesCatchByTrap`.
- Capability/applicability: every serialized field is editable except the
  add-on mode and the pattern-part shape flag; both are validated on write so
  the record cannot change size.
- Tests: `Kya.Bank.PatternFixtureRoundTrip` (84-byte class record with mode 0,
  56-byte behaviour 3, empty behaviour 4, byte-identical unedited export, seven
  edited fields through export/reimport, add-on-mode and pattern-shape
  rejection). Real-bank evidence: `Kya.Bank.SelectedActorRoundTrip` against
  `LEVEL_5/LEVEL.BNK` exports byte-identically and the post-change inventory
  reports `withRaw=0` for both class-0x3F actors (previously raw up to 84
  bytes).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. `CPatternPart` runtime point-pattern initialization and the
  add-on generator's generated money/bonus instances are out of scope.

## Findings: Ship

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorShip` (class 0x36; `CActor` -> `CObject`),
  `CBehaviourShip` and `CBehaviourShipFLY`. Bank actors are named
  `HUNTERSHIP_01`, which is why the cross-bank table lists 0x36 as HUNTERSHIP.
- Factory registration: class 0x36 (`Types.h` `SHIP`); `CActorShip::Create` at
  `ActorShip.cpp:12`.
- `BuildBehaviour` only adds id 2 (`SHIP_BEHAVIOUR_FLY`); both
  `CBehaviourShip::Create` and `CBehaviourShipFLY::Create` return immediately
  (`ActorShip.cpp:956`, `:978`). Ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | care boys | `S_ACTOR_STREAM_REF`: count u32 + count x 4-byte actor references |
  | fx ints | `CFxArrayManager`: count s32 + count x s32 |
  | actors | jaw care-boy, air-cannon and hunter references s32 |
  | tuning | fields 0x214/0x210/0x218/0x21C/0x220/0x224/0x230, `kimTouchRange`, field 0x22C f32 |
  | fields | fx `field_0x18` s32, fields 0x234/0x238 s32, field 0x1AC s32 |
  | block A | fields 0x18C[0..3] u32 |
  | block B | fields 0x19C/0x1A0/0x1A4/0x1A8 u32 |
  | vision | `CVision::Create`: four f32 + flags u32 |
  | notification | `S_NTF_SWITCH`: count u32 + count x 28-byte target + 32-byte camera |

  The real LEVEL_3 record is 244 bytes and is consumed exactly, with four
  care-boy refs, three fx ints and two switch targets.
- Editor model/reader/writer: `Public/Actors/KyaShip.h` and
  `Private/Actors/KyaShip.cpp`; model fields `ShipCareBoyActorRefs`,
  `ShipFxIntValues`, `ShipJawCareBoyRef`/`ShipAirCannonRef`/`ShipHunterRef`,
  `ShipField214` ... `ShipKimTouchRange`/`ShipField22C`, `ShipFxField18`,
  `ShipField234/238/1AC`, `ShipWords18C`, `ShipField19C/1A0/1A4/1A8`,
  `ShipVisionRange/HalfAngle/Field38/Field3C/Flags` and `ShipSwitch`. Factory
  class dispatch, exporter writer and `Ship.Tail` in both `RawOffset` chains.
  Catalog family `KyaShipActorFamily.cpp` registers `CActorShip` and ids 0/1/2;
  0x36 was added to `KyaActorFamilies::HasEmptyPayload` for id 0 and id 2.
- Capability/applicability: every serialized field is editable; the care-boy
  list, fx list, 4-word block and switch count are validated before writing so
  the record cannot change size.
- Tests: `Kya.Bank.ShipFixtureRoundTrip` (both lists, three actor refs, nine
  floats, both word blocks, vision and one switch target, empty behaviour 2,
  byte-identical unedited export, seven edited fields through export/reimport,
  care-boy-count and word-block rejection). Real-bank evidence: LEVEL_3 class
  inventory reports `withRaw=0` for 0x36 (previously raw up to 244 bytes) and a
  direct probe of `HUNTERSHIP_01` (actor index 483) shows four care-boy refs,
  three fx ints, vision range 34 and two switch targets with no raw tail.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The whole-bank `Kya.Bank.SelectedActorRoundTrip` on LEVEL_3 stops
  at actor index 301 (a pre-existing moving-platform record unrelated to this
  class; the ship is index 483, so it is never reached). The ship class itself
  is fully consumed by the inventory probe above; per-actor export of index 483
  cannot be exercised until that platform record is fixed.

## Findings: Bunch

Status: **class block and both Bunch behaviours typed; Wolfen behaviour ids
reuse the shared Wolfen codec**.

- Source classes: `CActorBunch` (class 0x4E; `CActorWolfen` -> `CActorFighter`),
  `CBehaviourBunch`, `CBehaviourBunchRepel`, `CBehaviourBunchFence`,
  `CBehaviourBunchFighterProjected` and `CBehaviourFighterBunch`.
- Factory registration: class 0x4E; `CActorBunch::Create` at
  `ActorBunch.cpp:11`.
- `BuildBehaviour` adds id 3 (fighter default), 4 (projected), 26
  (`BUNCH_BEHAVIOUR_REPEL`) and 27 (`BUNCH_BEHAVIOUR_FENCE`); every other id
  delegates to `CActorWolfen::BuildBehaviour`, so the Wolfen behaviour codec now
  accepts class 0x4E as well. `CBehaviourBunch::Create`,
  `CBehaviourBunchFighterProjected` and `CBehaviourFighterBunch` add no Create
  body of their own.
- Class payload layout: the shared `CActorWolfen` prefix (parsed only to locate
  its end, bytes untouched), then:

  | Section | Layout |
  | --- | --- |
  | field | field 0xFC8 f32 |
  | entries | count u32 + records |
  | entry | field 0 u32, has-behaviour u32; when nonzero a behaviour id u32; zone id u32; field 0xC u32 |
  | mesh | mesh component id s32, mesh index s32 |

  The real LEVEL_9 record is 3960 bytes and is consumed exactly after the
  Wolfen prefix.
- Behaviour 26 (`CBehaviourBunchRepel`, 80 + 4n bytes):

  | Section | Layout |
  | --- | --- |
  | waypoint | waypoint stream reference s32 |
  | shockwaves | count u32, then three f32 (field 0x14, shock-wave fields 0x10/0x14) |
  | indices | count u32 + count x u32 |
  | message | damage f32, field 0x30 f32, field 0x70 f32 |
  | vectors | vector 0x20 xyz f32, vector 0x30 xyz f32 |
  | mesh | component s32, mesh s32, type u32, reserved u32, default id u32 |

- Behaviour 27 (`CBehaviourBunchFence`):

  | Section | Layout |
  | --- | --- |
  | refs | actor stream reference s32, waypoint stream reference s32 |
  | waypoints | count u32 + count x s32 |
  | fences | count u32 + groups |
  | group | field 0 u32, field 4 u32, entry count u32, then entries |
  | entry | waypoint reference s32, field 4 s32 |
  | meshes | StaticMeshComponentAdvanced field 0xF0 (two s32), field 0x70 (two s32) |
  | fx | four u32 effect types |

- Editor model/reader/writer: `Public/Actors/KyaBunch.h` and
  `Private/Actors/KyaBunch.cpp`; model fields `BunchFieldFC8`,
  `BunchStreamEntries`, `BunchMeshComponentId/MeshIndex`,
  `BunchRepelWayPointRef`, `BunchRepelShockWaveCount`,
  `BunchRepelField14/ShockWaveField10/ShockWaveField14`, `BunchRepelIndices`,
  `BunchRepelDamage/MsgHitField30/MsgHitField70`,
  `BunchRepelVector20/30`, `BunchRepelMeshComponentId/MeshIndex/MeshType/
  Reserved/DefaultId`, `BunchFenceActorRef/WayPointRef`, `BunchFenceWayPoints`,
  `BunchFenceGroups`, the two mesh pairs and `BunchFenceFxTypes`. Factory
  class/behaviour dispatch, exporter class and behaviour writers,
  `Bunch.Tail` in both `RawOffset` chains. Catalog family
  `KyaBunchActorFamily.cpp` registers `CActorBunch` and the full id set;
  0x4E was added to `HasEmptyPayload` for id 0/3/4 and to `UsesCatchByTrap`.
- Capability/applicability: every typed field is editable; entry, index, fence
  and group counts are validated before writing so records cannot change size.
  The Wolfen prefix remains raw and is not editable through this module.
- Tests: `Kya.Bank.BunchFixtureRoundTrip` (Wolfen prefix, two stream entries
  with and without a behaviour id, empty ids 3/4, typed Wolfen track id 15,
  repel id 26 and fence id 27, byte-identical unedited export, nine edited
  fields through export/reimport, entry-count and repel-index-count rejection).
  Real-bank evidence: `Kya.Bank.SelectedActorRoundTrip` on `LEVEL_9/LEVEL.BNK`
  exports byte-identically and the class inventory reports `withRaw=0` for 0x4E
  (previously raw up to 3960 bytes).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Shock-wave and fence mesh runtime instances and the Wolfen prefix's
  fight-parameter records are not re-exposed here.

## Findings: Brazul

Status: **class block and both Brazul behaviours typed; Wolfen ids reuse the
shared Wolfen codec**.

- Source classes: `CActorBrazul` (class 0x4F; `CActorWolfen` -> `CActorFighter`),
  `CBehaviourBrazulA`, `CBehaviourBrazulB` and `CBehaviourFighterBrazul`.
- Factory registration: class 0x4F; `CActorBrazul::Create` at
  `ActorBrazul.cpp:8`.
- `BuildBehaviour` adds id 3 (fighter default), 26 (`CBehaviourBrazulA`) and 27
  (`CBehaviourBrazulB`); every other id delegates to `CActorWolfen::BuildBehaviour`,
  so the Wolfen behaviour codec accepts class 0x4F too. Ids 4/5/6 are the
  inherited fighter behaviours and `CBehaviourFighterBrazul` adds no Create body.
- Class payload layout (after the shared `CActorWolfen` prefix, whose bytes are
  untouched):

  | Section | Layout |
  | --- | --- |
  | fields | fields 0xFC0/0xFC4 f32 |
  | path | `CPathFollowReader` index s32 |
  | fields | field 0x1060 u32, reserved u32 |
  | material | material id s32 |
  | sparks | two spark parameters u32 |
  | fx | three `CFxHandleExt` ids u32, fx init u32, two more handle ids u32 |
  | meshes | two `StaticMeshComponentAdvanced` records (texture + mesh s32) |

  The real bank record is 2964 bytes and is consumed exactly after the Wolfen
  prefix.
- Behaviour 26 (`CBehaviourBrazulA`): count u32 + sub-objects. Each sub-object:
  field 0 s32, field 4 u32, field 8 f32, fields 0xC/0x10 u32, path count u32 +
  count x `CPathFollowReader` index s32.
- Behaviour 27 (`CBehaviourBrazulB`): waypoint stream reference s32 plus one
  `S_NTF_SWITCH` (count u32 + count x 28-byte target + 32-byte camera).
- Editor model/reader/writer: `Public/Actors/KyaBrazul.h` and
  `Private/Actors/KyaBrazul.cpp`; model fields `BrazulFieldFC0/FC4`,
  `BrazulPathIndex`, `BrazulField1060`, `BrazulReserved`, `BrazulMaterialId`,
  `BrazulSparkParamA/B`, `BrazulFxHandles`, `BrazulFxInit`,
  `BrazulMeshATexture/Mesh`, `BrazulMeshBTexture/Mesh`, and the behaviour fields
  `BrazulSubObjs`, `BrazulBWayPointRef`, `BrazulBSwitch`. Factory class and
  behaviour dispatch, exporter class/behaviour writers, `Brazul.Tail` in both
  `RawOffset` chains. Catalog family `KyaBrazulActorFamily.cpp` registers
  `CActorBrazul` and the full id set; 0x4F was added to `HasEmptyPayload` for
  id 0/3/4/5/6 and to `UsesCatchByTrap`.
- Capability/applicability: every typed field is editable; fx-handle, sub-object
  and path counts are validated before writing so records cannot change size.
  The Wolfen prefix remains raw and is not editable through this module.
- Tests: `Kya.Bank.BrazulFixtureRoundTrip` (Wolfen prefix, five fx handles with
  the init word between the groups, both meshes, empty fighter ids 3/4, typed
  Wolfen track id 15, Brazul A sub-object with two paths and Brazul B switch,
  byte-identical unedited export, nine edited fields through export/reimport,
  fx-count and path-count rejection). Real-bank evidence:
  `Kya.Bank.SelectedActorRoundTrip` exports `LEVEL_11`, `LEVEL_12` and
  `LEVEL_13` byte-identically and the class inventory reports `withRaw=0` for
  0x4F (previously raw up to 2964 bytes).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Spark/bone-physics runtime instances and the Wolfen fight-parameter
  records are not re-exposed here.

## Findings: Minigame family

Status: **all five minigame class ids and the shared behaviour ids typed**.

- Source classes: `CActorMiniGame` (class 0x44), `CActorMiniGameTimeAttack`
  (0x45), `CActorMiniGameBoxCounter` (0x46), `CActorMiniGameBoomy` (0x47),
  `CActorMiniGameDistance` (0x55), and the behaviours `CBehaviourMiniGame`
  (base), `CBehaviourMiniGameTraining`, `CBehaviourMiniGameBetting` and
  `CBehaviourMiniGameMulti`. The four variant actors reuse those behaviour
  classes (their `Training`/`Betting`/`Multi` wrappers add no `Create` body).
- Factory registration: 0x44/0x45/0x46/0x47/0x55; `CActorMiniGame::Create` at
  `ActorMiniGame.cpp:785`.
- `BuildBehaviour` maps id 2 (`training`), 3 (`betting`) and 4 (`multi`); ids
  0/1 are inherited. `CBehaviourMiniGameTraining` and `CBehaviourMiniGameMulti`
  have empty `Create`; `CBehaviourMiniGameBetting::Create` reads a bet count and
  count x {cost s32, reward s32} (`ActorMiniGame.cpp:1253`).
- Shared class layout:

  | Section | Layout |
  | --- | --- |
  | fields | field 0x160 u64, fields 0x168/0x16C u32 |
  | actors | `S_ACTOR_STREAM_REF`: count u32 + count x 4-byte actor refs |
  | floats | count s32 + count x f32 |
  | scores | count s32 + scores |
  | score | score f32, name string (`ByteCode::GetString`, 4-byte aligned) |
  | default score | score f32 + name string, present only when the scene is >= 2.21 |
  | refs | waypoint ref s32, field 0x190 s32, field 0x1A8 u64 |
  | notification | `S_NTF_SWITCH`: count u32 + count x 28-byte target + 32-byte camera |

  The scene-version default score changes the record size; the reader parses
  both shapes and selects the one that consumes the payload exactly, and the
  writer rejects a shape change.
- Variant tails:

  | Class | Tail |
  | --- | --- |
  | 0x44 base | none |
  | 0x45 TimeAttack | field 0x1E0 s32 |
  | 0x46 BoxCounter | time limit f32 |
  | 0x47 Boomy | target list, penalty-target list, time limit f32, max penalties s32, game flags u32, sequence list |
  | 0x55 Distance | two `CPathFollowReader` indices s32 |

  A Boomy target is an actor ref s32 plus one `S_NTF_SWITCH`; a Boomy sequence is
  a count s32 followed by count x target index s32.
- Editor model/reader/writer: `Public/Actors/KyaMiniGame.h` and
  `Private/Actors/KyaMiniGame.cpp`; model fields `MiniGameField160/168/16C`,
  `MiniGameActorRefs`, `MiniGameFloats`, `MiniGameScores`,
  `bMiniGameHasDefaultScore`, `MiniGameDefaultScore`, `MiniGameWayPointRef`,
  `MiniGameField190/1A8`, `MiniGameSwitch`,
  `MiniGameTimeAttackField1E0`, `MiniGameTimeLimit`, `MiniGamePathA/B`,
  `MiniGameBoomyTargets`, `MiniGameBoomyPenaltyTargets`,
  `MiniGameMaxPenalties`, `MiniGameGameFlags`, `MiniGameSequences`, and the
  behaviour field `MiniGameBets`. Factory class/behaviour dispatch, exporter
  class and behaviour writers, `MiniGame.Tail` in both `RawOffset` chains.
  Catalog families `KyaMiniGameActorFamily.cpp` register all five classes with
  ids 0/1/2/3/4 and ids 3 for the betting codec; all five ids were added to
  `HasEmptyPayload` for id 0/2/4.
- Capability/applicability: every typed field is editable. Score names are
  written inside their original aligned span (growth is rejected); score,
  float, actor, bet, target, penalty and sequence counts are validated before
  writing.
- Tests: `Kya.Bank.MiniGameTimeAttackFixtureRoundTrip`,
  `Kya.Bank.MiniGameBoomyFixtureRoundTrip` and
  `Kya.Bank.MiniGameVariantsFixtureRoundTrip` (0x44 base and 0x46 box counter).
  They cover score names, the default score, the switch, betting, the
  time-attack field, Boomy target/penalty/sequence lists, byte-identical
  unedited export, edited-field round trips and count rejections. 0x44 and 0x46
  do not appear in any shipped bank (all 16 banks were scanned); their evidence
  is the fixture round trip. Real-bank evidence:
  `Kya.Bank.SelectedActorRoundTrip` on `NATIV/LEVEL.BNK` exports
  byte-identically and the class inventory reports `withRaw=0` for 0x45 (8
  actors), 0x47 (2) and 0x55 (1); previously raw up to 3988 bytes.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. 0x44/0x46 have no shipped-bank instance; score-list runtime sorting
  and the minigame manager/organizer wiring are out of scope.

## Findings: Row-to-finding index

Every checklist row that predates the per-family findings above is linked
here to the finding or progress section that records its factory ids,
behaviour ids and payload disposition.

| Row | Covering finding | Disposition |
| --- | --- | --- |
| `CActorAcceleratos` | Implementation progress: Helper Sign, Wood Monster, Shocker Command, Acceleratos | class and behaviour codecs in `Actors/KyaAcceleratos.*`, `KyaHelperSign.*`, `KyaShockerCmd.*`, `KyaWoodMonster.*` |
| `CActorHelperSign` | Implementation progress: Helper Sign, Wood Monster, Shocker Command, Acceleratos | class and behaviour codecs in `Actors/KyaAcceleratos.*`, `KyaHelperSign.*`, `KyaShockerCmd.*`, `KyaWoodMonster.*` |
| `CActorShockerCmd` | Implementation progress: Helper Sign, Wood Monster, Shocker Command, Acceleratos | class and behaviour codecs in `Actors/KyaAcceleratos.*`, `KyaHelperSign.*`, `KyaShockerCmd.*`, `KyaWoodMonster.*` |
| `CActorWoodMonster` | Implementation progress: Helper Sign, Wood Monster, Shocker Command, Acceleratos | class and behaviour codecs in `Actors/KyaAcceleratos.*`, `KyaHelperSign.*`, `KyaShockerCmd.*`, `KyaWoodMonster.*` |
| `CBehaviourBlazer` | Implementation progress: Blazer | class codec in `Actors/KyaBlazer.*`; behaviour 3 is empty |
| `CBehaviourBlazerStand` | Implementation progress: Blazer | class codec in `Actors/KyaBlazer.*`; behaviour 3 is empty |
| `CActorDCA::CBhvDefault` | Findings: DCA | behaviour 2 empty; behaviours 3/5 typed in `Actors/KyaDca.*` |
| `CActorDCA::CBhvControlled` | Findings: DCA | behaviour 2 empty; behaviours 3/5 typed in `Actors/KyaDca.*` |
| `CActorDCA::CBhvTrajectory` | Findings: DCA | behaviour 2 empty; behaviours 3/5 typed in `Actors/KyaDca.*` |
| `CBehaviourFighterSlave` | Findings: Shared base behaviours | fighter ids 4/5/6 dispatch to empty Create bodies |
| `CBehaviourFighterRidden` | Findings: Shared base behaviours | fighter ids 4/5/6 dispatch to empty Create bodies |
| `CBehaviourFighterProjected` | Findings: Shared base behaviours | fighter ids 4/5/6 dispatch to empty Create bodies |
| `CActorFx::CBhvSingle` | Findings: FX | behaviour ids 2/3/4 typed in `Actors/KyaFx.*` |
| `CActorFx::CBhvPath` | Findings: FX | behaviour ids 2/3/4 typed in `Actors/KyaFx.*` |
| `CActorFx::CBhvUnknown` | Findings: FX | behaviour ids 2/3/4 typed in `Actors/KyaFx.*` |
| `CBehaviourHelperSignStand` | Implementation progress: Helper Sign, Wood Monster, Shocker Command, Acceleratos | HelperSign stand id is empty in `Actors/KyaHelperSign.*` |
| `CBehaviourHunter` | Implementation progress: Hunter | behaviour codecs in `Actors/KyaHunter.*`; the watch id is empty |
| `CBehaviourHunterWatch` | Implementation progress: Hunter | behaviour codecs in `Actors/KyaHunter.*`; the watch id is empty |
| `CBehaviourMiniGameBoomyTraining` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameBoomyBetting` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameBoomyMulti` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameBoxCounterTraining` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameBoxCounterBetting` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameBoxCounterMulti` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameDistanceTraining` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameDistanceBetting` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameDistanceMulti` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameTimeAttackTraining` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameTimeAttackBetting` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGameTimeAttackMulti` | Findings: Minigame family | variant wrappers reuse the base training (empty), betting (typed) and multi (empty) codecs |
| `CBehaviourMiniGamesManager` | Implementation progress: Minigames Manager and Organizer | class/behaviour codecs in `Actors/KyaMiniGames*.*`; stand ids empty |
| `CBehaviourMiniGamesManagerStand` | Implementation progress: Minigames Manager and Organizer | class/behaviour codecs in `Actors/KyaMiniGames*.*`; stand ids empty |
| `CBehaviourMiniGamesOrganizer` | Implementation progress: Minigames Manager and Organizer | class/behaviour codecs in `Actors/KyaMiniGames*.*`; stand ids empty |
| `CBehaviourMiniGamesOrganizerStand` | Implementation progress: Minigames Manager and Organizer | class/behaviour codecs in `Actors/KyaMiniGames*.*`; stand ids empty |
| `CBehaviourPlatformStand` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourPlatformTrajectory` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourPlatformSlab` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourPlatformDestroyed` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourWeighingMachine` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourWeighingMachineMaster` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourWeighingMachineSlave` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourSelector` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourSelectorMaster` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourSelectorSlave` | Findings: Moving platform | platform behaviour ids typed in `Actors/KyaMovingPlatform.*`; slab/destroyed and stand bodies are source-disposed |
| `CBehaviourTeleportRandom` | Findings: Teleporter | `CBehaviourTeleportRandom` typed in `Actors/KyaTeleporter.*` |
| `CBehaviourNativExorcisme` | Findings: Nativ | speak/live/seller/take-and-put/exorcism codecs in `Actors/KyaNativ.*` (see also the Nativ progress sections) |
| `CBehaviourNativSeller` | Findings: Nativ | speak/live/seller/take-and-put/exorcism codecs in `Actors/KyaNativ.*` (see also the Nativ progress sections) |
| `CBehaviourNativTakeAndPut` | Findings: Nativ | speak/live/seller/take-and-put/exorcism codecs in `Actors/KyaNativ.*` (see also the Nativ progress sections) |
| `CBehaviourNativSpeak` | Findings: Nativ | speak/live/seller/take-and-put/exorcism codecs in `Actors/KyaNativ.*` (see also the Nativ progress sections) |
| `CBehaviourNativLive` | Findings: Nativ | speak/live/seller/take-and-put/exorcism codecs in `Actors/KyaNativ.*` (see also the Nativ progress sections) |
| `CBehaviourProjectileStand` | Findings: Projectile | projectile behaviour ids typed in `Actors/KyaProjectile.*` |
| `CBehaviourProjectileLavaBall` | Findings: Projectile | projectile behaviour ids typed in `Actors/KyaProjectile.*` |
| `CBehaviourProjectileStraight` | Findings: Projectile | projectile behaviour ids typed in `Actors/KyaProjectile.*` |
| `CBehaviourProjectileExcuse` | Findings: Projectile | projectile behaviour ids typed in `Actors/KyaProjectile.*` |
| `CBehaviourProjectilePortable` | Findings: Projectile | projectile behaviour ids typed in `Actors/KyaProjectile.*` |
| `CBehaviourProjectileNew` | Findings: Projectile | projectile behaviour ids typed in `Actors/KyaProjectile.*` |
| `CBehaviourSwitchLever` | Findings: Switch | switch behaviour ids typed in `Actors/KyaSwitch.*` |
| `CBehaviourSwitchMagic` | Findings: Switch | switch behaviour ids typed in `Actors/KyaSwitch.*` |
| `CBehaviourSwitchWolfenCounter` | Findings: Switch | switch behaviour ids typed in `Actors/KyaSwitch.*` |
| `CBehaviourSwitchTarget` | Findings: Switch | switch behaviour ids typed in `Actors/KyaSwitch.*` |
| `CBehaviourSwitchTimed` | Findings: Switch | switch behaviour ids typed in `Actors/KyaSwitch.*` |
| `CBehaviourSwitchMultiCondition` | Findings: Switch | switch behaviour ids typed in `Actors/KyaSwitch.*` |
| `CBehaviourSwitchSequence` | Findings: Switch | switch behaviour ids typed in `Actors/KyaSwitch.*` |
| `CBehaviourLost` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourExorcism` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourEscape` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourWolfenWeapon` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourSnipe` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourTrackWeaponStand` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourTrackWeapon` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourTrackWeaponSnipe` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourAvoid` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourGuardArea` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourDCA` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourWolfenFighterSlave` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourWolfenFighterProjected` | Findings: Wolfen | Wolfen behaviour ids typed in `Actors/KyaWolfen.*` (fighter variants are empty) |
| `CBehaviourWoof` | Implementation progress: Woof | Woof behaviour codecs in `Actors/KyaWoof.*` |
| `CBehaviourWoofGuard` | Implementation progress: Woof | Woof behaviour codecs in `Actors/KyaWoof.*` |
| `CBehaviourWoofVerticalJump` | Implementation progress: Woof | Woof behaviour codecs in `Actors/KyaWoof.*` |

## Findings: Hedgehog

Status: **implemented in the editor and source-verified; no LEVEL_1 instance**.

- Source classes: `CActorHedgehog` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor` -> `CObject`) and `CBehaviourHedgehog` (`CBehaviour`) with
  `CBehaviourHedgehogWatchDog`, `CBehaviourHedgehogGuardArea`,
  `CBehaviourHedgehogWatchDogArmor` and `CBehaviourHedgehogGuardAreaArmor`.
- Factory registration: `src/b-witch/Types.h` defines `HEDGEHOG = 0x22`;
  `ActorFactory.cpp` allocates `CActorHedgehog` for that id.
- `CActorHedgehog::Create` (`src/b-witch/ActorHedgehog.cpp:9`) calls
  `CActorAutonomous::Create`, reads nine scalars, a `CAddOnGenerator`, then one
  trailing integer. The collision-flag update is runtime state, not another read.
  `CActorAutonomous::Create` (`ActorAutonomous.cpp:200`) consumes
  `CActorMovable`'s weightB plus life maximum, gravity scale and two
  dynamic-extension floats.
- Serialized class payload, relative to the first class-specific byte (verified
  against `CAddOnGenerator::Create`, `src/b-witch/ActorBonusServices.cpp:504`,
  and `CAddOnGenerator_SubObj::Create`, same file:479):

  | Offset | Size | Field |
  | --- | --- | --- |
  | 0x00 | 4 | `movable.weightB` f32 |
  | 0x04 | 4 | autonomous life maximum (serialized u32; stored as int64 so the full range round trips) |
  | 0x08 | 4 | gravity scale f32 |
  | 0x0C | 4 | `dynamicExt.field_0x4` f32 |
  | 0x10 | 4 | `dynamicExt.field_0x8` f32 |
  | 0x14 | 16 | walkSpeed, walkAcceleration, walkRotSpeed, runSpeed |
  | 0x24 | 4 | `field_0x374` u32 |
  | 0x28 | 4 | `field_0x378` u32 |
  | 0x2C | 12 | `field_0x368`, `field_0x36C`, `field_0x370` |
  | 0x38 | 20 | add-on flags, money type, money count, bonus count, field C |
  | 0x4C | 4 | add-on sub-object mode |
  | 0x50 | 20 | five sub-object floats when mode == 1 (the sixth runtime value is defaulted, not serialized) |
  | 0x50/0x64 | 4 | `field_0x380` u32 |

  Total class data is 0x54 (84) bytes for every mode other than 1, or 0x68
  (104) bytes when mode == 1. Bytes after the payload remain raw in `ClassData`.
- `BuildBehaviour` (`ActorHedgehog.cpp:64`): 3 = GuardArea, 4 = WatchDog,
  5 = GuardAreaArmor, 6 = WatchDogArmor; every other id delegates to
  `CActorAutonomous`. GuardArea and GuardAreaArmor `Create` each read one s32
  `CPathFollowReader::index` (`Actor.cpp:65`); WatchDog and WatchDogArmor `Create`
  are empty, and the shared `CBehaviourHedgehog::Create` is empty.
- Editor model/reader: typed fields (`bHasHedgehogData` and `Hedgehog*`) in
  `KyaLevelBank.h`; `KyaReadHedgehogOffsets` in `KyaHedgehog.h/.cpp`; the factory
  branch (class 0x22) and per-behaviour branch in `KyaLevelBankFactory.cpp`.
  The decoded prefix is removed from `ClassData`, which keeps the undecoded tail.
- Export/writer: `KyaLevelBankExporter.cpp` writes every typed field, validates
  the sub-object mode against the parsed layout, preserves the raw tail, and
  writes the GuardArea/GuardAreaArmor path index. The field allowlist in
  `KyaExportedActorFields.h` makes the typed rows exportable; presence flags stay
  inspect-only. There are no variable-length arrays, so there is no count editing
  limit beyond the fixed class-data size. Changing the add-on sub-object mode is
  rejected because it would change the serialized size.
- Inspector: Details groups are `Hedgehog` and `Hedgehog behaviour`; empty groups
  are hidden by `EditConditionHides(true)` in `KyaActorDetailsCustomization.cpp`.
  `HedgehogPathFollowIndex` is an editable nested behaviour field (shown only
  for behaviour 3/5, `-1` means none); it is not in the top-level reference table
  because that table addresses placement fields only. WatchDog/WatchDogArmor
  show no payload fields.
  Edits use the standard placement patch path, so refresh/undo follow the shared
  edit service.
- Family module: `Source/KyaLevel/Private/Actors/KyaHedgehogActorFamily.cpp` owns
  the class id, display name, notes and behaviour names;
  `Source/KyaLevel/Public/Actors/KyaHedgehog.h` owns the byte codec. The catalog
  reads both through `KyaActorFamily.h` / `KyaActorFamilyRegistry.cpp` rather
  than a private table.
- Tests: `Kya.Bank.HedgehogFixtureRoundTrip` imports a source-derived 104-byte
  fixture with behaviours 3 (path index) and 4 (empty), verifies every typed
  field, exports byte-identically, edits four fields, reimports exact values,
  and rejects a sub-object mode change. `Kya.Bank.LevelOneRoundTrip` covers the
  84/104-byte offset fixtures, truncation rejection and both empty/typed
  behaviour shapes. `Kya.Editor.ActorCatalog` covers class/behaviour labels, the
  registry entry and unknown inherited ids.
- Real-bank evidence: `KyaLevelEditor/Tools/inventory_actor_classes.py` reports
  616 actors across 36 classes for LEVEL_1 and **no class 0x22**, so there is no
  real Hedgehog instance to inspect. The fixture is the only round-trip evidence.
- Remaining exclusions: Ghidra is not available in this environment (no MCP
  resources), so original PS2 function addresses were not recorded and the
  layout is source-derived and Codex-reviewed, not binary-verified. Behaviour
  4/6 emptiness is source-verified and fixture-covered. A future LEVEL containing
  Hedgehog should be added to the real-bank gate. Nested behaviour reference
  pickers are still top-level-only, so the path index has a numeric field but no
  picker; the editor's reference table remains placement-scoped.

## Findings: Clusteriser

Status: **behaviour 2 zone codec extracted; class has no payload**.

- Source classes: `CActorClusteriser` (`CActor` -> `CObject`) and
  `CBehaviourClusteriserZones`.
- Factory registration: class 0x18 (`Types.h` `CLUSTERISER`).
- `CActorClusteriser::Create` has no class-specific payload beyond
  `CActor::Create`. `BuildBehaviour` only adds id 2
  (`behaviourClusteriserZones`); ids 0/1 are inherited.
- Behaviour 2 payload (`CStreamClusterZoneInternal`): count s32 + count x zone.

  | Section | Layout |
  | --- | --- |
  | header | flags u32, zone ref s32 |
  | actors | count u32 + count x 4-byte actor refs |
  | other actors | count u32 + count x 4-byte refs |
  | cameras | count u32 + count x 4-byte refs |
  | lights | count u32 + count x 4-byte refs |
  | hashes | count u32 + count x 8-byte hashes |
  | tail | other zone ref s32 |

- Editor model/reader/writer: `Public/Actors/KyaClusteriser.h` and
  `Private/Actors/KyaClusteriser.cpp` own the zone codec. `Flags` is read as an
  unsigned 32-bit value into int64 so `0x80000000` no longer sign-extends and
  fails validation. The factory and exporter dispatch to the module; the old
  `FKyaClusterZoneOffsets`/`KyaReadClusterZoneOffsets` and the inline writer were
  removed from `KyaCameraStream.h`/the exporter.
- Capability/applicability: every typed field is exported; the zone count and all
  five per-zone list counts are validated before writing. Unknown tails are
  preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Cluster zones`; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.ClusteriserFixtureRoundTrip` covers a populated zone (flags,
  zone ref, one actor, one camera, one hash, other zone ref), byte-identical
  unedited export, six edited fields through export/reimport and actor-count
  rejection. `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip`
  cover the real clusteriser; `Kya.Editor.ActorCatalog`/`Kya.Editor.MilestoneCGate`
  check its labels and exported fields.
- Real-bank evidence (LEVEL_1, 1 Clusteriser): behaviour 2 with a 172-byte
  payload split across multiple zones.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Actor/camera/light ids are scalar references without per-entry
  pickers.

## Findings: Projectile

Status: **class and all projectile-owned behaviour codecs extracted; real-bank
IDs reconciled**.

- Source classes: `CActorProjectile` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor` -> `CObject`) and `CBehaviourProjectile` with `Stand`, `Straight`,
  `Portable`, `Excuse`, `LavaBall`, `New`; id 9 uses the shared
  `CBehaviourInventory`, id 12 uses `CBehaviourInactive`.
- Factory registration: class 0x1c (`Types.h` `PROJECTILE`).
- `CActorProjectile::Create` (`src/b-witch/ActorProjectile.cpp:27`) calls
  `CActorAutonomous::Create`, copies one 0x48-byte `ProjectileSubObj` directly
  from the stream, then calls `PatternPart::Create`. The autonomous prefix is
  weightB, life maximum (u32), gravity scale and two dynamic-extension floats.
- Class payload offsets (relative to the first class-specific byte):

  | Offset | Size | Field |
  | --- | --- | --- |
  | 0x00 | 4 | `movable.weightB` |
  | 0x04 | 16 | life maximum u32, gravity scale, dynamic ext 4/8 |
  | 0x14 | 72 | `ProjectileSubObj` (time/range/duration/flags, damage/hit/warn radii, fields 1c-28, material, fields 30-3c, fields 40/44) |
  | 0x5C | 40 or 44 | `PatternPart`; the optional field 0x24 makes the payload 136 instead of 132 bytes |

  `ProjectileLifeMax` is stored as int64 in the editor model so the full u32
  round-trips exactly. Bytes after the payload remain raw in `ClassData`.
- `BuildBehaviour` (`ActorProjectile.cpp:183`): 3 Stand, 4 Straight, 5 Portable,
  9 Inventory, 10 LavaBall, 11 Excuse, 12 Inactive, 13 New; 0 Stand, 1 Cinematic
  and 2 CatchByTrap are inherited from `CActor`/`CActorAutonomous`. Payloads:

  | Id | Source | Size | Fields |
  | --- | --- | --- | --- |
  | 3 | ProjectileStand | 12 | behaviourId, field C, material |
  | 4 | ProjectileStraight | 20 | behaviourId, field C, speed, acceleration, field 18 |
  | 5 | ProjectilePortable | 12 | behaviourId, field C, field 10 |
  | 9 | Inventory | 8/44/56 | CInventoryInfo present flag, two u64s, purchase/field/money values, optional object block, initial behaviour |
  | 10 | ProjectileLavaBall | 40 or 44 | behaviourId, field C, fields 10/14/30, optional field 18, path index, fields 2c/34/38/3c |
  | 11 | ProjectileExcuse | 12 | behaviourId, field C, actor reference |
  | 12 | CBehaviourInactive | 12 | activate message, flags, active behaviour |
  | 13 | ProjectileNew | 4 | behaviourId only (base `CBehaviourProjectile::Create`) |

- Editor model/reader/writer: `Public/Actors/KyaProjectile.h` and
  `Private/Actors/KyaProjectile.cpp` own the class, behaviour and inventory
  codecs; the factory and exporter only dispatch to them. The class reader
  replaces `ClassData` with the undecoded tail; the writers overwrite typed
  fields and leave unknown bytes intact.
- Capability/applicability: every typed field is written and therefore
  exportable; `bHasLavaBallField18` is layout presence and inspect-only, and the
  field 18 row is hidden when absent. Changing pattern field 0x24 presence or a
  sub-object/version-gated layout is rejected. Inventory object-block presence
  comes from the payload and is not user-toggleable.
- Inspector: Details groups are `Projectile` and `Projectile behaviour`; empty
  groups hide through `EditConditionHides(true)`, and edits use the shared
  placement patch path (refresh/undo included).
- Tests: `Kya.Bank.ProjectileFixtureRoundTrip` builds a 132-byte class payload
  with behaviours 3/4/5/9/10/11/12/13, verifies every typed value, exact u32
  life maximum above 2^24, byte-identical unedited export, eight edited fields
  through export/reimport, and rejection of pattern-field/life-layout changes.
  `Kya.Bank.LevelOneRoundTrip` covers the 132/136 class offsets and the 40/44
  LavaBall, 12 Inactive and 4 New layouts. `Kya.Editor.MilestoneCGate` audits the
  exported projectile class/behaviour properties on the real 136-byte bank
  layout.
- Real-bank evidence (LEVEL_1, 26 projectiles): 8 x id 1 (size 4, generic
  cinematic), 25 x id 3, 9 x id 4, 1 x id 5, 2 x id 9 (size 44) and 1 x id 11.
  Ids 10, 12 and 13 do not occur in LEVEL_1 and are fixture-covered only.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The LavaBall field-18 version gate is inferred from payload size
  because `CScene::field_0x1c` is not serialized in the bank. The class reader
  selects the 132/136 pattern layout from the exact payload size; a malformed
  132-byte payload followed by an unrelated 4-byte tail would be read as the
  newer layout. That case is not a valid record (nothing consumes a post-pattern
  tail), and all 26 LEVEL_1 projectiles use the 136-byte modern layout, so
  observed data is unaffected. Inherited id 2 (CatchByTrap) still
  has only the Micken/Nativ generic decoder; `CBehaviourInventory` is shared with
  Fruit and that consumer is tracked separately.

## Findings: Moving platform

Status: **class and all platform behaviour codecs extracted; real-bank IDs
reconciled**.

- Source classes: `CActorMovingPlatform` (`CActorMovable` -> `CActor` ->
  `CObject`); behaviours `CBehaviourPlatform` (empty base `Create`),
  `PlatformStand`, `TeleportRandom`, `PlatformTrajectory`, `PlatformSlab`,
  `PlatformDestroyed`, `WeighingMachine`, `WeighingMachineMaster`,
  `WeighingMachineSlave`, `Selector`, `SelectorMaster`, `SelectorSlave`.
- Factory registration: class 0x0d (`Types.h` `MOVING_PLATFORM`).
- `CActorMovingPlatform::Create` (`src/b-witch/ActorMovingPlatform.cpp:87`) calls
  `CActorMovable::Create` (weightB), then streams in order: carried actors,
  zones, no-friction zones, camera records, the 0x4c-byte
  `CActorMovingPlatform_SubObj`, and two platform-sound streams. Collision-flag
  updates after the properties block are runtime state.
- Class payload layout:

  | Section | Layout |
  | --- | --- |
  | carried actors | count u32 + count x 8-byte (`actorRef`, `carryMethod`) |
  | zones | count u32 + count x 4-byte zone ref |
  | no-friction zones | count u32 + per zone: zone ref s32, actor-stream count u32, count x 4-byte actor refs |
  | camera records | count u32 + count x 72-byte entries (`S_BRIDGE_CAMERA_STREAM_ENTRY`) |
  | properties | 0x4c bytes (`CActorMovingPlatform_SubObj`) |
  | sounds A/B | each count u32 + count x 16-byte `PLATFORM_SOUND_STREAM_ENTRY` |

  `KyaReadMovingPlatformActor` preserves any bytes after the final sound stream
  in `ClassData`; valid records consume the whole class payload.
- `BuildBehaviour` (`ActorMovingPlatform.cpp:417`): 0/1 are the generic
  stand/cinematic behaviours; 2 Trajectory, 3 Slab, 4 WeighingMachineSlave,
  5 WeighingMachineMaster, 6 Destroyed, 7 PlatformStand, 8 SelectorMaster,
  9 SelectorSlave, 10 TeleportRandom. Behaviour payloads:

  | Id | Source | Size | Layout |
  | --- | --- | --- | --- |
  | 2 | PlatformTrajectory | 24 | absolute path reader (path index, timing, field 4, total time), segment start, loop delay |
  | 3 | PlatformSlab | 56+ | on/off switch targets/camera, four floats, sound ref; bank sizes 56 and 84 |
  | 4 | WeighingMachineSlave | 8 | path index, timing |
  | 5 | WeighingMachineMaster | 56+ | path/timing, stream actor, two floats, analog switch list, camera; bank size 96 |
  | 6 | PlatformDestroyed | 24 | absolute path reader, field 40, alternate model id |
  | 7 | PlatformStand | 8 | sound ref, field 0x0c |
  | 8 | SelectorMaster | 64+ | selector base (24), actor stream count+refs, switch-ex count+16-byte entries, camera; bank size 100 |
  | 9 | SelectorSlave | 24 | selector base (field 18, material, three floats, sound ref) |
  | 10 | TeleportRandom | 4 | one path-follow index |

- Editor model/reader/writer: `Public/Actors/KyaMovingPlatform.h` and
  `Private/Actors/KyaMovingPlatform.cpp` own the class, prefix/details, camera,
  behaviour and switch codecs. The factory and exporter now dispatch to them;
  the old structs and readers were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; array counts are
  layout-controlled and validated (carried actors, zones, no-friction zones,
  cameras, sounds, analog/selector lists). No-friction actor-list counts cannot
  be resized. Camera/event and property records stay fixed-size; unknown class
  tails are preserved. Edits use the shared placement patch path
  (refresh/undo included).
- Inspector: Details groups are `Moving platform` / `Platform behaviour`;
  empty groups hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.MovingPlatformFixtureRoundTrip` covers the class payload and
  id 10, byte-identical unedited export, six edited fields through
  export/reimport and count-change rejection. `Kya.Bank.SmallBankRoundTrip`
  remains the class-prefix/details round trip. `Kya.Bank.LevelOneRoundTrip`
  covers real ids 2-9, including both slab sizes and the empty selector-master
  layout, and `Kya.Editor.MilestoneCGate` audits the exported fields on the real
  bank.
- Real-bank evidence (LEVEL_1, 166 platforms): id 0 x49, id 1 x47, id 2 x84,
  id 3 x6 (four 56-byte, two 84-byte), id 4 x1, id 5 x1 (96-byte), id 6 x1,
  id 7 x44, id 8 x4 (100-byte), id 9 x12. Id 10 does not occur and is
  fixture-covered.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Nested actor/zone reference pickers are still top-level-only.
  Weighing-machine master/slave and selector-master instances each occur only
  once (or a few times) in LEVEL_1, so their nested lists have limited
  real-bank variety.

## Findings: Switch

Status: **class and all switch behaviour codecs extracted; real-bank IDs
reconciled**.

- Source classes: `CActorSwitch` (`CActor` -> `CObject`) and
  `CBehaviourSwitch` with `Lever`, `Magic`, `WolfenCounter`,
  `MultiCondition`, `Target`, `Timed`, `Sequence`.
- Factory registration: class 0x11 (`Types.h` `SWITCH`).
- `CActorSwitch::Create` (`src/b-witch/ActorSwitch.cpp:24`) calls `CActor::Create`
  then `targetSwitch.Create` (`S_NTF_TARGET_STREAM_REF`): target count, count x
  28-byte target records, then a 32-byte camera event. No other serialized class
  payload exists.
- `BuildBehaviour` (`ActorSwitch.cpp:130`): 2 Lever, 3 Magic, 4 WolfenCounter,
  5 MultiCondition, 6 Target, 7 Sequence, 8 Timed; ids 0/1 are the inherited
  stand/cinematic behaviours. Payloads:

  | Id | Source | Size | Layout |
  | --- | --- | --- | --- |
  | 2 | SwitchLever | 8 | two sound references |
  | 3 | SwitchMagic | 76 | two floats, digits count, field 14, field 4C, six ids/floats, open texture/mesh, 32-byte camera |
  | 4 | SwitchWolfenCounter | 20 | required count, trigger radius, zone/actor refs, material id |
  | 5 | SwitchMultiCondition | 12 | field 14, field 08, field 0C |
  | 6 | SwitchTarget | 12 | displacement, spring strength, damping |
  | 7 | SwitchSequence | 12+4n | field 08, field 0C, count, count x 4-byte values |
  | 8 | SwitchTimed | 16+sum(40+28n) | base delay, entry count, per entry S_NTF_SWITCH (count + targets + camera) and delay, playback mode, flags |

- Editor model/reader/writer: `Public/Actors/KyaSwitch.h` and
  `Private/Actors/KyaSwitch.cpp` own the class and behaviour codecs; the factory
  and exporter dispatch to them. The old switch structs/readers were removed
  from `KyaCameraStream.h`; the shared `KyaReadSwitchValue` helper remains for
  other families' embedded switch records.
- Capability/applicability: every typed field is exported; target/sequence/timed
  list counts are layout-controlled and validated before writing. Unknown class
  tails are preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Switch behaviour`; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.SwitchFixtureRoundTrip` covers the class payload and ids 4,
  7 and 8 (the three shapes absent from LEVEL_1), byte-identical unedited export,
  six edited fields through export/reimport and sequence-count rejection.
  `Kya.Bank.LevelOneRoundTrip` covers real ids 2/3/5/6 and the shared switch
  camera; `Kya.Editor.MilestoneCGate` audits exported fields on the real bank.
- Real-bank evidence (LEVEL_1, 17 switches): id 2 x5 (8 bytes), id 3 x4 (76
  bytes), id 5 x7 (12 bytes), id 6 x1 (12 bytes). Ids 4, 7 and 8 do not occur
  and are fixture-covered.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Wolfen-counter zone/actor references and Timed target lists are
  nested array records without reference pickers; the magic camera is numeric
  only. Real-bank variety for ids 4/7/8 is absent.

## Findings: Wind

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorWind` (`CActor` -> `CObject`) and `CBehaviourWind`
  (`CBehaviour`, empty `Create`).
- Factory registration: class 0x12 (`Types.h` `WIND`).
- `CActorWind::Create` (`src/b-witch/ActorWind.cpp:39`) calls `CActor::Create`
  then reads the tuning block, sound/zone/actor streams, material/colour/waypoint
  references, the sector hierarchy, version-gated switch records, five floats and
  the waypoint tail. Behaviour 2 has no serialized payload; ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Offset | Size | Field |
  | --- | --- | --- |
  | 0x00 | 4 | `field_0x160` flags u32 |
  | 0x04 | 40 | maxWind, fields 168/16C/174/180/184/188/18C/178/17C |
  | 0x2C | 4 | `field_0x190` u32 |
  | 0x30 | 4 | sound id |
  | 0x34 | 4 | activation zone s32 |
  | 0x38 | 4+4n | actor stream count + actor refs |
  | then | 24 | material id, zone 1C4, colour 1C8, waypoint 1CC, 64-bit hierarchy hash |
  | modern | 3 x (4+28n+32) | three S_NTF_SWITCH records (level version >= 2.2) |
  | then | 20 | fields 194/198/19C/1A0/1A4 |
  | then | 12 | waypoint ref, fields 1AC/1B0 |

  The modern switch block adds three switch records; the reader selects it when
  the remaining size can hold three empty switches (140 bytes), matching the
  level-version gate. Bytes after the payload remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaWind.h` and
  `Private/Actors/KyaWind.cpp` own the class codec and reuse
  `KyaReadSwitchValue`/`KyaWriteSwitchValue` for the embedded switch records.
  The factory and exporter only dispatch to the module.
- Capability/applicability: every typed field is exported; the actor-reference
  count and each switch target count are validated before writing. Waypoint/zone
  references are exposed as scalar pickers. Unknown tails and absent legacy
  switches are preserved.
- Inspector: Details uses the `Wind` category; switch records appear under the
  existing switch fields, and edits use the shared placement patch path
  (refresh/undo).
- Tests: `Kya.Bank.WindFixtureRoundTrip` covers the modern 2.2 layout with three
  switch records and the empty id 2 behaviour, byte-identical unedited export,
  six edited fields through export/reimport and actor-count rejection.
  `Kya.Bank.LevelOneRoundTrip` covers the real bank layout;
  `Kya.Editor.MilestoneCGate` audits exported fields on the real bank.
- Real-bank evidence (LEVEL_1, 25 Winds): all 25 carry behaviour id 2 with a
  zero-byte payload; the class payload parses with no raw tail.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The 2.2 switch gate is inferred from payload size because the scene
  version is not serialized in the bank. Nested switch target lists have no
  per-entry reference pickers.

## Findings: Trap

Status: **class and both trap behaviour codecs extracted; real-bank IDs
reconciled**.

- Source classes: `CActorTrap` (`CActorMovable` -> `CActor` -> `CObject`) and
  `CBehaviourTrap`, `CBehaviourTrapStand`, `CBehaviourTrapStand_ActorTrap`;
  id 3 uses the shared `CBehaviourInactive`.
- Factory registration: class 0x2c (`Types.h` `TRAP`).
- `CActorTrap::Create` (`src/b-witch/ActorTrap.cpp:14`) calls
  `CActorMovable::Create` (weightB) then
  `CBehaviourTrapStand_ActorTrap::CreateForTrap` (`ActorTrap.cpp:628`):
  actor stream, four tuning words, optional field 0x24 (level version >= 2.24),
  actor reference, speed, acceleration, path-follow index, field 0x4C, required
  escape count and a trap stream of 60-byte switch/camera entries.
- Class payload layout:

  | Section | Layout |
  | --- | --- |
  | weightB | f32 |
  | actor stream | count u32 + count x 4-byte actor refs |
  | tuning | fields 14/18/1C/20 (four u32) |
  | optional | field 24 s32 on newer levels |
  | movement | actor ref, speed f32, acceleration f32, path-follow s32 |
  | escape | field 4C f32, required escape attempts u32 |
  | trap stream | count u32 + count x 60-byte entry (28-byte target + 32-byte camera) |

  Bytes after the payload remain raw in `ClassData`.
- `BuildBehaviour` (`ActorTrap.cpp:46`): 2 TrapStand, 3 Inactive; ids 0/1 are
  inherited. Both behaviour payloads are 12 bytes:
  - id 2: flags, default behaviour id, catch-effect id.
  - id 3: activate message, flags, active behaviour id.
- Editor model/reader/writer: `Public/Actors/KyaTrap.h` and
  `Private/Actors/KyaTrap.cpp` own the class, trap-stream and behaviour codecs;
  the factory and exporter dispatch to them. The old structs/readers were
  removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; actor/trap-stream
  counts and the optional field-24 presence are validated before writing.
  Unknown tails are preserved; edits use the shared placement patch path.
- Inspector: Details groups are `Trap` and `Trap behaviour`; empty groups hide
  through `EditConditionHides(true)`.
- Tests: `Kya.Bank.TrapFixtureRoundTrip` covers the modern class payload, one
  trap-stream entry and both behaviours 2/3, byte-identical unedited export,
  seven edited fields through export/reimport and trap-stream-count rejection.
  `Kya.Bank.LevelOneRoundTrip` covers the real bank, including the 52/56-byte
  legacy/modern layouts and a 112-byte legacy payload with a stream entry;
  `Kya.Editor.MilestoneCGate` audits exported fields on the real bank.
- Real-bank evidence (LEVEL_1, 24 traps): 24 x id 2 (12 bytes) and 1 x id 3
  (12 bytes); class payloads parse with no raw tail.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The field-24 gate is selected by exact payload consumption because
  the scene version is not serialized in the bank. Trap stream target/camera
  records have no per-entry reference pickers.

## Findings: Weapon

Status: **class and all weapon behaviour codecs extracted; real-bank IDs
reconciled**.

- Source classes: `CActorWeapon` (`CActorMovable` -> `CActor` -> `CObject`) and
  `CBehaviourWeapon`, `CBehaviourWeaponBurst`, `CBehaviourWeaponBullets`,
  `CBehaviourWeaponPistol`, `CBehaviourWeaponSniper`, `CBehaviourWeaponRpg`.
- Factory registration: class 0x20 (`Types.h` `WEAPON`).
- `CActorWeapon::Create` (`src/b-witch/ActorWeapon.cpp:10`) calls
  `CActorMovable::Create` (weightB) then reads two floats. The whole class
  payload is 12 bytes.
- `BuildBehaviour` (`ActorWeapon.cpp:55`): 2 Default (empty `Create`), 3 Pistol,
  4 Sniper, 6 RPG; id 5 is an implementation guard and ids 0/1 are inherited.
  Payloads:

  | Id | Source | Size | Layout |
  | --- | --- | --- | --- |
  | 2 | Weapon default | 0 | no serialized payload |
  | 3 | WeaponPistol | 52 | two 16-byte burst entries, field 40/44, sound ref, pistol material/field |
  | 4 | WeaponSniper | 56 | burst block, field 40/44, sound ref, texture/mesh/material ids |
  | 6 | WeaponRpg | 36+4n | burst block, actor stream count + count x 4-byte refs |

- Editor model/reader/writer: `Public/Actors/KyaWeapon.h` and
  `Private/Actors/KyaWeapon.cpp` own the class and behaviour codecs; the factory
  and exporter dispatch to them. The old structs/readers were removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the RPG actor count
  is validated before writing. Unknown class tails are preserved; edits use the
  shared placement patch path (refresh/undo).
- Inspector: Details groups are `Weapon` and `Weapon behaviour`; empty groups
  hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.WeaponFixtureRoundTrip` covers the class payload and
  behaviours 3/4/6, byte-identical unedited export, five edited fields through
  export/reimport and RPG-count rejection. `Kya.Bank.LevelOneRoundTrip` covers
  the real bank; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 18 weapons): id 0 x15 (empty), id 2 x15 (empty),
  id 3 x15 (52 bytes), id 4 x2 (56 bytes), id 6 x1 (52 bytes, four RPG refs).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Behaviour 5 is an implementation guard in source and has no codec.
  RPG actor refs are a nested array without a per-entry reference picker.

## Findings: Araignos

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorAraignos` (`CActorMovable` -> `CActor` -> `CObject`)
  and the nested `CActorAraignos::CBhvDefault` (`CBehaviour`, empty `Create`).
- Factory registration: class 0x21 (`Types.h` `ARAIGNOS`).
- `CActorAraignos::Create` (`src/b-witch/ActorAraignos.cpp:9`) calls
  `CActorMovable::Create` (weightB), reads a path-follow index, three floats,
  three sound ids and a `CAddOnGenerator`. Behaviour 2 has no serialized
  payload; ids 0/1 are inherited.
- Class payload layout:

  | Offset | Size | Field |
  | --- | --- | --- |
  | 0x00 | 4 | movable weightB |
  | 0x04 | 4 | path-follow index |
  | 0x08 | 12 | fields 1F8/1FC/200 (third is damage) |
  | 0x14 | 12 | sound ids 240/238/23C |
  | 0x20 | 20 | add-on flags, money type/count, bonus count, field C |
  | 0x34 | 4 | add-on sub-object mode |
  | 0x38 | 20 | five sub-object floats when mode == 1 |

  Total class data is 56 bytes for other modes or 76 bytes for mode 1. Bytes
  after the payload remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaAraignos.h` and
  `Private/Actors/KyaAraignos.cpp` own the class codec and add-on mapping; the
  factory and exporter dispatch to them. The old structs/readers were removed
  from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; changing the add-on
  sub-object mode is rejected because it changes the serialized size. Unknown
  tails are preserved; edits use the shared placement patch path.
- Inspector: Details uses the `Araignos` category; edits use the shared
  placement patch path (refresh/undo).
- Tests: `Kya.Bank.AraignosFixtureRoundTrip` covers the 76-byte mode-1 payload,
  the empty id 2 behaviour, byte-identical unedited export, six edited fields
  through export/reimport and sub-object-mode rejection.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Editor.MilestoneCGate` cover the real
  bank.
- Real-bank evidence (LEVEL_1, 39 Araignos): all carry behaviour id 2 with a
  zero-byte payload and fully consume the class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Path-follow and sound ids are scalar fields; no per-field picker is
  registered for the path-follow index.

## Findings: Bonus

Status: **class and turn/path/flock/add-on codecs extracted; real-bank IDs
reconciled**.

- Source classes: `CActorBonus` (`CActorMovable` -> `CActor` -> `CObject`) and
  `CBehaviourBonusBase`, `CBehaviourBonusAlone`, `CBehaviourBonusTurn`,
  `CBehaviourBonusPath`, `CBehaviourBonusFlock`, `CBehaviourBonusAddOn`.
- Factory registration: class 0x0e (`Types.h` `BONUS`).
- `CActorBonus::Create` (`src/b-witch/ActorBonus.cpp:23`) calls
  `CActorMovable::Create` (weightB), then reads a sound reference and
  flare/animation material ids. The class payload is 16 bytes.
- `BuildBehaviour` (`ActorBonus.cpp:116`): id 2 returns null, 3 Turn, 4 Path,
  5 Flock, 6 AddOn; ids 0/1 are inherited. Payloads:

  | Id | Source | Size | Layout |
  | --- | --- | --- | --- |
  | 2 | (none) | 0 | no behaviour object is created |
  | 3 | BonusTurn | 32 | alone prefix (field 14, fields 1B8/1B4, material, field 1B0, FxTail count, G2D index) + turn value |
  | 4 | BonusPath | 52 | alone prefix + path-follow index, four floats, one word |
  | 5 | BonusFlock | 4 | path-follow reference |
  | 6 | BonusAddOn | 4 | add-on field 18 |

- Editor model/reader/writer: `Public/Actors/KyaBonus.h` and
  `Private/Actors/KyaBonus.cpp` own the class and behaviour codecs; the factory
  and exporter dispatch to them. The old structs/readers were removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; fixed behaviour
  lengths are validated before writing. Unknown class tails are preserved;
  edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Bonus` and `Bonus behaviour`; the Path-only
  fields are hidden for other ids and empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.BonusFixtureRoundTrip` covers the class payload and ids
  3/4/5/6 (including Path, absent from LEVEL_1), byte-identical unedited export
  and seven edited fields through export/reimport. `Kya.Bank.LevelOneRoundTrip`
  covers real ids 3/5/6; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 16 bonuses): id 3 x1 (32 bytes), id 5 x15
  (4 bytes), id 6 x1 (4 bytes). Id 4 is fixture-covered; id 2 has no object.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Flock/Path references are scalar ids without per-field pickers.
  Behaviour 2 intentionally has no serialized payload or source object.

## Findings: Teleporter

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorTeleporter` (`CActor` -> `CObject`) and
  `CBehaviourTeleporterDefault` (`CBehaviour`, empty `Create`).
- Factory registration: class 0x24 (`Types.h` `TELEPORTER`).
- `CActorTeleporter::Create` (`src/b-witch/ActorTeleporter.cpp:14`) calls
  `CActor::Create`, then reads a subsector material id, a 64-bit hash, a word,
  `S_DESTINATION_LIST`, three `S_NTF_TARGET_STREAM_REF` switch records, nine
  zone/material/cinematic words, an `ACTOR_WAYPOINT_LIST` and a
  `ConditionedOperationArray`. Behaviour 2 has no payload; ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | header | subsector material s32, hash u64, field 16C u32 |
  | destinations | count u32 + count x 12-byte (level id, elevator id, field 8) |
  | switches | three `S_NTF_TARGET_STREAM_REF`: count u32 + count x 28-byte target + 32-byte camera |
  | zone/material | field 188, field 18C, material 1, cinematic id, material 2, fields 1D4/1D8, material 3, field 1E0 (nine words) |
  | waypoints | count u32 + count x 8-byte (actor ref, waypoint ref) |
  | conditioned ops | block count u32, then per block: op count + 12-byte operations, condition count + 16-byte conditions |

  The conditioned-operation block stores operations before conditions, matching
  `ScenaricCondition.cpp ConditionedOperationArray::Create`. Bytes after the
  payload remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaTeleporter.h` and
  `Private/Actors/KyaTeleporter.cpp` own the class codec and reuse
  `KyaReadSwitchValue`/`KyaWriteSwitchValue` for the three switch records. The
  factory and exporter dispatch to them; the old codec was removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; destination, waypoint,
  switch and per-block list counts are validated before writing. Unknown tails
  are preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Teleporter` and `Teleporter condition`; edits use
  the shared placement patch path.
- Tests: `Kya.Bank.TeleporterFixtureRoundTrip` covers a populated class payload
  (one destination, one populated switch, two waypoints, one block with two
  operations and two conditions) plus the empty id 2 behaviour, byte-identical
  unedited export, seven edited fields through export/reimport and
  destination-count rejection. `Kya.Bank.LevelOneRoundTrip` covers the real bank
  offsets; `Kya.Editor.MilestoneCGate` audits exported fields on the real bank.
- Real-bank evidence (LEVEL_1, 5 teleporters): all carry behaviour id 2 with a
  zero-byte payload; class payloads parse with no raw tail.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Destination, waypoint and switch target references are scalar ids
  without per-entry reference pickers.

## Findings: Amortos

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorAmortos` (`CActor` -> `CObject`) and
  `CBehaviourAmortos` / `CBehaviourAmortosStand` (both empty `Create`).
- Factory registration: class 0x29 (`Types.h` `AMORTOS`).
- `CActorAmortos::Create` (`src/b-witch/ActorAmortos.cpp:11`) calls
  `CActor::Create`, reads three `S_ACTOR_STREAM_REF` lists, one actor reference,
  nine floats, a `CAddOnGenerator`, an attachment id, then two
  `S_NTF_TARGET_STREAM_REF` switch records. Behaviour 2 has no payload; ids 0/1
  are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | checkpoint actors | count u32 + count x 4-byte actor refs |
  | stream 188 | count u32 + count x 4-byte actor refs |
  | stream 18C | count u32 + count x 4-byte actor refs |
  | actor ref | field 0x190 s32 |
  | tuning | nine f32 in source order 164,168,160,16C,170,174,178,17C,180 |
  | add-on | 20 bytes (`CAddOnGenerator`) |
  | sub-object | mode u32; when 1, five f32 |
  | attachment | field 0x1C8 u32 |
  | switches | two `S_NTF_TARGET_STREAM_REF`: count u32 + count x 28-byte target + 32-byte camera |

  Total class data is 208 bytes for the fixture layout (mode 1, one target in the
  first switch). The earlier editor mapping labelled the first three floats
  160/164/168 in serialized order; the module now maps them to the real source
  fields 164/168/160. Bytes after the payload remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaAmortos.h` and
  `Private/Actors/KyaAmortos.cpp` own the class codec and reuse
  `KyaReadSwitchValue`/`KyaWriteSwitchValue` for the two switch records. The
  factory and exporter dispatch to them; the old codec was removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the three actor-stream
  counts, switch target counts and the add-on sub-object mode are validated
  before writing. Unknown tails are preserved; edits use the shared placement
  patch path (refresh/undo).
- Inspector: Details uses the `Amortos` category; edits use the shared placement
  patch path.
- Tests: `Kya.Bank.AmortosFixtureRoundTrip` covers a populated mode-1 class
  payload (three streams, nine floats, add-on, two switch records) plus the empty
  id 2 behaviour, byte-identical unedited export, seven edited fields through
  export/reimport and sub-object-mode rejection.
  `Kya.Bank.LevelOneRoundTrip` covers the real bank; `Kya.Editor.MilestoneCGate`
  audits exported fields on the real bank.
- Real-bank evidence (LEVEL_1, 6 Amortos): two carry behaviour id 1 (4-byte
  cinematic payload) and six carry the empty id 2; class payloads parse with no
  raw tail.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Actor refs and switch targets are scalar ids without per-entry
  reference pickers. `CActorAmortosCmd` is a separate class and remains tracked
  by its own rows.

## Findings: DCA

Status: **class and controlled/trajectory codecs extracted; real-bank IDs
reconciled**.

- Source classes: `CActorDCA` (`CActor` -> `CObject`) and its nested
  `CBhvDefault`, `CBhvControlled`, `CBhvIAControlled`, `CBhvTrajectory`.
- Factory registration: class 0x1b (`Types.h` `DCA`).
- `CActorDCA::Create` (`src/b-witch/ActorDCA.cpp:11`) calls `CActor::Create`,
  then reads four stream refs, a float, an aim-adjust angle, a `CFireShot` actor
  stream and two words. `CFireShot::Create` is just an `S_ACTOR_STREAM_REF`.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | ids | field 504 s32, field 500 s32 |
  | tuning | field 464 f32 |
  | stream ids | field 508 s32, field 50C s32 |
  | aim | aim-adjust degrees f32 (source multiplies by pi/180) |
  | fire-shot stream | count u32 + count x 4-byte actor refs |
  | tail | field 450 u32, field 454 u32 |

  Bytes after the payload remain raw in `ClassData`.
- `BuildBehaviour` (`ActorDCA.cpp:171`): 2 Default (empty `Create`), 3
  Controlled, 4 AI-controlled, 5 Trajectory; ids 0/1 are inherited. Payloads:

  | Id | Source | Size | Layout |
  | --- | --- | --- | --- |
  | 2 | CBhvDefault | 0 | no serialized payload |
  | 3 | CBhvControlled | 40 | fields 0/4/8, FOV degrees, vector 10 xyz, vector 20 xyz |
  | 4 | CBhvIAControlled | - | `Create` is an implementation guard; left raw |
  | 5 | CBhvTrajectory | 12 | actor ref s32, field 18 f32, field 8 u32 |

- Editor model/reader/writer: `Public/Actors/KyaDca.h` and
  `Private/Actors/KyaDca.cpp` own the class and behaviour codecs; the factory
  and exporter dispatch to them. The old structs/readers were removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the fire-shot stream
  count is validated before writing. Unknown class/behaviour tails are preserved;
  edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `DCA` and `DCA behaviour`; controlled/trajectory
  rows are gated by their presence flags and empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.DcaFixtureRoundTrip` covers the class payload and behaviours
  2/3/5, byte-identical unedited export, six edited fields through
  export/reimport and fire-shot-count rejection. `Kya.Bank.LevelOneRoundTrip`
  covers the real bank IDs; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 4 DCA actors): id 0 x2, id 1 x2 (4-byte
  cinematic), id 2 x2 (empty), id 3 x2 (40 bytes), id 5 x3 (12 bytes). Id 4 is
  absent; source does not decode it.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Behaviour 4 stays raw because its `Create` is an implementation
  guard. Aim input/output vectors are runtime state, not serialized.

## Findings: Fruit

Status: **class and squashed/inventory/bonus codecs extracted; real-bank IDs
reconciled** (class 0x16; the task brief's 0x0c does not match `Types.h`).

- Source classes: `CActorFruit` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor` -> `CObject`) and `CBehaviourFruit`, `CBehaviourFruitSuspended`,
  `CBehaviourFruitBonus`, `CBehaviourFruitSquashed`; ids 1/3/7 reuse shared
  behaviours and id 5 uses the shared `CBehaviourInventory`.
- Factory registration: class 0x16 (`Types.h` `FRUIT = 0x16`).
- `CActorFruit::Create` (`src/b-witch/ActorFruit.cpp:9`) calls
  `CActorAutonomous::Create` (weight/life/gravity/dynamic-extension prefix),
  reads field 0x360, then one switch notification: `S_NTF_SWITCH` on levels at or
  above 2.2, or `S_NTF_SWITCH_EX_LIST` on older levels.
- Class payload layout:

  | Section | Layout |
  | --- | --- |
  | autonomous prefix | weight f32, life u32, gravity f32, two dynamic-ext f32 |
  | field 0x360 | u32 |
  | modern switch | count u32 + count x 28-byte target + 32-byte camera |
  | legacy switch | count u32 + count x 16-byte target + 32-byte camera |

- `BuildBehaviour` (`ActorFruit.cpp:65`): 3 Suspended (empty), 4 Squashed,
  5 Inventory, 6 Bonus, 7 ProjectileNew; ids 0/1/2 are inherited/generic.
  Payloads:

  | Id | Source | Size | Layout |
  | --- | --- | --- | --- |
  | 3 | FruitSuspended | 0 | no serialized payload |
  | 4 | FruitSquashed | 4 | one f32 |
  | 5 | Inventory | 8/40/44/56 | `CInventoryInfo` |
  | 6 | FruitBonus | 8 | actor ref s32, initial behaviour s32 |
  | 7 | ProjectileNew | 4 | base `CBehaviourProjectile::Create` only; left raw |
- Editor model/reader/writer: `Public/Actors/KyaFruit.h` and
  `Private/Actors/KyaFruit.cpp` own the class and behaviour 4/5/6 codecs; the
  factory and exporter dispatch to them. The old `FKyaFruitOffsets`,
  `FKyaFruitInventoryValues` and readers were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; `FruitLifeMax` is
  stored as int64 and validated to 0..`MAX_uint32` so the serialized u32
  round-trips exactly. The switch target count and inventory shape are validated
  before writing. Unknown class tails
  and the raw id 7 payload are preserved; edits use the shared placement patch
  path (refresh/undo).
- Inspector: Details groups are `Fruit` and `Fruit behaviour`; empty groups hide
  through `EditConditionHides(true)`.
- Tests: `Kya.Bank.FruitFixtureRoundTrip` covers the modern class payload,
  behaviours 3/4/5/6 and the raw id 7 record, byte-identical unedited export,
  seven edited fields through export/reimport and switch-count rejection.
  `Kya.Bank.LevelOneRoundTrip` covers the real bank IDs;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 9 fruits): id 1 x3 (4-byte cinematic), id 3 x1
  (empty), id 4 x6 (4 bytes), id 5 x3 (44 bytes), id 6 x3 (8 bytes); id 7 does
  not occur.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The bank does not serialize the scene version, so the legacy
  16-byte switch layout is not auto-selected; such payloads stay raw. Behaviour
  7 is named but has no editable payload. Inventory 8/40/44/56 shapes are
  decoded, but `CBehaviourInventory` remains shared with Projectile and a future
  common inventory module can remove the duplicate field mapping.

## Findings: Nativ

This section records the earlier TakeAndPut milestone and its fixture evidence.
For current behaviour coverage, see **Implementation progress: Nativ Speak,
Live, Seller and Exorcism** and **Implementation progress: Nativ Akasa** above;
their implementations supersede the raw-only exclusions below. Current rows are
`[~]`, including the subsequently added code that has not been built or tested.

Status: **class and take-and-put codec extracted; real-bank IDs reconciled**.

- Source classes: `CActorNativ` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor` -> `CObject`) and `CBehaviourNativ` with `Speak`, `Live`,
  `TakeAndPut`, `Seller`, `Akasa`, `Exorcisme`.
- Factory registration: class 0x0a (`Types.h` `NATIVE`).
- `CActorNativ::Create` (`src/b-witch/ActorNativ.cpp:23`) calls
  `CActorAutonomous::Create`, then reads eight words/floats and five trailing
  words for a fixed 64-byte payload.
- `BuildBehaviour` (`ActorNativ.cpp:330`): 3 Speak, 4 Exorcism (empty), 5 Live,
  6 TakeAndPut, 7 Seller, 8 Akasa; 0/1/2 are inherited. Only behaviour 6 is
  decoded here: count u32 + count x 16-byte trajectory (`field 0` f32,
  path-follow index s32, optional word, field 18 f32), matching
  `CPathFollowReader::Create` (`Actor.cpp:65`).
- Editor model/reader/writer: `Public/Actors/KyaNativ.h` and
  `Private/Actors/KyaNativ.cpp` own the class and behaviour 6 codecs; the factory
  and exporter dispatch to them. `NativLifeMax` is int64 and validated to
  0..`MAX_uint32` so the serialized u32 round-trips exactly. The old readers were
  removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the trajectory count
  is validated before writing. Unknown class tails and undecoded behaviour
  payloads (5/7/8) stay raw; edits use the shared placement patch path.
- Inspector: Details groups are `Nativ` and `Nativ behaviour`; empty groups hide
  through `EditConditionHides(true)`.
- Tests: `Kya.Bank.NativFixtureRoundTrip` covers the 64-byte class payload,
  empty behaviour 4 (Exorcism), behaviour 6 with one trajectory, byte-identical
  unedited export, six edited fields through export/reimport and trajectory-count
  rejection. `Kya.Bank.LevelOneRoundTrip` covers the real bank;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 7 Nativ): id 0 x7 (empty), id 1 x2 (cinematic),
  id 2 x2 (catch-by-trap), id 4 x5 (empty), id 6 x2 (20 bytes).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Behaviours 3 (Speak), 5 (Live), 7 (Seller) and 8 (Akasa) are not
  decoded in this pass and stay raw. The take-and-put optional 4-byte field is
  preserved but not semantically named.

## Findings: Wolfen

Status: **class/fight-parameter and twelve behaviour codecs extracted; shared
Fighter/switch-script helpers split out**.

- Shared helpers: `Actors/KyaFighter.h` now owns the `CActorFighter` prefix
  offsets and blow/grab/react/combo traversal used by Wolfen and Kim;
  `Actors/KyaSwitchScript.h` owns the four-category switch-script reader used by
  the Wolfen behaviours. `KyaCameraStream.h` includes both.
- Source classes: `CActorWolfen` (`CActorFighter` -> `CActorAutonomous` ->
  `CActorMovable` -> `CActor` -> `CObject`) and the Wolfen behaviour family.
- Factory registration: class 0x09 (`Types.h` `WOLFEN`).
- `CActorWolfen::Create` (`src/b-witch/ActorWolfen.cpp:151`) calls
  `CActorFighter::Create`, reads field 0xB74, vision, start sector, hearing and
  vision-detection props, movement floats, capability/combo fight parameters and
  a trailing block. The class codec lives in `Actors/KyaWolfen.h/.cpp`; the
  fighter prefix population remains shared with Kim in the factory.
- `BuildBehaviour` (`ActorWolfen.cpp:786`): fighter ids 3/4/5/6 plus Wolfen
  ids 8 WatchDog, 10 GuardArea, 12 Sleep, 15 Track, 16 TrackStand,
  17/18/19 TrackWeapon variants, 21 Snipe, 22 Lost, 23 DCA, 24 Avoid,
  25 GrindDeath, 14 Exorcism and 13 Escape. Decoded payloads:
  | Id | Layout |
  | --- | --- |
  | 8 | field 80/94, switch script, on/off targets + camera |
  | 10 | guard path/fields, switch script, on/off targets + camera |
  | 13 | reader count + 4-byte path indices, fields 88/9C/90/8C/94/98/A0, switch script |
  | 14 | 48 bytes: material, field 24, four mesh/texture ids, field 18, five words |
  | 15 | flag + float |
  | 17/18/19 | field 90/94/98, 20-byte sub-objects, trailing fields |
  | 21 | snipe fields, shadow material, spot/shadow counts, switch script |
  | 22 | switch script |
  | 23 | flag + actor ref + cooldown |
  | 24 | flag + two floats |
- Editor model/reader/writer: `Public/Actors/KyaWolfen.h` and
  `Private/Actors/KyaWolfen.cpp` own the class, fight-parameter and behaviour
  codecs; the factory/exporter dispatch to them and still write the shared
  fighter prefix. `KyaCameraStream.h` retains only the shared prefixes.
- Capability/applicability: every typed field is exported; capability/combo and
  switch-script counts are validated, including a per-slot category check so a
  category swap cannot write the wrong record width. `FighterLifeMax` is int64
  and validated to 0..`MAX_uint32` in the shared fighter path. Unknown/undecoded
  payloads (16, 25 and the fighter behaviours) stay raw.
- Tests: `Kya.Bank.WolfenFixtureRoundTrip` covers a minimal fighter prefix,
  class/fight parameters and behaviours 15/23/24, byte-identical unedited export,
  eight edited fields through export/reimport and combo-count rejection.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip` cover the
  real Wolfen bank, including the Escape path-list fix.
- Real-bank evidence (LEVEL_1): 24 Wolfen actors with fighter ids 0–6 and Wolfen
  behaviours 8/10/13/14/15/16/17/18/19/21/22/23/24; the selected-actor gate
  writes a sector bank containing SCRAWNY_01 and GRUNT_01.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. TrackStand (16), GrindDeath (25), Sleep (12) and the fighter
  behaviours have no decoded payload yet. The shared fighter prefix is populated
  and written by the factory/exporter rather than the Wolfen module, and the
  reader retains the existing 4-byte gap between walk values and the magic count
  to match real-bank data.

## Findings: Commander

Status: **class and behaviour 2 codec extracted; real-bank IDs reconciled**.

- Source classes: `CActorCommander` (`CActor` -> `CObject`) and
  `CBehaviourCommander`, `CBehaviourCommanderDefault`,
  `CBehaviourCommanderBeatUp`.
- Factory registration: class 0x13 (`Types.h` `COMMANDER`).
- `CActorCommander::Create` (`src/b-witch/ActorCommander.cpp:26`) calls
  `CActor::Create`, reads five zone/path words, a 12-byte `CSquad`, team enemy
  indices, `CreateSequence` component pairs and a 56-byte camera-figure block.
- `BuildBehaviour` (`ActorCommander.cpp:243`): 0 creates a default commander,
  2 returns the stored default behaviour, 3 is BeatUp; 1 is inherited.
- Behaviour 2 payload (`CBehaviourCommander::Create`): 8-byte squad config plus
  one target/camera switch record. Bank shapes are 44 bytes (zero targets) and
  72 bytes (one target).
- Editor model/reader/writer: `Public/Actors/KyaCommander.h` and
  `Private/Actors/KyaCommander.cpp` own the class, team/component/camera and
  behaviour 2 codecs; the factory and exporter dispatch to them. The old readers
  were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; team, component and
  switch-target counts are validated before writing. Unknown tails and the
  BeatUp payload stay raw; edits use the shared placement patch path.
- Inspector: Details groups are `Commander` and `Commander behaviour`; empty
  groups hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.CommanderFixtureRoundTrip` covers a populated class payload
  (one team, one component, 14 camera values) and a 72-byte behaviour 2,
  byte-identical unedited export, eight edited fields through export/reimport and
  switch-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real bank;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 8 Commanders): behaviour 1 x1 (cinematic),
  behaviour 2 x7 at 44 bytes and x1 at 72 bytes.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Behaviour 3 (BeatUp) is not decoded in this pass. The CSquad internals
  are exposed as three scalar words rather than a named struct.

## Findings: Kim / hero private

Status: **class codec extracted; real-bank IDs reconciled** (class 0x06; the
brief's 0x0b is Electrolla).

- Source classes: `CActorHeroPrivate` (`CActorHero` -> `CActorFighter` ->
  `CActorAutonomous`) and `CBehaviourHero`, `CBehaviourHeroDefault`,
  `CBehaviourHeroUnknown`, `CBehaviourHeroRideJamGut`.
- Factory registration: class 0x06 (`Types.h` `ACTOR_HERO_PRIVATE`). The same
  class also carries the HeroUnknown behaviours 9/10/11.
- `CActorHeroPrivate::Create` (`src/b-witch/ActorHero_Private.cpp:111`) calls
  `CActorFighter::Create` (shared prefix), reads ten bone/anim keys, a climb-zone
  stream, tutorial zones (24-byte header + target list + camera), projectile
  zones, boomy zone pairs, a zone stream, material/fx/joke values, actor cameras
  and a 20-byte tail.
- Editor model/reader/writer: `Public/Actors/KyaKim.h` and
  `Private/Actors/KyaKim.cpp` own the Kim class codec. The shared actor-camera
  reader moved to `Public/Actors/KyaActorCamera.h` (used by Kim and available to
  Boomy/Aton). The factory still populates the shared fighter prefix; the
  exporter still writes it. The old Kim/camera codecs were removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; climb/tutorial/
  projectile/pair/zone/camera counts and FX handles are validated. Behaviour
  3/4/7 payloads are empty; 9/10/11 use the existing HeroUnknown codec; 12
  RideJamGut stays raw.
- Inspector: Details groups are `Fighter`, `Kim` and the hero behaviours; empty
  groups hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.KimFixtureRoundTrip` covers the shared fighter prefix, ten
  bone keys, one zone-stream entry, material/fx/joke, FX handles and the empty
  behaviour 3, byte-identical unedited export, eight edited fields through
  export/reimport and zone-stream-count rejection. `Kya.Bank.LevelOneRoundTrip`
  and `Kya.Bank.SelectedActorRoundTrip` cover the real Kim;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Kim): behaviour 1 (cinematic), 3/4/7 empty,
  9/10/11 (28 bytes, HeroUnknown).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Tutorial/camera records are supported but absent from LEVEL_1, so
  the fixture uses the zero-camera layout. Behaviour 12 and the fighter
  behaviours are not decoded here.

## Findings: Boomy

Status: **class/camera codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorBoomy` (`CActorMovable` -> `CActor` -> `CObject`) and
  `CBehaviourBoomyLaunch` (empty `Create`).
- Factory registration: class 0x05 (`Types.h` `BOOMY`).
- `CActorBoomy::Create` (`src/b-witch/ActorBoomy.cpp:14`) calls
  `CActorMovable::Create` (weight), then reads the 20-byte vision record,
  tail/fx values, particles, texture/mesh, a full actor-camera record and a
  16-byte type table. `BuildBehaviour` only adds id 2 (`launchBehaviour`);
  ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | weight | f32 |
  | vision | range f32, angle f32, fields 38/3C f32, flags u32 |
  | tail/fx | sound 2B8/2BC u32, field 744 f32, tail width axis u32, field 740 f32, tail segments u32 |
  | particles | ids 3D0/3D4/3DC u32, field 3CC u32 |
  | mesh | texture s32, mesh s32 |
  | camera | 64-byte `CCamera` record + optional switch/conditions/config/extra |
  | types | count u32 + count x 16-byte (field 4 f32, vision range f32, hit damage s32, fx colour u32) |

  Bytes after the type table remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaBoomy.h` and
  `Private/Actors/KyaBoomy.cpp` own the class codec; the camera record uses the
  shared `KyaReadActorCameraOffsets`/`KyaWriteActorCameraValues` in
  `Public/Actors/KyaActorCamera.h`. The factory and exporter dispatch to the
  module; the old struct/reader were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the type count and
  all camera list/mode counts are validated before writing, including the
  layout-changing `Flags & 2` switch bit. Unknown tails are preserved; edits use
  the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Boomy`, `Boomy camera` and `Boomy type`; empty
  groups hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.BoomyFixtureRoundTrip` covers a populated class payload
  (switch-enabled camera with one target, one condition, mode-1 config and 12
  extra values, plus one type entry) and the empty id 2 behaviour, byte-identical
  unedited export, eight edited fields through export/reimport and type-count
  rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real Boomy;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Boomy): a single actor with empty behaviour id 2
  and a fully consumed class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Vision/type entries and particle ids are exposed as scalar ids
  without per-entry reference pickers.

## Findings: Aton

Status: **class/path/add-on codec extracted; behaviour id 3 audited as empty**.

- Source classes: `CActorAton` (`CActorAutonomous` -> `CActorMovable` -> `CActor`
  -> `CObject`), `CBehaviourAton`, `CBehaviourAtonEscape` and the
  `CBehaviourAddOnAton` sub-object block.
- Factory registration: class 0x38 (`Types.h` `ATON`).
- `CActorAton::Create` (`src/b-witch/ActorAton.cpp:42`) calls
  `CActorAutonomous::Create` (weight/life/gravity/dynamic-extension prefix),
  reads four words, effect type, texture/mesh, trail material and path delay,
  then `CPathPlaneArray::Create` and `CBehaviourAddOnAton::Create`.
  `BuildBehaviour` only adds id 3 (`behaviourAtonEscape`); ids 0/1/2 are
  inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | fields | field 354/358/35C/360 u32 |
  | visuals | effect type u32, texture s32, mesh s32, trail material s32 |
  | path | path default delay f32, plane count u32 + count x 4-byte path index |
  | add-on header | field 18 u32 |
  | other sub-objects | count u32 + count x (field 0 u32, cinematic count u32, ids) |
  | Aton sub-objects | count u32 + count x (field 18 u32, field 0 u32, cinematic count u32, ids) |
  | add-on tail | field 24 f32 |

  Bytes after field 24 remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaAton.h` and
  `Private/Actors/KyaAton.cpp` own the class codec. `AtonLifeMax` is int64 and
  validated to 0..`MAX_uint32` so the serialized u32 round-trips exactly. The
  factory and exporter dispatch to the module; the old struct/reader were
  removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; path, sub-object and
  per-sub-object cinematic counts are validated before writing. Unknown tails
  are preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Aton`; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.AtonFixtureRoundTrip` covers a populated class payload (one
  path plane, one "other" and one Aton sub-object with cinematics) and the empty
  id 3 behaviour, byte-identical unedited export, eight edited fields through
  export/reimport, life-range rejection and cinematic-count rejection.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip` cover the
  real Aton; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Aton): behaviour 1 (4-byte cinematic) and empty
  id 3; the class payload is fully consumed.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The source assigns `field_0x24 = 10.0f` immediately after reading
  it; the editor keeps the serialized byte value. Path and cinematic ids are
  scalar without per-entry reference pickers.

## Findings: Rope

Status: **class/switch codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorRope` (`CActor` -> `CObject`) and
  `CBehaviourRopeStand` (empty `Create`).
- Factory registration: class 0x0f (`Types.h` `ROPE`).
- `CActorRope::Create` (`src/b-witch/ActorRope.cpp:8`) calls `CActor::Create`,
  then reads a path index, rope scale, field 0x17C, material id, packed colour,
  sound reference, field 0x194, two actor-stream indices, one
  `S_NTF_TARGET_STREAM_REF` switch record and the rope flags. `BuildBehaviour`
  only adds id 2 (`behaviourRopeStand`); ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | path | path index s32 |
  | tuning | rope scale f32, field 17C f32 |
  | visuals | material id s32, packed colour u32 |
  | sound | sound reference s32, field 194 s32 |
  | actors | actor-stream index A s32, actor-stream index B s32 |
  | switch | `S_NTF_TARGET_STREAM_REF`: count u32 + count x 28-byte target + 32-byte camera |
  | flags | rope flags u32 |

  Bytes after the flags remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaRope.h` and
  `Private/Actors/KyaRope.cpp` own the class codec and reuse the shared
  `KyaReadSwitchValue`/`KyaWriteSwitchValue`. The factory and exporter dispatch
  to the module; the old struct/reader were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the switch target
  count is validated before writing. Unknown tails are preserved; edits use the
  shared placement patch path (refresh/undo).
- Inspector: Details uses the `Rope` category; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.RopeFixtureRoundTrip` covers a populated class payload (one
  switch target with camera), the empty id 2 behaviour, byte-identical unedited
  export, seven edited fields through export/reimport and switch-count
  rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real Rope;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Rope): a single actor with empty behaviour id 2
  and a fully consumed class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The path index is exposed as a scalar path reference without a
  per-entry picker beyond the existing reference field.

## Findings: Stiller

Status: **class/switch/add-on codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorStiller` (`CActor` -> `CObject`),
  `CBehaviourStiller` and `CBehaviourStillerStand` (both empty `Create`).
- Factory registration: class 0x4C (`Types.h` `STILLER`).
- `CActorStiller::Create` (`src/b-witch/ActorStiller.cpp:8`) calls
  `CActor::Create`, then reads the vision record, four bone words, a path-follow
  index, field 0x1E0, a `CLifeBase` (f32 maximum), three floats, field 0x1F8, an
  `S_ACTOR_STREAM_REF` actor list, field 0x200, two
  `S_NTF_TARGET_STREAM_REF` switch records, field 0x214 and a
  `CAddOnGenerator`. `BuildBehaviour` only adds id 2 (`behaviourStand`); ids 0/1
  are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | vision | range f32, half-angle f32, fields 38/3C f32, flags u32 |
  | bones | fields 1D0/1D4/1D8/1DC u32 |
  | path | path-follow index s32 |
  | life | field 1E0 f32, life maximum f32 |
  | tuning | fields 1EC/1F0/1F4 f32 |
  | identity | field 1F8 u32 |
  | actors | count u32 + count x 4-byte actor refs |
  | field 200 | s32 |
  | switches | two `S_NTF_TARGET_STREAM_REF`: count u32 + count x 28-byte target + 32-byte camera |
  | field 214 | s32 |
  | add-on | 20 bytes (`CAddOnGenerator`); sub-object mode u32, then five f32 when 1 |

  Bytes after the add-on remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaStiller.h` and
  `Private/Actors/KyaStiller.cpp` own the class codec and reuse the shared
  `FKyaSwitchActorOffsets`/`KyaReadSwitchValue`/`KyaWriteSwitchValue`. The
  factory and exporter dispatch to the module; the old struct/reader were
  removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the actor-list count,
  both switch target counts and the add-on sub-object mode are validated before
  writing. Unknown tails are preserved; edits use the shared placement patch
  path (refresh/undo).
- Inspector: Details groups are `Stiller`; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.StillerFixtureRoundTrip` covers a populated class payload
  (one actor ref, one target in the first switch, mode-1 add-on) and the empty
  id 2 behaviour, byte-identical unedited export, eight edited fields through
  export/reimport and switch-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real Stiller;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 2 Stiller): both actors carry empty behaviour id 2
  and fully consumed class payloads.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The path index is a scalar path reference; bone/actor ids have no
  per-entry pickers.

## Findings: Ambre

Status: **class/switch codec extracted; behaviour ids 2/3 audited as empty**.

- Source classes: `CActorAmbre` (`CActor` -> `CObject`), `CBehaviourAmbre` and
  `CBehaviourAmbreUnknown`.
- Factory registration: class 0x14 (`Types.h` `AMBER`).
- `CActorAmbre::Create` (`src/b-witch/ActorAmbre.cpp:21`) calls `CActor::Create`,
  then reads a material id, effect type, texture/mesh ids, field 0x1D0 and one
  `S_NTF_TARGET_STREAM_REF` switch record. `BuildBehaviour` maps id 2 to
  `behaviourAmber` and id 3 to a new `CBehaviourAmbreUnknown`; neither overrides
  `Create`, so both payloads are empty. Ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | visuals | material id s32, effect type s32, texture s32, mesh s32 |
  | field 1D0 | s32 |
  | switch | `S_NTF_TARGET_STREAM_REF`: count u32 + count x 28-byte target + 32-byte camera |

  Bytes after the camera remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaAmbre.h` and
  `Private/Actors/KyaAmbre.cpp` own the class codec and reuse the shared
  `FKyaSwitchActorOffsets`/`KyaReadSwitchValue`/`KyaWriteSwitchValue`. The
  factory and exporter dispatch to the module; the old struct/reader were
  removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the switch target
  count is validated before writing. Unknown tails are preserved; edits use the
  shared placement patch path (refresh/undo).
- Inspector: Details uses the `Ambre` category; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.AmbreFixtureRoundTrip` covers a populated class payload (one
  switch target with camera) and the empty id 2 behaviour, byte-identical
  unedited export, seven edited fields through export/reimport and switch-count
  rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real Ambre;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 2 Ambre): both actors carry empty behaviour id 2
  and fully consumed class payloads.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Effect/mesh ids are scalar and have no per-entry pickers.

## Findings: Bonus fountain

Status: **add-on codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorBonusFountain` (`CActor` -> `CObject`) and
  `CBehaviourBonusFountainStand`.
- Factory registration: class 0x52 (`Types.h` `BONUS_FOUNTAIN`).
- `CActorBonusFountain::Create` (`src/b-witch/ActorBonusFountain.cpp:10`) calls
  `CActor::Create` and then `CAddOnGenerator::Create`; the entire class payload is
  the add-on generator. `BuildBehaviour` only adds id 2; ids 0/1 are inherited.
  `CBehaviourBonusFountainStand` has no `Create` override, so its payload is
  empty.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | add-on | 20 bytes: field 0, money type, money count, bonus count, field C |
  | sub-object | mode u32; when 1, five f32 |

  Bytes after the sub-object block remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaFountain.h` and
  `Private/Actors/KyaFountain.cpp` own the add-on codec. The factory and exporter
  dispatch to the module; the old `FKyaBonusFountainOffsets`/reader were removed
  from `KyaCameraStream.h` and the model keeps the existing
  `BonusFountainAddOn` field.
- Capability/applicability: every typed field is exported; the add-on sub-object
  mode is validated before writing so the optional five-float block cannot be
  added or removed. Unknown tails are preserved; edits use the shared placement
  patch path (refresh/undo).
- Inspector: Details uses the `Bonus fountain` category; the group hides through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.FountainFixtureRoundTrip` covers a mode-1 add-on payload and
  the empty id 2 behaviour, byte-identical unedited export, seven edited fields
  through export/reimport and mode rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real fountain;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Bonus fountain): empty behaviour id 2 and a
  fully consumed add-on payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Money/bonus instances are runtime state and are not serialized.

## Findings: Checkpoints

Status: **checkpoint record/pair codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorCheckpointManager` (`CActor` -> `CObject`) and
  `CBehaviourCheckpoint`.
- Factory registration: class 0x1E (`Types.h` `CHECKPOINT_MANAGER`).
- `CActorCheckpointManager::Create` (`src/b-witch/ActorCheckpointManager.cpp:9`)
  calls `CActor::Create`, reads a checkpoint count, then per checkpoint: zone
  ref, waypoint A, sector id, waypoint B, flags and an actor/waypoint pair list.
  `BuildBehaviour` only adds id 2 (`behaviourCheckpoint`), whose `Create` is
  empty; ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | count | checkpoint count s32 |
  | record | zone ref s32, waypoint A s32, sector id s32, waypoint B s32, flags u32 |
  | pairs | pair count s32 + count x 8-byte (actor ref, waypoint ref) |

  Bytes after the last record remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaCheckpoints.h` and
  `Private/Actors/KyaCheckpoints.cpp` own the record codec and keep per-record
  offsets so the writer never re-scans. The factory and exporter dispatch to the
  module; the inline `KyaReadCheckpointOffsets` reader and the exporter's inline
  writer were removed. `Flags` is int64 and validated to 0..`MAX_uint32` because
  the serialized field is a u32.
- Capability/applicability: every typed field is exported; the record count,
  per-record pair counts and flags are validated before writing. Unknown tails
  are preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details uses the `Checkpoints` category; the group hides through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.CheckpointsFixtureRoundTrip` covers one checkpoint with one
  actor/waypoint pair and the empty id 2 behaviour, byte-identical unedited
  export, seven edited fields through export/reimport and pair-count rejection.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip` cover the
  real checkpoints; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 2 Checkpoint managers): both carry empty behaviour
  id 2 and fully consumed checkpoint records.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Zone/waypoint/actor ids are scalar references without per-entry
  pickers.

## Findings: Bridge

Status: **class and basic/wind-aware behaviour codecs extracted**.

- Source classes: `CActorBridge` (`CActor` -> `CObject`) and
  `CBehaviourBridgeBasic` / `CBehaviourBridgeWindAware`.
- Factory registration: class 0x17 (`Types.h` `BRIDGE`).
- `CActorBridge::Create` (`src/b-witch/ActorBridge.cpp:481`) calls `CActor::Create`,
  reads an `S_ACTOR_STREAM_REF` actor list, two waypoint indices, five floats,
  field 0x344, config flags, field 0x348 and the step-collision scale.
- `BuildBehaviour` (`ActorBridge.cpp:1097`): 2 Basic, 3 WindAware; ids 0/1 are
  inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | actors | count u32 + count x 4-byte actor refs |
  | waypoints | waypoint A s32, waypoint B s32 |
  | tuning | fields 330/334/33C/340/338 f32 |
  | config | field 344 u32, config flags u32, field 348 s32, step-collision scale f32 |

  Bytes after the step-collision scale remain raw in `ClassData`.
- Behaviour payloads:

  | Id | Source | Size | Layout |
  | --- | --- | --- | --- |
  | 2 | BridgeBasic | 32 | direction xyz f32, five f32 at +12 |
  | 3 | BridgeWindAware | 20 | five f32 |

- Editor model/reader/writer: `Public/Actors/KyaBridge.h` and
  `Private/Actors/KyaBridge.cpp` own the class and behaviour codecs; the factory
  and exporter dispatch to them. The old structs/readers were removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the actor-ref count
  and the two behaviour payload sizes are validated before writing. Unknown
  tails are preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Bridge` and `Bridge behaviour`; empty groups
  hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.BridgeFixtureRoundTrip` covers a populated class payload (one
  actor ref, both waypoints, five tuning floats) plus behaviours 2 and 3,
  byte-identical unedited export, seven edited fields through export/reimport
  and actor-ref-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real bridge;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Bridge): the 32-byte basic behaviour id 2 and a
  fully consumed class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The wind-aware behaviour is source-derived and fixture-covered but
  absent from LEVEL_1. Waypoint ids are scalar references.

## Findings: Fog manager

Status: **hierarchy and fog-zone codecs extracted; real-bank IDs reconciled**.

- Source classes: `CActorFogManager` (`CActor` -> `CObject`) and
  `CBehaviourFogManagerZones`.
- Factory registration: class 0x3D (`Types.h` `FOG_MANAGER`).
- `CActorFogManager::Create` (`src/b-witch/ActorFogManager.cpp:8`) calls
  `CActor::Create`, reads a fog-hierarchy count, then per hierarchy
  `CSectorHierarchy::Create` (u64 hash) plus two floats. `BuildBehaviour` only
  adds id 2 (`CBehaviourFogManagerZones`); ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | count | hierarchy count s32 |
  | hierarchy | hash u64, field 20 f32, field 24 f32 (16 bytes each) |

  Bytes after the last hierarchy remain raw in `ClassData`.
- Behaviour 2 payload (`CStreamFogZoneInternal`): count u32 + count x 28-byte
  zone (flags, zone ref, clip value, field 4, fog RGBA, fog flags, field 18).
- Editor model/reader/writer: `Public/Actors/KyaFog.h` and
  `Private/Actors/KyaFog.cpp` own the class and behaviour codecs; the factory and
  exporter dispatch to them. The old structs/readers were removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the hierarchy count
  and fog-zone count are validated before writing. Unknown tails are preserved;
  edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Fog` and `Fog zone`; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.FogFixtureRoundTrip` covers one hierarchy and one 28-byte fog
  zone, byte-identical unedited export, seven edited fields through
  export/reimport and hierarchy-count rejection.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip` cover the
  real fog managers; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 3 Fog managers): each carries the 4-byte id 2
  behaviour with a zero zone count; class payloads are fully consumed.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The fixture uses a populated zone list because LEVEL_1 has none.

## Findings: Shoot

Status: **class/add-on and fire/wave behaviour codecs extracted**.

- Source classes: `CActorShoot` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor`), `CBehaviourShoot`, `CBehaviourShootFire` and
  `CBehaviourShootFireWave`.
- Factory registration: class 0x3E (`Types.h` `SHOOT`).
- `CActorShoot::Create` (`src/b-witch/ActorShoot.cpp:15`) calls
  `CActorAutonomous::Create`, reads two words, texture/mesh ids, four floats, a
  word, four floats, four trailing words and a `CAddOnGenerator`.
- `BuildBehaviour` (`ActorShoot.cpp:133`): 3 Fire, 4 FireWave; ids 0/1/2 are
  inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | ids | field 350 u32, field 354 u32 |
  | visuals | texture s32, mesh s32 |
  | tuning | fields 3C0/3C4/3C8/3CC f32, field 3D0 u32, fields 3D4/3D8/3DC/3E0 f32 |
  | tail words | fields 3E8/3E4/3EC/3F0 u32 |
  | add-on | 20 bytes (`CAddOnGenerator`); sub-object mode u32, then five f32 when 1 |

  Bytes after the add-on remain raw in `ClassData`.
- Behaviour payloads:

  | Id | Source | Layout |
  | --- | --- | --- |
  | 3 | ShootFire | field 2B0 u32, field 8 f32, field 2B4 f32, then S_ACTOR_STREAM_REF (count u32 + refs) |
  | 4 | ShootFireWave | seven f32 (`CConicalWaveShoot`) |

- Editor model/reader/writer: `Public/Actors/KyaShoot.h` and
  `Private/Actors/KyaShoot.cpp` own the class and behaviour codecs. `ShootLifeMax`
  is int64 and validated to 0..`MAX_uint32` so the serialized u32 round-trips
  exactly. The factory and exporter dispatch to the module; the old
  structs/readers were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the life range, the
  add-on sub-object mode and the fire actor-ref count are validated before
  writing. Unknown tails are preserved; edits use the shared placement patch
  path (refresh/undo).
- Inspector: Details groups are `Shoot` and the two behaviour groups; empty
  groups hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.ShootFixtureRoundTrip` covers the full class payload with a
  mode-1 add-on plus fire behaviour 3 (three refs) and wave behaviour 4,
  byte-identical unedited export, eight edited fields through export/reimport and
  fire-ref-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real shooters;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 3 Shoot actors): each carries only the 28-byte
  fire behaviour id 3.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The wave behaviour is source-derived and fixture-covered but absent
  from LEVEL_1.

## Findings: Money

Status: **class and flock/add-on behaviour codecs extracted**.

- Source classes: `CActorMoney` (`CActorMovable` -> `CActor`) and
  `CBehaviourMoneyFlock` / `CBehaviourMoneyAddOn`.
- Factory registration: class 0x32 (`Types.h` `MONEY`).
- `CActorMoney::Create` (`src/b-witch/ActorMoney.cpp:11`) calls
  `CActorMovable::Create` (weight), then reads the money value, sound reference
  and material id.
- `BuildBehaviour` (`ActorMoney.cpp:97`): 2 Flock, 3 AddOn; ids 0/1 are
  inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | weight | f32 |
  | money | money value u32 |
  | sound | sound reference s32 |
  | material | material id s32 |

  Bytes after the material remain raw in `ClassData`.
- Behaviour payloads:

  | Id | Source | Layout |
  | --- | --- | --- |
  | 2 | MoneyFlock | path-follow index s32 |
  | 3 | MoneyAddOn | empty `Create` |

- Editor model/reader/writer: `Public/Actors/KyaMoney.h` and
  `Private/Actors/KyaMoney.cpp` own the class and behaviour codecs. The flock
  path index is exposed as `MoneyFlockPathIndex` gated by
  `bHasTypedMoneyFlockBehaviour`. The factory and exporter dispatch to the
  module; the old struct/reader were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the flock payload size
  is validated before writing. Unknown tails are preserved; edits use the shared
  placement patch path (refresh/undo).
- Inspector: Details groups are `Money` and `Money behaviour`; empty groups hide
  through `EditConditionHides(true)`.
- Tests: `Kya.Bank.MoneyFixtureRoundTrip` covers the class payload and
  behaviours 2 (path index) and 3 (empty), byte-identical unedited export and
  five edited fields through export/reimport.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip` cover the
  real money actors; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 3 Money actors): each carries only the empty
  add-on behaviour id 3.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Behaviour 2 is source-derived and fixture-covered but absent from
  LEVEL_1.

## Findings: Box

Status: **class/switch/add-on codec extracted; behaviour id 3 audited as empty**.

- Source classes: `CActorBox` (`CActorAutonomous` -> `CActorMovable` -> `CActor`),
  `CBehaviourBox` and `CBehaviourBoxStand`.
- Factory registration: class 0x35 (`Types.h` `BOX`).
- `CActorBox::Create` (`src/b-witch/ActorBox.cpp:6`) calls
  `CActorAutonomous::Create`, reads field 0x350, an `S_ACTOR_STREAM_REF` actor
  list, four floats, field 0x364, four more words, one
  `S_NTF_TARGET_STREAM_REF` switch record and a `CAddOnGenerator`.
  `BuildBehaviour` only adds id 3 (`behaviourBoxStand`); ids 0/1/2 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | autonomous prefix | weight f32, life u32, gravity f32, two dynamic-ext f32 |
  | field 350 | s32 |
  | actors | count u32 + count x 4-byte actor refs |
  | tuning | fields 354/358/35C/360 f32 |
  | fields | field 364 u32, fields 370/374/378/37C s32 |
  | switch | `S_NTF_TARGET_STREAM_REF`: count u32 + count x 28-byte target + 32-byte camera |
  | add-on | 20 bytes (`CAddOnGenerator`); sub-object mode u32, then five f32 when 1 |

  Bytes after the add-on remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaBox.h` and
  `Private/Actors/KyaBox.cpp` own the class codec and reuse the shared
  `KyaReadSwitchValue`/`KyaWriteSwitchValue`. `BoxLifeMax` is int64 and validated
  to 0..`MAX_uint32`. The factory and exporter dispatch to the module; the old
  struct/reader were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the life range,
  actor-ref count, switch target count and add-on mode are validated before
  writing. Unknown tails are preserved; edits use the shared placement patch path
  (refresh/undo).
- Inspector: Details groups are `Box`; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.BoxFixtureRoundTrip` covers a populated class payload (one
  actor ref, one switch target, mode-1 add-on) and the empty id 3 behaviour,
  byte-identical unedited export, eight edited fields through export/reimport and
  actor-ref-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real boxes;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 4 Box actors): each carries only the empty
  box-stand behaviour id 3 with fully consumed class payloads.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Actor/waypoint ids are scalar references without per-entry pickers.

## Findings: FX

Status: **class and single/path/unknown behaviour codecs extracted**.

- Source classes: `CActorFx` (`CActor` -> `CObject`) and the nested
  `CBhvSingle`, `CBhvPath` and `CBhvUnknown` behaviours.
- Factory registration: class 0x2E (`Types.h` `FX`).
- `CActorFx::Create` (`src/b-witch/ActorFx.cpp:8`) calls `CActor::Create` and
  reads two floats (field 0x19C, clamped by the game to 0..1, and field 0x1A0).
  `BuildBehaviour` maps 2 Single, 3 Path and 4 Unknown; ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | effect timing | field 19C f32, field 1A0 f32 |

  Bytes after the second float remain raw in `ClassData`.
- Behaviour payloads:

  | Id | Source | Layout |
  | --- | --- | --- |
  | 2 | CBhvSingle | effect id s32 |
  | 3 | CBhvPath | effect id s32, path-follow index s32 |
  | 4 | CBhvUnknown | count u32 + count x s32 |

- Editor model/reader/writer: `Public/Actors/KyaFx.h` and
  `Private/Actors/KyaFx.cpp` own the class and behaviour codecs. The factory and
  exporter dispatch to them; the old structs/readers were removed from
  `KyaCameraStream.h`. The id-4 writer previously clobbered the leading count
  with `FxBehaviourValue`; it now rewrites the count.
- Capability/applicability: every typed field is exported; the unknown-list count
  is validated before writing. Unknown tails are preserved; edits use the shared
  placement patch path (refresh/undo).
- Inspector: Details groups are `FX`/`Behaviour`; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.FxFixtureRoundTrip` covers the class payload and behaviours
  2/3/4, byte-identical unedited export, five edited fields through
  export/reimport and unknown-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the 65 real FX actors;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 65 FX actors): 62 carry id 2 (4 bytes) and 3 carry
  id 3 (8 bytes); id 4 is fixture-covered.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. The class-level clamp on field 0x19C is runtime behaviour, not a
  serialization constraint, so the editor keeps the stored value.

## Findings: Companion

Status: **class and alert behaviour codecs extracted**.

- Source classes: `CActorCompanion` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor`) and `CBehaviourCompanion`.
- Factory registration: class 0x19 (`Types.h` `COMPANION`).
- `CActorCompanion::Create` (`src/b-witch/ActorCompanion.cpp:16`) calls
  `CActorAutonomous::Create` and reads three trailing words (field 0x358,
  material 0x35C, field 0x360). `BuildBehaviour` only adds the companion
  behaviour for `COMPANION_BEHAVIOUR_DEFAULT` (id 3); ids 0/1/2 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | autonomous prefix | weight f32, life u32, gravity f32, two dynamic-ext f32 |
  | fields | field 358 s32, material 35C s32, field 360 s32 |

  Bytes after field 360 remain raw in `ClassData`.
- Behaviour 3 payload (`CBehaviourCompanion::Create`): alert count s32 + one
  record per alert.

  | Section | Layout |
  | --- | --- |
  | conditions | count + count x 16-byte condition |
  | flags | flags u32, field 08 s32, field 10 s32, field 14 s32, field 18 f32 |
  | references | actor s32, vector field 20 xyz f32, camera s32 |
  | string list | count s32 + count x s32 |
  | text | level string key u64, field 50 f32, field 58 s32, field 5C f32 |
  | switch | `S_NTF_TARGET_STREAM_REF`: count + count x 28-byte target + 32-byte camera |
  | conditioned | count + blocks of operation/condition records (shared reader) |

- Editor model/reader/writer: `Public/Actors/KyaCompanion.h` and
  `Private/Actors/KyaCompanion.cpp` own the class and behaviour codecs; the
  shared conditioned/scenario readers moved to
  `Public/Actors/KyaScenarioCondition.h`. `CompanionLifeMax` is int64 and
  validated to 0..`MAX_uint32`. The factory and exporter dispatch to the module;
  the old structs/readers were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the alert count, each
  condition/string/switch/block count and the life range are validated before
  writing. Unknown tails are preserved; edits use the shared placement patch path
  (refresh/undo).
- Inspector: Details groups are `Companion` and `Companion alert`; empty groups
  hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.CompanionFixtureRoundTrip` covers the class payload and one
  alert (empty conditions/string list/switch/conditioned list), byte-identical
  unedited export, eight edited fields through export/reimport and alert-count
  rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real companion;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Companion): behaviour 3 with a 240-byte payload
  and a fully consumed 32-byte class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were recorded.
  Scenario variable/target ids are scalar without per-entry pickers.

## Findings: Gravity aware

Status: **class and fall/bell behaviour codecs extracted**.

- Source classes: `CActorGravityAware` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor`) and `CBehaviourGravityAwareFall` / `CBehaviourGravityAwareBell`.
- Factory registration: class 0x10 (`Types.h` `GRAVITY_AWARE`).
- `CActorGravityAware::Create` (`src/b-witch/ActorGravityAware.cpp:6`) calls
  `CActorAutonomous::Create`; the class adds no payload of its own, so the
  serialized class record is the 20-byte autonomous prefix (weight, life,
  gravity and two dynamic-extension floats).
- `BuildBehaviour`: 3 Fall, 4 Bell; ids 0/1/2 are inherited.
- Behaviour payloads:

  | Id | Source | Layout |
  | --- | --- | --- |
  | 3 | GravityAwareFall | sound reference s32 |
  | 4 | GravityAwareBell | sound reference s32, field C f32, `S_NTF_TARGET_STREAM_REF` switch record |

- Editor model/reader/writer: `Public/Actors/KyaGravity.h` and
  `Private/Actors/KyaGravity.cpp` own the class and behaviour codecs and reuse
  the shared `KyaReadSwitchValue`/`KyaWriteSwitchValue`. `GravityLifeMax` is
  int64 and validated to 0..`MAX_uint32`. The factory and exporter dispatch to
  the module; the old structs/readers were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the life range and
  the bell switch target count are validated before writing. Unknown tails are
  preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Gravity aware` and `Gravity behaviour`; empty
  groups hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.GravityFixtureRoundTrip` covers the class payload plus fall
  and bell behaviours (one bell switch target with camera), byte-identical
  unedited export, seven edited fields through export/reimport, life-range
  rejection and bell switch-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real gravity actor;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Gravity aware actor): behaviour 3 (4 bytes) and
  behaviour 4 (184 bytes).
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Sound/camera ids are scalar references without per-entry pickers.

## Findings: Rune

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorRune` (`CActor` -> `CObject`) and `CBehaviourRune`.
- Factory registration: class 0x51 (`Types.h` `RUNE`).
- `CActorRune::Create` (`src/b-witch/ActorRune.cpp:11`) calls `CActor::Create`,
  then reads a cinematic id, trigger distance, scenario-variable id, fx id and
  two material ids. `BuildBehaviour` only adds id 2 (`behaviourRune`), whose
  `Create` is empty; ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | cinematic | cinematic id s32 |
  | trigger | trigger distance f32 |
  | references | scenario variable s32, fx s32 |
  | materials | material id A s32, material id B s32 |

  Bytes after material B remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaRune.h` and
  `Private/Actors/KyaRune.cpp` own the class codec; the factory and exporter
  dispatch to it. The old struct/reader were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; no counts need
  guarding. Unknown tails are preserved; edits use the shared placement patch
  path (refresh/undo).
- Inspector: Details uses the `Rune` category; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.RuneFixtureRoundTrip` covers the full class payload and the
  empty id 2 behaviour, byte-identical unedited export and six edited fields
  through export/reimport. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real rune;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Rune): empty behaviour id 2 and a fully
  consumed 24-byte class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Cinematic/scenario/fx/material ids are scalar without per-entry
  pickers.

## Findings: Micken

Status: **class and eat behaviour codecs extracted**.

- Source classes: `CActorMicken` (`CActorAutonomous` -> `CActorMovable` ->
  `CActor`), `CBehaviourMickenEat`, `CBehaviourMickenKicked` and
  `CBehaviourMickenSquashed`.
- Factory registration: class 0x07 (`Types.h` `MICKEN`).
- `CActorMicken::Create` (`src/b-witch/ActorMicken.cpp:14`) calls
  `CActorAutonomous::Create`, reads a waypoint stream index, an actor stream
  index, field 0x358, field 0x35C and field 0x360.
- `BuildBehaviour` (`ActorMicken.cpp:170`): 4 Eat, 5/6 Kicked, 7/8 Squashed;
  ids 0/1/2 are inherited (id 2 is the shared catch-by-trap payload).
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | autonomous prefix | weight f32, life u32, gravity f32, two dynamic-ext f32 |
  | references | waypoint stream index s32, actor stream index s32 |
  | fields | field 358 s32, field 35C f32, field 360 s32 |

  Bytes after field 360 remain raw in `ClassData`.
- Behaviour 4 payload (`CBehaviourMickenEat::Create`): field 8 f32, field C f32,
  field 10 u32, path-follow index s32. Behaviours 5/6/7/8 have no `Create`
  override, so their payloads are empty.
- Editor model/reader/writer: `Public/Actors/KyaMicken.h` and
  `Private/Actors/KyaMicken.cpp` own the class and behaviour codecs.
  `MickenLifeMax` is int64 and validated to 0..`MAX_uint32`. The factory and
  exporter dispatch to the module; the old structs/readers were removed from
  `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the life range and
  the 16-byte eat payload size are validated before writing. Unknown tails are
  preserved; edits use the shared placement patch path (refresh/undo).
- Inspector: Details groups are `Micken` and `Micken behaviour`; empty groups
  hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.MickenFixtureRoundTrip` covers the full class payload plus
  eat behaviour 4 and an empty squashed record, byte-identical unedited export,
  seven edited fields through export/reimport and life-range rejection.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip` cover the
  real Micken actors; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 11 Micken actors): shapes are catch-by-trap id 2
  (4/24 bytes), eat id 4 (16 bytes) and empty ids 5/6.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Waypoint/actor/path ids are scalar references without per-entry
  pickers.

## Findings: Basic box

Status: **class codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorBasicBox` (`CActor` -> `CObject`) and
  `CBehaviourBasicBoxStand`.
- Factory registration: class 0x39 (`Types.h` `BASIC_BOX`).
- `CActorBasicBox::Create` (`src/b-witch/ActorBasicBox.cpp:18`) calls
  `CActor::Create`, reads field 0x160, a life value, an `S_ACTOR_STREAM_REF`
  actor list, four vibration floats, field 0x180, four words, one switch record
  and a `CAddOnGenerator`. `BuildBehaviour` only adds id 2
  (`CBehaviourBasicBoxStand`), whose `Create` is empty; ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | header | field 160 s32, life u32 |
  | actors | count u32 + count x 4-byte actor refs |
  | vibration | fields 170/174/178/17C f32 |
  | fields | field 180 u32, fields 18C/190/198/194 s32 |
  | switch | `S_NTF_TARGET_STREAM_REF`: count u32 + count x 28-byte target + 32-byte camera |
  | add-on | 20 bytes (`CAddOnGenerator`); sub-object mode u32, then five f32 when 1 |

  Bytes after the add-on remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaBasicBox.h` and
  `Private/Actors/KyaBasicBox.cpp` own the class codec and reuse the shared
  `KyaReadSwitchValue`/`KyaWriteSwitchValue`. `BasicBoxLifeMax` is int64 and
  validated to 0..`MAX_uint32`. The factory and exporter dispatch to the module;
  the old struct/reader were removed from `KyaCameraStream.h`.
- Capability/applicability: every typed field is exported; the life range,
  actor-ref count, switch target count and add-on mode are validated before
  writing. Unknown tails are preserved; edits use the shared placement patch
  path (refresh/undo).
- Inspector: Details uses the `Basic box` category; empty groups hide through
  `EditConditionHides(true)`.
- Tests: `Kya.Bank.BasicBoxFixtureRoundTrip` covers a populated class payload
  (one actor ref, one switch target, mode-1 add-on) and the empty id 2
  behaviour, byte-identical unedited export, eight edited fields through
  export/reimport and actor-ref-count rejection.
  `Kya.Bank.LevelOneRoundTrip` and `Kya.Bank.SelectedActorRoundTrip` cover the
  118 real basic boxes; `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 118 Basic box actors): every actor carries only
  the empty stand behaviour id 2 with a fully consumed class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Actor ids are scalar references without per-entry pickers.

## Findings: Nativ command

Status: **class/add-on codec extracted; behaviour id 2 audited as empty**.

- Source classes: `CActorNativCmd` (`CActor` -> `CObject`) and
  `CBehaviourNativCmdStand`.
- Factory registration: class 0x3C (`Types.h` `NATIV_CMD`).
- `CActorNativCmd::Create` (`src/b-witch/ActorNativCmd.cpp:11`) calls
  `CActor::Create`, reads an `S_ACTOR_STREAM_REF` nativ list, field 0x164 and the
  four `CAddOnNativ` blocks A-D (`CAddOnNativ::Create`: sub-object count plus
  `CAddOnSubObj` records). `BuildBehaviour` only adds id 2
  (`CBehaviourNativCmdStand`), whose `Create` is empty; ids 0/1 are inherited.
- Class payload layout (relative to the first class-specific byte):

  | Section | Layout |
  | --- | --- |
  | nativs | count u32 + count x 4-byte actor refs |
  | field 164 | s32 |
  | add-ons | four blocks (A-D), each: sub-object count u32 + sub-objects |

  Each sub-object is field 0 u32, cinematic count u32, then the ids. Bytes after
  block D remain raw in `ClassData`.
- Editor model/reader/writer: `Public/Actors/KyaNativCmd.h` and
  `Private/Actors/KyaNativCmd.cpp` own the class codec. The factory and exporter
  dispatch to the module; the old struct/reader and inline writer were removed
  from `KyaCameraStream.h`/the exporter.
- Capability/applicability: every typed field is exported; the actor-ref count,
  four add-on counts and each sub-object cinematic count are validated before
  writing. Unknown tails are preserved; edits use the shared placement patch path
  (refresh/undo).
- Inspector: Details groups are `Nativ command` / `Nativ command add-on`; empty
  groups hide through `EditConditionHides(true)`.
- Tests: `Kya.Bank.NativCmdFixtureRoundTrip` covers the actor list, field 164 and
  four add-on blocks (populated A/C, empty B/D) plus the empty id 2 behaviour,
  byte-identical unedited export, five edited fields through export/reimport and
  cinematic-count rejection. `Kya.Bank.LevelOneRoundTrip` and
  `Kya.Bank.SelectedActorRoundTrip` cover the real Nativ command;
  `Kya.Editor.MilestoneCGate` audits exported fields.
- Real-bank evidence (LEVEL_1, 1 Nativ command): empty behaviour id 2 and a fully
  consumed class payload.
- Remaining exclusions: Ghidra is unavailable, so no PS2 addresses were
  recorded. Actor/cinematic ids are scalar references without per-entry pickers.

## File organization

Status: **shared registry and per-family metadata modules implemented; byte
readers are migrating family by family**.

- [x] Establish shared codec/model interfaces and preserve reflected names and
  serialized asset compatibility before moving existing declarations.
  `KyaActorFamily.h` defines the shared family/behaviour/reference types and
  `EKyaReferenceKind`; `KyaActorCatalog.cpp` now delegates class, behaviour and
  reference lookup to `KyaActorFamilies`, while every reflected property and
  bank byte contract is unchanged.
- [x] Extract shared prefixes and payload helpers from the large bank header and
  importer/writer implementations without changing their byte contracts.
  Hedgehog, Projectile, Moving platform, Switch, Wind, Trap, Weapon, Araignos,
  Bonus, Teleporter, Amortos, DCA, Fruit, Nativ, Wolfen, Commander, Kim, Boomy,
  Aton, Rope, Stiller, Ambre, Bonus fountain, Checkpoints, Bridge, Fog, Shoot,
  Money, Box, FX, Clusteriser, Companion, Gravity aware, Rune, Micken, Basic
  box and Nativ command are fully extracted into their own modules; the shared
  Fighter prefix, switch-script, actor-camera, conditioned/scenario,
  catch-by-trap and hero-unknown helpers live in `Actors/KyaFighter.h`,
  `Actors/KyaSwitchScript.h`, `Actors/KyaActorCamera.h`,
  `Actors/KyaScenarioCondition.h`, `Actors/KyaCatchByTrap.h` and
  `Actors/KyaHeroUnknown.h`. No actor-family byte codec remains in
  `KyaCameraStream.h` or the factory/exporter.
- [x] Migrate existing families incrementally, validating each move before adding
  missing layouts. One module per serialized class now exists under
  `Source/KyaLevel/Private/Actors/` for the 36 classes present in LEVEL_1 plus
  Hedgehog; each module records whether its byte codec has moved.
- [x] Add missing families using the same interfaces, starting with Hedgehog.
  Hedgehog is implemented end to end and covered by
  `Kya.Bank.HedgehogFixtureRoundTrip`.

### Family module layout

- Shared types and registry: `Source/KyaLevel/Public/KyaActorFamily.h`,
  `Source/KyaLevel/Private/Actors/KyaActorFamilyRegistry.cpp`.
- One metadata module per serialized class: `Kya*ActorFamily.cpp` in
  `Source/KyaLevel/Private/Actors/`.
- Family codecs are exported from `Source/KyaLevel/Public/Actors/` and implemented
  under `Source/KyaLevel/Private/Actors/`. Hedgehog, Projectile, Moving platform,
  Switch, Wind, Trap, Weapon, Araignos, Bonus, Teleporter, Amortos, DCA, Fruit,
  Nativ, Wolfen, Commander, Kim, Boomy, Aton, Rope, Stiller, Ambre, Bonus
  fountain, Checkpoints, Bridge, Fog, Shoot, Money, Box, FX, Clusteriser,
  Companion, Gravity aware, Rune, Micken, Basic box and Nativ command are the
  reference examples;
  Projectile
  also moved inventory and per-behaviour codecs, Moving platform moved
  prefix/details/camera, Switch moved the target-switch and behaviour codecs,
  Wind moved its tuning/switch block, Trap moved its class/trap-stream/behaviour
  codecs, Weapon moved its burst/pistol/sniper/RPG codecs, Araignos moved its
  class/add-on codec, Bonus moved its class and turn/path/flock/add-on codecs,
  Teleporter moved its class/switch/waypoint/condition codec, Amortos moved its
  class actor-stream/add-on/switch codec, DCA moved its class/fire-shot/
  controlled/trajectory codec, Fruit moved its class and squashed/inventory/
  bonus codecs, Nativ moved its class and take-and-put codec, Wolfen moved its
  class/fight-parameter and twelve behaviour codecs, Commander moved its class
  and behaviour 2 codec, Kim moved its class and camera codec, Boomy moved
  its class/camera/type codec, Aton moved its class/path/add-on codec, Rope
  moved its class/switch codec, Stiller moved its class/switch/add-on codec,
  Ambre moved its class/switch codec, Bonus fountain moved its add-on codec,
  Checkpoints moved its record/pair codec, Bridge moved its class plus
  basic/wind-aware behaviour codecs, Fog moved its hierarchy/fog-zone codec,
  Shoot moved its class/add-on and fire/wave codecs, Money moved its class and
  flock/add-on behaviour codecs, Box moved its class/switch/add-on codec, FX
  moved its class and single/path/unknown behaviour codecs, Clusteriser moved
  its zone-list codec, Companion moved its class and alert behaviour codecs, and
  Gravity aware moved its class and fall/bell behaviour codecs, Rune moved its
  class codec, Micken moved its class and eat behaviour codecs, Basic box moved
  its class codec, and Nativ command moved its class/add-on codec.
- The catalog, pickers and tests read the registry; the factory/exporter remain
  the centralized dispatch points until each byte reader moves.

### Inventory evidence used for this update

Run from `unreal/` with the engine commandlet:

```powershell
UnrealEditor-Cmd.exe BankTest.uproject -run=pythonscript `
  -script=KyaLevelEditor/Tools/inventory_actor_classes.py -unattended -NullRHI
```

Result for LEVEL_1: 616 actors, 36 classes, every class payload fully consumed
(`withRaw=0`; this does not prove behaviour payload coverage), classes 0x05-0x52,
and no class 0x22.

Projectile behaviour inventory (same bank): 26 class-0x1c actors carrying ids
1/3/4/5/9/11 only, with payload sizes 4/12/20/12/44/12 and typed coverage
cinematic/projectile/inventory. Ids 10, 12 and 13 are source-derived and
fixture-covered but absent from LEVEL_1. Reproduce with
`KyaLevelEditor/Tools/inventory_actor_behaviours.py` and
`KYA_ACTOR_CLASS=0x1c`.

Moving-platform inventory (same bank): 166 class-0x0d actors carrying ids
0/1/2/3/4/5/6/7/8/9 with payload sizes 0/4/24/56-or-84/8/96/24/8/100/24; no
id 10. Reproduce with `KYA_ACTOR_CLASS=0x0d`.

Switch inventory (same bank): 17 class-0x11 actors carrying ids 2/3/5/6 with
payload sizes 8/76/12/12; ids 4, 7 and 8 are fixture-covered. Reproduce with
`KYA_ACTOR_CLASS=0x11`.

Wind inventory (same bank): 25 class-0x12 actors, all with empty behaviour id 2
and a fully consumed class payload. Reproduce with `KYA_ACTOR_CLASS=0x12`.

Trap inventory (same bank): 24 class-0x2c actors carrying id 2 (24) and id 3
(1), both 12-byte payloads. Reproduce with `KYA_ACTOR_CLASS=0x2c`.

Weapon inventory (same bank): 18 class-0x20 actors carrying id 0/2 (empty),
id 3 (15), id 4 (2) and id 6 (1). Reproduce with `KYA_ACTOR_CLASS=0x20`.

Araignos inventory (same bank): 39 class-0x21 actors, all with empty behaviour
id 2 and a fully consumed class payload. Reproduce with
`KYA_ACTOR_CLASS=0x21`.

Bonus inventory (same bank): 16 class-0x0e actors carrying id 3 (1), id 5 (15)
and id 6 (1); id 4 is fixture-covered. Reproduce with `KYA_ACTOR_CLASS=0x0e`.

Teleporter inventory (same bank): 5 class-0x24 actors, all with empty behaviour
id 2 and a fully consumed class payload. Reproduce with `KYA_ACTOR_CLASS=0x24`.

Amortos inventory (same bank): 6 class-0x29 actors; two carry the 4-byte
cinematic id 1 and six carry the empty id 2. Reproduce with
`KYA_ACTOR_CLASS=0x29`.

DCA inventory (same bank): 4 class-0x1b actors carrying ids 0/1/2/3/5, with
payload sizes 0/4/0/40/12; no id 4. Reproduce with `KYA_ACTOR_CLASS=0x1b`.

Fruit inventory (same bank): 9 class-0x16 actors carrying ids 1/3/4/5/6 with
payload sizes 4/0/4/44/8; no id 7. Reproduce with `KYA_ACTOR_CLASS=0x16`.

Nativ inventory (same bank): 7 class-0x0a actors carrying ids 0/1/2/4/6 with
payload sizes 0/4/4/0/20. Reproduce with `KYA_ACTOR_CLASS=0x0a`.

Wolfen inventory (same bank): 22 class-0x09 actors with fighter ids 0/1/3/4/5/6
and Wolfen behaviour ids 8/10/13/14/15/17/18/19/21/22/23/24. Escape (13) payloads
are 72/80 bytes, WatchDog (8) 80/200 and TrackWeapon (17) 48/68. Reproduce with
`KYA_ACTOR_CLASS=0x09`.

Commander inventory (same bank): 8 class-0x13 actors with behaviour 1 (4 bytes)
and behaviour 2 at 44 bytes (7) or 72 bytes (1). Reproduce with
`KYA_ACTOR_CLASS=0x13`.

Kim inventory (same bank): 1 class-0x06 actor with behaviour 1 (4 bytes), empty
3/4/7 and HeroUnknown 9/10/11 (28 bytes each). Class 0x0b is Electrolla and has
no LEVEL_1 actors. Reproduce with `KYA_ACTOR_CLASS=0x06`.

Boomy inventory (same bank): 1 class-0x05 actor with empty behaviour id 2 and a
fully consumed class payload. Reproduce with `KYA_ACTOR_CLASS=0x05`.

Aton inventory (same bank): 1 class-0x38 actor with behaviour 1 (4 bytes) and
empty id 3. Reproduce with `KYA_ACTOR_CLASS=0x38`.

Rope inventory (same bank): 1 class-0x0f actor with empty behaviour id 2 and a
fully consumed class payload. Reproduce with `KYA_ACTOR_CLASS=0x0f`.

Stiller inventory (same bank): 2 class-0x4c actors, both with empty behaviour
id 2 and fully consumed class payloads. Reproduce with `KYA_ACTOR_CLASS=0x4c`.

Ambre inventory (same bank): 2 class-0x14 actors, both with empty behaviour id 2
and fully consumed class payloads. Reproduce with `KYA_ACTOR_CLASS=0x14`.

Bonus fountain inventory (same bank): 1 class-0x52 actor with empty behaviour
id 2 and a fully consumed add-on payload. Reproduce with `KYA_ACTOR_CLASS=0x52`.

Checkpoint inventory (same bank): 2 class-0x1e actors, both with empty behaviour
id 2 and fully consumed checkpoint records. Reproduce with `KYA_ACTOR_CLASS=0x1e`.

Bridge inventory (same bank): 1 class-0x17 actor carrying only the 32-byte basic
behaviour id 2. Reproduce with `KYA_ACTOR_CLASS=0x17`.

Fog inventory (same bank): 3 class-0x3d actors, each carrying a 4-byte fog-zone
behaviour id 2 (zero zones). Reproduce with `KYA_ACTOR_CLASS=0x3d`.

Shoot inventory (same bank): 3 class-0x3e actors, each carrying the 28-byte fire
behaviour id 3. Reproduce with `KYA_ACTOR_CLASS=0x3e`.

Money inventory (same bank): 3 class-0x32 actors, each carrying the empty
add-on behaviour id 3. Reproduce with `KYA_ACTOR_CLASS=0x32`.

Box inventory (same bank): 4 class-0x35 actors, each carrying the empty box-stand
behaviour id 3. Reproduce with `KYA_ACTOR_CLASS=0x35`.

FX inventory (same bank): 65 class-0x2e actors; 62 carry the 4-byte single
behaviour id 2 and 3 carry the 8-byte path behaviour id 3. Reproduce with
`KYA_ACTOR_CLASS=0x2e`.

Clusteriser inventory (same bank): 1 class-0x18 actor carrying the 172-byte
cluster-zone behaviour id 2. Reproduce with `KYA_ACTOR_CLASS=0x18`.

Companion inventory (same bank): 1 class-0x19 actor carrying the 240-byte alert
behaviour id 3. Reproduce with `KYA_ACTOR_CLASS=0x19`.

Gravity-aware inventory (same bank): 1 class-0x10 actor carrying behaviour 3
(4 bytes) and behaviour 4 (184 bytes). Reproduce with `KYA_ACTOR_CLASS=0x10`.

Rune inventory (same bank): 1 class-0x51 actor carrying the empty behaviour id 2.
Reproduce with `KYA_ACTOR_CLASS=0x51`.

Micken inventory (same bank): 11 class-0x07 actors with catch-by-trap id 2
(4/24 bytes), eat id 4 (16 bytes) and empty ids 5/6. Reproduce with
`KYA_ACTOR_CLASS=0x07`.

Basic box inventory (same bank): 118 class-0x39 actors, each carrying the empty
stand behaviour id 2. Reproduce with `KYA_ACTOR_CLASS=0x39`.

Nativ command inventory (same bank): 1 class-0x3c actor carrying the empty
stand behaviour id 2. Reproduce with `KYA_ACTOR_CLASS=0x3c`.


### Cross-bank coverage (all 16 LEVEL banks)

LEVEL_1 alone is fully consumed (`withRaw=0`), but the tables above cover the
whole `src/` hierarchy, and other banks instantiate classes that LEVEL_1 does
not. Running `inventory_actor_classes.py` against every `assets/CDEURO/LEVEL/*`
bank gives the full serialized set: 86 distinct class ids. 13 of them still
produce nonempty `ClassData`; none is fully unparsed any more. 0x3b keeps a
typed header with a raw manager tail, and every other id
(0x1d/0x26/0x30/0x34/0x36/0x3f/0x45/0x47/0x4b/0x4e/0x4f/0x55) is fully typed:

| Class | Example actor | Max raw | Banks | Finding |
| --- | --- | --- | --- | --- |
| 0x1d | EXPLOSIVE_DISTRIBUTOR | 124 | 10,2,3,5,6,7,8 | module-backed (Findings: Explosive distributor) |
| 0x26 | NOZEMONSTER | 304 | 2 | module-backed (Findings: Nose monster) |
| 0x30 | PUNCHINGBALL | 728 | 4,NATIV | module-backed (Findings: Punching ball) |
| 0x34 | NATIV_SHOP | 76 | 4,NATIV | module-backed (Findings: Nativ shop) |
| 0x36 | HUNTERSHIP_01 (Ship) | 244 | 3 | module-backed (Findings: Ship) |
| 0x3b | EVENTGENERATOR | 68 | 10,12,13,2,3,4,5,6,7,8,9 | header typed, managers raw (Findings: Event generator) |
| 0x3f | LINER (Pattern controller) | 84 | 5 | module-backed (Findings: Pattern controller) |
| 0x45 | MINIGAMESTIMEATTACK | 180 | NATIV | module-backed (Findings: Minigame family) |
| 0x47 | MINIGAMEBOOMY | 3988 | NATIV | module-backed (Findings: Minigame family) |
| 0x4b | SHOCKER | 184 | 10,2,3,7 | module-backed (Findings: Shocker) |
| 0x4e | KRONOS_BUNCH | 3960 | 13,9 | module-backed (Findings: Bunch) |
| 0x4f | BRAZUL | 3004 | 11,12,13 | module-backed (Findings: Brazul) |
| 0x55 | MINIGAMEDISTANCE | 176 | NATIV | module-backed (Findings: Minigame family) |

The remaining class ids have empty class payloads (`withRaw=0`); their
behaviours still need the same per-behaviour inventory check. The actor and
behaviour checklists therefore stay unchecked for every type that is not one of
the 37 audited LEVEL_1/Hedgehog families until these banks are covered, and the
LEVEL_1 "fully consumed" statement must not be read as game-wide coverage.

Reproduce the per-bank scan with `KYA_ACTOR_BANK=<bank>` and
`Tools/inventory_actor_classes.py` (one line per class id, including
`withRaw`/`maxRaw`).

## Finding template

For each family, record: source classes; factory and behaviour IDs; inherited
contracts; `Create`/helper locations and PS2 addresses; editor model/reader/writer
paths; per-field applicability; no-payload cases; tests and real-bank evidence;
remaining gaps and disposition. Link the finding from the relevant inventory rows.

## Inventory maintenance

The tables below were discovered from `src/` declarations with inheritance closure,
not from the subset instantiated by a bank. Nested names are qualified to avoid
merging unrelated `CBhvDefault` classes. Discovery includes headers and source files.

Run `python KyaLevelEditor/Tools/inventory_source_hierarchy.py` from `unreal/`
to print a fresh inventory. Compare its type names with this maintained checklist;
do not overwrite audit progress with the generated unchecked rows. The scanner
strips comments and strings but is not a C++ parser. Cross-check factory dispatch,
macro-generated types and any reported unresolved/duplicate bases before declaring
the audit exhaustive. Line links may drift as source changes.

Source inventory: 77 actor definitions and 223 behaviour definitions (including roots and shared/abstract bases).

Reconciled inventory status (implementation recorded; verification still pending):

| Inventory | `[ ]` Incomplete | `[~]` Awaiting verification | `[x]` Verified | Total |
| --- | ---: | ---: | ---: | ---: |
| Actors | 21 | 56 | 0 | 77 |
| Behaviours | 68 | 155 | 0 | 223 |
| **Total** | **89** | **211** | **0** | **300** |

Shared/root rows such as CActor, CActorAutonomous, CBehaviourStand and
CBehaviourCatchByTrap remain incomplete where only a subset of descendant
applicability has been reconciled. A consumer's `[~]` does not close that shared
contract or imply that its entire family is implemented. Preserve all three
status symbols when comparing or refreshing the generated inventory.

## Actor checklist

Legend: `[x]` = audited and module-backed; `[~]` = implementation or
empty-payload disposition recorded, verification still open; `[ ]` = not yet
audited (source-only type or still-raw cross-bank class).

| Status | Source type | Immediate base(s) | Declaration |
| --- | --- | --- | --- |
| [~] | `CActor` | `CObject` | [src/b-witch/Actor.h:515](../../src/b-witch/Actor.h#L515) |
| [~] | `CActorCinematic` | `CActor` | [src/b-witch/Actor_Cinematic.h:38](../../src/b-witch/Actor_Cinematic.h#L38) |
| [~] | `CActorAcceleratos` | `CActor` | [src/b-witch/ActorAcceleratos.h:42](../../src/b-witch/ActorAcceleratos.h#L42) |
| [~] | `CActorAddOnPurchase` | `CActor` | [src/b-witch/ActorAddOnPurchase.h:19](../../src/b-witch/ActorAddOnPurchase.h#L19) |
| [~] | `CActorAmbre` | `CActor` | [src/b-witch/ActorAmbre.h:42](../../src/b-witch/ActorAmbre.h#L42) |
| [~] | `CActorAmortos` | `CActor` | [src/b-witch/ActorAmortos.h:55](../../src/b-witch/ActorAmortos.h#L55) |
| [~] | `CActorAmortosCmd` | `CActor` | [src/b-witch/ActorAmortosCmd.h:35](../../src/b-witch/ActorAmortosCmd.h#L35) |
| [~] | `CActorAraignos` | `CActorMovable` | [src/b-witch/ActorAraignos.h:16](../../src/b-witch/ActorAraignos.h#L16) |
| [~] | `CActorAton` | `CActorAutonomous` | [src/b-witch/ActorAton.h:74](../../src/b-witch/ActorAton.h#L74) |
| [~] | `CActorAutonomous` | `CActorMovable` | [src/b-witch/ActorAutonomous.h:112](../../src/b-witch/ActorAutonomous.h#L112) |
| [~] | `CActorBasicBox` | `CActor` | [src/b-witch/ActorBasicBox.h:38](../../src/b-witch/ActorBasicBox.h#L38) |
| [~] | `CActorBlazer` | `CActorAutonomous` | [src/b-witch/ActorBlazer.h:43](../../src/b-witch/ActorBlazer.h#L43) |
| [~] | `CActorBombLauncher` | `CActor` | [src/b-witch/ActorBombLauncher.h:33](../../src/b-witch/ActorBombLauncher.h#L33) |
| [~] | `CActorBonus` | `CActorMovable` | [src/b-witch/ActorBonus.h:183](../../src/b-witch/ActorBonus.h#L183) |
| [~] | `CActorBonusFountain` | `CActor` | [src/b-witch/ActorBonusFountain.h:25](../../src/b-witch/ActorBonusFountain.h#L25) |
| [~] | `CActorBoomy` | `CActorMovable` | [src/b-witch/ActorBoomy.h:51](../../src/b-witch/ActorBoomy.h#L51) |
| [~] | `CActorBox` | `CActorAutonomous` | [src/b-witch/ActorBox.h:40](../../src/b-witch/ActorBox.h#L40) |
| [~] | `CActorBrazul` | `CActorWolfen` | [src/b-witch/ActorBrazul.h:142](../../src/b-witch/ActorBrazul.h#L142) |
| [~] | `CActorBridge` | `CActor` | [src/b-witch/ActorBridge.h:177](../../src/b-witch/ActorBridge.h#L177) |
| [~] | `CActorBunch` | `CActorWolfen` | [src/b-witch/ActorBunch.h:183](../../src/b-witch/ActorBunch.h#L183) |
| [~] | `CActorCareBoy` | `CActorMovable` | [src/b-witch/ActorCareBoy.h:33](../../src/b-witch/ActorCareBoy.h#L33) |
| [~] | `CActorCheckpointManager` | `CActor` | [src/b-witch/ActorCheckpointManager.h:34](../../src/b-witch/ActorCheckpointManager.h#L34) |
| [~] | `CActorClusteriser` | `CActor` | [src/b-witch/ActorClusteriser.h:78](../../src/b-witch/ActorClusteriser.h#L78) |
| [~] | `CActorCommander` | `CActor` | [src/b-witch/ActorCommander.h:81](../../src/b-witch/ActorCommander.h#L81) |
| [~] | `CActorCompanion` | `CActorAutonomous` | [src/b-witch/ActorCompanion.h:116](../../src/b-witch/ActorCompanion.h#L116) |
| [~] | `CActorCredits` | `CActor` | [src/b-witch/ActorCredits.h:35](../../src/b-witch/ActorCredits.h#L35) |
| [~] | `CActorDCA` | `CActor` | [src/b-witch/ActorDCA.h:30](../../src/b-witch/ActorDCA.h#L30) |
| [~] | `CActorEgg` | `CActorMovable` | [src/b-witch/ActorEgg.h:33](../../src/b-witch/ActorEgg.h#L33) |
| [~] | `CActorElectrolla` | `CActor` | [src/b-witch/ActorElectrolla.h:33](../../src/b-witch/ActorElectrolla.h#L33) |
| [~] | `CActorEventGenerator` | `CActor` | [src/b-witch/ActorEventGenerator.h:159](../../src/b-witch/ActorEventGenerator.h#L159) |
| [~] | `CActorExplosiveDistributor` | `CActor` | [src/b-witch/ActorExplosiveDistributor.h:29](../../src/b-witch/ActorExplosiveDistributor.h#L29) |
| [~] | `CActorFighter` | `CActorAutonomous` | [src/b-witch/ActorFighter.h:567](../../src/b-witch/ActorFighter.h#L567) |
| [~] | `CActorFogManager` | `CActor` | [src/b-witch/ActorFogManager.h:67](../../src/b-witch/ActorFogManager.h#L67) |
| [~] | `CActorFruit` | `CActorAutonomous` | [src/b-witch/ActorFruit.h:63](../../src/b-witch/ActorFruit.h#L63) |
| [~] | `CActorFx` | `CActor` | [src/b-witch/ActorFx.h:14](../../src/b-witch/ActorFx.h#L14) |
| [~] | `CActorGravityAware` | `CActorAutonomous` | [src/b-witch/ActorGravityAware.h:47](../../src/b-witch/ActorGravityAware.h#L47) |
| [~] | `CActorHedgehog` | `CActorAutonomous` | [src/b-witch/ActorHedgehog.h:72](../../src/b-witch/ActorHedgehog.h#L72) |
| [~] | `CActorHelperSign` | `CActor` | [src/b-witch/ActorHelperSign.h:82](../../src/b-witch/ActorHelperSign.h#L82) |
| [~] | `CActorHero` | `CActorFighter` | [src/b-witch/ActorHero.h:336](../../src/b-witch/ActorHero.h#L336) |
| [~] | `CActorHeroPrivate` | `CActorHero` | [src/b-witch/ActorHero_Private.h:220](../../src/b-witch/ActorHero_Private.h#L220) |
| [~] | `CActorHunter` | `CActorAutonomous` | [src/b-witch/ActorHunter.h:33](../../src/b-witch/ActorHunter.h#L33) |
| [~] | `CActorJamGut` | `CActorAutonomous` | [src/b-witch/ActorJamGut.h:101](../../src/b-witch/ActorJamGut.h#L101) |
| [~] | `CActorMicken` | `CActorAutonomous` | [src/b-witch/ActorMicken.h:76](../../src/b-witch/ActorMicken.h#L76) |
| [~] | `CActorMiniGame` | `CActor` | [src/b-witch/ActorMiniGame.h:121](../../src/b-witch/ActorMiniGame.h#L121) |
| [~] | `CActorMiniGameBoomy` | `CActorMiniGame` | [src/b-witch/ActorMiniGameBoomy.h:61](../../src/b-witch/ActorMiniGameBoomy.h#L61) |
| [~] | `CActorMiniGameBoxCounter` | `CActorMiniGame` | [src/b-witch/ActorMiniGameBoxCounter.h:37](../../src/b-witch/ActorMiniGameBoxCounter.h#L37) |
| [~] | `CActorMiniGameDistance` | `CActorMiniGame` | [src/b-witch/ActorMiniGameDistance.h:38](../../src/b-witch/ActorMiniGameDistance.h#L38) |
| [~] | `CActorMiniGamesManager` | `CActor` | [src/b-witch/ActorMiniGamesManager.h:35](../../src/b-witch/ActorMiniGamesManager.h#L35) |
| [~] | `CActorMiniGamesOrganizer` | `CActor` | [src/b-witch/ActorMiniGamesOrganizer.h:39](../../src/b-witch/ActorMiniGamesOrganizer.h#L39) |
| [~] | `CActorMiniGameTimeAttack` | `CActorMiniGame` | [src/b-witch/ActorMiniGameTimeAttack.h:37](../../src/b-witch/ActorMiniGameTimeAttack.h#L37) |
| [~] | `CActorMoney` | `CActorMovable` | [src/b-witch/ActorMoney.h:101](../../src/b-witch/ActorMoney.h#L101) |
| [~] | `CActorMovable` | `CActor` | [src/b-witch/ActorMovable.h:140](../../src/b-witch/ActorMovable.h#L140) |
| [~] | `CActorMovingPlatform` | `CActorMovable` | [src/b-witch/ActorMovingPlatform.h:343](../../src/b-witch/ActorMovingPlatform.h#L343) |
| [~] | `CActorNativ` | `CActorAutonomous` | [src/b-witch/ActorNativ.h:360](../../src/b-witch/ActorNativ.h#L360) |
| [~] | `CActorNativCmd` | `CActor` | [src/b-witch/ActorNativCmd.h:86](../../src/b-witch/ActorNativCmd.h#L86) |
| [~] | `CActorNativShop` | `CActor` | [src/b-witch/ActorNativShop.h:106](../../src/b-witch/ActorNativShop.h#L106) |
| [~] | `CActorNoseMonster` | `CActorAutonomous` | [src/b-witch/ActorNoseMonster.h:74](../../src/b-witch/ActorNoseMonster.h#L74) |
| [~] | `CActorPattern` | `CActorAutonomous` | [src/b-witch/ActorPattern.h:74](../../src/b-witch/ActorPattern.h#L74) |
| [~] | `CActorPatternCmd` | `CActor` | [src/b-witch/ActorPatternCmd.h:40](../../src/b-witch/ActorPatternCmd.h#L40) |
| [~] | `CActorProjectile` | `CActorAutonomous` | [src/b-witch/ActorProjectile.h:181](../../src/b-witch/ActorProjectile.h#L181) |
| [~] | `CActorPunchingBall` | `CActorFighter` | [src/b-witch/ActorPunchingBall.h:68](../../src/b-witch/ActorPunchingBall.h#L68) |
| [~] | `CActorRope` | `CActor` | [src/b-witch/ActorRope.h:25](../../src/b-witch/ActorRope.h#L25) |
| [~] | `CActorRune` | `CActor` | [src/b-witch/ActorRune.h:27](../../src/b-witch/ActorRune.h#L27) |
| [~] | `CActorShip` | `CActor` | [src/b-witch/ActorShip.h:64](../../src/b-witch/ActorShip.h#L64) |
| [~] | `CActorShocker` | `CActorAutonomous` | [src/b-witch/ActorShocker.h:60](../../src/b-witch/ActorShocker.h#L60) |
| [~] | `CActorShockerCmd` | `CActor` | [src/b-witch/ActorShockerCmd.h:42](../../src/b-witch/ActorShockerCmd.h#L42) |
| [~] | `CActorShoot` | `CActorAutonomous` | [src/b-witch/ActorShoot.h:68](../../src/b-witch/ActorShoot.h#L68) |
| [~] | `CActorStiller` | `CActor` | [src/b-witch/ActorStiller.h:49](../../src/b-witch/ActorStiller.h#L49) |
| [~] | `CActorSwitch` | `CActor` | [src/b-witch/ActorSwitch.h:237](../../src/b-witch/ActorSwitch.h#L237) |
| [~] | `CActorTeleporter` | `CActor` | [src/b-witch/ActorTeleporter.h:56](../../src/b-witch/ActorTeleporter.h#L56) |
| [~] | `CActorTrap` | `CActorMovable` | [src/b-witch/ActorTrap.h:90](../../src/b-witch/ActorTrap.h#L90) |
| [~] | `CActorWantedZoo` | `CActor` | [src/b-witch/ActorWantedZoo.h:23](../../src/b-witch/ActorWantedZoo.h#L23) |
| [~] | `CActorWeapon` | `CActorMovable` | [src/b-witch/ActorWeapon.h:192](../../src/b-witch/ActorWeapon.h#L192) |
| [~] | `CActorWind` | `CActor` | [src/b-witch/ActorWind.h:313](../../src/b-witch/ActorWind.h#L313) |
| [~] | `CActorWolfen` | `CActorFighter` | [src/b-witch/ActorWolfen.h:1096](../../src/b-witch/ActorWolfen.h#L1096) |
| [~] | `CActorWoodMonster` | `CActor` | [src/b-witch/ActorWoodMonster.h:47](../../src/b-witch/ActorWoodMonster.h#L47) |
| [~] | `CActorWoof` | `CActorAutonomous` | [src/b-witch/ActorWoof.h:68](../../src/b-witch/ActorWoof.h#L68) |

## Behaviour checklist

Legend: `[~]` = implementation or empty-payload disposition recorded,
verification still open; `[ ]` = not yet audited (source-only behaviour or a
still-raw cross-bank class).

| Status | Source type | Immediate base(s) | Declaration |
| --- | --- | --- | --- |
| [~] | `CBehaviour` | � | [src/b-witch/Actor.h:225](../../src/b-witch/Actor.h#L225) |
| [~] | `CBehaviourInactive` | `CBehaviour` | [src/b-witch/Actor.h:247](../../src/b-witch/Actor.h#L247) |
| [~] | `CBehaviourStand` | `CBehaviour` | [src/b-witch/Actor.h:263](../../src/b-witch/Actor.h#L263) |
| [~] | `CBehaviourCinematic` | `CBehaviour` | [src/b-witch/Actor_Cinematic.h:15](../../src/b-witch/Actor_Cinematic.h#L15) |
| [~] | `CBehaviourAcceleratos` | `CBehaviour` | [src/b-witch/ActorAcceleratos.h:12](../../src/b-witch/ActorAcceleratos.h#L12) |
| [~] | `CBehaviourAcceleratosStand` | `CBehaviourAcceleratos` | [src/b-witch/ActorAcceleratos.h:21](../../src/b-witch/ActorAcceleratos.h#L21) |
| [~] | `CBhvAddOnPurchase` | `CBehaviour` | [src/b-witch/ActorAddOnPurchase.h:10](../../src/b-witch/ActorAddOnPurchase.h#L10) |
| [~] | `CBehaviourAmbre` | `CBehaviour` | [src/b-witch/ActorAmbre.h:16](../../src/b-witch/ActorAmbre.h#L16) |
| [~] | `CBehaviourAmbreUnknown` | `CBehaviourAmbre` | [src/b-witch/ActorAmbre.h:29](../../src/b-witch/ActorAmbre.h#L29) |
| [~] | `CBehaviourAmortos` | `CBehaviour` | [src/b-witch/ActorAmortos.h:33](../../src/b-witch/ActorAmortos.h#L33) |
| [~] | `CBehaviourAmortosStand` | `CBehaviourAmortos` | [src/b-witch/ActorAmortos.h:44](../../src/b-witch/ActorAmortos.h#L44) |
| [~] | `CBehaviourAmortosCmd` | `CBehaviour` | [src/b-witch/ActorAmortosCmd.h:13](../../src/b-witch/ActorAmortosCmd.h#L13) |
| [~] | `CBehaviourAmortosCmdStand` | `CBehaviourAmortosCmd` | [src/b-witch/ActorAmortosCmd.h:24](../../src/b-witch/ActorAmortosCmd.h#L24) |
| [~] | `CActorAraignos::CBhvDefault` | `CBehaviour` | [src/b-witch/ActorAraignos.h:19](../../src/b-witch/ActorAraignos.h#L19) |
| [~] | `CBehaviourAton` | `CBehaviour` | [src/b-witch/ActorAton.h:52](../../src/b-witch/ActorAton.h#L52) |
| [~] | `CBehaviourAtonEscape` | `CBehaviourAton` | [src/b-witch/ActorAton.h:63](../../src/b-witch/ActorAton.h#L63) |
| [~] | `CBehaviourAutonomous` | `CBehaviour` | [src/b-witch/ActorAutonomous.h:64](../../src/b-witch/ActorAutonomous.h#L64) |
| [~] | `CBehaviourCatchByTrap` | `CBehaviourAutonomous` | [src/b-witch/ActorAutonomous.h:74](../../src/b-witch/ActorAutonomous.h#L74) |
| [~] | `CBehaviourBasicBox` | `CBehaviour` | [src/b-witch/ActorBasicBox.h:14](../../src/b-witch/ActorBasicBox.h#L14) |
| [~] | `CBehaviourBasicBoxStand` | `CBehaviourBasicBox` | [src/b-witch/ActorBasicBox.h:25](../../src/b-witch/ActorBasicBox.h#L25) |
| [~] | `CBehaviourBlazer` | `CBehaviour` | [src/b-witch/ActorBlazer.h:21](../../src/b-witch/ActorBlazer.h#L21) |
| [~] | `CBehaviourBlazerStand` | `CBehaviourBlazer` | [src/b-witch/ActorBlazer.h:32](../../src/b-witch/ActorBlazer.h#L32) |
| [~] | `CBehaviourBombLauncher` | `CBehaviour` | [src/b-witch/ActorBombLauncher.h:10](../../src/b-witch/ActorBombLauncher.h#L10) |
| [~] | `CBehaviourBombLauncherStand` | `CBehaviourBombLauncher` | [src/b-witch/ActorBombLauncher.h:21](../../src/b-witch/ActorBombLauncher.h#L21) |
| [~] | `CBehaviourBonusBase` | `CBehaviour` | [src/b-witch/ActorBonus.h:50](../../src/b-witch/ActorBonus.h#L50) |
| [~] | `CBehaviourBonusAlone` | `CBehaviourBonusBase` | [src/b-witch/ActorBonus.h:72](../../src/b-witch/ActorBonus.h#L72) |
| [~] | `CBehaviourBonusTurn` | `CBehaviourBonusAlone` | [src/b-witch/ActorBonus.h:104](../../src/b-witch/ActorBonus.h#L104) |
| [~] | `CBehaviourBonusPath` | `CBehaviourBonusAlone` | [src/b-witch/ActorBonus.h:117](../../src/b-witch/ActorBonus.h#L117) |
| [~] | `CBehaviourBonusFlock` | `CBehaviourBonusBase` | [src/b-witch/ActorBonus.h:135](../../src/b-witch/ActorBonus.h#L135) |
| [~] | `CBehaviourBonusAddOn` | `CBehaviourBonusFlock` | [src/b-witch/ActorBonus.h:166](../../src/b-witch/ActorBonus.h#L166) |
| [~] | `CBehaviourBonusFountainStand` | `CBehaviour` | [src/b-witch/ActorBonusFountain.h:12](../../src/b-witch/ActorBonusFountain.h#L12) |
| [~] | `CBehaviourBoomyLaunch` | `CBehaviour` | [src/b-witch/ActorBoomy.h:32](../../src/b-witch/ActorBoomy.h#L32) |
| [~] | `CBehaviourBox` | `CBehaviour` | [src/b-witch/ActorBox.h:18](../../src/b-witch/ActorBox.h#L18) |
| [~] | `CBehaviourBoxStand` | `CBehaviourBox` | [src/b-witch/ActorBox.h:29](../../src/b-witch/ActorBox.h#L29) |
| [~] | `CBehaviourBrazulA` | `CBehaviour` | [src/b-witch/ActorBrazul.h:77](../../src/b-witch/ActorBrazul.h#L77) |
| [~] | `CBehaviourBrazulB` | `CBehaviour` | [src/b-witch/ActorBrazul.h:106](../../src/b-witch/ActorBrazul.h#L106) |
| [~] | `CBehaviourFighterBrazul` | `CBehaviourFighterWolfen` | [src/b-witch/ActorBrazul.h:124](../../src/b-witch/ActorBrazul.h#L124) |
| [~] | `CActorBridge::CBhvBasic` | `CBehaviour` | [src/b-witch/ActorBridge.h:180](../../src/b-witch/ActorBridge.h#L180) |
| [~] | `CActorBridge::CBhvWindAware` | `CBehaviour` | [src/b-witch/ActorBridge.h:202](../../src/b-witch/ActorBridge.h#L202) |
| [~] | `CBehaviourBunch` | `CBehaviour` | [src/b-witch/ActorBunch.h:60](../../src/b-witch/ActorBunch.h#L60) |
| [~] | `CBehaviourBunchRepel` | `CBehaviourBunch` | [src/b-witch/ActorBunch.h:77](../../src/b-witch/ActorBunch.h#L77) |
| [~] | `CBehaviourBunchFence` | `CBehaviourBunch` | [src/b-witch/ActorBunch.h:120](../../src/b-witch/ActorBunch.h#L120) |
| [~] | `CBehaviourBunchFighterProjected` | `CBehaviourWolfenFighterProjected` | [src/b-witch/ActorBunch.h:164](../../src/b-witch/ActorBunch.h#L164) |
| [~] | `CBehaviourFighterBunch` | `CBehaviourFighterWolfen` | [src/b-witch/ActorBunch.h:170](../../src/b-witch/ActorBunch.h#L170) |
| [~] | `CBehaviourCareBoy` | `CBehaviour` | [src/b-witch/ActorCareBoy.h:12](../../src/b-witch/ActorCareBoy.h#L12) |
| [~] | `CBehaviourCareBoySteady` | `CBehaviourCareBoy` | [src/b-witch/ActorCareBoy.h:23](../../src/b-witch/ActorCareBoy.h#L23) |
| [~] | `CBehaviourCheckpoint` | `CBehaviour` | [src/b-witch/ActorCheckpointManager.h:11](../../src/b-witch/ActorCheckpointManager.h#L11) |
| [~] | `CBehaviourClusteriserZones` | `CBehaviour` | [src/b-witch/ActorClusteriser.h:37](../../src/b-witch/ActorClusteriser.h#L37) |
| [~] | `CBehaviourCommander` | `CBehaviour` | [src/b-witch/ActorCommander.h:31](../../src/b-witch/ActorCommander.h#L31) |
| [~] | `CBehaviourCommanderBeatUp` | `CBehaviourCommander` | [src/b-witch/ActorCommander.h:52](../../src/b-witch/ActorCommander.h#L52) |
| [~] | `CBehaviourCommanderDefault` | `CBehaviourCommander` | [src/b-witch/ActorCommander.h:67](../../src/b-witch/ActorCommander.h#L67) |
| [~] | `CBehaviourCompanion` | `CBehaviour` | [src/b-witch/ActorCompanion.h:67](../../src/b-witch/ActorCompanion.h#L67) |
| [~] | `CBehaviourCredits` | `CBehaviour` | [src/b-witch/ActorCredits.h:20](../../src/b-witch/ActorCredits.h#L20) |
| [~] | `CBehaviourCreditsDefault` | `CBehaviourCredits` | [src/b-witch/ActorCredits.h:28](../../src/b-witch/ActorCredits.h#L28) |
| [~] | `CActorDCA::CBhvDefault` | `CBehaviour` | [src/b-witch/ActorDCA.h:36](../../src/b-witch/ActorDCA.h#L36) |
| [~] | `CActorDCA::CBhvControlled` | `CActorDCA::CBhvDefault` | [src/b-witch/ActorDCA.h:51](../../src/b-witch/ActorDCA.h#L51) |
| [~] | `CActorDCA::CBhvIAControlled` | `CActorDCA::CBhvControlled` | [src/b-witch/ActorDCA.h:63](../../src/b-witch/ActorDCA.h#L63) |
| [~] | `CActorDCA::CBhvTrajectory` | `CActorDCA::CBhvDefault` | [src/b-witch/ActorDCA.h:69](../../src/b-witch/ActorDCA.h#L69) |
| [~] | `CBehaviourEgg` | `CBehaviour` | [src/b-witch/ActorEgg.h:11](../../src/b-witch/ActorEgg.h#L11) |
| [~] | `CBehaviourEggMove` | `CBehaviourEgg` | [src/b-witch/ActorEgg.h:22](../../src/b-witch/ActorEgg.h#L22) |
| [~] | `CBehaviourElectrolla` | `CBehaviour` | [src/b-witch/ActorElectrolla.h:22](../../src/b-witch/ActorElectrolla.h#L22) |
| [~] | `CBehaviourEventGen` | `CBehaviour` | [src/b-witch/ActorEventGenerator.h:69](../../src/b-witch/ActorEventGenerator.h#L69) |
| [~] | `CBehaviourExplosiveDistributor` | `CBehaviour` | [src/b-witch/ActorExplosiveDistributor.h:18](../../src/b-witch/ActorExplosiveDistributor.h#L18) |
| [~] | `CBehaviourFighter` | `CBehaviour` | [src/b-witch/ActorFighter.h:1073](../../src/b-witch/ActorFighter.h#L1073) |
| [~] | `CBehaviourFighterSlave` | `CBehaviourFighter` | [src/b-witch/ActorFighter.h:1096](../../src/b-witch/ActorFighter.h#L1096) |
| [~] | `CBehaviourFighterRidden` | `CBehaviourFighter` | [src/b-witch/ActorFighter.h:1118](../../src/b-witch/ActorFighter.h#L1118) |
| [~] | `CBehaviourFighterProjected` | `CBehaviourFighter` | [src/b-witch/ActorFighter.h:1139](../../src/b-witch/ActorFighter.h#L1139) |
| [~] | `CBehaviourFogManagerZones` | `CBehaviour` | [src/b-witch/ActorFogManager.h:53](../../src/b-witch/ActorFogManager.h#L53) |
| [~] | `CBehaviourFruit` | `CBehaviour` | [src/b-witch/ActorFruit.h:16](../../src/b-witch/ActorFruit.h#L16) |
| [~] | `CBehaviourFruitSuspended` | `CBehaviourFruit` | [src/b-witch/ActorFruit.h:26](../../src/b-witch/ActorFruit.h#L26) |
| [~] | `CBehaviourFruitBonus` | `CBehaviourFruit` | [src/b-witch/ActorFruit.h:38](../../src/b-witch/ActorFruit.h#L38) |
| [~] | `CBehaviourFruitSquashed` | `CBehaviourFruit` | [src/b-witch/ActorFruit.h:50](../../src/b-witch/ActorFruit.h#L50) |
| [~] | `CActorFx::CBhvSingle` | `CBehaviour` | [src/b-witch/ActorFx.h:17](../../src/b-witch/ActorFx.h#L17) |
| [~] | `CActorFx::CBhvPath` | `CBehaviour` | [src/b-witch/ActorFx.h:33](../../src/b-witch/ActorFx.h#L33) |
| [~] | `CActorFx::CBhvUnknown` | `CBehaviour` | [src/b-witch/ActorFx.h:51](../../src/b-witch/ActorFx.h#L51) |
| [~] | `CBehaviourGravityAwareBell` | `CBehaviour` | [src/b-witch/ActorGravityAware.h:14](../../src/b-witch/ActorGravityAware.h#L14) |
| [~] | `CBehaviourGravityAwareFall` | `CBehaviour` | [src/b-witch/ActorGravityAware.h:34](../../src/b-witch/ActorGravityAware.h#L34) |
| [~] | `CBehaviourHedgehog` | `CBehaviour` | [src/b-witch/ActorHedgehog.h:11](../../src/b-witch/ActorHedgehog.h#L11) |
| [~] | `CBehaviourHedgehogWatchDog` | `CBehaviourHedgehog` | [src/b-witch/ActorHedgehog.h:29](../../src/b-witch/ActorHedgehog.h#L29) |
| [~] | `CBehaviourHedgehogGuardArea` | `CBehaviourHedgehog` | [src/b-witch/ActorHedgehog.h:39](../../src/b-witch/ActorHedgehog.h#L39) |
| [~] | `CBehaviourHedgehogWatchDogArmor` | `CBehaviourHedgehogWatchDog` | [src/b-witch/ActorHedgehog.h:52](../../src/b-witch/ActorHedgehog.h#L52) |
| [~] | `CBehaviourHedgehogGuardAreaArmor` | `CBehaviourHedgehogGuardArea` | [src/b-witch/ActorHedgehog.h:62](../../src/b-witch/ActorHedgehog.h#L62) |
| [~] | `CBehaviourHelperSign` | `CBehaviour` | [src/b-witch/ActorHelperSign.h:21](../../src/b-witch/ActorHelperSign.h#L21) |
| [~] | `CBehaviourHelperSignStand` | `CBehaviourHelperSign` | [src/b-witch/ActorHelperSign.h:31](../../src/b-witch/ActorHelperSign.h#L31) |
| [~] | `CBehaviourHelperSignHighScore` | `CBehaviourHelperSign` | [src/b-witch/ActorHelperSign.h:44](../../src/b-witch/ActorHelperSign.h#L44) |
| [~] | `CBehaviourRideJamGut` | `CBehaviour` | [src/b-witch/ActorHero.h:299](../../src/b-witch/ActorHero.h#L299) |
| [~] | `CBehaviourHero` | `CBehaviourFighter` | [src/b-witch/ActorHero_Private.h:131](../../src/b-witch/ActorHero_Private.h#L131) |
| [~] | `CBehaviourHeroUnknown` | `CBehaviourHero` | [src/b-witch/ActorHero_Private.h:142](../../src/b-witch/ActorHero_Private.h#L142) |
| [~] | `CBehaviourHeroDefault` | `CBehaviourHero` | [src/b-witch/ActorHero_Private.h:154](../../src/b-witch/ActorHero_Private.h#L154) |
| [~] | `CBehaviourHeroRideJamGut` | `CBehaviourRideJamGut` | [src/b-witch/ActorHero_Private.h:1006](../../src/b-witch/ActorHero_Private.h#L1006) |
| [~] | `CBehaviourHunter` | `CBehaviour` | [src/b-witch/ActorHunter.h:12](../../src/b-witch/ActorHunter.h#L12) |
| [~] | `CBehaviourHunterWatch` | `CBehaviourHunter` | [src/b-witch/ActorHunter.h:23](../../src/b-witch/ActorHunter.h#L23) |
| [~] | `CBehaviourJamGut` | `CBehaviour` | [src/b-witch/ActorJamGut.h:51](../../src/b-witch/ActorJamGut.h#L51) |
| [~] | `CBehaviourJamGutStand` | `CBehaviourJamGut` | [src/b-witch/ActorJamGut.h:61](../../src/b-witch/ActorJamGut.h#L61) |
| [~] | `CBehaviourJamGutRidden` | `CBehaviourJamGut` | [src/b-witch/ActorJamGut.h:72](../../src/b-witch/ActorJamGut.h#L72) |
| [~] | `CBehaviourMicken` | `CBehaviour` | [src/b-witch/ActorMicken.h:21](../../src/b-witch/ActorMicken.h#L21) |
| [~] | `CBehaviourMickenEat` | `CBehaviourMicken` | [src/b-witch/ActorMicken.h:32](../../src/b-witch/ActorMicken.h#L32) |
| [~] | `CBehaviourMickenKicked` | `CBehaviourMicken` | [src/b-witch/ActorMicken.h:50](../../src/b-witch/ActorMicken.h#L50) |
| [~] | `CBehaviourMickenSquashed` | `CBehaviourMicken` | [src/b-witch/ActorMicken.h:61](../../src/b-witch/ActorMicken.h#L61) |
| [~] | `S_MINI_GAME_NAME_BEHAVIOUR` | `CBehaviour` | [src/b-witch/ActorMiniGame.cpp:1115](../../src/b-witch/ActorMiniGame.cpp#L1115) |
| [~] | `CBehaviourMiniGame` | `CBehaviour` | [src/b-witch/ActorMiniGame.h:45](../../src/b-witch/ActorMiniGame.h#L45) |
| [~] | `CBehaviourMiniGameBetting` | `CBehaviourMiniGame` | [src/b-witch/ActorMiniGame.h:57](../../src/b-witch/ActorMiniGame.h#L57) |
| [~] | `CBehaviourMiniGameTraining` | `CBehaviourMiniGame` | [src/b-witch/ActorMiniGame.h:81](../../src/b-witch/ActorMiniGame.h#L81) |
| [~] | `CBehaviourMiniGameMulti` | `CBehaviourMiniGame` | [src/b-witch/ActorMiniGame.h:98](../../src/b-witch/ActorMiniGame.h#L98) |
| [~] | `CBehaviourMiniGameBoomyTraining` | `CBehaviourMiniGameTraining` | [src/b-witch/ActorMiniGameBoomy.h:31](../../src/b-witch/ActorMiniGameBoomy.h#L31) |
| [~] | `CBehaviourMiniGameBoomyBetting` | `CBehaviourMiniGameBetting` | [src/b-witch/ActorMiniGameBoomy.h:41](../../src/b-witch/ActorMiniGameBoomy.h#L41) |
| [~] | `CBehaviourMiniGameBoomyMulti` | `CBehaviourMiniGameMulti` | [src/b-witch/ActorMiniGameBoomy.h:51](../../src/b-witch/ActorMiniGameBoomy.h#L51) |
| [~] | `CBehaviourMiniGameBoxCounterTraining` | `CBehaviourMiniGameTraining` | [src/b-witch/ActorMiniGameBoxCounter.h:7](../../src/b-witch/ActorMiniGameBoxCounter.h#L7) |
| [~] | `CBehaviourMiniGameBoxCounterBetting` | `CBehaviourMiniGameBetting` | [src/b-witch/ActorMiniGameBoxCounter.h:17](../../src/b-witch/ActorMiniGameBoxCounter.h#L17) |
| [~] | `CBehaviourMiniGameBoxCounterMulti` | `CBehaviourMiniGameMulti` | [src/b-witch/ActorMiniGameBoxCounter.h:27](../../src/b-witch/ActorMiniGameBoxCounter.h#L27) |
| [~] | `CBehaviourMiniGameDistanceTraining` | `CBehaviourMiniGameTraining` | [src/b-witch/ActorMiniGameDistance.h:8](../../src/b-witch/ActorMiniGameDistance.h#L8) |
| [~] | `CBehaviourMiniGameDistanceBetting` | `CBehaviourMiniGameBetting` | [src/b-witch/ActorMiniGameDistance.h:18](../../src/b-witch/ActorMiniGameDistance.h#L18) |
| [~] | `CBehaviourMiniGameDistanceMulti` | `CBehaviourMiniGameMulti` | [src/b-witch/ActorMiniGameDistance.h:28](../../src/b-witch/ActorMiniGameDistance.h#L28) |
| [~] | `CBehaviourMiniGamesManager` | `CBehaviour` | [src/b-witch/ActorMiniGamesManager.h:13](../../src/b-witch/ActorMiniGamesManager.h#L13) |
| [~] | `CBehaviourMiniGamesManagerStand` | `CBehaviourMiniGamesManager` | [src/b-witch/ActorMiniGamesManager.h:24](../../src/b-witch/ActorMiniGamesManager.h#L24) |
| [~] | `CBehaviourMiniGamesOrganizer` | `CBehaviour` | [src/b-witch/ActorMiniGamesOrganizer.h:16](../../src/b-witch/ActorMiniGamesOrganizer.h#L16) |
| [~] | `CBehaviourMiniGamesOrganizerStand` | `CBehaviourMiniGamesOrganizer` | [src/b-witch/ActorMiniGamesOrganizer.h:27](../../src/b-witch/ActorMiniGamesOrganizer.h#L27) |
| [~] | `CBehaviourMiniGameTimeAttackTraining` | `CBehaviourMiniGameTraining` | [src/b-witch/ActorMiniGameTimeAttack.h:7](../../src/b-witch/ActorMiniGameTimeAttack.h#L7) |
| [~] | `CBehaviourMiniGameTimeAttackBetting` | `CBehaviourMiniGameBetting` | [src/b-witch/ActorMiniGameTimeAttack.h:17](../../src/b-witch/ActorMiniGameTimeAttack.h#L17) |
| [~] | `CBehaviourMiniGameTimeAttackMulti` | `CBehaviourMiniGameMulti` | [src/b-witch/ActorMiniGameTimeAttack.h:27](../../src/b-witch/ActorMiniGameTimeAttack.h#L27) |
| [~] | `CBehaviourMoneyFlock` | `CBehaviour` | [src/b-witch/ActorMoney.h:46](../../src/b-witch/ActorMoney.h#L46) |
| [~] | `CBehaviourMoneyAddOn` | `CBehaviourMoneyFlock` | [src/b-witch/ActorMoney.h:84](../../src/b-witch/ActorMoney.h#L84) |
| [~] | `CBehaviourPlatform` | `CBehaviour` | [src/b-witch/ActorMovingPlatform.h:67](../../src/b-witch/ActorMovingPlatform.h#L67) |
| [~] | `CBehaviourPlatformStand` | `CBehaviourPlatform` | [src/b-witch/ActorMovingPlatform.h:79](../../src/b-witch/ActorMovingPlatform.h#L79) |
| [~] | `CBehaviourTeleportRandom` | `CBehaviourPlatform` | [src/b-witch/ActorMovingPlatform.h:97](../../src/b-witch/ActorMovingPlatform.h#L97) |
| [~] | `CBehaviourPlatformTrajectory` | `CBehaviourPlatform` | [src/b-witch/ActorMovingPlatform.h:113](../../src/b-witch/ActorMovingPlatform.h#L113) |
| [~] | `CBehaviourPlatformSlab` | `CBehaviourPlatform` | [src/b-witch/ActorMovingPlatform.h:139](../../src/b-witch/ActorMovingPlatform.h#L139) |
| [~] | `CBehaviourPlatformDestroyed` | `CBehaviourPlatform` | [src/b-witch/ActorMovingPlatform.h:162](../../src/b-witch/ActorMovingPlatform.h#L162) |
| [~] | `CBehaviourWeighingMachine` | `CBehaviourPlatform` | [src/b-witch/ActorMovingPlatform.h:188](../../src/b-witch/ActorMovingPlatform.h#L188) |
| [~] | `CBehaviourWeighingMachineMaster` | `CBehaviourWeighingMachine` | [src/b-witch/ActorMovingPlatform.h:202](../../src/b-witch/ActorMovingPlatform.h#L202) |
| [~] | `CBehaviourWeighingMachineSlave` | `CBehaviourWeighingMachine` | [src/b-witch/ActorMovingPlatform.h:220](../../src/b-witch/ActorMovingPlatform.h#L220) |
| [~] | `CBehaviourSelector` | `CBehaviourPlatform` | [src/b-witch/ActorMovingPlatform.h:233](../../src/b-witch/ActorMovingPlatform.h#L233) |
| [~] | `CBehaviourSelectorMaster` | `CBehaviourSelector` | [src/b-witch/ActorMovingPlatform.h:260](../../src/b-witch/ActorMovingPlatform.h#L260) |
| [~] | `CBehaviourSelectorSlave` | `CBehaviourSelector` | [src/b-witch/ActorMovingPlatform.h:275](../../src/b-witch/ActorMovingPlatform.h#L275) |
| [~] | `CBehaviourNativ` | `CBehaviour` | [src/b-witch/ActorNativ.h:40](../../src/b-witch/ActorNativ.h#L40) |
| [~] | `CBehaviourNativExorcisme` | `CBehaviourNativ` | [src/b-witch/ActorNativ.h:53](../../src/b-witch/ActorNativ.h#L53) |
| [~] | `CBehaviourNativSeller` | `CBehaviourNativ` | [src/b-witch/ActorNativ.h:142](../../src/b-witch/ActorNativ.h#L142) |
| [~] | `CBehaviourNativAkasa` | `CBehaviourNativ` | [src/b-witch/ActorNativ.h:169](../../src/b-witch/ActorNativ.h#L169) |
| [~] | `CBehaviourNativTakeAndPut` | `CBehaviourNativ` | [src/b-witch/ActorNativ.h:275](../../src/b-witch/ActorNativ.h#L275) |
| [~] | `CBehaviourNativSpeak` | `CBehaviourNativ` | [src/b-witch/ActorNativ.h:304](../../src/b-witch/ActorNativ.h#L304) |
| [~] | `CBehaviourNativLive` | `CBehaviourNativ` | [src/b-witch/ActorNativ.h:333](../../src/b-witch/ActorNativ.h#L333) |
| [~] | `CBehaviourNativCmdStand` | `CBehaviour` | [src/b-witch/ActorNativCmd.h:74](../../src/b-witch/ActorNativCmd.h#L74) |
| [~] | `CBehaviourNativShopSell` | `CBehaviour` | [src/b-witch/ActorNativShop.h:40](../../src/b-witch/ActorNativShop.h#L40) |
| [~] | `CBehaviourNoseMonster` | `CBehaviour` | [src/b-witch/ActorNoseMonster.h:24](../../src/b-witch/ActorNoseMonster.h#L24) |
| [~] | `CBehaviourNoseMonsterSleep` | `CBehaviourNoseMonster` | [src/b-witch/ActorNoseMonster.h:36](../../src/b-witch/ActorNoseMonster.h#L36) |
| [~] | `CBehaviourNoseMonsterAttack` | `CBehaviourNoseMonster` | [src/b-witch/ActorNoseMonster.h:43](../../src/b-witch/ActorNoseMonster.h#L43) |
| [~] | `CBehaviourNoseMonsterDeath` | `CBehaviourNoseMonster` | [src/b-witch/ActorNoseMonster.h:55](../../src/b-witch/ActorNoseMonster.h#L55) |
| [~] | `CBehaviourPattern` | `CBehaviour` | [src/b-witch/ActorPattern.h:32](../../src/b-witch/ActorPattern.h#L32) |
| [~] | `CBehaviourPatternAlone` | `CBehaviourPattern` | [src/b-witch/ActorPattern.h:43](../../src/b-witch/ActorPattern.h#L43) |
| [~] | `CBehaviourPatternMultiple` | `CBehaviourPattern` | [src/b-witch/ActorPattern.h:61](../../src/b-witch/ActorPattern.h#L61) |
| [~] | `CBehaviourPatternCmd` | `CBehaviour` | [src/b-witch/ActorPatternCmd.h:19](../../src/b-witch/ActorPatternCmd.h#L19) |
| [~] | `CBehaviourPatternCmdStand` | `CBehaviourPatternCmd` | [src/b-witch/ActorPatternCmd.h:30](../../src/b-witch/ActorPatternCmd.h#L30) |
| [~] | `CBehaviourProjectile` | `CBehaviour` | [src/b-witch/ActorProjectile.h:32](../../src/b-witch/ActorProjectile.h#L32) |
| [~] | `CBehaviourProjectileStand` | `CBehaviourProjectile` | [src/b-witch/ActorProjectile.h:45](../../src/b-witch/ActorProjectile.h#L45) |
| [~] | `CBehaviourProjectileLavaBall` | `CBehaviourProjectile` | [src/b-witch/ActorProjectile.h:65](../../src/b-witch/ActorProjectile.h#L65) |
| [~] | `CBehaviourProjectileStraight` | `CBehaviourProjectile` | [src/b-witch/ActorProjectile.h:98](../../src/b-witch/ActorProjectile.h#L98) |
| [~] | `CBehaviourProjectileExcuse` | `CBehaviourProjectile` | [src/b-witch/ActorProjectile.h:116](../../src/b-witch/ActorProjectile.h#L116) |
| [~] | `CBehaviourProjectilePortable` | `CBehaviourProjectile` | [src/b-witch/ActorProjectile.h:128](../../src/b-witch/ActorProjectile.h#L128) |
| [~] | `CBehaviourProjectileNew` | `CBehaviourProjectile` | [src/b-witch/ActorProjectile.h:144](../../src/b-witch/ActorProjectile.h#L144) |
| [~] | `CBehaviourPunchingBall` | `CBehaviour` | [src/b-witch/ActorPunchingBall.h:26](../../src/b-witch/ActorPunchingBall.h#L26) |
| [~] | `CBehaviourPunchingBallStand` | `CBehaviourPunchingBall` | [src/b-witch/ActorPunchingBall.h:37](../../src/b-witch/ActorPunchingBall.h#L37) |
| [~] | `CBehaviourPunchingBallSlave` | `CBehaviourFighterSlave` | [src/b-witch/ActorPunchingBall.h:48](../../src/b-witch/ActorPunchingBall.h#L48) |
| [~] | `CBehaviourPunchingBallRidden` | `CBehaviourFighterRidden` | [src/b-witch/ActorPunchingBall.h:56](../../src/b-witch/ActorPunchingBall.h#L56) |
| [~] | `CBehaviourPunchingBallDefault` | `CBehaviourFighter` | [src/b-witch/ActorPunchingBall.h:63](../../src/b-witch/ActorPunchingBall.h#L63) |
| [~] | `CBehaviourRopeStand` | `CBehaviour` | [src/b-witch/ActorRope.h:15](../../src/b-witch/ActorRope.h#L15) |
| [~] | `CBehaviourRune` | `CBehaviour` | [src/b-witch/ActorRune.h:17](../../src/b-witch/ActorRune.h#L17) |
| [~] | `CBehaviourShip` | `CBehaviour` | [src/b-witch/ActorShip.h:43](../../src/b-witch/ActorShip.h#L43) |
| [~] | `CBehaviourShipFLY` | `CBehaviourShip` | [src/b-witch/ActorShip.h:54](../../src/b-witch/ActorShip.h#L54) |
| [~] | `CBehaviourShocker` | `CBehaviour` | [src/b-witch/ActorShocker.h:33](../../src/b-witch/ActorShocker.h#L33) |
| [~] | `CBehaviourShockerFireWave` | `CBehaviourShocker` | [src/b-witch/ActorShocker.h:45](../../src/b-witch/ActorShocker.h#L45) |
| [~] | `CBehaviourShockerCmd` | `CBehaviour` | [src/b-witch/ActorShockerCmd.h:20](../../src/b-witch/ActorShockerCmd.h#L20) |
| [~] | `CBehaviourShockerCmdStand` | `CBehaviourShockerCmd` | [src/b-witch/ActorShockerCmd.h:31](../../src/b-witch/ActorShockerCmd.h#L31) |
| [~] | `CBehaviourShoot` | `CBehaviour` | [src/b-witch/ActorShoot.h:17](../../src/b-witch/ActorShoot.h#L17) |
| [~] | `CBehaviourShootFire` | `CBehaviourShoot` | [src/b-witch/ActorShoot.h:28](../../src/b-witch/ActorShoot.h#L28) |
| [~] | `CBehaviourShootFireWave` | `CBehaviourShoot` | [src/b-witch/ActorShoot.h:50](../../src/b-witch/ActorShoot.h#L50) |
| [~] | `CBehaviourStiller` | `CBehaviour` | [src/b-witch/ActorStiller.h:27](../../src/b-witch/ActorStiller.h#L27) |
| [~] | `CBehaviourStillerStand` | `CBehaviourStiller` | [src/b-witch/ActorStiller.h:38](../../src/b-witch/ActorStiller.h#L38) |
| [~] | `CBehaviourSwitch` | `CBehaviour` | [src/b-witch/ActorSwitch.h:40](../../src/b-witch/ActorSwitch.h#L40) |
| [~] | `CBehaviourSwitchLever` | `CBehaviourSwitch` | [src/b-witch/ActorSwitch.h:51](../../src/b-witch/ActorSwitch.h#L51) |
| [~] | `CBehaviourSwitchMagic` | `CBehaviourSwitch` | [src/b-witch/ActorSwitch.h:76](../../src/b-witch/ActorSwitch.h#L76) |
| [~] | `CBehaviourSwitchWolfenCounter` | `CBehaviourSwitch` | [src/b-witch/ActorSwitch.h:117](../../src/b-witch/ActorSwitch.h#L117) |
| [~] | `CBehaviourSwitchTarget` | `CBehaviourSwitch` | [src/b-witch/ActorSwitch.h:143](../../src/b-witch/ActorSwitch.h#L143) |
| [~] | `CBehaviourSwitchTimed` | `CBehaviourSwitch` | [src/b-witch/ActorSwitch.h:168](../../src/b-witch/ActorSwitch.h#L168) |
| [~] | `CBehaviourSwitchMultiCondition` | `CBehaviourSwitch` | [src/b-witch/ActorSwitch.h:195](../../src/b-witch/ActorSwitch.h#L195) |
| [~] | `CBehaviourSwitchSequence` | `CBehaviourSwitch` | [src/b-witch/ActorSwitch.h:214](../../src/b-witch/ActorSwitch.h#L214) |
| [~] | `CBehaviourTeleporterDefault` | `CBehaviour` | [src/b-witch/ActorTeleporter.h:17](../../src/b-witch/ActorTeleporter.h#L17) |
| [~] | `CBehaviourTrap` | `CBehaviour` | [src/b-witch/ActorTrap.h:21](../../src/b-witch/ActorTrap.h#L21) |
| [~] | `CBehaviourTrapStand` | `CBehaviourTrap` | [src/b-witch/ActorTrap.h:30](../../src/b-witch/ActorTrap.h#L30) |
| [~] | `CBehaviourTrapStand_ActorTrap` | `CBehaviourTrapStand` | [src/b-witch/ActorTrap.h:79](../../src/b-witch/ActorTrap.h#L79) |
| [~] | `CBehaviourWeapon` | `CBehaviour` | [src/b-witch/ActorWeapon.h:23](../../src/b-witch/ActorWeapon.h#L23) |
| [~] | `CBehaviourWeaponBurst` | `CBehaviourWeapon` | [src/b-witch/ActorWeapon.h:50](../../src/b-witch/ActorWeapon.h#L50) |
| [~] | `CBehaviourWeaponRpg` | `CBehaviourWeaponBurst` | [src/b-witch/ActorWeapon.h:96](../../src/b-witch/ActorWeapon.h#L96) |
| [~] | `CBehaviourWeaponBullets` | `CBehaviourWeaponBurst` | [src/b-witch/ActorWeapon.h:110](../../src/b-witch/ActorWeapon.h#L110) |
| [~] | `CBehaviourWeaponSniper` | `CBehaviourWeaponBullets` | [src/b-witch/ActorWeapon.h:163](../../src/b-witch/ActorWeapon.h#L163) |
| [~] | `CBehaviourWeaponPistol` | `CBehaviourWeaponBullets` | [src/b-witch/ActorWeapon.h:179](../../src/b-witch/ActorWeapon.h#L179) |
| [~] | `CBehaviourWind` | `CBehaviour` | [src/b-witch/ActorWind.h:300](../../src/b-witch/ActorWind.h#L300) |
| [~] | `CBehaviourWolfen` | `CBehaviour` | [src/b-witch/ActorWolfen.h:461](../../src/b-witch/ActorWolfen.h#L461) |
| [~] | `CBehaviourLost` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:517](../../src/b-witch/ActorWolfen.h#L517) |
| [~] | `CBehaviourExorcism` | `CBehaviour` | [src/b-witch/ActorWolfen.h:536](../../src/b-witch/ActorWolfen.h#L536) |
| [~] | `CBehaviourTrackStand` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:576](../../src/b-witch/ActorWolfen.h#L576) |
| [~] | `CBehaviourTrack` | `CBehaviourTrackStand` | [src/b-witch/ActorWolfen.h:586](../../src/b-witch/ActorWolfen.h#L586) |
| [~] | `CBehaviourWatchDog` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:597](../../src/b-witch/ActorWolfen.h#L597) |
| [~] | `CBehaviourSleep` | `CBehaviourWatchDog` | [src/b-witch/ActorWolfen.h:614](../../src/b-witch/ActorWolfen.h#L614) |
| [~] | `CBehaviourEscape` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:620](../../src/b-witch/ActorWolfen.h#L620) |
| [~] | `CBehaviourWolfenWeapon` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:649](../../src/b-witch/ActorWolfen.h#L649) |
| [~] | `CBehaviourSnipe` | `CBehaviourWolfenWeapon` | [src/b-witch/ActorWolfen.h:689](../../src/b-witch/ActorWolfen.h#L689) |
| [~] | `CBehaviourTrackWeaponStand` | `CBehaviourWolfenWeapon` | [src/b-witch/ActorWolfen.h:755](../../src/b-witch/ActorWolfen.h#L755) |
| [~] | `CBehaviourTrackWeapon` | `CBehaviourTrackWeaponStand` | [src/b-witch/ActorWolfen.h:791](../../src/b-witch/ActorWolfen.h#L791) |
| [~] | `CBehaviourTrackWeaponSnipe` | `CBehaviourTrackWeaponStand` | [src/b-witch/ActorWolfen.h:806](../../src/b-witch/ActorWolfen.h#L806) |
| [~] | `CBehaviourAvoid` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:829](../../src/b-witch/ActorWolfen.h#L829) |
| [~] | `CBehaviourGuardArea` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:846](../../src/b-witch/ActorWolfen.h#L846) |
| [~] | `CBehaviourDCA` | `CBehaviourWolfen` | [src/b-witch/ActorWolfen.h:867](../../src/b-witch/ActorWolfen.h#L867) |
| [~] | `CBehaviourWolfenGrindDeath` | `CBehaviour` | [src/b-witch/ActorWolfen.h:899](../../src/b-witch/ActorWolfen.h#L899) |
| [~] | `CBehaviourFighterWolfen` | `CBehaviourFighter` | [src/b-witch/ActorWolfen.h:923](../../src/b-witch/ActorWolfen.h#L923) |
| [~] | `CBehaviourWolfenFighterRidden` | `CBehaviourFighterRidden` | [src/b-witch/ActorWolfen.h:1008](../../src/b-witch/ActorWolfen.h#L1008) |
| [~] | `CBehaviourWolfenFighterSlave` | `CBehaviourFighterSlave` | [src/b-witch/ActorWolfen.h:1016](../../src/b-witch/ActorWolfen.h#L1016) |
| [~] | `CBehaviourWolfenFighterProjected` | `CBehaviourFighterProjected` | [src/b-witch/ActorWolfen.h:1027](../../src/b-witch/ActorWolfen.h#L1027) |
| [~] | `CBehaviourWoodMonster` | `CBehaviour` | [src/b-witch/ActorWoodMonster.h:20](../../src/b-witch/ActorWoodMonster.h#L20) |
| [~] | `CBehaviourWoodMonsterGet` | `CBehaviourWoodMonster` | [src/b-witch/ActorWoodMonster.h:31](../../src/b-witch/ActorWoodMonster.h#L31) |
| [~] | `CBehaviourWoof` | `CBehaviour` | [src/b-witch/ActorWoof.h:33](../../src/b-witch/ActorWoof.h#L33) |
| [~] | `CBehaviourWoofGuard` | `CBehaviourWoof` | [src/b-witch/ActorWoof.h:44](../../src/b-witch/ActorWoof.h#L44) |
| [~] | `CBehaviourWoofVerticalJump` | `CBehaviourWoof` | [src/b-witch/ActorWoof.h:56](../../src/b-witch/ActorWoof.h#L56) |
| [~] | `CBehaviourInventory` | `CBehaviour` | [src/b-witch/BehaviourInventory.h:10](../../src/b-witch/BehaviourInventory.h#L10) |

## Inventory review

Lexical discovery includes all source configurations. Review macro-generated types, factory registrations, and unresolved inheritance before claiming exhaustiveness.

