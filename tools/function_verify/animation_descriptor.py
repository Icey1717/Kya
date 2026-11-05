"""Execute real descriptor-parser bodies over bounded relocated animation banks."""
from verify import Unsupported
import z3
from call_graph import link
from execution_state import ExecutionState
from memory_relation import MemoryRelation, load, store
from transitions import TransitionStep, invocation, outcomes, prove


SYMBOL = '?BuildDesc@edAnmStateParser@@QEAAPEAUedAnmStateDesc@@PEAU2@HH@Z'
ENTRY = '00241490'
# Original stores and loads audited against the 24 instructions at 00241490.
PS2_FIELDS = dict(field=0, animation=4, header_a=8, flags=12, max_key=16,
                  header_b=20, original_animation=24)


def compare(manifest, originals, *, table_words=1):
    """Prove all parser outputs, input preservation, return identity and stores.

    The last header offset is symbolic and aligned within a 128-byte bank.
    Header bytes, both animation IDs and initial scalar outputs are arbitrary.
    This is a parser proof, not a playback or whole-fighter verdict.
    """
    if type(table_words) is not int or not 1 <= table_words <= 4:
        raise ValueError('Descriptor domain requires 1..4 table words')
    layout = manifest['layouts']['Animation']
    required = ['state_desc_size', 'state_parser_size', 'state_parser_data']
    required += ['state_desc_' + key for key in PS2_FIELDS]
    if any(key not in layout for key in required):
        raise Unsupported('Missing compiler-backed animation descriptor layout')
    body = next((e for e in manifest['exports'] if e.get('coff', {}).get('symbol') == SYMBOL), None)
    if body is None or ENTRY not in originals:
        raise Unsupported('Missing complete original/compiled descriptor bodies')
    graphs = (link([body], int(body['entry'], 16)), link([originals[ENTRY]], int(ENTRY, 16)))
    relation = MemoryRelation()
    sizes = dict(parser=(layout['state_parser_size'], 4),
                 descriptor=(layout['state_desc_size'], 28), bank=(128, 128))
    bases = dict(parser=(0x100000, 0x1000), descriptor=(0x110000, 0x2000), bank=(0x120000, 0x3000))
    memories = [z3.Array('descriptor_' + side, z3.BitVecSort(bits), z3.BitVecSort(8))
                for side, bits in [('windows', 64), ('ps2', 32)]]
    for index, side in enumerate(('windows', 'ps2')):
        for name in sizes:
            relation.region(side, name, bases[name][index], sizes[name][index])
        relation.region(side, 'stack', 0x7c00, 0x410, private=True)
    for offset in range(0, 128, 4):
        relation.identity(bases['bank'][0] + offset, bases['bank'][1] + offset)
    relation.identity(*bases['descriptor'])
    relation.field('parser', layout['state_parser_data'], 'parser', 0, 8, pointer=True)
    relation.field('bank', 0, 'bank', 0, 128)
    ids = {key: z3.BitVec('descriptor_' + key, 32) for key in ('animation', 'original_animation')}
    header_offset = z3.BitVec('descriptor_last_header_offset', 32)
    domain = z3.And(header_offset >= table_words * 4, header_offset <= 116, header_offset & 3 == 0)
    if table_words == 1:
        domain = z3.And(domain, header_offset == 4)
    payload = z3.BitVec('descriptor_bank_bytes', 1024)
    for index in range(2):
        memories[index] = store(memories[index], bases['parser'][index] + (layout['state_parser_data'] if index == 0 else 0),
                                z3.BitVecVal(bases['bank'][index], 64 if index == 0 else 32))
        memories[index] = store(memories[index], bases['bank'][index], payload)
        memories[index] = store(memories[index], bases['bank'][index], z3.BitVecVal(table_words * 4, 32))
        if table_words > 1:
            memories[index] = store(memories[index], bases['bank'][index] + (table_words - 1) * 4, header_offset)
    for key, ps2_offset in PS2_FIELDS.items():
        pointer = key.startswith('header_')
        relation.field('descriptor', layout['state_desc_' + key], 'descriptor', ps2_offset,
                       8 if pointer else 4, pointer=pointer)
        initial = z3.BitVecVal(0, 32) if pointer else z3.BitVec('descriptor_initial_' + key, 32)
        memories[0] = store(memories[0], bases['descriptor'][0] + layout['state_desc_' + key],
                            z3.ZeroExt(32, initial) if pointer else initial)
        memories[1] = store(memories[1], bases['descriptor'][1] + ps2_offset, initial)
    relation.require_satisfiable(domain)
    prove(relation.condition(*memories), domain, 'Descriptor input relation not established')
    entries = [ExecutionState('windows', memories[0], {
        ('register', 0x10, 8): z3.BitVecVal(bases['descriptor'][0], 64),
        ('register', 0x88, 8): z3.ZeroExt(32, ids['original_animation'])}),
        ExecutionState('ps2', memories[1], {
        ('register', 0x50, 8): z3.BitVecVal(bases['descriptor'][1], 64),
        ('register', 0x70, 8): z3.SignExt(32, ids['original_animation'])})]
    step = TransitionStep('BuildAnimDesc', *graphs, None)
    results = []
    for index, side in enumerate(('windows', 'ps2')):
        bits = 64 if index == 0 else 32
        options = dict(this_argument=z3.BitVecVal(bases['parser'][index], 64),
                       second_argument=z3.ZeroExt(32, ids['animation']),
                       regions=[(r.base, r.size) for r in relation.regions[side].values()])
        if index:
            options.update(graph_mode=True, call_mode=True)
        results.append(outcomes(graphs[index], invocation(entries[index]), None, domain, options, step))
    pairs = 0
    for w in results[0]:
        for s in results[1]:
            claims = [relation.condition(w.state.memory, s.state.memory, outputs=True)]
            feasible = z3.And(domain, w.state.condition, s.state.condition)
            for index, (side, outcome) in enumerate((('windows', w), ('ps2', s))):
                bits = 64 if index == 0 else 32
                claims.append(outcome.return_value == bases['descriptor'][index])
                claims.append(relation.writes_within_frame(side, outcome.writes))
                claims.append(load(outcome.state.memory, bases['bank'][index], 128) == load(memories[index], bases['bank'][index], 128))
                parser_offset = layout['state_parser_data'] if index == 0 else 0
                claims.append(load(outcome.state.memory, bases['parser'][index] + parser_offset, 8 if index == 0 else 4) == bases['bank'][index])
                expected = dict(field=0, animation=ids['animation'], flags=0, max_key=table_words - 1,
                                original_animation=ids['original_animation'],
                                header_a=z3.BitVecVal(bases['bank'][index], bits) + (z3.ZeroExt(32, header_offset) if index == 0 else header_offset),
                                header_b=bases['bank'][index] + table_words * 4)
                for key, value in expected.items():
                    offset = layout['state_desc_' + key] if index == 0 else PS2_FIELDS[key]
                    claims.append(load(outcome.state.memory, bases['descriptor'][index] + offset,
                                       8 if index == 0 and key.startswith('header_') else 4) == value)
            solver = z3.Solver()
            solver.set(timeout=30000)
            solver.add(feasible, z3.Not(z3.And(*claims)))
            answer = solver.check()
            if answer == z3.unknown:
                return dict(result='inconclusive', reason=solver.reason_unknown())
            if answer == z3.sat:
                return dict(result='mismatch', table_words=table_words,
                            counterexample=str(solver.model()))
            pairs += 1
    return dict(result='equivalent', scope='Bounded real descriptor parser; not animation playback or full fighter coverage',
                table_words=table_words, paired_outcomes=pairs, reference_models=[], numeric_policies=[],
                original_entry=ENTRY, full_fighter_contract=False,
                restrictions=['128-byte disjoint bank; 1..4 offset-table words',
                              'Aligned final header offset inside the bank; other header bytes arbitrary',
                              'Initial descriptor header pointers null; scalar fields and animation IDs arbitrary',
                              'Parser only; no layer playback, morphing or callbacks'])


def main():
    import argparse
    import json
    from pathlib import Path
    from fighter_transitions import load_artifacts, retain_fixture
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--artifacts', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--retain-fixture', type=Path)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.unlink(missing_ok=True)
    manifest, originals, _ = load_artifacts(args.artifacts)
    try:
        cases = [compare(manifest, originals, table_words=n) for n in range(1, 5)]
        verdict = ('mismatch' if any(c['result'] == 'mismatch' for c in cases) else
                   'equivalent' if all(c['result'] == 'equivalent' for c in cases) else 'inconclusive')
        report = dict(result=verdict, cases=cases, full_fighter_contract=False,
                      bundle_sha256=manifest['bundle_sha256'],
                      executable_sha256=originals[ENTRY]['executable_sha256'])
    except Unsupported as error:
        report = dict(result='unsupported', reason=str(error), full_fighter_contract=False)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    if args.retain_fixture and report['result'] == 'equivalent':
        retain_fixture(args.artifacts, args.retain_fixture)
    print(json.dumps(report))
    return 0 if report['result'] == 'equivalent' else 1


if __name__ == '__main__':
    raise SystemExit(main())
