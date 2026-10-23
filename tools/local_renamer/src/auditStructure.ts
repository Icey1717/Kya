import Parser, { type SyntaxNode } from "tree-sitter";
import Cpp from "tree-sitter-cpp";
import type { AuditFunction } from "./functionAudit.js";

const parser = new Parser();
parser.setLanguage(Cpp);
function parse(source: string) { return parser.parse(offset => source.slice(offset, offset + 16_384)); }
function walk(node: SyntaxNode, visit: (node: SyntaxNode) => boolean | void) {
  const pending = [node];
  while (pending.length) {
    const current = pending.pop()!;
    if (visit(current) === false) continue;
    for (const child of [...current.namedChildren].reverse()) pending.push(child);
  }
}
interface ClassInfo { name: string; bases: string[]; fields: Map<string, string | undefined>; methods: Map<string, Set<string>>; virtualMethods: Set<string>; unsupportedBases: boolean }
export interface AuditStructureContext { classes: Map<string, ClassInfo[]>; functions: Map<string, AuditFunction[]> }
function plainType(type: string) { return type.replace(/<.*>/, "").replace(/\b(?:const|volatile|class|struct)\b|[&*]/g, "").trim(); }
function singleClass(context: AuditStructureContext, name: string) {
  const exact = context.classes.get(plainType(name));
  return exact?.length === 1 ? exact[0] : undefined;
}

export function createAuditStructure(sources: string[], functions: AuditFunction[]): AuditStructureContext {
  const context: AuditStructureContext = { classes: new Map(), functions: new Map() };
  for (const fn of functions) {
    const entries = context.functions.get(fn.name) ?? [];
    entries.push(fn);
    context.functions.set(fn.name, entries);
  }
  for (const source of sources) walk(parse(source).rootNode, node => {
    if (node.type === "function_definition") return false;
    if (!["class_specifier", "struct_specifier"].includes(node.type)) return;
    const name = node.childForFieldName("name")?.text;
    const body = node.childForFieldName("body");
    if (!name || !body) return;
    const scopes: string[] = [];
    for (let parent = node.parent; parent; parent = parent.parent) {
      if (["namespace_definition", "class_specifier", "struct_specifier"].includes(parent.type)) {
        const scope = parent.childForFieldName("name")?.text;
        if (scope) scopes.unshift(scope);
      }
    }
    const fullName = [...scopes, name].join("::");
    const clause = node.namedChildren.find(child => child.type === "base_class_clause");
    const bases = clause?.namedChildren.filter(child => child.type !== "access_specifier").map(child => child.text) ?? [];
    const info: ClassInfo = { name: fullName, bases, fields: new Map(), methods: new Map(), virtualMethods: new Set(), unsupportedBases: Boolean(clause?.children.some(child => child.type === "virtual")) || bases.length > 1 };
    for (const declaration of body.namedChildren) {
      if (!["field_declaration", "function_definition"].includes(declaration.type)) continue;
      for (const declarator of declaration.childrenForFieldName("declarator")) {
        let isFunction = false;
        walk(declarator, child => {
          if (child.type === "function_declarator") {
            isFunction = true;
            const method = child.childForFieldName("declarator")?.text;
            if (method) {
              const signatures = info.methods.get(method) ?? new Set<string>();
              signatures.add(child.text);
              info.methods.set(method, signatures);
            }
            if (/\bvirtual\b|\boverride\b/.test(declaration.text)) {
              if (method) info.virtualMethods.add(method);
            }
            return false;
          }
        });
        if (isFunction || /\bstatic\b/.test(declaration.text)) continue;
        let field: SyntaxNode | undefined;
        walk(declarator, child => { if (child.type === "field_identifier") field = child; });
        const type = declaration.childForFieldName("type")?.text;
        if (field && type) {
          const fieldType = type + (declarator.text.includes("*") ? "*" : "");
          info.fields.set(field.text, !info.fields.has(field.text) || info.fields.get(field.text) === fieldType ? fieldType : undefined);
        }
      }
    }
    const entries = context.classes.get(fullName) ?? [];
    entries.push(info);
    context.classes.set(fullName, entries);
  });
  return context;
}

