"""Bounded real queue/start/interruption sequences, with retained runtime storage."""
from verify import Unsupported
import z3
from memory_relation import load, store
from transitions import TransitionStep, invocation, outcomes
from ee_interpreter import MODEL

INIT = '?MorphingInitDT@edAnmLayer@@QEAA_NPEAUedAnmStateDesc@@@Z'
START = '?MorphingStartDT@edAnmLayer@@QEAA_NXZ'
INIT_ENTRY, START_ENTRY = '002411d0', '002410e0'
# Explicit runtime inputs, not claims about TheAnimStage's constructor.
SETTINGS = dict(keep=(-6, -1), restart=(-4, 0), halfway=(500, 500), end=(1000, 1000))


def seed_settings(manifest, relation, memories, setting):
    stage = manifest['globals']['stage']
    bases = (manifest['addresses'][stage['symbol']], 0x466cd0)
    for index, side in enumerate(('windows', 'ps2')):
        relation.region(side, 'morph_settings', bases[index], 12)
    relation.field('morph_settings', 0, 'morph_settings', 0, 12)
    for index in range(2):
        for offset, value in ((0, SETTINGS[setting][0]), (4, SETTINGS[setting][1]), (8, 0x3e800000)):
            memories[index] = store(memories[index], bases[index] + offset, z3.BitVecVal(value, 32))
    # Constants are compiler-image backed by load_artifacts/validate_artifacts.
    symbol = '__real@447a0000'
    address = manifest['addresses'].get(symbol)
    if address is None:
        raise Unsupported('Missing compiler-backed milliseconds divisor')
    relation.region('windows', 'morph_divisor', address, 4, private=True)
    memories[0] = store(memories[0], address, z3.BitVecVal(0x447a0000, 32))
    return memories


def continue_sequence(manifest, relation, bases, sizes, fields, pairs, domain, wg, sg, ids, setting, initial):
    from animation_playback import PARSER, LAYER, float_bits
    layout = manifest['layouts']['Animation']
    reports = []
    mode, milliseconds = SETTINGS[setting]
    normalized = max(0, milliseconds) / 1000
    current_normalized = 1 if mode == -4 else max(0, mode) / 1000
    stages = [('BuildNextDescriptor', PARSER, 1), ('QueueDifferentAnimation', LAYER, 1),
              ('StartMorph', START, 1), ('RepeatQueuedAnimation', LAYER, 1),
              ('BuildInterruptedDescriptor', PARSER, 2), ('InterruptMorph', LAYER, 2)]
    for stage, symbol, increment in stages:
        following = []
        step = TransitionStep(stage, wg[symbol], sg[symbol], None)
        for previous in pairs:
            paired = z3.And(domain, *[state.condition for state in previous])
            results = []
            for index, side in enumerate(('windows', 'ps2')):
                options = dict(this_argument=z3.BitVecVal(bases['parser' if symbol == PARSER else 'layer'][index], 64),
                    regions=[(r.base, r.size) for r in relation.regions[side].values()], fp_mode=True)
                if symbol == PARSER:
                    options['second_argument'] = z3.ZeroExt(32, ids['animation'] + increment)
                if index:
                    options.update(graph_mode=True, call_mode=True)
                argument = None if symbol == START else z3.BitVecVal(bases['descriptor'][index], 64)
                entry = invocation(previous[index])
                if symbol == PARSER:
                    offset = 0x88 if index == 0 else 0x70
                    for old in list(entry.registers):
                        if old[1] < offset + 8 and offset < old[1] + old[2]:
                            del entry.registers[old]
                    entry.registers[('register', offset, 8)] = (z3.ZeroExt if index == 0 else z3.SignExt)(32, ids['original_animation'])
                results.append(outcomes(wg[symbol] if index == 0 else sg[symbol], entry,
                                        argument, paired, options, step))
            for w in results[0]:
                for s in results[1]:
                    feasible = z3.And(paired, w.state.condition, s.state.condition)
                    solver = z3.Solver()
                    solver.set(timeout=30000)
                    solver.add(feasible)
                    answer = solver.check()
                    if answer == z3.unsat:
                        continue
                    if answer != z3.sat:
                        raise Unsupported('Morph sequence pairing inconclusive')
                    claims = [relation.condition(w.state.memory, s.state.memory, outputs=True)]
                    for index, (side, outcome) in enumerate((('windows', w), ('ps2', s))):
                        claims.append(relation.writes_within_frame(side, outcome.writes))
                        def layer(key, memory=None):
                            pointer = key.split('_')[-1] in ('pHdrA', 'pHdrB', 'pAnimKeyTableEntry',
                                'pKeyDataArray', 'pFunction', 'pActor', 'pAnimManagerKeyData') or key == 'pFunction_0xc0'
                            return load(outcome.state.memory if memory is None else memory,
                                bases['layer'][index] + (layout['playback_' + key] if index == 0 else fields[key]),
                                8 if pointer and index == 0 else 4)
                        for region in ('parser', 'bank', 'keys', 'data'):
                            claims.append(load(outcome.state.memory, bases[region][index], sizes[region][index]) ==
                                          load(initial[index], bases[region][index], sizes[region][index]))
                        for region in ('morph_settings',) + (('morph_divisor',) if index == 0 else ()):
                            r = relation.regions[side][region]
                            claims.append(load(outcome.state.memory, r.base, r.size) == load(initial[index], r.base, r.size))
                        if symbol == PARSER:
                            claims += [outcome.return_value == bases['descriptor'][index]]
                            claims += [layer(key) == layer(key, previous[index].memory) for key in fields]
                        else:
                            claims.append(load(outcome.state.memory, bases['descriptor'][index], sizes['descriptor'][index]) ==
                                          load(previous[index].memory, bases['descriptor'][index], sizes['descriptor'][index]))
                            if symbol == START:
                                claims.append(z3.Extract(7, 0, outcome.return_value) == 1)
                            if stage == 'RepeatQueuedAnimation':
                                claims += [layer(key) == layer(key, previous[index].memory) for key in fields]
                            else:
                                expected = dict(animPlayState=1 if stage == 'QueueDifferentAnimation' else 2,
                                    currentAnimDesc_animType=ids['animation'] + (1 if stage == 'InterruptMorph' else 0),
                                    nextAnimDesc_animType=ids['animation'] + increment,
                                    nextAnimDesc_animMode=(0xfffffffa if stage == 'InterruptMorph' else mode & 0xffffffff),
                                    field_0xb8=milliseconds & 0xffffffff, field_0xbc=0x3e800000,
                                    currentAnimDesc_animMode=0)
                                if stage != 'QueueDifferentAnimation':
                                    key_start = z3.If(layer('currentAnimDesc_state_currentAnimDataFlags') & 1 == 0,
                                                     z3.BitVecVal(float_bits(1), 32), z3.BitVecVal(float_bits(2), 32))
                                    expected.update(nextAnimDesc_morphDuration=0,
                                        nextAnimDesc_state_time_0xc=float_bits(normalized),
                                        currentAnimDesc_state_time_0xc=float_bits(normalized if stage == 'InterruptMorph' else current_normalized))
                                    for desc in ('currentAnimDesc', 'nextAnimDesc'):
                                        t = normalized if desc == 'nextAnimDesc' or stage == 'InterruptMorph' else current_normalized
                                        expected[desc + '_state_time_0x10'] = z3.If(key_start == float_bits(1),
                                            z3.BitVecVal(float_bits(t), 32), z3.BitVecVal(float_bits(2 * t), 32))
                                        expected[desc + '_state_keyStartTime_0x14'] = key_start
                                        expected[desc + '_state_keyDuration_0x18'] = float_bits(1)
                                claims += [layer(key) == value for key, value in expected.items()]
                    solver.add(z3.Not(z3.And(*claims)))
                    answer = solver.check()
                    if answer == z3.unknown:
                        return dict(result='inconclusive', stage=stage, reason=solver.reason_unknown())
                    if answer == z3.sat:
                        return dict(result='mismatch', stage=stage, setting=setting, counterexample=str(solver.model()))
                    following.append((w.state, s.state))
        if not following:
            raise Unsupported('Empty morph sequence stage')
        reports.append(dict(name=stage, paired_outcomes=len(following)))
        pairs = following
    references = sorted({r for pair in pairs for state in pair for r in state.processor.get('reference_models', ())})
    return dict(result='conditional-equivalent', steps=reports, setting=setting, reference_models=references,
        numeric_policies={}, full_fighter_contract=False,
        restrictions=['One layer, one shared single-key header, distinct symbolic animation IDs',
            'Explicit default-stage mode/time partition; zero transition records',
            'Queue, start, same-next replay, and interrupt without frame advance',
            'Header flags arbitrary; opaque actor/callback copied without invocation',
            'Exact equality conditional on pinned COP1 interpreter; no full fighter claim'])


