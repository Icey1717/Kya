"""Processor-selected numeric semantics, with explicit unsupported frontiers.

EE comparisons use the EE encoding (exponent zero is zero, exponent 255 is
finite). Add/sub requires an explicitly conditional pinned reference model.
EE arithmetic is never substituted with bare IEEE RTZ.
SSE scalar arithmetic uses Z3 IEEE arithmetic plus MXCSR handling. Arithmetic
NaN operands are rejected until payload/priority behavior has independent tests.
"""
from verify import Unsupported
import z3

VERSION = 'kya-processor-fp-1'


def ee_register(index):
    # Storage recovered from actual InitState raw p-code and instruction bytes
    # (fixtures/fighter-cop1-instructions.json); f20 additionally comes from
    # Initialize's DIV.S at 00241d94 (fixtures/animation-playback.json.gz).
    # UpdateAnimParams MUL.S/ADD.S at 00241c1c/00241c38 additionally validate
    # f3/f4 (fixtures/animation-blended-playback.json.gz).
    # In particular f1 precedes f0;
    # a linear encoding-index-to-offset formula is incorrect for this language.
    offsets = {0: 0x1004, 1: 0x1000, 2: 0x100c, 3: 0x1008, 4: 0x1014,
               12: 0x1034, 14: 0x103c, 20: 0x1054}
    if index not in offsets:
        raise Unsupported('Unvalidated R5900 floating-register storage: f' + str(index))
    return offsets[index]


def numeric_source(instruction, architecture):
    """Attribute one COP1 delay-slot operation to its own checked bytes."""
    if (architecture != 'ps2' or instruction.get('mnemonic', instruction.get('text', '').split(' ')[0]).upper() != 'JAL'
            or not any(op['opcode'].startswith('FLOAT_') for op in instruction['ops'])):
        return instruction
    try:
        owner = bytes.fromhex(instruction['bytes'])
        slots = instruction['delay_slot_instructions']
        if (len(owner) != 4 or int.from_bytes(owner, 'little') >> 26 != 3
                or instruction['delay_slots'] != 1 or len(slots) != 1 or slots[0]['length'] != 4
                or int(slots[0]['address'], 16) != int(instruction.get('original_address', instruction['address']), 16) + 4):
            raise ValueError()
        raw = bytes.fromhex(slots[0]['bytes'])
        word = int.from_bytes(raw, 'little')
        functions = {0: 'ADD.S', 1: 'SUB.S', 2: 'MUL.S', 3: 'DIV.S', 4: 'SQRT.S',
                     5: 'ABS.S', 7: 'NEG.S', 0x32: 'C.EQ.S', 0x34: 'C.LT.S', 0x36: 'C.LE.S'}
        if len(raw) != 4 or word >> 21 != 0x230 or word & 0x3f not in functions:
            raise ValueError()
    except (KeyError, ValueError, TypeError, IndexError):
        raise Unsupported('Unvalidated floating-point call delay-slot provenance')
    return dict(instruction, mnemonic=functions[word & 0x3f], text=functions[word & 0x3f], bytes=raw.hex(), length=4)


