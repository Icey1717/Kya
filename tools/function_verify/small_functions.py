"""Batch compiled-code survey of short functions. Unsupported cases fail closed.

Inferred single-field correspondences are hypotheses, explicitly reported as
conditional results. Source text selects identities/types, never behavior.
"""
import argparse
from collections import Counter, defaultdict
import copy
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import sys

from verify import ROOT, Unsupported, execute as ps2_execute
import z3
from windows_pcode import execute as pc_execute
from client import request, export_pcode
from compiled import coff_symbols, constant, lift
from compiled_extended import bundle
from compare_extended import store, word, prove
from paths import expand
from build_config import identity
from clang_discovery import recover_inventory, validate_definition
from ps2_identity import (DEFAULT_MANIFEST, DEFAULT_COHORT, load_manifest, listing,
                          resolve, validate_export, reconcile, metadata as identity_metadata)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def verify_original_bytes(original, elf):
    """Reject patched Ghidra instructions, including lifted delay-slot bytes."""
    if sha(elf) != original['executable_sha256']:
        raise Unsupported('PS2 ELF hash does not match export identity')
    if elf[:6] != b'\x7fELF\x01\x01':
        raise Unsupported('Expected little-endian ELF32 original executable')
    phoff = struct.unpack_from('<I', elf, 28)[0]
    stride, count = struct.unpack_from('<HH', elf, 42)
    segments = [struct.unpack_from('<IIIIIIII', elf, phoff + i * stride) for i in range(count)]
    segments = [segment for segment in segments if segment[0] == 1]
    for instruction in original['instructions']:
        for part in [instruction] + instruction.get('delay_slot_instructions', []):
            if 'bytes' not in part:
                raise Unsupported('Export lacks original instruction-byte provenance')
            raw, address = bytes.fromhex(part['bytes']), int(part['address'], 16)
            matches = [s for s in segments if s[2] <= address and address + len(raw) <= s[2] + s[4]]
            if len(matches) != 1 or not raw:
                raise Unsupported('Instruction is not in one file-backed original ELF segment')
            segment = matches[0]
            offset = segment[1] + address - segment[2]
            if elf[offset:offset+len(raw)] != raw:
                raise Unsupported('Ghidra instruction bytes differ from original ELF at ' + part['address'])


SCALARS = {'void': 0, 'bool': 8, 'char': 8, 'byte': 8, 'uchar': 8,
           'short': 16, 'ushort': 16, 'int': 32, 'uint': 32, 'ulong': 64,
           'undefined4': 32, 'undefined8': 64}


def validate_compiler_identity(fn):
    if not fn.get('parseError', False) and fn.get('discovery_result') and 'compiler_definition' not in fn:
        raise Unsupported('Compiler identity unavailable: ' + fn['discovery_result'])


def signature(fn):
    """Conservative discovery grammar; compiler output supplies semantics."""
    validate_compiler_identity(fn)
    if 'compiler_definition' in fn:
        try:
            validate_definition(fn['compiler_definition'])
        except ValueError as error:
            raise Unsupported(str(error)) from error
    if fn['parseError']:
        if 'compiler_definition' not in fn:
            raise Unsupported('Source parser error: ' + fn.get('discovery_result', 'no compiler recovery'))
    match = re.fullmatch(r'(?:(\w+)::)?(\w+)', fn['name'])
    if not match:
        raise Unsupported('Requires an unambiguous non-template function identity')
    owner, method = match.groups()
    declaration = fn['source'].split('{', 1)[0]
    qualifier = '(?:' + re.escape(owner) + '::)?' if owner else ''
    sig = re.search(r'\b(' + '|'.join(SCALARS) + r')\s+' + qualifier +
                    re.escape(method) + r'\s*\(([^()]*)\)\s*(?:const\s*)?$', declaration.strip())
    if not sig or re.search(r'\bstatic\b', declaration):
        raise Unsupported('Return type, static ABI, or declaration outside scalar subset')
    parameters = [] if sig[2].strip() in ('', 'void') else sig[2].split(',')
    arguments = []
    for parameter in parameters[:2]:
        typ = re.match(r'\s*(?:const\s+)?(\w+)', parameter)
        if '*' in parameter or '&' in parameter:
            arguments.append('pointer31')
        elif not typ or typ[1] not in SCALARS or typ[1] == 'void':
            # Unused unsupported parameters may still be harmless, but leaving
            # their registers unknown is mandatory if execution reaches them.
            arguments.append(None)
        else:
            arguments.append(typ[1])
    fields = sorted(set(re.findall(r'\bthis->(\w+)\b(?!\s*\()', fn['body'])))
    return dict(owner=owner, method=method, bits=SCALARS[sig[1]], arguments=arguments,
                fields=fields, return_type=sig[1])


