# Local Variable Renaming Tool Plan

## Objective

Build a local web application that scans a C++ source file, presents one function at a time, requests improved local-variable names from an OpenRouter model, lets the user edit or reject each suggestion, previews the resulting diff, and safely applies accepted names to the source file.

The first target is decompiled game code such as `src/b-witch/ActorShip.cpp`. The tool must respect the repository's layout-sensitive decompilation conventions and must never allow a language model to rewrite source code directly.

## Recommended implementation

- Location: `tools/local_renamer/`
- Runtime: Node.js 20 and TypeScript
- Backend: Express
- Frontend: React and Vite
- C++ parser: Tree-sitter C++
- Model service: OpenRouter
- Optional semantic validation: Clang tooling or `clangd`

TypeScript provides one language across the backend and frontend, while Tree-sitter can parse incomplete or partially decompiled C++ without requiring a successful compilation.

## Safety boundaries

The initial version will:

- Operate only on files beneath the repository root.
- Rename only local variable declarations and their bound references.
- Exclude parameters, members, globals, macros, types, labels, functions, and namespaces.
- Preserve fields such as `field_0xNN` and layout-sensitive declarations.
- Treat the language model only as a naming suggestion service.
- Reject invalid, duplicate, colliding, reserved, or unknown identifiers.
- Require the source file to have the same content hash it had when scanned.
- Show a diff before any write.
- Write changes atomically.
- Refuse automatic application when identifier binding is ambiguous.

## Phase 1: Application scaffold

1. Create the backend, frontend, shared TypeScript types, and test packages.
2. Add development commands that start both servers locally.
3. Read `OPENROUTER_API_KEY` only in the backend process.
4. Add configuration for repository root, selected model, request timeout, and maximum function size.
5. Ensure API keys and generated application state cannot be committed.

Deliverable: a local page that can reach a health endpoint and display the configured repository root.

## Phase 2: C++ analysis

1. Parse a selected C++ file with Tree-sitter.
2. Extract each function's:
   - stable session ID
   - qualified name
   - byte and line ranges
   - complete source text
   - parse-error state
3. Extract local declarations, including:
   - current identifier
   - declared type text
   - declaration location
   - lexical scope
   - exact identifier occurrence ranges
4. Implement lexical binding for nested scopes and shadowed names.
5. Exclude identifiers appearing as members, types, labels, macros, or qualified names.
6. Mark functions containing unresolved or ambiguous constructs as preview-only.
7. Investigate optional Clang validation using a compilation database generated with:

   ```powershell
   cmake --preset x64-debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   ```

Deliverable: an API that returns the functions and safely renameable declarations in `ActorShip.cpp`.

## Phase 3: OpenRouter integration

1. Implement a configurable OpenRouter client.
2. Pin exact model IDs; do not permit automatic paid fallbacks.
3. Send one complete function definition per request, including its comments and preprocessor text, together with an opaque function-local declaration inventory and repository-specific naming constraints. Do not send file paths, headers, neighboring functions, external declarations, rationales, or example renames.
4. Instruct the model to suggest names only when semantic evidence exists.
5. Require a response shaped like:

   ```json
   {
     "suggestions": [
       {
        "declaration_id": "local_001",
        "old_name": "pSVar1",
        "suggested_name": "pActorStreamRef",
        "confidence": 0.91
       }
     ]
   }
   ```

6. Require exactly one four-field response entry for every supplied declaration, including a `suggested_name` equal to `old_name` for a keep decision. Do not request or accept rationales.
7. Use provider-supported structured output when available.
8. For models without structured output, accept only raw JSON or one JSON object inside a Markdown fence, validate the complete response, and retry malformed responses once.
9. Reject the complete response for missing, duplicate, unknown, malformed, or invalid declarations, and normalize valid entries to declaration source order.
10. Validate confidence as a number from `0.0` through `1.0`, exact case-sensitive old names, C++ naming rules, required pointer and Boolean prefixes, and function-wide collisions.
11. Record the model ID, latency, token counts, validation failures, and raw response for local diagnostics.

Deliverable: validated suggestions for a selected function without modifying its source file.

## Phase 4: Rename and diff engine

1. Validate proposed names as C++ identifiers.
2. Reject keywords, collisions, scope conflicts, and duplicate targets. Normalize unchanged suggestions to explicit keep decisions and exclude them from rename edits.
3. Convert accepted suggestions into exact byte-range edits.
4. Apply edits from the end of the source buffer toward the beginning.
5. Reparse the modified function and confirm all accepted declarations were renamed consistently.
6. Produce a unified diff against the current file.
7. Recheck the original file hash immediately before writing.
8. Write through a temporary file followed by an atomic replacement.
9. Do not run a formatter automatically, because unrelated formatting changes would obscure the rename diff.

Deliverable: a preview API followed by a separately confirmed apply API.

## Phase 5: User workflow

The main page will provide:

- A repository-relative C++ file picker.
- A function list with pending, completed, skipped, and error states.
- A source viewer focused on the current function.
- An editable suggestions table containing:
  - accept checkbox
  - current name
  - proposed name
  - confidence
- `Generate`, `Regenerate`, `Preview diff`, `Apply`, `Skip`, and `Next` actions.
- Clear warnings for ambiguous parsing, stale files, model failures, and invalid names.
- Session progress that survives page refreshes while the backend remains running.

Confidence is visible in normal renaming sessions. The integrated evaluator described in `free-model-comparison.md` uses the same four-field response schema but hides confidence until each blind-review vote is submitted.

Evaluator sessions are strictly read-only: they may rank models and update the retained-model configuration, but they cannot preview, apply, or otherwise write source renames.

## Phase 6: Testing

Add automated coverage for:

- Function extraction and stable ordering.
- Multiple declarations in one statement.
- Nested blocks and shadowed identifiers.
- Loop initializers, lambdas, captures, and structured bindings.
- Comments, string literals, member access, and qualified names.
- Preprocessor macros and parse errors.
- Malformed model responses and unknown declaration IDs.
- Missing or duplicate declarations, invalid confidence values, and rationale or other extra fields.
- C++ keywords, duplicate proposals, and scope collisions.
- External file modification after scanning.
- Correct byte-range application with Windows line endings.
- Atomic write failure and unchanged-file guarantees.

Create golden fixtures from representative functions in `ActorShip.cpp`, but do not make the tests depend on changing the real source file.

## Phase 7: Repository verification

1. Run the complete application test suite.
2. Exercise a full session against a temporary copy of `ActorShip.cpp`.
3. Inspect the generated diff for changes outside accepted identifiers.
4. Confirm the changed source reparses successfully.
5. Run:

   ```powershell
   cmake --preset x64-debug
   cmake --build out/build/x64-debug
   ctest --test-dir out/build/x64-debug
   ```

6. Document installation, OpenRouter configuration, privacy settings, supported syntax, and known limitations.

## Completion criteria

The initial tool is complete when a user can select `ActorShip.cpp`, process its functions sequentially, edit or reject every suggestion, preview each exact diff, safely apply accepted local-variable renames, and finish with a buildable source tree without any unapproved changes.
