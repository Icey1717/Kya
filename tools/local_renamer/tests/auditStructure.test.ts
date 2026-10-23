import { describe, expect, it } from "vitest";
import { compareFunction, extractFunctions } from "../src/functionAudit.js";
import { createAuditContext } from "../src/auditNormalization.js";
import { createAuditStructure, expandTransparentHelpers } from "../src/auditStructure.js";
import { buildArguments, expandExpressionMacros, macroStates, parseExpressionMacro } from "../src/auditBuildMacros.js";

function compare(source: string, name: string, original: string) {
  const functions = extractFunctions("a.cpp", source);
  const context = createAuditContext([source]);
  context.structure = createAuditStructure([source], functions);
  const fn = functions.find(fn => fn.name === name)!;
  return compareFunction(fn, { function: { name, namespace: "Global", entry: "1" }, c: original }, functions, context);
}

describe("audit constructors and transparent helpers", () => {
  it("models the actual default base constructor without hiding duplicate calls", () => {
    const source = "class Base {}; class Derived : public Base { public: Derived(); }; Derived::Derived() { x = 0; }";
    const original = "void Derived::Derived(Derived* this) { Base::Base((Base*)this); x = 0; }";
    const result = compare(source, "Derived::Derived", original);
    expect(result.findings).toEqual([]);
    expect(result.normalizations.some(note => note.includes("implicit default base"))).toBe(true);
    expect(compare(source, "Derived::Derived", original.replace("x = 0;", "Base::Base((Base*)this); x = 0;")).findings.some(f => f.kind === "call_count")).toBe(true);
    expect(compare(source.replace("public Base", "public Other"), "Derived::Derived", original).findings.some(f => f.kind === "call_count")).toBe(true);
    expect(compare(source, "Derived::Derived", original.replace("(Base*)this", "(Base*)other")).findings.some(f => f.kind === "call_count")).toBe(true);
  });

  it("preserves explicit constructor and member initializer values", () => {
    const source = "class Base {}; class Derived : public Base { int value; public: Derived(); }; Derived::Derived() : Base(2), value(7) {}";
    const original = "void Derived::Derived(Derived* this) { Base::Base((Base*)this, 2); value = 7; }";
    expect(compare(source, "Derived::Derived", original).findings).toEqual([]);
    for (const wrong of [source.replace("Base(2)", "Base(3)"), source.replace("value(7)", "value(8)")]) {
      expect(compare(wrong, "Derived::Derived", original).findings.some(f => f.kind === "constant_count")).toBe(true);
    }
    expect(compare(source, "Derived::Derived", original.replace("Base::Base((Base*)this, 2);", "")).findings.some(f => f.kind === "call_count")).toBe(true);
    const reordered = "class D { int a; int b; D(); }; D::D() : b(GetB()), a(GetA()) {}";
    expect(compare(reordered, "D::D", "void D::D() { a = GetA(); b = GetB(); }").findings).toEqual([]);
    expect(compare(reordered, "D::D", "void D::D() { b = GetB(); a = GetA(); }").findings.some(f => f.kind === "call_order")).toBe(true);
  });

  it("does not guess multiple, virtual or conflicting inheritance", () => {
    for (const inheritance of ["public Base, public Other", "virtual public Base"]) {
      const result = compare(`class Base {}; class Other {}; class D : ${inheritance} { D(); }; D::D() {}`, "D::D", "void D::D() { Base::Base(this); }");
      expect(result.normalizations.some(note => note.includes("base construction"))).toBe(false);
    }
    const source = "class Base {}; class D : public Base {}; class D : public Base {}; D::D() {}";
    expect(compare(source, "D::D", "void D::D() { Base::Base(this); }").findings.length).toBeGreaterThan(0);
  });

  it("expands qualified inherited forwarding with the correct owner", () => {
    const source = `class Owner { public: void Stop(); }; class Base { public: Owner* pOwner; virtual void End(int next); };
      class D : public Base { void End(int next); };
      void Base::End(int next) { pOwner->Stop(); return; }
      void D::End(int next) { Base::End(next); Send(20, 1); }`;
    const original = "void D::End() { owner->Stop(); Send(20, 1); }";
    const result = compare(source, "D::End", original);
    expect(result.findings).toEqual([]);
    expect(result.expanded.source).toContain("->pOwner->Stop");
    expect(compare(source.replace("Send(20", "Send(21"), "D::End", original).findings.some(f => f.kind === "constant_count")).toBe(true);
  });

  it("expands return accessors only when the receiver type is known", () => {
    const source = `template<class T> struct Ref { int pObj; T* Get() { return (T*)LOAD_POINTER(this->pObj); } };
      class D { Ref<Actor> actorRef; void F(); }; void D::F() { Send(this->actorRef.Get(), 20); }`;
    const result = compare(source, "D::F", "void D::F() { Send(actorRef.pObj, 20); }");
    expect(result.findings).toEqual([]);
    expect(result.normalizations.some(note => note.includes("Ref::Get"))).toBe(true);
    const unknown = source.replace("this->actorRef.Get()", "other.Get()");
    expect(compare(unknown, "D::F", "void D::F() { Send(actorRef.pObj, 20); }").findings.some(f => f.detail.includes("Get"))).toBe(true);
    const shadowed = source.replace("void D::F() {", "void D::F() { Other actorRef;").replace("this->actorRef.Get()", "actorRef.Get()");
    expect(compare(shadowed, "D::F", "void D::F() { Send(actorRef.pObj, 20); }").findings.some(f => f.detail.includes("Get"))).toBe(true);
  });

  it("preserves unsafe arguments, virtual dispatch, overloads and existing original calls", () => {
    const source = `class D { public: virtual int Get(); int Forward(int x); void F(); };
      int D::Get() { return value; } int D::Forward(int x) { return Apply(x); }
      void D::F() { Use(Get()); Forward(Next()); }`;
    const functions = extractFunctions("a.cpp", source);
    const context = createAuditStructure([source], functions);
    const fn = functions.find(fn => fn.name === "D::F")!;
    expect(expandTransparentHelpers(fn.source, fn.name, context).notes).toEqual([]);
    expect(compare(source, "D::F", "void D::F() { Use(Get()); Forward(Next()); }").findings).toEqual([]);
    const overload = source + "int D::Forward(float x) { return Other(x); }";
    const result = compare(overload.replace("Forward(Next())", "Forward(2)"), "D::F", "void D::F() { Use(Get()); Apply(2); }");
    expect(result.findings.some(f => f.detail.includes("Forward"))).toBe(true);
    const inherited = "class B { virtual int Get(); }; class D : public B { int Get(); void F(); }; int D::Get() { return value; } void D::F() { Use(Get()); }";
    const inheritedResult = compare(inherited, "D::F", "void D::F() { Use(value); }");
    expect(inheritedResult.findings.some(f => f.detail.includes("Get"))).toBe(true);
    const declarations = "class D { int Forward(int x); int Forward(float x); void F(); }; int D::Forward(int x) { return Apply(x); } void D::F() { Use(Forward(2)); }";
    expect(compare(declarations, "D::F", "void D::F() { Use(Apply(2)); }").findings.some(f => f.detail.includes("Forward"))).toBe(true);
  });
});

