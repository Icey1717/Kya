"""All-state inventory and complete-body transition checks with explicit coverage gaps."""
import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import sys

from verify import Unsupported
import z3
from execution_state import ExecutionState
from fighter_cases import cases, configuration, digest, parse_pair, parse_shard
from fighter_domains import ProofDomain, RESOURCE_PROFILES, BEHAVIOUR_PROFILES, full_contract, job_document, read_cached
from fighter_runtime import graphs, inputs
from memory_relation import store
from proof_policy import POLICIES, load_reference, validate_result
from transitions import TransitionStep, invocation, outcomes, prove


ROOT = Path(__file__).resolve().parents[2]


def input_identity(manifest, originals, policy, reference_manifest):
    return digest(dict(manifest=manifest, originals=originals, proof_policy=policy,
        reference_manifest=reference_manifest.read_text() if reference_manifest else None,
        tools={str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
               for path in Path(__file__).parent.glob('*.py')}))


def load_artifacts(folder):
    folder = Path(folder)
    manifest = json.loads((folder / 'manifest.json').read_text())
    manifest.update(json.loads((folder / 'exports.json').read_text()))
    originals = json.loads((folder / 'originals.json').read_text())
    blob = (folder / 'functions.bin').read_bytes()
    return validate_artifacts(manifest, originals, blob)


def validate_artifacts(manifest, originals, blob):
    from compiled_extended import sha
    if manifest['bundle_sha256'] != sha(blob) or any(e['executable_sha256'] != sha(blob) for e in manifest['exports']):
        raise Unsupported('Transition exports do not match the compiled image')
    if not originals or len({e['executable_sha256'] for e in originals.values()}) != 1:
        raise Unsupported('Mixed or missing PS2 transition executable identities')
    for symbol, address in manifest['addresses'].items():
        literal = re.fullmatch(r'__(?:real@([0-9a-fA-F]{8})|xmm@([0-9a-fA-F]{32}))', symbol)
        if literal:
            raw_literal = int(literal[1] or literal[2], 16).to_bytes(4 if literal[1] else 16, 'little')
            if blob[address:address + len(raw_literal)] != raw_literal:
                raise Unsupported('Compiler literal does not match the retained compiled image')
    raw = bytes.fromhex(manifest['configuration']['initialized_bytes_hex'])
    configuration(raw)
    return manifest, originals, raw


def retain_fixture(folder, destination):
    import gzip
    manifest, originals, _ = load_artifacts(folder)
    document = dict(schema='kya.fighter-state-fixture.v1', manifest=manifest, originals=originals,
                    blob_hex=(Path(folder) / 'functions.bin').read_bytes().hex(),
                    scope='Complete selected production bodies; runtime/resource domain must be established separately')
    Path(destination).write_bytes(gzip.compress(json.dumps(document, separators=(',', ':')).encode(), mtime=0))


def load_fixture(path):
    import gzip
    document = json.loads(gzip.decompress(Path(path).read_bytes()))
    if document.get('schema') != 'kya.fighter-state-fixture.v1':
        raise Unsupported('Unknown retained fighter transition schema')
    return validate_artifacts(document['manifest'], document['originals'], bytes.fromhex(document['blob_hex']))


def private_writes(relation, side, writes):
    return relation.writes_within_frame(side, writes)


