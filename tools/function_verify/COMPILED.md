# Compiled Windows versus original PS2

This workflow compares machine-code semantics on both sides. It rebuilds the
production Windows translation units through their configured Ninja
targets, extracts actual x64 function bytes from the COFF objects, lifts them with
Ghidra 10.0, and compares them against live original PS2 P-code using Z3.
No handwritten C++ behavior expressions or source-body regex checks supply
the expected output in this workflow.

## Run

Use `cmake --preset x64-verify` in a Visual Studio x64 developer shell to configure
the logging-disabled build. The scripts default to `out/build/x64-verify` and
`out/verification/x64-verify`; override with `--build-dir` and `--output-root`.
Results record this build identity and apply to that configuration.
Use the installed Ghidra 10.0,
JDK 21, and Totala extension from this pilot. Python and Z3 dependencies are the
same as the original suite. From the repository root:

```powershell
python tools/function_verify/check.py
```

This runs offline regressions and four live compiled suites (twenty-two functions).
Every command must succeed; mismatches, unsupported operations, incomplete loop
proofs, bridge failures, and solver timeouts fail the check. Logs are
`out/verification/check-0.log` through `check-4.log`; `check-status.json` records
running/passed/failed state and timestamps. Per-function reports are in
`compiled/report.json`, `compiled-extended/report.json`, and
`compiled-subsystems/report.json`, and `compiled-combat/report.json` beneath that directory. `coverage.md` and
`coverage.json` combine the exact expected function sets and their domains.
A zero exit without a fresh, complete report fails validation. Failed suites
cannot reuse an older successful report.

For continuous checking while editing:

```powershell
python tools/function_verify/check.py --watch
```

The watcher runs immediately, then after changes to the eleven translation units,
their Ninja-recorded local header dependencies, CMake files/configuration, or the
verification/exporter tools or retained fixtures. It debounces saves, runs serially, and reruns if
inputs change during verification. Stop with Ctrl+C. It continues watching after
a failed check; fix the code or restore the bridge and save a watched file to
retry. It does not monitor external SDK/compiler installation changes; restart
it after changing those. Keep the PS2 program open in Ghidra. `--port`, `--ninja`,
`--ghidra`, and `--java` can override local defaults.

For a watcher started in the background, request a graceful stop with
`python tools/function_verify/check.py --stop`; it finishes any current check
before exiting. Runs use a process lock to protect shared artifacts: a competing
manual check exits 2, and a competing watcher waits before retrying. Start only
one watcher. To start it in the background on Windows:

```powershell
$python = (Get-Command python).Source
Start-Process -FilePath $python -ArgumentList 'tools/function_verify/check.py --watch' `
    -WorkingDirectory (Get-Location).Path -WindowStyle Hidden `
    -RedirectStandardOutput out/verification/watch.log `
    -RedirectStandardError out/verification/watch-errors.log
