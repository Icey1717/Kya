# Broader compiled validation

The initial control-flow pilot has expanded to 26 compiled function/domain checks.
The four comparison suites are actor flags (4), branches/calls/loop (5),
bytecode/save/HUD (8), and collision/combat (9). The SetStandAnim check has a
restricted non-dispatch domain; full transition coverage remains required
separately and currently fails. All compare real Windows
object-code P-code against the original PS2 executable; none use C++ source
behavior expressions as their expected result.

## Daily development

Run `python tools/function_verify/check.py --watch` while editing. Keep the
original PS2 program open in Ghidra with Totala enabled. The watcher derives
its translation-unit list from the selected cases and follows Ninja's local
header dependencies, build settings, verification scripts, and fixtures.
See [build and watcher instructions](COMPILED.md) for paths and overrides.

Configure `cmake --preset x64-verify` in a Visual Studio x64 developer shell.
Python tools default to that logging-disabled build and artifacts under
`out/verification/x64-verify`; CMake targets write inside their build directory.
Use `--build-dir` and `--output-root` for explicit alternatives. Reports identify
the logging setting and build cache hash.

Read `coverage.md` in the selected output root for a combined function/domain table, or
`coverage.json` for automation. `check-status.json` describes the current/last
live run. The checker requires the 26 function/domain proofs and two complete combat-transition
targets to be present and
equivalent; missing reports, missing functions, mismatches, unsupported behavior,
and incomplete proofs fail. Old reports are removed before each suite starts.
Runs use a process lock to prevent manual and watcher jobs mixing their artifacts.

## CTest and CI

Offline fixture regressions need Python and the pinned Z3 dependency, without
Ghidra, a game build, or the original executable:

```powershell
python -m pip install --target out/verification-deps -r tools/function_verify/requirements.txt
python tools/function_verify/check.py --offline
```

Use those two commands in CI and retain `out/verification/offline.log` and
`offline-status.json` on failure. Offline success means regression fixtures
passed; it does not prove that newly edited C++ still matches PS2 code.
It never overwrites live coverage/status with fixture-only results.

For a shorter edit/check cycle use `--offline --regression-tier fast`.
`--offline --regression-tier retained` selects the expensive fighter runtime,
animation and state-entry replays. These tiers form a disjoint, exhaustive
partition; omitting the option still runs everything. Tier-specific status/log
files include `-fast` or `-retained` and cannot replace the complete offline
status. Tier selection is rejected for live runs.

To include offline regressions in the normal CTest workflow:

```powershell
cmake --preset x64-debug -DKYA_ENABLE_FUNCTION_VERIFICATION_TESTS=ON
ctest --test-dir out/build/x64-debug -L verification --output-on-failure
```

This option registers `FunctionVerifyOffline` (`verification;offline;fast`) and
`FunctionVerifyRetained` (`verification;offline;retained`), using separate output
directories so CTest can schedule them independently. Run the `verification`
label to include both tiers.
`KYA_ENABLE_LIVE_FUNCTION_VERIFICATION_TESTS=ON` additionally registers
`FunctionVerifyLive`, labeled `verification;live`. Enable that only on workers
with the configured Windows compiler/build, Ghidra 10.0, JDK, extension, and
running PS2 bridge. Live runs rebuild and relift current production objects.
Live tests have an 1800-second timeout; fast offline tests have a 900-second
timeout, and retained replays have a 1200-second timeout.
Stop an active watcher before running the live CTest job to avoid contention.
Offline jobs use their own artifact lock and can run while the live watcher is
working. Restart the watcher after changing its orchestration or case registry
code so its in-memory configuration is refreshed.

Both explicit build targets also remain available: `verify-functions` and
`verify-functions-offline`. No live proof is attached to the ordinary game build.

## New subsystem coverage

| Function | Original PS2 entry | Mapped field: Windows / PS2 |
| --- | --- | --- |
| ByteCode::GetU32 | 00189b50 | currentSeekPos: 24 / 12 |
| ByteCode::GetS32 | 00189b70 | currentSeekPos: 24 / 12 |
| ByteCode::SetPosition | 00189be0 | currentSeekPos: 24 / 12 |
| CSaveManagement::clear_slot | 002f5220 | slotID_0x28: 40 / 40 |
| CMagicInterface::HasMagicInteractionAround | 001dcdc0 | bHasMagicAround: 12 / 8 |
| CMagicInterface::SetHasMagicInteractionAround | 001dcdd0 | bHasMagicAround: 12 / 8 |
| CLifeInterface::GetPriority | 001dc990 | priority: 8 / 4 |
| CLifeInterface::SetPriority | 001dc9a0 | priority: 8 / 4 |

Windows offsets above are the initial compiler-probed values and are recomputed
from the project headers each run. PS2 offsets were checked against live raw
P-code/disassembly and decompilation. Function identities and mappings are in
`subsystems.py`; retained exports are in `fixtures/compiled_subsystems.json`.

The readers start with an aligned readable uint32 at modeled address 0x4000.
All 32 payload bits are symbolic. Both returned low 32 bits and the resulting
cursor are compared, along with preservation of the payload and all other
observable memory. Windows pointers map by zero-extension of PS2 pointer
identities; these checks do not cover arbitrary native 64-bit address ranges,
buffer exhaustion, invalid pointers, or faults. Other subsystem cases quantify
over all 32-bit initial field and setter argument values. Setters and clear_slot
compare side effects without assigning meaning to a void return register.

The compiled cursor negative control is:

```powershell
python tools/function_verify/compiled_extended.py --suite subsystems --mutate cursor
```

It compiles a disposable MemoryStream.cpp with GetU32 advancing eight bytes
instead of four. The live comparison detects different final cursors even when
the returned values agree; the other seven functions still match. Exit 1 is
expected. Repository game source remains untouched.

## Extending coverage

Add production symbol identities to the case list, compiler-derived layout
constants to the probe, and explicit PS2 ABI/field mappings. Share symbolic
inputs between the two machine-code executions. Do not supply a handwritten
source algorithm as an expected output. Retain real exports, add a meaningful
negative control, and require both path coverage and unreachable loop cutoffs.

This is bounded, explicit coverage rather than whole-project equivalence.
Linked executable/LTO effects, floating-point semantics, general pointer graphs,
and arbitrary call graphs remain outside the current proof scope.

See [the collision/combat report](COMBAT.md) for the five added targets,
restricted memory/VU support, the corrected stagger predicate, and the explicit
remaining blockers for full state-transition coverage.
