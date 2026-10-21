# Special paste

Run `special_paste.ahk` with AutoHotkey v2, then press Ctrl+E to transform and
paste clipboard text. Python must be on PATH. Keep `transform.py` and
`symbol_index.py` and `vector_assignments.py` beside the AutoHotkey script.

Pastes use tabs, `Type* variable` pointer declarations, and a single newline
between function headers and opening braces. Whole functions get blank lines
after declarations, between initial values and following calls, and before
final returns. Existing LF/CRLF line endings are retained. Strings (including
raw strings), character literals, comments, and preprocessor directives are
protected from code rewrites.

Explicit calls on `this` retain their class qualifier, so
`CActor::IsProjectionAim((CActor*)this)` becomes `CActor::IsProjectionAim()`.
Virtual calls require one argument matching the owner of `pVTable`/`pVtable`;
ordinary function pointers and ambiguous receivers are left alone.
Ghidra's `(code *)` casts on virtual calls are supported. Simple member-path
parentheses are removed and commas get a following space.

C-style casts are preserved: choosing `static_cast` versus `reinterpret_cast`
requires type information this scanner does not have. Timer pointer locals
assigned once from `GetTimer()` or `Timer::GetTimer()` are replaced with that
lookup when all uses are member reads in the same straight-line block. Other
timer sources, value snapshots, and escaped pointers are preserved.

Consecutive x/y/z assignments collapse contextually using local declarations,
parameters, repository fields, and base classes. Each `edF32VECTOR3` operand
stays a whole object; each `edF32VECTOR4` operand uses its `.xyz` view so `w`
is preserved:

```cpp
// edF32VECTOR4 local; edF32VECTOR3 position;
local.xyz = position;
position = local.xyz;
local.xyz = local.xyz * scale; // scale is a known float
```

Four matching x/y/z/w writes between known `edF32VECTOR4` objects collapse to
whole-object assignment. Adjacent component loads through float temporaries
also collapse when those temporaries have no remaining later references or
escaped addresses; declarations that become unused are removed.
Unknown/conflicting types, calls, subscripts, comments
between writes, and mismatching components stay explicit. No names or vector
dimensions are guessed. The index recognizes repository `PACK(struct ...)`
declarations as well as ordinary class/struct declarations.

The transformer indexes declarations in the repository's `src/` headers and
C++ sources. This lets it convert explicit object arguments without requiring
particular global or singleton names:

```cpp
Service::Run(&globalObject, 2);      // globalObject.Run(2);
Service::Run(Service::instance, 2); // Service::instance->Run(2);
```

The index records global values, global pointers, static/instance data members,
base classes, and
whether class methods are static or non-static. New indexed conversions require
a matching object type and a known non-static method. Conflicting declarations
or a mixture of static and non-static overloads prevent conversion. Simple local
declarations in the clipboard take precedence over indexed globals.
Typed receiver lookup also covers parameters and nested member paths, and the
receiver may appear after scalar arguments (as in Ghidra's float-first calls).
Multiple matching receivers prevent conversion.

The generated JSON cache lives in the Windows temporary directory under
`kya_special_paste_symbols_<repository-id>.json`. It refreshes automatically when
source file paths, sizes, or modification times change. Deleting it forces a
rebuild on the next invocation. A new Python process runs for every paste.

This is a lightweight declaration scanner, not a C++ compiler. Macro-generated
declarations (apart from `PACK`), complex templates, inherited-method lookup, and general expression
type inference are not supported. Existing decompiler-specific cast/base-chain
and known-class heuristics still apply when indexed information is unavailable;
known static methods are excluded from those conversions too.

Checks:

```powershell
python -B tools/special_paste/transform.py --test
python -B -m unittest discover -s tools/special_paste -p 'test_*.py'
```
