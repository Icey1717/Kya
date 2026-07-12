import { createHash, randomBytes } from "node:crypto";
import { mkdir, readFile, rename, writeFile } from "node:fs/promises";
import { setTimeout as delay } from "node:timers/promises";
import path from "node:path";
import { analyzeCppSource } from "./cppAnalyzer.js";
import { requestSuggestions } from "./openRouter.js";
import type { Declaration, FileRecord, FunctionRecord, ModelMetadata, NormalizedResponse, RequestRecord } from "./types.js";
import { validateAndNormalizeResponse, validateChangedName, validateNameWarnings } from "./validation.js";

export interface ApplicationChoice {
  decision: string;
  customName?: string;
  warnings?: string[];
  submittedAt: string;
}

export interface ApplicationItem {
  key: string;
  filePath: string;
  functionId: string;
  functionName: string;
  declarationId: string;
  oldName: string;
  choice?: ApplicationChoice;
  appliedAt?: string;
}

export interface ApplicationRun {
  schemaVersion: 1;
  runId: string;
  createdAt: string;
  updatedAt: string;
  repositoryRoot: string;
  modelIds: string[];
  models: ModelMetadata[];
  timeoutMs: number;
  files: FileRecord[];
  requests: Record<string, RequestRecord>;
  items: ApplicationItem[];
  status: "ready" | "running" | "review" | "applied" | "failed";
  error?: string;
}

export interface PublicApplicationItem {
  key: string;
  filePath: string;
  functionName: string;
  declarationId: string;
  oldName: string;
  functionSource: string;
  highlightRanges: Array<{ start: number; end: number }>;
  options: Array<{ decision: string; modelIds: string[]; confidences: number[]; warnings: string[] }>;
  failures: Array<{ modelId: string; error: string }>;
  choice?: ApplicationChoice;
}

function requestKey(functionId: string, modelId: string): string {
  return createHash("sha256").update(`${functionId}\0${modelId}`).digest("hex").slice(0, 24);
}

function itemKey(functionId: string, declarationId: string): string {
  return createHash("sha256").update(`${functionId}\0${declarationId}`).digest("hex").slice(0, 24);
}

function sourcePath(repositoryRoot: string, relativePath: string): string {
  if (path.isAbsolute(relativePath)) throw new Error(`File path must be repository-relative: ${relativePath}`);
  const normalized = relativePath.replaceAll("\\", "/");
  const absolute = path.resolve(repositoryRoot, normalized);
  const relative = path.relative(repositoryRoot, absolute);
  if (relative.startsWith("..") || path.isAbsolute(relative)) throw new Error(`File escapes repository root: ${relativePath}`);
  if (!/\.(?:cc|cpp|cxx)$/i.test(absolute)) throw new Error(`File is not a C++ source file: ${relativePath}`);
  return absolute;
}

function findFunction(run: ApplicationRun, functionId: string): FunctionRecord {
  for (const file of run.files) {
    const fn = file.functions.find((candidate) => candidate.id === functionId);
    if (fn) return fn;
  }
  throw new Error(`Unknown function: ${functionId}`);
}

function findDeclaration(run: ApplicationRun, item: ApplicationItem): Declaration {
  const declaration = findFunction(run, item.functionId).declarations.find((candidate) => candidate.id === item.declarationId);
  if (!declaration) throw new Error(`Unknown declaration: ${item.declarationId}`);
  return declaration;
}

export function createApplicationRun(input: {
  repositoryRoot: string;
  relativePaths: string[];
  models: ModelMetadata[];
  timeoutMs: number;
}): Promise<ApplicationRun> {
  return createApplicationRunInner(input);
}

