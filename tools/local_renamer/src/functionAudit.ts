import { createHash } from "node:crypto";
import Parser, { type SyntaxNode } from "tree-sitter";
import Cpp from "tree-sitter-cpp";
import { auditCallName, normalizeAuditSource, type AuditContext } from "./auditNormalization.js";

const parser = new Parser();
parser.setLanguage(Cpp);

export interface AuditFunction {
  id: string;
  name: string;
  path: string;
  line: number;
  source: string;
  body: string;
  noArguments: boolean;
  parseError: boolean;
}
export interface Ps2Function {
  entry: string;
  name: string;
  namespace: string;
  signature?: string;
}
export interface SnapshotEntry {
  function: Ps2Function;
  c?: string;
  error?: string;
}
export interface Finding { kind: string; detail: string }

export function auditSourceHash(source: string): string {
  return createHash("sha256").update(source.replace(/\r\n/g, "\n")).digest("hex");
}

function walk(node: SyntaxNode, visit: (node: SyntaxNode) => boolean | void): void {
  const pending = [node];
  while (pending.length) {
    const current = pending.pop()!;
    if (visit(current) === false) continue;
    const children = current.namedChildren;
    for (let i = children.length - 1; i >= 0; i--) pending.push(children[i]);
  }
}

function parse(source: string) {
  // Ghidra calling-convention annotations are not C++ grammar constructs.
  const input = source.replace(/\b(?:__thiscall|__stdcall|__cdecl|__fastcall)\b/g, s => " ".repeat(s.length));
  return parser.parse(offset => input.slice(offset, offset + 16_384));
}

export function extractFunctions(path: string, source: string): AuditFunction[] {
  const functions: AuditFunction[] = [];
  walk(parse(source).rootNode, node => {
    if (node.type !== "function_definition") return;
    let declarator = node.childForFieldName("declarator");
    while (declarator && declarator.type !== "function_declarator") {
      declarator = declarator.childForFieldName("declarator");
    }
    const nameNode = declarator?.childForFieldName("declarator");
    const body = node.childForFieldName("body");
    if (!nameNode || !body) return false;
    let name = nameNode.text.replace(/\s+/g, "");
    const scopes: string[] = [];
    if (!name.includes("::")) {
      for (let parent = node.parent; parent; parent = parent.parent) {
        if (["class_specifier", "struct_specifier", "namespace_definition"].includes(parent.type)) {
          const owner = parent.childForFieldName("name");
          if (owner) scopes.unshift(owner.text);
        }
      }
      name = [...scopes, name].join("::");
    }
    const parameters = declarator?.childForFieldName("parameters");
    const text = source.slice(node.startIndex, node.endIndex);
    functions.push({
      id: createHash("sha256").update(`${path}\0${node.startIndex}\0${text}`).digest("hex").slice(0, 20),
      name, path, line: node.startPosition.row + 1, source: text,
      body: source.slice(body.startIndex, body.endIndex),
      noArguments: /^\(\s*(?:void\s*)?\)$/.test(parameters?.text ?? "?"),
      parseError: node.hasError
    });
    return false;
  });
  return functions;
}

export function qualifiedName(fn: Ps2Function): string {
  return fn.namespace === "Global" || !fn.namespace || fn.name.includes("::")
    ? fn.name : `${fn.namespace}::${fn.name}`;
}

export function matchFunctions(fn: AuditFunction, originals: Ps2Function[]): Ps2Function[] {
  const exact = originals.filter(original => qualifiedName(original) === fn.name);
  if (exact.length) return exact;
  // FUN_ addresses survive renames. Do not guess by an unqualified member name.
  const address = /(?:^|::)FUN_([0-9a-f]{8})$/i.exec(fn.name)?.[1];
  return address ? originals.filter(original => original.entry.toLowerCase().replace(/^0x/, "") === address.toLowerCase()) : [];
}

function bodyNode(source: string): SyntaxNode | undefined {
  let body: SyntaxNode | undefined;
  walk(parse(source).rootNode, node => {
    if (node.type === "function_definition") {
      body = node.childForFieldName("body") ?? undefined;
      return false;
    }
  });
  return body;
}

function callName(node: SyntaxNode): string | undefined {
  return auditCallName(node);
}

