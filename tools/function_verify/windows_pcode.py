"""Strict Windows x64 raw P-code executor for bounded compiled pilot paths."""
from verify import Unsupported
import z3
from region_memory import RegionMemory


def execute(export, argument, initial, *, path_mode=False, precondition=None, regions=None,
            second_argument=None, float_argument=None, this_argument=None, write_regions=None, store_trace=None,
            guard_trace=None, state_in=None, return_state=False, fp_mode=False, regional_initial_stores=False,
            access_trace=None):
    if export.get('language') != 'x86:LE:64:default' or export.get('compiler') != 'windows' or not export.get('complete'):
        raise Unsupported('Expected complete Windows x64 export')
    registers, temporaries = {}, {}
    def write(node, value):
        nonlocal memory
        if node['space'] == 'ram':
            if value.size() != node['size'] * 8:
                raise Unsupported('Invalid x64 direct memory output width')
            offset = address([dict(space='const', offset='1b1', size=8, constant=True),
                              dict(space='const', offset=node['offset'], size=8, constant=True)], node['size'], True)
            if store_trace is not None:
                store_trace.append((z3.simplify(offset), value))
            heap.store(offset, value, condition)
            memory = heap.memory
            return
        target = registers if node['space'] == 'register' else temporaries
        if node['space'] not in ('register', 'unique') or value.size() != node['size'] * 8:
            raise Unsupported('Invalid x64 output')
        value = z3.simplify(value)
        offset = int(node['offset'], 16)
        for i in range(node['size']):
            target[offset + i] = z3.Extract(i * 8 + 7, i * 8, value)
    def reg(offset, size=8):
        return dict(space='register', offset=hex(offset)[2:], size=size, constant=False)
    def read(node):
        if node['constant']:
            return z3.BitVecVal(int(node['offset'], 16), node['size'] * 8)
        if node['space'] == 'ram':
            offset = address([dict(space='const', offset='1b1', size=8, constant=True),
                              dict(space='const', offset=node['offset'], size=8, constant=True)], node['size'])
            return heap.load(offset, node['size'], condition)
        target = registers if node['space'] == 'register' else temporaries
        if node['space'] not in ('register', 'unique'):
            raise Unsupported('Unexpected x64 varnode space')
        try:
            parts = [target[int(node['offset'], 16) + i] for i in reversed(range(node['size']))]
        except KeyError:
            raise Unsupported('Uninitialized x64 register or temporary')
        return z3.Concat(*parts) if len(parts) > 1 else parts[0]
    old_rbp = z3.BitVec('incoming_rbp', 64)
    for offset, value in [(0, z3.BitVec('incoming_rax', 64)),
                          (8, z3.BitVecVal(0x1000, 64) if this_argument is None else this_argument),
                          (0x10, argument), (0x20, z3.BitVecVal(0x8000, 64)), (0x28, old_rbp)]:
        if value is not None:
            write(reg(offset), value)
    write(reg(0x1200, 16), z3.BitVec('incoming_xmm0', 128))
    if second_argument is not None:
        write(reg(0x80), second_argument)  # Windows x64 R8, third argument.
    if float_argument is not None:
        if float_argument.size() != 32:
            raise Unsupported('Expected raw binary32 argument')
        write(reg(0x1220, 16), z3.Concat(z3.BitVec('incoming_xmm1_high', 96), float_argument))
    if state_in is not None:
        state_in.validate('windows')
        for (space, offset, size), value in state_in.registers.items():
            write(reg(offset, size), value)
        if argument is not None:
            write(reg(0x10), argument)
        if second_argument is not None:
            write(reg(0x80), second_argument)
        if this_argument is not None:
            write(reg(8), this_argument)
        if float_argument is not None:
            write(reg(0x1220, 4), float_argument)
        old_rbp = read(reg(0x28))
        initial = state_in.memory
    memory = initial
    for i in range(8):
        memory = z3.Store(memory, z3.BitVecVal(0x8000 + i, 64), z3.BitVecVal((0xdead0000 >> (i * 8)) & 255, 8))
    baseline = memory
    count, returned = 0, False
    condition = state_in.condition if state_in is not None else z3.BoolVal(True)
    domain = precondition if precondition is not None else z3.BoolVal(True)
    processor = dict(state_in.processor) if state_in else {}
    heap = RegionMemory(memory, (regions if regions is not None else [(0x1000, 0x100)]) +
                        [(0x7e00 if path_mode else 0x7fc0, (0x200 if path_mode else 0x40) + 8)], domain,
                        prune_initial_stores=regional_initial_stores)
    def prove(claim, reason):
        claim = z3.simplify(claim)
        if z3.is_true(claim):
            return
        solver = z3.Solver()
        solver.set(timeout=5000)
        solver.add(domain, condition, z3.Not(claim))
        if solver.check() != z3.unsat:
            raise Unsupported(reason)
    def address(inputs, size, store=False):
        if not inputs[0]['constant'] or int(inputs[0]['offset'], 16) != 0x1b1:
            raise Unsupported('Unknown x64 memory space')
        value = z3.simplify(read(inputs[1]))
        selected_regions = write_regions if store and write_regions is not None else regions
        allowed = (selected_regions if selected_regions is not None else [(0x1000, 0x100)]) + [(0x7e00 if path_mode else 0x7fc0, (0x200 if path_mode else 0x40) + (0 if store else 8))]
        prove(z3.Or(*[z3.And(z3.UGE(value, base), z3.ULE(value, base + length - size))
                       for base, length in allowed if length >= size]),
              'x64 memory access outside object/private stack at ' +
              instruction.get('original_address', instruction['address']) +
              (' (store ' if store else ' (load ') + str(size) + ' bytes' +
              (', address ' + hex(value.as_long()) if z3.is_bv_value(value) else ', symbolic address') + ')')
        if access_trace is not None:
            access_trace.append((z3.simplify(value), size))
        return value
    def boolbits(value, bits):
        return z3.If(value, z3.BitVecVal(1, bits), z3.BitVecVal(0, bits))
    def outcome(value=None, returned=True):
        from execution_state import ExecutionState, ExecutionOutcome
        provenance = {i['address']: i.get('original_address', i['address']) for i in export['instructions']}
        return ExecutionOutcome(ExecutionState('windows', memory,
            {('register', offset, 1): value for offset, value in registers.items()}, condition,
            dict(processor)), value, baseline, returned,
            tuple((provenance[address], taken) for address, taken in (guard_trace or ())),
            writes=tuple(store_trace or ()))
    for instruction in export['instructions']:
        if path_mode and instruction['address'] == 'cutoff':
            if return_state:
                return outcome(returned=False)
            return None, memory, baseline, condition
        temporaries.clear()
        if fp_mode:
            from processor_fp import validate_instruction
            validate_instruction(instruction, 'windows')
        for op in instruction['ops']:
            count += 1
            if returned:
                raise Unsupported('Operations after x64 return')
            code, inputs = op['opcode'], op['inputs']
            if code == 'UNRESOLVED_TRANSFER':
                raise Unsupported(op.get('reason', 'Reachable unresolved transfer'))
            if path_mode and code == 'PATH_GUARD':
                if guard_trace is not None:
                    guard_trace.append((instruction['address'], op['taken']))
                condition = z3.And(condition, (read(inputs[0]) != 0) == op['taken'])
                solver = z3.Solver()
                solver.set(timeout=5000)
                solver.add(domain, condition)
                answer = solver.check()
                if answer == z3.unsat:
                    if return_state:
                        return outcome(returned=False)
                    return None
                if answer != z3.sat:
                    raise Unsupported('Path feasibility inconclusive at ' + instruction['address'] + ': ' + solver.reason_unknown())
                continue
            if path_mode and code in ('CHECK_TARGET', 'CHECK_DISPATCH'):
                targets = op['expected'] if isinstance(op['expected'], list) else [op['expected']]
                prove(z3.Or(*[read(inputs[0]) == target for target in targets]),
                      'Indirect dispatch target not proven' if code == 'CHECK_DISPATCH' else 'Call return target not proven')
                continue
            if code == 'RETURN':
                if len(inputs) != 1 or inputs[0] != reg(0x288):
                    raise Unsupported('Unexpected x64 return')
                prove(z3.And(read(inputs[0]) == 0xdead0000, read(reg(0x20)) == 0x8008,
                             read(reg(0x28)) == old_rbp), 'Windows stack/frame/return preservation failed')
                returned = True
                continue
            if code == 'STORE':
                value = read(inputs[2])
                offset = address(inputs, value.size() // 8, True)
                if store_trace is not None:
                    store_trace.append((z3.simplify(offset), value))
                heap.store(offset, value, condition)
                memory = heap.memory
                continue
            output = op.get('output')
            if not output:
                raise Unsupported('Unsupported x64 side effect: ' + code)
            if output['space'] == 'register':
                offset = int(output['offset'], 16)
                if not any(start <= offset and offset + output['size'] <= start + size for start, size in
                           [(0, 8), (8, 8), (0x10, 8), (0x20, 8), (0x28, 8), (0x80, 32), (0x1200, 16), (0x288, 8),
                            (0x200, 1), (0x202, 1), (0x204, 1), (0x206, 1), (0x207, 1), (0x20b, 1)] +
                           [(0x1200 + index * 0x20, 16) for index in range(1, 6)]):
                    raise Unsupported('Unmodeled Windows register write')
            bits = output['size'] * 8
            if code == 'LOAD':
                offset = address(inputs, output['size'])
                value = heap.load(offset, output['size'], condition)
            else:
                values = [read(node) for node in inputs]
                a = values[0]
                b = values[1] if len(values) == 2 else None
                binary = {'INT_ADD': lambda: a + b, 'INT_SUB': lambda: a - b,
                          'INT_AND': lambda: a & b, 'INT_OR': lambda: a | b, 'INT_XOR': lambda: a ^ b}
                comparisons = {'INT_EQUAL': lambda: a == b, 'INT_NOTEQUAL': lambda: a != b,
                               'INT_LESS': lambda: z3.ULT(a, b), 'INT_SLESS': lambda: a < b,
                               'INT_LESSEQUAL': lambda: z3.ULE(a, b), 'INT_SLESSEQUAL': lambda: a <= b,
                               'INT_CARRY': lambda: z3.ULT(a + b, a),
                               'INT_SCARRY': lambda: z3.And((a < 0) == (b < 0), ((a + b) < 0) != (a < 0)),
                               'INT_SBORROW': lambda: z3.And((a < 0) != (b < 0), ((a - b) < 0) != (a < 0))}
                if (code.startswith('FLOAT_') or code == 'INT2FLOAT') and fp_mode:
                    from processor_fp import ProcessorFP
                    value = ProcessorFP('windows', processor, prove).evaluate(instruction, op, values, bits)
                elif code == 'COPY' and len(values) == 1:
                    value = a
                elif code in binary and len(values) == 2:
                    value = binary[code]()
                elif code in comparisons and len(values) == 2:
                    value = boolbits(comparisons[code](), bits)
                elif code == 'BOOL_NEGATE' and len(values) == 1:
                    value = boolbits(a == 0, bits)
                elif code in ('INT_DIV', 'INT_SDIV', 'INT_REM', 'INT_SREM') and len(values) == 2:
                    if a.size() != b.size() or bits != a.size():
                        raise Unsupported('Invalid integer division widths')
                    prove(b != 0, 'Reachable integer division by zero')
                    if code in ('INT_SDIV', 'INT_SREM'):
                        prove(z3.Not(z3.And(a == 1 << (bits - 1), b == -1)), 'Reachable signed division overflow')
                    value = {'INT_DIV': lambda: z3.UDiv(a, b), 'INT_SDIV': lambda: a / b,
                             'INT_REM': lambda: z3.URem(a, b), 'INT_SREM': lambda: z3.SRem(a, b)}[code]()
                elif code in ('BOOL_AND', 'BOOL_OR', 'BOOL_XOR') and len(values) == 2:
                    value = boolbits({'BOOL_AND': z3.And, 'BOOL_OR': z3.Or, 'BOOL_XOR': z3.Xor}[code](a != 0, b != 0), bits)
                elif code == 'SUBPIECE' and len(values) == 2 and z3.is_bv_value(b):
                    low = b.as_long() * 8
                    value = z3.Extract(low + bits - 1, low, a)
                elif code in ('INT_LEFT', 'INT_RIGHT', 'INT_SRIGHT', 'INT_MULT') and len(values) == 2:
                    if code != 'INT_MULT':
                        if b.size() < a.size():
                            b = z3.ZeroExt(a.size() - b.size(), b)
                        elif b.size() > a.size():
                            b = z3.If(z3.UGE(b, a.size()), z3.BitVecVal(a.size(), a.size()),
                                      z3.Extract(a.size() - 1, 0, b))
                    value = {'INT_LEFT': lambda: a << b, 'INT_RIGHT': lambda: z3.LShR(a, b),
                             'INT_SRIGHT': lambda: a >> b, 'INT_MULT': lambda: a * b}[code]()
                elif code in ('INT_ZEXT', 'INT_SEXT') and len(values) == 1 and bits >= a.size():
                    value = (z3.ZeroExt if code == 'INT_ZEXT' else z3.SignExt)(bits - a.size(), a)
                elif code == 'POPCOUNT' and len(values) == 1:
                    value = sum([z3.ZeroExt(bits - 1, z3.Extract(i, i, a)) for i in range(a.size())], z3.BitVecVal(0, bits))
                else:
                    raise Unsupported('Unsupported x64 opcode: ' + code)
            write(output, value)
    if not returned or count != export['operation_count']:
        raise Unsupported('Incomplete x64 execution')
    if return_state:
        return outcome(read(reg(0)))
    result = (read(reg(0)), memory, baseline)
    return result + (condition,) if path_mode else result