async function createApplicationRunInner(input: {
  repositoryRoot: string;
  relativePaths: string[];
  models: ModelMetadata[];
  timeoutMs: number;
}): Promise<ApplicationRun> {
  if (input.relativePaths.length === 0) throw new Error("Select at least one source file.");
  if (input.models.length === 0) throw new Error("Run mode requires at least one configured model.");
  const paths = input.relativePaths.map((entry) => entry.replaceAll("\\", "/"));
  if (new Set(paths.map((entry) => entry.toLowerCase())).size !== paths.length) throw new Error("Selected files must be unique.");
  const files: FileRecord[] = [];
  for (const relativePath of paths) {
    const source = await readFile(sourcePath(input.repositoryRoot, relativePath), "utf8");
    const functions = analyzeCppSource(relativePath, source);
    files.push({
      relativePath,
      sha256: createHash("sha256").update(source).digest("hex"),
      functions,
      randomizedFunctionIds: functions.filter((fn) => !fn.exclusionReason).map((fn) => fn.id),
      reviewAvailable: false
    });
  }
  const requests: Record<string, RequestRecord> = {};
  const items: ApplicationItem[] = [];
  for (const file of files) {
    for (const fn of file.functions.filter((candidate) => !candidate.exclusionReason)) {
      for (const model of input.models) {
        requests[requestKey(fn.id, model.id)] = { modelId: model.id, functionId: fn.id, filePath: file.relativePath, status: "pending", attempts: [] };
      }
      for (const declaration of fn.declarations) {
        items.push({ key: itemKey(fn.id, declaration.id), filePath: file.relativePath, functionId: fn.id, functionName: fn.qualifiedName, declarationId: declaration.id, oldName: declaration.oldName });
      }
    }
  }
  const now = new Date().toISOString();
  return {
    schemaVersion: 1,
    runId: `${new Date().toISOString().replace(/[-:.TZ]/g, "").slice(0, 14)}-${randomBytes(4).toString("hex")}`,
    createdAt: now,
    updatedAt: now,
    repositoryRoot: input.repositoryRoot,
    modelIds: input.models.map((model) => model.id),
    models: input.models,
    timeoutMs: input.timeoutMs,
    files,
    requests,
    items,
    status: "ready"
  };
}

async function renameWithRetry(source: string, destination: string): Promise<void> {
  const retries = 8;
  for (let attempt = 0; ; attempt++) {
    try {
      await rename(source, destination);
      return;
    } catch (error) {
      const code = typeof error === "object" && error !== null && "code" in error ? (error as { code?: unknown }).code : undefined;
      if (attempt >= retries || (code !== "EPERM" && code !== "EBUSY")) throw error;
      await delay(25 * (attempt + 1));
    }
  }
}
export class ApplicationStore {
  private writes = new Map<string, Promise<void>>();

  constructor(private readonly root: string) {}

  private directory(runId: string): string {
    if (!/^[a-zA-Z0-9_-]+$/.test(runId)) throw new Error("Invalid run ID.");
    return path.join(this.root, runId);
  }

  async load(runId: string): Promise<ApplicationRun> {
    return JSON.parse(await readFile(path.join(this.directory(runId), "run.json"), "utf8")) as ApplicationRun;
  }

  async save(run: ApplicationRun): Promise<void> {
    run.updatedAt = new Date().toISOString();
    const previous = (this.writes.get(run.runId) ?? Promise.resolve()).catch(() => undefined);
    const next = previous.then(async () => {
      const directory = this.directory(run.runId);
      await mkdir(directory, { recursive: true });
      const destination = path.join(directory, "run.json");
      const temporary = `${destination}.${process.pid}.${Date.now()}.tmp`;
      await writeFile(temporary, JSON.stringify(run, null, 2) + "\n", "utf8");
      await renameWithRetry(temporary, destination);
    });
    this.writes.set(run.runId, next);
    await next.finally(() => {
      if (this.writes.get(run.runId) === next) this.writes.delete(run.runId);
    });
  }
}

export class ApplicationEngine {
  private active = new Map<string, Promise<void>>();

  constructor(private readonly store: ApplicationStore, private readonly apiKey: string, private readonly fetchImpl: typeof fetch = fetch) {}

  isActive(runId: string): boolean { return this.active.has(runId); }

  start(runId: string): Promise<void> {
    const existing = this.active.get(runId);
    if (existing) return existing;
    const task = this.execute(runId).finally(() => this.active.delete(runId));
    this.active.set(runId, task);
    return task;
  }

  private async execute(runId: string): Promise<void> {
    const run = await this.store.load(runId);
    if (run.status !== "ready" && run.status !== "running") throw new Error(`Cannot start a ${run.status} application run.`);
    try {
      run.status = "running";
      await this.store.save(run);
      for (const file of run.files) {
        for (const fn of file.functions.filter((candidate) => !candidate.exclusionReason)) {
          await Promise.all(run.models.map((model) => this.request(run, fn, model)));
        }
      }
      run.status = "review";
      await this.store.save(run);
    } catch (error) {
      run.status = "failed";
      run.error = error instanceof Error ? error.message : String(error);
      await this.store.save(run);
      throw error;
    }
  }

