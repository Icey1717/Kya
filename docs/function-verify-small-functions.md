# Short-function verification survey, 5 November 2026

The [batch runner](../tools/function_verify/SMALL_FUNCTIONS.md) inventoried 8,324
function definitions in 813 files under `src/`. Of these, 4,415 have fewer than
20 physical lines, including signatures, braces, comments and blank lines.
Inline header definitions are included; this is a count of source definitions,
not unique linked functions.

| Result | Definitions |
| --- | ---: |
| Equivalent in the reported input domain, without inferred field correspondence | 438 |
| Equivalent conditional on an inferred single-field correspondence | 73 |
| Raw mismatches, reviewed below | 4 |
| Unsupported; no equivalence verdict | 3,900 |
| Total selected | 4,415 |

The runner found 3,252 scalar function candidates and 2,826 live PS2 identity
matches. It dependency-checked 300 production objects in `out/build/x64-verify`,
then extracted, lifted and attempted comparison of 1,265 relocation-approved
function slices. Every instruction and lifted delay slot in those PS2 exports
matched the original executable bytes. The executable SHA-256 is
`d7d85d0fc342f8f8f684aac6ebbea938cc3f1d9f063d071796f76dbf18b7c0cb`.
The Windows build has game logging disabled. These are COFF comparisons,
not linked executable or whole-program proofs.

## Follow-up: integer IsActive returns

The base `CInterface::IsActive` and its `CInventoryInterface`, `CMagicInterface`
and `OtherInterface` overrides now return `int`. The base still returns 1;
the overrides preserve the complete integer fields. Live Ghidra comparison used
`001d7cc0` for the base and `001dc7e0`, `001dce10`, `001dbbc0` for the overrides,
including each return delay slot. Ghidra labels the base return as bool, but its
instructions produce the full integer value 1.

Freshly rebuilt production code for all four matches the original PS2 returns
and observable memory. The getters cover every 32-bit initial field value;
their identities/PS2 offsets were explicitly reviewed against decompilation and
disassembly, and Windows offsets were compiler-probed again. Original instruction
bytes were rechecked against the ELF. Evidence is in
`out/verification/interface-is-active/reviewed-report.json`, with positive
regression exports in `tools/function_verify/fixtures/interface-is-active.json`.
The original bool exports below remain as historical negative controls.

The normal `x64-debug` `KyaPortTest` target rebuilt successfully and its smoke
suite passed. The survey tests also check all four corrected exports and reject
review records for a different field or function. The original full-survey
counts above describe the earlier baseline; they have not been recomputed.

## Reviewed discrepancies

| Function | PS2 entry | Observation |
| --- | --- | --- |
| `CInventoryInterface::IsActive` | `001dc7e0` | PS2 returns the integer at `this+4`; Windows returns `bool(bActive)`. |
| `CMagicInterface::IsActive` | `001dce10` | Same integer-to-bool representation difference. |
| `OtherInterface::IsActive` | `001dbbc0` | Same difference for `field_0x4`. |
| `StaticEdFileBase::Remove` | `00401f10` | PS2 decrements the user count; Windows additionally clamps negative results to zero. |

All four field correspondences were checked against live Ghidra decompilation
and disassembly. The three getters consist of `jr ra` with `lw v0,4(a0)` in the
delay slot (`001dc7e4`, `001dce14`, `001dbbc4`). Their corresponding Windows
integer fields are at offset 8, confirmed by compiler probes.

The getter counterexamples require noncanonical initial fields. For example,
with field 8, the raw PS2 return is 8 while the Windows bool is 1. The survey
compares the declared Windows return width, so a field whose only set bits are
above bit 7 can also produce low-byte 0 versus bool 1. This is a raw ABI/return
representation discrepancy, not evidence that the original caller observed
false for a nonzero integer. Retained machine-code tests confirm the returns
agree when the field is 0 or 1. The visible activation paths assign those
canonical values; the survey does not prove that invariant throughout every
object lifetime. No gameplay bug is confirmed from these three findings.

`Remove` loads the count at `00401f10`, decrements at `00401f14`, and stores it
in the return delay slot at `00401f1c`. Its Windows count is also at offset 8;
the PS2 count is at offset 4. A concrete zero-count call produces `0xffffffff`
on PS2 and zero on Windows. The clamp in `src/b-witch/EdFileBase.cpp` is explicitly
marked `HACK`, so this confirms an existing deliberate port divergence. No game
source was changed by this survey.

The four real production export pairs are retained in
`tools/function_verify/fixtures/small-functions-survey.json`; regression tests
reproduce all four broad-domain mismatches, the canonical getter states, and
the zero-count edge case.

## Coverage limits and useful next work

The largest limitations are unresolved compiled relocations/callees (1,456), declarations
outside the scalar subset (874), the 426 missing or ambiguous live PS2 identities
from the initial audit (all now have reviewed dispositions), unmodeled incoming
registers/arguments (228), and parser errors (158). Other blockers include
templates/nested identities, object graphs, multiple fields, floating point,
indirect dispatch and loops. Each definition has its exact first blocker in the
generated JSON. An unsupported result does not identify a defect.

The 73 conditional matches need review of their individual field identities;
access-span inference alone can conceal a wrong-field selection. The existing
audited suites already cover some overlapping functions with stronger mappings
and broader call/loop support. This generic survey does not replace them.

