import { readFile, writeFile, mkdir, readdir, rename } from "node:fs/promises";
import { resolve, relative, join, dirname } from "node:path";
import { createConnection } from "node:net";
import { createHash } from "node:crypto";
import { parseArgs } from "node:util";
import { fileURLToPath } from "node:url";
import { auditSourceHash, compareFunction, extractFunctions, matchFunctions, qualifiedName, type AuditFunction, type Ps2Function, type SnapshotEntry } from "./functionAudit.js";
import { createAuditContext, type ReviewedAuditFinding } from "./auditNormalization.js";
import { renderAuditReports } from "./auditReport.js";
import { createAuditStructure } from "./auditStructure.js";
import { AuditBuildMacros } from "./auditBuildMacros.js";

const repo = resolve(dirname(fileURLToPath(import.meta.url)), "../../..");
const { values, positionals } = parseArgs({ allowPositionals: true, options: {
  root: { type: "string", default: "src/b-witch" }, out: { type: "string", default: "out/function-audit" },
  snapshot: { type: "string" }, filter: { type: "string", default: "" }, limit: { type: "string", default: "0" },
  port: { type: "string", default: "18437" }, refresh: { type: "boolean", default: false }, help: { type: "boolean" },
  reviews: { type: "string", default: "tools/function-audit-reviewed.json" },
  "build-dir": { type: "string", default: "out/build/x64-debug" }, "no-build-macros": { type: "boolean", default: false }
} });

async function atomicJson(path: string, value: unknown) {
  await mkdir(dirname(path), { recursive: true });
  await writeFile(`${path}.tmp`, JSON.stringify(value, null, 2) + "\n");
  await rename(`${path}.tmp`, path);
}

async function request(command: Record<string, unknown>): Promise<any> {
  return new Promise((accept, reject) => {
    const socket = createConnection({ host: "127.0.0.1", port: Number(values.port) });
    let data = "";
    socket.setEncoding("utf8");
    socket.setTimeout(330_000, () => socket.destroy(new Error("Ghidra request timed out")));
    socket.on("connect", () => socket.write(JSON.stringify(command) + "\n"));
    socket.on("error", reject);
    socket.on("end", () => { if (!data.includes("\n")) reject(new Error("Ghidra disconnected before responding")); });
    socket.on("data", chunk => {
      data += chunk;
      if (data.length > 32 * 1024 * 1024) { socket.destroy(new Error("Ghidra response exceeds 32 MiB")); return; }
      if (!data.includes("\n")) return;
      socket.end();
      try {
        const response = JSON.parse(data.slice(0, data.indexOf("\n")));
        if (!response.ok) reject(new Error(`${response.code}: ${response.error}`));
        else accept(response.result);
      } catch (error) { reject(error); }
    });
  });
}

