// Run with tools/local_renamer's installed tsx. Parsing is discovery only;
// machine code, not these source bodies, supplies comparison semantics.
import { readdir, readFile, writeFile, mkdir } from 'node:fs/promises';
import { resolve, relative, join, dirname } from 'node:path';
import { extractFunctions } from '../local_renamer/src/functionAudit.js';

const root = resolve(process.argv[2] ?? 'src');
const output = resolve(process.argv[3] ?? 'out/verification/small-functions/inventory.json');
const functions: Array<ReturnType<typeof extractFunctions>[number] & { lines: number }> = [];
let files = 0;
async function visit(path: string) {
  for (const entry of (await readdir(path, { withFileTypes: true })).sort((a, b) => a.name.localeCompare(b.name))) {
    if (entry.name.startsWith('.')) continue;
    const child = join(path, entry.name);
    if (entry.isDirectory()) await visit(child);
    else if (/\.(cpp|c|cc|cxx|h|hpp|inl)$/i.test(entry.name)) {
      files++;
      functions.push(...extractFunctions(relative(process.cwd(), child).replaceAll('\\', '/'), await readFile(child, 'utf8'), { diagnostics: true })
        .map(fn => ({ ...fn, lines: fn.source.split('\n').length })));
    }
  }
}
await visit(root);
await mkdir(dirname(output), { recursive: true });
await writeFile(output, JSON.stringify({ root, files, functions }, null, 2) + '\n');
console.log(`${functions.length} functions in ${files} files -> ${output}`);
