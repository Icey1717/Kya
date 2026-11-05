"""Compare real Windows compiler output with live PS2 P-code, without source behavior models."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
from verify import ROOT, Unsupported, execute as execute_ps2, request
import z3
from windows_pcode import execute as execute_windows


def compare(pc, ps2, flags_offset):
    kind = z3.BitVec('kind', 64)
    field = z3.BitVec('initial_flags', 32)
    memory = z3.Array('pc_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    ps2_memory = z3.Array('ps2_memory', z3.BitVecSort(32), z3.BitVecSort(8))
    for byte in range(4):
        value = z3.Extract(byte * 8 + 7, byte * 8, field)
        memory = z3.Store(memory, z3.BitVecVal(0x1000 + flags_offset + byte, 64), value)
        ps2_memory = z3.Store(ps2_memory, z3.BitVecVal(8 + byte, 32), value)
    pc_return, pc_final, pc_baseline = execute_windows(pc, kind, memory)
    name = pc['name']
    if name == 'IsKindOfObject':
        ps2_return = execute_ps2(ps2, kind)
        ps2_final = ps2_memory
        mismatch = z3.Extract(7, 0, pc_return) != z3.Extract(7, 0, ps2_return)
    else:
        ps2_return, ps2_final = execute_ps2(ps2, None, ps2_memory)
        mismatch = z3.BoolVal(False)
        if name == 'IsLookingAt':
            mismatch = z3.Extract(31, 0, pc_return) != z3.Extract(31, 0, ps2_return)
    def word(array, offset, bits):
        return z3.Concat(*[array[z3.BitVecVal(offset + i, bits)] for i in (3, 2, 1, 0)])
    pc_flags = word(pc_final, 0x1000 + flags_offset, 64)
    ps2_flags = word(ps2_final, 8, 32)
    witness_pc = z3.BitVec('pc_address', 64)
    witness_ps2 = z3.BitVec('ps2_offset', 32)
    # Observe all memory except private Windows stack and the corresponding
    # field bytes, which are compared across architectures separately.
    pc_observed = z3.And(z3.Or(z3.ULT(witness_pc, 0x7fc0), z3.UGE(witness_pc, 0x8000)),
                         z3.Or(z3.ULT(witness_pc, 0x1000 + flags_offset),
                               z3.UGE(witness_pc, 0x1000 + flags_offset + 4)))
    ps2_observed = z3.Or(z3.ULT(witness_ps2, 8), z3.UGE(witness_ps2, 12))
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(z3.Or(mismatch, pc_flags != ps2_flags,
                     z3.And(pc_observed, pc_final[witness_pc] != pc_baseline[witness_pc]),
                     z3.And(ps2_observed, ps2_final[witness_ps2] != ps2_memory[witness_ps2])))
    answer = solver.check()
    report = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(answer)])
    if answer == z3.sat:
        model = solver.model()
        report['counterexample'] = {name: hex(model.eval(value, model_completion=True).as_long())
                                    for name, value in [('kind', kind), ('flags', field),
                                                        ('pc_flags', pc_flags), ('ps2_flags', ps2_flags)]}
        if name in ('IsKindOfObject', 'IsLookingAt'):
            bits = 8 if name == 'IsKindOfObject' else 32
            report['counterexample']['pc_return'] = hex(model.eval(z3.Extract(bits - 1, 0, pc_return), model_completion=True).as_long())
            report['counterexample']['ps2_return'] = hex(model.eval(z3.Extract(bits - 1, 0, ps2_return), model_completion=True).as_long())
    elif answer == z3.unknown:
        report['reason'] = solver.reason_unknown()
    return report


def run(path, port=18437):
    manifest = json.loads(path.read_text())
    def digest(path):
        return hashlib.sha256(path.read_bytes()).hexdigest()
    if digest(Path(manifest['object'])) != manifest['object_sha256'] or digest(ROOT / 'src/b-witch/Actor.cpp') != manifest['source_sha256']:
        raise Unsupported('Object/source changed; rebuild and relift with compiled.py')
    if digest(Path(manifest['compiled_source'])) != manifest['compiled_source_sha256']:
        raise Unsupported('Compiled source artifact changed')
    if manifest['compiled_layout']['ulong_size'] != 8:
        raise Unsupported('Unexpected compiled ulong width')
    blob = (path.parent / 'functions.bin').read_bytes()
    reports = []
    for pc in manifest['exports']:
        item = pc['coff']
        start, size = item['blob_offset'], item['size']
        if hashlib.sha256(blob[start:start + size]).hexdigest() != item['bytes_sha256']:
            raise Unsupported('Function bytes changed since lifting')
        ps2 = request('export_pcode', port, address=item['ps2_entry'])
        (path.parent / (item['ps2_entry'] + '.json')).write_text(json.dumps(ps2, indent=2))
        report = dict(function='CActor::' + pc['name'], ps2_entry=item['ps2_entry'],
                      windows_symbol=item['symbol'], windows_function_sha256=item['bytes_sha256'],
                      ps2_executable_sha256=ps2['executable_sha256'])
        if ps2['entry'] != item['ps2_entry'] or ps2['name'] != report['function']:
            raise Unsupported('PS2 function identity mismatch')
        try:
            report.update(compare(pc, ps2, manifest['compiled_layout']['flags_offset']))
        except (Unsupported, z3.Z3Exception, KeyError, ValueError) as error:
            report.update(result='inconclusive', reason=str(error))
        reports.append(report)
    document = dict(build=manifest.get('build'), scope='Production Windows x64 COFF machine code versus original PS2 machine-code P-code',
                    mutation=manifest['mutation'],
                    windows_object_sha256=manifest['object_sha256'], compiler_command=manifest['compile_command'],
                    compiler_version=manifest.get('compiler_version', 'See recorded compiler command'),
                    compiled_layout=manifest['compiled_layout'], solver=z3.get_version_string(),
                    assumptions=['Ghidra x86 and R5900 instruction lifters are correct',
                                 'Relocation-free functions; linker/LTO transformations outside scope',
                                 'Windows flags offset compiler-derived; PS2 flags offset 8 is audited mapping',
                                 'Windows ABI: RCX=this, RDX=kind, AL=bool, EAX=uint; private stack excluded',
                                 'All 64-bit kind values and all 32-bit initial flags; arbitrary other memory'], functions=reports)
    (path.parent / 'report.json').write_text(json.dumps(document, indent=2) + '\n')
    print(json.dumps(document, indent=2))
    return 0 if reports and all(r['result'] == 'equivalent' for r in reports) else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exports', type=Path, default=ROOT / 'out/verification/compiled/exports.json')
    parser.add_argument('--port', type=int, default=18437)
    args = parser.parse_args()
    sys.exit(run(args.exports, args.port))