def arguments(types):
    result, inputs = [], []
    for i, typ in enumerate(types):
        if typ is None:
            result.append(None)
            continue
        if typ == 'pointer31':
            value = z3.BitVec('argument' + str(i), 31)
            inputs.append(('argument' + str(i), value))
            result.append(z3.ZeroExt(33, value))
            continue
        bits = 1 if typ == 'bool' else SCALARS[typ]
        value = z3.BitVec('argument' + str(i), bits)
        inputs.append(('argument' + str(i), value))
        result.append(value if bits == 64 else
                      (z3.ZeroExt(64 - bits, value) if typ in ('bool', 'uint', 'ushort', 'uchar', 'byte')
                       else z3.SignExt(64 - bits, value)))
    return (result + [None, None])[:2], inputs


def emitted_candidates(fn, prefix, symbols):
    """Compiler-recovered definitions must use their exact emitted identity."""
    validate_compiler_identity(fn)
    compiler = fn.get('compiler_definition')
    if compiler is not None and not compiler['mangled']:
        raise Unsupported('Compiler definition has no emitted-symbol identity (generic template)')
    candidates = {symbol: targets for symbol, targets in symbols.items() if
                  (symbol == compiler['mangled'] if compiler is not None else
                   symbol.startswith(prefix) or symbol == fn['name'])}
    found = [(symbol, target) for symbol, targets in candidates.items() for target in targets
             if target.endswith(fn['path'] + '.obj')]
    if not found and Path(fn['path']).suffix in ('.h', '.hpp', '.inl'):
        found = [(symbol, targets[0]) for symbol, targets in candidates.items()]
    return found


def outcomes(pc, ps2, pm, sm, args, ptrace=None, strace=None, field_regions=None, global_function=False):
    pa, sa = field_regions or ([(0x1000, 0x6000)], [(0x1000, 0x6000)])
    pc_paths, ps2_paths = expand(pc), expand(ps2)
    options = dict(path_mode=True, precondition=z3.BoolVal(True))
    first, second = args
    pc_options, ps2_options = {}, {}
    if global_function:
        pc_options['this_argument'] = first
        ps2_options.update(this_argument=first, loop_mode=True)
        first, second = second, None
    po = [pc_execute(p, first, pm, second_argument=second, regions=pa, **pc_options,
                     access_trace=ptrace, **options) for p in pc_paths]
    so = [ps2_execute(p, first, sm, second_argument=second, regions=sa, **ps2_options,
                      access_trace=strace, **options) for p in ps2_paths]
    po = [p for p in po if p is not None]
    prove(z3.Or(*[p[3] for p in po]), z3.BoolVal(True), 'Windows path coverage not proven')
    prove(z3.Or(*[p[2] for p in so]), z3.BoolVal(True), 'PS2 path coverage not proven')
    return po, so


def object_bytes(trace):
    touched = set()
    for address, size in trace:
        if not z3.is_bv_value(address):
            raise Unsupported('Symbolic address requires an explicit pointer/array domain')
        address = address.as_long()
        if 0x1000 <= address < 0x7000:
            touched.update(range(address, address + size))
    return touched


