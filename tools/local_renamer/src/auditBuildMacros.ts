import { readFile } from "node:fs/promises";
import { execFile } from "node:child_process";
import { resolve, relative } from "node:path";
import { createHash } from "node:crypto";
import Parser from "tree-sitter";
import Cpp from "tree-sitter-cpp";

export interface AuditMacro { parameters: string[]; body: string }
const parser = new Parser();
parser.setLanguage(Cpp);

export function parseExpressionMacro(line: string): { name: string; macro: AuditMacro } | undefined {
  const match = /^#define\s+([A-Za-z_]\w*)\(([^)]*)\)\s+(.+)$/.exec(line);
  if (!match || /#|\b(?:assert|IMPLEMENTATION_GUARD\w*|LOAD_POINTER\w*|STORE_POINTER\w*)\b/.test(match[1])) return;
  const parameters = match[2].trim() ? match[2].split(",").map(value => value.trim()) : [];
  if (parameters.some(value => !/^[A-Za-z_]\w*$/.test(value)) || new Set(parameters).size !== parameters.length) return;
  const body = match[3];
  // Only one expression; no statements, token pasting, stringification or variadics.
  if (/[#;{}]/.test(body)) return;
  if (body.length > 4096) return;
  const tree = parser.parse(`void AuditMacro() { result = (${body}); }`);
  if (tree.rootNode.hasError) return;
  return { name: match[1], macro: { parameters, body } };
}

export function expandExpressionMacros(source: string, macros: Map<string, AuditMacro>) {
  if (/^\s*#\s*(?:define|undef|include|pragma)\b/m.test(source)) {
    return { source, notes: ["Skipped build macro expansion: function contains preprocessor state changes."] };
  }
  const notes = new Set<string>();
  function acyclic(name: string, active = new Set<string>(), depth = 0): boolean {
    if (active.has(name) || depth > 8) return false;
    const macro = macros.get(name);
    if (!macro) return true;
    const nested = new Set(active).add(name);
    for (const match of macro.body.matchAll(/(?<![\w:.])([A-Za-z_]\w*)\s*\(/g)) {
      if (macro.parameters.includes(match[1]) || !acyclic(match[1], nested, depth + 1)) return false;
    }
    return true;
  }
  const walk = (node: Parser.SyntaxNode, visit: (node: Parser.SyntaxNode) => boolean | void) => {
    const pending = [node];
    while (pending.length) {
      const current = pending.pop()!;
      if (visit(current) === false) continue;
      for (const child of [...current.namedChildren].reverse()) pending.push(child);
    }
  };
  // Bound both recursive macros and total growth. Partial bounded expansion is
  // still valid C++ and leaves remaining macro calls visible to the audit.
  for (let pass = 0; pass < 4; pass++) {
    const edits: { start: number; end: number; text: string }[] = [];
    const tree = parser.parse(offset => source.slice(offset, offset + 16_384));
    walk(tree.rootNode, node => {
      if (node.type !== "call_expression") return;
      const target = node.childForFieldName("function");
      if (target?.type !== "identifier") return;
      const macro = macros.get(target.text);
      const args = node.childForFieldName("arguments")?.namedChildren;
      if (!macro || !args || args.length !== macro.parameters.length || macro.body.length > 4096 || !acyclic(target.text)) return;
      let safe = true;
      for (const arg of args) walk(arg, child => {
        if (["call_expression", "assignment_expression", "update_expression", "new_expression", "delete_expression", "lambda_expression"].includes(child.type)) safe = false;
      });
      if (!safe || /["']/.test(macro.body)) return;
      // Token substitution intentionally does not add parentheses around actual
      // arguments: unparenthesized macro parameters must retain C macro semantics.
      const bindings = new Map(macro.parameters.map((name, index) => [name, args[index].text]));
      const body = macro.body.replace(/\b[A-Za-z_]\w*\b/g, word => bindings.get(word) ?? word);
      if (body.length > 16_384 || source.length + body.length > 256_000) return;
      notes.add(`Expanded build-active expression macro ${target.text}.`);
      edits.push({ start: node.startIndex, end: node.endIndex, text: `(${body})` });
      return false;
    });
    if (!edits.length) break;
    for (const edit of edits.sort((a, b) => b.start - a.start)) source = source.slice(0, edit.start) + edit.text + source.slice(edit.end);
  }
  return { source, notes: [...notes] };
}

function key(path: string) { return resolve(path).replaceAll("\\", "/").toLowerCase(); }

// This tokenizes generated argument lists only. They are passed directly to
// execFile; neither Ninja commands nor a shell are executed.
export function buildArguments(value: string): string[] {
  if (/\$(?![: $])/.test(value)) throw new Error("Unresolved Ninja variable in compiler arguments");
  return (value.replace(/\$([: $])/g, "$1").match(/(?:[^\s"]+|"[^"]*")+/g) ?? []).map(token => token.replaceAll('"', ""));
}

export function macroStates(output: string, sourcePath: string, lines: number[]) {
  const wanted = new Set(lines);
  const result = new Map<number, Map<string, AuditMacro>>();
  const active = new Map<string, AuditMacro>();
  const parsed = new Map<string, ReturnType<typeof parseExpressionMacro>>();
  let inSource = false, line = 0;
  const sourceKey = key(sourcePath);
  for (const text of output.split(/\r?\n/)) {
    const location = /^#\s+(\d+)\s+"([^"]+)"/.exec(text);
    if (location) { line = Number(location[1]); inSource = key(location[2].replaceAll("\\\\", "\\")) === sourceKey; continue; }
    const definition = /^#define\s+([A-Za-z_]\w*)/.exec(text);
    const undef = /^#undef\s+([A-Za-z_]\w*)/.exec(text);
    if (definition) {
      if (!parsed.has(text)) parsed.set(text, parseExpressionMacro(text));
      const value = parsed.get(text);
      active.delete(definition[1]);
      if (value) active.set(value.name, value.macro);
    } else if (undef) active.delete(undef[1]);
    else if (wanted.has(line) && inSource && !result.has(line)) result.set(line, new Map(active));
    line++;
  }
  return result;
}

interface CompileSettings { compiler: string; args: string[] }

export class AuditBuildMacros {
  readonly errors: { path: string; error: string }[] = [];
  readonly settings = new Map<string, CompileSettings>();
  private cache = new Map<string, Promise<Map<number, Map<string, AuditMacro>> | undefined>>();
  private processed = new Set<string>();
  private constructor(readonly directory: string, readonly sha256: string) {}

  static async open(directory: string) {
    const ninja = await readFile(resolve(directory, "build.ninja"), "utf8");
    const rules = await readFile(resolve(directory, "CMakeFiles/rules.ninja"), "utf8");
    const context = new AuditBuildMacros(directory, createHash("sha256").update(ninja).update(rules).digest("hex"));
    const compilers = new Map<string, string>();
    for (const match of rules.matchAll(/^rule (CXX_COMPILER__\w+):?\r?\n([\s\S]*?)(?=^rule |$(?![\s\S]))/gm)) {
      const executable = /^\s*command = (.*?) \$DEFINES/m.exec(match[2])?.[1].replace(/\$\{(?:LAUNCHER|CODE_CHECK)\}/g, "");
      if (!executable) continue;
      const tokens = buildArguments(executable);
      if (tokens.length === 1 && /(?:^|[\\/])clang(?:\+\+)?(?:\.exe)?$/i.test(tokens[0])) compilers.set(match[1], tokens[0]);
    }
    for (const match of ninja.matchAll(/^build CMakeFiles\/Kya\.dir\/[^\r\n]+?: (CXX_COMPILER__\w+) ([^\r\n]+)\r?\n((?:  [^\r\n]*\r?\n)+)/gm)) {
      const compiler = compilers.get(match[1]);
      if (!compiler) continue;
      const source = match[2].split(" || ")[0].replace(/\$([: $])/g, "$1");
      const value = (name: string) => new RegExp(`^  ${name} = (.*)$`, "m").exec(match[3])?.[1] ?? "";
      const args = [...buildArguments(value("DEFINES")), ...buildArguments(value("INCLUDES")), ...buildArguments(value("FLAGS"))];
      context.settings.set(key(resolve(directory, source)), { compiler, args });
    }
    if (!context.settings.size) throw new Error("No supported Kya Clang compile rules in build.ninja");
    return context;
  }

  async forFile(path: string, lines: number[]) {
    const source = key(path);
    const settings = this.settings.get(source);
    if (!settings) return;
    if (!this.cache.has(source)) {
      // Inventory comparison is grouped by file. Keep the current translation
      // unit's macro states without retaining every system-header map forever.
      this.cache.clear();
      this.processed.add(source);
      this.cache.set(source, new Promise<Map<number, Map<string, AuditMacro>> | undefined>(accept => {
        execFile(settings.compiler, [...settings.args, "-E", "-dD", path], { cwd: this.directory, windowsHide: true, maxBuffer: 64 * 1024 * 1024, timeout: 60_000 }, (error, stdout, stderr) => {
          if (error) {
            this.errors.push({ path, error: String(stderr || error.message).slice(0, 1500) });
            accept(undefined); // Failed preprocessing never supplies partial macros.
          } else accept(macroStates(stdout, path, lines));
        });
      }));
    }
    return this.cache.get(source)!;
  }

  describe(repo: string) { return { directory: relative(repo, this.directory).replaceAll("\\", "/"), sha256: this.sha256, files: this.processed.size, errors: this.errors }; }
}