def validate_instruction(instruction, architecture):
    """Reject removed/changed numeric ops before executing an FP instruction."""
    instruction = numeric_source(instruction, architecture)
    mnemonic = instruction.get('mnemonic', instruction.get('text', '').split(' ')[0]).upper()
    if mnemonic in ('CVT.S.W', 'CVTSI2SS'):
        validate_conversion(instruction, architecture)
        return
    if mnemonic in ('LDMXCSR', 'STMXCSR', 'CFC1', 'CTC1', 'CFC2', 'CTC2'):
        raise Unsupported('Processor control instruction requires a validated handler: ' + mnemonic)
    arithmetic = {'ADDSS': 'FLOAT_ADD', 'SUBSS': 'FLOAT_SUB', 'MULSS': 'FLOAT_MULT', 'DIVSS': 'FLOAT_DIV'}
    ee_scalar = {'ADD.S': 'FLOAT_ADD', 'SUB.S': 'FLOAT_SUB', 'MUL.S': 'FLOAT_MULT',
                 'DIV.S': 'FLOAT_DIV', 'SQRT.S': 'FLOAT_SQRT', 'ABS.S': 'FLOAT_ABS', 'NEG.S': 'FLOAT_NEG'}
    expected = (['FLOAT_NAN', 'FLOAT_NAN', 'FLOAT_EQUAL', 'FLOAT_LESS'] if mnemonic in ('COMISS', 'UCOMISS') else
                [arithmetic[mnemonic]] if mnemonic in arithmetic else
                [{'C.EQ.S': 'FLOAT_EQUAL', 'C.LT.S': 'FLOAT_LESS', 'C.LE.S': 'FLOAT_LESSEQUAL'}[mnemonic]]
                if mnemonic in ('C.EQ.S', 'C.LT.S', 'C.LE.S') else
                [ee_scalar[mnemonic]] if mnemonic in ee_scalar else None)
    actual = [op for op in instruction['ops'] if op['opcode'].startswith('FLOAT_') or op['opcode'] == 'INT2FLOAT']
    if expected is not None and [op['opcode'] for op in actual] != expected:
        raise Unsupported('Instruction floating-point p-code shape changed')
    if actual and expected is None:
        raise Unsupported('Processor instruction requires unimplemented FP semantics: ' + mnemonic)
    if architecture == 'ps2' and mnemonic.endswith('.S') and expected is None and mnemonic != 'MOV.S':
        raise Unsupported('EE instruction requires an instruction-level status handler: ' + mnemonic)
    if expected is None:
        return
    if (architecture == 'windows') != (mnemonic in set(arithmetic) | {'COMISS', 'UCOMISS'}):
        raise Unsupported('Floating-point instruction architecture mismatch')
    try:
        encoding = bytes.fromhex(instruction['bytes'])
    except (KeyError, ValueError):
        raise Unsupported('FP semantics require instruction-byte provenance')
    if architecture == 'windows':
        prefix = 1 if mnemonic in arithmetic else 0
        rex = encoding[prefix] if len(encoding) > prefix and 0x40 <= encoding[prefix] <= 0x4f else 0
        prefix += int(rex != 0)
        if len(encoding) < prefix + 3:
            raise Unsupported('Truncated SSE instruction encoding')
        modrm = encoding[prefix + 2]
        destination = 0x1200 + (((modrm >> 3) & 7) + (8 if rex & 4 else 0)) * 0x20
        first = actual[0]['inputs'][0]
        if first['space'] != 'register' or int(first['offset'], 16) != destination or first['size'] != 4:
            raise Unsupported('SSE destination operand disagrees with encoding')
        if mnemonic in arithmetic and actual[0].get('output') != first:
            raise Unsupported('SSE scalar output disagrees with destination')
        second = actual[0]['inputs'][1] if mnemonic in arithmetic else actual[1]['inputs'][0]
        if modrm >> 6 == 3:
            source = 0x1200 + ((modrm & 7) + (8 if rex & 1 else 0)) * 0x20
            if second['space'] != 'register' or int(second['offset'], 16) != source or second['size'] != 4:
                raise Unsupported('SSE source operand disagrees with encoding')
        elif not any(op['opcode'] == 'LOAD' and op.get('output') == second for op in instruction['ops']):
            raise Unsupported('SSE memory source lacks a matching p-code load')
        if mnemonic in ('COMISS', 'UCOMISS') and any(op['inputs'] != [first, second] for op in actual[2:]):
            raise Unsupported('SSE comparison operand pair changed')


def boolbits(value, bits=8):
    return z3.If(value, z3.BitVecVal(1, bits), z3.BitVecVal(0, bits))


