import { describe, expect, it } from "vitest";
import { auditSourceHash, compareFunction, expandHelpers, extractFunctions, fingerprint, matchFunctions } from "../src/functionAudit.js";
import { createAuditContext, normalizeAuditSource } from "../src/auditNormalization.js";

describe("function audit", () => {
  it("inventories member, free, inline and no-local functions without renamer exclusions", () => {
    const fns = extractFunctions("a.cpp", "namespace N { class A { void Inline() {} }; }\nvoid A::F() {}\nint Free() { return 2; }");
    expect(fns.map(fn => fn.name)).toEqual(["N::A::Inline", "A::F", "Free"]);
    expect(fns.every(fn => !fn.parseError)).toBe(true);
  });

  it("does not silently select an overload or a different class", () => {
    const fn = extractFunctions("a.cpp", "void A::F() {}")[0];
    const originals = [
      { name: "F", namespace: "A", entry: "00100000" },
      { name: "F", namespace: "B", entry: "00100004" },
      { name: "A::F", namespace: "Global", entry: "00100008" }
    ];
    expect(matchFunctions(fn, originals).map(f => f.entry)).toEqual(["00100000", "00100008"]);
    expect(matchFunctions(fn, [originals[1]])).toEqual([]);
  });

  it("matches address-named functions after Ghidra renaming", () => {
    const fn = extractFunctions("a.cpp", "void A::FUN_00123456() {}")[0];
    expect(matchFunctions(fn, [{ name: "Recovered", namespace: "A", entry: "00123456" }])).toHaveLength(1);
  });

  it("normalizes integer spelling without losing 64-bit precision", () => {
    const a = fingerprint("void F() { x = 0x20U; y = 0xffffffffffffffffULL; }");
    expect(a.constants.get("32")).toBe(1);
    expect(a.constants.get("18446744073709551615")).toBe(1);
    expect(fingerprint("void F() { x = 32; }").constants.get("32")).toBe(1);
    expect(fingerprint("void F() { x = -32; }").constants.get("-32")).toBe(1);
  });

  it("detects the Wolfen reset contamination only visible after helper expansion", () => {
    const source = `void Wolfen::Reset() { action = 0; action = action | 8; return; }
      void Wolfen::Init() { Reset(); action = 8; FlushInput(); Reset(); move = 0x26; FlushInput(); }`;
    const fns = extractFunctions("wolfen.cpp", source);
    const original = { function: { name: "Init", namespace: "Wolfen", entry: "001f87c0" },
      c: "void Wolfen::Init(Wolfen* this) { action = 0; action = 8; FlushInput(this); action = 0; move = 38; FlushInput(this); }" };
    const comparison = compareFunction(fns[1], original, fns);
    expect(comparison.compared).toBe(true);
    expect(comparison.findings).toContainEqual({ kind: "constant_count", detail: "Literal 8: port 3, PS2 1." });
    const fixed = extractFunctions("wolfen.cpp", source.replace("action = action | 8;", ""));
    expect(compareFunction(fixed[1], original, fixed).findings).toEqual([]);
  });

  it("does not inline another object's method or helpers with local bindings", () => {
    const fns = extractFunctions("a.cpp", `void A::Reset() { int x = 8; value = x; }
      void A::F() { other->Reset(); Reset(); }`);
    expect(expandHelpers(fns[1], fns).helpers).toEqual([]);
  });

  it("does not inline overloaded, conditional or non-void helpers", () => {
    const fns = extractFunctions("a.cpp", `void A::R() { value = 8; }
      void A::R(int x) { value = x; }
      void A::S() { if (value) value = 9; }
      int A::T() { return 8; }
      void A::F() { R(); S(); T(); }`);
    expect(expandHelpers(fns[4], fns).helpers).toEqual([]);
  });

  it("excludes comments, strings and member offsets from literal counts", () => {
    const result = fingerprint('void F() { /* value=8 */ Log("8"); this->field_0x8 = 0; }');
    expect(result.constants.has("8")).toBe(false);
  });

  it("reports direct call ordering and incomplete parsing", () => {
    const fn = extractFunctions("a.cpp", "void F() { A(); B(); }")[0];
    expect(compareFunction(fn, { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { B(); A(); }" }, [fn]).findings.map(f => f.kind)).toContain("call_order");
    expect(compareFunction(fn, { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { broken = ; }" }, [fn]).compared).toBe(false);
  });

  it("excludes assertion and instrumentation arguments without hiding runtime literals", () => {
    const fn = extractFunctions("a.cpp", 'void F() { assert(state < 5); ACTOR_LOG(2, "5"); value = 8; }')[0];
    const comparison = compareFunction(fn, { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { value = 8; }" }, [fn]);
    expect(comparison.findings).toEqual([]);
    expect(comparison.normalizations.some(note => note.includes("assert"))).toBe(true);
    const wrong = { ...fn, source: fn.source.replace("value = 8", "value = 9") };
    expect(compareFunction(wrong, { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { value = 8; }" }, [wrong]).findings.some(f => f.kind === "constant_count")).toBe(true);
  });

  it("resolves numeric macros and named virtual calls from the hold example", () => {
    const context = createAuditContext(["#define FIGHTER_HOLD_RUN 0x2a\n#define FIGHTER_HOLD_PUSH_PREPARE 0x34\n"]);
    const fn = extractFunctions("a.cpp", "void F() { SetState(FIGHTER_HOLD_RUN, -1); SetState(FIGHTER_HOLD_PUSH_PREPARE, -1); }")[0];
    const c = "void F() { (*((this->base).pVTable)->SetState)(this, 0x2a, -1); (*((this->base).pVTable)->SetState)(this, 0x34, -1); }";
    expect(compareFunction(fn, { function: { name: "F", namespace: "Global", entry: "1" }, c }, [fn], context).findings).toEqual([]);
  });

  it("does not guess conflicting conditional macros or width-dependent complements", () => {
    const context = createAuditContext(["#define A 8\n#define B (A | 2)\n#define C ~0U\n#define D 8\n", "#define D 9\n"]);
    expect(context.constants.get("B")).toBe(10n);
    expect(context.constants.has("C")).toBe(false);
    expect(context.constants.has("D")).toBe(false);
  });

  it("decodes Ghidra's exact float-bit cast without treating data loads as literals", () => {
    const context = createAuditContext([]);
    const source = "void F() { Draw((float)&DAT_3f4ccccd); }";
    expect(normalizeAuditSource(source, context).source).toContain("Draw(0.8f)");
    expect(normalizeAuditSource("void F() { Draw((float)&DAT_3f800000); }", context).source).toContain("Draw(1.0f)");
    expect(normalizeAuditSource("void F() { Draw((float)&DAT_80000000); }", context).source).toContain("Draw(-0.0f)");
    expect(normalizeAuditSource("void F() { Draw(*(float*)&DAT_3f4ccccd); }", context).source).not.toContain("Draw(0.8f)");
    const fn = extractFunctions("a.cpp", "void F() { Draw(0.8f); }")[0];
    expect(compareFunction(fn, { function: { name: "F", namespace: "Global", entry: "1" }, c: source }, [fn]).findings).toEqual([]);
  });

  it("canonicalizes allocation size spelling but still catches the wrong item size", () => {
    const fn = extractFunctions("a.cpp", "void F() { edMemAlloc(TO_HEAP(H_MAIN), count * (sizeof(uint) + sizeof(float))); }")[0];
    const original = { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { edMemAlloc(H_MAIN, count << 3); }" };
    expect(compareFunction(fn, original, [fn]).findings).toEqual([]);
    const wrong = { ...fn, source: fn.source.replace("sizeof(float)", "sizeof(short)") };
    expect(compareFunction(wrong, original, [wrong]).findings.some(f => f.kind === "constant_count")).toBe(true);
  });

  it("retains unresolved indirect calls such as the missing money reset", () => {
    const fn = extractFunctions("a.cpp", "void F() { return; }")[0];
    const original = { function: { name: "F", namespace: "Global", entry: "003615d0" }, c: "void F(int* param_1) { (**(code **)(*param_1 + 0x54))(); }" };
    const findings = compareFunction(fn, original, [fn]).findings;
    expect(findings.some(f => f.kind === "indirect_call_count")).toBe(true);
    expect(findings.some(f => f.detail.includes("84"))).toBe(true);
  });

  it("does not mistake named casts or typedef C casts for indirect calls", () => {
    const result = fingerprint("void F() { x = static_cast<ulong>(mask); p = reinterpret_cast<uint*>(data); Draw((uint)(state == 0x7a)); }");
    expect(result.indirectCalls).toBe(0);
    expect(result.calls).toEqual(["Draw"]);
    expect(result.constants.get("122")).toBe(1);
  });

  it("compares nested argument calls with Ghidra's explicit temporaries", () => {
    const fn = extractFunctions("a.cpp", "void F() { Apply(GetTimer()->delta, x); }")[0];
    const original = { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { timer = GetTimer(); Apply(timer->delta, x); }" };
    expect(compareFunction(fn, original, [fn]).findings).toEqual([]);
  });

  it("normalizes subtraction spelling while retaining a wrong sign or offset", () => {
    const fn = extractFunctions("a.cpp", "void F() { value = table + state - 5; }")[0];
    const original = { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { value = table + state + -5; }" };
    expect(compareFunction(fn, original, [fn]).findings).toEqual([]);
    for (const expression of ["+ 5", "- 6"]) {
      const wrong = { ...fn, source: fn.source.replace("- 5", expression) };
      expect(compareFunction(wrong, original, [wrong]).findings.some(f => f.kind === "constant_count")).toBe(true);
    }
  });

  it("applies reviewed stride exceptions only to the exact source and original", () => {
    const fn = extractFunctions("a.cpp", "void Commander::F() { Check(entries[i]); }")[0];
    const original = { function: { name: "F", namespace: "Commander", entry: "001710c0" }, c: "void Commander::F() { Check(entries); offset = offset + 24; }" };
    const context = createAuditContext([], [{
      name: fn.name, address: original.function.entry,
      sourceSha256: auditSourceHash(fn.source), ps2Sha256: auditSourceHash(original.c),
      findings: [{ kind: "constant_count", detail: "Literal 24: port 0, PS2 1." }],
      reason: "Verified original element size and native typed indexing.", dependencies: []
    }]);
    const result = compareFunction(fn, original, [fn], context);
    expect(result.findings).toEqual([]);
    expect(result.reviewedFindings).toHaveLength(1);
    const changed = { ...fn, source: fn.source.replace("entries[i]", "entries[i + 2]") };
    expect(compareFunction(changed, original, [changed], context).reviewedFindings).toEqual([]);
    expect(compareFunction(fn, { ...original, c: original.c.replace("24", "32") }, [fn], context).reviewedFindings).toEqual([]);
    expect(compareFunction({ ...fn, name: "Other::F" }, original, [fn], context).reviewedFindings).toEqual([]);
  });
});
