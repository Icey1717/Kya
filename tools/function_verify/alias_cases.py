"""Two-index swaps, including equal indices and symbolic memory writes."""
import hashlib
import json
import re
import z3
from verify import execute, Unsupported, request

SOURCE = '''inline void Swap(T a, T b) { T tmp;
tmp = (this->aEntries + a + -1)[1];
(this->aEntries + a + -1)[1] = (this->aEntries + b + -1)[1];
(this->aEntries + b + -1)[1] = tmp; }'''
CASES = [('0011ad00', 'CFixedTable<CActor,64>::Swap'), ('002000f0', 'CFixedTable<i,64>::Swap')]


def compare(export, mutation=False, equal=None):
    a, b = z3.BitVecs('index_a index_b', 32)
    domain = z3.And(a >= 1, a < 64, b >= 1, b < 64)
    if equal is not None:
        domain = z3.And(domain, a == b if equal else a != b)
    initial = z3.Array('object', z3.BitVecSort(32), z3.BitVecSort(8))
    offset_a, offset_b = 4 + a * 4, 4 + b * 4
    def read(offset):
        return z3.Concat(*[initial[offset + i] for i in (3, 2, 1, 0)])
    def write(memory, offset, value):
        for i in range(4):
            memory = z3.Store(memory, offset + i, z3.Extract(i * 8 + 7, i * 8, value))
        return memory
    old_a, old_b = read(offset_a), read(offset_b)
    expected = write(initial, offset_a, old_b)
    expected = write(expected, offset_b, old_b if mutation else old_a)
    _, actual = execute(export, z3.SignExt(32, a), initial, object_size=260,
                        precondition=domain, second_argument=z3.SignExt(32, b))
    solver = z3.Solver()
    solver.set(timeout=10000)
    solver.add(domain, actual != expected)
    verdict = solver.check()
    report = dict(result={'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(verdict)],
                  domain='indices 1..63', aliasing='equal and distinct indices included')
    if verdict == z3.sat:
        model = solver.model()
        report['counterexample'] = dict(a=model.eval(a).as_long(), b=model.eval(b).as_long(),
                                       old_a=hex(model.eval(old_a, model_completion=True).as_long()),
                                       old_b=hex(model.eval(old_b, model_completion=True).as_long()))
    elif verdict == z3.unknown:
        report['reason'] = solver.reason_unknown()
    return report


def run(root, output, port):
    reports = []
    for address, name in CASES:
        report = dict(address=address, function=name, source='src/b-witch/Types.h',
                      mapping='260-byte PS2 object: count at 0, 64 four-byte entries at 4',
                      assumptions=['source index parameters modeled as 32-bit integers',
                                   'source Swap intermediate pointer expression excludes index zero',
                                   'pointer specialization models pointer identity bits only'])
        try:
            source = (root / report['source']).read_text()
            normalize = lambda text: re.sub(r'\s+', '', re.sub(r'//[^\n]*', '', text))
            if normalize(source).count(normalize(SOURCE)) != 1:
                raise Unsupported('Source outside audited swap model')
            export = request('export_pcode', port, address=address)
            if export['entry'] != address or export['name'] != name:
                raise Unsupported('Swap identity mismatch')
            (output / (address + '.json')).write_text(json.dumps(export, indent=2), encoding='utf-8')
            report.update(compare(export))
            report['negative_control'] = compare(export, mutation=True)
            if report['negative_control']['result'] != 'mismatch':
                raise Unsupported('Lost temporary mutation not detected')
            report['source_body_sha256'] = hashlib.sha256(SOURCE.encode()).hexdigest()
            report['executable_sha256'] = export['executable_sha256']
        except (Unsupported, RuntimeError, OSError, ValueError, KeyError, z3.Z3Exception) as error:
            report.update(result='inconclusive', reason=str(error))
        reports.append(report)
    return reports
