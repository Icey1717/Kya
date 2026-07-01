# Free OpenRouter Model Evaluator Plan

## Objective

Add a single-stage, read-only evaluator to the planned local-variable renaming tool. The evaluator measures live free OpenRouter models on an exact user-selected set of source files. Every selected model evaluates every eligible function once, and a human reviews anonymized naming decisions per declaration.

The measured ranking and the set of models retained for normal renaming are separate outputs. The evaluator preselects the three highest-ranked qualifying models, but the reviewer may retain any number and combination of evaluated models.

## Scope and safety boundaries

- Integrate the evaluator beneath `tools/local_renamer/evaluation/` and reuse the renamer's C++ extraction, binding, prompt, response-validation, and naming-validation code.
- Keep evaluation read-only. It must never apply a rename or modify a selected source file.
- Use exact OpenRouter model IDs. Do not use `openrouter/free` or any other model-selection alias.
- Disable paid fallbacks while retaining normal OpenRouter provider routing for each exact model.
- Never commit run data automatically. A separate explicit export action creates a reproducibility bundle for the user to save or commit deliberately.

## Run setup

### Live model selection

At the start of a run, query OpenRouter's live model catalog. Show only models that:

- accept text input and produce text output;
- have zero prompt-token pricing; and
- have zero completion-token pricing.

Display the exact model ID, context limit, structured-output capability when advertised, and current catalog metadata needed to reproduce the selection. Require the user to select at least two exact model IDs. Freeze the selected IDs and their catalog metadata in the run manifest so a resumed run is not silently changed by later catalog updates.

### Exact file selection

Require the user to enter repository-relative source files one per line and derive the exact file count from the non-empty entries. Resolve and validate every path beneath the repository root, reject duplicates, and freeze the ordered file list and each file's initial content hash before requests begin.

Enumerate every function definition in every selected file, including member, free, and static functions. A function is eligible only when it parses sufficiently for safe lexical binding and contains at least one safely bound local declaration that is referenced outside its own definition. Include already descriptive referenced local names in that inventory.

Exclude a function before requests when:

- it contains no eligible locals;
- any local binding needed for the inventory is ambiguous;
- its definition cannot be parsed safely; or
- its complete request would exceed the context limit of any selected model.

Never truncate a function or construct different inputs for different models. Record every exclusion and its reason in the manifest.

### Request projection

After enumeration and before confirmation, show:

- the number of selected files, enumerated functions, eligible functions, and declarations;
- primary requests: `selected model count x eligible function count`; and
- worst-case requests: `2 x primary requests`, because each primary request can have at most one retry.

## Evaluation execution

Process selected files one at a time. Randomize eligible function order independently within each file and persist that order before issuing requests. For each function, request one result from every selected model.

Use these request controls:

- temperature `0`;
- a configurable timeout defaulting to 120 seconds;
- global concurrency of three requests; and
- at most one active request for any individual model.

Set the requested output limit per function to:

```text
min(8192, max(512, 64 x declaration count))
```

Each model receives one primary attempt per function. Make exactly one retry only when the primary attempt fails because of transport, timeout, rate limiting, response-schema failure, or response-validation failure. Do not retry a successful request and do not retry other failure classes.

Persist a successful response before scheduling more work. Pause and resume must reuse all successful results and retry history rather than repeat requests. On resume, verify the frozen manifest and initial source hashes; if a selected source file changed, stop and require a new run rather than mixing inputs.

Suggestions remain hidden until all models have either succeeded or exhausted their allowed retry for every eligible function in the current file. Review of a completed file may then begin while later files are evaluated, but model identities remain hidden until the complete review is finalized.

## Prompt contract

Send only:

1. instructions describing the naming task and response contract;
2. the declaration inventory; and
3. the function definition, including its comments and preprocessor text, after safely removable definition-only local declarations have been omitted from the prompt copy.

Do not send file paths, headers, neighboring functions, external declarations, rationales, or example renames.

