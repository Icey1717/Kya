import { describe, expect, it } from "vitest";
import type { Declaration } from "../src/types.js";
import { parseStrictJsonResponse, validateAndNormalizeResponse, validateChangedName, validateNameWarnings } from "../src/validation.js";

const declarations: Declaration[] = [
  {
    id: "local_001",
    oldName: "pCVar1",
    declaredType: "CActor *",
    startIndex: 0,
    endIndex: 6,
    scopePath: [0],
    isPointer: true,
    isBoolean: false
  },
  {
    id: "local_002",
    oldName: "bVar2",
    declaredType: "bool",
    startIndex: 10,
    endIndex: 15,
    scopePath: [0],
    isPointer: false,
    isBoolean: true
  }
];

function response(overrides: object[] = []) {
  return JSON.stringify({
    suggestions: declarations.map((declaration, index) => ({
      declaration_id: declaration.id,
      old_name: declaration.oldName,
      suggested_name: index === 0 ? "pTargetActor" : "bHasTarget",
      confidence: 0.8,
      ...(overrides[index] ?? {})
    }))
  });
}

describe("strict response validation", () => {
  it("accepts raw JSON and one fenced object only", () => {
    expect(parseStrictJsonResponse('{"suggestions":[]}')).toEqual({ suggestions: [] });
    expect(parseStrictJsonResponse('\`\`\`json\n{"suggestions":[]}\n\`\`\`')).toEqual({ suggestions: [] });
    expect(() => parseStrictJsonResponse('Here: {"suggestions":[]}')).toThrow();
    expect(() => parseStrictJsonResponse('\`\`\`json\n{}\n\`\`\`\n\`\`\`json\n{}\n\`\`\`')).toThrow();
  });

  it("accepts keyed declaration maps", () => {
    const raw = JSON.stringify({
      local_001: {
        old_name: declarations[0].oldName,
        suggested_name: "pTargetActor",
        confidence: 0.9
      },
      local_002: {
        old_name: declarations[1].oldName,
        suggested_name: declarations[1].oldName,
        confidence: 0.7
      }
    });
    const result = validateAndNormalizeResponse(raw, declarations, []);
    expect(result.suggestions.map((entry) => entry.declaration_id)).toEqual(["local_001", "local_002"]);
    expect(result.suggestions[1].decision).toBe("keep");
  });
  it("normalizes arbitrary entry order and unchanged names", () => {
    const parsed = JSON.parse(response());
    parsed.suggestions.reverse();
    parsed.suggestions[1].suggested_name = parsed.suggestions[1].old_name;
    const result = validateAndNormalizeResponse(JSON.stringify(parsed), declarations, []);
    expect(result.suggestions.map((entry) => entry.declaration_id)).toEqual(["local_001", "local_002"]);
    expect(result.suggestions[0].decision).toBe("keep");
  });

  it.each([
    ["missing", JSON.stringify({ suggestions: [JSON.parse(response()).suggestions[0]] })],
    ["duplicate", JSON.stringify({ suggestions: [JSON.parse(response()).suggestions[0], JSON.parse(response()).suggestions[0]] })],
    ["unknown", response([{ declaration_id: "local_999" }])],
    ["old name case", response([{ old_name: "pcvar1" }])],
    ["confidence", response([{ confidence: 2 }])],
    ["extra field", response([{ reason: "not allowed" }])]
  ])("rejects an invalid whole response: %s", (_name, raw) => {
    expect(() => validateAndNormalizeResponse(raw, declarations, [])).toThrow();
  });

  it("enforces naming and prefix rules", () => {
    expect(validateChangedName("pTargetActor", declarations[0])).toEqual([]);
    expect(validateChangedName("targetActor", declarations[0])).toContain(
      "pointer names must start with p followed by uppercase"
    );
    expect(validateChangedName("bHasTarget", declarations[1])).toEqual([]);
    expect(validateChangedName("hasTarget", declarations[1])).toContain(
      "Boolean names must start with b followed by uppercase"
    );
    expect(validateChangedName("pHTTPActor2", declarations[0])).toEqual([]);
    expect(validateNameWarnings("pHTTPActor2")).toEqual(expect.arrayContaining(["contains adjacent uppercase letters", "ends in a digit"]));
    expect(validateChangedName("pG3DManager", declarations[0])).toEqual([]);
    expect(validateNameWarnings("pG3DManager")).toContain("contains adjacent uppercase letters");
    expect(validateChangedName("class", { isPointer: false, isBoolean: false })).toContain("is a C++ keyword");
  });

  it("accepts warning-only names in model responses", () => {
    const result = validateAndNormalizeResponse(response([{ suggested_name: "pG3DManager" }]), declarations, []);
    expect(result.suggestions[0].warnings).toContain("contains adjacent uppercase letters");
  });
  it("rejects collisions and grandfathers only unchanged shadowing", () => {
    expect(() => validateAndNormalizeResponse(response([{ suggested_name: "pParameter" }]), declarations, ["pParameter"]))
      .toThrow(/collision/i);
    const shadowed = declarations.map((entry) => ({ ...entry, oldName: "value", isPointer: false, isBoolean: false }));
    const kept = JSON.stringify({
      suggestions: shadowed.map((entry) => ({
        declaration_id: entry.id,
        old_name: "value",
        suggested_name: "value",
        confidence: 0.5
      }))
    });
    expect(validateAndNormalizeResponse(kept, shadowed, []).suggestions).toHaveLength(2);
  });
});
