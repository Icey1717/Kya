"""Bounded symbolic table access, with solver-proven address bounds."""
import hashlib
import json
import re
import z3
from verify import execute, Unsupported, request
from paths import expand

ADDRESS = '001022b0'
SOURCE = '''uint CActor::GetBehaviourFlags(int state) { uint uVar1; assert(state < 2);
if (state == -1) { uVar1 = 0; } else { uVar1 = _gBehaviourFlags_ACT[state]; } return uVar1; }'''


def compare(export, mutation=False, unrestricted=False):
    state = z3.BitVec('state', 32)
    domain = z3.BoolVal(True) if unrestricted else z3.And(state >= -1, state < 2)
    table = z3.Array('table_bytes', z3.BitVecSort(32), z3.BitVecSort(8))
    offset = state * 4
    field = z3.Concat(*[table[offset + i] for i in (3, 2, 1, 0)])
    expected = z3.If(state == -1, z3.BitVecVal(1 if mutation else 0, 32), field)
    outcomes = [execute(path, z3.SignExt(32, state), table, memory_base=0x427d80,
                        object_size=8, path_mode=True, precondition=domain) for path in expand(export)]
    solver = z3.Solver()
    solver.set(timeout=5000)
    solver.add(domain, z3.Not(z3.Or(*[guard for _, _, guard in outcomes])))
    if solver.check() != z3.unsat:
        raise Unsupported('Input coverage not proven')
    solver.reset()
    solver.set(timeout=5000)
    solver.add(domain, z3.Or(*[z3.And(guard, z3.Or(z3.Extract(31, 0, value) != expected, memory != table))
                              for value, memory, guard in outcomes]))
    answer = solver.check()
    report = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(answer)],
                  paths=len(outcomes), domain='signed int state in {-1, 0, 1}', address_bounds='proven per load')
    if answer == z3.sat:
        report['counterexample'] = {'state': solver.model().eval(state).as_signed_long()}
    elif answer == z3.unknown:
        report['reason'] = solver.reason_unknown()
    return report


def run(root, output, port):
    report = dict(address=ADDRESS, function='CActor::GetBehaviourFlags', source='src/b-witch/Actor.cpp',
                  table_mapping='two arbitrary uint32 entries at PS2 0x427d80',
                  assumptions=['sign-extended int argument in a1', 'valid states -1, 0, 1',
                               'assert is a source precondition; table contents are symbolic'])
    try:
        source = (root / report['source']).read_text(encoding='utf-8')
        normalize = lambda text: re.sub(r'\s+', '', text)
        if normalize(source).count(normalize(SOURCE)) != 1:
            raise Unsupported('Source outside audited indexed-access model')
        export = request('export_pcode', port, address=ADDRESS)
        if export['entry'] != ADDRESS or export['name'] != report['function']:
            raise Unsupported('Function identity mismatch')
        (output / (ADDRESS + '.json')).write_text(json.dumps(export, indent=2), encoding='utf-8')
        report.update(compare(export))
        report['negative_control'] = compare(export, mutation=True)
        if report['negative_control']['result'] != 'mismatch':
            raise Unsupported('Sentinel mutation not detected')
        try:
            compare(export, unrestricted=True)
        except Unsupported as error:
            report['unrestricted_domain_control'] = str(error)
        else:
            raise Unsupported('Unrestricted index should fail bounds proof')
        report['source_body_sha256'] = hashlib.sha256(SOURCE.encode()).hexdigest()
        report['executable_sha256'] = export['executable_sha256']
    except (Unsupported, RuntimeError, OSError, ValueError, KeyError, z3.Z3Exception) as error:
        report.update(result='inconclusive', reason=str(error))
    return [report]
