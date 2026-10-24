# Function-by-function PS2 audit

This read-only tool scans `src/b-witch`, pairs functions with a Ghidra snapshot,
and generates a review queue without making any model requests. It reuses the
local renamer's installed Tree-sitter dependencies. Source files and the Ghidra
program are never modified.

The first pass detects implementation guards, differences in named direct call
counts/order, and differences in literal value counts. It expands small,
unambiguous, parameterless `void` helpers containing straight-line statements
before comparing. This catches the Wolfen `FunReset` mistake: callers acquire
extra occurrences of the cancel value `8` after helper expansion.

These are **candidates**, not confirmed bugs. The comparison is lexical, not
path-sensitive. It does not fully resolve preprocessing, types, field offsets, indirect calls,
or equivalent control-flow rewrites. Local temporary expressions, wrapper
functions, named constants and Ghidra's constant folding can produce noise.
Matching signal counts do not establish equivalence. No automatic fixes are made.

## Existing audit and where to resume

The local full-folder report saved on **22 October 2026** is
`out/function-audit/report.json` (with a readable `report.md`). It used filter
version 3 and the `SLES_514.73` PS2 snapshot, image base `00100000`:

| Status | Functions |
|---|---:|
| `candidate` | 2,273 |
| `no_findings` | 3,027 |
| `parse_error` | 1,287 |
| `unmatched` | 751 |
| `ambiguous` | 146 |
| `reviewed` | 2 |
| **Total inventoried** | **7,486** |

The 2,273 entries are heuristic candidates, not confirmed defects. These counts
describe that saved run; use a fresh report for current source. Audit artifacts
under `out/` are local generated files and may be absent in another checkout.

The existing `out/function-audit/ps2.jsonl` allows comparison without recapturing
Ghidra. Preserve the old report and regenerate packets from current source into
a separate directory:

```powershell
npm --prefix tools/local_renamer run audit -- compare --out out/function-audit-resume --snapshot out/function-audit/ps2.jsonl
```

Read `out/function-audit-resume/report.md` and consume packets from that same
directory. A source edit can change function IDs, signals and review validity;
do not carry an old packet's verdict onto a new body. This refresh updates the
source comparison only. Recapture the snapshot when relevant Ghidra names,
types or analysis have changed.

The **23 October 2026** refresh in that directory still inventories 7,486
functions: 2,274 candidates, 3,027 `no_findings`, 1,287 parse errors, 751 unmatched,
146 ambiguous and one reviewed. The particle setter's previous exception no
longer applies to the current source/dependency hashes; its field-offset signal
has returned to the queue. A returning signal requires review, not an assumption
that the old defect has returned.

Earlier investigation notes are in `out/function-audit/sample-review.md`,
`filtering-review.md`, `random-sample-2-review.md` and
`random-sample-3-review.md`. They record the money `LoadContext` omission
(`003615d0`), particle `SetTimeScaler` omission (`0032cff0`), and several expected
representation differences. Both omissions were subsequently fixed; the initial
sample notes describe the pre-fix bodies. These exploratory samples do not
estimate the defect rate of the remaining queue. Notes alone do not suppress
findings; only matching entries in the reviewed-exception ledger do that.

The [fourth random sample](../docs/function-audit-sample-4.md), reviewed on
23 October, found a missing manager-destruction path in `CScene::~CScene`
(`001b9f30`) and four expected representation differences. The report records
supporting PS2 instruction addresses and the limits of that investigation.

The [fifth random sample](../docs/function-audit-sample-5.md) found five expected
differences, including implicit base construction and PS2 trig macros enabled by
the Windows build. It records general filtering opportunities without adding
per-function exceptions. All 8,192 used cosine-table entries matched live PS2
bytes in that review.

The [sixth random sample](../docs/function-audit-sample-6.md), drawn from the
version 5 queue of 2,148 candidates, found a missing quaternion restoration in
`CActorBlazer::Blazer_UpdatePosition` (`003d83d0`). The other four findings were
explained by helpers, virtual dispatch, allocator naming and Windows bookkeeping.
The report records PS2 instruction evidence; no game fixes or exceptions were added.

The [orchestrated audit](../docs/function-audit/README.md), started on 24 October,
reviews the remaining queue with three Luna agents. Its refreshed comparison has
2,144 candidate functions. The index tracks completed checkpoints and unresolved
signals; each function has a separate report with instruction evidence and a
suggested fix where supported. These reviews do not apply game fixes or suppress
the deterministic findings. See the linked original-program integrity report:
three switch instructions and seventeen data words in the selected Ghidra program
differ from the executable supplied for verification.

## Reviewing the remaining queue

Use the report's result IDs to select packets. For example, this lists candidates
with implementation guards (`unfinished` is the signal kind):

