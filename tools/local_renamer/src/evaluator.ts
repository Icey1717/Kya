import { createHash } from "node:crypto";
import { readFile } from "node:fs/promises";
import path from "node:path";
import { requestSuggestions, OpenRouterRequestError } from "./openRouter.js";
import { buildReviewItems } from "./review.js";
import { requestKey } from "./runFactory.js";
import { RunStore } from "./store.js";
import type {
  AttemptRecord,
  EvaluationRun,
  FunctionRecord,
  ModelMetadata,
  RequestFailureKind,
  RequestRecord
} from "./types.js";
import { validateAndNormalizeResponse } from "./validation.js";

function retryAfterMsFromRateLimitError(error: OpenRouterRequestError): number {
  if (!error.rawResponse) return 120_000;
  try {
    const parsed = JSON.parse(error.rawResponse) as {
      error?: {
        metadata?: {
          retry_after_seconds?: unknown;
          retry_after_seconds_raw?: unknown;
          headers?: { [key: string]: unknown };
        };
      };
    };
    const metadata = parsed.error?.metadata;
    const seconds = Number(metadata?.retry_after_seconds ?? metadata?.retry_after_seconds_raw);
    if (Number.isFinite(seconds) && seconds > 0) return Math.ceil(seconds * 1000);
    const header = Number(metadata?.headers?.["Retry-After"] ?? metadata?.headers?.["retry-after"]);
    if (Number.isFinite(header) && header > 0) return Math.ceil(header * 1000);
  } catch {
    // Fall back to a short cooldown when the provider response is not parseable.
  }
  return 120_000;
}
function locateFunction(run: EvaluationRun, functionId: string): FunctionRecord {
  for (const file of run.manifest.files) {
    const fn = file.functions.find((candidate) => candidate.id === functionId);
    if (fn) return fn;
  }
  throw new Error(`Unknown function: ${functionId}`);
}

async function verifySourceHashes(run: EvaluationRun): Promise<void> {
  for (const file of run.manifest.files) {
    const absolute = path.resolve(run.manifest.repositoryRoot, file.relativePath);
    const current = createHash("sha256").update(await readFile(absolute)).digest("hex");
    if (current !== file.sha256) throw new Error(`Selected source changed since run creation: ${file.relativePath}`);
  }
}

export class EvaluationEngine {
  private pauseRequested = new Set<string>();
  private active = new Map<string, Promise<void>>();

  constructor(
    private readonly store: RunStore,
    private readonly apiKey: string,
    private readonly fetchImpl: typeof fetch = fetch
  ) {}

  start(runId: string): Promise<void> {
    const existing = this.active.get(runId);
    if (existing) return existing;
    this.pauseRequested.delete(runId);
    const task = this.execute(runId).finally(() => this.active.delete(runId));
    this.active.set(runId, task);
    return task;
  }

  requestPause(runId: string): void {
    this.pauseRequested.add(runId);
  }

  isActive(runId: string): boolean {
    return this.active.has(runId);
  }

  private async execute(runId: string): Promise<void> {
    const run = await this.store.load(runId);
    if (run.status === "finalized") throw new Error("Finalized runs cannot be resumed.");
    try {
      await verifySourceHashes(run);
      run.status = "running";
      run.error = undefined;
      await this.store.save(run);

      for (const file of run.manifest.files) {
        if (file.reviewAvailable) continue;
        for (const functionId of file.randomizedFunctionIds) {
          if (this.pauseRequested.has(runId)) {
            run.status = "paused";
            await this.store.save(run);
            return;
          }
          const fn = locateFunction(run, functionId);
          const models = run.manifest.models;
          for (let offset = 0; offset < models.length; offset += run.manifest.config.globalConcurrency) {
            if (this.pauseRequested.has(runId)) {
              run.status = "paused";
              await this.store.save(run);
              return;
            }
            await Promise.all(
              models
                .slice(offset, offset + run.manifest.config.globalConcurrency)
                .map((model) => this.evaluateRequest(run, fn, model))
            );
          }
        }
        const complete = file.randomizedFunctionIds.every((functionId) =>
          run.manifest.modelIds.every(
            (modelId) => run.requests[requestKey(functionId, modelId)]?.status !== "pending"
          )
        );
        if (complete) {
          file.reviewAvailable = true;
          await this.store.save(run);
        }
      }
      run.reviewItems = buildReviewItems(run);
      run.status = "review";
      await this.store.save(run);
    } catch (error) {
      run.status = "failed";
      run.error = error instanceof Error ? error.message : String(error);
      await this.store.save(run);
      throw error;
    }
  }

