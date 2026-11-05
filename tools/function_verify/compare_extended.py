"""Compare compiled control-flow cases against original PS2 P-code paths."""
import argparse
import copy
import hashlib
import json
from pathlib import Path

from verify import ROOT, Unsupported, execute as ps2_execute, request
import z3
from windows_pcode import execute as pc_execute
from paths import expand
from call_cases import link as ps2_link
from compiled_extended import TABLES

IDENTITIES = {'HasChanged': 'CInventoryInterface::HasChanged', 'GetType': 'CActorMoney::GetType',
              'BaseFlags': 'CActor::GetBehaviourFlags', 'AutFlags': 'CActorAutonomous::GetBehaviourFlags',
              'IsInList': 'CFixedTable<CActor,4>::IsInList'}


def windows_call(caller, callee):
    caller, callee = copy.deepcopy(caller), copy.deepcopy(callee)
    calls = [(i, op) for i in caller['instructions'] for op in i['ops'] if op['opcode'] == 'CALL']
    if len(calls) != 1:
        raise Unsupported('Expected exactly one direct Windows call')
    instruction, call = calls[0]
    if call['inputs'] != [dict(space='ram', offset=hex(int(callee['entry'], 16))[2:], size=8, constant=False)]:
        raise Unsupported('Unexpected Windows callee')
    index = caller['instructions'].index(instruction)
    continuation = int(caller['instructions'][index + 1]['address'], 16)
    call['opcode'] = 'BRANCH'
    for instruction in callee['instructions']:
        for index, op in enumerate(instruction['ops']):
            if op['opcode'] == 'CALL':
                raise Unsupported('Nested direct call unsupported')
            if op['opcode'] == 'RETURN':
                instruction['ops'][index:] = [dict(opcode='CHECK_TARGET', inputs=op['inputs'], expected=continuation),
                    dict(opcode='BRANCH', inputs=[dict(space='ram', offset=hex(continuation)[2:], size=8, constant=False)])]
                break
    caller['instructions'] += callee['instructions']
    caller['operation_count'] = sum(len(i['ops']) for i in caller['instructions'])
    return caller