def main():
    import argparse
    import json
    import sys
    from pathlib import Path
    from animation_playback import compare
    from fighter_transitions import input_identity, load_artifacts, retain_fixture
    from proof_policy import validate_result
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--artifacts', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--reference-model', choices=(MODEL,), required=True)
    parser.add_argument('--retain-fixture', type=Path)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.unlink(missing_ok=True)
    manifest, originals, _ = load_artifacts(args.artifacts)
    identity = input_identity(manifest, originals, 'pcsx2-reference', None)
    try:
        cases = []
        for case in ([dict(morph=setting) for setting in SETTINGS] +
                     [dict(callback=True, blend='two'), dict(actor_entry=True)]):
            print('Animation composition: ' + str(case), file=sys.stderr, flush=True)
            cases.append(compare(manifest, originals, reference_model=args.reference_model, **case))
        verdict = ('mismatch' if any(c['result'] == 'mismatch' for c in cases) else
                   'conditional-equivalent' if all(c['result'] == 'conditional-equivalent' for c in cases) else 'inconclusive')
        report = dict(result=verdict, cases=cases, full_fighter_contract=False,
            scope='Bounded morph sequences, fighter callback weight update and actor/controller playback',
            proof_policy='pcsx2-reference', numeric_policies={},
            reference_models=sorted({r for c in cases for r in c.get('reference_models', ())}),
            approved_reference_models=[args.reference_model], inputs_sha256=identity,
            bundle_sha256=manifest['bundle_sha256'], executable_sha256=originals[START_ENTRY]['executable_sha256'])
        if verdict == 'conditional-equivalent':
            validate_result(report, policy='pcsx2-reference', approved_models=[args.reference_model])
    except Unsupported as error:
        report = dict(result='unsupported', reason=str(error), full_fighter_contract=False)
    current, current_originals, _ = load_artifacts(args.artifacts)
    if input_identity(current, current_originals, 'pcsx2-reference', None) != identity:
        raise Unsupported('Morph/callback proof inputs changed during execution')
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    if args.retain_fixture and report['result'] == 'conditional-equivalent':
        retain_fixture(args.artifacts, args.retain_fixture)
    print(json.dumps(report))
    return 0 if report['result'] == 'conditional-equivalent' else 1


if __name__ == '__main__':
    raise SystemExit(main())