export function expandHelpers(fn: AuditFunction, functions: AuditFunction[]): { source: string; helpers: string[] } {
  const owner = fn.name.includes("::") ? fn.name.slice(0, fn.name.lastIndexOf("::")) : "";
  const candidates = new Map<string, AuditFunction[]>();
  for (const helper of functions) {
    const list = candidates.get(helper.name) ?? [];
    list.push(helper);
    candidates.set(helper.name, list);
  }
  const helpers: string[] = [];
  function expand(source: string, active: Set<string>, depth: number): string {
    if (depth > 4) return source;
    const root = bodyNode(source);
    if (!root) return source;
    const replacements: { start: number; end: number; text: string }[] = [];
    walk(root, node => {
      if (node.type !== "expression_statement" || node.namedChildren.length !== 1) return;
      const call = node.namedChildren[0];
      if (call.type !== "call_expression" || call.childForFieldName("arguments")?.text.replace(/\s/g, "") !== "()") return;
      const target = call.childForFieldName("function");
      if (!target) return;
      // Never inline another object's method: its 'this' would be different.
      if (target.type === "field_expression" && target.childForFieldName("argument")?.text !== "this") return;
      const name = callName(target);
      if (!name) return;
      const fullName = target.text.includes("::") ? target.text : owner ? `${owner}::${name}` : name;
      const matches = candidates.get(fullName) ?? [];
      if (matches.length !== 1) return;
      const helper = matches[0];
      const helperRoot = bodyNode(helper.source);
      if (!helper.noArguments || helper.parseError || !/^\s*void\b/.test(helper.source) || !helperRoot || helperRoot.namedChildren.length > 20 || active.has(helper.id)) return;
      // Pure straight-line field updates and helper calls only; locals would need binding-aware substitution.
      if (helperRoot.namedChildren.some(child => !["expression_statement", "return_statement", "comment"].includes(child.type))) return;
      if (helperRoot.namedChildren.some(child => child.type === "return_statement" && child.text.replace(/\s/g, "") !== "return;")) return;
      if (helperRoot.namedChildren.some((child, i, all) => child.type === "return_statement" && all.slice(i + 1).some(next => next.type !== "comment"))) return;
      const expanded = expand(helper.source, new Set([...active, helper.id]), depth + 1);
      const expandedBody = bodyNode(expanded)?.text;
      if (!expandedBody) return;
      helpers.push(`${helper.name} (${helper.path}:${helper.line})`);
      replacements.push({ start: node.startIndex, end: node.endIndex, text: `{ /* expanded ${helper.name} */ ${expandedBody.slice(1, -1).replace(/\breturn\s*;/g, "")} }` });
      return false;
    });
    for (const replacement of replacements.sort((a, b) => b.start - a.start)) {
      source = source.slice(0, replacement.start) + replacement.text + source.slice(replacement.end);
    }
    return source;
  }
  return { source: expand(fn.source, new Set([fn.id]), 0), helpers: [...new Set(helpers)] };
}

const ignoredCalls = new Set([
  "ZONE_SCOPED", "ZONE_SCOPED_NAME", "TRACK_ALLOC", "TRACK_FREE", "assert",
  "STORE_POINTER", "LOAD_POINTER", "LOAD_POINTER_CAST", "static_cast", "reinterpret_cast", "const_cast", "dynamic_cast", "TO_HEAP",
  "uint", "ulong", "ushort", "byte", "uchar", "undefined1", "undefined2", "undefined4", "undefined8",
  "int", "float", "double", "bool", "short", "long"
]);

export function fingerprint(source: string) {
  const calls: { name: string; end: number }[] = [];
  let indirectCalls = 0;
  const ignored = new Set<string>();
  const constants = new Map<string, number>();
  const root = bodyNode(source);
  if (root) walk(root, node => {
    if (node.type === "call_expression") {
      const target = node.childForFieldName("function");
      const name = target ? callName(target) : undefined;
      if (name && (name === "assert" || /^(?:ZONE_SCOPED.*|TRACK_ALLOC|TRACK_FREE|[A-Z_]*LOG(?:_SLOW)?|IMPLEMENTATION_GUARD.*)$/.test(name))) {
        ignored.add(name);
        return false;
      }
      if (!name) indirectCalls++;
      if (name && !ignoredCalls.has(name) && !/^(?:[A-Z_]*LOG(?:_SLOW)?|IMPLEMENTATION_GUARD.*)$/.test(name)) calls.push({ name, end: node.endIndex });
    }
    if (node.type === "number_literal") {
      const text = node.text;
      // Integer values use BigInt, so 64-bit masks never lose precision.
      const integer = text.replace(/[uUlL]+$/, "");
      let value: string;
      try {
        if (/^(?:0[xX][\da-fA-F]+|0[bB][01]+|\d+)$/.test(integer)) {
          value = String(BigInt(/^0[0-7]+$/.test(integer) ? `0o${integer.slice(1)}` : integer));
        } else {
          value = String(Number(text.replace(/[fFlL]$/, "")));
          if (!Number.isFinite(Number(value))) return;
        }
      } catch { return; }
      if (node.parent?.type === "unary_expression" && node.parent.text.trimStart().startsWith("-")) value = `-${value}`;
      constants.set(value, (constants.get(value) ?? 0) + 1);
    }
  });
  // Nested argument calls must finish before the enclosing call. Ghidra often
  // splits those arguments into preceding temporary assignments.
  return { calls: calls.sort((a, b) => a.end - b.end).map(call => call.name), constants, indirectCalls, ignored: [...ignored], parseError: !root || parse(source).rootNode.hasError };
}