```powershell
$audit = Get-Content out/function-audit-resume/report.json -Raw | ConvertFrom-Json
$audit.results | Where-Object { $_.status -eq 'candidate' -and $_.findings.kind -contains 'unfinished' } | Select-Object name, address, path, line, id
```

Start with implementation guards and plausible missing calls, then inspect
literal differences in context. Keep unresolved indirect calls in the queue
until their targets are established. Parse errors and ambiguous matches are
coverage gaps requiring separate investigation, even if candidate counts fall.

For each reviewed function:

1. Check the current source against its packet, including expanded helpers and
   normalized inputs. Identify the exact expression or operation in question.
2. Compare the relevant original decompilation **and disassembly** through the
   Ghidra MCP before changing `src/b-witch/` behavior. Resolve vtable targets and
   type/layout questions where needed, and record the original function address
   and supporting instruction addresses.
3. Record a confirmed difference, an expected representation difference, or an
   unresolved question with the supporting evidence. Use a narrowly hashed
   reviewed exception only when that evidence justifies it; do not suppress a
   whole class of constants, offsets or calls based on one example.
4. For a confirmed fix, run the relevant build and meaningful regression checks,
   then regenerate the comparison. A remaining lexical signal can still need
   manual evidence even after the behavior is fixed.

`--filter` and `--limit` select source functions before comparison; they do not
select candidates from an existing report. Every comparison writes
`inventory.json`, `report.json`, `report.md` and packets under `--out`, including
filtered runs. Give a focused run its own output directory to retain full-folder
coverage:

```powershell
npm --prefix tools/local_renamer run audit -- compare --filter CActorWolfen:: --limit 10 --out out/function-audit-wolfen --snapshot out/function-audit/ps2.jsonl
```

## Whole-folder workflow

From the repository root, install the existing tooling dependencies if needed:

```powershell
npm --prefix tools/local_renamer ci
npm --prefix tools/local_renamer run audit -- inventory
```

This writes `out/function-audit/inventory.json`, including functions without
locals and inline functions in headers. In Ghidra's Script Manager, add
`tools/ghidra-module/totala/ghidra_scripts` as a script directory and run
`ExportFunctionAudit.java`. Select the generated inventory, then save the output
as `out/function-audit/ps2.jsonl`. This exports all matching original functions
using one decompiler instance. It records individual decompilation failures and
checks for cancellation between functions. An interrupted export leaves a `.tmp`
file; rerunning the batch export restarts it.

Then run:

```powershell
npm --prefix tools/local_renamer run audit -- compare
```

Open `out/function-audit/report.md` for a summary and unresolved candidate table.
Reviewed expected differences are omitted from this queue and retained in
`reviewed.md`. Functions which could not be compared are listed in `coverage.md`;
the summary still includes every coverage status. Splitting these views does not
reduce the number of candidate functions.
`report.json` includes every inventoried function and its coverage status:

- `candidate`: heuristic differences to review;
- `no_findings`: compared successfully, no implemented heuristic fired;
- `reviewed`: the only signals were previously reviewed expected differences,
  with matching function and dependency hashes;
- `unmatched`: no exact symbol/address counterpart in this snapshot;
- `ambiguous`: multiple originals match; overloads are never guessed;
- `decompile_failed`: Ghidra could not supply a function body;
- `parse_error`: a body could not be fully parsed, so comparison was skipped.

An unmatched function may be a port-only helper, a renamed original, or absent
because a filtered snapshot was used. The function inventory is not guaranteed
complete for malformed C++; completely unrecognizable definitions cannot be
enumerated. Inspect coverage instead of interpreting a small candidate count as
a clean bill of health.

## Small live capture / resumable capture

With the existing Ghidra bridge enabled on `127.0.0.1:18437`:

```powershell
npm --prefix tools/local_renamer run audit -- capture --filter CActorWolfen:: --limit 10
npm --prefix tools/local_renamer run audit -- compare --filter CActorWolfen:: --limit 10
```

Omit the filter and limit to capture the whole folder. Live capture caches symbol
queries and successful decompilations under the output directory, so rerunning
after interruption reuses completed work. Batch export is faster for a first
whole-folder run. `--refresh` refreshes live caches after Ghidra names, types or
analysis change; the program identity alone does not detect those edits. Failed
decompilations are retried on the next live capture.

`--root`, `--out`, and `--snapshot` accept paths relative to the repository root
or absolute paths. Use `--help` for all options. Functions match by fully qualified
name or a `FUN_XXXXXXXX` address suffix. Renamed functions need corresponding
Ghidra names before they can match. Overloads remain explicitly ambiguous.

## Keeping model review cheap

