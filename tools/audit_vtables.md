# PS2 virtual override audit

`audit_vtables.py` inventories virtual classes under `src/b-witch`, discovers
their original PS2 tables through the open Ghidra program, and compares changed
slots against source declarations. It runs without an agent, Python packages,
or changes to the Ghidra project. Python 3.10+ is required.

## Run

Open `SLES_514.73` in Ghidra with the existing Totala bridge enabled, then run
from the repository root:

```powershell
python tools/audit_vtables.py
```

The default bridge endpoint is `127.0.0.1:18437`, the Ghidra bridge rather than
the game debug socket. Use `--port` for another configured bridge. Keep the same
program selected during capture. The checker accepts only the 32-bit,
little-endian PS2 R5900 language.

Outputs under `out/vtable-audit/`:

- `report.md`: findings followed by a class-by-class coverage table.
- `report.json`: source declarations, table addresses, slots, target bytes,
  function metadata, and coverage limitations.
- `ghidra-snapshot.json`: cached read-only bridge responses and program identity.

The first capture takes a few minutes. Later runs reuse those responses. Recheck
source edits without Ghidra or network access:

```powershell
python tools/audit_vtables.py --offline
```

Use `--refresh` after Ghidra annotations change, or a separate `--snapshot` path.
The checker refuses to reuse a snapshot belonging to another executable,
image base, or language. A saved snapshot is a historical capture: it does not
automatically reflect later Ghidra edits.

Source-only inventory and focused checks:

```powershell
python tools/audit_vtables.py --inventory-only
python tools/audit_vtables.py --class CActorMicken --class 'CBehaviourMicken*'
```

Class filters include source ancestors automatically. To preserve the complete
default report when doing a focused check, pass another `--output` directory.
Use `--source` to audit another source tree, and `--max-slots` to increase the
default 256-slot safety cap.

## Tables absent from constructor references

Base constructors are often inlined. The checker collects **all** named table
references encountered in derived constructors, including base and embedded
behaviour tables. It then captures every uniquely discovered source class table.

Some tables have no discoverable named constructor. To include all table labels
in the project, run `ExportVtableMap.java` from Ghidra's Script Manager. The script
is in `tools/ghidra-module/totala/ghidra_scripts/`; add this directory to Script
Manager's script directories if needed. Choose an output such as
`out/vtable-audit/table-map.json`, then run:

```powershell
python tools/audit_vtables.py --table-map out/vtable-audit/table-map.json
```

The exporter only reads symbols and writes JSON; it does not define functions,
rename symbols, or modify the project. It exports `_vt`, `vtable`, and `vftable`
labels with their full namespaces. Program identity is checked before use.
Pass the same table map when replaying its capture offline. Classes with multiple
tables, absent symbols, or unsupported layouts remain explicit coverage gaps.

## Interpretation

- `missing_override`: a child PS2 target differs from its parent, the inherited
  virtual name is mapped, and the source child has no matching member declaration.
  These are **review candidates**, not proof of a behavioral bug.
- `review`: the slot name/signature is ambiguous, a virtual-dispatch wrapper needs
  inspection, a parent table is missing, or source inheritance is complex.
- A source declaration without `virtual` or `override` still counts as an override
  of an inherited virtual. Overloads, parameter-type changes, and const differences
  are not silently accepted based on name alone.
- Simple unconditional MIPS jump thunks to the same implementation, equivalent
  tiny return bodies, and implicit destructor/free slots are excluded.
- An unnamed `jr ra` getter is retained even if Ghidra has no function definition;
  this is how the missing Micken wind getter can be detected from the parent slot.

Tables are read from their two-zero-word Metrowerks header until a zero entry,
unmapped/unrecognized target, or slot cap. The report records the boundary.
An internal null slot can stop a scan early; unusual headers, multiple inheritance,
secondary tables, external bases, templates, and unmapped names need manual review.

This is a balanced lexical declaration scanner, not a C++ compiler. It does not
evaluate conditional compilation, fully resolve typedefs/namespaces, verify return
covariance, or prove that a declared method is implemented correctly. Duplicate
class names are marked ambiguous. A zero-finding class means no discrepancy was
detected in the recovered comparable slots, not that its entire ABI was verified.
Unavailable classes are never reported as passes. Original classes may legitimately
have no emitted table, but that absence is not inferred from a failed lookup.

For automation, `--fail-on-findings` returns 1 for findings or incomplete capture;
usage/connection errors return 2. The default returns 0 after producing a report,
even when there are candidates or unavailable classes. `--inventory-only` performs
no Ghidra checks and should not be used as a verification gate.

## Tests

```powershell
python -m unittest discover -s tools -p test_audit_vtables.py
```

Tests cover declaration parsing, implicit overrides, signature ambiguity,
constructor-discovered base tables, unnamed getters, MIPS jump/virtual thunks,
capture bounds, and read-only offline replay. They do not launch the game or
the native test executable, and do not touch the game's logs.
