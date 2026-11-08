# Parser-failure investigation

The short-function survey's `parseError` flag comes from Tree-sitter source
discovery. It is separate from unsupported P-code, missing call graphs and solver
failures. Valid C++ can fail this discovery parser while compiling normally.

## Results, 5 November 2026

The current inventory has 8,324 source-definition records, of which 1,182 carry
Tree-sitter errors. The original under-20-line selection contains 158 of those
records. Production-configured Clang was used to investigate those 158:

| Discovery outcome | Records |
| --- | ---: |
| Valid compiler definition, matching qualified name and source range | 149 |
| Valid compiler definition needing template identity reconciliation | 1 |
| `PACK(...)` struct declarations incorrectly inventoried as functions | 8 |
| Total | 158 |

All 61 selected translation units parsed without compiler errors. For the 149
exact matches, Clang confirmed the original function line spans; none grew past
the 20-line cutoff. The same translation-unit run also matched 594 larger
functions previously flagged by Tree-sitter, including `CActor::Create`,
`CActor::InterpretMessage`, `CActorAutonomous::ManageDyn` and
`CActorBrazul::Manage`. This is not a complete investigation of all 1,182 records.

The template case is `PlatformHeader::Release` in `ActorMovingPlatform.cpp`:
Clang identifies its owner as `PlatformHeader<PlatformBehaviourType>`. A generic
template definition still needs a concrete instantiation/emitted symbol before
compiled verification. Another template, `CNotificationTargetArray<T>::FUN_003c38c0`,
was recovered with its qualified template name intact.

## Main causes

1. **Comma expressions in conditions.** The installed Tree-sitter C++ grammar
   rejects this valid minimal example:

   ```cpp
   void F() { if (a && (b = c, b != 0)) {} }
   ```

   This pattern is common in the recovered Ghidra-style code. Among the 158
   records, 112 contain an assignment/comma pattern in their source. That is a
   feature count, not an exclusive attribution of every error.

2. **Typed macros.** `LOAD_POINTER_CAST(Type*, value)` looks like a call with a
   type as an expression argument before preprocessing. Eighteen failing records
   contain this macro. `IMPLEMENTATION_GUARD` and other statement macros also
   produce missing-semicolon diagnostics when parsed without their build-time
   expansions. Sixteen contain an implementation guard; categories overlap.

3. **Pointer-array allocation.** `new BonePhysicsEntry*[count]()` fails the
   installed grammar in `CActorBonePhysics::SetObjCounts`. This has a matching
   [upstream grammar issue](https://github.com/tree-sitter/tree-sitter-cpp/issues/289).

4. **Declaration macros.** Eight `PACK(...)` declarations are mistaken for
   function definitions. A compiler AST identifies them as types and never emits
   a function named `PACK`.

There is also a legacy non-UTF-8 comment byte in `CLinkConstraint::Resolve` in
`ActorBridge.cpp`. The compiler accepts the source. The discovery tool preserves
original byte offsets/hashes and retains hexadecimal source bytes for affected
definitions; replacement decoding is only for display. No source was re-encoded.

## What the next verification stage needs

Of the 149 exact compiler matches:

| Next stage | Functions |
| --- | ---: |
| Outside the generic runner's scalar/static-ABI signature subset | 23 |
| Require compiled relocation/dependency support | 104 |
| Can be bundled for lifting with the current relocation policy | 22 |

This eligibility audit used existing production COFF objects and is not an
equivalence check. The 22 bundle candidates still need PS2 identities, ABI and
memory domains, instruction support and complete path proofs. Examples include
`CAnimation::PhysicalLayerFromLayerId`, `CNewFx::Stop`, `CActorNativ::CanSpeak`,
`CActorFighter::SetStandAnim` and the `edStrCmp`/`edStrICmp`/`edStrnCopy` functions.
Some overlap existing dedicated verification suites; they are not necessarily
simple leaves. String routines need buffer/loop domains; virtual calls need
explicit target sets.

## Recommended implementation

Keep Tree-sitter as the fast inventory pass, with a **compiler AST fallback** for
failed definitions. Use the production include paths, defines, language mode
and target configuration. Clang provides source ranges, semantic owner names,
return/argument types and mangled symbols. Its
[C API](https://clang.llvm.org/doxygen/group__CINDEX.html) exposes these directly;
[compilation commands](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
provide the configuration needed to interpret each translation unit correctly.

An accepted fallback should retain its compiler/build identity, exact source
range and byte hash, preprocessing configuration and emitted-symbol identity.
Compiler errors must reject the entire translation unit's proposed definitions.
Inactive branches, macro-generated declarations, overloads and uninstantiated
templates need explicit accounting. Header definitions need an actual consuming
translation unit. The verifier can then continue comparing complete COFF bodies
with PS2 P-code; it does not need a source-expression interpreter.

This avoids modifying recovered game code to accommodate a parser. Merely
clearing the Tree-sitter error flag would lose the compiler validation and source
boundary checks. Grammar upgrades may help individual patterns, but should be
measured on the retained corpus rather than assumed to resolve macro handling.

## Tools and reproduction

`inventory.mts` now records up to 16 parser diagnostics per failing function,
including missing punctuation, source line/column and a short text excerpt.
`small_functions.py` carries those diagnostics into new reports. The local
renamer's default extraction behavior remains unchanged.

`clang_discovery.py` is a read-only compiler discovery tool using the
installed production compiler's `libclang.dll` through Python ctypes. No Python
Clang package or dependency upgrade is needed. The short-function runner now
invokes it automatically and attaches accepted definitions to a separate
`recovered-inventory.json`; original Tree-sitter verdicts remain intact. See
[the survey instructions](SMALL_FUNCTIONS.md) for fallback controls and artifacts.
Discovery acceptance does not establish equivalence or bypass the existing
signature, dependency, lifting and proof gates.

From the repository root in a Visual Studio x64 developer shell:

```powershell
node --import ./tools/local_renamer/node_modules/tsx/dist/loader.mjs tools/function_verify/inventory.mts src out/verification/parser-investigation/inventory.json
python tools/function_verify/clang_discovery.py --inventory out/verification/parser-investigation/inventory.json
```

`--source src/b-witch/Actor.cpp` focuses on one production translation unit.
`--all-failures` includes definitions above the default line cutoff.
`--max-tus` limits an exploratory run; unexamined records remain explicit.
Exit 1 records a translation-unit failure; no definitions from that unit are
accepted. The report is discovery evidence, even on exit 0.

Local evidence is under `out/verification/parser-investigation/`:

- `inventory.json`: fresh source discovery with Tree-sitter diagnostics.
- `clang.json`: build/compiler identity, production commands, compiler diagnostics,
  recovered source spans, types, symbols and per-function outcomes.
- `next-gates.json`: the separate signature/COFF eligibility audit.
- `clang.log`: progress and translation-unit results.

Validation passed: TypeScript type checking, all 25 function-audit tests, four
real-libclang discovery tests and the 17 existing short-function verification
tests. Compiler tests cover comma expressions, pointer arrays, macro declarations,
conditional compilation, overload/template identities, exact UTF-8/legacy byte
ranges and rejection of partial ASTs after compiler errors.
