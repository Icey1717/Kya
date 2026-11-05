"""Check the compiled Debug animation validator, including its history writes."""
import json
from pathlib import Path
from verify import Unsupported
import z3
from call_graph import link
from paths import expand
from memory_relation import load, store
from windows_pcode import execute


def compare(manifest, *, animation=0, entry_count=2, bank_offsets=None):
    if type(entry_count) is not int or not 1 <= entry_count <= 8:
        raise ValueError('Animation validator requires 1..8 bank entries')
    if type(animation) is not int or not 0 <= animation < entry_count:
        raise ValueError('Animation index outside validator bank')
    offsets = list(bank_offsets) if bank_offsets is not None else [entry_count * 4 + index * 4 for index in range(entry_count)]
    if (len(offsets) != entry_count or any(type(offset) is not int or offset % 4
            or not entry_count * 4 <= offset <= 4092 for offset in offsets)
            or len(set(offsets)) != entry_count):
        raise ValueError('Validator bank offsets require distinct aligned in-bounds track records')
    layout = manifest['layout']
    globals = manifest['globals']
    def global_address(key):
        return manifest['addresses'][globals[key]['symbol']]
    memory = z3.Array('animation_validator_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    actor = z3.BitVec('layer_actor_pointer', 64)
    count = z3.BitVec('history_lookup_count', 32)
    track = [z3.BitVec('bank_track_' + str(i), 32) for i in range(entry_count)]
    domain = z3.And(*[z3.And(t >= -1, t < entry_count) for t in track])
    animator, layers, bank, snapshot, manager = 0x100000, 0x200000, 0x300000, 0x400000, 0x500000
    def put(address, value, bits=64):
        nonlocal memory
        memory = store(memory, address, z3.BitVecVal(value, bits) if isinstance(value, int) else value)
    put(animator + layout['animator_layers'], layers)
    put(animator + layout['animator_layer_count'], 1, 32)
    put(animator + layout['animator_bank'], bank)
    put(layers + layout['layer_actor'], actor)
    put(global_address('bank'), bank)
    # Candidate debug-vector ABI; complete compiled vector helpers validate it.
    bank_size = max(offsets) + 4
    for offset, value in [(8, snapshot), (16, snapshot + bank_size), (24, snapshot + bank_size)]:
        put(global_address('snapshot') + offset, value)
    for index in range(bank_size):
        byte = z3.BitVec('bank_byte_' + str(index), 8)
        put(bank + index, byte)
        put(snapshot + index, byte)
    for index, offset in enumerate(offsets):
        for base in (bank, snapshot):
            put(base + index * 4, offset, 32)
            put(base + offset, track[index])
    put(global_address('scene') + layout['scene_track_manager'], manager)
    put(manager + layout['track_count'], entry_count, 32)
    put(global_address('lookup_count'), count)
    history = global_address('history')
    entry_size = layout['history_entry_size']
    if globals['history']['size'] != 32 * entry_size:
        raise Unsupported('Unexpected history array extent')
    regions = [(animator, layout['animator_size']), (layers, layout['layer_size']),
               (bank, bank_size), (snapshot, bank_size), (manager, layout['track_manager_size'])]
    regions += [(global_address(key), value['size']) for key, value in globals.items()]
    root = next(e for e in manifest['exports'] if e['name'] == 'DebugValidateAnimTrackData')
    # Keep failure calls as explicit terminal obligations. They must be proved
    # unreachable, including for malformed input mutations; no failure body is
    # summarized as a successful return.
    bodies = [e for e in manifest['exports'] if not e.get('coff', {}).get('symbol', '').startswith('?DebugAnimTrackFailure@')]
    graph = link(bodies, int(root['entry'], 16), max_depth=16, defer_unresolved=True)
    paths = expand(graph, max_paths=4096, max_instructions=4096, loop_visits=2)
    conditions = []
    unreachable_prefixes = []
    for path in paths:
        guards = tuple((i['address'], o['taken']) for i in path['instructions'] for o in i['ops'] if o['opcode'] == 'PATH_GUARD')
        if any(guards[:len(prefix)] == prefix for prefix in unreachable_prefixes):
            continue
        trace = []
        reached_guards = []
        result = execute(path, z3.BitVecVal(0, 64), memory, this_argument=z3.BitVecVal(animator, 64),
                         second_argument=z3.BitVecVal(animation, 64), path_mode=True, precondition=domain,
                         regions=regions, write_regions=[(history, globals['history']['size']),
                                                        (global_address('lookup_count'), 4)], store_trace=trace,
                         guard_trace=reached_guards)
        if result is None:
            # execute() returns None only after proving this exact prefix UNSAT
            # under the same initial memory/domain. Unknown never enters cache.
            unreachable_prefixes.append(tuple(reached_guards))
            continue
        actual, final, baseline, condition = result
        if path.get('cutoff') or actual is None:
            raise Unsupported('Reachable animation validator loop cutoff')
        expected = store(baseline, global_address('lookup_count'), count + 1)
        entry = z3.BitVecVal(history, 64) + z3.ZeroExt(32, count & 31) * entry_size
        for key, value in [('animator', z3.BitVecVal(animator, 64)), ('actor', actor),
                           ('layer', z3.BitVecVal(0, 32)), ('animation', z3.BitVecVal(animation, 32)),
                           ('offset', z3.BitVecVal(offsets[animation], 32)), ('track', track[animation])]:
            expected = store(expected, entry + layout['history_' + key], value)
        stack = set()
        for address, value in trace:
            if z3.is_bv_value(address) and 0x7e00 <= address.as_long() < 0x8000:
                stack.update(range(address.as_long(), address.as_long() + value.size() // 8))
        for address in sorted(stack):
            index = z3.BitVecVal(address, 64)
            final = z3.Store(final, index, baseline[index])
        solver = z3.Solver()
        solver.set(timeout=30000)
        solver.add(domain, condition, z3.simplify(z3.Or(actual != bank + offsets[animation], final != expected)))
        answer = solver.check()
        if answer == z3.unknown:
            return dict(result='inconclusive', reason=solver.reason_unknown())
        if answer == z3.sat:
            model = solver.model()
            return dict(result='mismatch', counterexample=dict(
                animation=animation, history_count=hex(model.eval(count, model_completion=True).as_long()),
                actual_pointer=hex(model.eval(actual, model_completion=True).as_long()),
                expected_pointer=hex(bank + offsets[animation]), path=[i['address'] for i in path['instructions']]))
        conditions.append(condition)
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(domain, z3.Not(z3.Or(*conditions)))
    if solver.check() != z3.unsat:
        raise Unsupported('Animation validator input coverage not proved')
    return dict(result='equivalent', scope='Compiled Windows Debug validator contract; not a PS2 transition proof',
                animation=animation, feasible_paths=len(conditions),
                entry_count=entry_count,
                bank_offsets=offsets,
                domain=f'One valid layer; {entry_count} animation entries; equal live/snapshot banks; track IDs -1..{entry_count - 1}; arbitrary uint32 history count and history bytes')


def prepare_and_run(objects, globals, evidence, folder, ghidra, java, build_identity):
    from compiled import lift, coff_symbols
    from compiled_extended import bundle_closure, sha
    folder.mkdir(parents=True, exist_ok=True)
    (folder / 'report.json').unlink(missing_ok=True)
    names = [s[0] for s in coff_symbols(objects['Animation'])[0]
             if s[0].startswith('?DebugValidateAnimTrackData@') and s[2] > 0]
    if len(names) != 1:
        raise Unsupported('Missing/ambiguous compiled Debug animation validator')
    cases = [('Animation', None, 'DebugValidateAnimTrackData', names[0])]
    selected_globals = {k: v for k, v in globals.items() if v[0] == 'Animation' or v[0] == 'Scene'}
    _, prepared, frontier = bundle_closure({'Animation': objects['Animation'], 'Scene': objects['Scene']}, cases, {}, globals=selected_globals)
    report = dict(result='unsupported', reason='Validator dependency frontier: ' + '; '.join(frontier))
    if prepared is not None:
        blob, functions, addresses = prepared
        manifest = dict(build=build_identity, functions=functions, addresses=addresses,
                        layout=evidence['layouts']['Animation'], bundle_sha256=sha(blob),
                        globals={k: v for k, v in evidence['globals'].items() if v['symbol'] in selected_globals})
        (folder / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
        (folder / 'functions.bin').write_bytes(blob)
        lift(folder, ghidra, java)
        manifest = json.loads((folder / 'exports.json').read_text())
        try:
            results = [compare(manifest, animation=i) for i in (0, 1)]
            results += [compare(manifest, animation=i, entry_count=3, bank_offsets=(12, 32, 52))
                        for i in range(3)]
            status = ('equivalent' if all(r['result'] == 'equivalent' for r in results) else
                      'mismatch' if any(r['result'] == 'mismatch' for r in results) else 'inconclusive')
            report = dict(result=status, cases=results)
        except Unsupported as error:
            report = dict(result='unsupported', reason=str(error))
    (folder / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return report
