"""Compose real descriptor construction and stopped-layer playback bodies."""
from verify import Unsupported
import struct
from fractions import Fraction
import z3
from animation_descriptor import SYMBOL as PARSER, ENTRY as PARSER_ENTRY, PS2_FIELDS
from call_graph import link
from ee_interpreter import MODEL
from execution_state import ExecutionState
from memory_relation import MemoryRelation, load, store
from transitions import TransitionStep, invocation, outcomes, prove


LAYER = '?SetAnim@edAnmLayer@@QEAAXPEAUedAnmStateDesc@@@Z'
INITIALIZE = '?Initialize@edAnmMacroAnimator@@QEAAXMPEAUedANM_HDR@@_NI@Z'
UPDATE = '?UpdateAnimParams@edAnmMacroAnimator@@QEAAXXZ'
UPDATE_ENTRY = '00241b40'
LAYER_ENTRY, INITIALIZE_ENTRY = '00240530', '00241ce0'
TIMINGS = ((0, 0), (0, 2), (1, 2), (2, 1), (-1, 1))
# Finite dyadic domains: input, intermediate and normalized values are exact
# binary32 numbers. These are data partitions, never callee summaries.
BLENDS = dict(empty=((), ()), two=((0, 2), (1, 1)),
              three=((2, 0, 1), (1, 2, 1)), zero=((1, 2), (0, 0)),
              negative=((0, 2), (-1, -1)), alias=((0, 0), (1, 1)),
              cancelling=((0, 2), (1, -1)))


def partitions():
    return ([dict(timing=t, key_index=i) for t in TIMINGS for i in range(3)] +
            [dict(blend=b) for b in BLENDS] +
            [dict(blend=b, timing=t) for b in ('two', 'three', 'alias') for t in ((-1, 1), (2, 1))])


def float_bits(value):
    return int.from_bytes(struct.pack('<f', value), 'little')


def layer_fields():
    """PS2 offsets: SetAnim 00240530 and Initialize 00241ce0 disassembly.

    Include inactive members as preservation observations; never equate padding.
    """
    fields = dict(blendOp=0, blendWeight=4, animPlayState=8, field_0xc=12,
                  field_0xb8=0xb8, field_0xbc=0xbc, pFunction_0xc0=0xc0,
                  pActor=0xc4, pAnimManagerKeyData=0xc8, field_0xcc=0xcc,
                  playRate=0xd0, field_0xd4=0xd4)
    desc = dict(animType=0, pHdrA=4, flags=8, maxKey_0xc=12, pHdrB=16,
                origAnimType=20, morphDuration=0x4c, animMode=0x50)
    macro = dict(flags=0, currentAnimDataFlags=4, field_0x8=8, time_0xc=12,
                 time_0x10=16, keyStartTime_0x14=20, keyDuration_0x18=24,
                 pAnimKeyTableEntry=28, pKeyDataArray=32, pFunction=36,
                 pActor=40, animationType=44, playingDuration=48)
    for name, base in [('currentAnimDesc', 0x10), ('nextAnimDesc', 0x64)]:
        fields.update({name + '_' + key: base + offset for key, offset in desc.items()})
        fields.update({name + '_state_' + key: base + 24 + offset for key, offset in macro.items()})
    return fields


