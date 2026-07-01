import { randomUUID } from "node:crypto";
import { mkdir, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { afterEach, describe, expect, it } from "vitest";
import { EvaluationEngine } from "../src/evaluator.js";
import { outputTokenLimit } from "../src/prompt.js";
import { publicReviewItems, submitVote } from "../src/review.js";
import { rankRun } from "../src/ranking.js";
import { createEvaluationRun, requestKey } from "../src/runFactory.js";
import { RunStore } from "../src/store.js";
import type { EvaluationRun, ModelMetadata } from "../src/types.js";

const temporaryRoots: string[] = [];

function models(contextLength = 32768): ModelMetadata[] {
  return ["one", "two", "three", "four"].map((name) => ({
    id: `test/${name}:free`,
    name,
    contextLength,
    promptPrice: 0,
    completionPrice: 0,
    inputModalities: ["text"],
    outputModalities: ["text"],
    supportedParameters: [],
    raw: {}
  }));
}

async function fixture(source: string): Promise<{ root: string; store: RunStore; run: EvaluationRun }> {
  const root = path.join(process.cwd(), "evaluation", `test-${randomUUID()}`);
  temporaryRoots.push(root);
  const repositoryRoot = path.join(root, "repo");
  await mkdir(path.join(repositoryRoot, "src"), { recursive: true });
  await writeFile(path.join(repositoryRoot, "src", "fixture.cpp"), source, "utf8");
  const store = new RunStore(path.join(root, "runs"));
  const run = await createEvaluationRun({
    repositoryRoot,
    requestedFileCount: 1,
    relativePaths: ["src/fixture.cpp"],
    models: models(),
    timeoutMs: 5000
  });
  await store.save(run);
  return { root, store, run };
}

function successfulResponse(model: string): Response {
  return new Response(
    JSON.stringify({
      provider: `provider-${model}`,
      choices: [{
        message: {
          content: JSON.stringify({
            suggestions: [{
              declaration_id: "local_001",
              old_name: "iVar1",
              suggested_name: "iVar1",
              confidence: 0.7
            }]
          })
        }
      }],
      usage: { prompt_tokens: 20, completion_tokens: 10, total_tokens: 30 }
    }),
    { status: 200, headers: { "Content-Type": "application/json" } }
  );
}

afterEach(async () => {
  await Promise.all(temporaryRoots.splice(0).map((root) => rm(root, { recursive: true, force: true })));
});

describe("run setup and scheduling", () => {
  it("computes dynamic limits and exact request projections", async () => {
    expect(outputTokenLimit(1)).toBe(512);
    expect(outputTokenLimit(100)).toBe(6400);
    expect(outputTokenLimit(1000)).toBe(8192);
    const { run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    expect(run.manifest.primaryRequestCount).toBe(4);
    expect(run.manifest.worstCaseRequestCount).toBe(8);
    expect(run.manifest.files[0].randomizedFunctionIds).toHaveLength(1);
  });

  it("retries rate limits once, respects concurrency, and does not repeat a persisted success", async () => {
    const { store, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    const fn = run.manifest.files[0].functions.find((candidate) => !candidate.exclusionReason)!;
    const persistedModel = run.manifest.models[3];
    const persisted = run.requests[requestKey(fn.id, persistedModel.id)];
    persisted.status = "success";
    persisted.normalized = {
      suggestions: [{
        declaration_id: "local_001",
        old_name: "iVar1",
        suggested_name: "iVar1",
        confidence: 0.9,
        decision: "keep",
        keep: true
      }]
    };
    persisted.attempts.push({
      attempt: 1,
      startedAt: new Date().toISOString(),
      finishedAt: new Date().toISOString(),
      latencyMs: 1,
      status: "success"
    });
    await store.save(run);

    let active = 0;
    let maximumActive = 0;
    const activeByModel = new Map<string, number>();
    const maximumByModel = new Map<string, number>();
    const calls = new Map<string, number>();
    const fetchMock: typeof fetch = async (_url, init) => {
      const body = JSON.parse(String(init?.body));
      const model = body.model as string;
      calls.set(model, (calls.get(model) ?? 0) + 1);
      active++;
      maximumActive = Math.max(maximumActive, active);
      activeByModel.set(model, (activeByModel.get(model) ?? 0) + 1);
      maximumByModel.set(model, Math.max(maximumByModel.get(model) ?? 0, activeByModel.get(model)!));
      await new Promise((resolve) => setTimeout(resolve, 10));
      active--;
      activeByModel.set(model, activeByModel.get(model)! - 1);
      if (model.endsWith("one:free") && calls.get(model) === 1) return new Response("limited", { status: 429 });
      return successfulResponse(model);
    };
    await new EvaluationEngine(store, "test-key", fetchMock).start(run.manifest.runId);
    const completed = await store.load(run.manifest.runId);
    expect(completed.status).toBe("review");
    expect(maximumActive).toBeLessThanOrEqual(3);
    expect([...maximumByModel.values()].every((value) => value <= 1)).toBe(true);
    expect(calls.get(run.manifest.models[0].id)).toBe(2);
    expect(calls.get(persistedModel.id)).toBeUndefined();
    expect(completed.manifest.files[0].reviewAvailable).toBe(true);
  });

  it("pauses between request batches and resumes without repeating successes", async () => {
    const { store, run } = await fixture(
      "int first() { int iVar1 = 1; return iVar1; }\nint second() { int iVar1 = 2; return iVar1; }\n"
    );
    const calls = new Map<string, number>();
    const fetchMock: typeof fetch = async (_url, init) => {
      const model = JSON.parse(String(init?.body)).model as string;
      calls.set(model, (calls.get(model) ?? 0) + 1);
      await new Promise((resolve) => setTimeout(resolve, 15));
      return successfulResponse(model);
    };
    const engine = new EvaluationEngine(store, "test-key", fetchMock);
    const firstStart = engine.start(run.manifest.runId);
    await new Promise((resolve) => setTimeout(resolve, 2));
    engine.requestPause(run.manifest.runId);
    await firstStart;
    expect((await store.load(run.manifest.runId)).status).toBe("paused");
    const countsAtPause = new Map(calls);
    await engine.start(run.manifest.runId);
    expect((await store.load(run.manifest.runId)).status).toBe("review");
    for (const [modelId, count] of countsAtPause) {
      expect(calls.get(modelId)).toBeGreaterThanOrEqual(count);
    }
    expect([...calls.values()].reduce((sum, count) => sum + count, 0)).toBe(8);
  });

  it("retries a whole-response validation failure and preserves the failed raw response", async () => {
    const { store, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    let invalidSent = false;
    const fetchMock: typeof fetch = async (_url, init) => {
      const model = JSON.parse(String(init?.body)).model as string;
      if (!invalidSent && model.endsWith("one:free")) {
        invalidSent = true;
        return new Response(
          JSON.stringify({ choices: [{ message: { content: "{}" } }] }),
          { status: 200, headers: { "Content-Type": "application/json" } }
        );
      }
      return successfulResponse(model);
    };
    await new EvaluationEngine(store, "test-key", fetchMock).start(run.manifest.runId);
    const completed = await store.load(run.manifest.runId);
    const fn = completed.manifest.files[0].functions.find((candidate) => !candidate.exclusionReason)!;
    const request = completed.requests[requestKey(fn.id, completed.manifest.models[0].id)];
    expect(request.attempts).toHaveLength(2);
    expect(request.attempts[0].failureKind).toBe("validation");
    expect(request.attempts[0].rawResponseFile).toBeTruthy();
    expect(request.status).toBe("success");
  });

  it("penalizes only the failed model while reviewing successful answers from the same function", async () => {
    const { store, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    const fetchMock: typeof fetch = async (_url, init) => {
      const model = JSON.parse(String(init?.body)).model as string;
      return model.endsWith("one:free")
        ? new Response("request rejected", { status: 400 })
        : successfulResponse(model);
    };
    await new EvaluationEngine(store, "test-key", fetchMock).start(run.manifest.runId);
    const completed = await store.load(run.manifest.runId);
    expect(completed.reviewItems).toHaveLength(1);
    const answeredOption = completed.reviewItems[0].options.find((option) => option.modelIds.length > 0)!;
    submitVote(completed, completed.reviewItems[0].key, {
      preferredLabel: answeredOption.label,
      acceptableLabels: []
    });
    const ranking = rankRun(completed);
    expect(ranking.pairedFunctionCount).toBe(1);
    expect(ranking.excludedPairedFunctionCount).toBe(0);
    const failed = ranking.models.find((model) => model.modelId.endsWith("one:free"))!;
    expect(failed).toMatchObject({ acceptanceRate: 0, preferredRate: 0, completionRate: 0, terminalFailures: 1, qualifies: false });
    expect(ranking.models.filter((model) => model !== failed).every(
      (model) => model.acceptanceRate === 1 && model.preferredRate === 1 && model.completionRate === 1 && model.qualifies
    )).toBe(true);
  });

  it("omits a function only when every model fails it", async () => {
    const { store, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    const fetchMock: typeof fetch = async () => new Response("request rejected", { status: 400 });
    await new EvaluationEngine(store, "test-key", fetchMock).start(run.manifest.runId);
    const completed = await store.load(run.manifest.runId);
    expect(completed.reviewItems).toHaveLength(0);
    const ranking = rankRun(completed);
    expect(ranking.pairedFunctionCount).toBe(0);
    expect(ranking.excludedPairedFunctionCount).toBe(1);
    expect(ranking.models.every(
      (model) => model.acceptanceRate === 0 && model.completionRate === 0 && model.terminalFailures === 1
    )).toBe(true);
  });
});

describe("blind review and ranking", () => {
  it("collapses duplicate decisions, hides mappings, locks votes, and ranks paired results", async () => {
    const { store, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    const fetchMock: typeof fetch = async (_url, init) => successfulResponse(JSON.parse(String(init?.body)).model);
    await new EvaluationEngine(store, "test-key", fetchMock).start(run.manifest.runId);
    const completed = await store.load(run.manifest.runId);
    expect(completed.reviewItems).toHaveLength(1);
    expect(completed.reviewItems[0].options).toHaveLength(1);
    completed.manifest.files[0].functions[0].declarations[0].occurrences = undefined;
    const publicBefore = publicReviewItems(completed)[0];
    expect(publicBefore.functionSource).toContain("int iVar1 = 1");
    expect(publicBefore.highlightRanges).toHaveLength(2);
    expect(publicBefore.highlightRanges.map((range) => publicBefore.functionSource.slice(range.start, range.end)))
      .toEqual(["iVar1", "iVar1"]);
    expect(publicBefore.options[0]).not.toHaveProperty("modelIds");
    expect(publicBefore.options[0]).not.toHaveProperty("confidences");
    const option = completed.reviewItems[0].options[0];
    submitVote(completed, completed.reviewItems[0].key, {
      preferredLabel: option.label,
      acceptableLabels: []
    });
    expect(() =>
      submitVote(completed, completed.reviewItems[0].key, {
        preferredLabel: option.label,
        acceptableLabels: []
      })
    ).toThrow(/locked/i);
    expect(publicReviewItems(completed)[0].options[0].confidences).toHaveLength(4);
    const pricedModelId = completed.manifest.modelIds[0];
    const pricedMetadata = completed.manifest.models.find((model) => model.id === pricedModelId)!;
    pricedMetadata.promptPrice = 0.001;
    pricedMetadata.completionPrice = 0.002;
    const ranking = rankRun(completed);
    expect(ranking.models.every((model) => model.acceptanceRate === 1)).toBe(true);
    expect(ranking.models.every((model) => model.preferredRate === 1)).toBe(true);
    expect(ranking.models.every((model) => model.qualifies)).toBe(true);
    expect(ranking.models.find((model) => model.modelId === pricedModelId)?.cost).toEqual({
      total: 0.04,
      perSuccessfulAnswer: 0.04,
      estimated: true
    });
    const pricedRequest = Object.values(completed.requests).find((request) => request.modelId === pricedModelId)!;
    pricedRequest.attempts[0].cost = 0.00042;
    expect(rankRun(completed).models.find((model) => model.modelId === pricedModelId)?.cost).toEqual({
      total: 0.00042,
      perSuccessfulAnswer: 0.00042,
      estimated: false
    });
  });

  it("awards no points when the reviewer authors another name", async () => {
    const { store, run } = await fixture("int test() { int iVar1 = 1; return iVar1; }\n");
    const fetchMock: typeof fetch = async (_url, init) => successfulResponse(JSON.parse(String(init?.body)).model);
    await new EvaluationEngine(store, "test-key", fetchMock).start(run.manifest.runId);
    const completed = await store.load(run.manifest.runId);
    submitVote(completed, completed.reviewItems[0].key, { customName: "loopIndex" });
    expect(rankRun(completed).models.every((model) => model.acceptanceRate === 0)).toBe(true);
  });
});
