import { describe, expect, it } from "vitest";
import { applyRateLimitStatus, filterFreeTextModels, filterSelectableTextModels, paidModelIds, validateModelSelection } from "../src/catalog.js";

function model(id: string, prompt: string, completion: string, input = ["text"], output = ["text"]) {
  return {
    id,
    name: id,
    context_length: 8192,
    pricing: { prompt, completion },
    architecture: { input_modalities: input, output_modalities: output },
    supported_parameters: ["temperature"]
  };
}

describe("OpenRouter catalog", () => {
  it("marks recently rate-limited models and rejects them for selection", () => {
    const catalog = filterFreeTextModels({ data: [model("free", "0", "0"), model("other", "0", "0")] });
    const rateLimited = applyRateLimitStatus(catalog, {
      free: {
        blockedUntil: new Date(Date.now() + 60_000).toISOString(),
        lastFailureAt: new Date().toISOString(),
        retryAfterMs: 60_000,
        failureCount: 1,
        lastError: "rate limited"
      }
    });
    expect(rateLimited.find((entry) => entry.id === "free")).toMatchObject({ rateLimitedUntil: expect.any(String) });
    expect(() => validateModelSelection(catalog, ["free", "other", "a", "b"], new Set(["free"]))).toThrow(
      /rate limited/i
    );
  });

  it("keeps only zero-priced text-to-text models", () => {
    const result = filterFreeTextModels({
      data: [
        model("free", "0", "0"),
        model("paid-prompt", "0.1", "0"),
        model("paid-completion", "0", "0.1"),
        model("image", "0", "0", ["image"], ["text"])
      ]
    });
    expect(result.map((entry) => entry.id)).toEqual(["free"]);
  });

  it("adds only the explicit paid allowlist to selectable models", () => {
    const result = filterSelectableTextModels({
      data: [
        model("free", "0", "0"),
        ...paidModelIds.map((id) => model(id, "0.000001", "0.000002")),
        model("paid-but-not-allowed", "0.000001", "0.000002")
      ]
    });
    expect(result.map((entry) => entry.id)).toEqual(["free", ...paidModelIds].sort());
  });

  it("requires two unique exact catalog IDs", () => {
    const catalog = filterFreeTextModels({ data: ["a", "b", "c", "d"].map((id) => model(id, "0", "0")) });
    expect(() => validateModelSelection(catalog, ["a"])).toThrow(/at least two/i);
    expect(() => validateModelSelection(catalog, ["a", "b", "c", "missing"])).toThrow(/not currently/i);
    expect(validateModelSelection(catalog, ["a", "b"])).toHaveLength(2);
  });
});
