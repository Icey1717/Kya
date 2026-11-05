"""Prove the actual Windows fighter-table initializer against original PS2 bytes.

COFF BSS is deliberately not used as initialized configuration. This executes
the production dynamic initializer and all 108 StateConfig constructors.
"""
import json
import copy
from pathlib import Path
import struct
from verify import Unsupported
import z3
from call_graph import link
from memory_relation import load
from windows_pcode import execute


TABLE = '?_gStateCfg_FIG@CActorFighter@@2PAUStateConfig@@A'
INITIALIZER = '??__E?_gStateCfg_FIG@CActorFighter@@2PAUStateConfig@@A@@YAXXZ'
PS2_TABLE = '00434460'  # GetStateCfg 0031dd64..0031dd74, stride 8.


def straight_path(graph):
    """Follow an acyclic, unconditional expanded graph without Python recursion."""
    instructions = graph['instructions']
    indexes = {int(i['address'], 16): index for index, i in enumerate(instructions)}
    selected, visited, index = [], set(), 0
    while index not in visited:
        visited.add(index)
        instruction = copy.deepcopy(instructions[index])
        selected.append(instruction)
        controls = [op for op in instruction['ops'] if op['opcode'] in
                    ('BRANCH', 'CBRANCH', 'CALL', 'CALLIND', 'BRANCHIND', 'RETURN', 'UNRESOLVED_TRANSFER')]
        if len(controls) > 1 or (controls and controls[0] is not instruction['ops'][-1]):
            raise Unsupported('Ambiguous configuration control flow')
        if not controls:
            index += 1
        elif controls[0]['opcode'] == 'RETURN':
            return dict(graph, instructions=selected,
                        operation_count=sum(len(i['ops']) for i in selected))
        elif controls[0]['opcode'] == 'BRANCH' and controls[0]['inputs'][0]['space'] == 'ram':
            target = int(controls[0]['inputs'][0]['offset'], 16)
            if target not in indexes:
                raise Unsupported('Configuration branch leaves complete graph')
            instruction['ops'].remove(controls[0])
            index = indexes[target]
        else:
            raise Unsupported('Configuration initializer requires unconditional complete bodies')
        if not 0 <= index < len(instructions):
            raise Unsupported('Configuration initializer falls out of body')
    raise Unsupported('Configuration initializer cycle')


def compare(manifest, blob, original):
    from compiled_extended import sha
    if manifest.get('bundle_sha256') != sha(blob) or any(e['executable_sha256'] != sha(blob) for e in manifest['exports']):
        raise Unsupported('Configuration exports do not match compiled image')
    source = manifest['ps2_source']
    if source['entry'] != '0031dd50' or source['name'] != 'CActorFighter::GetStateCfg' or not source['complete']:
        raise Unsupported('PS2 configuration accessor identity differs')
    layout = manifest['layout']
    if (layout['config_size'], layout['config_animation'], layout['config_flags'],
            layout['fighter_config_size']) != (8, 0, 4, 108 * 8):
        raise Unsupported('Unsupported fighter configuration layout')
    if original['address'] != PS2_TABLE or not any(
            label['name'] == '_gStateCfg_FIG' and label['namespace'] == 'CActorFighter'
            for label in original['labels']):
        raise Unsupported('PS2 fighter configuration identity differs')
    raw = bytes.fromhex(original['hex'])
    if len(raw) != 108 * 8:
        raise Unsupported('Incomplete PS2 fighter configuration')
    base = manifest['addresses'][TABLE]
    roots = [e for e in manifest['exports'] if e['coff']['symbol'] == INITIALIZER]
    if len(roots) != 1:
        raise Unsupported('Missing/ambiguous configuration initializer')
    root = roots[0]
    graph = link(manifest['exports'], int(root['entry'], 16), max_depth=4)
    constructors = [e for e in manifest['exports'] if e['coff']['symbol'] == '??0StateConfig@@QEAA@HI@Z']
    if len(constructors) != 1:
        raise Unsupported('Missing/ambiguous configuration constructor')
    constructor = constructors[0]
    constructor_calls = sum(int(i['original_address'], 16) == int(constructor['entry'], 16)
                            for i in graph['instructions'])
    if constructor_calls != 108:
        raise Unsupported('Unexpected fighter configuration constructor count')
    path = straight_path(graph)
    initial = z3.Array('fighter_configuration_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    result = execute(path, z3.BitVecVal(0, 64), initial, path_mode=True, return_state=True,
                     regions=[(base, len(raw))], write_regions=[(base, len(raw))])
    if not result.returned or result.obligations:
        raise Unsupported('Configuration initializer did not return completely')
    actual = load(result.state.memory, base, len(raw))
    solver = z3.Solver()
    solver.set(timeout=30000)
    solver.add(actual != z3.BitVecVal(int.from_bytes(raw, 'little'), len(raw) * 8))
    answer = solver.check()
    if answer == z3.unknown:
        raise Unsupported('Configuration output proof inconclusive')
    entries = [dict(state=i + 6, animation=struct.unpack_from('<i', raw, i * 8)[0],
                    flags=hex(struct.unpack_from('<I', raw, i * 8 + 4)[0])) for i in range(108)]
    return dict(result='equivalent' if answer == z3.unsat else 'mismatch',
                build=manifest.get('build'), windows_image_sha256=sha(blob),
                object_sha256=layout['provenance']['object_sha256'],
                scope='Windows dynamic initializer produces original PS2 fighter table; no transition equivalence claimed',
                entries=entries, windows_table=base, ps2_table=PS2_TABLE,
                initialized_bytes_hex=raw.hex(), constructor_invocations=constructor_calls,
                ps2_executable_sha256=source['executable_sha256'],
                assumptions=['Trusted lifters; original PS2 table at 00434460 audited through GetStateCfg 0031dd50',
                             'Every write restricted to the configuration table or private stack',
                             'Arbitrary initial table/non-stack memory; constructors execute, BSS zeros not assumed',
                             'Later table mutation and configured actor/effect initialization remain caller obligations'])


