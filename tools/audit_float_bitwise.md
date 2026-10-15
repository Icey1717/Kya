# Auditing float casts and bitwise expressions

Run from the repository root (Python standard library only):

```powershell
python tools/audit_float_bitwise.py
python tools/audit_float_bitwise.py src/b-witch/ActorPatternService.cpp
python tools/audit_float_bitwise.py --format json --output out/reports/float-bitwise-audit.json
python tools/audit_float_bitwise.py --min-confidence low
python tools/audit_float_bitwise.py --include-reviewed
```

The scanner reports file, line, column, expression, and heuristic confidence:

- **High:** float conversion around integer casts combined with a comparison and
  bitwise OR, AND, or XOR. Includes comparison/multiply and negative-mask selections.
- **Medium:** float bit constants, or apparent float values cast to integers
  before bitwise operations. These also include deliberate quantization.
- **Low:** other float conversions of bitwise expressions. Unsigned integer
  conversion helpers and address arithmetic often appear here.

Comments, quoted strings, raw strings, and logical `||`/`&&` are excluded.
Balanced delimiters and bounded unary operands handle multiline expressions,
unparenthesized C cast chains, member access, calls, and indexing without pulling
in unrelated arguments or statements. Functional casts such as `float(bits)` and
`double(bits)` are also scanned. IEEE sign, mantissa, and exponent masks and common
float/double bit patterns are recognized, including integer literal suffixes.
Overlapping input
directories are deduplicated. Default input is `src/`, independent of the current
working directory; explicit paths are relative to the current directory.

Findings are candidates, not confirmed bugs. Check the containing function's
Ghidra decompilation **and disassembly**, record the PS2 address, and determine
whether the original used `min.s`, `max.s`, bit reinterpretation, or an intentional
integer conversion. Do not replace these expressions automatically.

The nine intentional conversions reviewed in the first Ghidra audit are recorded
in `tools/float-bitwise-reviewed.json`, including reasons and PS2 function addresses.
They are hidden by default; `--include-reviewed` shows them with their review notes.
Matching uses the source path and expression with whitespace removed, so line
changes do not invalidate a review but changed values or operands do. Identical
expressions in the same file share an exception. To inspect all such expressions,
including any newly added identical copies, use `--include-reviewed`.
`--reviewed path/to/file.json` selects a different set of documented exceptions;
an empty set is `{"reviewed": []}`. The report includes a `reviewed_count`.

This is not a C++ parser: it does not resolve variable types, macros, complex
templates, typedefs, or inactive preprocessor branches. Implicit conversions are
left to the project's enabled compiler warnings. Selection patterns without
bitwise operators, float bits first stored in intermediate integer variables,
and casts through aliases can still be missed. Unary address-of and binary AND
can produce low-confidence false positives.

Run scanner tests:

```powershell
python -m unittest discover -s tools -p test_audit_float_bitwise.py
```