function baseName(context: AuditStructureContext, owner: string, name: string) {
  if (singleClass(context, name)) return plainType(name);
  const scope = owner.includes("::") ? owner.slice(0, owner.lastIndexOf("::")) : "";
  const scoped = scope ? `${scope}::${plainType(name)}` : plainType(name);
  return singleClass(context, scoped) ? scoped : undefined;
}
function lookupField(context: AuditStructureContext, owner: string, name: string, depth = 0): string | undefined {
  if (depth > 8) return;
  const info = singleClass(context, owner);
  if (!info) return;
  if (info.fields.has(name)) return info.fields.get(name);
  if (info.unsupportedBases || info.bases.length !== 1) return;
  const base = baseName(context, owner, info.bases[0]);
  return base ? lookupField(context, base, name, depth + 1) : undefined;
}
function lookupMethod(context: AuditStructureContext, owner: string, name: string, depth = 0): AuditFunction | undefined {
  if (depth > 8) return;
  const info = singleClass(context, owner);
  if ((info?.methods.get(name)?.size ?? 0) > 1) return;
  const matches = context.functions.get(owner ? `${owner}::${name}` : name);
  if (matches) return matches.length === 1 ? matches[0] : undefined;
  if (info?.methods.has(name)) return; // Do not bind a hidden base method to an unrecovered override.
  if (!info || info.unsupportedBases || info.bases.length !== 1) return;
  const base = baseName(context, owner, info.bases[0]);
  return base ? lookupMethod(context, base, name, depth + 1) : undefined;
}
function mayDispatchVirtually(context: AuditStructureContext, owner: string, name: string, depth = 0): boolean {
  if (!owner) return false;
  const info = singleClass(context, owner);
  if (!info || depth > 8 || info.unsupportedBases) return true;
  if (info.virtualMethods.has(name)) return true;
  return info.bases.some(type => {
    const base = baseName(context, owner, type);
    return !base || mayDispatchVirtually(context, base, name, depth + 1);
  });
}

function functionNode(source: string) {
  let fn: SyntaxNode | undefined;
  walk(parse(source).rootNode, node => { if (node.type === "function_definition") { fn = node; return false; } });
  return fn;
}
function safeActual(node: SyntaxNode): boolean {
  let safe = true;
  walk(node, child => {
    if (!["identifier", "this", "field_expression", "field_identifier", "number_literal", "true", "false", "null", "nullptr", "parenthesized_expression", "pointer_expression", "qualified_identifier", "namespace_identifier"].includes(child.type)) safe = false;
  });
  return safe;
}