Useful extensions are to reuse those audited mappings/callee graphs in the batch
runner, add reviewed ABI setup for additional argument registers and globals,
then select small multi-field functions with explicit compiler/PS2 layouts.
Line count alone does not make a function independently verifiable.

The original compiled-survey outputs and prepared replay are retained under
`out/verification/small-functions-identities/`; that report was replayed against
unchanged source bodies/objects. The current `report.md` and `report.json` in
`out/verification/small-functions/` are an identity-only refresh: they retain
the first identity blocker for each selected function and report all 4,415 as
unsupported because compiled comparison was not attempted. The identity-only
report does not replace the historical compiled results. The earlier compiled
survey exits 1 because its four raw mismatches remain visible.

Validation: the 244-test fast offline regression suite passed, as did all 15
survey tests. The latter include real compiled negative controls, missing/extra
write controls, original-ELF delay-slot validation and incomplete-survey exit
status checks. The normal compiled fixture suite also passed its nine tests.

## PS2 identity follow-up, 6 November 2026

The [reviewed identity workflow](../tools/function_verify/PS2_IDENTITIES.md)
now freezes the original 426 identity blockers independently of later discovery
and source changes. Repository mappings and hashed review evidence resolve
name/namespace differences without changing the shared Ghidra project.

| Original cohort disposition | Definitions |
| --- | ---: |
| Reviewed PS2 entry mapping | 192 |
| Reviewed host helper without a standalone PS2 EE counterpart | 234 |
| Still missing a defensible identity | 0 |
| Still ambiguous | 0 |
| Total accounted for | 426 |

The initial 11 mapped definitions reached another blocker: eight have unapproved
relocations/callees or global storage, one requires loop support, one requires
symbolic pointer memory support, and one lacks an unambiguous emitted Windows
symbol. A subsequent compiled pass reviewed a total of 107 mappings; it proves
**12 equivalent**, leaves **two conditional-equivalent**, and retains later
blockers for the other **93** mappings. Later identity review expanded the
manifest to 192 mappings and classified another 234 helpers as non-standalone.
Those later records have not all been through compiled comparison. No address
was assigned merely to reduce the count: the last two source-only helpers were
reviewed against their callers and the corresponding PS2 parent routines.

`CActor::Init` at `001047d0` was distinguished from its three jump wrappers.
The review also covers three globally named BonePhysics routines, owner/name
aliases, two source spellings of the scene flag routine at `002a5440`, and
`randImpl` versus PS2 `rand` at `0021bd98`. Windows-only pointer registries,
debug drawing and host VU emulator helpers account for the original 17
non-standalone dispositions. `randImpl` remains a mapping with an unapproved
seed-global relationship, rather than being classified as port-only.

Compiler discovery additionally supplies exact emitted symbols for ordinary
overloads. All 83 selected production translation units parsed without errors.
An overloaded definition without a matching active compiler identity now fails
closed; it cannot fall back to another definition's symbol prefix.

The initial production-object survey and subsequent prepared replay are retained
under `out/verification/small-functions-identities/`. The final report covers
4,407 definitions: **438 equivalent, 77 conditional-equivalent and 3,892
unsupported**, with zero raw mismatches in the current working tree. Eight
compiler-confirmed `PACK(...)` type declarations were excluded from the old
4,415-definition discovery count. Current source includes the pre-existing
interface/Remove corrections; these new counts do not replace the historical
baseline above. No game source was changed by this identity follow-up.

Expanded evidence includes actual jump/delay-slot distinctions, overload
arguments and bodies, profiling callers, named original vtable slots, and
caller-side expansions for inline helpers. It resolves all 68 original
ambiguities, including duplicate `CActorSound::FadeTo` implementations and an
inline base `CObjectManager::Level_Init` definition. A recently reviewed helper,
`CActorHunter::SetupCollisionMatrix`, is expanded in all five state branches of
PS2 `CBehaviourHunterWatch::Manage @003b9d10`, selected by vtable slot
`00445db0+0x18`. Vtables also identify 28
formerly missing callbacks, including inherited actor wrappers, FX template
specializations and Ghidra spelling errors. Later reviews add, among others,
the bone-physics transforms and attachment helpers, screen-matrix wrappers,
animation checks, and fighter climb-vector logic.

The retained compiled expansion is in
`out/verification/identity-expansion/compiled/report.json`: 107 reviewed cohort
definitions plus one additional overload selected by the same name filter.
The 12 unconditional and two conditional matches belong to the original cohort;
the additional overload remains unsupported. The current identity-only pass at
`out/verification/ps2-identities-live/report.json` reconciles all 426 entries:
192 mapped, 234 non-standalone, none missing and none ambiguous. Its matching
function-level first blockers are also retained in
`out/verification/small-functions/report.json`. The older full compiled-survey
counts above remain historical; its manifest/object hashes are not silently
refreshed when a later recovery pass rebuilds production objects.

The identity-only refresh used the live Ghidra bridge and matched the pinned
executable SHA-256. It did **not** recheck instruction bytes against a local
ELF, and it did not build or compare code. Existing retained instruction and
delay-slot negative controls still apply; supply `--ps2-elf` for fresh byte
validation. The earlier compiled survey and regression-suite results remain
historical validation of their recorded source and object hashes.
