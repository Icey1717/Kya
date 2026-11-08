# Reviewed small-function PS2 identities

`ps2-identities-reviewed.json` records the disposition of the original 426
identity blockers: 358 missing qualified-name matches and 68 ambiguous matches.
`fixtures/ps2-identity-baseline.json` freezes that cohort, its declarations,
source hashes, candidate entries and original report hash. Do not regenerate it
from a later survey: fresh counts and baseline progress answer different questions.

The current review records **192 mapped definitions**, 234 host helpers without
a standalone EE counterpart, and no unresolved definitions. All 68 originally
ambiguous definitions now have reviewed PS2 mappings; all 358 originally
missing-name cases now have a reviewed disposition, including source-only
helpers with no defensible one-to-one PS2 function. The latest identity-only
reconciliation is in `out/verification/ps2-identities-live/report.json`.
These dispositions are not equivalence verdicts.

The retained compiled pass covers the earlier 107-definition mapping set: it
proves 12 definitions equivalent in the reported input domains, leaves two
field mappings conditional, and retains later blockers for the other 93. It is
under `out/verification/identity-expansion/compiled/`; the later identity
reviews have not been compiled as a group.

## Running and reviewing

Run in the production Visual Studio x64 developer environment, with Totala
serving the original game. Keep the historical output directory intact:

```powershell
python tools/function_verify/small_functions.py --output out/verification/small-functions-identities --ps2-elf 'D:/path/to/SLES_514.73'
python tools/function_verify/small_functions.py --output out/verification/identities-only --identities-only --ps2-elf 'D:/path/to/SLES_514.73'
python tools/function_verify/small_functions.py --output out/verification/ps2-identities-live --identities-only --no-clang-fallback
python tools/function_verify/small_functions.py --output out/verification/small-functions-identities --compare-prepared --ps2-elf 'D:/path/to/SLES_514.73'
```

`--identity-manifest` selects an alternate manifest; the checked-in manifest is
the default. `--identities-only` stops before rebuilding objects or lifting code.
It reports identity-only rows as unsupported because no compiled equivalence
was attempted, and it exits 2 for that reason. `--inventory-only` still stops
before any live identity resolution.

Without `--ps2-elf`, the live exporter supplies the executable SHA-256 used to
validate reviews. Original instruction-byte correspondence remains unchecked,
as in the existing survey. Supply the local original ELF for that additional
check, including delay slots. The executable path reported by Ghidra may belong
to a different machine and is not automatically downloaded.

Fresh reports include `ps2_identity` diagnostics per scalar candidate and an
`identity_baseline` reconciliation for all 426 original records. A recovered
identity may expose another unsupported obligation. Source edits or changed
discovery names can prevent reconciliation; these entries are reported rather
than dropped. An external report can also be reconciled explicitly:

```powershell
python tools/function_verify/ps2_identity.py --report out/verification/small-functions/report.json --output out/verification/identity-reconciliation.json
```

## Manifest contract

Each record includes the original cohort ID, exact source path/name/declaration,
normalized source-body SHA-256, a disposition, an obligation and evidence.
The manifest pins the original executable SHA-256. IDs include byte positions;
after unrelated source moves, lookup can use path/name/declaration, but the
complete source hash must still match.

- `mapped`: contains exact reviewed PS2 entry, name, namespace, signature and
  body boundaries, plus hashed live-review evidence. The resolver fetches the
  entry directly and rejects changed metadata. The export must match that
  entry, qualified live name and executable hash, and must be complete.
- `non-standalone`: requires hashed source-review evidence explaining the host
  implementation. It remains unsupported and cannot authorize a PS2 export.
- `unresolved`: retains the missing obligation and baseline evidence. It does
  not override a newly unique exact qualified-name match.

Duplicate definitions, incomplete reviews and changed evidence are errors.
Invalid source/ELF/live identity reviews produce a stale diagnostic without
falling back to a guessed match. Prepared replay rejects a changed manifest and
revalidates reviewed source and mapped live metadata; reviewed replay therefore
requires the live bridge. Replay still checks source/object/build freshness.

To approve another record, inspect live decompilation **and disassembly**, then
record callers, vtable slots, receiver relationships or other distinguishing
evidence as needed. Commit compact evidence under `fixtures/` and its hash in
the manifest. Never approve based solely on an address suffix, short name,
signature, shared bytes or a successful solver comparison. Keep wrappers and
implementations distinct. Ghidra is read-only throughout this workflow.

Normal overloaded definitions now receive exact compiler symbols through the
same production-configured discovery used for parser recovery. Compiler errors
invalidate the translation unit's evidence. This does not expand the scalar,
template, argument or relocation proof subset.

## Reviewed examples and limits

The ambiguity pass distinguishes implementation entries from actual jump
wrappers and separates overloads by their complete declaration and body. All
68 original ambiguous definitions now have reviewed dispositions, including
the duplicate `CActorSound::FadeTo` implementations and the inline base
`CObjectManager::Level_Init` initializer. No address was selected from a name
or body similarity alone.

Named PS2 vtables distinguish derived callbacks that Ghidra labels under a base
class, template specialization or misspelled name. For example, the Aton table
at `00444520` selects its Term wrapper at `00389440`, while the original
autonomous Term implementation remains `00123f00`. These are separate reviewed
source identities and entries. Concrete FX-manager tables identify their pool
specializations. The two identical `edCFiler::configure` bodies are separated by
slots `+0xc` and `+0x10`, not by selecting the first zero-return function.

An inherited original dispatch target can also be the counterpart of an explicit
source override that forwards to its base. Such records state that relationship;
they do not invent a standalone PS2 body for the derived class. Original virtual
slots can identify the intended counterpart of a guarded source stub too;
that identity does not approve the stub's behavior.

`CActor::Init` maps to `001047d0`: the sound-list traversal and SetBehaviour
dispatch distinguish it from the three jump wrappers at `0037fae0`, `003ba7c0`
and `003eb400`. The wrappers retain their own entries in the evidence.

`CActorHunter::SetupCollisionMatrix` is reviewed as non-standalone. The
`CBehaviourHunterWatch` vtable at `00445db0` selects `Manage @003b9d10` in its
`+0x18` slot; that callback repeats the helper's matrix initialization,
collision-flag mask, bone-position update and collision-matrix update in five
state branches. The source callback has the same five helper calls.

The three BonePhysics entries at `003c2910`, `003c28a0` and `003c2aa0` use global
Ghidra names. Their source member spelling does not prevent reviewed lookup.
The review does not approve additional source writes or null guards.

`randImpl` maps to `rand` at `0021bd98` through its multiplier, increment, upper
word selection and mask. Its host seed and PS2 `__impure_ptr+0xa8` remain an
unreviewed global-memory correspondence. It is not classified as port-only.

The address `00242330` belongs to a `CAnimation` receiver accessing its nested
animator, whereas the source getter belongs to `edAnmBinMetaAnimator`. Its
reviewed offset-zero base/member layout now supports that identity. The two
animation-end wrappers are separately classified as non-standalone because
their checks are expanded in the PS2 Aton manage callback. Other template and
renamed helpers retain their individual obligations in the manifest.

Validation:

```powershell
python -m unittest discover -s tools/function_verify -p test_ps2_identity.py
python -m unittest discover -s tools/function_verify -p test_small_functions.py
python -m unittest discover -s tools/function_verify -p test_clang_discovery.py
python tools/function_verify/regressions.py --tier fast
```
