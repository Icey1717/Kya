import { buildPrompt, outputTokenLimit } from "./prompt.js";
import { isPaidModel } from "./catalog.js";
import type { FunctionRecord, ModelMetadata, RequestFailureKind } from "./types.js";
import { suggestionJsonSchema } from "./validation.js";

export class OpenRouterRequestError extends Error {
  constructor(
    message: string,
    readonly kind: RequestFailureKind,
    readonly retryable: boolean,
    readonly rawResponse?: string
  ) {
    super(message);
  }
}

export interface OpenRouterResult {
  content: string;
  raw: string;
  provider?: string;
  usage: { prompt?: number; completion?: number; total?: number; cost?: number };
}

function supportsStructuredOutput(model: ModelMetadata): boolean {
  return model.supportedParameters.includes("structured_outputs");
}

function reasoningConfig(model: ModelMetadata): object | undefined {
  if (!model.supportedParameters.includes("reasoning")) return undefined;
  const metadata = typeof model.raw === "object" && model.raw !== null
    ? (model.raw as { reasoning?: unknown }).reasoning
    : undefined;
  const reasoning = typeof metadata === "object" && metadata !== null
    ? metadata as { mandatory?: unknown; supported_efforts?: unknown }
    : undefined;
  if (reasoning?.mandatory !== true) return { effort: "none", exclude: true };
  const supported = Array.isArray(reasoning.supported_efforts)
    ? reasoning.supported_efforts.filter((effort): effort is string => typeof effort === "string")
    : [];
  const lowestEffort = ["minimal", "low", "medium", "high", "xhigh", "max"].find((effort) => supported.includes(effort));
  return lowestEffort ? { effort: lowestEffort, exclude: true } : { exclude: true };
}

export async function requestSuggestions(input: {
  apiKey: string;
  model: ModelMetadata;
  fn: FunctionRecord;
  timeoutMs: number;
  fetchImpl?: typeof fetch;
}): Promise<OpenRouterResult> {
  const fetchImpl = input.fetchImpl ?? fetch;
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), input.timeoutMs);
  const structured = supportsStructuredOutput(input.model);
  const body: Record<string, unknown> = {
    model: input.model.id,
    messages: [{ role: "user", content: buildPrompt(input.fn) }],
    temperature: 0,
    max_tokens: outputTokenLimit(input.fn.declarations.length),
    stream: false,
    provider: {
      ...(isPaidModel(input.model.id) ? {} : { max_price: { prompt: 0, completion: 0 } }),
      ...(structured ? { require_parameters: true } : {})
    }
  };
  const reasoning = reasoningConfig(input.model);
  if (reasoning) body.reasoning = reasoning;
  if (structured) {
    body.response_format = {
      type: "json_schema",
      json_schema: {
        name: "local_variable_suggestions",
        strict: true,
        schema: suggestionJsonSchema(input.fn.declarations)
      }
    };
  }

  try {
    const response = await fetchImpl("https://openrouter.ai/api/v1/chat/completions", {
      method: "POST",
      signal: controller.signal,
      headers: {
        Authorization: `Bearer ${input.apiKey}`,
        "Content-Type": "application/json",
        "HTTP-Referer": "http://localhost:4310",
        "X-Title": "Kya Local Renamer Evaluator"
      },
      body: JSON.stringify(body)
    });
    const raw = await response.text();
    if (!response.ok) {
      const message = `OpenRouter HTTP ${response.status}: ${raw.slice(0, 500)}`;
      if (response.status === 429) throw new OpenRouterRequestError(message, "rate_limit", true, raw);
      if (response.status === 408) throw new OpenRouterRequestError(message, "timeout", true, raw);
      if (/schema|response_format|structured/i.test(raw)) {
        throw new OpenRouterRequestError(message, "schema", true, raw);
      }
      if (response.status >= 500) throw new OpenRouterRequestError(message, "transport", true, raw);
      throw new OpenRouterRequestError(message, "http", false, raw);
    }

    let decoded: {
      provider?: unknown;
      choices?: Array<{ message?: { content?: unknown } }>;
      usage?: { prompt_tokens?: unknown; completion_tokens?: unknown; total_tokens?: unknown; cost?: unknown };
    };
    try {
      decoded = JSON.parse(raw);
    } catch {
      throw new OpenRouterRequestError("OpenRouter returned malformed JSON.", "schema", true, raw);
    }
    const content = decoded.choices?.[0]?.message?.content;
    if (typeof content !== "string") {
      throw new OpenRouterRequestError("OpenRouter response has no text content.", "schema", true);
    }
    return {
      content,
      raw,
      provider: typeof decoded.provider === "string" ? decoded.provider : undefined,
      usage: {
        prompt: Number(decoded.usage?.prompt_tokens) || undefined,
        completion: Number(decoded.usage?.completion_tokens) || undefined,
        total: Number(decoded.usage?.total_tokens) || undefined,
        cost: typeof decoded.usage?.cost === "number" && Number.isFinite(decoded.usage.cost)
          ? decoded.usage.cost
          : undefined
      }
    };
  } catch (error) {
    if (error instanceof OpenRouterRequestError) throw error;
    if (controller.signal.aborted) {
      throw new OpenRouterRequestError(`Request timed out after ${input.timeoutMs}ms.`, "timeout", true);
    }
    throw new OpenRouterRequestError(error instanceof Error ? error.message : String(error), "transport", true);
  } finally {
    clearTimeout(timeout);
  }
}
