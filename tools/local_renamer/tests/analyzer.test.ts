import { describe, expect, it } from "vitest";
import { analyzeCppSource } from "../src/cppAnalyzer.js";

const source = [
  "/* single-line attached comment */",
  "// attached comment",
  "static int freeFunction(int input)",
  "{",
  "#if FEATURE",
  "    int iVar1 = input;",
  "    {",
  "        int iVar1 = 2;",
  "        bool bVar2 = iVar1 != 0;",
  "    }",
  "    return iVar1;",
  "#endif",
  "}",
  "",
  "int Thing::Member()",
  "{",
  "    Actor* pAVar1 = nullptr;",
  "    return pAVar1 != nullptr;",
  "}",
  "",
  "int noLocals() { return 1; }",
  "",
  "int ambiguous()",
  "{",
  "    auto callback = []() { int nested = 1; return nested; };",
  "    return callback();",
  "}",
  ""
].join("\n");

describe("C++ analysis", () => {
  it("enumerates free, static, and member functions in source order", () => {
    const functions = analyzeCppSource("fixture.cpp", source);
    expect(functions.map((fn) => fn.qualifiedName)).toEqual([
      "freeFunction",
      "Thing::Member",
      "noLocals",
      "ambiguous"
    ]);
    expect(functions[0].source).toContain("// attached comment");
    expect(functions[0].source.startsWith("/* single-line attached comment */")).toBe(true);
    expect(functions[0].source).toContain("#if FEATURE");
    expect(functions[0].declarations.map((declaration) => declaration.id)).toEqual([
      "local_001",
      "local_002"
    ]);
    expect(functions[0].parameters).toEqual(["input"]);
    expect(functions[2].exclusionReason).toBe("no_locals");
    expect(functions[3].exclusionReason).toBe("ambiguous_binding");
  });

  it("includes loop and condition declarations", () => {
    const [fn] = analyzeCppSource(
      "loops.cpp",
      "void loops() { for (int i = 0; i < 2; ++i) {} for (auto* pItem : items) { use(pItem); } if (int value = get()) { use(value); } }"
    );
    expect(fn.declarations.map((declaration) => declaration.oldName)).toEqual(["i", "pItem", "value"]);
  });

  it("preserves qualified, array, and function-pointer type shape", () => {
    const [fn] = analyzeCppSource(
      "types.cpp",
      "void types() { static const unsigned int values[3] = {}; int (*callback)(int) = nullptr; use(values, callback); }"
    );
    expect(fn.declarations.map((declaration) => declaration.declaredType)).toEqual([
      "static const unsigned int []",
      "int *"
    ]);
  });

  it("tolerates Tree-sitter errors for decompiler comma expressions in conditions", () => {
    const [fn] = analyzeCppSource(
      "comma.cpp",
      "void commaCondition() { int value; long result; if ((value != 0) || (result = call(), result == 0)) {} }"
    );
    expect(fn.parseError).toBe(false);
    expect(fn.exclusionReason).toBeUndefined();
    expect(fn.declarations.map((declaration) => declaration.oldName)).toEqual(["value", "result"]);
  });

  it("still rejects parser errors in declarations", () => {
    const [fn] = analyzeCppSource("broken.cpp", "void broken() { int value = ; }");
    expect(fn.parseError).toBe(true);
    expect(fn.exclusionReason).toBe("parse_failure");
  });

  it("removes definition-only locals from the inventory and prompt source", () => {
    const [fn] = analyzeCppSource(
      "unused.cpp",
      "void unused() { int unusedValue; int usedValue; usedValue = 1; }"
    );
    expect(fn.declarations.map((declaration) => [declaration.id, declaration.oldName])).toEqual([
      ["local_001", "usedValue"]
    ]);
    expect(fn.source).toContain("int unusedValue;");
    expect(fn.promptSource).not.toContain("int unusedValue;");
    expect(fn.promptSource).toContain("int usedValue;");
    expect(fn.declarations[0].occurrences).toHaveLength(2);
  });

  it("keeps mixed declaration syntax in the prompt while excluding its unused declarator", () => {
    const [fn] = analyzeCppSource(
      "mixed.cpp",
      "void mixed() { int unusedValue, usedValue; usedValue = 1; }"
    );
    expect(fn.declarations.map((declaration) => declaration.oldName)).toEqual(["usedValue"]);
    expect(fn.promptSource).toContain("int unusedValue, usedValue;");
  });
});
