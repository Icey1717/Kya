"""Reproduce concrete unresolved collision/combat call-graph blockers."""
import json
from pathlib import Path
import sys
import argparse
import subprocess
from compiled import ROOT, lift
from compiled_extended import bundle, bundle_closure, sha
from call_graph import link
from fighter_dispatch import recover
from verify import request, Unsupported
from transition_data import prepare as prepare_memory
from registry_memory import prepare_and_run as verify_registry
from semantic_frontier import inventory
from animation_memory import prepare_and_run as verify_debug_animation
from fighter_runtime import prepare_and_run as verify_fighter_runtime
from fighter_configuration import prepare_and_run as verify_fighter_configuration


def main():
    from build_config import arguments, identity
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--audit-only', action='store_true', help='Exit zero after recording blockers; never constitutes equivalence')
    parser.add_argument('--output', type=Path)
    parser.add_argument('--ninja', default='C:/Program Files/Microsoft Visual Studio/18/Insiders/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe')
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    args = parser.parse_args()
    build = args.build_dir.resolve()
    configuration = identity(build)
    output = (args.output or args.output_root / 'combat-blockers').resolve()
    output.mkdir(parents=True, exist_ok=True)
    (output / 'report.json').unlink(missing_ok=True)
    cases = [('ActorFighter', '0031ba50', 'SetStandAnim', '?SetStandAnim@CActorFighter@@QEAAXH@Z'),
             ('Actor', '00102a00', 'SetState', '?SetState@CActor@@UEAAXHH@Z')]
    targets = ['CMakeFiles/Kya.dir/src/b-witch/' + c[0] + '.cpp.obj' for c in cases]
    targets.append('CMakeFiles/Kya.dir/src/b-witch/ActorFactory.cpp.obj')
    subprocess.run([args.ninja, '-C', str(build), *targets], check=True)
    objects = {c[0]: (build / target).read_bytes() for c, target in zip(cases, targets)}
    dispatch = recover(build, args.port)
    (output / 'fighter-dispatch.json').write_text(json.dumps(dispatch, indent=2) + '\n')
    # Analyze complete bodies of known direct dependencies together. This does
    # not approve their effects; unresolved descendants remain bundle failures.
    helpers = [
        ('Actor', '00103f60', 'GetBehaviour', '?GetBehaviour@CActor@@QEAAPEAVCBehaviour@@H@Z'),
        ('Actor', '00103db0', 'ActorPlayAnim', '?PlayAnim@CActor@@QEAAXH@Z'),
        ('ActorMovable', '0011e7c0', 'MovableSetState', '?SetState@CActorMovable@@UEAAXHH@Z'),
        ('Audio', '00184470', 'AudioLeave', '?FUN_00184470@CAudioManager@@QEAAXXZ'),
        ('Audio', '001844a0', 'AudioEnter', '?FUN_001844a0@CAudioManager@@QEAAXXZ'),
        ('Animation', '002425a0', 'LayerTimeWarper', '?SetLayerTimeWarper@edAnmBinMetaAnimator@@QEAAXMH@Z'),
    ]
    helper_targets = {c[0]: 'CMakeFiles/Kya.dir/src/b-witch/' + c[0] + '.cpp.obj' for c in helpers}
    helper_targets.update({name: 'CMakeFiles/Kya.dir/src/b-witch/' + name + '.cpp.obj'
                           for name in ('AnmManager', 'EventTrack', 'Fx')})
    helper_targets['PointerConv'] = 'CMakeFiles/Kya.dir/src/port/pointer_conv.cpp.obj'
    subprocess.run([args.ninja, '-C', str(build), *helper_targets.values()], check=True)
    objects.update({name: (build / target).read_bytes() for name, target in helper_targets.items()})
    globals, memory_evidence = prepare_memory(build, output / 'memory', args.ninja, objects)
    configuration_report = verify_fighter_configuration(objects, globals, memory_evidence,
        output / 'fighter-configuration', args.ghidra, args.java, configuration, args.port)
    registry_report = verify_registry(objects, globals, memory_evidence, output / 'registry', args.ghidra, args.java, configuration)
    debug_animation_report = verify_debug_animation(objects, globals, memory_evidence, output / 'debug-animation', args.ghidra, args.java, configuration)
    helper_exports = {c[1]: request('export_pcode', args.port, address=c[1]) for c in helpers}
    hashes = {e['executable_sha256'] for e in helper_exports.values()} | {dispatch['ps2_executable_sha256']}
    if len(hashes) != 1 or any(not e['complete'] for e in helper_exports.values()):
        raise Unsupported('Incomplete or mixed-executable transition dependencies')
    (output / 'dependencies.json').write_text(json.dumps(helper_exports, indent=2) + '\n')
    runtime_originals = {'00103f60': helper_exports['00103f60'],
                         '00102a00': request('export_pcode', args.port, address='00102a00'),
                         '0031dd50': dispatch['exports']['0031dd50']}
    if runtime_originals['00102a00']['executable_sha256'] not in hashes:
        raise Unsupported('Mixed PS2 runtime executable identities')
    runtime_report = verify_fighter_runtime(objects, globals, memory_evidence, output / 'fighter-runtime',
                                            args.ghidra, args.java, configuration, runtime_originals,
                                            configuration_report, dispatch)
    rows = []
    for case in cases:
        original = request('export_pcode', args.port, address=case[1])
        (output / (case[1] + '.json')).write_text(json.dumps(original, indent=2))
        row = dict(function=original['name'], ps2_entry=case[1], windows_object_sha256=sha(objects[case[0]]),
                   ps2_executable_sha256=original['executable_sha256'], status='blocked', result='unsupported',
                   indirect_calls=[i['address'] for i in original['instructions'] if any(o['opcode'] == 'CALLIND' for o in i['ops'])])
        try:
            row['stage'] = 'bundle'
            selected = [case] + helpers + ([cases[1]] if case == cases[0] else [])
            selected, prepared, frontier = bundle_closure(objects, selected, {}, globals=globals)
            row['compiled_dependency_functions'] = [dict(source=c[0], symbol=c[3]) for c in selected]
            row['unresolved_dependencies'] = frontier
            if prepared is None:
                raise Unsupported('Unresolved dependency closure: ' + '; '.join(frontier))
            blob, functions, addresses = prepared
            folder = output / case[2]
            folder.mkdir(exist_ok=True)
            (folder / 'functions.bin').write_bytes(blob)
            (folder / 'manifest.json').write_text(json.dumps(dict(functions=functions, addresses=addresses), indent=2))
            row['stage'] = 'lift'
            lift(folder, args.ghidra, args.java)
            exports = json.loads((folder / 'exports.json').read_text())['exports']
            semantic_evidence = inventory(exports)
            (folder / 'semantic-frontier.json').write_text(json.dumps(semantic_evidence, indent=2) + '\n')
            row['semantic_evidence'] = case[2] + '/semantic-frontier.json'
            row['floating_point_instructions'] = len(semantic_evidence['floating_point'])
            row['indirect_transfers_in_closure'] = len(semantic_evidence['indirect_transfers'])
            row['stage'] = 'call-graph'
            indirect_targets = {}
            if case[2] == 'SetStandAnim':
                transfers = [i for i in exports[0]['instructions'] if any(o['opcode'] == 'CALLIND' for o in i['ops'])]
                if len(transfers) != 1:
                    raise Unsupported('Unexpected SetStandAnim dispatch shape')
                movable = next(e for e in exports if e['name'] == 'MovableSetState')
                indirect_targets[int(transfers[0]['address'], 16)] = int(movable['entry'], 16)
            graph = link(exports, int(exports[0]['entry'], 16), max_depth=16,
                         indirect_targets=indirect_targets, defer_unresolved=True)
            row['unresolved_graph_frontiers'] = [dict(address=i['address'], reason=o['reason'])
                                                for i in graph['instructions'] for o in i['ops']
                                                if o['opcode'] == 'UNRESOLVED_TRANSFER']
            row.update(status='needs-memory-and-callee-setup', result='inconclusive', stage='comparison',
                       reason='Call expansion passed; bounded unchanged-state proof recorded separately; full configured transition proof unavailable')
        except (ValueError, Unsupported) as error:
            row['reason'] = str(error)
        rows.append(row)
    document = dict(build=configuration, scope='Capability audit only; no equivalence claimed', functions=rows,
                    dispatch_evidence='fighter-dispatch.json',
                    memory_evidence='memory/memory-layout.json',
                    registry_resolution=registry_report,
                    debug_animation_validation=debug_animation_report,
                    fighter_runtime=runtime_report,
                    fighter_configuration=configuration_report,
                    memory_obligation='Compiled storage/layout evidence does not establish runtime initialization or memory equivalence',
                    dispatch_obligation='Recovered slots do not prove object dynamic types or complete callee execution',
                    available_support=['Explicit machine states and complete-body transition composition',
                                       'Complete compiled fighter configuration initializer matches all 108 original PS2 entries',
                                       'Bounded sorted behaviour lists and live registry resolution composed in unchanged idle/run SetState',
                                       'Checked finite virtual target sets and independent LOD IsLooped/Stop callee proofs',
                                       'SSE scalar/MXCSR semantics and EE encoding comparisons',
                                       'Opt-in pinned PCSX2 corrected EE add/sub reference model; conditional verdicts only'],
                    required_support=['Establish runtime vtable target identity at every reachable virtual call on both architectures',
                                      'Expanded behavior/animation/audio call graph with object/global/stack memory mappings',
                                      'Floating-point/VU semantics wherever those reachable callees require them'])
    (output / 'report.json').write_text(json.dumps(document, indent=2) + '\n')
    print(json.dumps(document, indent=2))
    return 0 if args.audit_only else 1  # No whole-function proof is implemented here.


if __name__ == '__main__':
    raise SystemExit(main())
