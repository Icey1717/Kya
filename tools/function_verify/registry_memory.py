"""Bounded symbolic registry inputs and compiled pointer-resolution checks.

The slot relation is an input precondition, not a proof of the registry's callers.
Only executing the complete compiled resolver can discharge its implementation.
"""
import json
from pathlib import Path
from verify import Unsupported
import z3
from call_graph import link
from paths import expand
from memory_relation import load, store
from windows_pcode import execute


def seed(memory, addresses, layout, *, capacity=2, handle=None, pointer_values=None):
    if not 1 <= capacity <= 8:
        raise ValueError('Registry capacity must be between 1 and 8')
    if layout['vector_size'] != 32 or layout['slot_size'] <= 0:
        raise Unsupported('Unsupported registry vector layout; compiled resolver proof required')
    handle = z3.BitVec('registry_handle', 32) if handle is None else handle
    generation = z3.Extract(30, 20, handle)
    index = handle & 0xfffff
    transient = z3.Extract(31, 31, handle) == 1
    slot_size = layout['slot_size']
    values, domains, regions = [], [], []
    for name, base in [('persistent', 0x200000), ('transient', 0x300000)]:
        matches = [a for symbol, a in addresses.items() if symbol.startswith('?g' + name.title() + 'Registry@')]
        if len(matches) != 1:
            raise Unsupported('Missing/ambiguous compiled registry global')
        global_base = matches[0]
        vector = global_base + layout['registry_slots']
        # This is a candidate ABI relation, checked against the actual compiled
        # vector helpers by compare(). It is never accepted as a helper summary.
        for offset, value in [(8, base), (16, base + capacity * slot_size), (24, base + capacity * slot_size)]:
            memory = store(memory, vector + offset, z3.BitVecVal(value, 64))
        regions += [(global_base, layout[name + '_size']), (base, capacity * slot_size)]
        pointers = []
        for i in range(capacity):
            pointer = z3.BitVec(name + '_slot_' + str(i) + '_pointer', 64)
            if pointer_values and (name, i) in pointer_values:
                pointer = pointer_values[name, i]
                pointer = z3.BitVecVal(pointer, 64) if isinstance(pointer, int) else pointer
                if pointer.size() != 64:
                    raise ValueError('Registry slot pointer must be 64 bits')
            refs = z3.BitVec(name + '_slot_' + str(i) + '_refs', 32)
            gen = z3.BitVec(name + '_slot_' + str(i) + '_generation', 16)
            slot = base + i * slot_size
            memory = store(memory, slot + layout['slot_pointer'], pointer)
            memory = store(memory, slot + layout['slot_refcount'], refs)
            memory = store(memory, slot + layout['slot_generation'], gen)
            # Unselected slots remain arbitrary, including free/stale slots.
            selected = z3.And(handle != 0, transient == (name == 'transient'), index == i)
            domains.append(z3.Implies(selected, z3.And(refs != 0, gen == z3.ZeroExt(5, generation))))
            pointers.append(pointer)
        selected_value = pointers[-1]
        for i in reversed(range(capacity - 1)):
            selected_value = z3.If(index == i, pointers[i], selected_value)
        values.append(selected_value)
    domain = z3.simplify(z3.And(z3.Or(handle == 0, z3.ULT(index, capacity)), *domains))
    expected = z3.simplify(z3.If(handle == 0, z3.BitVecVal(0, 64), z3.If(transient, values[1], values[0])))
    return memory, handle, expected, domain, regions