describe("build-active expression macros", () => {
  it("uses compiler macro state at the function location, including undef and redefinition", () => {
    const output = '# 1 "C:/repo/a.cpp"\n#define Scale(x) ((x) * 2)\nvoid F() {}\n#undef Scale\nvoid G() {}\n#define Scale(x) ((x) * 3)\nvoid H() {}';
    const states = macroStates(output, "C:/repo/a.cpp", [2, 4, 6]);
    expect(states.get(2)?.get("Scale")?.body).toBe("((x) * 2)");
    expect(states.get(4)?.has("Scale")).toBe(false);
    expect(states.get(6)?.get("Scale")?.body).toBe("((x) * 3)");
  });

  it("preserves macro token substitution, changed constants and nested operations", () => {
    const macro = parseExpressionMacro("#define Scale(x) ((x) * 2)")!;
    const macros = new Map([[macro.name, macro.macro]]);
    const source = "void F() { Draw(Scale(value)); }";
    const functions = extractFunctions("a.cpp", source);
    const context = { ...createAuditContext([]), macros };
    const original = { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { Draw(value * 2); }" };
    expect(compareFunction(functions[0], original, functions, context).findings).toEqual([]);
    const changed = new Map([["Scale", { parameters: ["x"], body: "((x) * 3)" }]]);
    expect(compareFunction(functions[0], original, functions, { ...context, macros: changed }).findings.some(f => f.kind === "constant_count")).toBe(true);
    const raw = parseExpressionMacro("#define Raw(x) (x * 2)")!;
    expect(expandExpressionMacros("void F() { Raw(a + b); }", new Map([["Raw", raw.macro]])).source).toContain("(a + b * 2)");
    expect(expandExpressionMacros("void F() { Scale(Next()); }", macros).notes).toEqual([]);
    const stateChanges = "void F() {\n#undef Scale\n#define Scale(x) ((x) * 7)\nScale(value);\n}";
    expect(expandExpressionMacros(stateChanges, macros).source).toBe(stateChanges);
    const recursive = new Map([["A", { parameters: ["x"], body: "B(x) + 2" }], ["B", { parameters: ["x"], body: "A(x) + 3" }]]);
    expect(expandExpressionMacros("void F() { A(value); }", recursive).notes).toEqual([]);
    const hex = new Map([["Scale", { parameters: ["x"], body: "((x) * 0x1p+1f)" }]]);
    expect(compareFunction(functions[0], original, functions, { ...context, macros: hex }).findings).toEqual([]);
    const wrongHex = new Map([["Scale", { parameters: ["x"], body: "((x) * 0x1.8p+1f)" }]]);
    expect(compareFunction(functions[0], original, functions, { ...context, macros: wrongHex }).findings.some(f => f.kind === "constant_count")).toBe(true);
  });

  it("does not apply source build macros to original libc calls or accept statement macros", () => {
    for (const line of ["#define X(x) do { Apply(x); } while(0)", "#define X(x) #x", "#define X(...) Apply(__VA_ARGS__)"]) expect(parseExpressionMacro(line)).toBeUndefined();
    const source = "void F() { sinf(angle); }", functions = extractFunctions("a.cpp", source);
    const context = { ...createAuditContext([]), macros: new Map([["sinf", { parameters: ["x"], body: "Table[(int)(x * 2)]" }]]) };
    const result = compareFunction(functions[0], { function: { name: "F", namespace: "Global", entry: "1" }, c: source }, functions, context);
    expect(result.findings.some(f => f.detail.includes("sinf: port 0, PS2 1"))).toBe(true);
    expect(result.normalized.ps2).toBe(source);
    expect(buildArguments('-DVALUE=1 -I"C:/SDK folder/include"')).toEqual(["-DVALUE=1", "-IC:/SDK folder/include"]);
    expect(() => buildArguments("$UNRESOLVED")).toThrow();
  });

  it("models the PS2 magnitude sqrt only for the exact active shim and retains wrong arguments", () => {
    const source = "void F() { x = sqrtf(value + 2.0f); }", functions = extractFunctions("a.cpp", source);
    const context = { ...createAuditContext([]), macros: new Map([["sqrtf", { parameters: ["x"], body: "(std::sqrtf(std::fabs(x)))" }]]) };
    const original = { function: { name: "F", namespace: "Global", entry: "1" }, c: "void F() { x = SQRT(value + 2.0f); }" };
    expect(compareFunction(functions[0], original, functions, context).findings).toEqual([]);
    expect(compareFunction({ ...functions[0], source: source.replace("2.0f", "3.0f") }, original, functions, context).findings.some(f => f.kind === "constant_count")).toBe(true);
    const wrong = { ...context, macros: new Map([["sqrtf", { parameters: ["x"], body: "(std::sqrtf(std::fabs(x) + 4.0f))" }]]) };
    expect(compareFunction(functions[0], original, functions, wrong).findings.length).toBeGreaterThan(0);
    expect(compareFunction(functions[0], { ...original, c: original.c.replace("SQRT", "sqrtf") }, functions, context).findings.some(f => f.detail.includes("fabsf"))).toBe(true);
  });
});