async function inventory(root: string): Promise<{ functions: AuditFunction[]; files: number; sources: string[] }> {
  const functions: AuditFunction[] = [];
  const sources: string[] = [];
  let files = 0;
  async function visit(path: string) {
    for (const entry of (await readdir(path, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
      const child = join(path, entry.name);
      if (entry.isDirectory()) await visit(child);
      else if (entry.isFile() && /\.(?:cpp|c|cc|cxx|h|hpp|inl)$/i.test(entry.name)) {
        files++;
        const source = await readFile(child, "utf8");
        sources.push(source);
        functions.push(...extractFunctions(relative(repo, child).replaceAll("\\", "/"), source));
      }
    }
  }
  await visit(root);
  return { functions, files, sources };
}

async function capture(functions: AuditFunction[], out: string): Promise<{ program: unknown; entries: SnapshotEntry[] }> {
  const program = await request({ command: "status" });
  const identity = createHash("sha256").update(JSON.stringify(program)).digest("hex").slice(0, 16);
  const cache = join(out, "cache", identity);
  await mkdir(cache, { recursive: true });
  const entries = new Map<string, SnapshotEntry>();
  const names = [...new Set(functions.map(fn => fn.name.split("::").at(-1)!))];
  for (let i = 0; i < names.length; i++) {
    const name = names[i];
    const key = createHash("sha256").update(name).digest("hex");
    const path = join(cache, `names-${key}.json`);
    let originals: Ps2Function[];
    try { if (values.refresh) throw new Error(); originals = JSON.parse(await readFile(path, "utf8")); }
    catch {
      const address = /^FUN_([0-9a-f]{8})$/i.exec(name)?.[1];
      if (address) {
        try { originals = [await request({ command: "get_function", address })]; }
        catch (error) {
          if (!String(error).includes("target_not_found")) throw error;
          originals = [];
        }
      } else {
        const result = await request({ command: "list_functions", filter: name, limit: 1000 });
        if (result.functions.length >= 1000) throw new Error(`Ghidra listing for ${name} may be truncated; use the batch exporter.`);
        originals = result.functions.filter((fn: Ps2Function) => fn.name === name || fn.name.endsWith(`::${name}`));
      }
      await atomicJson(path, originals);
    }
    for (const original of originals) {
      if (!functions.some(fn => matchFunctions(fn, [original]).length)) continue;
      const path = join(cache, `function-${original.entry}.json`);
      let entry: SnapshotEntry;
      try { if (values.refresh) throw new Error(); entry = JSON.parse(await readFile(path, "utf8")); }
      catch {
        try { entry = await request({ command: "decompile", address: original.entry }); }
        catch (error) { entry = { function: original, error: String(error) }; }
        // Errors are recorded in the snapshot, but retried on the next capture.
        if (!entry.error) await atomicJson(path, entry);
      }
      entries.set(original.entry, entry);
    }
    console.log(`[${i + 1}/${names.length}] ${name}: ${entries.size} originals captured`);
  }
  return { program, entries: [...entries.values()] };
}

async function main() {
  const command = positionals[0];
  if (values.help || !command) {
    console.log("Usage: npm run audit -- inventory|capture|compare [--root src/b-witch] [--out out/function-audit] [--filter Class::] [--limit N] [--snapshot path] [--reviews path] [--refresh] [--build-dir out/build/x64-debug] [--no-build-macros]\nPaths are relative to the repository. capture uses the local Ghidra bridge; compare makes no network/model requests.");
    return;
  }
  if (!["inventory", "capture", "compare"].includes(command)) throw new Error(`Unknown command ${command}`);
  const out = resolve(repo, values.out!);
  const all = await inventory(resolve(repo, values.root!));
  let selected = all.functions.filter(fn => fn.name.includes(values.filter!));
  const limit = Number(values.limit);
  if (!Number.isInteger(limit) || limit < 0) throw new Error("--limit must be a non-negative integer");
  if (limit) selected = selected.slice(0, limit);
  await mkdir(out, { recursive: true });
  await atomicJson(join(out, "inventory.json"), { schema: 1, files: all.files, functions: selected });
  console.log(`${selected.length} selected functions; ${all.files} source files scanned.`);
  if (command === "inventory") return;
  const snapshotPath = resolve(repo, values.snapshot ?? join(out, "ps2.jsonl"));
  if (command === "capture") {
    const snapshot = await capture(selected, out);
    const finalProgram = await request({ command: "status" });
    if (JSON.stringify(finalProgram) !== JSON.stringify(snapshot.program)) throw new Error("Selected Ghidra program changed during capture; snapshot not written.");
    await mkdir(dirname(snapshotPath), { recursive: true });
    await writeFile(snapshotPath, [JSON.stringify({ schema: 1, program: snapshot.program }), ...snapshot.entries.map(entry => JSON.stringify(entry))].join("\n") + "\n");
    console.log(`Saved ${snapshot.entries.length} originals to ${snapshotPath}`);
    return;
  }
  const lines = (await readFile(snapshotPath, "utf8")).split(/\r?\n/).filter(Boolean).map(line => JSON.parse(line));
  const identity = lines.shift();
  if (identity?.schema !== 1 || !identity.program) throw new Error("Expected a schema 1 PS2 JSONL snapshot");
  const entries = lines as SnapshotEntry[];
  if (entries.some(entry => !entry.function?.entry || !entry.function.name)) throw new Error("Invalid snapshot function entry");
  const originals = entries.map(entry => entry.function);
  let reviews: ReviewedAuditFinding[] = [];
  const ledger = JSON.parse(await readFile(resolve(repo, values.reviews!), "utf8"));
  if (ledger.schema !== 1 || !Array.isArray(ledger.reviews)) throw new Error("Invalid audit review ledger");
  if (ledger.program === identity.program.program && ledger.imageBase === identity.program.image_base) {
    for (const review of ledger.reviews as ReviewedAuditFinding[]) {
      if (!review.name || !review.address || !Array.isArray(review.findings) || !Array.isArray(review.dependencies) ||
          !/^[a-f0-9]{64}$/.test(review.sourceSha256) || !/^[a-f0-9]{64}$/.test(review.ps2Sha256) || !review.reason) throw new Error("Invalid audit review entry");
      let dependenciesMatch = true;
      for (const dependency of review.dependencies) {
        const path = resolve(repo, dependency.path);
        const withinRepo = relative(repo, path);
        if (withinRepo.startsWith("..") || resolve(path) === repo) throw new Error("Review dependency must be inside the repository");
        try { if (auditSourceHash(await readFile(path, "utf8")) !== dependency.sha256) dependenciesMatch = false; }
        catch { dependenciesMatch = false; }
      }
      if (dependenciesMatch) reviews.push(review);
    }
  }
  const context = createAuditContext(all.sources, reviews);
  context.structure = createAuditStructure(all.sources, all.functions);
  let buildMacros: AuditBuildMacros | undefined;
  if (!values["no-build-macros"]) {
    try { buildMacros = await AuditBuildMacros.open(resolve(repo, values["build-dir"]!)); }
    catch (error) { console.log(`Build macro expansion unavailable: ${String(error)}`); }
  }
  const results = [];
  await mkdir(join(out, "packets"), { recursive: true });
  for (const fn of selected) {
    const matches = matchFunctions(fn, originals);
    const original = matches.length === 1 ? entries.find(entry => entry.function.entry === matches[0].entry) : undefined;
    const macroStates = original?.c && buildMacros ? await buildMacros.forFile(resolve(repo, fn.path), selected.filter(entry => entry.path === fn.path).map(entry => entry.line)) : undefined;
    const comparison = original?.c ? compareFunction(fn, original, all.functions, { ...context, macros: macroStates?.get(fn.line) }) : undefined;
    const status = matches.length === 0 ? "unmatched" : matches.length > 1 ? "ambiguous" : !original?.c ? "decompile_failed" : !comparison?.compared ? "parse_error" : comparison.findings.length ? "candidate" : comparison.reviewedFindings.length ? "reviewed" : "no_findings";
    const result = { id: fn.id, name: fn.name, path: fn.path, line: fn.line, status, address: original?.function.entry, findings: comparison?.findings ?? [], reviewedFindings: comparison?.reviewedFindings ?? [], normalizations: comparison?.normalizations ?? [], error: original?.error };
    results.push(result);
    if (comparison && original) await atomicJson(join(out, "packets", `${fn.id}.json`), {
      schema: 1, program: identity.program, result, source: fn.source,
      expandedSource: comparison.expanded.source, expandedHelpers: comparison.expanded.helpers,
      normalizedSource: comparison.normalized.port, normalizedOriginal: comparison.normalized.ps2,
      original, limitations: "Lexical AST triage, not equivalence. Macros, field layouts, indirect calls, local temporaries, and control-flow rewrites can explain differences. Verify candidates against PS2 disassembly."
    });
  }
  const counts: Record<string, number> = {};
  for (const result of results) counts[result.status] = (counts[result.status] ?? 0) + 1;
  await atomicJson(join(out, "report.json"), { schema: 1, filtersVersion: 5, numericMacroCount: context.constants.size - (context.enumNames?.size ?? 0), numericEnumCount: context.enumNames?.size ?? 0, buildMacros: buildMacros?.describe(repo) ?? null, program: identity.program, counts, results });
  const reports = renderAuditReports(results, counts);
  await writeFile(join(out, "report.md"), reports.report);
  await writeFile(join(out, "coverage.md"), reports.coverage);
  await writeFile(join(out, "reviewed.md"), reports.reviewed);
  console.log(`Report: ${join(out, "report.md")}\n${JSON.stringify(counts)}`);
}

main().catch(error => { console.error(error); process.exitCode = 1; });
