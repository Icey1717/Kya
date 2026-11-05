"""Execute fighter lookup and unchanged-state domains with related runtime heaps.

This proves bounded input domains, not constructor/configuration invariants.
Both architectures execute complete bodies, including the Windows registry
closure. Unreached playback, virtual calls and diagnostics remain frontiers.
"""
import json
from pathlib import Path
from verify import Unsupported
import z3

from call_graph import link
from execution_state import ExecutionState
from memory_relation import MemoryRelation, store
from registry_memory import seed as seed_registry
from transitions import TransitionStep, invocation, outcomes


ENTRIES = {'GetBehaviour': ('00103f60', 'CActor::GetBehaviour'),
           'SetState': ('00102a00', 'CActor::SetState')}
IDENTITIES = dict(ENTRIES, GetStateCfg=('0031dd50', 'CActorFighter::GetStateCfg'))


def inputs(manifest, *, count, transient, sorted_ids=True, null_handles=False, states=(6, 9), configured=False,
           general_bindings=False, capacity=2, concrete_bindings=False, fighter_object=False):
    if not 0 <= count <= 8 or transient not in (0, 1) or not 1 <= capacity <= 8 or (not general_bindings and count > capacity):
        raise ValueError('Runtime domain requires at most eight entries/slots and a registry selector')
    if not states or any(type(state) is not int or not 6 <= state <= 113 for state in states):
        raise ValueError('Runtime states must be inside the fighter configuration table')
    actor = manifest['layouts']['Actor']
    registry = manifest['layouts']['PointerConv']
    if actor['entry_size'] != 8 or actor['list_entries'] != 4:
        raise Unsupported('Unsupported compiled behaviour-list layout')
    wm = z3.Array('fighter_windows_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    sm = z3.Array('fighter_ps2_memory', z3.BitVecSort(32), z3.BitVecSort(8))
    relation = MemoryRelation()
    # Different addresses ensure a native pointer cannot accidentally stand in
    # for a registry handle or a PS2 pointer.
    actor_size = manifest['layouts']['ActorFighter']['fighter_size'] if fighter_object else actor['actor_size']
    for side, base, size in [('windows', 0x100000, actor_size), ('ps2', 0x1000, 0x1000)]:
        relation.region(side, 'actor', base, size)
    for side, base in [('windows', 0x110000), ('ps2', 0x3000)]:
        relation.region(side, 'list', base, max(20, 4 + count * 8))
    relation.region('windows', 'stack', 0x7c00, 0x400, private=True)
    # Complete fighter entry nests SetState's 0x70-byte frame, InitState's
    # 0x110-byte frame and GetStateCfg's 0x10-byte frame (entry SP 0x8010).
    # The unchanged-state lookup domain does not require this deeper storage.
    ps2_stack = (0x7e00, 0x210) if fighter_object else (0x7f00, 0x110)
    relation.region('ps2', 'stack', *ps2_stack, private=True)
    wm = store(wm, 0x100000 + actor['actor_components'], z3.BitVecVal(0x110000, 64))
    sm = store(sm, 0x10f8, z3.BitVecVal(0x3000, 32))
    wm = store(wm, 0x110000, z3.BitVecVal(count, 32))
    sm = store(sm, 0x3000, z3.BitVecVal(count, 32))
    ids = [z3.BitVec('behaviour_id_' + str(i), 32) for i in range(count)]
    query = z3.BitVec('behaviour_query', 32)
    animation = z3.BitVec('unchanged_animation', 32)
    if configured and len(states) == 1:
        # This domain already requires the old state's configured animation.
        # Use its known bits directly so every prefix need not rediscover the
        # same scalar equality through nested stack/heap reads.
        raw = bytes.fromhex(manifest.get('configuration', {}).get('initialized_bytes_hex', ''))
        if len(raw) != 864:
            raise Unsupported('Incomplete runtime fighter configuration')
        offset = (states[0] - 6) * 8
        animation = z3.BitVecVal(int.from_bytes(raw[offset:offset + 4], 'little'), 32)
    state = z3.BitVecVal(states[0], 32) if len(states) == 1 else z3.BitVec('unchanged_state', 32)
    claims = [z3.Or(*[state == value for value in states])]
    if not configured:
        claims.append(animation != -1)
    if sorted_ids:
        claims += [ids[i] < ids[i + 1] for i in range(count - 1)]
    wexpected, sexpected = z3.BitVecVal(0, 64), z3.BitVecVal(0, 32)
    list_size = max(20, 4 + count * 8)
    regions = {'windows': [(0x100000, actor_size), (0x110000, list_size), (0x7c00, 0x200)],
               'ps2': [(0x1000, 0x1000), (0x3000, list_size), (0x451698, 4), ps2_stack]}
    for i, identifier in enumerate(ids):
        generation = z3.BitVec('behaviour_generation_' + str(i), 11)
        handle = (z3.BitVecVal(0, 32) if null_handles else
                  z3.Concat(z3.BitVecVal(transient, 1), generation, z3.BitVecVal(i, 20)))
        if general_bindings:
            handle = z3.BitVec('behaviour_handle_' + str(i), 32)
        wm, _, resolved, valid, storage = seed_registry(wm, manifest['addresses'], registry,
            capacity=capacity, handle=handle, pointer_values={
                ('transient' if transient else 'persistent', j): 0x120000 + j * 0x1000
                for j in range(count)} if concrete_bindings and not general_bindings and not null_handles else None)
        wp, sp = (0, 0) if null_handles else (0x120000 + i * 0x1000, 0x4000 + i * 0x1000)
        if general_bindings:
            identity = z3.BitVec('behaviour_identity_' + str(i), 1)
            wp = z3.If(handle == 0, z3.BitVecVal(0, 64), z3.If(identity == 0,
                         z3.BitVecVal(0x120000, 64), z3.BitVecVal(0x121000, 64)))
            sp = z3.If(handle == 0, z3.BitVecVal(0, 32), z3.If(identity == 0,
                         z3.BitVecVal(0x4000, 32), z3.BitVecVal(0x5000, 32)))
        claims += [valid, resolved == wp]
        if not null_handles and not general_bindings:
            claims.append(handle != 0)
        regions['windows'] += storage
        wo, so = 0x110000 + actor['list_entries'] + i * actor['entry_size'], 0x3004 + i * 8
        wm = store(wm, wo + actor['entry_id'], identifier)
        wm = store(wm, wo + actor['entry_handle'], handle)
        sm = store(sm, so, identifier)
        sm = store(sm, so + 4, z3.BitVecVal(sp, 32) if isinstance(sp, int) else sp)
        wexpected = z3.If(query == identifier, z3.BitVecVal(wp, 64) if isinstance(wp, int) else wp, wexpected)
        sexpected = z3.If(query == identifier, z3.BitVecVal(sp, 32) if isinstance(sp, int) else sp, sexpected)
    for key, offset, value in [('behaviour', 0xfc, query), ('state', 0x104, state),
                               ('animation', 0x11c, animation)]:
        wm = store(wm, 0x100000 + actor['actor_' + key], value)
        sm = store(sm, 0x1000 + offset, value)
    scenes = [address for name, address in manifest['addresses'].items() if name == '?ptable@CScene@@2TManagerContainer@@A']
    if len(scenes) != 1:
        raise Unsupported('Missing/ambiguous scene runtime storage')
    regions['windows'].append((scenes[0], manifest['layouts']['Scene']['scene_size']))
    config_expected = None
    if configured:
        from fighter_configuration import TABLE, PS2_TABLE
        report = manifest.get('configuration', {})
        probe = manifest['layouts'].get('ActorFighter', {}).get('provenance', {})
        if (report.get('result') != 'equivalent' or not probe.get('object_sha256')
                or report.get('object_sha256') != probe['object_sha256']):
            raise Unsupported('Configuration constructor proof must match the current fighter object')
        raw = bytes.fromhex(report['initialized_bytes_hex'])
        if len(raw) != 864:
            raise Unsupported('Incomplete runtime fighter configuration')
        entries = [e for e in manifest['exports'] if e['coff']['symbol'] == '?GetStateCfg@CActorFighter@@UEAAPEAUStateConfig@@H@Z']
        if len(entries) != 1:
            raise Unsupported('Missing/ambiguous fighter configuration accessor')
        wb, sb = manifest['addresses'][TABLE], int(PS2_TABLE, 16)
        wm = store(wm, wb, z3.BitVecVal(int.from_bytes(raw, 'little'), 864 * 8))
        sm = store(sm, sb, z3.BitVecVal(int.from_bytes(raw, 'little'), 864 * 8))
        wm = store(wm, 0x100000, z3.BitVecVal(0x130000, 64))
        sm = store(sm, 0x1000, z3.BitVecVal(0x441a90, 32))
        mapping = [r for r in manifest['dispatch']['mappings'] if r['ps2_target'] == '0031dd50']
        if (len(mapping) != 1 or mapping[0]['ps2_vtable'] != '00441a90'
                or mapping[0]['windows_symbol'] != entries[0]['coff']['symbol']
                or int.from_bytes(bytes.fromhex(mapping[0]['ps2_table_bytes'])[0x54:0x58], 'little') != 0x31dd50):
            raise Unsupported('Invalid concrete fighter configuration dispatch evidence')
        wm = store(wm, 0x130000 + mapping[0]['windows_offset'], z3.BitVecVal(int(entries[0]['entry'], 16), 64))
        sm = store(sm, 0x441a90 + mapping[0]['ps2_offset'], z3.BitVecVal(0x31dd50, 32))
        regions['windows'] += [(wb, 864), (0x130000, 0x200)]
        regions['ps2'] += [(sb, 864), (0x441a90, 0x100)]
        from fighter_cases import configuration
        selected = z3.BitVecVal(0, 32)
        for row in configuration(raw):
            selected = z3.If(state == row['state'], z3.BitVecVal(row['animation'], 32), selected)
        claims.append(animation == selected)
        config_expected = (z3.BitVecVal(wb, 64) + (z3.ZeroExt(32, state) - 6) * 8,
                           z3.BitVecVal(sb, 32) + (state - 6) * 8)
    saved = {('register', offset, 16): z3.BitVec('fighter_saved_' + hex(offset), 128)
             for offset in range(0x100, 0x180, 16)}
    if fighter_object:
        # Actual InitState save/restore at 00313da0 / 00314d60. Arbitrary bits
        # model the caller's f20 value; the executor must prove it is preserved.
        saved[('register', 0x1054, 4)] = z3.BitVec('fighter_saved_f20', 32)
    return dict(windows=ExecutionState('windows', wm), ps2=ExecutionState('ps2', sm, saved),
                domain=z3.And(*claims), relation=relation, regions=regions, query=query,
                state=state, animation=animation, ids=ids, expected=(wexpected, sexpected),
                config_expected=config_expected)


def graphs(manifest, originals, name, *, configured=False, all_config_calls=False):
    address, identity = IDENTITIES[name]
    if (originals[address]['entry'] != address or originals[address]['name'] != identity
            or originals['00103f60']['name'] != ENTRIES['GetBehaviour'][1]):
        raise Unsupported('PS2 runtime function identity mismatch')
    if len({e['executable_sha256'] for e in originals.values()}) != 1:
        raise Unsupported('Mixed PS2 runtime executable identities')
    exports = manifest['exports']
    symbol = {'GetBehaviour': '?GetBehaviour@CActor@@QEAAPEAVCBehaviour@@H@Z',
              'SetState': '?SetState@CActor@@UEAAXHH@Z',
              'GetStateCfg': '?GetStateCfg@CActorFighter@@UEAAPEAUStateConfig@@H@Z'}[name]
    roots = [e for e in exports if e.get('coff', {}).get('symbol') == symbol]
    if len(roots) != 1:
        raise Unsupported('Missing/ambiguous compiled runtime entry')
    root = roots[0]
    wi, si = {}, {}
    ps2_bodies = [originals[a] for a in ENTRIES_ADDRESS_ORDER]
    if configured:
        if name not in ('SetState', 'GetStateCfg'):
            raise ValueError('Configured playback requires SetState')
        getter = next(e for e in exports if e['coff']['symbol'] == '?GetStateCfg@CActorFighter@@UEAAPEAUStateConfig@@H@Z')
        calls = [i for i in root['instructions'] if any(op['opcode'] == 'CALLIND' for op in i['ops'])]
        original = originals['0031dd50']
        if original['name'] != 'CActorFighter::GetStateCfg' or original['entry'] != '0031dd50':
            raise Unsupported('PS2 fighter configuration accessor identity mismatch')
        if name == 'SetState':
            wi[int(calls[0]['address'], 16)] = int(getter['entry'], 16)
            si[0x102a54] = 0x31dd50
            if all_config_calls:
                for helper in exports:
                    if helper['coff']['symbol'] == '?GetStateFlags@CActor@@QEAAIH@Z':
                        for instruction in helper['instructions']:
                            if any(op['opcode'] == 'CALLIND' for op in instruction['ops']):
                                wi[int(instruction['address'], 16)] = int(getter['entry'], 16)
                si.update({0x102aa8: 0x31dd50, 0x102ad8: 0x31dd50})
                ps2_bodies.append(originals['00103db0'])
        ps2_bodies.append(original)
    return (link(exports, int(root['entry'], 16), max_depth=16, indirect_targets=wi, defer_unresolved=True),
            link(ps2_bodies, int(address, 16), indirect_targets=si, defer_unresolved=True))


ENTRIES_ADDRESS_ORDER = ('00103f60', '00102a00')


def compare_case(manifest, originals, name, *, count=2, transient=0, sorted_ids=True, null_handles=False,
                 states=(6, 9), configured=False, general_bindings=False, capacity=2):
    data = inputs(manifest, count=count, transient=transient, sorted_ids=sorted_ids, null_handles=null_handles,
                  states=states, configured=configured, general_bindings=general_bindings, capacity=capacity)
    data['relation'].require_satisfiable(data['domain'])
    wg, sg = graphs(manifest, originals, name, configured=configured)
    argument = data['query'] if name == 'GetBehaviour' else data['state']
    wo = dict(this_argument=z3.BitVecVal(0x100000, 64), regions=data['regions']['windows'],
              write_regions=[(0x7c00, 0x200)])
    so = dict(this_argument=z3.BitVecVal(0x1000, 64), regions=data['regions']['ps2'],
              call_mode=True, graph_mode=True, loop_mode=True)
    if name == 'SetState':
        wo['second_argument'] = z3.BitVecVal(0xffffffff, 64) if configured else z3.ZeroExt(32, data['animation'])
        so['second_argument'] = z3.BitVecVal(0xffffffffffffffff, 64) if configured else z3.SignExt(32, data['animation'])
    step = TransitionStep(name, wg, sg, z3.SignExt(32, argument), loop_visits=max(4, count + 2),
                          windows_options=wo, ps2_options=so)
    totals = []
    expected_returns = data['config_expected'] if name == 'GetStateCfg' else data['expected']
    if expected_returns is None:
        raise Unsupported('Configuration lookup requires initialized configuration mappings')
    for side, graph, options, expected in [('windows', wg, wo, expected_returns[0]),
                                           ('ps2', sg, so, expected_returns[1])]:
        results = outcomes(graph, invocation(data[side]), step.argument, data['domain'], options, step)
        for result in results:
            condition = z3.And(data['domain'], result.state.condition)
            solver = z3.Solver()
            solver.set(timeout=30000)
            # Every executor memory mutation is recorded in writes (including
            # x64 direct RAM outputs). Proving each write private establishes
            # the frame for the full byte array without large restore chains.
            checks = [z3.Or(*[z3.And(z3.UGE(address, base),
                                    z3.ULE(address, base + size - value.size() // 8))
                              for base, size in data['relation'].private[side]
                              if size >= value.size() // 8])
                      for address, value in result.writes]
            if name in ('GetBehaviour', 'GetStateCfg'):
                actual = result.return_value
                checks.append(z3.Extract(expected.size() - 1, 0, actual) == expected)
            solver.add(condition, z3.Not(z3.And(*checks)))
            answer = solver.check()
            if answer == z3.unknown:
                raise Unsupported('Runtime output/frame proof inconclusive: ' + solver.reason_unknown())
            if answer == z3.sat:
                model = solver.model()
                return dict(result='mismatch', architecture=side,
                            counterexample=dict(query=str(model.eval(data['query'], model_completion=True)),
                                                state=str(model.eval(data['state'], model_completion=True)),
                                                behaviour_ids=[str(model.eval(value, model_completion=True))
                                                               for value in data['ids']]))
        totals.append(len(results))
    return dict(result='equivalent', count=count, registry='mixed-symbolic' if general_bindings else 'transient' if transient else 'persistent',
                null_handles=null_handles, states=list(states), configured=configured,
                general_bindings=general_bindings, capacity_per_registry=capacity,
                windows_paths=totals[0], ps2_paths=totals[1])


def compare(manifest, blob, originals):
    from compiled_extended import sha
    if manifest.get('bundle_sha256') != sha(blob) or any(e['executable_sha256'] != sha(blob) for e in manifest['exports']):
        raise Unsupported('Runtime exports do not match the compiled image')
    rows = []
    for name, (address, _) in ENTRIES.items():
        partitions = [compare_case(manifest, originals, name, count=count, transient=transient)
                      for count in (0, 1, 2) for transient in (0, 1)]
        partitions.append(compare_case(manifest, originals, name, null_handles=True))
        partitions.extend(compare_case(manifest, originals, name, count=count, general_bindings=True)
                          for count in (1, 2))
        rows.append(dict(function=ENTRIES[name][1], ps2_entry=address,
                         result='equivalent' if all(p['result'] == 'equivalent' for p in partitions) else 'mismatch',
                         partitions=partitions))
    return dict(scope='Bounded runtime input relations; unchanged-state subdomain only', functions=rows,
                build=manifest.get('build'), windows_image_sha256=sha(blob),
                ps2_executable_sha256=originals['00102a00']['executable_sha256'],
                result='equivalent' if all(r['result'] == 'equivalent' for r in rows) else 'mismatch',
                assumptions=['0..2 strictly increasing signed behaviour IDs; symbolic query and matching live registry slots',
                             'General partitions permit mixed registries, null/live entries and aliases to two mapped behaviour identities',
                             'Two slots per registry, arbitrary matching generation/refcount and unrelated memory',
                             'SetState: idle (6) or run (9) unchanged; explicit animation != -1 equals current animation',
                             'Disjoint actor/list/registry/stack storage; runtime initialization is an input obligation',
                             'Complete lookup/resolver bodies execute; no summary or diagnostic stubs',
                             'No configured animation selection, playback, state-changing path or EE/VU arithmetic proved'])


def prepare_and_run(objects, globals, memory_evidence, folder, ghidra, java, build_identity, originals,
                    configuration_report=None, dispatch_evidence=None, *, configured_only=False):
    from compiled import lift
    from compiled_extended import bundle_closure, sha
    folder.mkdir(parents=True, exist_ok=True)
    (folder / 'report.json').unlink(missing_ok=True)
    cases = [('Actor', '00102a00', 'SetState', '?SetState@CActor@@UEAAXHH@Z'),
             ('Actor', '00103f60', 'GetBehaviour', '?GetBehaviour@CActor@@QEAAPEAVCBehaviour@@H@Z')]
    if configuration_report is not None:
        cases.append(('ActorFighter', '0031dd50', 'FighterGetStateCfg', '?GetStateCfg@CActorFighter@@UEAAPEAUStateConfig@@H@Z'))
    _, prepared, frontier = bundle_closure(objects, cases, {}, globals=globals)
    if prepared is None:
        raise Unsupported('Runtime dependency frontier: ' + '; '.join(frontier))
    blob, functions, addresses = prepared
    manifest = dict(build=build_identity, functions=functions, addresses=addresses,
                    layouts=memory_evidence['layouts'], bundle_sha256=sha(blob))
    if configuration_report is not None:
        manifest.update(configuration=configuration_report, dispatch=dispatch_evidence)
    (folder / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    (folder / 'functions.bin').write_bytes(blob)
    (folder / 'originals.json').write_text(json.dumps(originals, indent=2) + '\n')
    lift(folder, ghidra, java)
    return run(folder, configured_only=configured_only)


def run(folder, *, configured_only=False):
    folder = Path(folder)
    manifest = json.loads((folder / 'manifest.json').read_text())
    manifest.update(json.loads((folder / 'exports.json').read_text()))
    originals = json.loads((folder / 'originals.json').read_text())
    try:
        blob = (folder / 'functions.bin').read_bytes()
        if configured_only:
            from compiled_extended import sha
            if manifest['bundle_sha256'] != sha(blob) or any(e['executable_sha256'] != sha(blob) for e in manifest['exports']):
                raise Unsupported('Runtime exports do not match the compiled image')
            lookup = compare_case(manifest, originals, 'GetStateCfg', count=0,
                                  states=tuple(range(6, 114)), configured=True)
            configured = [compare_case(manifest, originals, 'SetState', count=0, states=(state,), configured=True)
                          for state in (6, 9)]
            report = dict(result='equivalent' if lookup['result'] == 'equivalent' and all(
                r['result'] == 'equivalent' for r in configured) else 'mismatch',
                scope='Complete fighter GetStateCfg for 108 entries and unchanged configured idle/run with empty behaviour list',
                configuration_lookup=lookup, configured_unchanged=configured)
        else:
            report = compare(manifest, blob, originals)
    except Unsupported as error:
        report = dict(result='unsupported', reason=str(error))
    (folder / ('configured-report.json' if configured_only else 'report.json')).write_text(json.dumps(report, indent=2) + '\n')
    return report


def main():
    import argparse
    import subprocess
    from build_config import arguments, identity
    from check import NINJA
    from client import export_pcode
    from transition_data import prepare
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--ninja', default=NINJA)
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    parser.add_argument('--replay', type=Path, help='Recheck retained exports without rebuilding or using the live bridge')
    parser.add_argument('--configured-only', action='store_true', help='Check the full fighter table lookup and configured idle/run unchanged subdomain')
    args = parser.parse_args()
    if args.replay:
        report = run(args.replay, configured_only=args.configured_only)
    else:
        build, output = args.build_dir.resolve(), args.output_root.resolve()
        objects = {}
        globals, evidence = prepare(build, output / 'memory', args.ninja, objects)
        sources = ('ActorMovable', 'Audio', 'EventTrack', 'Fx', 'ActorFactory')
        targets = ['CMakeFiles/Kya.dir/src/b-witch/' + source + '.cpp.obj' for source in sources]
        subprocess.run([args.ninja, '-C', str(build), *targets], check=True)
        objects.update({source: (build / target).read_bytes() for source, target in zip(sources, targets)})
        originals = {address: export_pcode(address, args.port) for address in ENTRIES_ADDRESS_ORDER}
        configuration_report, dispatch_evidence = None, None
        if args.configured_only:
            from fighter_configuration import prepare_and_run as configure
            from fighter_dispatch import recover
            configuration_report = configure(objects, globals, evidence, output / 'fighter-configuration',
                                             args.ghidra, args.java, identity(build), args.port)
            dispatch_evidence = recover(build, args.port)
            originals['0031dd50'] = dispatch_evidence['exports']['0031dd50']
            originals['00103db0'] = export_pcode('00103db0', args.port)
        report = prepare_and_run(objects, globals, evidence, output / 'fighter-runtime', args.ghidra,
                                 args.java, identity(build), originals, configuration_report, dispatch_evidence,
                                 configured_only=args.configured_only)
    print(json.dumps(report, indent=2))
    return 0 if report['result'] == 'equivalent' else 1


if __name__ == '__main__':
    raise SystemExit(main())
