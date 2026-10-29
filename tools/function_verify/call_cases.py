"""One audited direct-call graph: caller -> thunk -> base implementation."""
import copy
import hashlib
import json
import re
import z3
from verify import execute, Unsupported, request
from paths import expand
from indexed_cases import SOURCE as BASE_SOURCE

SOURCE = '''uint CActorAutonomous::GetBehaviourFlags(int state) { uint behaviourFlags;
if (state < 2) { behaviourFlags = CActor::GetBehaviourFlags(state); } else {
assert(state == 2); behaviourFlags = _gBehaviourFlags_AUT[state - 2]; } return behaviourFlags; }'''
ENTRIES = ['00123c50', '0011e790', '001022b0']


def link(exports):
    """Allow one direct call, one tail thunk, no recursion or indirect dispatch."""
    caller, thunk, callee = copy.deepcopy(exports)
    if [e['entry'] for e in exports] != ENTRIES or len({e['executable_sha256'] for e in exports}) != 1:
        raise Unsupported('Unexpected call graph identity')
    for export in exports:
        if not export.get('complete') or export['operation_count'] != sum(len(i['ops']) for i in export['instructions']):
            raise Unsupported('Incomplete call graph')
        if export['language'] != caller['language'] or export['schema'] != caller['schema']:
            raise Unsupported('Incompatible callee export')
    calls = [(i, op) for i in caller['instructions'] for op in i['ops'] if op['opcode'] == 'CALL']
    if len(calls) != 1 or calls[0][1]['inputs'] != [dict(space='ram', offset='11e790', size=4, constant=False)]:
        raise Unsupported('Unapproved direct call')
    calls[0][1]['opcode'] = 'BRANCH'
    for instruction in callee['instructions']:
        for index, op in enumerate(instruction['ops']):
            if op['opcode'] in ('CALL', 'CALLIND'):
                raise Unsupported('Nested call unsupported')
            if op['opcode'] == 'RETURN':
                instruction['ops'][index:] = [dict(opcode='CHECK_TARGET', inputs=op['inputs'], expected=0x123c80),
                    dict(opcode='BRANCH', inputs=[dict(space='ram', offset='123c80', size=4, constant=False)])]
                break
    # External branches are resolved only through these explicitly listed bodies.
    caller['instructions'] += thunk['instructions'] + callee['instructions']
    caller['operation_count'] = sum(len(i['ops']) for i in caller['instructions'])
    return caller


def compare(exports, mutation=False):
    state = z3.BitVec('state', 32)
    domain = z3.And(state >= -1, state <= 2)
    initial = z3.Array('memory', z3.BitVecSort(32), z3.BitVecSort(8))
    def word(address):
        return z3.Concat(*[initial[address + i] for i in (3, 2, 1, 0)])
    expected = z3.If(state == -1, z3.BitVecVal(0, 32),
                    z3.If(state == 2, word(0x428e24 if mutation else 0x428e20), word(0x427d80 + state * 4)))
    outcomes = [execute(path, z3.SignExt(32, state), initial, path_mode=True, precondition=domain,
                        regions=[(0x427d80, 8), (0x428e20, 4), (0x8000, 16)], call_mode=True)
                for path in expand(link(exports))]
    solver = z3.Solver()
    solver.set(timeout=5000)
    solver.add(domain, z3.Not(z3.Or(*[guard for _, _, guard in outcomes])))
    if solver.check() != z3.unsat:
        raise Unsupported('Call graph coverage not proven')
    # Private stack-frame bytes may change. Every other address must be preserved.
    witness = z3.BitVec('observable_address', 32)
    observable = z3.Or(z3.ULT(witness, 0x8000), z3.UGE(witness, 0x8010))
    solver.reset()
    solver.set(timeout=5000)
    solver.add(domain, z3.Or(*[z3.And(guard, z3.Or(z3.Extract(31, 0, value) != expected,
                        z3.And(observable, memory[witness] != initial[witness]))) for value, memory, guard in outcomes]))
    answer = solver.check()
    result = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(answer)], paths=len(outcomes))
    if answer == z3.sat:
        result['counterexample'] = dict(state=solver.model().eval(state).as_signed_long())
    elif answer == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result


def run(root, output, port):
    report = dict(address=ENTRIES[0], function='CActorAutonomous::GetBehaviourFlags',
                  source='src/b-witch/ActorAutonomous.cpp', callees=ENTRIES[1:],
                  domain='signed state in {-1, 0, 1, 2}', stack='private 16-byte frame at 0x8000',
                  tables='arbitrary uint32 values: ACT at 0x427d80 (2), AUT at 0x428e20 (1)')
    try:
        normalize = lambda s: re.sub(r'\s+', '', s)
        for filename, body in [('ActorAutonomous.cpp', SOURCE), ('Actor.cpp', BASE_SOURCE)]:
            if normalize((root / 'src/b-witch' / filename).read_text()).count(normalize(body)) != 1:
                raise Unsupported('Source outside audited call model')
        exports = [request('export_pcode', port, address=address) for address in ENTRIES]
        for export in exports:
            (output / (export['entry'] + '.json')).write_text(json.dumps(export, indent=2), encoding='utf-8')
        report.update(compare(exports))
        report['negative_control'] = compare(exports, mutation=True)
        if report['negative_control']['result'] != 'mismatch':
            raise Unsupported('Wrong AUT table mapping not detected')
        report['source_body_sha256'] = hashlib.sha256(SOURCE.encode()).hexdigest()
        report['executable_sha256'] = exports[0]['executable_sha256']
    except (Unsupported, RuntimeError, OSError, ValueError, KeyError, z3.Z3Exception) as error:
        report.update(result='inconclusive', reason=str(error))
    return [report]
