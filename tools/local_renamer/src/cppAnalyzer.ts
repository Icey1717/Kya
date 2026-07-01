import { createHash } from "node:crypto";
import Parser, { type SyntaxNode } from "tree-sitter";
import Cpp from "tree-sitter-cpp";
import type { Declaration, FunctionRecord } from "./types.js";

const parser = new Parser();
parser.setLanguage(Cpp);

export function identifierOccurrences(source: string, name: string): Array<{ startIndex: number; endIndex: number }> {
  const occurrences: Array<{ startIndex: number; endIndex: number }> = [];
  const tree = parser.parse(source);
  walk(tree.rootNode, (node) => {
    if (node.type === "identifier" && source.slice(node.startIndex, node.endIndex) === name) {
      occurrences.push({ startIndex: node.startIndex, endIndex: node.endIndex });
    }
  });
  return occurrences;
}

const scopeTypes = new Set([
  "compound_statement",
  "for_statement",
  "for_range_loop",
  "while_statement",
  "do_statement",
  "if_statement",
  "switch_statement",
  "try_statement",
  "catch_clause"
]);

function walk(node: SyntaxNode, visit: (node: SyntaxNode) => boolean | void): void {
  if (visit(node) === false) return;
  for (const child of node.namedChildren) walk(child, visit);
}

function hasNodeType(node: SyntaxNode, type: string): boolean {
  let found = false;
  walk(node, (child) => {
    if (child.type === type) {
      found = true;
      return false;
    }
  });
  return found;
}

function declaratorIdentifier(node: SyntaxNode | null): SyntaxNode | null {
  if (!node) return null;
  if (node.type === "identifier" || node.type === "field_identifier") return node;
  const nested = node.childForFieldName("declarator");
  if (nested) return declaratorIdentifier(nested);
  for (const child of node.namedChildren) {
    const result = declaratorIdentifier(child);
    if (result) return result;
  }
  return null;
}

function functionName(node: SyntaxNode, source: string): string {
  let declarator = node.childForFieldName("declarator");
  while (declarator?.childForFieldName("declarator")) declarator = declarator.childForFieldName("declarator");
  if (!declarator) return "<anonymous>";
  return source.slice(declarator.startIndex, declarator.endIndex).replace(/\s+/g, " ").trim();
}

function scopePath(node: SyntaxNode, functionNode: SyntaxNode): number[] {
  const path: number[] = [];
  let current: SyntaxNode | null = node.parent;
  while (current && current.id !== functionNode.id) {
    if (scopeTypes.has(current.type)) {
      const parent = current.parent;
      path.push(parent ? parent.namedChildren.findIndex((child) => child.id === current!.id) : 0);
    }
    current = current.parent;
  }
  return path.reverse();
}

function declaredType(base: string, declarator: SyntaxNode, source: string): string {
  const declaratorText = source.slice(declarator.startIndex, declarator.endIndex);
  const pointer = declaratorText.includes("*") ? " *" : "";
  const reference = declaratorText.includes("&&") ? " &&" : declaratorText.includes("&") ? " &" : "";
  const array = declaratorText.includes("[") ? " []" : "";
  return `${base || "unknown"}${pointer}${reference}${array}`.trim();
}

function declarationBaseType(owner: SyntaxNode, firstDeclarator: SyntaxNode, source: string): string {
  let prefix = source.slice(owner.startIndex, firstDeclarator.startIndex).trim();
  if (owner.type === "for_range_loop") prefix = prefix.slice(prefix.lastIndexOf("(") + 1).trim();
  return prefix;
}

function declarationDeclarators(node: SyntaxNode): SyntaxNode[] {
  const results: SyntaxNode[] = [];
  for (const declarator of node.childrenForFieldName("declarator")) {
    const unwrapped = declarator.type === "init_declarator"
      ? declarator.childForFieldName("declarator")
      : declarator;
    if (unwrapped) results.push(unwrapped);
  }
  if (results.length > 0) return results;
  for (const child of node.namedChildren) {
    if (child.type === "init_declarator") {
      const declarator = child.childForFieldName("declarator");
      if (declarator) results.push(declarator);
      continue;
    }
    if (
      child.type.endsWith("_declarator") &&
      child.type !== "structured_binding_declarator" &&
      child.type !== "function_declarator"
    ) {
      results.push(child);
    }
  }
  return results;
}