def base_transition(manifest, originals, *, old_states=(6,), new_states=(9,), animation='configured', idle_loops='zero',
                    live_behaviour=False, progress=None, reference_model=None, resources='null', function='SetState',
                    sequence_states=None, registry=0, behaviour_profile=None):
    """Execute actual transition bodies in an explicit bounded runtime domain."""
    if registry not in (0, 1):
        raise ValueError('Unknown behaviour registry selector')
    if behaviour_profile is None:
        behaviour_profile = ('fighter-transient-slot-0' if registry else 'fighter-slot-0') if live_behaviour else 'empty'
    profile = ProofDomain(resources, behaviour_profile)
    live_behaviour = behaviour_profile != 'empty'
    registry = int(behaviour_profile == 'fighter-transient-slot-0')
    general_bindings = behaviour_profile in ('fighter-general-one', 'fighter-general-two')
    behaviour_count = 2 if behaviour_profile == 'fighter-general-two' else int(live_behaviour)
    if sequence_states is not None:
        if (function != 'SetState' or animation != 'configured' or len(sequence_states) < 2
                or tuple(old_states) != (sequence_states[0],) or tuple(new_states) != (sequence_states[1],)
                or any(type(value) is not int or not 6 <= value <= 113 for value in sequence_states)):
            raise ValueError('Sequences require configured SetState calls and matching first state pair')
    if resources == 'populated':
        raise Unsupported('Populated animation controller/bank/layer input relation is not established')
    if function not in ('SetState', 'SetStandAnim'):
        raise ValueError('Unknown fighter transition function')
    animation_modes = ('configured', 'explicit-unchanged', 'explicit-changed') if function == 'SetState' else ('stand-unchanged', 'stand-changed')
    if animation not in animation_modes or idle_loops not in ('zero', 'nonzero'):
        raise ValueError('Invalid animation/idle-loop partition')
    if reference_model is not None:
        from ee_interpreter import MODEL
        if reference_model != MODEL:
            raise ValueError('Unknown transition processor reference profile')
    data = inputs(manifest, count=behaviour_count, transient=registry, states=old_states, configured=True,
                  concrete_bindings=live_behaviour, fighter_object=live_behaviour or function == 'SetStandAnim',
                  general_bindings=general_bindings)
    wm, sm = data['windows'].memory, data['ps2'].memory
    for symbol, address in manifest['addresses'].items():
        literal = re.fullmatch(r'__(?:real@([0-9a-fA-F]{8})|xmm@([0-9a-fA-F]{32}))', symbol)
        if literal:
            size = 4 if literal[1] else 16
            wm = store(wm, address, z3.BitVecVal(int(literal[1] or literal[2], 16), size * 8))
            data['regions']['windows'].append((address, size))
    layout, relation = manifest['layouts']['Actor'], data['relation']
    new = z3.BitVecVal(new_states[0], 32) if len(new_states) == 1 else z3.BitVec('transition_new_state', 32)
    claims = [data['domain'], z3.Or(*[new == value for value in new_states])]
    if not new_states or any(type(value) is not int or not 6 <= value <= 113 for value in new_states):
        raise ValueError('New states must be inside 6..113')
    definitions = [('state', 0x104, data['state']), ('animation', 0x11c, data['animation']),
                   ('previous_state', 0x108, z3.BitVec('transition_previous_state', 32)),
                   ('air_time', 0x10c, z3.BitVec('transition_air_time', 32)),
                   ('idle_timer', 0x110, z3.BitVec('transition_idle_timer', 32))]
    for key, ps2_offset, value in definitions:
        wm = store(wm, 0x100000 + layout['actor_' + key], value)
        sm = store(sm, 0x1000 + ps2_offset, value)
        relation.field('actor', layout['actor_' + key], 'actor', ps2_offset, 4)
    loops = z3.BitVec('transition_idle_loops', 8)
    claims.append(loops == 0 if idle_loops == 'zero' else loops != 0)
    wm = store(wm, 0x100000 + layout['actor_idle_loops'], z3.SignExt(24, loops))
    sm = store(sm, 0x1012, loops)
    relation.field('actor', layout['actor_idle_loops'], 'actor', 0x12, 4, ps2_size=1, signed=True)
    for key, offset in [('animation_controller', 0x28), ('macro_table', 0x114)]:
        present = resources == ('controller-only' if key == 'animation_controller' else 'macro-table-only')
        wp, sp = (0x150000, 0x6000) if present else (0, 0)
        if present:
            # Opaque identity: a reachable dereference still fails bounds checks.
            # A non-null pointer alone is not a valid controller/bank invariant.
            relation.identity(wp, sp)
        wm = store(wm, 0x100000 + layout['actor_' + key], z3.BitVecVal(wp, 64))
        sm = store(sm, 0x1000 + offset, z3.BitVecVal(sp, 32))
        relation.field('actor', layout['actor_' + key], 'actor', offset, 8, pointer=True)
    if live_behaviour:
        fighter = manifest['layouts']['ActorFighter']
        for key, offset, size in [('fight_flags', 0x368, 4), ('flags_44c', 0x44c, 4),
                                  ('speed', 0x1a8, 4), ('excluded_count', 0x638, 4), ('vector_4e0', 0x4e0, 16)]:
            value = z3.BitVec('fighter_initial_' + key, size * 8)
            wm = store(wm, 0x100000 + fighter['fighter_' + key], value)
            sm = store(sm, 0x1000 + offset, value)
            relation.field('actor', fighter['fighter_' + key], 'actor', offset, size)
        # One registered, live exact CBehaviourFighter. Generation/refcount and
        # unrelated registry slots stay symbolic; the selected pointer is concrete.
        if not general_bindings:
            wm = store(wm, 0x100000 + layout['actor_behaviour'], z3.BitVecVal(3, 32))
            sm = store(sm, 0x10fc, z3.BitVecVal(3, 32))
            wm = store(wm, 0x110004, z3.BitVecVal(3, 32))
            sm = store(sm, 0x3004, z3.BitVecVal(3, 32))
        relation.identity(0x100000, 0x1000)
        # General handles may alias either of the two concrete fighter objects,
        # including when the list has only one entry, or resolve to null.
        for index in range(2 if general_bindings else 1):
            name, wp, sp = 'behaviour_' + str(index), 0x120000 + index * 0x1000, 0x4000 + index * 0x1000
            relation.region('windows', name, wp, fighter['behaviour_size'])
            relation.region('ps2', name, sp, 8)
            relation.field(name, fighter['behaviour_owner'], name, 4, 8, pointer=True)
            wm = store(wm, wp + fighter['behaviour_owner'], z3.BitVecVal(0x100000, 64))
            sm = store(sm, sp + 4, z3.BitVecVal(0x1000, 32))
            wm = store(wm, wp, z3.BitVecVal(0x140000, 64))
            sm = store(sm, sp, z3.BitVecVal(0x441a30, 32))
            data['regions']['windows'].append((wp, fighter['behaviour_size']))
            data['regions']['ps2'].append((sp, 8))
        for row in manifest['dispatch']['mappings']:
            if row['concrete_type'] == 'CBehaviourFighter':
                target = next(e for e in manifest['exports'] if e['coff']['symbol'] == row['windows_symbol'])
                wm = store(wm, 0x140000 + row['windows_offset'], z3.BitVecVal(int(target['entry'], 16), 64))
                sm = store(sm, 0x441a30 + row['ps2_offset'], z3.BitVecVal(int(row['ps2_target'], 16), 32))
        data['regions']['windows'].append((0x140000, 0x100))
        data['regions']['ps2'].append((0x441a30, 0x60))
        zero = manifest['globals']['zero_vector']
        raw = bytes.fromhex(zero['initial_bytes_hex'])
        ps2_zero = manifest['ps2_data']['zero_vector']
        if len(raw) != 16 or len(bytes.fromhex(ps2_zero['hex'])) != 16:
            raise Unsupported('Incomplete zero-vector global evidence')
        wb, sb = manifest['addresses'][zero['symbol']], int(ps2_zero['address'], 16)
        wm = store(wm, wb, z3.BitVecVal(int.from_bytes(raw, 'little'), 128))
        sm = store(sm, sb, z3.BitVecVal(int.from_bytes(bytes.fromhex(ps2_zero['hex']), 'little'), 128))
        data['regions']['windows'].append((wb, 16))
        data['regions']['ps2'].append((sb, 16))
        for compiled_function in manifest['functions']:
            for table in compiled_function.get('jump_tables', []):
                base, length = table['address'], table['entries'] * 4
                raw = bytes.fromhex(manifest['jump_table_bytes'][table['symbol']])
                if len(raw) != length:
                    raise Unsupported('Incomplete jump table memory image')
                wm = store(wm, base, z3.BitVecVal(int.from_bytes(raw, 'little'), length * 8))
                data['regions']['windows'].append((base, length))
    argument = z3.BitVecVal(-1, 32) if animation == 'configured' else data['animation']
    if animation == 'explicit-changed':
        argument = z3.BitVec('transition_explicit_animation', 32)
        claims.extend([argument != -1, argument != data['animation']])
    if function == 'SetStandAnim':
        if tuple(old_states) != tuple(new_states):
            raise ValueError('SetStandAnim inventory must retain the entry state')
        fighter = manifest['layouts']['ActorFighter']
        stand = z3.BitVec('transition_stand_animation', 32)
        argument = stand if animation == 'stand-unchanged' else z3.BitVec('transition_new_stand_animation', 32)
        if animation == 'stand-changed':
            claims.append(argument != stand)
        for key, offset, value in [('stand_animation', 0x480, stand),
                                   ('weight', 0x1b8, z3.BitVec('transition_weight', 32))]:
            if 'fighter_' + key not in fighter:
                raise Unsupported('Missing compiled fighter layout: ' + key)
            wm = store(wm, 0x100000 + fighter['fighter_' + key], value)
            sm = store(sm, 0x1000 + offset, value)
            relation.field('actor', fighter['fighter_' + key], 'actor', offset, 4)
        mapping = [r for r in manifest['dispatch']['mappings'] if r['ps2_target'] == '0011e7c0']
        if len(mapping) != 1:
            raise Unsupported('Missing concrete fighter SetState dispatch evidence')
        dispatch = mapping[0]
        target = next((e for e in manifest['exports'] if e['coff']['symbol'] == dispatch['windows_symbol']), None)
        if target is None:
            raise Unsupported('Missing compiled movable SetState body')
        wm = store(wm, 0x130000 + dispatch['windows_offset'], z3.BitVecVal(int(target['entry'], 16), 64))
        sm = store(sm, 0x441a90 + dispatch['ps2_offset'], z3.BitVecVal(0x11e7c0, 32))
    if resources == 'populated-reset':
        from fighter_animation import seed_reset
        wm, sm = seed_reset(manifest, data, wm, sm)
    domain = z3.And(*claims)
    relation.require_satisfiable(domain)
    prove(relation.condition(wm, sm), domain, 'Transition initial field relation not established')
    if live_behaviour or function == 'SetStandAnim' or resources == 'populated-reset':
        from fighter_behaviour import graphs as behaviour_graphs
        wg, sg = behaviour_graphs(manifest, originals, function)
    else:
        wg, sg = graphs(manifest, originals, 'SetState', configured=True, all_config_calls=True)
    wo = dict(this_argument=z3.BitVecVal(0x100000, 64), second_argument=z3.ZeroExt(32, argument),
              regions=data['regions']['windows'], write_regions=[(0x7c00, 0x200)] +
              [(row[0], row[2][0]) for row in relation.fields])
    so = dict(this_argument=z3.BitVecVal(0x1000, 64), second_argument=z3.SignExt(32, argument),
              regions=data['regions']['ps2'], graph_mode=True, call_mode=True, loop_mode=True,
              fp_mode=live_behaviour)
    step = TransitionStep('fighter ' + function, wg, sg,
                          z3.SignExt(32, new if function == 'SetState' else argument), progress=progress)
    if reference_model is not None:
        wo['fp_mode'] = True
        so['fp_mode'] = True
    ps2_processor = {'ee_arithmetic_model': reference_model} if reference_model is not None else {}
    if sequence_states is not None:
        from transitions import compare
        from memory_relation import load
        steps = []
        for previous, following in zip(sequence_states, sequence_states[1:]):
            def entry_condition(w, s, previous=previous):
                return z3.And(load(w.memory, 0x100000 + layout['actor_state'], 4) == previous,
                              load(s.memory, 0x1104, 4) == previous)
            steps.append(TransitionStep(f'SetState {previous}:{following}', wg, sg,
                z3.BitVecVal(following, 64), windows_options=wo, ps2_options=so,
                precondition=entry_condition, progress=progress))
        # compare retains actual memory, path conditions and processor state.
        result = compare(steps, ExecutionState('windows', wm, processor={'mxcsr': 0x1f80}),
                         ExecutionState('ps2', sm, data['ps2'].registers, processor=ps2_processor), relation, domain,
                         frame_by_writes=True)
        result.update(sequence_states=list(sequence_states), domain_profile=profile.document(),
                      restrictions=profile.restrictions, obligations={}, full_fighter_contract=False)
        return result
    windows = outcomes(wg, invocation(ExecutionState('windows', wm, processor={'mxcsr': 0x1f80})),
                       step.argument, domain, wo, step)
    ps2 = outcomes(sg, invocation(ExecutionState('ps2', sm, data['ps2'].registers, processor=ps2_processor)),
                   step.argument, domain, so, step)
    pairs = 0
    for w in windows:
        for s in ps2:
            feasible = z3.And(domain, w.state.condition, s.state.condition)
            solver = z3.Solver()
            solver.set(timeout=30000)
            solver.add(feasible)
            answer = solver.check()
            if answer == z3.unsat:
                continue
            if answer != z3.sat:
                raise Unsupported('Transition pair feasibility inconclusive')
            claim = z3.And(relation.condition(w.state.memory, s.state.memory),
                           private_writes(relation, 'windows', w.writes), private_writes(relation, 'ps2', s.writes))
            solver.add(z3.Not(claim))
            answer = solver.check()
            if answer == z3.unknown:
                raise Unsupported('Transition memory/frame comparison inconclusive')
            if answer == z3.sat:
                model = solver.model()
                return dict(result='mismatch', counterexample=dict(
                    old_state=str(model.eval(data['state'])), new_state=str(model.eval(new))))
            pairs += 1
    if not pairs:
        raise Unsupported('No feasible transition output pairs')
    references = sorted({model for outcome in windows + ps2 for model in outcome.state.processor.get('reference_models', ())})
    return dict(result='conditional-equivalent' if references else 'equivalent', paired_outcomes=pairs,
                old_states=list(old_states), new_states=list(new_states),
                animation=animation, idle_loops=idle_loops, reference_models=references, numeric_policies={},
                domain='Behaviour profile: ' + behaviour_profile +
                       '; animation resources: ' + resources + '; current animation from old-state configuration',
                domain_profile=profile.document(), restrictions=profile.restrictions, obligations={},
                full_fighter_contract=False)