```

CMake also provides explicit targets after configuration:

```powershell
cmake --build out/build/x64-debug --target verify-functions
cmake --build out/build/x64-debug --target verify-functions-offline
```

They are explicit targets because live proofs require a running local Ghidra
bridge. Normal game builds do not launch verification or a background watcher.
The offline target is suitable for automation without Ghidra, using the retained
real compiler-output fixtures. Z3 must be installed as described in README.md.
The [validation guide](VALIDATION.md) documents opt-in CTest registration and the
offline CI command. Offline results use `offline-status.json` and cannot replace
the latest live proof status.

To run an individual live suite:

```powershell
python tools/function_verify/compiled.py
python tools/function_verify/compiled_extended.py
python tools/function_verify/compiled_extended.py --suite subsystems
python tools/function_verify/compiled_extended.py --suite combat
```

The live PS2 program must be open with the bridge on port 18437; `--port` selects
another port. `--ghidra`, `--java`, and `--ninja` override local installation paths.
The plugin's locally built `tools/ghidra-module/totala/build/ghidra10/totala.jar`
is required; [build instructions](../ghidra-module/totala/README.md) reproduce it.
No plugin reinstall is needed. The exporter runs from the repository script path.

`--export-only` prepares Windows exports without querying the live bridge.
To repeat comparison of prepared artifacts:

```powershell
python tools/function_verify/compare_binaries.py
```

Build and lift again after source/header or build-setting changes. Ninja performs
the normal dependency checks during `compiled.py`; the standalone comparison
checks the recorded object and source hashes but does not rerun dependency analysis.

## Evidence and proof scope

Artifacts live in `out/verification/compiled/`: extracted bytes, compile commands,
source/object/function hashes, compiler version, layout constants, P-code exports,
logs, and `report.json`. Ghidra runs in a disposable project with its own user home
and extension cache inside this directory. The running PS2 project is unchanged.
Ghidra's bundled Python scripting avoids its Java-script OSGi compiler's Java 21
compatibility issue.

| Windows function | PS2 entry | Compared observations |
| --- | --- | --- |
| CActor::IsKindOfObject | 00100460 | bool return, flags preservation, other memory preservation |
| CActor::IsLookingAt | 001003f0 | uint return, flags preservation, other memory preservation |
| CActor::SetLookingAtOn | 00100400 | flags and other memory preservation |
| CActor::SetLookingAtOff | 00100410 | flags and other memory preservation |

All four passed on the initial run. The Windows executor models raw P-code,
including condition flags, parity/popcount, partial registers, stack spills,
return-address loading, and frame restoration. Unsupported instructions or
uninitialized state fail closed. These four functions are straight-line;
The extended suite adds bounded Windows branch/call/loop exploration.

| Additional compiled function | PS2 entry | Domain |
| --- | --- | --- |
| CInventoryInterface::HasChanged | 001dc7b0 | All uint32 dirty values; bool return and final dirty field |
| CActorMoney::GetType | 00362f30 | All uint32 money values; int return |
| CActor::GetBehaviourFlags | 001022b0 | Signed state -1..1; arbitrary ACT table contents |
| CActorAutonomous::GetBehaviourFlags | 00123c50 | Signed state -1..2; actual direct call into the base implementation |
| CFixedTable<CActor*,4>::IsInList | 00131f10 | Count 0..4; arbitrary pointer identities and query |

The template function is taken from the production `ActorProjectile.cpp.obj`,
not a recreated loop or a verification wrapper. Ghidra names its original PS2
symbol `CFixedTable<CActor,4>::IsInList`. Windows count/entries offsets are
compiler-derived (initially 0 and 8); PS2 offsets are 0 and 4. Pointers map
injectively from arbitrary 32-bit identities to zero-extended 64-bit identities;
no pointee is dereferenced. The loop explores five Windows header visits and
four PS2 back-edge visits, proving every cutoff unreachable and every valid
input covered. An insufficient bound is an error, never a successful proof.

The call case uses actual caller/callee bodies on Windows and caller/thunk/base
bodies on PS2. Both return targets are checked; the final Windows stack and RBP
and the PS2 stack/return register must be restored. Memory bounds are proved
under each path guard. Both branch functions cover their entire uint32 input
range, and all cases check observable memory preservation as well as returns.

The Windows ABI supplies RCX=this and RDX=kind; AL carries a bool result and
EAX carries a uint result. Initial kind covers all 64-bit values; initial flags
covers all 32-bit values. Both instruction lifters are trusted. The object and
private stack are placed at fixed modeled addresses; no fault model is included.
The executor proves stack/frame/return-address restoration and disallows writes
outside approved object and private-stack regions. All memory outside the private
stack and the corresponding flags field must remain unchanged. Field bytes are
compared directly across architectures.

The Windows flags offset is obtained by compiling `__builtin_offsetof(CActor,
flags)` against the real project headers and compile flags in a tiny layout-only
translation unit. On the initial build it is 0x50. The PS2 flags offset 8 remains
an explicit audited mapping. Object identity and ABI mapping still need to be
specified; behavior models no longer do. The layout probe also checks compiled
`sizeof(ulong)` is 8.

The original four-case extractor rejects COFF relocations. The extended bundle
resolves only AMD64 REL32 relocations to the selected base function, ACT/AUT
tables, and unreachable assertion references. Addends and original/patched body
hashes are recorded. Assertion imports are traps: execution must prove those
paths unreachable under the declared state domains. Unknown relocation targets
and reachable unmodeled calls fail closed. Global table contents remain symbolic
and shared across architectures, rather than proving their initial binary values.
These checks apply to production compiler output before final linking.
They do not prove linked-executable behavior after linker/LTO transformations,
all C++ language-defined behavior, or equivalence of initial global data.
The normal thirteen-case source suite remains available through `verify.py`;
its results should not be described as compiled-code proofs.

The [collision/combat capability report](COMBAT.md) records newly supported
direct/tail call graphs, a symbolic collision-object pointer, restricted VU
identity operations, a recovered signedness bug, and concrete blockers that
still prevent full combat state-transition proofs.

## Real compiled negative control

```powershell
python tools/function_verify/compiled.py --mutate-mask
```

This creates a disposable `Actor.cpp` copy under
`out/verification/compiled-mutant`, changes only `IsKindOfObject`'s mask from 1
to 2, and compiles it with the same production command. Repository source is
unchanged. Its separately lifted machine code must mismatch for that function;
the three other compiled functions should still match. The command exits 1 for
this expected mismatch. Inspect `compiled-mutant/report.json` for inputs and
the differing return values. The initial control run met those expectations.

Offline regression tests retain actual positive and mutated exports. Run all
tests using:

```powershell
python -m unittest discover -s tools/function_verify -p test_*.py
```

They cover both compiler outputs, wrong layout mappings, partial-register behavior,
unbalanced Windows stack state, unknown instructions, missing returns, exact COFF
symbol extraction, and refusal of relocated functions, alongside the original
source-model tests.

The extended suite has three additional real compiled negative controls:

```powershell
python tools/function_verify/compiled_extended.py --mutate branch
python tools/function_verify/compiled_extended.py --mutate call
python tools/function_verify/compiled_extended.py --mutate loop
```

These compile disposable copies with, respectively, an incorrect money threshold,
an incorrect argument to the base call, and a search omitting the last entry.
Each must exit 1 with a mismatch for the changed function and equivalent results
for the other four. Artifacts live in `compiled-extended-mutant-<kind>`. The loop
control preprocesses the production TU before editing its template body, leaving
repository headers untouched. All three controls were detected in the live run;
their actual P-code is retained for offline regression tests.