function counts(values: string[]) {
  const result = new Map<string, number>();
  for (const value of values) result.set(value, (result.get(value) ?? 0) + 1);
  return result;
}

export function compareFunction(fn: AuditFunction, original: SnapshotEntry, functions: AuditFunction[], context: AuditContext = { constants: new Map() }) {
  const expanded = expandHelpers(fn, functions);
  const normalizedPort = normalizeAuditSource(expanded.source, context);
  const normalizedPs2 = normalizeAuditSource(original.c ?? "", context);
  const port = fingerprint(normalizedPort.source);
  const ps2 = fingerprint(normalizedPs2.source);
  const normalizations = [...new Set([...normalizedPort.notes, ...normalizedPs2.notes, ...port.ignored.map(name => `Excluded diagnostic/instrumentation call ${name} and its arguments.`)])];
  const normalized = { port: normalizedPort.source, ps2: normalizedPs2.source };
  const findings: Finding[] = [];
  const reviewedFindings: { finding: Finding; reason: string }[] = [];
  if (/\bIMPLEMENTATION_GUARD\w*\s*\(/.test(fn.source)) findings.push({ kind: "unfinished", detail: "Contains an implementation guard; inspect compiled behavior." });
  if (fn.parseError || port.parseError || ps2.parseError) {
    findings.push({ kind: "parse_error", detail: "At least one function did not parse completely; comparisons are incomplete." });
    return { findings, expanded, normalizations, normalized, reviewedFindings, compared: false };
  }
  const portCalls = counts(port.calls);
  const ps2Calls = counts(ps2.calls);
  for (const name of new Set([...portCalls.keys(), ...ps2Calls.keys()])) {
    const a = portCalls.get(name) ?? 0, b = ps2Calls.get(name) ?? 0;
    if (a !== b) findings.push({ kind: "call_count", detail: `${name}: port ${a}, PS2 ${b}.` });
  }
  if (port.calls.join("\0") !== ps2.calls.join("\0") && findings.every(f => f.kind !== "call_count")) {
    findings.push({ kind: "call_order", detail: "Direct call counts match, but their lexical order differs." });
  }
  if (port.indirectCalls !== ps2.indirectCalls) findings.push({ kind: "indirect_call_count", detail: `Unresolved indirect calls: port ${port.indirectCalls}, PS2 ${ps2.indirectCalls}. Inspect targets before deciding equivalence.` });
  for (const value of new Set([...port.constants.keys(), ...ps2.constants.keys()])) {
    // These values are dominated by C++/decompiler syntax differences.
    if (["0", "1", "-1"].includes(value)) continue;
    const a = port.constants.get(value) ?? 0, b = ps2.constants.get(value) ?? 0;
    if (a !== b) findings.push({ kind: "constant_count", detail: `Literal ${value}: port ${a}, PS2 ${b}.` });
  }
  for (const review of context.reviewed ?? []) {
    if (review.name !== fn.name || review.address !== original.function.entry ||
        review.sourceSha256 !== auditSourceHash(fn.source) || review.ps2Sha256 !== auditSourceHash(original.c ?? "")) continue;
    for (let i = findings.length - 1; i >= 0; i--) {
      if (!review.findings.some(finding => finding.kind === findings[i].kind && finding.detail === findings[i].detail)) continue;
      reviewedFindings.unshift({ finding: findings[i], reason: review.reason });
      findings.splice(i, 1);
    }
  }
  return { findings, expanded, normalizations, normalized, reviewedFindings, compared: true };
}