def guard_inventory(path):
    """Syntactic obligation inventory, never a reachability or implementation proof."""
    raw = Path(path).read_text()
    masked = re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                    lambda match: ''.join('\n' if c == '\n' else ' ' for c in match[0]), raw, flags=re.S)
    function, label, rows = None, None, []
    for number, line in enumerate(masked.splitlines(), 1):
        signature = re.match(r'^\w[\w\s:*<>]*\s+(\w+::\w+)\([^;]*\)', line)
        if signature:
            function, label = signature[1], None
        case = re.search(r'\bcase\s+(\w+)\s*:', line)
        if case:
            label = case[1]
        if re.search(r'\bIMPLEMENTATION_GUARD\s*\(', line):
            rows.append(dict(function=function, case=label, line=number))
    return dict(scope='Syntactic guard inventory only; no guard is automatically declared unreachable',
                source=str(path), source_sha256=hashlib.sha256(Path(path).read_bytes()).hexdigest(), guards=rows)


def summarize(expected, rows, *, policy='strict', approved_models=(), allowed_numeric_fields=()):
    items = list(expected)
    expected = {case.key: case.document() for case in items}
    if len(expected) != len(items):
        raise ValueError('Expected transition case set contains duplicate identities')
    seen, counts, execution_counts = set(), Counter(), Counter()
    for row in rows:
        key = row.get('case_id')
        if key not in expected or key in seen or any(row.get(k) != v for k, v in expected[key].items()):
            raise ValueError('Missing, duplicate, stale or unexpected transition case identity')
        seen.add(key)
        execution_counts[row.get('result', 'missing')] += 1
        if not full_contract(row, policy=policy, approved_models=approved_models,
                             allowed_numeric_fields=allowed_numeric_fields):
            counts['incomplete'] += 1
            continue
        try:
            validate_result(row, policy=policy, approved_models=approved_models,
                            allowed_numeric_fields=allowed_numeric_fields)
        except ValueError:
            counts['incomplete'] += 1
        else:
            counts[row['result']] += 1
    if seen != set(expected):
        raise ValueError('Transition report does not cover its complete expected case set')
    return dict(result='passed' if expected and not counts['incomplete'] else 'incomplete',
                proof_policy=policy, cases=len(expected), verdict_counts=dict(counts),
                execution_counts=dict(execution_counts))


