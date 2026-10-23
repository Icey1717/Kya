import Parser, { type SyntaxNode } from "tree-sitter";
import Cpp from "tree-sitter-cpp";
import { expandExpressionMacros, type AuditMacro } from "./auditBuildMacros.js";
import type { AuditStructureContext } from "./auditStructure.js";

const parser = new Parser();
parser.setLanguage(Cpp);
export interface ReviewedAuditFinding {
  name: string;
  address: string;
  sourceSha256: string;
  ps2Sha256: string;
  findings: { kind: string; detail: string }[];
  reason: string;
  dependencies: { path: string; sha256: string }[];
}
export interface AuditContext { constants: Map<string, bigint>; enumNames?: Set<string>; reviewed?: ReviewedAuditFinding[]; macros?: Map<string, AuditMacro>; structure?: AuditStructureContext; ps2SqrtMagnitude?: boolean }

function parse(source: string) { return parser.parse(offset => source.slice(offset, offset + 16_384)); }

function walk(root: SyntaxNode, visit: (node: SyntaxNode) => boolean | void) {
  const pending = [root];
  while (pending.length) {
    const node = pending.pop()!;
    if (visit(node) === false) continue;
    for (const child of [...node.namedChildren].reverse()) pending.push(child);
  }
}

function integer(node: SyntaxNode, constants: Map<string, bigint>): bigint | undefined {
  if (node.type === "identifier") return constants.get(node.text);
  if (node.type === "number_literal") {
    const text = node.text.replace(/[uUlL]+$/, "");
    if (!/^(?:0[xX][\da-fA-F]+|0[bB][01]+|\d+)$/.test(text)) return undefined;
    try { return BigInt(/^0[0-7]+$/.test(text) ? `0o${text.slice(1)}` : text); } catch { return undefined; }
  }
  if (node.type === "parenthesized_expression") return node.namedChildren.length === 1 ? integer(node.namedChildren[0], constants) : undefined;
  if (node.type === "unary_expression") {
    const arg = node.childForFieldName("argument");
    const value = arg ? integer(arg, constants) : undefined;
    const operator = node.childForFieldName("operator")?.text;
    if (value !== undefined && operator === "-") return -value;
    if (value !== undefined && operator === "+") return value;
  }
  if (node.type === "sizeof_expression") {
    // Fixed-width types used in PS2 data; do not guess pointer/class/long sizes.
    const type = /^sizeof\s*\(\s*(uint|int|float|byte|uchar|ushort|short|undefined4|undefined2|undefined1)\s*\)$/.exec(node.text)?.[1];
    if (type) return ["byte", "uchar", "undefined1"].includes(type) ? 1n : ["ushort", "short", "undefined2"].includes(type) ? 2n : 4n;
  }
  if (node.type === "binary_expression") {
    const left = node.childForFieldName("left"), right = node.childForFieldName("right");
    const a = left ? integer(left, constants) : undefined, b = right ? integer(right, constants) : undefined;
    if (a === undefined || b === undefined) return undefined;
    switch (node.childForFieldName("operator")?.text) {
      case "+": return a + b;
      case "-": return a - b;
      case "*": return a * b;
      case "<<": return a >= 0n && b >= 0n && b < 64n ? a << b : undefined;
      case "|": return a >= 0n && b >= 0n ? a | b : undefined;
      case "&": return a >= 0n && b >= 0n ? a & b : undefined;
    }
  }
  return undefined;
}