function collectParameters(functionNode: SyntaxNode, source: string): string[] {
  const declarator = functionNode.childForFieldName("declarator");
  if (!declarator) return [];
  const parameters: string[] = [];
  walk(declarator, (node) => {
    if (node.type !== "parameter_declaration" && node.type !== "optional_parameter_declaration") return;
    const identifier = declaratorIdentifier(node.childForFieldName("declarator"));
    if (identifier) parameters.push(source.slice(identifier.startIndex, identifier.endIndex));
    return false;
  });
  return parameters;
}

function collectDeclarations(
  functionNode: SyntaxNode,
  source: string
): { declarations: Declaration[]; ambiguous: boolean; removableDefinitionRanges: Array<{ start: number; end: number }> } {
  const body = functionNode.childForFieldName("body");
  if (!body) return { declarations: [], ambiguous: true, removableDefinitionRanges: [] };
  const pending: Array<Omit<Declaration, "id"> & { owner: SyntaxNode }> = [];
  let ambiguous = false;

  const addDeclarator = (owner: SyntaxNode, declarator: SyntaxNode, baseType?: string): void => {
    if (hasNodeType(declarator, "structured_binding_declarator")) {
      ambiguous = true;
      return;
    }
    if (hasNodeType(declarator, "function_declarator") && !hasNodeType(declarator, "pointer_declarator")) return;
    const identifier = declaratorIdentifier(declarator);
    if (!identifier) {
      ambiguous = true;
      return;
    }
    const oldName = source.slice(identifier.startIndex, identifier.endIndex);
    const type = declaredType(baseType ?? declarationBaseType(owner, declarator, source), declarator, source);
    pending.push({
      oldName,
      declaredType: type,
      startIndex: identifier.startIndex,
      endIndex: identifier.endIndex,
      scopePath: scopePath(identifier, functionNode),
      isPointer: type.includes("*"),
      isBoolean: !type.includes("*") && /\b(?:bool|BOOL|boolean)\b/.test(type),
      owner
    });
  };

  walk(body, (node) => {
    if (node.type === "lambda_expression") {
      ambiguous = true;
      return false;
    }
    if (node.type === "for_range_loop") {
      const declarator = node.childForFieldName("declarator");
      if (declarator) addDeclarator(node, declarator);
      else ambiguous = true;
      return;
    }
    if (node.type !== "declaration") return;
    if (hasNodeType(node, "structured_binding_declarator")) {
      ambiguous = true;
      return false;
    }
    const declarators = declarationDeclarators(node);
    const baseType = declarators[0] ? declarationBaseType(node, declarators[0], source) : "";
    for (const declarator of declarators) addDeclarator(node, declarator, baseType);
  });

  pending.sort((left, right) => left.startIndex - right.startIndex);
  const identifierOccurrences = new Map<string, Array<{ startIndex: number; endIndex: number }>>();
  walk(functionNode, (node) => {
    if (node.type !== "identifier") return;
    const name = source.slice(node.startIndex, node.endIndex);
    const occurrences = identifierOccurrences.get(name) ?? [];
    occurrences.push({ startIndex: node.startIndex, endIndex: node.endIndex });
    identifierOccurrences.set(name, occurrences);
  });
  const retained = pending.filter((declaration) => (identifierOccurrences.get(declaration.oldName)?.length ?? 0) > 1);
  const retainedStarts = new Set(retained.map((declaration) => declaration.startIndex));
  const owners = new Map<number, SyntaxNode>();
  for (const declaration of pending) owners.set(declaration.owner.id, declaration.owner);
  const removableDefinitionRanges = [...owners.values()]
    .filter((owner) => owner.type === "declaration" && owner.parent?.type === "compound_statement")
    .filter((owner) => {
      const owned = pending.filter((declaration) => declaration.owner.id === owner.id);
      return owned.length > 0 && owned.every((declaration) => !retainedStarts.has(declaration.startIndex));
    })
    .map((owner) => ({ start: owner.startIndex, end: owner.endIndex }));
  return {
    declarations: retained.map(({ owner: _owner, ...declaration }, index) => ({
      ...declaration,
      occurrences: identifierOccurrences.get(declaration.oldName) ?? [],
      id: `local_${String(index + 1).padStart(3, "0")}`
    })),
    ambiguous,
    removableDefinitionRanges
  };
}