export function expandTransparentHelpers(source: string, functionName: string, context: AuditStructureContext, originalCalls = new Set<string>()) {
  const notes: string[] = [];
  const owner = functionName.includes("::") ? functionName.slice(0, functionName.lastIndexOf("::")) : "";
  const active = new Set<string>([functionName]);
  function expand(input: string, scope: string, receiver: string, depth: number): string {
    if (depth > 4 || input.length > 256_000) return input;
    const fn = functionNode(input);
    const root = fn?.childForFieldName("body");
    if (!root) return input;
    const shadowed = new Set<string>();
    walk(fn!, node => {
      if (!["declaration", "parameter_declaration", "optional_parameter_declaration"].includes(node.type)) return;
      for (const declarator of node.childrenForFieldName("declarator")) walk(declarator, binding => {
        if (binding.type === "init_declarator") {
          const name = binding.childForFieldName("declarator");
          if (name) walk(name, child => { if (child.type === "identifier") shadowed.add(child.text); });
          return false;
        }
        if (binding.type === "identifier") shadowed.add(binding.text);
      });
    });
    const edits: { start: number; end: number; text: string }[] = [];
    walk(root, call => {
      if (call.type !== "call_expression") return;
      const target = call.childForFieldName("function");
      const args = call.childForFieldName("arguments")?.namedChildren;
      if (!target || !args || args.some(arg => !safeActual(arg))) return;
      let methodScope = scope, methodName: string | undefined, object = receiver, explicit = false;
      if (target.type === "identifier") { if (shadowed.has(target.text)) return; methodName = target.text; }
      else if (target.type === "qualified_identifier") {
        methodName = target.childForFieldName("name")?.text;
        const qualifier = target.childForFieldName("scope")?.text;
        if (!qualifier || !singleClass(context, qualifier)) return;
        methodScope = qualifier;
        explicit = true; // Qualified base calls bypass virtual dispatch.
      } else if (target.type === "field_expression") {
        const argument = target.childForFieldName("argument");
        methodName = target.childForFieldName("field")?.text;
        if (!argument || !safeActual(argument)) return;
        if (argument.type === "this") object = receiver;
        else {
          if (argument.type === "identifier" && shadowed.has(argument.text)) return;
          const field = argument.type === "identifier" ? argument.text
            : argument.type === "field_expression" && argument.childForFieldName("argument")?.type === "this" ? argument.childForFieldName("field")?.text : undefined;
          const type = field ? lookupField(context, scope, field) : undefined;
          if (!type || !singleClass(context, plainType(type))) return;
          methodScope = plainType(type);
          object = target.children.some(child => child.type === "->") ? argument.text : `&(${argument.text})`;
        }
      }
      if (!methodName) return;
      if (originalCalls.has(methodName)) return; // Leave already matching named operations intact.
      const helper = lookupMethod(context, methodScope, methodName);
      if (!helper || helper.parseError || active.has(helper.name)) return;
      const helperOwner = helper.name.includes("::") ? helper.name.slice(0, helper.name.lastIndexOf("::")) : "";
      if (methodName === helperOwner.split("::").at(-1) || methodName.startsWith("~")) return;
      if (!explicit && mayDispatchVirtually(context, methodScope, methodName)) return;
      const definition = functionNode(helper.source);
      let declarator = definition?.childForFieldName("declarator");
      while (declarator && declarator.type !== "function_declarator") declarator = declarator.childForFieldName("declarator");
      const parameters = declarator?.childForFieldName("parameters")?.namedChildren ?? [];
      if (parameters.length !== args.length || parameters.some(param => param.type !== "parameter_declaration")) return;
      const bindings = new Map<string, string>();
      for (let index = 0; index < parameters.length; index++) {
        const parameter = parameters[index];
        const name = parameter.childForFieldName("declarator");
        // No pointer/ref parameter rebinding or implicit conversions are guessed.
        if (!name || name.type !== "identifier" || parameter.childForFieldName("type")?.text === "auto") return;
        bindings.set(name.text, `(${args[index].text})`);
      }
      const body = definition?.childForFieldName("body");
      const statements = body?.namedChildren.filter(child => child.type !== "comment") ?? [];
      if (definition?.childForFieldName("type")?.text === "void" && statements.at(-1)?.text.replace(/\s/g, "") === "return;") statements.pop();
      if (statements.length !== 1) return;
      const statement = statements[0];
      const expression = statement.namedChildren[0];
      const returning = statement.type === "return_statement";
      if (!expression || !["expression_statement", "return_statement"].includes(statement.type)) return;
      if (parameters.some(param => (expression.text.match(new RegExp(`\\b${param.childForFieldName("declarator")!.text}\\b`, "g"))?.length ?? 0) > 1)) return;
      const forwarding = expression.type === "call_expression";
      // Accessors may only read fields (possibly through pointer conversion).
      let accessor = returning && expression.type !== "number_literal";
      walk(expression, node => {
        if (["assignment_expression", "update_expression", "new_expression", "delete_expression", "lambda_expression", "conditional_expression"].includes(node.type)) accessor = false;
        if (node.type === "call_expression") {
          const name = node.childForFieldName("function")?.text;
          if (!name || !/^(?:LOAD_POINTER(?:_CAST)?|static_cast<[^>]+>|reinterpret_cast<[^>]+>)$/.test(name)) accessor = false;
        }
      });
      if (!forwarding && !accessor) return;
      if (!returning && call.parent?.type !== "expression_statement") return;
      // A forwarding return can evaluate its callee, but not hidden argument
      // calls or writes. They need sequencing/type analysis before expansion.
      let unsafe = false;
      walk(expression, node => {
        if (["assignment_expression", "update_expression", "new_expression", "delete_expression", "lambda_expression"].includes(node.type)) unsafe = true;
        if (node.type === "call_expression" && node.id !== expression.id && !/^LOAD_POINTER/.test(node.childForFieldName("function")?.text ?? "")) unsafe = true;
      });
      if (unsafe) return;
      const replacements: { start: number; end: number; text: string }[] = [];
      walk(expression, node => {
        let text: string | undefined;
        if (node.type === "this") text = `(${object})`;
        if (node.type === "identifier") {
          text = bindings.get(node.text);
          if (!text && lookupField(context, helperOwner, node.text)) text = `(${object})->${node.text}`;
        }
        if (text) replacements.push({ start: node.startIndex - expression.startIndex, end: node.endIndex - expression.startIndex, text });
      });
      let replacement = expression.text;
      for (const edit of replacements.sort((a, b) => b.start - a.start)) replacement = replacement.slice(0, edit.start) + edit.text + replacement.slice(edit.end);
      // Refuse unused actuals that would be erased from the fingerprint. Even a
      // literal argument can carry evidence of a wrong parameter value.
      if (parameters.some((param, index) => !new RegExp(`\\b${param.childForFieldName("declarator")!.text}\\b`).test(expression.text) && args[index].type !== "identifier")) return;
      active.add(helper.name);
      const wrapper = `void AuditExpansion() { ${returning ? "value = " : ""}${replacement}; }`;
      const nested = expand(wrapper, helperOwner, object, depth + 1);
      active.delete(helper.name);
      const nestedBody = functionNode(nested)?.childForFieldName("body")?.namedChildren[0];
      if (returning) replacement = nestedBody?.namedChildren[0]?.childForFieldName("right")?.text ?? replacement;
      else replacement = nestedBody?.namedChildren[0]?.text ?? replacement;
      notes.push(`Expanded transparent helper ${helper.name} (${helper.path}:${helper.line}).`);
      edits.push({ start: call.startIndex, end: call.endIndex, text: `(${replacement})` });
      return false;
    });
    for (const edit of edits.sort((a, b) => b.start - a.start)) input = input.slice(0, edit.start) + edit.text + input.slice(edit.end);
    return input;
  }
  return { source: expand(source, owner, "this", 0), notes: [...new Set(notes)] };
}

