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

Open `out/function-audit/report.md` for a summary and candidate table.
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
npm --prefix tools/local_renamer run typecheck
```