  private async request(run: ApplicationRun, fn: FunctionRecord, model: ModelMetadata): Promise<void> {
    const request = run.requests[requestKey(fn.id, model.id)];
    if (!request || request.status !== "pending") return;
    const started = Date.now();
    try {
      const result = await requestSuggestions({ apiKey: this.apiKey, model, fn, timeoutMs: run.timeoutMs, fetchImpl: this.fetchImpl });
      request.normalized = validateAndNormalizeResponse(result.content, fn.declarations, fn.parameters);
      request.status = "success";
      request.attempts.push({ attempt: 1, startedAt: new Date(started).toISOString(), finishedAt: new Date().toISOString(), latencyMs: Date.now() - started, status: "success", provider: result.provider, promptTokens: result.usage.prompt, completionTokens: result.usage.completion, totalTokens: result.usage.total, cost: result.usage.cost });
    } catch (error) {
      request.status = "failed";
      request.attempts.push({ attempt: 1, startedAt: new Date(started).toISOString(), finishedAt: new Date().toISOString(), latencyMs: Date.now() - started, status: "failure", failureKind: "validation", error: error instanceof Error ? error.message : String(error) });
    }
    await this.store.save(run);
  }
}

export function publicApplicationRun(run: ApplicationRun, active: boolean): object {
  const requests = Object.values(run.requests);
  const appliedItems = run.items.filter((item) => item.appliedAt);
  const pendingItems = run.items.filter((item) => !item.appliedAt);
  return {
    runId: run.runId,
    status: run.status,
    error: run.error,
    active,
    modelIds: run.modelIds,
    files: run.files.map((file) => ({ relativePath: file.relativePath, functionCount: file.functions.length, eligibleFunctionCount: file.functions.filter((fn) => !fn.exclusionReason).length })),
    requestStats: { pending: requests.filter((request) => request.status === "pending").length, success: requests.filter((request) => request.status === "success").length, failed: requests.filter((request) => request.status === "failed").length },
    reviewCount: pendingItems.length,
    submittedChoiceCount: pendingItems.filter((item) => item.choice).length,
    totalItemCount: run.items.length,
    appliedItemCount: appliedItems.length,
    selectedItemCount: run.items.filter((item) => item.choice).length,
    readyFunctions: [...new Map(pendingItems.map((item) => [item.functionId, item.functionName])).entries()]
      .filter(([functionId]) => pendingItems.filter((item) => item.functionId === functionId).every((item) => item.choice))
      .map(([functionId, functionName]) => {
        const items = pendingItems.filter((item) => item.functionId === functionId);
        const changedItems = items.filter((item) => item.choice?.decision !== item.oldName);
        return {
          functionId,
          functionName,
          declarationCount: items.length,
          changedDeclarationCount: changedItems.length,
          occurrenceCount: changedItems.reduce((sum, item) => sum + (findDeclaration(run, item).occurrences?.length ?? 0), 0)
        };
      })
  };
}

export function publicApplicationItems(run: ApplicationRun): PublicApplicationItem[] {
  return run.items.filter((item) => !item.appliedAt).map((item) => {
    const fn = findFunction(run, item.functionId);
    const declaration = findDeclaration(run, item);
    const options = new Map<string, { decision: string; modelIds: string[]; confidences: number[]; warnings: string[] }>();
    for (const modelId of run.modelIds) {
      const suggestion = run.requests[requestKey(fn.id, modelId)]?.normalized?.suggestions.find((entry) => entry.declaration_id === declaration.id);
      if (!suggestion) continue;
      const decision = suggestion.keep ? declaration.oldName : suggestion.decision;
      const option = options.get(decision) ?? {
        decision,
        modelIds: [],
        confidences: [],
        warnings: suggestion.keep ? [] : (suggestion.warnings ?? validateNameWarnings(decision))
      };
      option.modelIds.push(modelId);
      option.confidences.push(suggestion.confidence);
      options.set(decision, option);
    }
    return {
      key: item.key,
      filePath: item.filePath,
      functionName: item.functionName,
      declarationId: item.declarationId,
      oldName: item.oldName,
      functionSource: fn.source,
      highlightRanges: declaration.occurrences?.map((occurrence) => ({ start: occurrence.startIndex - fn.startIndex, end: occurrence.endIndex - fn.startIndex })) ?? [],
      options: [...options.values()],
      failures: run.modelIds.flatMap((modelId) => {
        const request = run.requests[requestKey(fn.id, modelId)];
        const error = request?.attempts.at(-1)?.error;
        return request?.status === "failed" && error ? [{ modelId, error }] : [];
      }),
      choice: item.choice
    };
  });
}

