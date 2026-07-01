import { createHash, randomBytes, randomInt } from "node:crypto";
import { readFile } from "node:fs/promises";
import path from "node:path";
import { analyzeCppSource } from "./cppAnalyzer.js";
import { estimatePromptTokens, outputTokenLimit } from "./prompt.js";
import type { EvaluationRun, FileRecord, ModelMetadata, RequestRecord } from "./types.js";

function safeSourcePath(repositoryRoot: string, relativePath: string): string {
  if (path.isAbsolute(relativePath)) throw new Error(`File path must be repository-relative: ${relativePath}`);
  const normalized = relativePath.replaceAll("\\", "/");
  const absolute = path.resolve(repositoryRoot, normalized);
  const relative = path.relative(repositoryRoot, absolute);
  if (relative.startsWith("..") || path.isAbsolute(relative)) throw new Error(`File escapes repository root: ${relativePath}`);
  if (!/\.(?:cc|cpp|cxx)$/i.test(absolute)) throw new Error(`File is not a C++ source file: ${relativePath}`);
  return absolute;
}

function shuffle<T>(items: T[]): T[] {
  const result = [...items];
  for (let index = result.length - 1; index > 0; index--) {
    const other = randomInt(index + 1);
    [result[index], result[other]] = [result[other], result[index]];
  }
  return result;
}

export async function createEvaluationRun(input: {
  repositoryRoot: string;
  requestedFileCount: number;
  relativePaths: string[];
  models: ModelMetadata[];
  timeoutMs?: number;
}): Promise<EvaluationRun> {
  if (!Number.isInteger(input.requestedFileCount) || input.requestedFileCount < 1) {
    throw new Error("File count must be a positive integer.");
  }
  if (input.relativePaths.length !== input.requestedFileCount) {
    throw new Error("Select exactly the requested number of files.");
  }
  const normalizedPaths = input.relativePaths.map((entry) => entry.replaceAll("\\", "/"));
  if (new Set(normalizedPaths.map((entry) => entry.toLowerCase())).size !== normalizedPaths.length) {
    throw new Error("Selected files must be unique.");
  }
  if (input.models.length < 2) throw new Error("Select at least two models.");
  if (input.timeoutMs !== undefined && (!Number.isFinite(input.timeoutMs) || input.timeoutMs <= 0)) {
    throw new Error("Request timeout must be a positive number of milliseconds.");
  }

  const minContext = Math.min(...input.models.map((model) => model.contextLength));
  const files: FileRecord[] = [];
  for (const relativePath of normalizedPaths) {
    const source = await readFile(safeSourcePath(input.repositoryRoot, relativePath), "utf8");
    const functions = analyzeCppSource(relativePath, source);
    for (const fn of functions) {
      fn.estimatedPromptTokens = estimatePromptTokens(fn);
      if (
        !fn.exclusionReason &&
        fn.estimatedPromptTokens + outputTokenLimit(fn.declarations.length) > minContext
      ) {
        fn.exclusionReason = "context_limit";
      }
    }
    files.push({
      relativePath,
      sha256: createHash("sha256").update(source).digest("hex"),
      functions,
      randomizedFunctionIds: shuffle(functions.filter((fn) => !fn.exclusionReason).map((fn) => fn.id)),
      reviewAvailable: false
    });
  }

  const eligibleFunctions = files.flatMap((file) => file.functions).filter((fn) => !fn.exclusionReason);
  const runId = `${new Date().toISOString().replace(/[-:.TZ]/g, "").slice(0, 14)}-${randomBytes(4).toString("hex")}`;
  const requests: Record<string, RequestRecord> = {};
  for (const file of files) {
    for (const functionId of file.randomizedFunctionIds) {
      for (const model of input.models) {
        const key = requestKey(functionId, model.id);
        requests[key] = {
          modelId: model.id,
          functionId,
          filePath: file.relativePath,
          status: "pending",
          attempts: []
        };
      }
    }
  }
  const now = new Date().toISOString();
  const primaryRequestCount = eligibleFunctions.length * input.models.length;
  return {
    manifest: {
      schemaVersion: 1,
      runId,
      createdAt: now,
      repositoryRoot: input.repositoryRoot,
      requestedFileCount: input.requestedFileCount,
      modelIds: input.models.map((model) => model.id),
      models: input.models,
      files,
      config: {
        timeoutMs: input.timeoutMs ?? 120_000,
        temperature: 0,
        globalConcurrency: 3,
        maxPerModel: 1
      },
      primaryRequestCount,
      worstCaseRequestCount: primaryRequestCount * 2
    },
    status: "ready",
    requests,
    reviewItems: [],
    retainedModelIds: [],
    updatedAt: now
  };
}

export function requestKey(functionId: string, modelId: string): string {
  return createHash("sha256").update(`${functionId}\0${modelId}`).digest("hex").slice(0, 24);
}