`packets/<function-id>.json` contains the source, expanded source, helper origins,
PS2 decompilation, original address, program identity, and detected signals.
Source-content hashes in the IDs keep changed functions distinct. These packets
can be handed to a local model or a cheaper hosted model without running an
interactive agent for each function. The tool itself does not contact a model or
spend API credits.

Ask a reviewing model to return only suspected behavioral differences, cite the
exact source and PS2 expressions, and distinguish uncertainty from a concrete
counterexample. Check its findings against disassembly before changing game
behavior. For broad coverage, models can also inspect packets marked
`no_findings`; the deterministic checks deliberately cover only a few classes of
differences. Packet files are regenerated from current source on each comparison;
the report lists the packets belonging to the current run. Old packets can remain
in the output directory, so consume report IDs rather than globbing all packets.

## Noise filtering (version 2)

The comparison now applies these conservative normalizations, recording them in
each result's `normalizations` and retaining both raw inputs in its packet:

- Exclude assertions and known logging/profiling calls together with their
  arguments, so diagnostic-only constants and calls do not become candidates.
- Resolve unambiguous object-like integer macros from the scanned source/header
  files. Conflicting definitions, function-like macros and unsupported expressions
  are left unresolved. This is not a compiler preprocessor or include resolver.
- Recognize named calls through Ghidra's explicit vtable-pointer syntax.
- Exclude named casts from call counts and compare nested argument calls before
  their enclosing call, accommodating Ghidra's temporary assignments.
- Decode `(float)&DAT_XXXXXXXX` as the exact float32 bit pattern, using a decimal
  literal which round-trips to those bits. Ordinary memory loads are not decoded.
- Canonicalize `edMemAlloc` item-size expressions, including shifts and sums of
  `sizeof` for known fixed-width PS2 scalar types. Pointer/class sizes are not
  guessed, and differing item sizes still produce findings.

Packets also contain `normalizedSource` and `normalizedOriginal` for inspection.
`report.json` records `filtersVersion` and the number of resolved numeric macros.
Unresolved indirect-call count differences have their own signal and remain in
the queue, even if other formatting differences were normalized away. A fixed
function can still need review when its original call target is unresolved.
Normalization reduces noise; it does not establish semantic equivalence or
replace disassembly checks.

Version 3 also canonicalizes subtraction of an integer literal, so `state - 5`
and `state + -5` have the same constant fingerprint. A changed sign or magnitude
still produces findings.

Version 4 adds rules that apply across the entire inventory:

- Resolve unambiguous global, unscoped integer enum values, including implicit
  increments and supported constant expressions. Scoped or nested enums,
  conflicting values, unsupported expressions and locally shadowed names are
  left unresolved. This handles enum message IDs such as
  `MESSAGE_GET_VISUAL_DETECTION_POINT`, rather than excepting individual callers.
- Canonicalize Ghidra floating-point operation names `ABS`/`SQRT` and native
  `fabs`/`fabsf`/`sqrtf`. Calls, their arguments, nested calls, order and constants
  remain in the comparison. Integer `abs`, double `sqrt`, object methods and
  custom namespace functions are not folded into these identities. This compares
  operation fingerprints; it does not prove floating-point edge-case equivalence.

The math-name rule was checked against `CActor::SetupDefaultPosition`
(`00105350`: `abs.S` at `0010539c`, `001053e0`, `00105418`) and
`CActorAton::ChooseWaitJumpCallState` (`00387510`: `sqrt.S` at `0038759c`).

Every applied rule remains in `normalizations` and both raw bodies remain in the
packet. `report.json` records `numericEnumCount` separately from
`numericMacroCount`. No new per-function review exceptions were added for the
fourth sample. Its wrapper, field-offset and packet-macro findings remain until
a general rule can account for their supporting evidence.

The version 4 full-folder comparison is saved in
`out/function-audit-filtered-v4/report.md`. Against the 23 October baseline, the
general rules reduce candidates from **2,274 to 2,168**; `no_findings` rises from
3,027 to 3,133. All 7,486 functions remain inventoried and coverage-gap counts
are unchanged. `CScene::~CScene` remains a candidate. The exception ledger is
unchanged; this reduction comes from the general rules, not additional reviews
or the separation of Markdown views. The queue still contains substantial noise.

## Structural and build-aware filtering (version 5)

Three additional general rules use source declarations and the selected build:

- Model default base construction when a unique, non-virtual single-inheritance
  declaration and the original constructor's `this` receiver identify the base.
  Explicit base arguments and member-initializer expressions remain in the
  fingerprint. Duplicate original calls and changed values remain findings;
  multiple/virtual inheritance and conflicting declarations are not guessed.
