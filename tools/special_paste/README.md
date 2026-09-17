# Special paste

Run `special_paste.ahk` with AutoHotkey v2, then press Ctrl+E to transform and
paste clipboard text. Python must be on PATH. Keep `transform.py` and
`symbol_index.py` beside the AutoHotkey script.

The transformer indexes declarations in the repository's `src/` headers and
C++ sources. This lets it convert explicit object arguments without requiring
particular global or singleton names:

```cpp
Service::Run(&globalObject, 2);      // globalObject.Run(2);
Service::Run(Service::instance, 2); // Service::instance->Run(2);
```

The index records global values, global pointers, static data members, and
whether class methods are static or non-static. New indexed conversions require
a matching object type and a known non-static method. Conflicting declarations
or a mixture of static and non-static overloads prevent conversion. Simple local
declarations in the clipboard take precedence over indexed globals.

The generated JSON cache lives in the Windows temporary directory under
`kya_special_paste_symbols_<repository-id>.json`. It refreshes automatically when
source file paths, sizes, or modification times change. Deleting it forces a
rebuild on the next invocation. A new Python process runs for every paste.

This is a lightweight declaration scanner, not a C++ compiler. Macro-generated
declarations, complex templates, inherited-method lookup, and general expression
type inference are not supported. Existing decompiler-specific cast/base-chain
and known-class heuristics still apply when indexed information is unavailable;
known static methods are excluded from those conversions too.

Checks:

```powershell
python -B tools/special_paste/transform.py --test
python -B -m unittest discover -s tools/special_paste -p test_symbol_index.py
```