def run(folder):
    folder = Path(folder)
    manifest = json.loads((folder / 'manifest.json').read_text())
    manifest.update(json.loads((folder / 'exports.json').read_text()))
    try:
        report = compare(manifest, (folder / 'functions.bin').read_bytes(),
                         json.loads((folder / 'original.json').read_text()))
    except Unsupported as error:
        report = dict(result='unsupported', reason=str(error))
    (folder / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def prepare_and_run(objects, globals, memory_evidence, folder, ghidra, java, build_identity, port):
    from client import request, export_pcode
    from compiled import lift
    from compiled_extended import bundle_closure, sha
    folder.mkdir(parents=True, exist_ok=True)
    (folder / 'report.json').unlink(missing_ok=True)
    cases = [('ActorFighter', None, 'FighterConfigurationInitializer', INITIALIZER)]
    _, prepared, frontier = bundle_closure({'ActorFighter': objects['ActorFighter']}, cases, {},
                                          globals={TABLE: globals[TABLE]})
    if prepared is None:
        raise Unsupported('Configuration dependency frontier: ' + '; '.join(frontier))
    blob, functions, addresses = prepared
    source = export_pcode('0031dd50', port)
    manifest = dict(build=build_identity, functions=functions, addresses=addresses, ps2_source=source,
                    layout=memory_evidence['layouts']['ActorFighter'], bundle_sha256=sha(blob))
    original = request('inspect_data', port, address=PS2_TABLE, length=108 * 8)
    if source != export_pcode('0031dd50', port):
        raise Unsupported('PS2 configuration source changed during capture')
    (folder / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    (folder / 'functions.bin').write_bytes(blob)
    (folder / 'original.json').write_text(json.dumps(original, indent=2) + '\n')
    lift(folder, ghidra, java)
    return run(folder)


def main():
    import argparse
    from build_config import arguments, identity
    from check import NINJA
    from transition_data import prepare
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--ninja', default=NINJA)
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    parser.add_argument('--replay', type=Path)
    args = parser.parse_args()
    if args.replay:
        report = run(args.replay)
    else:
        objects = {}
        globals, evidence = prepare(args.build_dir.resolve(), args.output_root.resolve() / 'memory', args.ninja, objects)
        report = prepare_and_run(objects, globals, evidence, args.output_root.resolve() / 'fighter-configuration',
                                 args.ghidra, args.java, identity(args.build_dir), args.port)
    print(json.dumps(report, indent=2))
    return 0 if report['result'] == 'equivalent' else 1


if __name__ == '__main__':
    raise SystemExit(main())
