import { mkdir, readFile, rename, writeFile } from "node:fs/promises";
import path from "node:path";
import type { EvaluationRun, ModelAvailabilityCacheEntry, ModelRateLimitStatus } from "./types.js";

function isActiveRateLimit(status: ModelRateLimitStatus, now = Date.now()): boolean {
  return new Date(status.blockedUntil).getTime() > now;
}

export class RunStore {
  readonly runsRoot: string;
  private writes = new Map<string, Promise<void>>();
  private rateLimitWrites: Promise<void> = Promise.resolve();
  private availabilityWrites: Promise<void> = Promise.resolve();

  constructor(runsRoot: string) {
    this.runsRoot = runsRoot;
  }

  runDirectory(runId: string): string {
    if (!/^[a-zA-Z0-9_-]+$/.test(runId)) throw new Error("Invalid run ID.");
    return path.join(this.runsRoot, runId);
  }

  private rateLimitCachePath(): string {
    return path.join(this.runsRoot, ".rate-limits.json");
  }

  private availabilityCachePath(): string {
    return path.join(this.runsRoot, ".available-models.json");
  }

  async getAvailableModels(): Promise<Record<string, ModelAvailabilityCacheEntry>> {
    try {
      const parsed = JSON.parse(await readFile(this.availabilityCachePath(), "utf8")) as unknown;
      if (typeof parsed !== "object" || parsed === null) return {};
      return Object.fromEntries(Object.entries(parsed).filter(
        (entry): entry is [string, ModelAvailabilityCacheEntry] =>
          typeof entry[1] === "object" && entry[1] !== null &&
          typeof (entry[1] as { checkedAt?: unknown }).checkedAt === "string"
      ));
    } catch {
      return {};
    }
  }

  async replaceAvailableModels(cache: Record<string, ModelAvailabilityCacheEntry>): Promise<void> {
    const previous = this.availabilityWrites;
    const next = previous.then(async () => {
      await mkdir(this.runsRoot, { recursive: true });
      const destination = this.availabilityCachePath();
      const temporary = `${destination}.${process.pid}.${Date.now()}.tmp`;
      await writeFile(temporary, JSON.stringify(cache, null, 2) + "\n", "utf8");
      await rename(temporary, destination);
    });
    this.availabilityWrites = next;
    await next;
  }

  async updateAvailableModels(updates: Record<string, ModelAvailabilityCacheEntry | null>): Promise<void> {
    const previous = this.availabilityWrites;
    const next = previous.then(async () => {
      const cache = await this.getAvailableModels();
      for (const [modelId, entry] of Object.entries(updates)) {
        if (entry) cache[modelId] = entry;
        else delete cache[modelId];
      }
      await mkdir(this.runsRoot, { recursive: true });
      const destination = this.availabilityCachePath();
      const temporary = `${destination}.${process.pid}.${Date.now()}.tmp`;
      await writeFile(temporary, JSON.stringify(cache, null, 2) + "\n", "utf8");
      await rename(temporary, destination);
    });
    this.availabilityWrites = next;
    await next;
  }

  private async loadRateLimitCache(): Promise<Record<string, ModelRateLimitStatus>> {
    try {
      return JSON.parse(await readFile(this.rateLimitCachePath(), "utf8")) as Record<string, ModelRateLimitStatus>;
    } catch {
      return {};
    }
  }

  private async saveRateLimitCache(cache: Record<string, ModelRateLimitStatus>): Promise<void> {
    await mkdir(this.runsRoot, { recursive: true });
    const destination = this.rateLimitCachePath();
    const temporary = `${destination}.${process.pid}.${Date.now()}.tmp`;
    await writeFile(temporary, JSON.stringify(cache, null, 2) + "\n", "utf8");
    await rename(temporary, destination);
  }

  async load(runId: string): Promise<EvaluationRun> {
    return JSON.parse(await readFile(path.join(this.runDirectory(runId), "run.json"), "utf8")) as EvaluationRun;
  }

  async save(run: EvaluationRun): Promise<void> {
    run.updatedAt = new Date().toISOString();
    const runId = run.manifest.runId;
    const previous = this.writes.get(runId) ?? Promise.resolve();
    const next = previous.then(async () => {
      const directory = this.runDirectory(runId);
      await mkdir(directory, { recursive: true });
      const destination = path.join(directory, "run.json");
      const temporary = path.join(directory, `run.json.${process.pid}.${Date.now()}.tmp`);
      await writeFile(temporary, JSON.stringify(run, null, 2) + "\n", "utf8");
      await rename(temporary, destination);
    });
    this.writes.set(runId, next);
    await next.finally(() => {
      if (this.writes.get(runId) === next) this.writes.delete(runId);
    });
  }

  async saveRawResponse(runId: string, requestKey: string, attempt: number, raw: string): Promise<string> {
    const relative = path.join("responses", requestKey, `attempt-${attempt}.txt`);
    const destination = path.join(this.runDirectory(runId), relative);
    await mkdir(path.dirname(destination), { recursive: true });
    const temporary = `${destination}.${process.pid}.tmp`;
    await writeFile(temporary, raw, "utf8");
    await rename(temporary, destination);
    return relative.replaceAll("\\", "/");
  }

  async markModelRateLimited(modelId: string, retryAfterMs: number, lastError?: string): Promise<void> {
    if (!Number.isFinite(retryAfterMs) || retryAfterMs <= 0) return;
    const previous = this.rateLimitWrites;
    const next = previous.then(async () => {
      const cache = await this.loadRateLimitCache();
      const now = Date.now();
      const previousEntry = cache[modelId];
      const failureCount = previousEntry && isActiveRateLimit(previousEntry, now) ? previousEntry.failureCount + 1 : 1;
      cache[modelId] = {
        blockedUntil: new Date(now + retryAfterMs).toISOString(),
        lastFailureAt: new Date(now).toISOString(),
        retryAfterMs,
        failureCount,
        ...(lastError ? { lastError } : {})
      };
      await this.saveRateLimitCache(cache);
    });
    this.rateLimitWrites = next;
    await next;
  }

  async clearModelRateLimit(modelId: string): Promise<void> {
    const previous = this.rateLimitWrites;
    const next = previous.then(async () => {
      const cache = await this.loadRateLimitCache();
      if (!(modelId in cache)) return;
      delete cache[modelId];
      await this.saveRateLimitCache(cache);
    });
    this.rateLimitWrites = next;
    await next;
  }

  async getActiveRateLimitedModels(now = new Date()): Promise<Record<string, ModelRateLimitStatus>> {
    const cache = await this.loadRateLimitCache();
    let changed = false;
    const active: Record<string, ModelRateLimitStatus> = {};
    for (const [modelId, status] of Object.entries(cache)) {
      if (isActiveRateLimit(status, now.getTime())) {
        active[modelId] = status;
      } else {
        changed = true;
      }
    }
    if (changed) await this.saveRateLimitCache(active);
    return active;
  }
}