def _compare_case(manifest, blob, *, capacity=2, handle=None):
    exports = manifest['exports']
    root = next(e for e in exports if e['name'] == 'ResolvePointerChecked')
    graph = link(exports, int(root['entry'], 16), max_depth=16, defer_unresolved=True)
    paths = expand(graph, max_paths=2048, max_instructions=4096)
    memory = z3.Array('registry_initial_memory', z3.BitVecSort(64), z3.BitVecSort(8))
    # Runtime globals replace link-time BSS. Code/import storage is deliberately
    # not readable in this contract: reaching a diagnostic import must fail.
    memory, handle, expected, domain, regions = seed(memory, manifest['addresses'], manifest['layout'], capacity=capacity, handle=handle)
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(domain)
    if solver.check() != z3.sat:
        raise Unsupported('Empty or inconclusive registry input domain')
    conditions = []
    for path in paths:
        result = execute(path, z3.BitVecVal(0, 64), memory, this_argument=z3.ZeroExt(32, handle),
                         path_mode=True, precondition=domain, regions=regions, write_regions=[])
        if result is None:
            continue
        actual, final, baseline, condition = result
        if path.get('cutoff') or actual is None:
            raise Unsupported('Registry resolver reached a cutoff')
        # The executor has proved each STORE lies in its private stack. This is
        # a frame proof for the entire non-stack memory, including both registries.
        solver = z3.Solver()
        solver.set(timeout=10000)
        solver.add(domain, condition, z3.simplify(actual != expected))
        answer = solver.check()
        if answer == z3.unknown:
            return dict(result='inconclusive', reason=solver.reason_unknown())
        if answer == z3.sat:
            model = solver.model()
            return dict(result='mismatch', counterexample=dict(
                handle=hex(model.eval(handle, model_completion=True).as_long()),
                expected_pointer=hex(model.eval(expected, model_completion=True).as_long()),
                actual_pointer=hex(model.eval(actual, model_completion=True).as_long()),
                path=[i['address'] for i in path['instructions']]))
        conditions.append(condition)
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(domain, z3.Not(z3.Or(*conditions)))
    if solver.check() != z3.unsat:
        raise Unsupported('Registry resolver input coverage not proved')
    return dict(result='equivalent', scope='Compiled Windows resolver satisfies registry memory relation; not a PS2 transition proof',
                capacity_per_registry=capacity, feasible_paths=len(conditions),
                assumptions=['Nonzero handles select an in-bounds live slot with matching generation',
                             'Registry storage does not alias private stack; no concurrent registry mutation',
                             'Slot pointer identities must separately be related to PS2 objects'])


def compare(manifest, blob, *, capacity=2):
    from compiled_extended import sha
    if any(e['executable_sha256'] != sha(blob) for e in manifest['exports']):
        raise Unsupported('Registry exports do not match the compiled image')
    # Exhaustive selector/slot partitioning keeps addresses concrete while all
    # generation bits, pointer values, refcounts and unrelated memory stay symbolic.
    if not 1 <= capacity <= 8:
        raise ValueError('Registry capacity must be between 1 and 8')
    generation = z3.BitVec('handle_generation', 11)
    handles = [z3.BitVecVal(0, 32)] + [z3.Concat(z3.BitVecVal(transient, 1), generation, z3.BitVecVal(index, 20))
                                                  for transient in (0, 1) for index in range(capacity)]
    paths = 0
    for handle in handles:
        result = _compare_case(manifest, blob, capacity=capacity, handle=handle)
        if result['result'] != 'equivalent':
            return result
        paths += result['feasible_paths']
    return dict(result, feasible_paths=paths, exhaustive_handle_partitions=len(handles))


def run(folder):
    folder = Path(folder)
    manifest = json.loads((folder / 'exports.json').read_text())
    # ExportCompiledPilot carries function metadata, but not extra manifest keys.
    manifest.update(json.loads((folder / 'manifest.json').read_text()))
    try:
        report = compare(manifest, (folder / 'functions.bin').read_bytes())
    except Unsupported as error:
        report = dict(result='unsupported', reason=str(error))
    (folder / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def prepare_and_run(objects, globals, memory_evidence, folder, ghidra, java, build_identity):
    from compiled import lift
    from compiled_extended import bundle_closure, sha
    folder.mkdir(parents=True, exist_ok=True)
    (folder / 'report.json').unlink(missing_ok=True)
    cases = [('PointerConv', None, 'ResolvePointerChecked', '?ResolvePointerChecked@PointerConv@@YAPEAXH@Z')]
    selected_globals = {k: v for k, v in globals.items() if v[0] == 'PointerConv'}
    selected, prepared, frontier = bundle_closure({'PointerConv': objects['PointerConv']}, cases, {}, globals=selected_globals)
    if prepared is None:
        report = dict(result='unsupported', reason='Registry dependency frontier: ' + '; '.join(frontier))
        (folder / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        return report
    blob, functions, addresses = prepared
    manifest = dict(build=build_identity, functions=functions, addresses=addresses,
                    layout=memory_evidence['layouts']['PointerConv'], bundle_sha256=sha(blob))
    (folder / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    (folder / 'functions.bin').write_bytes(blob)
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
    parser.add_argument('--ghidra', type=Path, default=Path('G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC'))
    parser.add_argument('--java', type=Path, default=Path('G:/jdk/jdk-21.0.9/bin/java.exe'))
    args = parser.parse_args()
    output = args.output_root.resolve()
    objects = {}
    globals, evidence = prepare(args.build_dir.resolve(), output / 'memory', args.ninja, objects)
    report = prepare_and_run(objects, globals, evidence, output / 'registry', args.ghidra, args.java, identity(args.build_dir))
    print(json.dumps(report, indent=2))
    return 0 if report['result'] == 'equivalent' else 1


if __name__ == '__main__':
    raise SystemExit(main())