  private async evaluateRequest(run: EvaluationRun, fn: FunctionRecord, model: ModelMetadata): Promise<void> {
    const key = requestKey(fn.id, model.id);
    const request = run.requests[key];
    if (!request || request.status !== "pending") return;

    while (request.attempts.length < 2) {
      const attemptNumber = (request.attempts.length + 1) as 1 | 2;
      const started = Date.now();
      const startedAt = new Date(started).toISOString();
      let rawResponseFile: string | undefined;
      let provider: string | undefined;
      let promptTokens: number | undefined;
      let completionTokens: number | undefined;
      let totalTokens: number | undefined;
      let cost: number | undefined;
      try {
        const result = await requestSuggestions({
          apiKey: this.apiKey,
          model,
          fn,
          timeoutMs: run.manifest.config.timeoutMs,
          fetchImpl: this.fetchImpl
        });
        rawResponseFile = await this.store.saveRawResponse(run.manifest.runId, key, attemptNumber, result.raw);
        provider = result.provider;
        promptTokens = result.usage.prompt;
        completionTokens = result.usage.completion;
        totalTokens = result.usage.total;
        cost = result.usage.cost;
        let normalized;
        try {
          normalized = validateAndNormalizeResponse(result.content, fn.declarations, fn.parameters);
        } catch (error) {
          throw new OpenRouterRequestError(
            error instanceof Error ? error.message : String(error),
            "validation",
            true
          );
        }
        request.attempts.push({
          attempt: attemptNumber,
          startedAt,
          finishedAt: new Date().toISOString(),
          latencyMs: Date.now() - started,
          status: "success",
          provider,
          promptTokens,
          completionTokens,
          totalTokens,
          cost,
          rawResponseFile
        });
        request.normalized = normalized;
        request.status = "success";
        await this.store.clearModelRateLimit(model.id);
        await this.store.save(run);
        return;
      } catch (error) {
        const kind: RequestFailureKind =
          error instanceof OpenRouterRequestError ? error.kind : "transport";
        const retryable = error instanceof OpenRouterRequestError ? error.retryable : true;
        if (!rawResponseFile && error instanceof OpenRouterRequestError && error.rawResponse !== undefined) {
          rawResponseFile = await this.store.saveRawResponse(
            run.manifest.runId,
            key,
            attemptNumber,
            error.rawResponse
          );
        }
        if (error instanceof OpenRouterRequestError && error.kind === "rate_limit") {
          await this.store.markModelRateLimited(model.id, retryAfterMsFromRateLimitError(error), error.message);
        }
        const attempt: AttemptRecord = {
          attempt: attemptNumber,
          startedAt,
          finishedAt: new Date().toISOString(),
          latencyMs: Date.now() - started,
          status: "failure",
          failureKind: kind,
          error: error instanceof Error ? error.message : String(error),
          rawResponseFile,
          provider,
          promptTokens,
          completionTokens,
          totalTokens,
          cost
        };
        request.attempts.push(attempt);
        if (!retryable || attemptNumber === 2) request.status = "failed";
        await this.store.save(run);
        if (!retryable || attemptNumber === 2) return;
      }
    }
  }
}