The inventory must contain every safely bound local declaration referenced outside its own definition, including already descriptive names, and must exclude definition-only locals, parameters, members, globals, macros, types, labels, and ambiguous declarations. Assign opaque function-local IDs in source order, such as `local_001`. Each inventory entry contains only:

- `declaration_id`;
- `old_name`; and
- declared type text.

The prompt requires exactly one suggestion for every inventory entry. A model expresses a decision to retain a name by returning `suggested_name` equal to `old_name`; the application normalizes that decision to `keep`. Do not ask for or accept rationale text.

## Response contract

The only accepted response shape is:

```json
{
  "suggestions": [
    {
      "declaration_id": "local_001",
      "old_name": "pCVar1",
      "suggested_name": "pTargetActor",
      "confidence": 0.92
    }
  ]
}
```

Use provider-enforced JSON Schema when the selected model and resolved provider support it. Otherwise accept only raw JSON or exactly one JSON object inside a Markdown code fence. Do not extract JSON from prose or repair malformed output.

Validate the complete response atomically:

- the top-level object and every field have the required shape and type;
- there is exactly one entry for every inventory declaration;
- every `declaration_id` is known and appears exactly once;
- `old_name` exactly matches the inventory, including case;
- `suggested_name` is a string;
- `confidence` is a numeric value from `0.0` through `1.0`; and
- no unknown, missing, duplicated, or extra entry or field is present.

Entries may arrive in any order. After validation, normalize them to declaration source order. Reject the entire response if any entry is missing, duplicated, unknown, malformed, or invalid.

For changed names, enforce all of these rules:

- valid C++ identifier and not a C++ keyword;
- lower camel case with no underscores, adjacent uppercase letters, or trailing digits;
- pointer types use `p[A-Z]...`;
- non-pointer Boolean types use `b[A-Z]...`; and
- no other type-prefix convention is enforced.

An unchanged name is a valid `keep` decision even when the legacy name does not meet the style rules. Validate final decisions for function-wide uniqueness against local and parameter names. Member names do not collide because member access uses `this->`. Existing shadowed local names are grandfathered only when each affected declaration is retained unchanged; a changed suggestion may not create or reuse that shadowing.

## Blind review

Review one declaration at a time. Show its current name and the distinct decisions returned for it. Always include `keep existing`, even when no model returned it.

Collapse identical naming decisions into one answer based on the normalized `suggested_name` or `keep` value, regardless of confidence, without revealing how many models produced it. Randomize anonymous answer labels independently for every function, persist the label-to-decision and decision-to-model mappings, and never reveal those mappings during review.

Before a vote, show the names but hide confidence values. The reviewer must:

1. select one preferred answer; and
2. optionally mark any additional displayed answers as acceptable.

The reviewer may instead enter a different valid name. In that case, treat every model answer as unacceptable and award no model points for the declaration. There is no separate semantic-unsafety rejection class because all suggestions require human review.

After submission, reveal confidence metadata for that declaration without exposing the number or identity of models behind a collapsed answer, and permanently lock the vote. Review state cannot be edited, reset, or replayed within the run. Model identities stay hidden until all declaration votes are complete and the reviewer explicitly finalizes review.

## Scoring and ranking

For each reviewed declaration, award one full acceptance point to every model behind the preferred answer or any additionally accepted answer. A model receives a preferred-choice point only when it is behind the single preferred answer. Duplicate collapsed answers therefore award the same applicable point to every model that produced that decision.

If a model fails a function after its retry, retain that function for review when at least one other model succeeded. The failed model is behind no answer for that function, receives no acceptance or preferred-choice credit for its declarations, and retains the failed request in completion, validation-failure, retry, and reliability reporting. Exclude a function from review only when every selected model failed it.

Rank models by:

1. declaration-level acceptance rate;
2. preferred-choice rate as the first tie-breaker; and
3. a stable exact-model-ID ordering only to make any remaining tie deterministic.

