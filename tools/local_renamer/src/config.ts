import config from "../config.json" with { type: "json" };

interface LocalRenamerConfig {
  runModelIds?: unknown;
  runTimeoutMs?: unknown;
}

const parsed = config as LocalRenamerConfig;

if (!Array.isArray(parsed.runModelIds) || parsed.runModelIds.length === 0 || !parsed.runModelIds.every((id) => typeof id === "string")) {
  throw new Error("config.json must contain a non-empty runModelIds array.");
}
if (new Set(parsed.runModelIds).size !== parsed.runModelIds.length) {
  throw new Error("config.json runModelIds must be unique.");
}
if (parsed.runTimeoutMs !== undefined && (typeof parsed.runTimeoutMs !== "number" || !Number.isFinite(parsed.runTimeoutMs) || parsed.runTimeoutMs <= 0)) {
  throw new Error("config.json runTimeoutMs must be a positive number.");
}

export const runModeConfig = {
  modelIds: [...parsed.runModelIds],
  timeoutMs: parsed.runTimeoutMs ?? 120_000
};