def store(array, offset, value):
    bits = array.domain().size()
    for byte in range(value.size() // 8):
        array = z3.Store(array, z3.BitVecVal(offset + byte, bits), z3.Extract(byte * 8 + 7, byte * 8, value))
    return array


def word(array, offset, size):
    return z3.Concat(*[array[z3.BitVecVal(offset + i, array.domain().size())] for i in reversed(range(size))])


def prove(claim, domain, reason):
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(domain, z3.Not(claim))
    if solver.check() != z3.unsat:
        raise Unsupported(reason)


def compare(manifest, name, originals, *, loop_visits=5):
    pcs = {e['name']: e for e in manifest['exports']}
    pc = pcs[name]
    ps2 = originals[pc['coff']['ps2_entry']]
    layout = manifest['layout']
    if layout['pointer_size'] != 8:
        raise Unsupported('Expected 64-bit Windows pointers')
    pm = z3.Array('windows_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    sm = z3.Array('ps2_memory', z3.BitVecSort(32), z3.BitVecSort(8))
    field, state, query = z3.BitVec('field', 32), z3.BitVec('state', 32), z3.BitVec('query', 32)
    domain, argument = z3.BoolVal(True), z3.BitVecVal(0, 64)
    pc_regions, ps2_options, mapped = [(0x1000, 0x1000)], {}, []
    inputs = [('field', field)]
    if name in ('HasChanged', 'GetType'):
        po, so = (layout['dirty'], 12) if name == 'HasChanged' else (layout['money'], 0x1d0)
        pm, sm = store(pm, 0x1000 + po, field), store(sm, so, field)
        mapped = [(0x1000 + po, so, 4)]
        ps2_options['object_size'] = so + 4
    elif name in ('BaseFlags', 'AutFlags'):
        domain = z3.And(state >= -1, state <= (1 if name == 'BaseFlags' else 2))
        argument = z3.SignExt(32, state)
        inputs = [('state', state)]
        for symbol, (_, length) in TABLES.items():
            po = manifest['addresses'][symbol]
            so = 0x427d80 if '_ACT@' in symbol else 0x428e20
            pc_regions.append((po, length))
            for i in range(length // 4):
                value = z3.BitVec('table_' + str(so + i * 4), 32)
                inputs.append(('table_' + hex(so + i * 4), value))
                pm, sm = store(pm, po + i * 4, value), store(sm, so + i * 4, value)
        ps2_options.update(regions=[(0x427d80, 8), (0x428e20, 4), (0x8000, 16)])
        if name == 'AutFlags':
            pc = windows_call(pc, pcs['BaseFlags'])
            ps2 = ps2_link([originals[a] for a in ['00123c50', '0011e790', '001022b0']])
            ps2_options['call_mode'] = True
    elif name == 'IsInList':
        domain = z3.And(field >= 0, field <= 4)
        argument = z3.ZeroExt(32, query)
        inputs = [('count', field), ('query', query)]
        pm, sm = store(pm, 0x1000 + layout['count'], field), store(sm, 0, field)
        mapped = [(0x1000 + layout['count'], 0, 4)]
        for i in range(4):
            pointer = z3.BitVec('entry_' + str(i), 32)
            inputs.append(('entry_' + str(i), pointer))
            pm = store(pm, 0x1000 + layout['entries'] + i * 8, z3.ZeroExt(32, pointer))
            sm = store(sm, 4 + i * 4, pointer)
        ps2_options.update(object_size=20, loop_mode=True)
    else:
        raise Unsupported('Unknown compiled case')
    outcomes, frontiers = [], []
    for path in expand(pc, max_paths=256, max_instructions=512,
                       loop_visits=loop_visits if name == 'IsInList' else None):
        result = pc_execute(path, argument, pm, path_mode=True, precondition=domain, regions=pc_regions)
        if result is None:
            continue
        if path.get('cutoff'):
            frontiers.append(result[3])
        else:
            outcomes.append(result)
    prove(z3.Not(z3.Or(*frontiers)), domain, 'Reachable or unproven Windows loop cutoff')
    prove(z3.Or(*[r[3] for r in outcomes]), domain, 'Windows path coverage not proven')
    ps2_outcomes, ps2_frontiers = [], []
    for path in expand(ps2, max_paths=128, max_instructions=512, loop_visits=4 if name == 'IsInList' else None):
        arg = z3.SignExt(32, query) if name == 'IsInList' else argument
        result = ps2_execute(path, arg, sm, precondition=domain, path_mode=True, **ps2_options)
        if path.get('cutoff'):
            ps2_frontiers.append(result[2])
        else:
            ps2_outcomes.append(result)
    prove(z3.Not(z3.Or(*ps2_frontiers)), domain, 'Reachable or unproven PS2 loop cutoff')
    prove(z3.Or(*[r[2] for r in ps2_outcomes]), domain, 'PS2 path coverage not proven')
    # Witness any externally visible write, including writes outside mapped fields.
    pa, sa = z3.BitVec('pc_address', 64), z3.BitVec('ps2_address', 32)
    pc_observed = z3.Or(z3.ULT(pa, 0x7e00), z3.UGE(pa, 0x8000))
    ps2_observed = z3.Or(z3.ULT(sa, 0x8000), z3.UGE(sa, 0x8010)) if name == 'AutFlags' else z3.BoolVal(True)
    for po, so, size in mapped:
        pc_observed = z3.And(pc_observed, z3.Or(z3.ULT(pa, po), z3.UGE(pa, po + size)))
        ps2_observed = z3.And(ps2_observed, z3.Or(z3.ULT(sa, so), z3.UGE(sa, so + size)))
    bits = 8 if name in ('HasChanged', 'IsInList') else 32
    differences = []
    for pv, pf, baseline, pg in outcomes:
        for sv, sf, sg in ps2_outcomes:
            # The two architectures partition the same input domain. Avoid
            # feeding impossible cross-products (especially loop exits) and
            # their large byte-array expressions into the final query.
            feasible = z3.Solver()
            feasible.set(timeout=10000)
            feasible.add(domain, pg, sg)
            feasibility = feasible.check()
            if feasibility == z3.unsat:
                continue
            if feasibility != z3.sat:
                raise Unsupported('Cross-architecture path feasibility inconclusive')
            differences.append(z3.And(pg, sg, z3.Or(
                z3.Extract(bits - 1, 0, pv) != z3.Extract(bits - 1, 0, sv),
                *[word(pf, po, size) != word(sf, so, size) for po, so, size in mapped],
                z3.And(pc_observed, pf[pa] != baseline[pa]),
                z3.And(ps2_observed, sf[sa] != sm[sa]))))
    solver = z3.Solver()
    solver.set(timeout=30000)
    solver.add(domain, z3.Or(*differences))
    answer = solver.check()
    report = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(answer)],
                  windows_paths=len(outcomes), ps2_paths=len(ps2_outcomes), coverage='all inputs in documented domain')
    if name == 'IsInList':
        report.update(domain='0 <= count <= 4; pointer identities injectively mapped from arbitrary 32-bit values',
                      windows_loop_visits=loop_visits, cutoff_proof='unreachable on both architectures')
    elif name in ('BaseFlags', 'AutFlags'):
        report['domain'] = '-1 <= state <= ' + ('1' if name == 'BaseFlags' else '2') + '; arbitrary shared uint32 table contents'
    else:
        report['domain'] = 'all uint32 field values'
    if answer == z3.sat:
        model = solver.model()
        report['counterexample'] = {key: hex(model.eval(value, model_completion=True).as_long()) for key, value in inputs}
        for pv, pf, _, pg in outcomes:
            for sv, sf, sg in ps2_outcomes:
                if z3.is_true(model.eval(z3.And(pg, sg), model_completion=True)):
                    report['counterexample'].update(
                        windows_return=hex(model.eval(z3.Extract(bits - 1, 0, pv), model_completion=True).as_long()),
                        ps2_return=hex(model.eval(z3.Extract(bits - 1, 0, sv), model_completion=True).as_long()))
    elif answer == z3.unknown:
        report['reason'] = solver.reason_unknown()
    return report


def run(path, port=18437):
    manifest = json.loads(path.read_text())
    subsystem_suite = manifest.get('suite') == 'subsystems'
    if subsystem_suite:
        from subsystems import MAPPINGS, compare as compare_case
        identities = {name: item[0] for name, item in MAPPINGS.items()}
    elif manifest.get('suite') == 'combat':
        from combat_cases import IDENTITIES as identities, compare as compare_case
    else:
        identities, compare_case = IDENTITIES, compare
    for record in manifest['provenance'].values():
        for key in ['object', 'source']:
            if hashlib.sha256(Path(record[key]).read_bytes()).hexdigest() != record[key + '_sha256']:
                raise Unsupported('Artifact changed; rebuild with compiled_extended.py')
    blob = (path.parent / 'functions.bin').read_bytes()
    originals = {}
    for pc in manifest['exports']:
        item = pc['coff']
        if hashlib.sha256(blob[item['blob_offset']:item['blob_offset'] + item['size']]).hexdigest() != item['bytes_sha256']:
            raise Unsupported('Lifted function bytes changed')
        original = request('export_pcode', port, address=item['ps2_entry'])
        if original['entry'] != item['ps2_entry'] or original['name'] != identities[pc['name']]:
            raise Unsupported('Original function identity mismatch')
        originals[item['ps2_entry']] = original
    if manifest.get('suite', 'control-flow') == 'control-flow':
        originals['0011e790'] = request('export_pcode', port, address='0011e790')
    if len({e['executable_sha256'] for e in originals.values()}) != 1:
        raise Unsupported('Mixed PS2 executable identities')
    (path.parent / 'originals.json').write_text(json.dumps(originals, indent=2))
    reports = []
    for pc in manifest['exports']:
        try:
            result = compare_case(manifest, pc['name'], originals)
        except (Unsupported, z3.Z3Exception, KeyError, ValueError) as error:
            result = dict(result='inconclusive', reason=str(error))
        result.update(function=identities[pc['name']], ps2_entry=pc['coff']['ps2_entry'])
        reports.append(result)
        print(json.dumps(result), flush=True)
    document = dict(build=manifest.get('build'), scope='Production x64 object code versus original PS2 P-code', suite=manifest.get('suite', 'control-flow'),
                    functions=reports, compiled_layout=manifest['layout'], provenance=manifest['provenance'],
                    compiler_version=manifest['compiler_version'], solver=z3.get_version_string(),
                    ps2_executable_sha256=next(iter(originals.values()))['executable_sha256'],
                    assumptions=['Trusted Ghidra x86/R5900 lifters; pre-link code only',
                                 'Allow-listed REL32 relocations resolved in an isolated bundle',
                                 'Assertion paths must be proven unreachable; no calls stubbed as successful',
                                 'Compiler-derived Windows field offsets; audited PS2 layout and global mappings',
                                 'Private Windows stack at 0x7e00..0x7fff and PS2 call stack at 0x8000..0x800f excluded',
                                 'Pointer identities map injectively by zero-extension; no pointer dereferences or faults'])
    if subsystem_suite:
        document['assumptions'] = ['Trusted Ghidra x86/R5900 lifters; pre-link code only',
            'Compiler-derived Windows offsets; audited PS2 offsets',
            'Pointer identities zero-extended from 32 bits; reader cursor fixed at aligned readable address 0x4000',
            'All uint32 payload, initial field, and setter argument values; no fault model',
            'Windows stack 0x7e00..0x7fff private; all other unmapped memory preserved']
    elif manifest.get('suite') == 'combat':
        document['assumptions'] = ['Trusted Ghidra x86/R5900 lifters; pre-link code only',
            'Explicit acyclic direct calls and tail transfers; no recursion or unresolved indirect calls',
            'VU support restricted to decoded full-lane zero move/rotate and proven +0 + +1 in identity initialization',
            'Compiler-probed Windows fields; audited PS2 offsets and zero-extended pointer identities',
            'EndFlip: aligned, non-null, disjoint collision object pointer in 0x4000..0x5000',
            'CollisionReset: arbitrary uint32 initial contact flags; upper bytes preserved',
            'Audio helpers: arbitrary uint32 counter and zero-extended music pointer identity; no pointee dereference',
            'LayerTimeWarper: index 0, valid disjoint layer at 0x4000; raw binary32 copies only, no FP arithmetic approximation',
            'Windows stack 0x7e00..0x7fff and PS2 graph stack 0x7f00..0x800f private',
            'PS2 s0 (128 bits), stack, and return address preserved; Windows frame/stack/return preserved']
    (path.parent / 'report.json').write_text(json.dumps(document, indent=2) + '\n')
    return 0 if reports and all(r['result'] == 'equivalent' for r in reports) else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exports', type=Path, default=ROOT / 'out/verification/compiled-extended/exports.json')
    parser.add_argument('--port', type=int, default=18437)
    args = parser.parse_args()
    raise SystemExit(run(args.exports, args.port))