def validate_conversion(instruction, architecture):
    """Signed 32-bit conversions; validate observed base+displacement loads too."""
    try:
        raw = bytes.fromhex(instruction['bytes'])
        mnemonic = instruction.get('mnemonic', instruction.get('text', '').split(' ')[0]).upper()
        if mnemonic != ('CVT.S.W' if architecture == 'ps2' else 'CVTSI2SS'):
            raise ValueError()
        ops = [op for op in instruction['ops'] if op['opcode'].startswith('FLOAT_') or op['opcode'] == 'INT2FLOAT']
        if len(ops) != 1 or ops[0]['opcode'] != 'INT2FLOAT' or len(ops[0]['inputs']) != 1:
            raise ValueError()
        source, target = ops[0]['inputs'][0], ops[0]['output']
        if architecture == 'ps2':
            word = int.from_bytes(raw, 'little')
            if len(raw) != 4 or word >> 21 != 0x234 or word & 0x3f != 0x20 or word >> 16 & 31:
                raise ValueError()
            src, dst = ee_register(word >> 11 & 31), ee_register(word >> 6 & 31)
        elif architecture == 'windows':
            pos = 1
            rex = raw[pos] if len(raw) > pos and 0x40 <= raw[pos] <= 0x4f else 0
            pos += bool(rex)
            if raw[:1] != b'\xf3' or raw[pos:pos + 2] != b'\x0f\x2a' or len(raw) < pos + 3 or rex & 8:
                raise ValueError()
            modrm = raw[pos + 2]
            reg = (modrm & 7) + (8 if rex & 1 else 0)
            src = reg * 8 if reg < 8 else 0x80 + (reg - 8) * 8
            dst = 0x1200 + (((modrm >> 3) & 7) + (8 if rex & 4 else 0)) * 0x20
            mode = modrm >> 6
            if mode in (1, 2) and modrm & 7 != 4:
                width = 1 if mode == 1 else 4
                if len(raw) != pos + 3 + width or source['space'] != 'unique' or source['size'] != 4:
                    raise ValueError()
                displacement = int.from_bytes(raw[pos + 3:], 'little', signed=True) & 0xffffffffffffffff
                adds = [op for op in instruction['ops'] if op['opcode'] == 'INT_ADD']
                loads = [op for op in instruction['ops'] if op['opcode'] == 'LOAD']
                if len(adds) != 1 or len(loads) != 1 or len(instruction['ops']) != 3:
                    raise ValueError()
                base, disp = adds[0]['inputs']
                if (base['space'] != 'register' or base['size'] != 8 or int(base['offset'], 16) != src
                        or disp['space'] != 'const' or disp['size'] != 8 or int(disp['offset'], 16) != displacement
                        or loads[0]['output'] != source or loads[0]['inputs'][1] != adds[0]['output']
                        or loads[0]['inputs'][0]['space'] != 'const'
                        or int(loads[0]['inputs'][0]['offset'], 16) != 0x1b1):
                    raise ValueError()
                src = None
            elif mode != 3 or len(raw) != pos + 3:
                raise ValueError()
        else:
            raise ValueError()
        if any(n['space'] != 'register' or n['size'] != 4 or int(n['offset'], 16) != offset
               for n, offset in ((source, src), (target, dst)) if offset is not None):
            raise ValueError()
    except (KeyError, ValueError, TypeError, IndexError):
        raise Unsupported('Integer-to-float conversion encoding/operands disagree with p-code')
    return ops[0]


def exponent(value):
    return z3.Extract(30, 23, value)


def zero(value):
    return value & 0x7fffffff == 0


def subnormal(value):
    return z3.And(exponent(value) == 0, z3.Extract(22, 0, value) != 0)


def nan(value):
    return z3.And(exponent(value) == 255, z3.Extract(22, 0, value) != 0)


def snan(value):
    return z3.And(nan(value), z3.Extract(22, 22, value) == 0)


def signed_zero(value):
    return value & 0x80000000