def worker_main():
    payload = json.load(sys.stdin)
    manifest, originals, _ = load_artifacts(payload['artifacts'])
    def progress(event):
        with Path(payload['progress']).open('a') as stream:
            stream.write(json.dumps(dict(event, case_id=payload['case_id'], resources=payload.get('resources', 'null'),
                                         live_behaviour=payload['live_behaviour'],
                                         sequence_states=payload.get('sequence_states'))) + '\n')
    try:
        result = base_transition(manifest, originals, old_states=(payload['old_state'],),
                                 new_states=(payload['new_state'],), animation=payload['animation'],
                                 idle_loops=payload['idle_loops'], live_behaviour=payload['live_behaviour'],
                                 progress=progress, reference_model=payload.get('reference_model'),
                                 resources=payload.get('resources', 'null'), function=payload['function'],
                                 sequence_states=payload.get('sequence_states'), registry=payload.get('registry', 0),
                                 behaviour_profile=payload.get('behaviour_profile'))
    except (Unsupported, ValueError, z3.Z3Exception) as error:
        result = dict(result='unsupported', full_fighter_contract=False, reason=str(error))
    print(json.dumps(result), flush=True)


def run_case_worker(artifacts, case, progress, *, live_behaviour=False, timeout=600, reference_model=None,
                    resources='null', sequence_states=None, registry=0, behaviour_profile=None):
    payload = dict(case.document(), artifacts=str(Path(artifacts).resolve()), progress=str(Path(progress).resolve()),
                   live_behaviour=live_behaviour, reference_model=reference_model, resources=resources,
                   sequence_states=sequence_states, registry=registry, behaviour_profile=behaviour_profile)
    try:
        completed = subprocess.run([sys.executable, '-c', 'from fighter_transitions import worker_main; worker_main()'],
                                   cwd=Path(__file__).parent, input=json.dumps(payload), text=True,
                                   capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return dict(result='inconclusive', full_fighter_contract=False,
                    reason='Proof case exceeded wall-clock limit', wall_clock_limit_seconds=timeout)
    if completed.returncode:
        return dict(result='unsupported', full_fighter_contract=False,
                    reason='Proof worker failed: ' + completed.stderr[-4000:])
    try:
        result = json.loads(completed.stdout)
        if not isinstance(result, dict) or result.get('result') not in ('equivalent', 'conditional-equivalent', 'mismatch', 'inconclusive', 'unsupported'):
            raise ValueError('Invalid worker result')
        return result
    except (ValueError, TypeError):
        return dict(result='unsupported', full_fighter_contract=False, reason='Malformed proof worker output')


def main():
    from build_config import arguments
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--artifacts', type=Path, required=True, help='Prepared fighter-runtime artifact folder')
    parser.add_argument('--state-pair', type=parse_pair, action='append', default=[])
    parser.add_argument('--state-family', type=lambda value: int(value, 0))
    parser.add_argument('--shard', type=parse_shard, default=(0, 1))
    parser.add_argument('--proof-policy', choices=POLICIES, default='strict')
    parser.add_argument('--reference-manifest', type=Path)
    parser.add_argument('--resources', choices=RESOURCE_PROFILES, action='append',
                        help='Resource domains to execute (repeatable; default: null)')
    parser.add_argument('--resume', action='store_true', help='Reuse completed jobs with identical content hashes')
    parser.add_argument('--function', choices=('SetState', 'SetStandAnim'))
    parser.add_argument('--animation', choices=('configured', 'explicit-unchanged', 'explicit-changed',
                                               'stand-unchanged', 'stand-changed'))
    parser.add_argument('--idle-loops', choices=('zero', 'nonzero'))
    parser.add_argument('--registry', choices=('persistent', 'transient'), default='persistent',
                        help='Registry for the live fighter in slot 0')
    parser.add_argument('--behaviour-profile', choices=BEHAVIOUR_PROFILES,
                        help='Explicit bounded list/registry profile (overrides --registry)')
    parser.add_argument('--sequence', help='Configured state sequence, e.g. 6:9:6 (2..8 states; zero initial idle loops)')
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument('--inventory-only', action='store_true', help='Write obligations without claiming a proof')
    modes.add_argument('--dormant-actor', action='store_true', help='Execute the restricted null-behaviour/animation subdomain')
    modes.add_argument('--live-behaviour', action='store_true', help='Execute live fighter behaviour with null animation resources')
    parser.add_argument('--case-timeout', type=float, default=600, help='Maximum wall-clock seconds per isolated proof case')
    args = parser.parse_args()
    if args.reference_manifest and args.proof_policy != 'pcsx2-reference':
        parser.error('--reference-manifest requires --proof-policy pcsx2-reference')
    if not math.isfinite(args.case_timeout) or args.case_timeout <= 0:
        parser.error('--case-timeout must be finite and positive')
    approved = load_reference(args.reference_manifest) if args.reference_manifest else ()
    manifest, originals, raw = load_artifacts(args.artifacts)
    resources = args.resources or ['null']
    if len(set(resources)) != len(resources):
        parser.error('Duplicate resource profile')
    sequence_states = None
    if args.sequence:
        try:
            sequence_states = tuple(int(value, 0) for value in args.sequence.split(':'))
        except ValueError:
            parser.error('Sequence requires colon-separated state IDs')
        if (not 2 <= len(sequence_states) <= 8 or any(not 6 <= value <= 113 for value in sequence_states)
                or args.state_pair or args.state_family is not None or args.shard != (0, 1)
                or args.function == 'SetStandAnim' or args.animation or args.idle_loops):
            parser.error('Sequence requires 2..8 valid states and cannot combine with other case selectors')
    inputs_hash = input_identity(manifest, originals, args.proof_policy, args.reference_manifest)

    def check_inputs():
        current_manifest, current_originals, _ = load_artifacts(args.artifacts)
        if input_identity(current_manifest, current_originals, args.proof_policy, args.reference_manifest) != inputs_hash:
            raise Unsupported('Verification inputs changed during the run; restart with fresh job identities')
    selected = list(cases(raw, pairs=args.state_pair, family=args.state_family, shard=args.shard))
    if args.function:
        selected = [case for case in selected if case.function == args.function]
    if args.animation:
        selected = [case for case in selected if case.animation == args.animation]
    if args.idle_loops:
        selected = [case for case in selected if case.idle_loops == args.idle_loops]
    if sequence_states:
        selected = [case for case in cases(raw, pairs=[sequence_states[:2]])
                    if case.function == 'SetState' and case.animation == 'configured' and case.idle_loops == 'zero']
    if not selected:
        parser.error('Selection contains no cases')
    output = args.output_root.resolve() / 'fighter-transitions'
    output.mkdir(parents=True, exist_ok=True)
    (output / 'report.json').unlink(missing_ok=True)
    rows, reports = [], {}
    cache = output / 'jobs'
    cache.mkdir(exist_ok=True)
    progress = output / 'progress.jsonl'
    progress.write_text('')
    with (output / 'cases.jsonl').open('w') as stream:
        for resource in resources:
            behaviour = ('fighter-transient-slot-0' if args.registry == 'transient' else 'fighter-slot-0') if args.live_behaviour else 'empty'
            domain = ProofDomain(resource, args.behaviour_profile or behaviour)
            domain_rows = []
            for index, case in enumerate(selected):
                if not args.inventory_only:
                    check_inputs()
                job = job_document(case, domain, inputs_hash, policy=args.proof_policy,
                                   reference_models=approved, timeout=args.case_timeout, sequence_states=sequence_states)
                cached = read_cached(cache / (job['job_id'] + '.json'), job) if args.resume else None
                # Inventory mode must never reuse an executable proof result.
                row = cached if cached is not None and not args.inventory_only else None
                if row is None:
                    result = dict(result='unsupported', full_fighter_contract=False,
                                  reason='Live behaviour, animation, callback/effect closure and arithmetic obligations remain open')
                    if not args.inventory_only and (args.dormant_actor or args.live_behaviour or args.behaviour_profile):
                        print(f'Case {index + 1}/{len(selected)} [{resource}]: {case.old_state}:{case.new_state} {case.animation} {case.idle_loops}', flush=True)
                        result = run_case_worker(args.artifacts, case, progress, live_behaviour=args.live_behaviour,
                                                 timeout=args.case_timeout, reference_model=approved[0] if approved else None,
                                                 resources=resource, sequence_states=sequence_states,
                                                 registry=int(args.registry == 'transient'),
                                                 behaviour_profile=domain.behaviour)
                        print('  ' + result['result'] + (': ' + result['reason'] if 'reason' in result else ''), flush=True)
                    if not args.inventory_only:
                        check_inputs()
                    result.setdefault('domain_profile', domain.document())
                    result.setdefault('restrictions', domain.restrictions)
                    result.setdefault('obligations', {})
                    row = {**case.document(), **result, **job}
                    if not args.inventory_only:
                        destination = cache / (job['job_id'] + '.json')
                        temporary = destination.with_suffix('.tmp')
                        temporary.write_text(json.dumps(row, sort_keys=True) + '\n')
                        temporary.replace(destination)
                rows.append(row)
                domain_rows.append(row)
                stream.write(json.dumps(row, sort_keys=True) + '\n')
                stream.flush()
            reports[resource] = summarize(selected, domain_rows, policy=args.proof_policy, approved_models=approved)
            reports[resource]['domain_profile'] = domain.document()
    check_inputs()
    report = dict(result='passed' if all(r['result'] == 'passed' for r in reports.values()) else 'incomplete',
                  proof_policy=args.proof_policy, cases=len(selected), jobs=len(rows), domains=reports)
    report.update(scope='All-state case coverage; restricted actor subproofs cannot discharge full fighter contracts',
                  selected_scope=bool(args.state_pair or args.state_family is not None or args.shard != (0, 1)
                                      or args.function or args.animation or args.idle_loops or sequence_states),
                  sequence_states=sequence_states,
                  full_matrix_pairs=108 * 108, required_matrix_cases=108 * 108 * 6 + 108 * 4,
                  inputs_sha256=inputs_hash,
                  implementation_guards=guard_inventory(ROOT / 'src/b-witch/ActorFighter.cpp'))
    (output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return 0 if args.inventory_only or report['result'] == 'passed' else 1


if __name__ == '__main__':
    raise SystemExit(main())
