"""Restricted integer leaf-function pilot. See README.md for proof scope."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'out/verification-deps'))
import z3
from client import request


class Unsupported(ValueError):
    pass


def node_key(node):
    return node['space'], int(node['offset'], 16), node['size']


def execute(export, argument, memory=None, *, object_size=16, path_mode=False,
            memory_base=0x1000, precondition=None, regions=None, call_mode=False, loop_mode=False,
            second_argument=None, vu_mode=False, graph_mode=False):
    """All paths supported by this pilot are straight-line, with one return.

    ABI: R5900 low 64-bit a1 at 0x50; v0 at 0x20; ra at 0x1f0.
    Covered subrange reads are extracted little-endian; unknown aliases fail
    closed. Optional byte memory enables bounded object getter/setter cases.
    """
    if (export.get('schema') != 'kya.raw-pcode.v1' or
            export.get('language') != 'r5900:LE:32:default' or
            export.get('big_endian') is not False or export.get('complete') is not True):
        raise Unsupported('Unsupported or incomplete export')
    state = {('register', 0x50, 8): argument}
    if memory is not None:
        state = {('register', 0x40, 4): z3.BitVecVal(0x1000, 32)}
        if argument is not None:
            state[('register', 0x50, 8)] = argument
    if call_mode:
        state[('register', 0x1d0, 8)] = z3.BitVecVal(0x8010, 64)
        state[('register', 0x1f0, 8)] = z3.BitVecVal(0xdead0000, 64)
    returned = False
    if second_argument is not None:
        state[('register', 0x60, 8)] = second_argument
    if graph_mode:
        state[('register', 0x40, 8)] = z3.BitVecVal(0x1000, 64)
        state[('register', 0x100, 16)] = z3.BitVec('incoming_s0', 128)
    if vu_mode:
        state[('register', 0x4100, 16)] = z3.BitVecVal(0x3f800000 << 96, 128)
    count = 0
    condition = z3.BoolVal(True)

    def read(node):
        if node['constant']:
            return z3.BitVecVal(int(node['offset'], 16), node['size'] * 8)
        key = node_key(node)
        if key not in state:
            for old, value in state.items():
                if old[0] == key[0] and old[1] <= key[1] and key[1] + key[2] <= old[1] + old[2]:
                    start = (key[1] - old[1]) * 8
                    return z3.Extract(start + key[2] * 8 - 1, start, value)
            if graph_mode:
                parts = []
                for byte in reversed(range(key[2])):
                    candidates = [(old, value) for old, value in state.items()
                                  if old[0] == key[0] and old[1] <= key[1] + byte < old[1] + old[2]]
                    if len(candidates) != 1:
                        raise Unsupported(f'Uninitialized or ambiguous graph register: {key}')
                    old, value = candidates[0]
                    shift = (key[1] + byte - old[1]) * 8
                    parts.append(z3.Extract(shift + 7, shift, value))
                return z3.Concat(*parts) if len(parts) > 1 else parts[0]
            raise Unsupported(f'Uninitialized or aliased read: {key}')
        return state[key]

    def memory_address(inputs, size):
        if memory is None or not inputs[0]['constant'] or int(inputs[0]['offset'], 16) != 0x1a1:
            raise Unsupported('Unsupported memory space (expected audited R5900 ram ID 0x1a1)')
        address = z3.simplify(read(inputs[1]))
        if regions is not None:
            solver = z3.Solver()
            solver.set(timeout=5000)
            solver.add(precondition, condition, z3.Not(z3.Or(*[
                z3.And(z3.UGE(address, base), z3.ULE(address, base + length - size))
                for base, length in regions if length >= size])))
            if solver.check() != z3.unsat:
                raise Unsupported('Region bounds not proven')
            return address
        if z3.is_bv_value(address):
            if not memory_base <= address.as_long() <= memory_base + object_size - size:
                raise Unsupported('Memory access outside modeled region')
            return address.as_long() - memory_base
        if precondition is None or address.size() != 32:
            raise Unsupported('Symbolic address requires explicit domain')
        solver = z3.Solver()
        solver.set(timeout=5000)
        solver.add(precondition, condition,
                   z3.Not(z3.And(z3.UGE(address, memory_base),
                                 z3.ULE(address, memory_base + object_size - size))))
        if solver.check() != z3.unsat:
            raise Unsupported('Symbolic address bounds not proven')
        return address - memory_base

    def index(offset):
        return z3.BitVecVal(offset, 32) if isinstance(offset, int) else offset

    for instruction in export['instructions']:
        # Unique temporaries are local to an instruction (including its slot).
        state = {key: value for key, value in state.items() if key[0] != 'unique'}
        for op in instruction['ops']:
            count += 1
            if returned:
                raise Unsupported('Operations after RETURN')
            code = op['opcode']
            inputs = op['inputs']
            if code == 'CALLOTHER' and vu_mode:
                from vu_identity import evaluate
                value = evaluate(op, instruction['text'], read)
                state[node_key(op['output'])] = value
                continue
            if code == 'CHECK_TARGET' and call_mode:
                solver = z3.Solver()
                solver.set(timeout=5000)
                solver.add(precondition, condition, read(inputs[0]) != op['expected'])
                if solver.check() != z3.unsat:
                    raise Unsupported('Call return target not proven')
                continue
            if code == 'PATH_GUARD' and path_mode:
                condition = z3.And(condition, (read(inputs[0]) != 0) == op['taken'])
                continue
            if code == 'STORE':
                if len(inputs) != 3:
                    raise Unsupported('Invalid STORE')
                value = read(inputs[2])
                offset = memory_address(inputs, value.size() // 8)
                for byte in range(value.size() // 8):
                    memory = z3.Store(memory, index(offset + byte),
                                      z3.Extract(byte * 8 + 7, byte * 8, value))
                continue
            if code == 'RETURN':
                if len(inputs) != 1 or node_key(inputs[0]) != ('register', 0x1f0, 8):
                    raise Unsupported('Unexpected return target')
                if call_mode:
                    solver = z3.Solver()
                    solver.set(timeout=5000)
                    solver.add(precondition, condition, z3.Or(read(inputs[0]) != 0xdead0000,
                               read(dict(space='register', offset='1d0', size=8, constant=False)) != 0x8010))
                    if solver.check() != z3.unsat:
                        raise Unsupported('Stack or return address not restored')
                if graph_mode:
                    solver = z3.Solver()
                    solver.set(timeout=5000)
                    solver.add(precondition, condition, read(dict(space='register', offset='100', size=16, constant=False)) != z3.BitVec('incoming_s0', 128))
                    if solver.check() != z3.unsat:
                        raise Unsupported('PS2 s0 preservation failed')
                returned = True
                continue
            output = op.get('output')
            if not output or output['constant']:
                raise Unsupported(f'Unsupported operation: {code}')
            key = node_key(output)
            allowed = [('register', 0x20, 8)]
            if memory is not None:
                allowed += [('register', offset, size) for offset in (0x10, 0x20, 0x30, 0x50)
                            for size in (4, 8)]
            if call_mode:
                allowed += [('register', offset, size) for offset in (0x1d0, 0x1f0) for size in (4, 8)]
            if loop_mode or second_argument is not None:
                allowed += [('register', offset, size) for offset in (0x40, 0x60) for size in (4, 8)]
            if graph_mode:
                allowed += [('register', offset, size) for offset in (0x40, 0x100) for size in (4, 8)]
                allowed += [('register', offset, 4) for offset in (0x104, 0x108, 0x10c)]
            if key[0] != 'unique' and key not in allowed:
                raise Unsupported(f'Unmodeled register or memory effect: {key}')
            bits = output['size'] * 8
            if code == 'LOAD':
                if len(inputs) != 2:
                    raise Unsupported('Invalid LOAD')
                offset = memory_address(inputs, output['size'])
                parts = [z3.Select(memory, index(offset + byte)) for byte in reversed(range(output['size']))]
                values = [z3.Concat(*parts) if len(parts) > 1 else parts[0]]
                code = 'COPY'
            else:
                values = [read(value) for value in inputs]
            if code == 'COPY' and len(values) == 1:
                value = values[0]
            elif code in ('INT_ZEXT', 'INT_SEXT') and len(values) == 1:
                extra = bits - values[0].size()
                if extra < 0:
                    raise Unsupported('Invalid extension width')
                value = (z3.ZeroExt if code == 'INT_ZEXT' else z3.SignExt)(extra, values[0])
            elif len(values) == 2 and values[0].size() == values[1].size():
                a, b = values
                arithmetic = {'INT_AND': lambda: a & b, 'INT_OR': lambda: a | b,
                              'INT_XOR': lambda: a ^ b, 'INT_ADD': lambda: a + b,
                              'INT_SUB': lambda: a - b, 'INT_MULT': lambda: a * b}
                arithmetic.update(INT_LEFT=lambda: a << b, INT_RIGHT=lambda: z3.LShR(a, b),
                                  INT_SRIGHT=lambda: a >> b)
                comparisons = {'INT_LESS': lambda: z3.ULT(a, b),
                               'INT_SLESS': lambda: a < b,
                               'INT_EQUAL': lambda: a == b,
                               'INT_NOTEQUAL': lambda: a != b}
                if code in arithmetic:
                    value = arithmetic[code]()
                elif code in comparisons and bits == 8:
                    value = z3.If(comparisons[code](), z3.BitVecVal(1, bits), z3.BitVecVal(0, bits))
                else:
                    raise Unsupported(f'Unsupported operation: {code}')
            else:
                raise Unsupported(f'Unsupported operation or widths: {code}')
            if value.size() != bits:
                raise Unsupported('Output width mismatch')
            # Never retain stale aliases after a write.
            for old in list(state):
                if old[0] == key[0] and old[1] < key[1] + key[2] and key[1] < old[1] + old[2]:
                    if graph_mode:
                        for byte in range(old[2]):
                            offset = old[1] + byte
                            if not key[1] <= offset < key[1] + key[2]:
                                state[(old[0], offset, 1)] = z3.Extract(byte * 8 + 7, byte * 8, state[old])
                    del state[old]
            state[key] = value
    if not returned or count != export['operation_count']:
        raise Unsupported('Missing return/value or inconsistent operation count')
    if memory is not None:
        if path_mode:
            return state.get(('register', 0x20, 8)), memory, condition
        return state.get(('register', 0x20, 8)), memory
    if ('register', 0x20, 8) not in state:
        raise Unsupported('Missing return value')
    return state[('register', 0x20, 8)]


def source_mask(path, class_name):
    source = path.read_text(encoding='utf-8')
    # Deliberately closed grammar: a changed body is rejected, never guessed.
    pattern = (r'bool\s+' + re.escape(class_name) +
               r'::IsKindOfObject\(ulong kind\)\s*\{\s*return\s*'
               r'\(kind\s*&\s*(0x[0-9a-fA-F]+|[0-9]+)\)\s*!=\s*0;\s*\}')
    matches = list(re.finditer(pattern, source))
    if len(matches) != 1:
        raise Unsupported('Source is outside the supported unsigned mask predicate grammar')
    mask = int(matches[0][1], 0)
    if not 0 <= mask < 2**64:
        raise Unsupported('Mask outside 64-bit range')
    return mask, matches[0][0]


def compare(export, mask, timeout=5000):
    kind = z3.BitVec('kind', 64)
    original = execute(export, kind)
    source = z3.If((kind & mask) != 0, z3.BitVecVal(1, 64), z3.BitVecVal(0, 64))
    solver = z3.Solver()
    solver.set(timeout=timeout)
    solver.add(original != source)
    verdict = solver.check()
    result = {'result': {'sat': 'mismatch', 'unsat': 'equivalent', 'unknown': 'inconclusive'}[str(verdict)]}
    if verdict == z3.sat:
        model = solver.model()
        result['counterexample'] = {name: hex(model.eval(value, model_completion=True).as_long())
                                   for name, value in [('kind', kind), ('ps2', original), ('source', source)]}
    elif verdict == z3.unknown:
        result['reason'] = solver.reason_unknown()
    return result


CASES = [('00100460', 'CActor', 'Actor.cpp'),
         ('0011e060', 'CActorMovable', 'ActorMovable.cpp'),
         ('00124180', 'CActorAutonomous', 'ActorAutonomous.cpp')]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/verification')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    reports = []
    for address, class_name, filename in CASES:
        report = {'address': address, 'function': class_name + '::IsKindOfObject',
                  'source': 'src/b-witch/' + filename}
        try:
            export = request('export_pcode', args.port, address=address)
            (args.output / (address + '.json')).write_text(json.dumps(export, indent=2), encoding='utf-8')
            if export['entry'] != address or export['name'] != report['function']:
                raise Unsupported('Function identity does not match manifest')
            mask, body = source_mask(ROOT / report['source'], class_name)
            report.update(compare(export, mask))
            report['executable_sha256'] = export['executable_sha256']
            report['source_body_sha256'] = hashlib.sha256(body.encode()).hexdigest()
            report['mask'] = mask
            report['wrong_mask_control'] = compare(export, mask ^ 1)
            if report['wrong_mask_control']['result'] != 'mismatch':
                raise Unsupported('Deliberately wrong mask was not detected')
        except (Unsupported, RuntimeError, OSError, ValueError, KeyError, z3.Z3Exception) as error:
            report.update(result='inconclusive', reason=str(error))
        reports.append(report)
    from memory_cases import run
    reports.extend(run(ROOT, args.output, args.port))
    from branch_cases import run as run_branches
    reports.extend(run_branches(ROOT, args.output, args.port))
    from indexed_cases import run as run_indexed
    reports.extend(run_indexed(ROOT, args.output, args.port))
    from call_cases import run as run_calls
    reports.extend(run_calls(ROOT, args.output, args.port))
    from loop_cases import run as run_loops
    reports.extend(run_loops(ROOT, args.output, args.port))
    from alias_cases import run as run_aliases
    reports.extend(run_aliases(ROOT, args.output, args.port))
    document = {'scope': 'Raw PS2 P-code versus restricted C++ source semantics; not compiled PC binary equivalence',
                'assumptions': ['Ghidra R5900 lifter is correct', 'ulong is unsigned 64-bit (port/include/port.h)',
                                'ABI a1 low 64 bits input, v0 low 64 bits result; return via ra',
                                'Memory cases: valid 16-byte object at modeled address 0x1000, uint flags maps to this+8',
                                'All initial bytes symbolic; final memory compared in full; uint return compared at 32 bits',
                                'R5900 ram space ID 0x1a1; a0 low 32 bits is this; v0/v1/a1 may be clobbered',
                                'Branch cases use explicit per-function object sizes and audited field mappings',
                                'Acyclic branches only: at most 32 paths, 128 instructions per path; exhaustion is inconclusive',
                                'One allow-listed direct call graph with checked returns and private stack frame',
                                'One bounded loop with solver-proven unreachable cutoff paths',
                                'No other calls or loops, floating point, or undefined reads accepted'],
                'solver': z3.get_version_string(), 'functions': reports}
    (args.output / 'report.json').write_text(json.dumps(document, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(document, indent=2))
    return 0 if all(item['result'] == 'equivalent' for item in reports) else 1


if __name__ == '__main__':
    sys.exit(main())
