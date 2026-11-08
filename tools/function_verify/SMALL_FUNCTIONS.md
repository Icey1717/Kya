# Short-function compiled survey

The [5 November survey and reviewed findings](../../docs/function-verify-small-functions.md)
record the first complete run and its coverage limits.

See [the parser-failure investigation](PARSE_FAILURES.md) for compiler-backed
recovery evidence and the distinction between parsing and proof support.
See [reviewed PS2 identities](PS2_IDENTITIES.md) for the fixed 426-definition
cohort, address mappings, host-helper dispositions and remaining obligations.

`small_functions.py` inventories **all of `src/`**, including inline header
definitions, and accounts for every function below an exclusive physical line
limit. The default is fewer than 20 lines, counting the signature, braces,
comments and blank lines inside the definition. This is a discovery threshold,
not a measure of semantic complexity.

Run in a Visual Studio x64 developer shell with the PS2 Totala bridge running:

```powershell
python tools/function_verify/small_functions.py --ps2-elf 'D:/path/to/SLES_514.73'
```

The parser reuses the installed Tree-sitter/tsx dependencies in
`tools/local_renamer/node_modules`. Python/Z3, Ghidra and Java requirements are
the same as [the compiled verifier](COMPILED.md). Installation paths, build
directory, bridge port and output directory have command-line overrides.

Tree-sitter remains the fast discovery pass. Failed short definitions now run
through `clang_discovery.py` automatically, using the selected production Ninja
build's compiler, include paths, defines and language flags. This requires the
same Visual Studio developer environment as the production build. Compiler
errors reject all proposed definitions from that translation unit. Use
`--no-clang-fallback` to retain Tree-sitter-only discovery.

Recovery requires matching literal source text and physical line boundaries.
When Tree-sitter omits a preceding template declaration, recovery retains the
compiler's complete range and recounts its lines before applying the cutoff.
The original parser errors and diagnostics remain recorded. Compiler evidence
includes exact byte ranges/hashes, qualified names, types, mangled symbols,
translation-unit commands, compiler version and build-cache identity. The survey
selects emitted COFF by the exact compiler symbol for recovered definitions.
Generic template owner names are reconciled for discovery, but remain outside
the runner's signature subset. `PACK(...)` struct false positives are excluded
only with positive compiler type evidence and listed in report metadata;
inactive, ambiguous and unexamined definitions remain unsupported.

Outputs default to `out/verification/small-functions/`:

- `report.md` and `report.json`: counts, every selected function, reasons for
  unsupported cases, mappings, input domains and counterexamples.
- `inventory.json`: parsed definitions and source spans across `src/`.
- `clang.json`, `recovered-inventory.json`: fresh production compiler discovery
  and the inventory carrying accepted compiler evidence.
- `ps2-identities.json`: live name/signature resolution.
- `layouts.json`, `layout/`: compiler probes, layout constants and compile logs.
- `batch-*/`: production function bytes, COFF identity/relocations, raw Windows
  P-code, raw PS2 P-code including instruction bytes, and headless lift logs.
- `build.log`: dependency-checked rebuild of the production `Kya` objects.

`--inventory-only` stops after discovery. `--reuse-inventory` skips parsing but
rejects changed source bodies. `--line-limit` changes the exclusive cutoff.
Compiler fallback runs afresh even with `--reuse-inventory`, so changed included
headers or preprocessing flags are checked. Prepared replay uses the recorded
recovered inventory and rechecks exact compiler source byte ranges.
`--filter` selects qualified function names with a regular expression, allowing
focused rechecks in a separate output directory.
There are no game source edits, automatic repairs, or Ghidra mutations.
`--identity-manifest` selects reviewed identities (the checked-in manifest is
the default). `--identities-only` refreshes live resolution and baseline
accounting without building or comparing; its exit status is 2.

To replay prepared exports after changing the survey comparator:

```powershell
python tools/function_verify/small_functions.py --compare-prepared --ps2-elf 'D:/path/to/SLES_514.73'
```

Replay checks recorded object hashes, source bodies and build-cache identity.
It also checks the identity manifest and revalidates reviewed mapped live
metadata, so replay of those mappings requires the bridge.
It does not rerun Ninja's header dependency checks or refresh live PS2 exports;
use the full command after game/header/build changes.

## Interpretation

| Verdict | Meaning |
| --- | --- |
| `equivalent` | Scalar returns and observable memory agree in the reported domain, without an inferred cross-architecture object-field correspondence. |
| `conditional-equivalent` | They agree under the recorded inferred single-field correspondence; the PS2 field identity still requires review. |
| `mismatch` | An explicit input/memory counterexample exists in the reported domain. Review ABI, field correspondence, runtime invariants and deliberate port changes before calling it a bug. |
| `unsupported` | No equivalence verdict. The report records the first unsupported identity, ABI, relocation, memory, instruction or path obligation. |

Exit 1 means mismatches were found. Exit 2 means incomplete/conditional coverage
without mismatches. Exit 0 requires every selected function to be unconditionally
equivalent, except that explicit inventory-only mode exits 0 after discovery.
Explicit reviewed field identities can be supplied to the Python `compare`
API; they must match the function, original entry, field name, PS2 offset and
size. The retained `interface-is-active.json` cases exercise this reviewed path.
The command-line survey continues to leave inferred mappings conditional.
These survey reports are separate from the hand-audited suites and do not
satisfy `check.py`'s required proof set.

The supported subset is integer/void member functions and ordinary global
functions. The first two explicit arguments can be integer scalars or pointer
identities below `0x80000000`; bool arguments are 0/1. Unused unsupported
arguments remain uninitialized, so reaching their values fails closed. Pointer
dereferences, globals, arbitrary object graphs, additional argument registers,
floating point and loops generally require a dedicated case. The existing
hand-audited suites can support cases this generic survey cannot.

Source text selects identities, argument widths, return width and candidate
field names. It never supplies expected behavior. Actual production COFF and
original raw PS2 P-code supply both behaviors. Field probes compile against the
production translation units with their production flags. A conditional mapping
requires exactly one non-bool integral source field, compiler-probed size and
offset, and a matching contiguous PS2 access span. Inferring correspondence from
accesses cannot establish the original field's identity or detect every wrong
field selection; those rows deliberately retain a conditional verdict.

The solver covers all stated scalar/field bits and checks path coverage,
observable returns, mapped final field bytes and preservation of every other
memory byte outside the private Windows stack. One-sided object access can also
expose missing/extra effects without mapping a field. Valid fixed object/stack
placement, Ghidra lifting and the stated ABI are assumptions. With `--ps2-elf`,
every exported instruction and lifted delay slot must match the hashed original
ELF bytes; patched Ghidra instructions are rejected. Without it, byte identity
with the executable remains an unchecked assumption.

Raw return comparison can flag representation differences: a PS2 integer getter
returning 8 and a Windows bool getter returning 1 differ as raw bits, even when
callers only care whether the result is nonzero. Arbitrary initial integral
fields can also violate runtime invariants. Do not silently suppress these
counterexamples or describe them as confirmed gameplay failures.

Tests use retained production exports, a real compiler mutation, and explicit
missing/extra-write controls:

```powershell
python -m unittest discover -s tools/function_verify -p test_small_functions.py
```
