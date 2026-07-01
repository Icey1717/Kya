import { describe, expect, it } from "vitest";
import { OpenRouterAuthenticationError, probeModelAvailability, probeModelsAvailability } from "../src/modelAvailability.js";
import type { ModelMetadata } from "../src/types.js";

function model(id: string): ModelMetadata {
  return {
    id,
    name: id,
    contextLength: 8192,
    promptPrice: 0,
    completionPrice: 0,
    inputModalities: ["text"],
    outputModalities: ["text"],
    supportedParameters: [],
    raw: {}
  };
}

describe("model availability probes", () => {
  it("marks successful and failed completion probes", async () => {
    const fetchMock: typeof fetch = async (_url, init) => {
      const id = JSON.parse(String(init?.body)).model;
      return id === "working"
        ? new Response('{"choices":[{"message":{"content":"OK"}}]}')
        : new Response('{"error":{"message":"No endpoints found"}}', { status: 404 });
    };
    const results = await probeModelsAvailability([model("working"), model("missing")], "key", fetchMock, 2);
    expect(results.map(({ modelId, available }) => ({ modelId, available }))).toEqual([
      { modelId: "working", available: true },
      { modelId: "missing", available: false }
    ]);
    expect(results[1].note).toBe("No endpoints found");
  });

  it("surfaces authentication failures instead of marking every model unavailable", async () => {
    const fetchMock: typeof fetch = async () => new Response('{"error":{"message":"Invalid API key"}}', { status: 401 });
    await expect(probeModelAvailability(model("working"), "bad-key", fetchMock)).rejects.toBeInstanceOf(
      OpenRouterAuthenticationError
    );
  });

  it("does not apply the zero-price provider cap to paid probes", async () => {
    let body: Record<string, any> = {};
    const paid = { ...model("openai/gpt-4o-mini"), promptPrice: 0.000001, completionPrice: 0.000002 };
    const fetchMock: typeof fetch = async (_url, init) => {
      body = JSON.parse(String(init?.body));
      return new Response('{"choices":[{"message":{"content":"OK"}}]}');
    };
    await probeModelAvailability(paid, "key", fetchMock);
    expect(body.provider).toEqual({});
  });
});
