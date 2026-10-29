"""Audited source models, guarded by exact whitespace-normalized body checks."""
import hashlib
import json
import re
import z3
from paths import expand
from verify import execute, Unsupported, request

CASES = [dict(address='001dc7b0', name='CInventoryInterface::HasChanged', file='ActorHero_Inventory.cpp',
              offset=12, size=16, source='''bool CInventoryInterface::HasChanged() {
bool bChanged; bChanged = this->bDirty != 0; if (bChanged) { this->bDirty = 0; } return bChanged; }'''),
         dict(address='00362f30', name='CActorMoney::GetType', file='ActorMoney.cpp', offset=0x1d0, size=0x1d4,
              source='''int CActorMoney::GetType() { uint moneyType;
if (this->moneyValue < 2) { moneyType = 0; } else { if (this->moneyValue < 6) {
moneyType = 1; } else { moneyType = 2; } } return moneyType; }''')]


def compare(export, case, mutation=False, **limits):
    initial = z3.Array('bytes', z3.BitVecSort(32), z3.BitVecSort(8))
    offset = case['offset']
    field = z3.Concat(*[initial[z3.BitVecVal(offset + i, 32)] for i in (3, 2, 1, 0)])
    expected_memory = initial
    if case['address'] == '001dc7b0':
        expected = z3.If(field != 0, z3.BitVecVal(1, 32), z3.BitVecVal(0, 32))
        if not mutation:
            for i in range(4):
                expected_memory = z3.Store(expected_memory, z3.BitVecVal(offset + i, 32), z3.BitVecVal(0, 8))
    else:
        expected = z3.If(z3.ULT(field, 3 if mutation else 2), z3.BitVecVal(0, 32),
                         z3.If(z3.ULT(field, 6), z3.BitVecVal(1, 32), z3.BitVecVal(2, 32)))
    outcomes = []
    for path in expand(export, **limits):
        value, memory, guard = execute(path, None, initial, object_size=case['size'], path_mode=True)
        if value is None:
            raise Unsupported('Missing return value')
        outcomes.append((value, memory, guard))
    solver = z3.Solver()
    solver.set(timeout=5000)
    solver.add(z3.Not(z3.Or(*[guard for _, _, guard in outcomes])))
    if solver.check() != z3.unsat:
        raise Unsupported('Path coverage not proven')
    solver.reset()
    solver.set(timeout=5000)
    solver.add(z3.Or(*[z3.And(guard, z3.Or(z3.Extract(31, 0, value) != expected, memory != expected_memory))
                       for value, memory, guard in outcomes]))
    verdict = solver.check()
    result = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(verdict)],
                  paths=len(outcomes), coverage='all inputs', limits=dict(max_paths=limits.get('max_paths', 32),
                                                                         max_instructions=128))
    if verdict == z3.sat:
        model = solver.model()
        result['counterexample'] = dict(field=hex(model.eval(field, model_completion=True).as_long()))
    elif verdict == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result


def run(root, output, port):
    reports = []
    for case in CASES:
        report = dict(address=case['address'], function=case['name'], source='src/b-witch/' + case['file'],
                      field_offset=case['offset'], object_size=case['size'])
        try:
            source = (root / report['source']).read_text(encoding='utf-8')
            normalize = lambda text: re.sub(r'\s+', '', text)
            if normalize(source).count(normalize(case['source'])) != 1:
                raise Unsupported('Source changed outside audited branch model')
            export = request('export_pcode', port, address=case['address'])
            if export['entry'] != case['address'] or export['name'] != case['name']:
                raise Unsupported('Function identity mismatch')
            (output / (case['address'] + '.json')).write_text(json.dumps(export, indent=2), encoding='utf-8')
            report.update(compare(export, case))
            report['negative_control'] = compare(export, case, mutation=True)
            if report['negative_control']['result'] != 'mismatch':
                raise Unsupported('Negative control failed')
            report['source_body_sha256'] = hashlib.sha256(case['source'].encode()).hexdigest()
            report['executable_sha256'] = export['executable_sha256']
        except (Unsupported, RuntimeError, OSError, ValueError, KeyError, z3.Z3Exception) as error:
            report.update(result='inconclusive', reason=str(error))
        reports.append(report)
    return reports
