# Auditing float casts and bitwise expressions

Run from the repository root (Python standard library only):

```powershell
python tools/audit_float_bitwise.py
python tools/audit_float_bitwise.py src/b-witch/ActorPatternService.cpp
python tools/audit_float_bitwise.py --format json --output out/reports/float-bitwise-audit.json
python tools/audit_float_bitwise.py --min-confidence low
```

The scanner reports file, line, column, expression, and heuristic confidence:

- **High:** float conversion around integer casts combined with bitwise OR,
  comparison, and multiplication. Often a mistranslated floating-point min/max.
- **Medium:** float bit constants, or apparent float values cast to integers
  before bitwise operations. These also include deliberate quantization.
- **Low:** other float conversions of bitwise expressions. Unsigned integer
  conversion helpers and address arithmetic often appear here.

Comments, quoted strings, raw strings, and logical `||`/`&&` are excluded.
Parentheses are balanced to handle multiline expressions. Overlapping input
directories are deduplicated. Default input is `src/`, independent of the current
working directory; explicit paths are relative to the current directory.

Findings are candidates, not confirmed bugs. Check the containing function's
Ghidra decompilation **and disassembly**, record the PS2 address, and determine
whether the original used `min.s`, `max.s`, bit reinterpretation, or an intentional
integer conversion. Do not replace these expressions automatically.

This is not a C++ parser: it does not resolve variable types, macros, templates,
or inactive preprocessor branches. It scans parenthesized C float/double casts
and `static_cast<float/double>` arguments. It can miss implicit conversions,
unparenthesized C casts, typedefs, and selection patterns without bitwise operators.

Run scanner tests:

```powershell
python -m unittest discover -s tools -p test_audit_float_bitwise.py
```