def compare(pc, ps2, sig, layout=None, *, reviewed_mapping=None):
    args, inputs = arguments(sig['arguments'])
    pm = z3.Array('pc_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    sm = z3.Array('ps2_memory', z3.BitVecSort(32), z3.BitVecSort(8))
    pt, st = [], []
    global_function = 'owner' in sig and sig['owner'] is None
    outcomes(pc, ps2, pm, sm, args, pt, st, global_function=global_function)
    pb, sb = object_bytes(pt), object_bytes(st)
    mapping = None
    independent = bool(pb) != bool(sb)
    if (pb or sb) and not independent:
        if not layout or len(sig['fields']) != 1 or not layout['integral'] or layout.get('boolean'):
            raise Unsupported('Object access requires a reviewed mapping or a single integral source field')
        size, po = layout['size'], 0x1000 + layout['offset']
        if size not in (1, 2, 4, 8) or pb != set(range(po, po + size)):
            raise Unsupported('Windows accesses do not match the compiler-probed integral field')
        if not sb or len(sb) != size or sb != set(range(min(sb), min(sb) + size)):
            raise Unsupported('PS2 accesses do not identify a single field of matching width')
        so = min(sb)
        field = z3.BitVec('initial_field', size * 8)
        inputs.append(('initial_field', field))
        pm, sm = store(pm, po, field), store(sm, so, field)
        mapping = dict(field=sig['fields'][0], windows_offset=po-0x1000,
                       ps2_offset=so-0x1000, size=size,
                       status='inferred correspondence; requires review against PS2 field identity')
        if reviewed_mapping is not None:
            if (reviewed_mapping.get('function') != ps2['name'] or
                    reviewed_mapping.get('ps2_entry') != ps2['entry'] or
                    not reviewed_mapping.get('evidence') or
                    any(reviewed_mapping.get(key) != mapping[key] for key in ('field', 'ps2_offset', 'size'))):
                raise Unsupported('Reviewed field identity does not match the exported function/accesses')
            mapping.update(status='reviewed', evidence=reviewed_mapping['evidence'])
    # Repeat with narrow approved regions. Discovery cannot authorize hidden
    # extra accesses introduced by the related symbolic input state.
    regions = ([(po, size)], [(so, size)]) if mapping else (
        ([(min(pb), max(pb)-min(pb)+1)] if pb else []),
        ([(min(sb), max(sb)-min(sb)+1)] if sb else []))
    po_out, so_out = outcomes(pc, ps2, pm, sm, args, field_regions=regions, global_function=global_function)
    pa, sa = z3.BitVec('pc_witness', 64), z3.BitVec('ps2_witness', 32)
    pc_observed = z3.Or(z3.ULT(pa, 0x7e00), z3.UGE(pa, 0x8000))
    ps2_observed = z3.BoolVal(True)
    if mapping:
        pc_observed = z3.And(pc_observed, z3.Or(z3.ULT(pa, po), z3.UGE(pa, po + size)))
        ps2_observed = z3.Or(z3.ULT(sa, so), z3.UGE(sa, so + size))
    differences, observations = [], []
    for pv, pf, baseline, pg in po_out:
        for sv, sf, sg in so_out:
            diff = [z3.And(pc_observed, pf[pa] != baseline[pa]),
                    z3.And(ps2_observed, sf[sa] != sm[sa])]
            values = []
            if sig['bits']:
                bits = sig['bits']
                if pv is None or sv is None or min(pv.size(), sv.size()) < bits:
                    raise Unsupported('Missing scalar return')
                pr, sr = z3.Extract(bits-1, 0, pv), z3.Extract(bits-1, 0, sv)
                diff.append(pr != sr)
                values.extend([('windows_return', pr), ('ps2_return', sr)])
            if mapping:
                pfield = pf[z3.BitVecVal(po, 64)] if size == 1 else word(pf, po, size)
                sfield = sf[z3.BitVecVal(so, 32)] if size == 1 else word(sf, so, size)
                diff.append(pfield != sfield)
                values.extend([('windows_field', pfield), ('ps2_field', sfield)])
            differences.append(z3.And(pg, sg, z3.Or(*diff)))
            observations.append((z3.And(pg, sg), values))
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(z3.Or(*differences))
    verdict = solver.check()
    conditional = mapping is not None and mapping['status'] != 'reviewed'
    result = dict(result={'sat': 'mismatch', 'unsat': 'conditional-equivalent' if conditional else 'equivalent',
                          'unknown': 'inconclusive'}[str(verdict)],
                  windows_paths=len(po_out), ps2_paths=len(so_out), mapping=mapping,
                  domain='All declared scalar input bits (bool 0/1); pointer arguments restricted to 31-bit identities; '
                         'all initial integral field bits; valid fixed object/private stack')
    if independent:
        result['observation'] = ('Only one implementation accesses object memory; compare returns and '
                                 'require that implementation to preserve every object byte')
    if verdict == z3.sat:
        model = solver.model()
        values = list(inputs)
        for guard, observed in observations:
            if z3.is_true(model.eval(guard, model_completion=True)):
                values.extend(observed)
                break
        result['counterexample'] = {key: hex(model.eval(value, model_completion=True).as_long()) for key, value in values}
        for pv, pf, baseline, pg in po_out:
            if z3.is_true(model.eval(z3.And(pg, pc_observed, pf[pa] != baseline[pa]), model_completion=True)):
                result['counterexample'].update({key: hex(model.eval(value, model_completion=True).as_long())
                    for key, value in [('windows_address', pa), ('windows_before', baseline[pa]), ('windows_after', pf[pa])]})
                break
        for sv, sf, sg in so_out:
            if z3.is_true(model.eval(z3.And(sg, ps2_observed, sf[sa] != sm[sa]), model_completion=True)):
                result['counterexample'].update({key: hex(model.eval(value, model_completion=True).as_long())
                    for key, value in [('ps2_address', sa), ('ps2_before', sm[sa]), ('ps2_after', sf[sa])]})
                break
    elif verdict == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result


def write_report(output, rows, metadata):
    counts = dict(Counter(r['result'] for r in rows))
    document = dict(**metadata, counts=counts, functions=rows)
    (output / 'report.json').write_text(json.dumps(document, indent=2) + '\n')
    lines = ['# Short-function compiled verification survey', '',
             'Only `equivalent` rows have no inferred field correspondence. '
             '`conditional-equivalent` rows require review of the recorded single-field mapping. '
             'Unsupported functions have no equivalence verdict.', '',
             f"Selected {len(rows)} functions with fewer than {metadata['line_limit']} physical source lines (signature and braces included).", '',
             '| Result | Count |', '| --- | ---: |']
    lines.extend(f'| {key} | {value} |' for key, value in sorted(counts.items()))
    lines.extend(['', '## Mismatches', ''])
    for row in rows:
        if row['result'] == 'mismatch':
            lines.extend([f"- `{row['name']}` (`{row['ps2_entry']}`), {row['path']}:{row['line']}: " +
                          json.dumps(row.get('counterexample', {}))])
    lines.extend(['', '## Unsupported reasons', ''])
    lines.extend(f'- {count}: {reason}' for reason, count in Counter(
        r.get('reason', '') for r in rows if r['result'] == 'unsupported').most_common())
    (output / 'report.md').write_text('\n'.join(lines) + '\n')
    return counts


def exit_status(rows):
    if any(row['result'] == 'mismatch' for row in rows):
        return 1
    return 0 if rows and all(row['result'] == 'equivalent' for row in rows) else 2


def compare_prepared(output, elf=None, identity_manifest=DEFAULT_MANIFEST, port=18437):
    """Replay prepared exports, rejecting changed objects or source bodies."""
    document = json.loads((output / 'report.json').read_text())
    manifest = load_manifest(identity_manifest)
    manifest_hash = sha(Path(identity_manifest).read_bytes())
    if document.get('identity_manifest_sha256', manifest_hash) != manifest_hash:
        raise Unsupported('Identity manifest changed; rebuild the survey')
    inventory_path = output / ('recovered-inventory.json' if document.get('clang_fallback') else 'inventory.json')
    inventory = {fn['id']: fn for fn in json.loads(inventory_path.read_text())['functions']}
    layouts = json.loads((output / 'layouts.json').read_text())
    artifacts, object_hashes, sources = {}, {}, {}
    rows = document.pop('functions')
    document.pop('counts')
    if identity(Path(document['build']['build_directory'])) != document['build']:
        raise Unsupported('Build configuration changed; rebuild the survey')
    for row in rows:
        try:
            validate_compiler_identity(inventory[row['id']])
        except Unsupported as error:
            # A stricter discovery gate can invalidate an older prepared row.
            # Keep source provenance, but do not retain its former proof claim.
            retained = {key: value for key, value in row.items() if key in
                        ('id', 'name', 'path', 'line', 'lines', 'source_sha256',
                         'parse_diagnostics', 'compiler_definition')}
            row.clear()
            row.update(retained)
            row.update(result='unsupported', reason=str(error))
            continue
        diagnostic = row.get('ps2_identity', {})
        if diagnostic.get('review_id'):
            fn = inventory[row['id']]
            source = (ROOT / fn['path']).read_bytes().decode('utf-8', errors='replace').replace('\r\n', '\n')
            if fn['source'].replace('\r\n', '\n') not in source:
                raise Unsupported('Reviewed source body changed; rebuild the survey')
            review = manifest.validate(fn, sha(elf) if elf is not None else document.get('ps2_executable_sha256'))
            if review and review['disposition'] == 'mapped':
                instances = review.get('ps2_instances', [review['ps2']])
                live_instances = [identity_metadata(request('get_function', port, address=item['entry']))
                                  for item in instances]
                live = live_instances[instances.index(review['ps2'])]
                if (live != review['ps2'] or diagnostic.get('selected') != live
                        or live_instances != instances
                        or diagnostic.get('selected_instances', [live]) != instances
                        or diagnostic.get('status') != 'reviewed' or row.get('ps2_entry') != live['entry']):
                    raise Unsupported('Reviewed live PS2 identity changed; rebuild the survey')
        if 'artifacts' not in row:
            continue
        path = Path(document['build']['build_directory']) / row['windows_object']
        if path not in object_hashes:
            object_hashes[path] = sha(path.read_bytes())
        if object_hashes[path] != row['object_sha256']:
            raise Unsupported('Production object changed; rebuild the survey: ' + str(path))
        source = ROOT / row['path']
        if source not in sources:
            sources[source] = source.read_bytes().decode('utf-8', errors='replace').replace('\r\n', '\n')
        if inventory[row['id']]['source'].replace('\r\n', '\n') not in sources[source]:
            raise Unsupported('Source body changed; rebuild the survey: ' + str(source))
        folder = ROOT / row['artifacts']
        if folder not in artifacts:
            artifacts[folder] = (json.loads((folder / 'exports.json').read_text())['exports'],
                                 json.loads((folder / 'ps2.json').read_text()))
        exports, originals = artifacts[folder]
        validate_export(inventory[row['id']], originals[row['ps2_entry']], diagnostic,
                        manifest, sha(elf) if elf is not None else document.get('ps2_executable_sha256'),
                        lambda entry: request('get_function', port, address=entry))
        pc = next(e for e in exports if e['coff']['symbol'] == row['windows_symbol'])
        row['signature'] = signature(inventory[row['id']])
        try:
            row.update(compare(pc, originals[row['ps2_entry']], row['signature'], layouts.get(row['id'])))
            row.pop('reason', None)
        except (ValueError, z3.Z3Exception) as error:
            row.update(result='unsupported', reason=str(error))
    document['replayed_prepared_exports'] = True
    document['survey_status'] = 'complete'
    if elf is not None:
        checked = {}
        for row in rows:
            if 'artifacts' not in row:
                continue
            original = artifacts[ROOT / row['artifacts']][1][row['ps2_entry']]
            entry = row['ps2_entry']
            if entry not in checked:
                try:
                    verify_original_bytes(original, elf)
                    checked[entry] = None
                except (ValueError, struct.error) as error:
                    checked[entry] = str(error)
            if checked[entry] is not None:
                row.update(result='unsupported', reason=checked[entry])
            else:
                row['original_instruction_bytes_verified'] = True
        document['original_elf_sha256'] = sha(elf)
    if DEFAULT_COHORT.exists():
        document['identity_baseline'] = reconcile(json.loads(DEFAULT_COHORT.read_text()), rows)
    print(write_report(output, rows, document), flush=True)
    return exit_status(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/verification/small-functions')
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'out/build/x64-verify')
    parser.add_argument('--line-limit', type=int, default=20, help='Exclusive physical source line limit')
    parser.add_argument('--filter', default='', help='Regular expression selecting qualified function names')
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--inventory-only', action='store_true')
    parser.add_argument('--identities-only', action='store_true',
                        help='Refresh live identities and cohort accounting without building/lifting')
    parser.add_argument('--reuse-inventory', action='store_true')
    parser.add_argument('--no-clang-fallback', action='store_true', help='Keep Tree-sitter failures unsupported without compiler discovery')
    parser.add_argument('--compare-prepared', action='store_true', help='Replay retained exports, checking source/object freshness; no new live export')
    parser.add_argument('--ps2-elf', type=Path, help='Verify every lifted PS2 instruction and delay slot against this original ELF')
    parser.add_argument('--identity-manifest', type=Path, default=DEFAULT_MANIFEST,
                        help='Reviewed PS2 mappings and triage dispositions')
    parser.add_argument('--ninja', default='C:/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe')
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    args = parser.parse_args()
    output, build = args.output.resolve(), args.build_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    elf = args.ps2_elf.read_bytes() if args.ps2_elf else None
    if args.compare_prepared:
        return compare_prepared(output, elf, args.identity_manifest, args.port)
    manifest = load_manifest(args.identity_manifest)
    (output / 'report.json').unlink(missing_ok=True)
    (output / 'report.md').unlink(missing_ok=True)
    if not args.reuse_inventory:
        subprocess.run(['node', '--import', (ROOT / 'tools/local_renamer/node_modules/tsx/dist/loader.mjs').as_uri(),
                        str(Path(__file__).with_name('inventory.mts')), str(ROOT / 'src'), str(output / 'inventory.json')], cwd=ROOT, check=True)
    inventory = json.loads((output / 'inventory.json').read_text())
    if not args.no_clang_fallback:
        # Always refresh compiler evidence with current production flags and
        # includes, even when reusing the fast Tree-sitter inventory.
        (output / 'clang.json').unlink(missing_ok=True)
        discovery = subprocess.run([sys.executable, str(Path(__file__).with_name('clang_discovery.py')),
            '--inventory', str(output / 'inventory.json'), '--output', str(output / 'clang.json'),
            '--build-dir', str(build), '--ninja', args.ninja, '--line-limit', str(args.line_limit),
            '--identities'], cwd=ROOT)
        if discovery.returncode not in (0, 1) or not (output / 'clang.json').exists():
            raise Unsupported('Compiler discovery failed; see diagnostics')
        evidence = json.loads((output / 'clang.json').read_text())
        inventory = recover_inventory(inventory, evidence)
        (output / 'recovered-inventory.json').write_text(json.dumps(inventory, indent=2) + '\n')
    sources = {}
    rows = []
    selected = []
    for fn in inventory['functions']:
        if args.filter and not re.search(args.filter, fn['name']):
            continue
        if fn['lines'] >= args.line_limit:
            continue
        path = ROOT / fn['path']
        if path not in sources:
            sources[path] = path.read_bytes().decode('utf-8', errors='replace').replace('\r\n', '\n')
        if fn['source'].replace('\r\n', '\n') not in sources[path]:
            raise Unsupported('Inventory source body changed; regenerate inventory: ' + str(path))
        row = {key: fn[key] for key in ('id', 'name', 'path', 'line', 'lines')}
        if fn.get('parseDiagnostics'):
            row['parse_diagnostics'] = fn['parseDiagnostics']
        row.update(result='unsupported', source_sha256=sha(fn['source'].encode()))
        if 'compiler_definition' in fn:
            row['compiler_definition'] = fn['compiler_definition']
            row['source_sha256'] = fn['compiler_definition']['source_sha256']
        rows.append(row)
        try:
            sig = signature(fn)
            row['signature'] = sig
            selected.append((fn, row, sig))
        except Unsupported as error:
            row['reason'] = str(error)
    metadata = dict(scope='Compiled Windows COFF versus live original PS2 raw P-code; survey, not whole-codebase proof',
                    survey_status='running',
                    line_limit=args.line_limit, inventoried_functions=len(inventory['functions']),
                    name_filter=args.filter,
                    build=identity(build), solver=z3.get_version_string())
    metadata['identity_manifest_sha256'] = sha(args.identity_manifest.read_bytes())
    metadata['excluded_declarations'] = inventory.get('excluded_declarations', [])
    metadata['clang_fallback'] = not args.no_clang_fallback
    if elf is not None:
        metadata['original_elf_sha256'] = sha(elf)
    if args.inventory_only:
        metadata['survey_status'] = 'inventory-only'
        for _, row, _ in selected:
            row['reason'] = 'Inventory only; execution not attempted'
        print(write_report(output, rows, metadata))
        return 0
    metadata['program'] = request('status', args.port)
    names = {}
    print(f'{len(rows)} short functions; {len(selected)} scalar function candidates', flush=True)
    for method in sorted({sig['method'] for _, _, sig in selected}):
        try:
            names[method] = listing(request('list_functions', args.port, filter=method, limit=1000))
        except ValueError as error:
            raise Unsupported(str(error) + ': ' + method) from error
    (output / 'ps2-identities.json').write_text(json.dumps(names, indent=2))
    # Hash a live entry rather than relying on an address whose review might be
    # stale. This establishes executable identity only; instruction-byte checking
    # still requires the local --ps2-elf.
    sample = next((f['entry'] for group in names.values() for f in group), None)
    if sample is None:
        sample = next((r['ps2']['entry'] for r in manifest.records.values() if r['disposition'] == 'mapped'), None)
    identity_hash = sha(elf) if elf is not None else (
        request('export_pcode', args.port, address=sample, limit=65536)['executable_sha256'] if sample else None)
    if identity_hash:
        metadata['ps2_executable_sha256'] = identity_hash
    matched = []
    for fn, row, sig in selected:
        match, diagnostic = resolve(fn, names[sig['method']], manifest,
                                    identity_hash,
                                    lambda entry: request('get_function', args.port, address=entry))
        row['ps2_identity'] = diagnostic
        if match is None:
            row['reason'] = {'missing': 'Missing live PS2 identity',
                             'ambiguous': 'Ambiguous live PS2 identity',
                             'stale': 'Stale or invalid reviewed PS2 identity',
                             'non-standalone': 'Reviewed: no standalone PS2 counterpart'}[diagnostic['status']]
            continue
        row['ps2_entry'] = match['entry']
        row['ps2_signature'] = match['signature']
        matched.append((fn, row, sig))
    if args.identities_only:
        for _, row, _ in matched:
            row['reason'] = 'Identity resolved; compiled comparison not attempted'
        metadata['survey_status'] = 'identities-only'
        metadata['identity_baseline'] = reconcile(json.loads(DEFAULT_COHORT.read_text()), rows)
        print(write_report(output, rows, metadata), flush=True)
        return 2
    targets = re.findall(r'^build (CMakeFiles/Kya\.dir/src/[^:]+\.obj):', (build / 'build.ninja').read_text(), re.M)
    print(f'{len(matched)} live PS2 matches; rebuilding {len(targets)} production objects', flush=True)
    with (output / 'build.log').open('w') as log:
        subprocess.run([args.ninja, '-C', str(build), *targets], stdout=log, stderr=subprocess.STDOUT, check=True)
    objects = {target: (build / target).read_bytes() for target in targets}
    symbols = defaultdict(list)
    for target, data in objects.items():
        for symbol, _, section, kind in coff_symbols(data)[0]:
            if section > 0 and kind == 0x20:
                symbols[symbol].append(target)
    prepared = []
    for fn, row, sig in matched:
        prefix = '?' + sig['method'] + ('@' + sig['owner'] if sig['owner'] else '') + '@@'
        try:
            found = emitted_candidates(fn, prefix, symbols)
        except Unsupported as error:
            row['reason'] = str(error)
            continue
        if len(found) != 1:
            row['reason'] = 'Missing or ambiguous emitted Windows member symbol'
            continue
        symbol, target = found[0]
        if sig['owner'] and not re.match(r'[QU][A-Z]AA', symbol[len(prefix):]):
            row['reason'] = 'Unsupported Windows member calling convention'
            continue
        if not sig['owner'] and symbol != fn['name'] and not symbol[len(prefix):].startswith('YA'):
            row['reason'] = 'Unsupported Windows global calling convention'
            continue
        row.update(windows_symbol=symbol, windows_object=target, object_sha256=sha(objects[target]))
        try:
            name = 'F' + str(len(prepared))
            blob, functions, _ = bundle({target: objects[target]}, [(target, row['ps2_entry'], name, symbol)], {})
            original = export_pcode(row['ps2_entry'], args.port)
            validate_export(fn, original, row['ps2_identity'], manifest,
                            identity_hash,
                            lambda entry: request('get_function', args.port, address=entry))
            if elf is not None:
                verify_original_bytes(original, elf)
                row['original_instruction_bytes_verified'] = True
            row['ps2_executable_sha256'] = original['executable_sha256']
            if metadata.setdefault('ps2_executable_sha256', original['executable_sha256']) != original['executable_sha256']:
                raise Unsupported('PS2 executable changed during survey')
            prepared.append((fn, row, sig, target, blob, functions[0], original))
        except (ValueError, RuntimeError) as error:
            row['reason'] = str(error)
    print(f'{len(prepared)} relocation-approved function slices; probing fields', flush=True)
    layouts = {}
    groups = defaultdict(list)
    for item in prepared:
        if len(item[2]['fields']) == 1:
            groups[item[3]].append(item)
    for index, (target, group) in enumerate(groups.items()):
        folder = output / 'layout' / str(index)
        folder.mkdir(parents=True, exist_ok=True)
        source = ROOT / target.removeprefix('CMakeFiles/Kya.dir/').removesuffix('.obj')
        probe, obj = folder / 'probe.cpp', folder / 'probe.obj'
        code = '#include "' + source.as_posix() + '"\nextern "C" {\n'
        for _, row, sig, *_ in group:
            name = row['id']
            field = sig['owner'] + ', ' + sig['fields'][0]
            expr = '((' + sig['owner'] + '*)0)->' + sig['fields'][0]
            code += f'extern const unsigned int o{name} = __builtin_offsetof({field});\n'
            code += f'extern const unsigned int s{name} = sizeof({expr});\n'
            code += f'extern const unsigned int i{name} = __is_integral(decltype({expr}));\n'
            code += f'extern const unsigned int b{name} = __is_same(decltype({expr}), bool);\n'
        probe.write_text(code + '}\n')
        command = subprocess.check_output([args.ninja, '-C', str(build), '-t', 'commands', target], text=True).splitlines()[-1]
        command = command.replace(source.as_posix(), probe.as_posix())
        command = command.replace(target.replace('/', '\\') + '.d', (folder / 'probe.d').as_posix())
        command = command.replace(target, obj.as_posix())
        with (folder / 'compile.log').open('w') as log:
            compiled = subprocess.run(command, cwd=build, stdout=log, stderr=subprocess.STDOUT)
        if compiled.returncode == 0:
            data = obj.read_bytes()
            for _, row, *_ in group:
                name = row['id']
                layouts[name] = dict(offset=constant(data, 'o' + name), size=constant(data, 's' + name),
                                     integral=bool(constant(data, 'i' + name)), boolean=bool(constant(data, 'b' + name)),
                                     probe_object_sha256=sha(data))
    (output / 'layouts.json').write_text(json.dumps(layouts, indent=2))
    for batch_start in range(0, len(prepared), 200):
        group = prepared[batch_start:batch_start+200]
        folder = output / ('batch-' + str(batch_start // 200))
        folder.mkdir(parents=True, exist_ok=True)
        blob, functions = bytearray(), []
        originals = {}
        for _, row, _, _, body, function, original in group:
            while len(blob) % 16:
                blob.append(0xcc)
            function = copy.deepcopy(function)
            function['blob_offset'] += len(blob)
            # Each slice is independently relocated. Internal relative targets
            # retain their displacement when moving the complete slice.
            blob.extend(body)
            functions.append(function)
            originals[row['ps2_entry']] = original
        (folder / 'functions.bin').write_bytes(blob)
        (folder / 'manifest.json').write_text(json.dumps(dict(functions=functions), indent=2))
        (folder / 'ps2.json').write_text(json.dumps(originals, indent=2))
        lift(folder, args.ghidra, args.java)
        exports = json.loads((folder / 'exports.json').read_text())['exports']
        if [pc['coff']['symbol'] for pc in exports] != [item[5]['symbol'] for item in group]:
            raise Unsupported('Missing, reordered, or unexpected Windows exports')
        for item, pc in zip(group, exports):
            _, row, sig, _, _, function, original = item
            row['artifacts'] = str(folder.relative_to(ROOT)).replace('\\', '/')
            try:
                row.update(compare(pc, original, sig, layouts.get(row['id'])))
                row.pop('reason', None)
            except (ValueError, z3.Z3Exception) as error:
                row.update(result='unsupported', reason=str(error))
        print(f'Compared {min(batch_start+200, len(prepared))}/{len(prepared)}: ' + str(write_report(output, rows, metadata)), flush=True)
    metadata['survey_status'] = 'complete'
    if DEFAULT_COHORT.exists():
        metadata['identity_baseline'] = reconcile(json.loads(DEFAULT_COHORT.read_text()), rows)
    print(write_report(output, rows, metadata), flush=True)
    return exit_status(rows)


if __name__ == '__main__':
    sys.exit(main())