export function submitApplicationChoice(run: ApplicationRun, itemKeyValue: string, input: { decision?: unknown; customName?: unknown }): ApplicationChoice {
  if (run.status !== "review") throw new Error("Choices are accepted only during review.");
  const item = run.items.find((candidate) => candidate.key === itemKeyValue);
  if (!item) throw new Error("Unknown application declaration.");
  const declaration = findDeclaration(run, item);
  const customName = typeof input.customName === "string" ? input.customName.trim() : "";
  const decision = customName || (typeof input.decision === "string" ? input.decision : "");
  const validDecisions = new Set([
    declaration.oldName,
    ...(publicApplicationItems(run).find((candidate) => candidate.key === item.key)?.options.map((option) => option.decision) ?? [])
  ]);
  if (!customName && !validDecisions.has(decision)) throw new Error("Select a model suggestion, use the existing name, or enter a custom name.");
  if (decision !== declaration.oldName) {
    const errors = validateChangedName(decision, declaration);
    if (errors.length > 0) throw new Error(`Invalid name: ${errors.join(", ")}`);
  }
  const choice = {
    decision,
    ...(customName ? { customName } : {}),
    ...(decision === declaration.oldName ? {} : { warnings: validateNameWarnings(decision) }),
    submittedAt: new Date().toISOString()
  };
  const previousChoice = item.choice;
  item.choice = choice;
  try {
    validateApplicationChoices(run);
  } catch (error) {
    item.choice = previousChoice;
    throw error;
  }
  const functionItems = run.items.filter((candidate) => candidate.functionId === item.functionId && !candidate.appliedAt);
  if (functionItems.length > 0 && functionItems.every((candidate) => candidate.choice?.decision === candidate.oldName)) {
    const appliedAt = new Date().toISOString();
    for (const candidate of functionItems) candidate.appliedAt = appliedAt;
    if (run.items.every((candidate) => candidate.appliedAt)) run.status = "applied";
  }
  return choice;
}

function validateApplicationChoices(run: ApplicationRun): void {
  for (const file of run.files) {
    for (const fn of file.functions.filter((candidate) => !candidate.exclusionReason)) {
      const names = new Map<string, Array<{ original: string; changed: boolean }>>();
      for (const parameter of fn.parameters) names.set(parameter, [{ original: parameter, changed: false }]);
      for (const declaration of fn.declarations) {
        const item = run.items.find((candidate) => candidate.functionId === fn.id && candidate.declarationId === declaration.id);
        const name = item?.choice?.decision ?? declaration.oldName;
        names.set(name, [...(names.get(name) ?? []), { original: declaration.oldName, changed: name !== declaration.oldName }]);
      }
      for (const [name, entries] of names) {
        if (entries.length > 1 && !entries.every((entry) => !entry.changed && entry.original === name)) throw new Error(`Function-wide name collision: ${name}`);
      }
    }
  }
}

