# Kya Local Renamer Evaluator

Local, read-only web application for comparing free and curated paid OpenRouter models on local-variable naming decisions in this repository's decompiled C++.

The evaluator never writes selected source files. It freezes file hashes, filters definition-only locals from the inventory and prompt copy, sends one function at a time, validates every declaration response, presents successful anonymous choices for locked human review, penalizes request failures only for the affected model, and preserves measured rankings separately from the retained-model selection.

## Requirements

- Node.js 20
- npm
- An OpenRouter API key

## Install and run

From `tools/local_renamer`:

```powershell
npm install
$env:OPENROUTER_API_KEY = "your-key"
npm run dev
```

Open `http://127.0.0.1:5173`. The API listens on `http://127.0.0.1:4310`.

For a production build served by Express:

```powershell
npm run build
$env:OPENROUTER_API_KEY = "your-key"
npm start
```

The API key is read only by the backend process. It is not returned by an endpoint, sent to the browser, or persisted in run state.

Optional backend configuration:

- `LOCAL_RENAMER_REPOSITORY_ROOT`: repository root; defaults to this repository.
- `LOCAL_RENAMER_PORT`: Express port; defaults to `4310`.

## Workflow

1. Load the live OpenRouter catalog. Models that passed the previous availability test are restored from the local cache. Free and curated paid models have separate re-test buttons so paid probes are always explicit. Select at least two verified model IDs; failed and untested models remain disabled.
2. Enter unique repository-relative `.cpp`, `.cc`, or `.cxx` files, one per line. The file count is derived from the list.
3. Create the frozen run and inspect its primary and worst-case request counts.
4. Start the run. Pause requests if needed and resume later from persisted successful responses.
5. Review collapsed anonymous answers. Select one preferred decision, optionally mark other acceptable decisions, or enter a different valid name.
6. Finalize the locked review to reveal the measured ranking.
7. Adjust the independently retained model set and explicitly export a reproducibility JSON bundle if required.

Runs are stored beneath `evaluation/runs/<run-id>/` and ignored by Git. They contain source text, prompts, raw model responses, hidden model mappings, and review decisions; treat this directory and exported bundles as repository-sensitive data.

## Request behavior

- Temperature is fixed at `0`.
- Timeout defaults to 120 seconds and is configurable when creating a run.
- At most three requests are active globally and at most one is active for a model.
- Exact selected model IDs are used without model fallbacks.
- Normal provider routing remains enabled. Free models cap provider prompt and completion prices at zero; explicitly selected paid models use their normal provider prices.
- JSON Schema is requested only when advertised by model metadata; other models must return raw JSON or one JSON fence.
- A transport, timeout, rate-limit, schema, or validation failure receives one retry.
- Functions are never truncated.

See [the evaluator plan](../../docs/plans/free-model-comparison.md) for the complete contract.

## Conservative exclusions

The analyzer excludes functions with parse errors, no local declarations, structured bindings, or lambdas whose nested bindings cannot yet be represented safely. It includes ordinary block shadowing, traditional loop initializers, range-for declarations, and condition declarations.

Context checks use UTF-8 byte length plus chat framing as a tokenizer-independent upper bound. This can exclude a function that would fit a particular model, but it prevents model-specific tokenization from causing truncation.

## Verification

```powershell
npm run typecheck
npm test
npm run build
```

Tests mock OpenRouter and do not consume API requests. They cover catalog filtering, candidate/file constraints, C++ enumeration, response parsing and naming validation, retries, concurrency, pause/resume, anonymous review, locked votes, paired scoring, completion gating, and ranking.

OpenRouter behavior is implemented against its official documentation for the [Models API](https://openrouter.ai/docs/guides/overview/models), [structured outputs](https://openrouter.ai/docs/guides/features/structured-outputs), and [provider routing](https://openrouter.ai/docs/guides/routing/provider-selection).