def ee_compare(code, values, bits):
    if len(values) != 2 or any(v.size() != 32 for v in values) or bits != 8:
        raise Unsupported('Unsupported EE comparison widths')
    a, b = [z3.If(exponent(v) == 0, signed_zero(v), v) for v in values]
    equal = z3.Or(a == b, z3.And(zero(a), zero(b)))
    sign_a, sign_b = z3.Extract(31, 31, a), z3.Extract(31, 31, b)
    less = z3.And(z3.Not(equal), z3.If(sign_a != sign_b, sign_a == 1,
                  z3.If(sign_a == 1, z3.UGT(a, b), z3.ULT(a, b))))
    if code not in ('FLOAT_EQUAL', 'FLOAT_LESS', 'FLOAT_LESSEQUAL', 'FLOAT_NOTEQUAL'):
        raise Unsupported('Unsupported EE comparison')
    return boolbits({'FLOAT_EQUAL': equal, 'FLOAT_NOTEQUAL': z3.Not(equal),
                     'FLOAT_LESS': less, 'FLOAT_LESSEQUAL': z3.Or(less, equal)}[code], bits)


class ProcessorFP:
    def __init__(self, architecture, state, prove):
        self.architecture, self.state, self.prove = architecture, state, prove

    def control(self):
        if 'mxcsr' not in self.state:
            raise Unsupported('SSE arithmetic requires explicit entry MXCSR')
        control = self.state['mxcsr']
        control = z3.BitVecVal(control, 32) if isinstance(control, int) else z3.simplify(control)
        if control.size() != 32:
            raise Unsupported('Invalid MXCSR width')
        self.prove(control & 0xffff0000 == 0, 'Reserved MXCSR bits must be zero')
        return control

    def flags(self, control, flags):
        raised = z3.BitVecVal(0, 32)
        for bit, condition in flags.items():
            raised = raised | z3.If(condition, z3.BitVecVal(1 << bit, 32), z3.BitVecVal(0, 32))
        self.prove((raised & ~(control >> 7) & 0x3f) == 0, 'Reachable unmasked SSE exception')
        self.state['mxcsr'] = z3.simplify(control | raised)

    def evaluate(self, instruction, op, values, bits):
        instruction = numeric_source(instruction, self.architecture)
        code = op['opcode']
        if not values or any(v.size() != 32 for v in values):
            raise Unsupported('Unsupported processor floating-point width')
        mnemonic = instruction.get('mnemonic', instruction.get('text', '').split(' ')[0]).upper()
        if code == 'INT2FLOAT':
            checked = validate_conversion(instruction, self.architecture)
            if checked != op or bits != 32 or len(values) != 1:
                raise Unsupported('Invalid signed conversion shape')
            if self.architecture == 'ps2':
                from ee_interpreter import MODEL, execute
                if self.state.get('ee_arithmetic_model') != MODEL:
                    raise Unsupported('EE conversion requires explicit pinned COP1 interpreter opt-in')
                from ee_interpreter import MXCSR
                if self.state.get('ee_interpreter_mxcsr', MXCSR) != MXCSR:
                    raise Unsupported('COP1 interpreter requires its captured host MXCSR profile')
                result = execute('CVT.S.W', values[0], 0, control=self.state.get('fcr31', 0x01000001))
                self.state['fcr31'] = result['fcr31']
                self.state['reference_models'] = tuple(sorted(set(self.state.get('reference_models', ())) | {MODEL}))
                return result['bits']
            control = self.control()
            rounding = z3.Extract(14, 13, control)
            mode = z3.If(rounding == 0, z3.RNE(), z3.If(rounding == 1, z3.RTN(),
                         z3.If(rounding == 2, z3.RTP(), z3.RTZ())))
            result = z3.fpSignedToFP(mode, values[0], z3.Float32())
            self.flags(control, {5: z3.fpToReal(result) != z3.ToReal(z3.BV2Int(values[0], is_signed=True))})
            return z3.fpToIEEEBV(result)
        try:
            encoding = bytes.fromhex(instruction['bytes'])
        except (KeyError, ValueError):
            raise Unsupported('FP semantics require instruction-byte provenance')
        if self.architecture == 'ps2':
            from ee_interpreter import MODEL as INTERPRETER_MODEL
            if self.state.get('ee_arithmetic_model') == INTERPRETER_MODEL:
                return self.interpreter(instruction, op, values, bits, encoding, mnemonic)
            if mnemonic in ('ADD.S', 'SUB.S'):
                from ee_reference import MODEL, add_sub
                if self.state.get('ee_arithmetic_model') != MODEL:
                    raise Unsupported('EE non-IEEE arithmetic requires explicit pinned PCSX2 reference-model opt-in')
                word = int.from_bytes(encoding, 'little')
                function = 0 if mnemonic == 'ADD.S' else 1
                if (len(encoding) != 4 or word >> 21 != 0x230 or word & 0x3f != function
                        or code != ('FLOAT_ADD' if function == 0 else 'FLOAT_SUB') or bits != 32 or len(values) != 2):
                    raise Unsupported('EE arithmetic encoding disagrees with p-code')
                registers = [ee_register((word >> shift) & 31) for shift in (11, 16)]
                if any(n['space'] != 'register' or n['size'] != 4 or int(n['offset'], 16) != address
                       for n, address in zip(op['inputs'], registers)):
                    raise Unsupported('EE arithmetic operands disagree with encoding')
                output = op.get('output', {})
                if (output.get('space') != 'register' or output.get('size') != 4
                        or int(output.get('offset', '-1'), 16) != ee_register((word >> 6) & 31)):
                    raise Unsupported('EE arithmetic destination disagrees with encoding')
                result, overflow, underflow = add_sub(*values, subtract=bool(function))
                old = self.state.get('fcr31', 0x01000001)
                old = z3.BitVecVal(old, 32) if isinstance(old, int) else old
                self.state['fcr31'] = (old & ~0xc000) | z3.If(overflow, z3.BitVecVal(0x8010, 32), z3.BitVecVal(0, 32)) | z3.If(
                    underflow, z3.BitVecVal(0x4008, 32), z3.BitVecVal(0, 32))
                self.state['reference_models'] = tuple(set(self.state.get('reference_models', ())) | {MODEL})
                return result
            expected = {'C.EQ.S': 'FLOAT_EQUAL', 'C.LT.S': 'FLOAT_LESS', 'C.LE.S': 'FLOAT_LESSEQUAL'}
            if mnemonic not in expected or expected[mnemonic] != code:
                raise Unsupported('EE instruction requires validated non-IEEE semantics: ' + mnemonic)
            word = int.from_bytes(encoding, 'little')
            function = {'C.EQ.S': 0x32, 'C.LT.S': 0x34, 'C.LE.S': 0x36}[mnemonic]
            if len(encoding) != 4 or word >> 21 != 0x230 or word & 0x3f != function:
                raise Unsupported('EE comparison encoding disagrees with p-code')
            inputs = op['inputs']
            expected_registers = [ee_register((word >> shift) & 31) for shift in (11, 16)]
            if any(n['space'] != 'register' or int(n['offset'], 16) != address
                   for n, address in zip(inputs, expected_registers)):
                raise Unsupported('EE comparison operands disagree with encoding')
            result = ee_compare(code, values, bits)
            old = self.state.get('fcr31', z3.BitVecVal(0x01000001, 32))
            old = z3.BitVecVal(old, 32) if isinstance(old, int) else old
            self.state['fcr31'] = (old & ~(1 << 23)) | z3.If(result != 0,
                z3.BitVecVal(1 << 23, 32), z3.BitVecVal(0, 32))
            return result
        if self.architecture != 'windows':
            raise Unsupported('Unsupported FP processor')
        allowed = {'ADDSS': 'FLOAT_ADD', 'SUBSS': 'FLOAT_SUB',
                   'MULSS': 'FLOAT_MULT', 'DIVSS': 'FLOAT_DIV'}
        comparison = mnemonic in ('COMISS', 'UCOMISS')
        if not comparison and allowed.get(mnemonic) != code:
            raise Unsupported('Unvalidated SSE instruction/p-code shape: ' + mnemonic)
        if comparison and code not in ('FLOAT_NAN', 'FLOAT_EQUAL', 'FLOAT_LESS'):
            raise Unsupported('Unvalidated SSE comparison p-code')
        prefix = 0 if comparison else 1
        opcode = {'ADDSS': 0x58, 'SUBSS': 0x5c, 'MULSS': 0x59,
                  'DIVSS': 0x5e, 'COMISS': 0x2f, 'UCOMISS': 0x2e}[mnemonic]
        if not comparison and encoding[:1] != b'\xf3':
            raise Unsupported('SSE scalar prefix disagrees with p-code')
        if len(encoding) > prefix and 0x40 <= encoding[prefix] <= 0x4f:
            prefix += 1
        if encoding[prefix:prefix + 2] != bytes([0x0f, opcode]) or len(encoding) < prefix + 3:
            raise Unsupported('SSE encoding disagrees with p-code')
        control = self.control()
        daz, ftz = control & 0x40 != 0, control & 0x8000 != 0
        denormal = z3.And(z3.Not(daz), z3.Or(*[subnormal(v) for v in values]))
        operands = [z3.If(z3.And(daz, subnormal(v)), signed_zero(v), v) for v in values]
        floats = [z3.fpBVToFP(v, z3.Float32()) for v in operands]
        invalid = z3.Or(*[(nan(v) if mnemonic == 'COMISS' else snan(v)) for v in operands])
        if comparison:
            if code == 'FLOAT_NAN' and len(values) == 1:
                # This p-code op is only a component of the comparison. The
                # two-operand op below computes instruction-level exceptions.
                return boolbits(nan(operands[0]), bits)
            if len(values) != 2 or bits != 8:
                raise Unsupported('Invalid SSE comparison shape')
            self.flags(control, {0: invalid,
                1: z3.And(denormal, z3.Not(z3.Or(*[nan(v) for v in operands])))})
            return boolbits(z3.fpEQ(*floats) if code == 'FLOAT_EQUAL' else z3.fpLT(*floats), bits)
        if len(values) != 2 or bits != 32:
            raise Unsupported('Invalid SSE scalar arithmetic shape')
        self.prove(z3.Not(z3.Or(*[nan(v) for v in operands])),
                   'SSE NaN payload arithmetic not validated')
        rounding = z3.Extract(14, 13, control)
        mode = z3.If(rounding == 0, z3.RNE(), z3.If(rounding == 1, z3.RTN(),
                     z3.If(rounding == 2, z3.RTP(), z3.RTZ())))
        operation = {'FLOAT_ADD': z3.fpAdd, 'FLOAT_SUB': z3.fpSub,
                     'FLOAT_MULT': z3.fpMul, 'FLOAT_DIV': z3.fpDiv}[code]
        result = operation(mode, *floats)
        invalid = z3.fpIsNaN(result)
        # Extended exponent with the same precision detects overflow after
        # rounding, including finite saturation under directed rounding.
        extended = [z3.fpToFP(z3.RNE(), v, z3.FPSort(9, 24)) for v in floats]
        wide = operation(mode, *extended)
        limit = z3.fpToFP(z3.RNE(), z3.fpBVToFP(z3.BitVecVal(0x7f7fffff, 32), z3.Float32()), z3.FPSort(9, 24))
        finite = z3.And(*[z3.Not(z3.fpIsInf(v)) for v in floats])
        division_zero = z3.And(finite, z3.Not(zero(operands[0])), zero(operands[1])) if code == 'FLOAT_DIV' else z3.BoolVal(False)
        overflow = z3.And(finite, z3.Not(division_zero), z3.fpGT(z3.fpAbs(wide), limit))
        a, b = [z3.fpToReal(v) for v in floats]
        exact = {'FLOAT_ADD': lambda: a + b, 'FLOAT_SUB': lambda: a - b,
                 'FLOAT_MULT': lambda: a * b, 'FLOAT_DIV': lambda: a / b}[code]()
        valid = z3.And(finite, z3.Not(division_zero), z3.Not(invalid))
        normal_min = z3.RealVal(1) / z3.RealVal(2 ** 126)
        tiny = z3.And(valid, exact != 0, z3.If(exact < 0, -exact, exact) < normal_min)
        flush = z3.And(ftz, tiny)
        inexact = z3.And(valid, z3.Or(overflow, z3.fpToReal(result) != exact, flush))
        underflow = z3.And(tiny, z3.Or(inexact, flush))
        self.flags(control, {0: invalid, 1: denormal, 2: division_zero, 3: overflow,
                             4: underflow, 5: inexact})
        # Z3 leaves NaN payloads unspecified. For invalid operations with no
        # NaN input, SSE produces the negative quiet indefinite encoding.
        raw = z3.If(invalid, z3.BitVecVal(0xffc00000, 32), z3.fpToIEEEBV(result))
        return z3.If(flush, signed_zero(raw), raw)

    def interpreter(self, instruction, op, values, bits, encoding, mnemonic):
        from ee_interpreter import MODEL, MXCSR, execute
        functions = {'ADD.S': (0, 'FLOAT_ADD'), 'SUB.S': (1, 'FLOAT_SUB'),
                     'MUL.S': (2, 'FLOAT_MULT'), 'DIV.S': (3, 'FLOAT_DIV'),
                     'SQRT.S': (4, 'FLOAT_SQRT'), 'ABS.S': (5, 'FLOAT_ABS'), 'NEG.S': (7, 'FLOAT_NEG'),
                     'C.EQ.S': (0x32, 'FLOAT_EQUAL'),
                     'C.LT.S': (0x34, 'FLOAT_LESS'), 'C.LE.S': (0x36, 'FLOAT_LESSEQUAL')}
        if mnemonic not in functions:
            raise Unsupported('COP1 interpreter instruction handler not implemented: ' + mnemonic)
        if self.state.get('ee_interpreter_mxcsr', MXCSR) != MXCSR:
            raise Unsupported('COP1 interpreter requires its captured host MXCSR profile')
        word = int.from_bytes(encoding, 'little')
        function, opcode = functions[mnemonic]
        comparison = function >= 0x30
        shifts = (16,) if mnemonic == 'SQRT.S' else (11,) if mnemonic in ('ABS.S', 'NEG.S') else (11, 16)
        if (len(encoding) != 4 or word >> 21 != 0x230 or word & 0x3f != function
                or op['opcode'] != opcode or bits != (8 if comparison else 32)
                or len(values) != len(shifts) or len(op['inputs']) != len(shifts)):
            raise Unsupported('COP1 interpreter instruction encoding disagrees with p-code')
        for operand, shift in zip(op['inputs'], shifts):
            if (operand['space'] != 'register' or operand['size'] != 4
                    or int(operand['offset'], 16) != ee_register((word >> shift) & 31)):
                raise Unsupported('COP1 interpreter source disagrees with encoding')
        if not comparison:
            target = op.get('output', {})
            if (target.get('space') != 'register' or target.get('size') != 4
                    or int(target.get('offset', '-1'), 16) != ee_register((word >> 6) & 31)):
                raise Unsupported('COP1 interpreter destination disagrees with encoding')
        control = self.state.get('fcr31', 0x01000001)
        result = execute(mnemonic, values[0] if mnemonic != 'SQRT.S' else 0,
                         values[-1] if mnemonic not in ('ABS.S', 'NEG.S') else 0, control=control)
        self.state['fcr31'] = result['fcr31']
        self.state['reference_models'] = tuple(sorted(set(self.state.get('reference_models', ())) | {MODEL}))
        return boolbits(result['fcr31'] & 0x800000 != 0, bits) if comparison else result['bits']