export async function applyApplicationFunction(run: ApplicationRun, functionId: string): Promise<void> {
  if (run.status !== "review") throw new Error("Only a review-stage application run can be applied.");
  const file = run.files.find((candidate) => candidate.functions.some((fn) => fn.id === functionId));
  const fn = file?.functions.find((candidate) => candidate.id === functionId);
  if (!file || !fn) throw new Error("Unknown application function.");
  const items = run.items.filter((item) => item.functionId === functionId && !item.appliedAt);
  if (items.length === 0) throw new Error("This function has already been applied.");
  if (items.some((item) => !item.choice)) throw new Error("Choose a name for every local in this function before applying it.");
  validateApplicationChoices(run);

  const destination = sourcePath(run.repositoryRoot, file.relativePath);
  const source = await readFile(destination, "utf8");
  if (createHash("sha256").update(source).digest("hex") !== file.sha256) {
    throw new Error(`Selected source changed since run creation: ${file.relativePath}`);
  }
  const replacements: Array<{ start: number; end: number; oldName: string; name: string }> = [];
  for (const declaration of fn.declarations) {
    const item = items.find((candidate) => candidate.declarationId === declaration.id);
    const name = item?.choice?.decision ?? declaration.oldName;
    if (name === declaration.oldName) continue;
    for (const occurrence of declaration.occurrences ?? []) replacements.push({ start: occurrence.startIndex, end: occurrence.endIndex, oldName: declaration.oldName, name });
  }
  let updated = source;
  for (const replacement of [...replacements].sort((left, right) => right.start - left.start)) {
    if (updated.slice(replacement.start, replacement.end) !== replacement.oldName) {
      throw new Error(`Source no longer matches ${replacement.oldName} in ${file.relativePath}`);
    }
    updated = updated.slice(0, replacement.start) + replacement.name + updated.slice(replacement.end);
  }
  if (updated !== source) {
    const temporary = `${destination}.${process.pid}.${Date.now()}.tmp`;
    await writeFile(temporary, updated, "utf8");
    await rename(temporary, destination);
  }
  file.sha256 = createHash("sha256").update(updated).digest("hex");

  const shiftIndex = (index: number): number => index + replacements.reduce(
    (delta, replacement) => replacement.start < index ? delta + replacement.name.length - replacement.oldName.length : delta,
    0
  );
  for (const candidate of file.functions) {
    if (candidate.id === functionId) continue;
    candidate.startIndex = shiftIndex(candidate.startIndex);
    candidate.endIndex = shiftIndex(candidate.endIndex);
    for (const declaration of candidate.declarations) {
      declaration.startIndex = shiftIndex(declaration.startIndex);
      declaration.endIndex = shiftIndex(declaration.endIndex);
      declaration.occurrences = declaration.occurrences?.map((occurrence) => ({
        startIndex: shiftIndex(occurrence.startIndex),
        endIndex: shiftIndex(occurrence.endIndex)
      }));
    }
  }
  const appliedAt = new Date().toISOString();
  for (const item of items) item.appliedAt = appliedAt;
  if (run.items.every((item) => item.appliedAt)) run.status = "applied";
}
export async function applyApplicationRun(run: ApplicationRun): Promise<void> {
  if (run.status !== "review") throw new Error("Only a review-stage application run can be applied.");
  if (run.items.some((item) => !item.choice)) throw new Error("Choose a name for every declaration before applying changes.");
  validateApplicationChoices(run);
  const sources = new Map<string, { destination: string; source: string }>();
  for (const file of run.files) {
    const destination = sourcePath(run.repositoryRoot, file.relativePath);
    const source = await readFile(destination, "utf8");
    if (createHash("sha256").update(source).digest("hex") !== file.sha256) {
      throw new Error(`Selected source changed since run creation: ${file.relativePath}`);
    }
    sources.set(file.relativePath, { destination, source });
  }
  for (const file of run.files) {
    const current = sources.get(file.relativePath)!;
    const replacements: Array<{ start: number; end: number; oldName: string; name: string }> = [];
    for (const fn of file.functions) {
      for (const declaration of fn.declarations) {
        const item = run.items.find((candidate) => candidate.functionId === fn.id && candidate.declarationId === declaration.id);
        const name = item?.choice?.decision ?? declaration.oldName;
        if (name === declaration.oldName) continue;
        for (const occurrence of declaration.occurrences ?? []) replacements.push({ start: occurrence.startIndex, end: occurrence.endIndex, oldName: declaration.oldName, name });
      }
    }
    let updated = current.source;
    for (const replacement of replacements.sort((left, right) => right.start - left.start)) {
      if (updated.slice(replacement.start, replacement.end) !== replacement.oldName) throw new Error(`Source no longer matches ${replacement.oldName} in ${file.relativePath}`);
      updated = updated.slice(0, replacement.start) + replacement.name + updated.slice(replacement.end);
    }
    if (updated === current.source) continue;
    const temporary = `${current.destination}.${process.pid}.${Date.now()}.tmp`;
    await writeFile(temporary, updated, "utf8");
    await rename(temporary, current.destination);
  }
  run.status = "applied";
}