function sourceWithoutRanges(source: string, sourceStart: number, ranges: Array<{ start: number; end: number }>): string {
  let result = source;
  for (const range of [...ranges].sort((left, right) => right.start - left.start)) {
    const start = range.start - sourceStart;
    const end = range.end - sourceStart;
    result = result.slice(0, start) + result.slice(end);
  }
  return result;
}

function attachedStart(source: string, functionStart: number): number {
  const prefix = source.slice(0, functionStart);
  const lines = prefix.split(/(?<=\n)/);
  let index = lines.length - 1;
  let inBlockComment = false;
  while (index >= 0) {
    const trimmed = lines[index].trim();
    if (inBlockComment) {
      if (trimmed.includes("/*")) inBlockComment = false;
      index--;
      continue;
    }
    if (trimmed.endsWith("*/")) {
      if (trimmed.includes("/*")) {
        index--;
        continue;
      }
      inBlockComment = true;
      index--;
      continue;
    }
    if (trimmed.startsWith("//")) {
      index--;
      continue;
    }
    break;
  }
  return lines.slice(0, index + 1).join("").length;
}

function isToleratedCommaConditionError(node: SyntaxNode, source: string): boolean {
  const text = source.slice(node.startIndex, node.endIndex).trim();
  if (text !== ")" && !text.endsWith(",")) return false;
  let ancestor: SyntaxNode | null = node.parent;
  while (ancestor && ancestor.type !== "function_definition") {
    if (ancestor.type === "if_statement" || ancestor.type === "while_statement") {
      const body = ancestor.childForFieldName("consequence") ?? ancestor.childForFieldName("body");
      const conditionText = source.slice(ancestor.startIndex, body?.startIndex ?? ancestor.endIndex);
      return conditionText.includes(",");
    }
    ancestor = ancestor.parent;
  }
  return false;
}

function containsUnsafeParseError(node: SyntaxNode, source: string): boolean {
  if (!node.hasError && !node.isMissing) return false;
  let unsafe = false;
  walk(node, (candidate) => {
    if (candidate.isMissing || (candidate.type === "ERROR" && !isToleratedCommaConditionError(candidate, source))) {
      unsafe = true;
      return false;
    }
  });
  return unsafe;
}

export function analyzeCppSource(relativePath: string, source: string): FunctionRecord[] {
  const tree = parser.parse(source);
  const functions: FunctionRecord[] = [];
  walk(tree.rootNode, (node) => {
    if (node.type !== "function_definition") return;
    const startIndex = attachedStart(source, node.startIndex);
    const extractedSource = source.slice(startIndex, node.endIndex);
    const collected = collectDeclarations(node, source);
    const parseError = containsUnsafeParseError(node, source);
    const id = createHash("sha256")
      .update(`${relativePath}\0${node.startIndex}\0${node.endIndex}\0${extractedSource}`)
      .digest("hex")
      .slice(0, 20);
    let exclusionReason: FunctionRecord["exclusionReason"];
    if (parseError) exclusionReason = "parse_failure";
    else if (collected.ambiguous) exclusionReason = "ambiguous_binding";
    else if (collected.declarations.length === 0) exclusionReason = "no_locals";
    functions.push({
      id,
      qualifiedName: functionName(node, source),
      relativePath,
      startIndex,
      endIndex: node.endIndex,
      startLine: source.slice(0, startIndex).split("\n").length,
      endLine: node.endPosition.row + 1,
      source: extractedSource,
      promptSource: sourceWithoutRanges(extractedSource, startIndex, collected.removableDefinitionRanges),
      parameters: collectParameters(node, source),
      declarations: collected.declarations,
      parseError,
      exclusionReason
    });
    return false;
  });
  return functions.sort((left, right) => left.startIndex - right.startIndex);
}
