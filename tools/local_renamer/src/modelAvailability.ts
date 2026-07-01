import type { ModelMetadata } from "./types.js";
import { isPaidModel } from "./catalog.js";

export interface ModelAvailabilityResult {
  modelId: string;
  available: boolean;
  checkedAt: string;
  note?: string;
}

export class OpenRouterAuthenticationError extends Error {}

function errorMessage(raw: string, status: number): string {
  try {
    const decoded = JSON.parse(raw) as { error?: { message?: unknown } | string; message?: unknown };
    if (typeof decoded.error === "string") return decoded.error;
    if (typeof decoded.error?.message === "string") return decoded.error.message;
    if (typeof decoded.message === "string") return decoded.message;
  } catch {
    // Fall back to the response text below.
  }
  return raw.trim().slice(0, 300) || `OpenRouter returned HTTP ${status}`;
}

export async function probeModelAvailability(
  model: ModelMetadata,
  apiKey: string,
  fetchImpl: typeof fetch = fetch,
  timeoutMs = 20_000
): Promise<ModelAvailabilityResult> {
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), timeoutMs);
  const checkedAt = new Date().toISOString();
  try {
    const response = await fetchImpl("https://openrouter.ai/api/v1/chat/completions", {
      method: "POST",
      signal: controller.signal,
      headers: {
        Authorization: `Bearer ${apiKey}`,
        "Content-Type": "application/json",
        "HTTP-Referer": "http://localhost:4310",
        "X-Title": "Kya Local Renamer Availability Test"
      },
      body: JSON.stringify({
        model: model.id,
        messages: [{ role: "user", content: "Reply with OK." }],
        temperature: 0,
        max_tokens: 1,
        stream: false,
        provider: isPaidModel(model.id) ? {} : { max_price: { prompt: 0, completion: 0 } }
      })
    });
    const raw = await response.text();
    if (response.status === 401 || response.status === 403) {
      throw new OpenRouterAuthenticationError(errorMessage(raw, response.status));
    }
    return response.ok
      ? { modelId: model.id, available: true, checkedAt }
      : { modelId: model.id, available: false, checkedAt, note: errorMessage(raw, response.status) };
  } catch (error) {
    if (error instanceof OpenRouterAuthenticationError) throw error;
    const note = controller.signal.aborted
      ? `Availability test timed out after ${Math.round(timeoutMs / 1000)} seconds.`
      : error instanceof Error ? error.message : String(error);
    return { modelId: model.id, available: false, checkedAt, note };
  } finally {
    clearTimeout(timeout);
  }
}

export async function probeModelsAvailability(
  models: ModelMetadata[],
  apiKey: string,
  fetchImpl: typeof fetch = fetch,
  concurrency = 3
): Promise<ModelAvailabilityResult[]> {
  const results = new Array<ModelAvailabilityResult>(models.length);
  let nextIndex = 0;
  async function worker(): Promise<void> {
    while (nextIndex < models.length) {
      const index = nextIndex++;
      results[index] = await probeModelAvailability(models[index], apiKey, fetchImpl);
    }
  }
  await Promise.all(Array.from({ length: Math.min(Math.max(1, concurrency), models.length) }, () => worker()));
  return results;
}
