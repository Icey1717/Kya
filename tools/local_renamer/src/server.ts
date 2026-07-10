import { randomBytes } from "node:crypto";
import { mkdir, readFile, rename, writeFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
import express, { type NextFunction, type Request, type Response } from "express";
import {
  ApplicationEngine,
  ApplicationStore,
  applyApplicationFunction,
  applyApplicationRun,
  createApplicationRun,
  publicApplicationItems,
  publicApplicationRun,
  submitApplicationChoice
} from "./application.js";
import { applyRateLimitStatus, fetchSelectableModels, isPaidModel, validateModelSelection } from "./catalog.js";
import { runModeConfig } from "./config.js";
import { EvaluationEngine } from "./evaluator.js";
import { probeModelsAvailability } from "./modelAvailability.js";
import { rankRun, withCostSummaries } from "./ranking.js";
import { publicReviewItems, submitVote } from "./review.js";
import { createEvaluationRun } from "./runFactory.js";
import { RunStore } from "./store.js";
import type { EvaluationRun } from "./types.js";

const here = path.dirname(fileURLToPath(import.meta.url));
const defaultRepositoryRoot = path.resolve(here, "../../..");
const repositoryRoot = path.resolve(process.env.LOCAL_RENAMER_REPOSITORY_ROOT ?? defaultRepositoryRoot);
const runsRoot = path.resolve(here, "../evaluation/runs");
const store = new RunStore(runsRoot);
const applicationStore = new ApplicationStore(path.resolve(here, "../application/runs"));
const apiKey = process.env.OPENROUTER_API_KEY ?? "";
const engine = new EvaluationEngine(store, apiKey);
const applicationEngine = new ApplicationEngine(applicationStore, apiKey);
const app = express();

app.use(express.json({ limit: "2mb" }));

function publicRun(run: EvaluationRun): object {
  const functionCount = run.manifest.files.reduce((sum, file) => sum + file.functions.length, 0);
  const eligibleFunctionCount = run.manifest.files.reduce(
    (sum, file) => sum + file.functions.filter((fn) => !fn.exclusionReason).length,
    0
  );
  const requests = Object.values(run.requests);
  const completedRequests = requests.filter((request) => request.status !== "pending").length;
  const requestStats = {
    pending: requests.filter((request) => request.status === "pending").length,
    success: requests.filter((request) => request.status === "success").length,
    failed: requests.filter((request) => request.status === "failed").length,
    attempts: requests.reduce((sum, request) => sum + request.attempts.length, 0)
  };
  const recentActivity = requests
    .flatMap((request) => request.attempts.map((attempt) => ({
      modelId: request.modelId,
      filePath: request.filePath,
      functionId: request.functionId,
      attempt: attempt.attempt,
      status: attempt.status,
      finishedAt: attempt.finishedAt,
      latencyMs: attempt.latencyMs,
      failureKind: attempt.failureKind,
      error: attempt.error
    })))
    .sort((left, right) => right.finishedAt.localeCompare(left.finishedAt))
    .slice(0, 8);
  return {
    runId: run.manifest.runId,
    status: run.status,
    createdAt: run.manifest.createdAt,
    updatedAt: run.updatedAt,
    error: run.error,
    files: run.manifest.files.map((file) => ({
      relativePath: file.relativePath,
      functionCount: file.functions.length,
      eligibleFunctionCount: file.functions.filter((fn) => !fn.exclusionReason).length,
      exclusions: Object.fromEntries(
        ["no_locals", "ambiguous_binding", "parse_failure", "context_limit"].map((reason) => [
          reason,
          file.functions.filter((fn) => fn.exclusionReason === reason).length
        ])
      ),
      reviewAvailable: file.reviewAvailable
    })),
    functionCount,
    eligibleFunctionCount,
    declarationCount: run.manifest.files
      .flatMap((file) => file.functions)
      .filter((fn) => !fn.exclusionReason)
      .reduce((sum, fn) => sum + fn.declarations.length, 0),
    modelCount: run.manifest.modelIds.length,
    modelIds: run.manifest.modelIds,
    primaryRequestCount: run.manifest.primaryRequestCount,
    worstCaseRequestCount: run.manifest.worstCaseRequestCount,
    completedRequests,
    requestStats,
    recentActivity,
    reviewCount: run.reviewItems.length,
    submittedVoteCount: run.reviewItems.filter((item) => item.vote).length,
    active: engine.isActive(run.manifest.runId),
    ...(run.status === "finalized" && run.ranking
      ? { ranking: withCostSummaries(run, run.ranking), retainedModelIds: run.retainedModelIds }
      : {})
  };
}

app.get("/api/health", (_request, response) => {
  response.json({ ok: true, repositoryRoot, apiKeyConfigured: Boolean(apiKey) });
});

app.get("/api/models", async (_request, response) => {
  const [models, rateLimited, availableModels] = await Promise.all([
    fetchSelectableModels(),
    store.getActiveRateLimitedModels(),
    store.getAvailableModels()
  ]);
  response.json({ models: applyRateLimitStatus(models, rateLimited).map((model) => ({
    ...model,
    paid: isPaidModel(model.id),
    availability: availableModels[model.id] ? "available" : "untested",
    availabilityCheckedAt: availableModels[model.id]?.checkedAt
  })) });
});

app.get("/api/application/config", (_request, response) => {
  response.json({ modelIds: runModeConfig.modelIds, timeoutMs: runModeConfig.timeoutMs });
});

app.post("/api/application/runs", async (request, response) => {
  const files = (request.body as { files?: unknown })?.files;
  if (!Array.isArray(files) || !files.every((file) => typeof file === "string")) {
    throw new Error("files must be an array of repository-relative paths.");
  }
  const catalog = await fetchSelectableModels();
  const byId = new Map(catalog.map((model) => [model.id, model]));
  const models = runModeConfig.modelIds.map((modelId) => {
    const model = byId.get(modelId);
    if (!model) throw new Error(`Configured run model is not available in the OpenRouter catalog: ${modelId}`);
    return model;
  });
  const run = await createApplicationRun({ repositoryRoot, relativePaths: files, models, timeoutMs: runModeConfig.timeoutMs });
  await applicationStore.save(run);
  response.status(201).json(publicApplicationRun(run, false));
});

app.get("/api/application/runs/:runId", async (request, response) => {
  const run = await applicationStore.load(request.params.runId);
  response.json(publicApplicationRun(run, applicationEngine.isActive(run.runId)));
});

app.post("/api/application/runs/:runId/start", async (request, response) => {
  if (!apiKey) {
    response.status(503).json({ error: "OPENROUTER_API_KEY is not configured in the backend." });
    return;
  }
  const run = await applicationStore.load(request.params.runId);
  if (run.status !== "ready" && !(run.status === "running" && !applicationEngine.isActive(run.runId))) throw new Error(`Cannot start a ${run.status} application run.`);
  void applicationEngine.start(run.runId).catch((error) => console.error(`Application run ${run.runId} failed:`, error));
  response.status(202).json({ status: "running" });
});

app.get("/api/application/runs/:runId/review", async (request, response) => {
  response.json({ items: publicApplicationItems(await applicationStore.load(request.params.runId)) });
});

app.post("/api/application/runs/:runId/review/:itemKey", async (request, response) => {
  const run = await applicationStore.load(request.params.runId);
  const choice = submitApplicationChoice(run, request.params.itemKey, request.body ?? {});
  await applicationStore.save(run);
  response.status(201).json({ choice });
});

app.post("/api/application/runs/:runId/functions/:functionId/apply", async (request, response) => {
  const run = await applicationStore.load(request.params.runId);
  await applyApplicationFunction(run, request.params.functionId);
  await applicationStore.save(run);
  response.json(publicApplicationRun(run, false));
});
app.post("/api/application/runs/:runId/apply", async (request, response) => {
  const run = await applicationStore.load(request.params.runId);
  await applyApplicationRun(run);
  await applicationStore.save(run);
  response.json(publicApplicationRun(run, false));
});
app.post("/api/models/test", async (request, response) => {
  if (!apiKey) {
    response.status(503).json({ error: "OPENROUTER_API_KEY is not configured in the backend." });
    return;
  }
  const tier = (request.body as { tier?: unknown })?.tier === "paid" ? "paid" : "free";
  const [models, rateLimited] = await Promise.all([fetchSelectableModels(), store.getActiveRateLimitedModels()]);
  const candidates = models.filter((model) =>
    tier === "paid" ? isPaidModel(model.id) : !isPaidModel(model.id)
  );
  const results = await probeModelsAvailability(candidates, apiKey);
  const resultsById = new Map(results.map((result) => [result.modelId, result]));
  await store.updateAvailableModels(Object.fromEntries(
    results.map((result) => [result.modelId, result.available ? { checkedAt: result.checkedAt } : null])
  ));
  const availableModels = await store.getAvailableModels();
  response.json({ models: applyRateLimitStatus(models, rateLimited).map((model) => ({
    ...model,
    paid: isPaidModel(model.id),
    availability: resultsById.has(model.id)
      ? (resultsById.get(model.id)!.available ? "available" : "unavailable")
      : (availableModels[model.id] ? "available" : "untested"),
    availabilityNote: resultsById.get(model.id)?.note,
    availabilityCheckedAt: resultsById.get(model.id)?.checkedAt ?? availableModels[model.id]?.checkedAt
  })) });
});

app.post("/api/evaluation/runs", async (request, response) => {
  const body = request.body as {
    modelIds?: unknown;
    files?: unknown;
    timeoutMs?: unknown;
  };
  if (!Array.isArray(body.modelIds) || !body.modelIds.every((id) => typeof id === "string")) {
    throw new Error("modelIds must be an array of exact model IDs.");
  }
  if (!Array.isArray(body.files) || !body.files.every((file) => typeof file === "string")) {
    throw new Error("files must be an array of repository-relative paths.");
  }
  const [catalog, rateLimited, availableModels] = await Promise.all([
    fetchSelectableModels(),
    store.getActiveRateLimitedModels(),
    store.getAvailableModels()
  ]);
  const models = validateModelSelection(catalog, body.modelIds, new Set(Object.keys(rateLimited)));
  const untestedOrUnavailable = models.find((model) => !availableModels[model.id]);
  if (untestedOrUnavailable) {
    throw new Error(`Test model availability before creating a run; ${untestedOrUnavailable.id} is not currently verified.`);
  }
  const run = await createEvaluationRun({
    repositoryRoot,
    requestedFileCount: body.files.length,
    relativePaths: body.files,
    models,
    timeoutMs: body.timeoutMs === undefined ? undefined : Number(body.timeoutMs)
  });
  await store.save(run);
  response.status(201).json(publicRun(run));
});

app.get("/api/evaluation/runs/:runId", async (request, response) => {
  response.json(publicRun(await store.load(request.params.runId)));
});

app.post("/api/evaluation/runs/:runId/start", async (request, response) => {
  if (!apiKey) {
    response.status(503).json({ error: "OPENROUTER_API_KEY is not configured in the backend." });
    return;
  }
  const run = await store.load(request.params.runId);
  const recoverableRunningState = run.status === "running" && !engine.isActive(run.manifest.runId);
  if (!["ready", "paused", "failed"].includes(run.status) && !recoverableRunningState) {
    throw new Error(`Cannot start a ${run.status} run.`);
  }
  console.log(`Evaluation run ${request.params.runId}: start accepted (${run.status}, ${run.manifest.primaryRequestCount} requests)`);
  void engine.start(request.params.runId).catch((error) => {
    console.error(`Evaluation run ${request.params.runId} failed:`, error);
  });
  response.status(202).json({ status: "running" });
});

app.post("/api/evaluation/runs/:runId/pause", async (request, response) => {
  engine.requestPause(request.params.runId);
  response.status(202).json({ status: "pause_requested" });
});

app.get("/api/evaluation/runs/:runId/review", async (request, response) => {
  const run = await store.load(request.params.runId);
  response.json({ items: publicReviewItems(run) });
});

app.post("/api/evaluation/runs/:runId/review/:itemKey/vote", async (request, response) => {
  const run = await store.load(request.params.runId);
  if (run.status !== "review") throw new Error("Votes are accepted only during review.");
  const vote = submitVote(run, request.params.itemKey, request.body ?? {});
  await store.save(run);
  response.status(201).json({ vote });
});

app.post("/api/evaluation/runs/:runId/finalize", async (request, response) => {
  const run = await store.load(request.params.runId);
  if (run.status !== "review") throw new Error("Only a review-stage run can be finalized.");
  run.ranking = rankRun(run);
  run.retainedModelIds = run.ranking.models.filter((model) => model.qualifies).slice(0, 3).map((model) => model.modelId);
  run.status = "finalized";
  await store.save(run);
  response.json(publicRun(run));
});

app.put("/api/evaluation/runs/:runId/retained-models", async (request, response) => {
  const run = await store.load(request.params.runId);
  if (run.status !== "finalized") throw new Error("Finalize review before changing retained models.");
  const ids = (request.body as { modelIds?: unknown }).modelIds;
  if (!Array.isArray(ids) || !ids.every((id) => typeof id === "string")) throw new Error("modelIds must be an array.");
  if (new Set(ids).size !== ids.length || ids.some((id) => !run.manifest.modelIds.includes(id))) {
    throw new Error("Retained models must be unique evaluated model IDs.");
  }
  run.retainedModelIds = ids;
  await store.save(run);
  response.json({ retainedModelIds: ids, ranking: run.ranking });
});

app.post("/api/evaluation/runs/:runId/export", async (request, response) => {
  const run = await store.load(request.params.runId);
  const rawResponses: Record<string, string> = {};
  for (const [key, record] of Object.entries(run.requests)) {
    for (const attempt of record.attempts) {
      if (!attempt.rawResponseFile) continue;
      rawResponses[`${key}/attempt-${attempt.attempt}`] = await readFile(
        path.join(store.runDirectory(run.manifest.runId), attempt.rawResponseFile),
        "utf8"
      );
    }
  }
  const exportDirectory = path.join(store.runDirectory(run.manifest.runId), "exports");
  await mkdir(exportDirectory, { recursive: true });
  const fileName = `reproducibility-${Date.now()}-${randomBytes(2).toString("hex")}.json`;
  const destination = path.join(exportDirectory, fileName);
  const temporary = `${destination}.tmp`;
  await writeFile(
    temporary,
    JSON.stringify({ exportedAt: new Date().toISOString(), run, rawResponses }, null, 2) + "\n",
    "utf8"
  );
  await rename(temporary, destination);
  response.status(201).json({ path: path.relative(repositoryRoot, destination).replaceAll("\\", "/") });
});

app.use((error: unknown, _request: Request, response: Response, _next: NextFunction) => {
  const message = error instanceof Error ? error.message : String(error);
  response.status(400).json({ error: message });
});

const webRoot = path.resolve(here, "../dist-web");
app.use(express.static(webRoot));
app.get("/{*splat}", (_request, response) => response.sendFile(path.join(webRoot, "index.html")));

if (process.env.NODE_ENV !== "test") {
  const port = Number(process.env.LOCAL_RENAMER_PORT ?? 4310);
  app.listen(port, "127.0.0.1", () => {
    console.log(`Kya Local Renamer: http://127.0.0.1:${port}`);
    console.log(`Repository root: ${repositoryRoot}`);
  });
}

export { app, repositoryRoot, runsRoot };
