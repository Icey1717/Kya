"""Read/write object cases with explicit PS2-to-source field mapping."""
import hashlib
import json
import re
import z3

CASES = [('001003f0', 'IsLookingAt', False),
         ('00100400', 'SetLookingAtOn', True),
         ('00100410', 'SetLookingAtOff', True)]


def compare(export, operator, mask, setter, offset=8):
    from verify import execute, Unsupported
    initial = z3.Array('object_bytes', z3.BitVecSort(32), z3.BitVecSort(8))
    value = z3.Concat(*[initial[z3.BitVecVal(offset + byte, 32)] for byte in (3, 2, 1, 0)])
    expected = value & mask if operator == '&' else value | mask
    actual, final = execute(export, None, initial)
    expected_memory = initial
    if setter:
        for byte in range(4):
            expected_memory = z3.Store(expected_memory, z3.BitVecVal(offset + byte, 32),
                                       z3.Extract(byte * 8 + 7, byte * 8, expected))
        different = final != expected_memory
    else:
        if actual is None:
            raise Unsupported('Missing getter result')
        different = z3.Or(z3.Extract(31, 0, actual) != expected, final != initial)
    solver = z3.Solver()
    solver.set(timeout=5000)
    solver.add(different)
    answer = solver.check()
    result = {'result': {'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(answer)]}
    if answer == z3.sat:
        model = solver.model()
        def bytes_of(memory):
            return [model.eval(memory[z3.BitVecVal(i, 32)], model_completion=True).as_long() for i in range(16)]
        result['counterexample'] = {'initial_bytes': bytes_of(initial), 'ps2_bytes': bytes_of(final),
                                   'source_bytes': bytes_of(expected_memory)}
        if not setter:
            result['counterexample'].update(ps2_return=hex(model.eval(actual).as_long() & 0xffffffff),
                                           source_return=hex(model.eval(expected).as_long()))
    elif answer == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result


def run(root, output, port):
    from verify import request, Unsupported
    reports = []
    source = (root / 'src/b-witch/Actor.cpp').read_text(encoding='utf-8')
    for address, name, setter in CASES:
        report = {'address': address, 'function': 'CActor::' + name, 'source': 'src/b-witch/Actor.cpp',
                  'field_mapping': 'source uint flags <-> PS2 4 bytes at this+8; explicit assumption'}
        try:
            export = request('export_pcode', port, address=address)
            (output / (address + '.json')).write_text(json.dumps(export, indent=2), encoding='utf-8')
            if export['entry'] != address or export['name'] != report['function']:
                raise Unsupported('Function identity mismatch')
            expression = r'this->flags\s*([&|])\s*(0x[0-9a-fA-F]+|[0-9]+)'
            body = (r'this->flags\s*=\s*' + expression + r';\s*return;' if setter else
                    r'return\s+' + expression + ';')
            pattern = (('void' if setter else 'uint') + r'\s+CActor::' + name + r'\(\)\s*\{\s*' + body + r'\s*\}')
            matches = list(re.finditer(pattern, source))
            if len(matches) != 1:
                raise Unsupported('Source outside supported field-mask grammar')
            match = matches[0]
            operator, mask = match[1], int(match[2], 0)
            if not 0 <= mask < 2**32:
                raise Unsupported('Mask exceeds uint32')
            report.update(compare(export, operator, mask, setter))
            report['source_body_sha256'] = hashlib.sha256(match[0].encode()).hexdigest()
            report['executable_sha256'] = export['executable_sha256']
            report['wrong_mask_control'] = compare(export, operator, mask ^ 1, setter)
            report['wrong_offset_control'] = compare(export, operator, mask, setter, offset=12)
            if any(report[key]['result'] != 'mismatch' for key in ('wrong_mask_control', 'wrong_offset_control')):
                raise Unsupported('Negative control did not detect a mismatch')
        except (Unsupported, RuntimeError, OSError, ValueError, KeyError, z3.Z3Exception) as error:
            report.update(result='inconclusive', reason=str(error))
        reports.append(report)
    return reports