- Expand small, unambiguous forwarding functions and return accessors. Class
  fields establish receiver types, and substitutions bind fields to that
  receiver. Qualified base calls may bypass virtual dispatch; unresolved virtual
  calls, overloads, side-effecting actual arguments, repeated parameters, local
  bindings and complex bodies remain unexpanded. Already matching original
  helper calls are retained rather than unnecessarily expanded.
- Expand supported expression macros using **Clang's active macro definitions at
  the start of the function's source definition**. This observes includes, conditional build
  definitions, `#undef` and redefinitions instead of collecting mutually exclusive
  definitions across the repository. Original PS2 code is never rewritten using
  Windows macros. Statement macros, token pasting, stringification, variadics,
  side-effecting arguments, recursive macro cycles and excessive expansion are
  left unresolved. Functions containing their own macro-state changes are not
  expanded using the entry snapshot.

The macro pass reads `build.ninja` and `CMakeFiles/rules.ninja` from
`out/build/x64-debug` by default, extracts the `Kya` Clang compile arguments, and
runs preprocessing only (`-E -dD`). It does not build the game or execute Ninja
command strings. The configured compiler and include dependencies must be
available. A failed preprocessing invocation contributes no partial macro state;
the comparison retains its ordinary lexical signals.

Use `--build-dir <path>` to select another configured Ninja build, or
`--no-build-macros` for the compiler-independent comparison. Header-only functions
without their own compile rule retain lexical macro handling. `report.json`
records the selected build directory, the build-settings hash, preprocessed file
count and errors in `buildMacros`. Expansion notes and normalized bodies remain
in each packet. Hexadecimal floating-point constants are decoded and retained,
so changed trig scale or phase values cannot disappear from literal counts.

These rules can expose additional differences hidden behind macros as well as
remove representation noise. Matching operation counts still do not establish
equivalence, and a function may retain signals for unresolved global data values
or layouts after its helper/macro spelling differences are explained.

When the active `sqrtf` macro is exactly the port's `sqrtf(fabs(x))` shim, the
original PS2 `SQRT` intrinsic is modeled with the same magnitude operation.
This follows the negative-operand handling in
[PCSX2's EE `SQRT_S` implementation](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/FPU.cpp).
It does not rewrite original libc calls or model FPU status flags, signed-zero
and denormal behavior. Modified shim arguments and constants remain findings.

The full version 5 comparison is in `out/function-audit-filtered-v5/report.md`.
It clears **25** version 4 candidates and exposes **five** previously hidden
`rand`/`randImpl` wrapper differences, for a net change from **2,168 to 2,148**
candidates. Those new differences remain unresolved, rather than being assumed
equivalent. The rules apply in 18 functions with implicit base construction,
60 with transparent helper expansion and 425 with expression-macro expansion;
many still have other signals. Clang preprocessing succeeded for all 193 selected
translation units with compile rules.

All 7,486 functions remain inventoried, coverage-gap counts are unchanged, and
all 97 implementation-guard candidates remain visible. The scene destructor and
money load-context unresolved-call findings remain candidates. The minigame
constructor and DCA end helper now report `no_findings`; the axis-angle matrix
function retains its two global-data-versus-literal signals after macro expansion.
The exception ledger is unchanged. The 34 tooling tests and typecheck pass;
all 124 constructor packets were also checked against the final constructor
model. No game source was modified or game build required for this tooling pass.

To reproduce that comparison:

```powershell
npm --prefix tools/local_renamer run audit -- compare --out out/function-audit-filtered-v5 --snapshot out/function-audit/ps2.jsonl
```

## Reviewed exceptions

`tools/function-audit-reviewed.json` holds narrowly reviewed exceptions for
differences which need manual type/layout or disassembly evidence. Each entry
specifies the original program and address, exact findings, a reason, SHA-256
hashes of both function versions, and hashes of relevant dependency headers.
Line endings are normalized to LF for hashing; other source changes invalidate
the review. If the original decompilation or any dependency changes, the finding
returns to the queue. New or unrelated findings are never suppressed by a review.

These are marked `reviewed`, rather than silently treated as automatic matches.
The original findings and reasons remain in `reviewedFindings` in the JSON report
and packet. For example, the reviewed commander's 24-byte stride does not cause
other occurrences of 24, or other array loops, to be ignored. The particle setter
has a reviewed field-offset difference only for its fixed implementation; its
old implementation guard does not match the review hashes.

Use `--reviews <path>` to select a different schema 1 ledger. An empty `reviews`
array disables manual exceptions. The program name and image base must match the
snapshot before any exceptions are applied. No comparison requires live Ghidra
access once the snapshot and review evidence have been captured.

Validate the tooling with:

```powershell
npm --prefix tools/local_renamer test -- tests/functionAudit.test.ts
npm --prefix tools/local_renamer test -- tests/auditStructure.test.ts
npm --prefix tools/local_renamer run typecheck
```
