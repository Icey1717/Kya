import { describe, expect, it } from "vitest";
import { requestSuggestions } from "../src/openRouter.js";
import type { FunctionRecord, ModelMetadata } from "../src/types.js";

const fn: FunctionRecord = {
  id: "function-1",
  qualifiedName: "test",
  relativePath: "src/test.cpp",
  startIndex: 0,
  endIndex: 40,
  startLine: 1,
  endLine: 1,
  source: "int test() { int iVar1 = 0; return iVar1; }",
  parameters: [],
  declarations: [{
    id: "local_001",
    oldName: "iVar1",
    declaredType: "int",
    startIndex: 17,
    endIndex: 22,
    scopePath: [0],
    isPointer: false,
    isBoolean: false
  }],
  parseError: false
};

function model(reasoning: unknown, id = "test/model:free"): ModelMetadata {
  return {
    id,
    name: "test",
    contextLength: 8192,
    promptPrice: 0,
    completionPrice: 0,
    inputModalities: ["text"],
    outputModalities: ["text"],
    supportedParameters: ["reasoning"],
    raw: { reasoning }
  };
}

async function requestBody(metadata: unknown, id?: string): Promise<Record<string, any>> {
  let body: Record<string, any> = {};
  const fetchMock: typeof fetch = async (_url, init) => {
    body = JSON.parse(String(init?.body));
    return new Response(JSON.stringify({ choices: [{ message: { content: "{}" } }] }));
  };
  await requestSuggestions({ apiKey: "key", model: model(metadata, id), fn, timeoutMs: 1000, fetchImpl: fetchMock });
  return body;
}

describe("OpenRouter requests", () => {
  it("returns the billed cost reported by OpenRouter", async () => {
    const fetchMock: typeof fetch = async () => new Response(JSON.stringify({
      choices: [{ message: { content: "{}" } }],
      usage: { prompt_tokens: 20, completion_tokens: 10, total_tokens: 30, cost: 0.00042 }
    }));
    const result = await requestSuggestions({
      apiKey: "key",
      model: model({ mandatory: false }),
      fn,
      timeoutMs: 1000,
      fetchImpl: fetchMock
    });
    expect(result.usage).toEqual({ prompt: 20, completion: 10, total: 30, cost: 0.00042 });
  });

  it("disables optional reasoning and states the complete strict response contract", async () => {
    const body = await requestBody({ mandatory: false });
    expect(body.reasoning).toEqual({ effort: "none", exclude: true });
    expect(body.messages[0].content).toContain('"suggestions"');
    expect(body.messages[0].content).toContain('"confidence":0.5');
    expect(body.messages[0].content).toContain("Do not return a keyed map or a top-level array");
  });

  it("uses the lowest supported effort when reasoning is mandatory", async () => {
    const body = await requestBody({ mandatory: true, supported_efforts: ["high", "medium", "low"] });
    expect(body.reasoning).toEqual({ effort: "low", exclude: true });
  });

  it("does not apply the zero-price provider cap to paid evaluations", async () => {
    const body = await requestBody({ mandatory: false }, "openai/gpt-4o-mini");
    expect(body.provider).toEqual({});
  });
});