def compare(manifest, originals, *, timing=(1, 2), key_index=0, blend=None, reference_model=None,
            morph=None, callback=False, actor_entry=False):
    """All animation IDs/header flags, exact finite timing partitions.

    Reference arithmetic is opt-in, propagated by the executor and reported as
    conditional equivalence. No arithmetic result or callee is summarized.
    """
    if timing not in TIMINGS:
        raise ValueError('Unsupported stopped-playback timing partition')
    if type(key_index) is not int or not 0 <= key_index < 3:
        raise ValueError('Key index must be 0..2')
    if blend is not None and (blend not in BLENDS or key_index != 0):
        raise ValueError('Unsupported blend partition or unused key index')
    if reference_model != MODEL:
        raise Unsupported('Stopped playback requires explicit pinned COP1 interpreter opt-in')
    if morph is not None:
        from animation_morphing import SETTINGS, INIT, START, INIT_ENTRY, START_ENTRY
        if morph not in SETTINGS or timing != (1, 2) or key_index != 0 or blend is not None:
            raise ValueError('Unsupported morph composition domain')
    if type(callback) is not bool or callback and (blend != 'two' or timing != (1, 2) or morph is not None):
        raise ValueError('Unsupported fighter callback composition domain')
    if type(actor_entry) is not bool or actor_entry and (morph is not None or callback or blend is not None or
                                                        timing != (1, 2) or key_index != 0):
        raise ValueError('Unsupported actor playback composition domain')
    layout = manifest['layouts']['Animation']
    fields = layer_fields()
    required = ['playback_' + key for key in fields]
    required += ['state_desc_' + key for key in PS2_FIELDS]
    required += ['layer_size', 'state_desc_size', 'state_parser_size', 'state_parser_data']
    if any(key not in layout for key in required):
        raise Unsupported('Missing compiler-backed stopped-playback layout')
    symbols = (PARSER, LAYER, INITIALIZE) + ((UPDATE,) if blend is not None else ())
    original_entries = (PARSER_ENTRY, LAYER_ENTRY, INITIALIZE_ENTRY) + ((UPDATE_ENTRY,) if blend is not None else ())
    if morph is not None:
        symbols += (INIT, START)
        original_entries += (INIT_ENTRY, START_ENTRY)
    if callback:
        from animation_callbacks import CALLBACK, FIGHTER, BASE, ENTRIES, dispatch
        symbols += (CALLBACK, FIGHTER, BASE)
        original_entries += ENTRIES
    if actor_entry:
        from actor_animation import ACTOR_PLAY, ACTOR_ENTRY, CONTROLLER, META, ORIGINAL_ENTRIES
        symbols += (ACTOR_PLAY, CONTROLLER, META, '?GetIdMacroAnim@CActor@@QEAAHH@Z')
        original_entries += ORIGINAL_ENTRIES
    selected = {e['coff']['symbol']: e for e in manifest['exports'] if e['coff']['symbol'] in symbols}
    if len(selected) != len(symbols) or any(e not in originals for e in original_entries):
        raise Unsupported('Missing complete original/compiled stopped-playback bodies')
    if actor_entry:
        selected = {e['coff']['symbol']: e for e in manifest['exports']
                    if not e['coff']['symbol'].startswith('?DebugAnimTrackFailure@')}
    wi, si = {}, {}
    if callback:
        wi, si, callback_dispatch = dispatch(manifest, selected, originals)
    # Unresolved morphing and callback invocation remain frontiers. They may only
    # be pruned by executing infeasible prefixes under the actual entry domain.
    roots = (PARSER, LAYER) + ((START,) if morph is not None else ())
    if actor_entry:
        roots += (ACTOR_PLAY,)
    wg = {name: link(list(selected.values()), int(selected[name]['entry'], 16),
                     defer_unresolved=True, max_depth=16 if actor_entry else 8,
                     indirect_targets=wi if name != PARSER else {}) for name in roots}
    sg = {name: link([originals[e] for e in original_entries],
                     int(entry, 16), defer_unresolved=True, max_depth=8, indirect_targets=si if name != PARSER else {})
          for name, entry in ((PARSER, PARSER_ENTRY), (LAYER, LAYER_ENTRY)) +
          (((START, START_ENTRY),) if morph is not None else ()) +
          (((ACTOR_PLAY, ACTOR_ENTRY),) if actor_entry else ())}
    relation = MemoryRelation()
    bases = dict(parser=(0x100000, 0x1000), descriptor=(0x110000, 0x110000),
                 bank=(0x120000, 0x3000), layer=(0x160000, 0xa000),
                 keys=(0x170000, 0xb000), data=(0x180000, 0xc000))
    indices, weights = BLENDS[blend] if blend is not None else ((key_index,), ())
    bank_size = 16 + (8 * len(indices) if blend is not None else 0)
    sizes = dict(parser=(layout['state_parser_size'], 4),
                 descriptor=(layout['state_desc_size'], 28), bank=(bank_size, bank_size),
                 layer=(layout['layer_size'], 0xd8), keys=(24, 12), data=(36, 36))
    memories = [z3.Array('playback_' + side, z3.BitVecSort(bits), z3.BitVecSort(8))
                for side, bits in [('windows', 64), ('ps2', 32)]]
    for index, side in enumerate(('windows', 'ps2')):
        for name in bases:
            relation.region(side, name, bases[name][index], sizes[name][index])
        relation.region(side, 'stack', 0x7c00, 0x410, private=True)
    for name, pair in bases.items():
        relation.identity(*pair)
    for record in (1, 2):
        relation.identity(bases['data'][0] + record * 12, bases['data'][1] + record * 12)
    relation.identity(bases['bank'][0] + 4, bases['bank'][1] + 4)
    relation.identity(0x190000, 0xd000)  # Opaque callback, copied but never invoked.
    relation.identity(0x1a0000, 0xe000)  # Opaque actor, copied but never accessed.
    callback_pointers = (0x190000, 0xd000)
    if callback:
        from animation_callbacks import seed
        memories, callback_pointers = seed(manifest, relation, memories, callback_dispatch)

    def field(region, wo, so, values, pointer=False):
        relation.field(region, wo, region, so, 8 if pointer else 4, pointer=pointer)
        for index in range(2):
            width = 64 if pointer and index == 0 else 32
            value = values[index]
            value = z3.BitVecVal(value, width) if isinstance(value, int) else value
            memories[index] = store(memories[index], bases[region][index] + (wo if index == 0 else so), value)

    field('parser', layout['state_parser_data'], 0, bases['bank'], True)
    relation.field('bank', 0, 'bank', 0, bank_size)
    flags = z3.BitVec('playback_bank_flags', 32)
    header = [(0, 4), (4, flags), (8, int(blend is not None)),
              (12, len(indices) if blend is not None else key_index)]
    if blend is not None:
        header += [(16 + i * 4, value) for i, value in enumerate(indices)]
        header += [(16 + (len(indices) + i) * 4, float_bits(value)) for i, value in enumerate(weights)]
    for offset, value in header:
        for index in range(2):
            memories[index] = store(memories[index], bases['bank'][index] + offset,
                                    z3.BitVecVal(value, 32) if isinstance(value, int) else value)
    for record in range(3):
        field('keys', record * 8, record * 4, tuple(base + record * 12 for base in bases['data']), True)
    relation.field('data', 0, 'data', 0, 36)
    start, end = timing
    records = [(start + 2 * i, end + 4 * i) for i in range(3)]
    if blend is None:
        records[key_index] = timing
        # Keep every unselected record distinct from the selected timing.
        for record in range(3):
            if record != key_index:
                records[record] = (10 + record, 20 + record)
    for index in range(2):
        for record, (a, b) in enumerate(records):
            for offset, value in [(0, record), (4, float_bits(b)), (8, float_bits(a))]:
                memories[index] = store(memories[index], bases['data'][index] + record * 12 + offset, z3.BitVecVal(value, 32))
    if blend is None:
        expected_start, expected_end, expected_duration = max(0, start), max(0, end), max(0, end - start)
    else:
        expected_weights = (Fraction(3, 4), Fraction(1, 4)) if callback else weights
        total = sum(expected_weights)
        denominator = total if total > 0 else 1
        expected_start, expected_end = [max(0, sum(Fraction(records[i][component] * w, denominator)
                                                   for i, w in zip(indices, expected_weights))) for component in (0, 1)]
        expected_duration = max(0, sum(Fraction((records[i][1] - records[i][0]) * w, denominator)
                                       for i, w in zip(indices, expected_weights)))
    for key, so in PS2_FIELDS.items():
        pointer = key.startswith('header_')
        value = 0 if pointer else z3.BitVec('playback_descriptor_initial_' + key, 32)
        field('descriptor', layout['state_desc_' + key], so, (value, value), pointer)
    expected = [{}, {}]
    for key, so in fields.items():
        pointer = key.split('_')[-1] in ('pHdrA', 'pHdrB', 'pAnimKeyTableEntry',
                                         'pKeyDataArray', 'pFunction', 'pActor',
                                         'pAnimManagerKeyData') or key == 'pFunction_0xc0'
        values = ((0, 0) if pointer else (z3.BitVec('playback_initial_' + key, 32),) * 2)
        if key == 'animPlayState':
            values = (0, 0)
        elif key == 'pAnimManagerKeyData':
            values = bases['keys']
        elif key == 'pFunction_0xc0':
            values = callback_pointers
        elif key == 'pActor':
            values = (0x1a0000, 0xe000)
        field('layer', layout['playback_' + key], so, values, pointer)
        for index in range(2):
            expected[index][key] = (values[index], pointer)
    ids = {key: z3.BitVec('playback_' + key, 32) for key in ('animation', 'original_animation')}
    if actor_entry:
        ids['animation'] = z3.BitVecVal(0, 32)
    incoming = {('register', offset, 8): z3.BitVec('playback_incoming_gpr_' + str(offset), 64)
                for offset in (0, 8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
                               0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8)}
    incoming.update({('register', 0x1200 + index * 0x20, 16):
                     z3.BitVec('playback_incoming_xmm_' + str(index), 128) for index in range(16)})
    incoming[('register', 0x88, 8)] = z3.ZeroExt(32, ids['original_animation'])
    entries = [ExecutionState('windows', memories[0], incoming,
        processor={'mxcsr': 0x1f80}),
        ExecutionState('ps2', memories[1], {
        ('register', 0x70, 8): z3.SignExt(32, ids['original_animation']),
        ('register', 0x110, 16): z3.BitVec('playback_incoming_s1', 128),
        ('register', 0x1054, 4): z3.BitVec('playback_incoming_f20', 32)},
        processor={'ee_arithmetic_model': reference_model, 'fcr31': 0x01000001})]
    if callback or actor_entry:
        for offset in range(0x100, 0x180, 0x10):
            entries[1].registers[('register', offset, 16)] = z3.BitVec('callback_incoming_s_' + str(offset), 128)
        for offset in range(0x1050, 0x1080, 4):
            entries[1].registers[('register', offset, 4)] = z3.BitVec('callback_incoming_f_' + str(offset), 32)
    if morph is not None:
        from animation_morphing import seed_settings
        memories = seed_settings(manifest, relation, memories, morph)
        for index in range(2):
            entries[index].memory = memories[index]
    if actor_entry:
        from actor_animation import seed
        memories = seed(manifest, relation, memories, bases, sizes, ids['original_animation'])
        for index in range(2):
            entries[index].memory = memories[index]
    # Nonnull opaque callbacks cannot be invoked without a checked body. The
    # high flag bypasses blending entirely, outside this weighted-data domain.
    domain = flags & 0x80000004 == 0 if blend is not None else z3.BoolVal(True)
    if callback:
        domain = z3.And(flags & 0x80000004 == 4, ids['original_animation'] == 0x27)
    if actor_entry:
        domain = z3.And(domain, ids['original_animation'] != -1)
    relation.require_satisfiable(domain)
    prove(relation.condition(*memories), domain, 'Stopped-playback inputs not related')
    pairs, reports = [tuple(entries)], []
    stages = ([('ActorStartPlayback', ACTOR_PLAY), ('ActorRepeatPlayback', ACTOR_PLAY)] if actor_entry else
              [('BuildAnimDesc', PARSER), ('StartStoppedLayer', LAYER), ('ReplaySameAnimation', LAYER)])
    for stage, symbol in stages:
        following = []
        step = TransitionStep(stage, wg[symbol], sg[symbol], None)
        if actor_entry:
            step.loop_visits = 5
        for previous in pairs:
            paired = z3.And(domain, *[e.condition for e in previous])
            results = []
            for index, side in enumerate(('windows', 'ps2')):
                options = dict(this_argument=z3.BitVecVal(bases['parser' if symbol == PARSER else 'layer'][index], 64),
                               regions=[(r.base, r.size) for r in relation.regions[side].values()], fp_mode=True)
                if symbol == PARSER:
                    options['second_argument'] = z3.ZeroExt(32, ids['animation'])
                if actor_entry:
                    options['this_argument'] = z3.BitVecVal(0x1a0000 if index == 0 else 0xe000, 64)
                    options['regional_initial_stores'] = True
                if index:
                    options.update(graph_mode=True, call_mode=True)
                results.append(outcomes(wg[symbol] if index == 0 else sg[symbol], invocation(previous[index]),
                                        (z3.ZeroExt if index == 0 else z3.SignExt)(32, ids['original_animation']) if actor_entry else
                                        z3.BitVecVal(bases['descriptor'][index], 64), paired, options, step))
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
                        raise Unsupported('Stopped-playback pairing inconclusive')
                    observed = [w.state.memory, s.state.memory]
                    if actor_entry:
                        from actor_animation import gameplay_memory
                        observed = [gameplay_memory(relation, side, outcome, feasible)
                                    for side, outcome in (('windows', w), ('ps2', s))]
                    claims = [relation.condition(*observed, outputs=True)]
                    for index, (side, outcome) in enumerate((('windows', w), ('ps2', s))):
                        claims.append(relation.writes_within_frame(side, outcome.writes))
                        if symbol == PARSER:
                            claims.append(outcome.return_value == bases['descriptor'][index])
                            claims.append(load(observed[index], bases['layer'][index], sizes['layer'][index]) ==
                                          load(memories[index], bases['layer'][index], sizes['layer'][index]))
                        for region in ('parser', 'bank', 'keys', 'data'):
                            expected_memory = memories[index]
                            if callback and symbol != PARSER and region == 'bank':
                                for offset, value in ((24, 0x3f400000), (28, 0x3e800000)):
                                    expected_memory = store(expected_memory, bases['bank'][index] + offset, z3.BitVecVal(value, 32))
                            claims.append(load(observed[index], bases[region][index], sizes[region][index]) ==
                                          load(expected_memory, bases[region][index], sizes[region][index]))
                        if callback:
                            from animation_callbacks import preservation
                            claims += preservation(relation, side, memories[index], outcome.state.memory)
                        if actor_entry:
                            from actor_animation import observations
                            claims += observations(manifest, relation, bases, memories[index], outcome.state.memory,
                                                   side, ids['original_animation'], outcome.writes, observed[index], feasible)
                        if symbol != PARSER:
                            # The descriptor is the actual parser output retained
                            # in memory, never reconstructed between invocations.
                            claims.append(load(observed[index], bases['descriptor'][index], sizes['descriptor'][index]) ==
                                          load(previous[index].memory, bases['descriptor'][index], sizes['descriptor'][index]))
                            updates = dict(animPlayState=1, field_0xc=0, nextAnimDesc_animType=0xffffffff)
                            current = dict(animType=ids['animation'], pHdrA=bases['bank'][index] + 4,
                                           flags=0, maxKey_0xc=0, pHdrB=bases['bank'][index] + 4,
                                           origAnimType=ids['original_animation'])
                            macro = dict(flags=0, currentAnimDataFlags=flags, field_0x8=0,
                                         time_0xc=0, time_0x10=0,
                                         keyStartTime_0x14=z3.If(flags & 1 == 0,
                                             z3.BitVecVal(float_bits(expected_start), 32),
                                             z3.BitVecVal(float_bits(expected_end), 32)),
                                         keyDuration_0x18=float_bits(expected_duration),
                                         pAnimKeyTableEntry=bases['bank'][index] + 4,
                                         pKeyDataArray=bases['keys'][index],
                                         pFunction=expected[index]['pFunction_0xc0'][0],
                                         pActor=expected[index]['pActor'][0],
                                         animationType=ids['original_animation'], playingDuration=0)
                            updates.update({'currentAnimDesc_' + k: v for k, v in current.items()})
                            updates.update({'currentAnimDesc_state_' + k: v for k, v in macro.items()})
                            for key, (initial, pointer) in expected[index].items():
                                offset = layout['playback_' + key] if index == 0 else fields[key]
                                claims.append(load(observed[index], bases['layer'][index] + offset,
                                                   8 if pointer and index == 0 else 4) == updates.get(key, initial))
                    if actor_entry:
                        pending, atoms = list(reversed(claims)), []
                        while pending:
                            claim = z3.simplify(pending.pop())
                            if z3.is_and(claim):
                                pending.extend(reversed(claim.children()))
                            elif not z3.is_true(claim):
                                atoms.append(claim)
                        answer = z3.unsat
                        for observation, claim in enumerate(atoms):
                            solver.push()
                            solver.add(z3.Not(claim))
                            answer = solver.check()
                            if answer != z3.unsat:
                                break
                            solver.pop()
                    else:
                        solver.add(z3.Not(z3.And(*claims)))
                        answer = solver.check()
                    if answer == z3.unknown:
                        return dict(result='inconclusive', stage=stage, reason=solver.reason_unknown(),
                                    observation=observation if actor_entry else None)
                    if answer == z3.sat:
                        return dict(result='mismatch', stage=stage, timing=timing, key_index=key_index,
                                    blend=blend, counterexample=str(solver.model()),
                                    observation=observation if actor_entry else None)
                    following.append((w.state, s.state))
        if not following:
            raise Unsupported('Empty stopped-playback stage')
        reports.append(dict(name=stage, paired_outcomes=len(following)))
        pairs = following
    if morph is not None:
        from animation_morphing import continue_sequence
        result = continue_sequence(manifest, relation, bases, sizes, fields, pairs, domain,
                                   wg, sg, ids, morph, memories)
        result['steps'] = reports + result.get('steps', [])
        result['original_entries'] = list(original_entries)
        return result
    references = sorted({r for pair in pairs for e in pair for r in e.processor.get('reference_models', ())})
    return dict(result='conditional-equivalent' if references else 'equivalent', steps=reports,
                timing=timing, key_index=key_index, blend=blend, key_indices=indices, blend_weights=weights,
                callback=callback, actor_entry=actor_entry,
                reference_models=references, numeric_policies={}, full_fighter_contract=False,
                original_entries=list(original_entries),
                restrictions=['One stopped layer; one macro header and three distinct key records',
                              'Selected key index 0..2 or bounded 0..3 blend terms; selected partition only',
                              'Blended inputs require callback set and high bypass clear' if callback else
                              'Blended inputs require callback and high bypass flags clear' if blend is not None else
                              'Single-key header flags arbitrary',
                              'One macro lookup entry selects macro zero; original ID arbitrary except -1; no event tracks' if actor_entry else
                              'Original animation fixed at 0x27; arbitrary macro animation ID; fixed timing partition' if callback else
                              'Arbitrary animation IDs; header flags subject to stated restrictions; fixed timing partition',
                              'Real layer-zero callback and fighter weight update for original animation 0x27' if callback else
                              'Populated actor/controller/macro bank; opaque callback copied without invocation' if actor_entry else
                              'Opaque nonnull actor/callback copied without access or invocation',
                              'Real actor lookup/controller/meta/parser/layer playback with exact Windows diagnostic history' if actor_entry else
                              'Parser, playback start and same-animation replay only; no morphing or frame advance',
                              'Conditional on pinned PCSX2 COP1 interpreter arithmetic'])