export function modelConstructor(source: string, name: string, original: string, context: AuditStructureContext) {
  const notes: string[] = [];
  const owner = name.slice(0, name.lastIndexOf("::"));
  if (!name.includes("::") || name.split("::").at(-1) !== owner.split("::").at(-1)) return { source, notes };
  const info = singleClass(context, owner), fn = functionNode(source), body = fn?.childForFieldName("body");
  if (!info || info.unsupportedBases || !body) return { source, notes };
  const initializers = fn!.namedChildren.find(node => node.type === "field_initializer_list")?.namedChildren ?? [];
  if (initializers.some(init => init.namedChildren[0]?.text === owner)) return { source, notes }; // delegating constructor
  if (initializers.some(init => !info.fields.has(init.namedChildren[0]?.text ?? "") && !info.bases.some(base => [base, base.split("::").at(-1)].includes(init.namedChildren[0]?.text)))) return { source, notes };
  const injected: string[] = [];
  for (const baseType of info.bases) {
    const base = baseName(context, owner, baseType);
    if (!base) continue;
    const short = base.split("::").at(-1)!;
    const initializer = initializers.find(init => [base, short].includes(init.namedChildren[0]?.text));
    const argumentsNode = initializer?.namedChildren.find(node => ["argument_list", "initializer_list"].includes(node.type));
    const args = argumentsNode?.namedChildren.map(node => node.text) ?? [];
    let matched = false;
    walk(parse(original).rootNode, node => {
      if (node.type !== "call_expression") return;
      const target = node.childForFieldName("function")?.text;
      const actual = node.childForFieldName("arguments")?.namedChildren;
      if (target !== `${base}::${short}` || !actual?.length) return;
      // Only the actual base subobject of this instance, never another object.
      let receiver = actual[0];
      while (["cast_expression", "parenthesized_expression"].includes(receiver.type)) receiver = receiver.childForFieldName("value") ?? receiver.namedChildren.at(-1)!;
      if (receiver?.type === "this" || receiver?.text === "this") matched = true;
    });
    if (!matched && !initializer) continue;
    injected.push(`${base}::${short}(${args.join(", ")});`);
    notes.push(`Modeled ${initializer ? "explicit" : "implicit default"} base construction ${base} in ${name}.`);
  }
  // C++ initializes members in declaration order, regardless of initializer-list spelling.
  const fieldOrder = [...info.fields.keys()];
  for (const init of [...initializers].sort((a, b) => fieldOrder.indexOf(a.namedChildren[0]?.text ?? "") - fieldOrder.indexOf(b.namedChildren[0]?.text ?? ""))) {
    if (info.bases.some(base => [base, base.split("::").at(-1)].includes(init.namedChildren[0]?.text))) continue;
    const args = init.namedChildren.find(node => ["argument_list", "initializer_list"].includes(node.type))?.namedChildren;
    if (args?.length) injected.push(`(${args.map(node => node.text).join(", ")});`);
  }
  if (injected.length) source = source.slice(0, body.startIndex + 1) + injected.join("\n") + source.slice(body.startIndex + 1);
  return { source, notes };
}