Calculate declaration-level acceptance and preferred-choice rates over all reviewed declarations, including zero credit where an individual model failed the containing function. Calculate completion as successful function requests after any allowed retry divided by all eligible function requests scheduled for that model. Do not include confidence or latency in the quality score. A model must successfully complete at least 90% of its function requests to qualify for recommendation.

Report, without folding them into the primary quality ranking:

- preferred-choice rate;
- equal-weight per-file acceptance results, so large files do not dominate that view;
- function-request completion rate;
- schema and validation failures;
- retries and terminal failures;
- latency distributions;
- resolved provider routing;
- prompt, completion, and other available token usage; and
- counts of functions excluded from paired quality scoring.

Preserve the measured ranking as immutable run output. Separately preselect the top three qualifying models as the retained set. The reviewer may change both the retained models and the retained count; save that discretionary selection without rewriting or reinterpreting the measured ranking.

## Persistence and export

Store resumable evaluator runs beneath:

```text
tools/local_renamer/evaluation/runs/<run-id>/
```

Gitignore this directory by default. Persist writes atomically and retain enough state to audit and resume a partially completed run, including:

- frozen run manifest and evaluator/schema versions;
- initial source hashes and exact file selection;
- randomized file-local function order and exclusion reasons;
- selected model catalog metadata and context limits;
- request configuration and dynamic output limits;
- resolved providers, request timing, token usage, and status;
- raw responses, normalized results, validation errors, and retry history;
- anonymous review state and hidden label mappings;
- permanently locked votes and confidence-reveal state;
- measured ranking; and
- discretionary retained-model selection.

Do not persist API keys. Provide an explicit export action that packages the reproducibility data while preserving the source hashes and exact inputs needed to interpret results. Exporting must not alter the run or commit anything.

## Tests

### Catalog, selection, and enumeration

- Live-catalog filtering rejects paid or non-text models.
- At least two exact candidate IDs are required.
- The file count is derived from the non-empty unique selected files.
- Every member, free, and static function in each selected file is enumerated.
- Random function order is persisted and restored on resume.
- Empty, ambiguous, parse-failed, and over-context functions receive the correct exclusion reason.
- Primary and worst-case request projections are exact.
- Dynamic output limits respect the 512-token floor, 64-token multiplier, and 8,192-token cap.

### Scheduling and recovery

- No more than three requests run globally and no model has two active requests.
- Retryable failures receive one retry and successful or non-retryable outcomes do not.
- File results stay hidden until every model has completed or exhausted retry work for that file.
- Pause and resume never repeat a successful request.
- Source-hash changes prevent a resume.

### Response validation

- Complete declaration coverage and arbitrary response order normalize to source order.
- Raw JSON and one fenced object are accepted; prose, multiple objects, and repaired JSON are rejected.
- Missing, extra, duplicated, unknown, malformed, and case-mismatched entries reject the whole response.
- Identifier, keyword, casing, underscore, adjacent-uppercase, and trailing-digit rules are enforced.
- Pointer and Boolean prefixes are enforced without inventing other prefixes.
- `suggested_name === old_name` normalizes to `keep`.
- Parameter/local collisions and newly introduced shadowing are rejected while unchanged legacy shadowing is allowed.

### Blind review and ranking

- Duplicate decisions collapse without exposing model counts.
- Labels are independently randomized per function and model mappings remain hidden until finalization.
- Confidence remains hidden before a vote and appears afterward.
- Submitted votes are permanently locked.
- Reviewer-authored alternatives give all models zero points.
- A terminal model failure excludes the function from every model's quality score.
- The 90% completion gate controls recommendation eligibility.
- Acceptance ranking, preferred-choice tie-breaking, and equal-weight per-file reporting are correct.
- The default top-three retention and reviewer-changed model/count selection remain separate from measured ranking.

## Completion criteria

The evaluator is complete when a user can select at least two currently free compatible models and an exact number of source files, resume a fully persisted run without duplicate successful requests, blindly review every scorable declaration, finalize an auditable paired ranking, choose any retained model set, and explicitly export a reproducibility bundle without any source modification.