export function createAuditContext(sources: string[], reviewed: ReviewedAuditFinding[] = []): AuditContext {
  const definitions = new Map<string, Set<string>>();
  const enumNames = new Set<string>();
  const excludedEnums = new Set<string>();
  function define(name: string, value: string) {
    const values = definitions.get(name) ?? new Set<string>();
    values.add(value);
    definitions.set(name, values);
  }
  for (const source of sources) walk(parse(source).rootNode, node => {
    if (node.type === "enum_specifier") {
      const entries = node.childForFieldName("body")?.namedChildren.filter(child => child.type === "enumerator") ?? [];
      // Only globally visible unscoped enums. Do not guess name lookup across
      // classes/namespaces or merge scoped constants into a global dictionary.
      let global = !node.children.some(child => ["class", "struct"].includes(child.type));
      for (let parent = node.parent; parent; parent = parent.parent) {
        if (["class_specifier", "struct_specifier", "namespace_definition", "function_definition"].includes(parent.type)) global = false;
      }
      let previous: string | undefined;
      for (const entry of entries) {
        const name = entry.childForFieldName("name")?.text;
        if (!name) continue;
        if (!global) { excludedEnums.add(name); continue; }
        enumNames.add(name);
        define(name, entry.childForFieldName("value")?.text ?? (previous ? `(${previous}) + 1` : "0"));
        previous = name;
      }
      return false;
    }
    if (node.type !== "preproc_def") return;
    const name = node.childForFieldName("name")?.text;
    const value = node.childForFieldName("value")?.text.replace(/\/\*[\s\S]*?\*\/|\/\/[^\n]*/g, "").trim();
    if (!name || !value) return false;
    define(name, value);
    return false;
  });
  for (const name of excludedEnums) definitions.delete(name);
  const constants = new Map<string, bigint>();
  const expressions = new Map<string, SyntaxNode[]>();
  for (const [name, values] of definitions) {
    const nodes: SyntaxNode[] = [];
    for (const value of values) {
      const tree = parse(`void AuditValue() { result = (${value}); }`);
      if (tree.rootNode.hasError) continue;
      walk(tree.rootNode, node => {
        if (node.type === "assignment_expression") {
          const right = node.childForFieldName("right");
          if (right) nodes.push(right);
          return false;
        }
      });
    }
    if (nodes.length === values.size) expressions.set(name, nodes);
  }
  for (let pass = 0; pass <= expressions.size; pass++) {
    let changed = false;
    for (const [name, nodes] of expressions) {
      if (constants.has(name)) continue;
      const values = nodes.map(node => integer(node, constants));
      if (values[0] !== undefined && values.every(value => value === values[0])) {
        constants.set(name, values[0]);
        changed = true;
      }
    }
    if (!changed) break;
  }
  return { constants, enumNames: new Set([...enumNames].filter(name => constants.has(name))), reviewed };
}

export function auditCallName(node: SyntaxNode): string | undefined {
  if (["identifier", "qualified_identifier", "field_identifier"].includes(node.type)) return node.text.split("::").at(-1);
  if (node.type === "field_expression") return node.childForFieldName("field")?.text;
  if (["template_function", "template_method"].includes(node.type)) {
    const name = node.childForFieldName("name");
    return name?.text ?? node.namedChildren[0]?.text;
  }
  // Ghidra's (*((this->base).pVTable)->SetState)(...) still names SetState.
  if (["parenthesized_expression", "pointer_expression"].includes(node.type) && node.namedChildren.length === 1) return auditCallName(node.namedChildren[0]);
  return undefined;
}

function floatLiteral(bits: string): string | undefined {
  const data = new DataView(new ArrayBuffer(4));
  data.setUint32(0, Number.parseInt(bits, 16));
  const value = data.getFloat32(0);
  if (!Number.isFinite(value)) return undefined;
  if (Object.is(value, -0)) return "-0.0f";
  // Shortest decimal which round-trips to the exact same float32 bits.
  for (let digits = 1; digits <= 9; digits++) {
    const rounded = Number(value.toPrecision(digits));
    if (Object.is(Math.fround(rounded), value)) {
      const decimal = String(rounded);
      return `${/[.eE]/.test(decimal) ? decimal : `${decimal}.0`}f`;
    }
  }
  return undefined;
}

