import type { ModelMetadata, ModelRateLimitStatus } from "./types.js";

interface CatalogModel {
  id?: unknown;
  name?: unknown;
  context_length?: unknown;
  pricing?: { prompt?: unknown; completion?: unknown };
  architecture?: {
    modality?: unknown;
    input_modalities?: unknown;
    output_modalities?: unknown;
  };
  supported_parameters?: unknown;
  [key: string]: unknown;
}

export const paidModelIds = [
  "google/gemini-2.5-flash-lite",
  "openai/gpt-4o-mini",
  "google/gemma-4-31b-it",
  "openai/gpt-5.4-nano",
  "qwen/qwen3.6-35b-a3b",
  "deepseek/deepseek-chat-v3-0324",
  "meta-llama/llama-4-maverick",
  "minimax/minimax-m2.5"
] as const;
const paidModelIdSet = new Set<string>(paidModelIds);

export function isPaidModel(modelId: string): boolean {
  return paidModelIdSet.has(modelId);
}

function stringArray(value: unknown): string[] {
  return Array.isArray(value) ? value.filter((entry): entry is string => typeof entry === "string") : [];
}

function modalities(model: CatalogModel, direction: "input" | "output"): string[] {
  const explicit = stringArray(model.architecture?.[direction === "input" ? "input_modalities" : "output_modalities"]);
  if (explicit.length > 0) return explicit;
  const modality = typeof model.architecture?.modality === "string" ? model.architecture.modality : "";
  const [input, output] = modality.split("->").map((part) => part?.trim());
  return (direction === "input" ? input : output)?.split("+").filter(Boolean) ?? [];
}

export function filterTextModels(payload: unknown): ModelMetadata[] {
  const data =
    typeof payload === "object" && payload !== null && Array.isArray((payload as { data?: unknown }).data)
      ? (payload as { data: CatalogModel[] }).data
      : [];

  return data
    .filter((model) => typeof model.id === "string" && model.id.length > 0)
    .map((model) => {
      const promptPrice = Number(model.pricing?.prompt);
      const completionPrice = Number(model.pricing?.completion);
      return {
        id: model.id as string,
        name: typeof model.name === "string" ? model.name : (model.id as string),
        contextLength: Number(model.context_length) || 0,
        promptPrice,
        completionPrice,
        inputModalities: modalities(model, "input"),
        outputModalities: modalities(model, "output"),
        supportedParameters: stringArray(model.supported_parameters),
        raw: model
      };
    })
    .filter(
      (model) =>
        Number.isFinite(model.promptPrice) &&
        Number.isFinite(model.completionPrice) &&
        model.promptPrice >= 0 &&
        model.completionPrice >= 0 &&
        model.contextLength > 0 &&
        model.inputModalities.includes("text") &&
        model.outputModalities.includes("text")
    )
    .sort((left, right) => left.id.localeCompare(right.id));
}

export function filterFreeTextModels(payload: unknown): ModelMetadata[] {
  return filterTextModels(payload).filter((model) => model.promptPrice === 0 && model.completionPrice === 0);
}

export function filterSelectableTextModels(payload: unknown): ModelMetadata[] {
  return filterTextModels(payload).filter(
    (model) => (model.promptPrice === 0 && model.completionPrice === 0) || isPaidModel(model.id)
  );
}

export async function fetchFreeModels(fetchImpl: typeof fetch = fetch): Promise<ModelMetadata[]> {
  const response = await fetchImpl("https://openrouter.ai/api/v1/models?output_modalities=text", {
    headers: { Accept: "application/json" }
  });
  if (!response.ok) throw new Error(`OpenRouter catalog request failed: HTTP ${response.status}`);
  return filterFreeTextModels(await response.json());
}

export async function fetchSelectableModels(fetchImpl: typeof fetch = fetch): Promise<ModelMetadata[]> {
  const response = await fetchImpl("https://openrouter.ai/api/v1/models?output_modalities=text", {
    headers: { Accept: "application/json" }
  });
  if (!response.ok) throw new Error(`OpenRouter catalog request failed: HTTP ${response.status}`);
  return filterSelectableTextModels(await response.json());
}

export function applyRateLimitStatus(
  catalog: ModelMetadata[],
  rateLimited: Record<string, ModelRateLimitStatus>
): Array<ModelMetadata & { rateLimitedUntil?: string; rateLimitNote?: string }> {
  return catalog.map((model) => {
    const status = rateLimited[model.id];
    if (!status) return model;
    return {
      ...model,
      rateLimitedUntil: status.blockedUntil,
      rateLimitNote: status.lastError ?? `temporarily rate limited for about ${Math.max(1, Math.round(status.retryAfterMs / 1000))}s`
    };
  });
}

export function validateModelSelection(
  catalog: ModelMetadata[],
  modelIds: string[],
  rateLimitedModelIds: Set<string> = new Set()
): ModelMetadata[] {
  if (modelIds.length < 2) throw new Error("Select at least two models.");
  if (new Set(modelIds).size !== modelIds.length) throw new Error("Model IDs must be unique.");
  const byId = new Map(catalog.map((model) => [model.id, model]));
  return modelIds.map((id) => {
    if (rateLimitedModelIds.has(id)) throw new Error(`Model is temporarily rate limited: ${id}`);
    const model = byId.get(id);
    if (!model) throw new Error(`Model is not currently available as a compatible free model: ${id}`);
    return model;
  });
}