def main():
    import argparse
    import json
    import sys
    from pathlib import Path
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
    inputs_hash = input_identity(manifest, originals, 'pcsx2-reference', None)
    try:
        cases, selected_cases = [], partitions()
        for index, case in enumerate(selected_cases):
            print(f'Playback partition {index + 1}/{len(selected_cases)}: {case}', file=sys.stderr, flush=True)
            cases.append(compare(manifest, originals, reference_model=args.reference_model, **case))
        verdict = ('mismatch' if any(c['result'] == 'mismatch' for c in cases) else
                   'conditional-equivalent' if all(c['result'] == 'conditional-equivalent' for c in cases) else 'inconclusive')
        report = dict(result=verdict, cases=cases, full_fighter_contract=False,
                      scope='Bounded descriptor construction, stopped-layer start and same-animation replay',
                      proof_policy='pcsx2-reference', numeric_policies={},
                      reference_models=sorted({r for c in cases for r in c.get('reference_models', ())}),
                      approved_reference_models=[args.reference_model], inputs_sha256=inputs_hash,
                      bundle_sha256=manifest['bundle_sha256'],
                      executable_sha256=originals[LAYER_ENTRY]['executable_sha256'])
        if verdict == 'conditional-equivalent':
            validate_result(report, policy='pcsx2-reference', approved_models=[args.reference_model])
    except Unsupported as error:
        report = dict(result='unsupported', reason=str(error), full_fighter_contract=False)
    current_manifest, current_originals, _ = load_artifacts(args.artifacts)
    if input_identity(current_manifest, current_originals, 'pcsx2-reference', None) != inputs_hash:
        raise Unsupported('Stopped-playback proof inputs changed during execution')
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    if args.retain_fixture and report['result'] == 'conditional-equivalent':
        retain_fixture(args.artifacts, args.retain_fixture)
    print(json.dumps(report))
    return 0 if report['result'] == 'conditional-equivalent' else 1


if __name__ == '__main__':
    raise SystemExit(main())