export function normalizeAuditSource(source: string, context: AuditContext) {
  const notes = new Set<string>();
  if (context.macros) {
    const expanded = expandExpressionMacros(source, context.macros);
    source = expanded.source;
    for (const note of expanded.notes) notes.add(note);
  }
  const shadowedEnums = new Set<string>();
  walk(parse(source).rootNode, node => {
    if (!["declaration", "parameter_declaration", "optional_parameter_declaration"].includes(node.type)) return;
    for (const declarator of node.childrenForFieldName("declarator")) walk(declarator, child => {
      if (child.type === "init_declarator") {
        const binding = child.childForFieldName("declarator");
        if (binding) walk(binding, name => { if (name.type === "identifier") shadowedEnums.add(name.text); });
        return false;
      }
      if (child.type === "identifier") shadowedEnums.add(child.text);
    });
  });
  function transform(visit: (node: SyntaxNode) => string | undefined) {
    const edits: { start: number; end: number; text: string }[] = [];
    walk(parse(source).rootNode, node => {
      const text = visit(node);
      if (text === undefined || text === node.text) return;
      edits.push({ start: node.startIndex, end: node.endIndex, text });
      return false;
    });
    for (const edit of edits.sort((a, b) => b.start - a.start)) source = source.slice(0, edit.start) + edit.text + source.slice(edit.end);
  }
  if (context.ps2SqrtMagnitude) transform(node => {
    if (node.type !== "call_expression" || node.childForFieldName("function")?.text !== "SQRT") return;
    const args = node.childForFieldName("arguments")?.namedChildren;
    if (args?.length !== 1) return;
    // EE SQRT.S takes the magnitude for negative finite operands. Model that
    // operation explicitly when comparing the build's matching magnitude shim;
    // do not remove fabs globally or rewrite original libc sqrt calls.
    notes.add("Modeled PS2 SQRT magnitude operation against the active sqrtf shim (FPU flags/edge cases remain outside lexical triage).");
    return `sqrtf(fabsf(${args[0].text}))`;
  });
  transform(node => {
    // Ghidra prints floating-point p-code operations as ABS/SQRT. Compare
    // operation counts rather than compiler-specific names; keep arguments,
    // nested calls and constants intact. Do not alias integer abs or double sqrt.
    if (!["identifier", "qualified_identifier"].includes(node.type)) return;
    const call = node.parent;
    if (call?.type !== "call_expression" || call.childForFieldName("function")?.id !== node.id ||
        call.childForFieldName("arguments")?.namedChildren.length !== 1) return;
    const name = node.text.replace(/^std::/, "");
    const canonical = name === "ABS" || name === "fabs" || name === "fabsf" ? "fabsf"
      : name === "SQRT" || name === "sqrtf" ? "sqrtf" : undefined;
    if (!canonical || canonical === node.text) return;
    notes.add(`Canonicalized floating-point operation ${node.text} as ${canonical}.`);
    return canonical;
  });
  transform(node => {
    if (node.type === "number_literal") {
      const hex = /^0x([\da-f]+)(?:\.([\da-f]*))?p([+-]?\d+)(f?)$/i.exec(node.text);
      if (hex) {
        const fraction = [...(hex[2] ?? "")].reduce((sum, digit, index) => sum + parseInt(digit, 16) / 16 ** (index + 1), 0);
        const value = (parseInt(hex[1], 16) + fraction) * 2 ** Number(hex[3]);
        if (!Number.isFinite(value)) return;
        if (hex[4]) {
          const data = new DataView(new ArrayBuffer(4));
          data.setFloat32(0, value);
          const literal = floatLiteral(data.getUint32(0).toString(16).padStart(8, "0"));
          if (literal) { notes.add("Decoded hexadecimal float literal without dropping its value."); return literal; }
        } else return String(value);
      }
    }
    if (node.type === "cast_expression") {
      const bits = /^\(\s*float\s*\)\s*&DAT_([\da-fA-F]{8})$/.exec(node.text)?.[1];
      const literal = bits ? floatLiteral(bits) : undefined;
      if (literal) { notes.add(`Decoded float bits DAT_${bits} as ${literal}.`); return literal; }
    }
    if (node.type !== "call_expression") return;
    const target = node.childForFieldName("function");
    if (!target || auditCallName(target) !== "edMemAlloc") return;
    const args = node.childForFieldName("arguments")?.namedChildren;
    if (!args || args.length !== 2) return;
    let size = args[1];
    while (size.type === "parenthesized_expression" && size.namedChildren.length === 1) size = size.namedChildren[0];
    if (size.type !== "binary_expression") return;
    const left = size.childForFieldName("left"), right = size.childForFieldName("right");
    if (!left || !right) return;
    const operator = size.childForFieldName("operator")?.text;
    let factor = integer(right, context.constants);
    if (operator === "<<" && factor !== undefined && factor >= 0n && factor < 32n) factor = 1n << factor;
    else if (operator !== "*") return;
    if (factor === undefined || factor <= 0n) return;
    notes.add("Canonicalized edMemAlloc item-size arithmetic using fixed-width PS2 types.");
    return `${target.text}(${args[0].text}, (${left.text}) * ${factor})`;
  });
  transform(node => {
    if (node.type !== "identifier") return;
    const value = context.constants.get(node.text);
    if (value === undefined) return;
    if (context.enumNames?.has(node.text) && shadowedEnums.has(node.text)) return;
    // Only expression uses; never change declarations, names, types or fields.
    if (!node.parent || node.parent.type.endsWith("declarator") || ["function_definition", "parameter_declaration", "qualified_identifier", "enumerator"].includes(node.parent.type)) return;
    if (node.parent.type === "call_expression" && node.parent.childForFieldName("function")?.id === node.id) return;
    notes.add(`Resolved numeric ${context.enumNames?.has(node.text) ? "enum" : "macro"} ${node.text} = ${value}.`);
    return value < 0n ? `(${value})` : String(value);
  });
  transform(node => {
    if (node.type !== "binary_expression" || node.childForFieldName("operator")?.text !== "-") return;
    const left = node.childForFieldName("left"), right = node.childForFieldName("right");
    if (!left || !right) return;
    // Only an integer literal (possibly parenthesized/negated), never a
    // variable, float, cast, or an expression with side effects.
    let literal = right;
    while (["parenthesized_expression", "unary_expression"].includes(literal.type) && literal.namedChildren.length === 1) literal = literal.namedChildren[0];
    if (literal.type !== "number_literal") return;
    const value = integer(right, new Map());
    if (value === undefined || value === 0n) return;
    notes.add("Canonicalized subtraction of an integer literal as addition of its negative.");
    return `${left.text} + (${String(-value)})`;
  });
  return { source, notes: [...notes] };
}
