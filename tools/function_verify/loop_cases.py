"""Bounded search loop, with explicit proof that no cutoff is reachable."""
import hashlib
import json
import re
import z3
from verify import execute, Unsupported, request
from paths import expand

ADDRESS = '00131f10'
SOURCE = '''inline bool IsInList(T value) { for (int i = 0; i < this->nbEntries; i++) {
if (this->aEntries[i] == value) { return true; } } return false; }'''


def compare(export, visits=4, mutation=False):
    memory = z3.Array('table', z3.BitVecSort(32), z3.BitVecSort(8))
    query = z3.BitVec('query', 32)
    def word(offset):
        return z3.Concat(*[memory[z3.BitVecVal(offset + byte, 32)] for byte in (3, 2, 1, 0)])
    count = word(0)
    domain = z3.And(count >= 0, count <= 4)
    matches = [z3.And(count > i, word(4 + i * 4) == query) for i in range(3 if mutation else 4)]
    expected = z3.If(z3.Or(*matches), z3.BitVecVal(1, 32), z3.BitVecVal(0, 32))
    outcomes, frontiers = [], []
    for path in expand(export, loop_visits=visits, max_paths=128):
        value, final, guard = execute(path, z3.SignExt(32, query), memory, object_size=20,
                                      precondition=domain, path_mode=True, loop_mode=True)
        if path.get('cutoff'):
            frontiers.append(guard)
        else:
            outcomes.append((value, final, guard))
    solver = z3.Solver()
    solver.set(timeout=5000)
    solver.add(domain, z3.Or(*frontiers))
    if solver.check() != z3.unsat:
        raise Unsupported('Loop bound insufficient: reachable or unproven cutoff')
    solver.reset()
    solver.set(timeout=5000)
    solver.add(domain, z3.Not(z3.Or(*[guard for _, _, guard in outcomes])))
    if solver.check() != z3.unsat:
        raise Unsupported('Loop path coverage not proven')
    solver.reset()
    solver.set(timeout=5000)
    solver.add(domain, z3.Or(*[z3.And(guard, z3.Or(z3.Extract(31, 0, value) != expected, final != memory))
                              for value, final, guard in outcomes]))
    answer = solver.check()
    result = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(answer)],
                  returning_paths=len(outcomes), cutoff_paths=len(frontiers), loop_visits=visits,
                  cutoff_proof='unreachable for all valid inputs', domain='0 <= nbEntries <= 4')
    if answer == z3.sat:
        model = solver.model()
        result['counterexample'] = dict(count=model.eval(count).as_long(), query=hex(model.eval(query).as_long()),
                                       entries=[hex(model.eval(word(4 + i * 4), model_completion=True).as_long()) for i in range(4)])
    elif answer == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result


def run(root, output, port):
    report = dict(address=ADDRESS, function='CFixedTable<CActor,4>::IsInList', source='src/b-witch/Types.h',
                  mapping='PS2 20-byte object: signed count at 0, four 32-bit pointer values at 4',
                  assumptions=['valid count 0..4', 'pointer identity represented by arbitrary 32-bit values',
                               'a1 sign-extended 32-bit pointer value; no pointer dereference'])
    try:
        normalize = lambda text: re.sub(r'\s+', '', text)
        if normalize((root / report['source']).read_text()).count(normalize(SOURCE)) != 1:
            raise Unsupported('Source outside audited loop model')
        export = request('export_pcode', port, address=ADDRESS)
        if export['entry'] != ADDRESS or export['name'] != report['function']:
            raise Unsupported('Function identity mismatch')
        (output / (ADDRESS + '.json')).write_text(json.dumps(export, indent=2), encoding='utf-8')
        report.update(compare(export))
        report['negative_control'] = compare(export, mutation=True)
        if report['negative_control']['result'] != 'mismatch':
            raise Unsupported('Missing-last-entry mutation not detected')
        try:
            compare(export, visits=3)
        except Unsupported as error:
            report['insufficient_bound_control'] = str(error)
        else:
            raise Unsupported('Insufficient loop bound not detected')
        report['source_body_sha256'] = hashlib.sha256(SOURCE.encode()).hexdigest()
        report['executable_sha256'] = export['executable_sha256']
    except (Unsupported, RuntimeError, OSError, ValueError, KeyError, z3.Z3Exception) as error:
        report.update(result='inconclusive', reason=str(error))
    return [report]
