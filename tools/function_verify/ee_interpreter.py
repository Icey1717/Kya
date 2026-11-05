"""Conditional semantics of the pinned PCSX2 COP1 interpreter profile.

This backend clamps exponent-255 inputs to IEEE Fmax. It is intentionally
distinct from the corrected extended-FPU recompiler reference in ee_reference.
Host arithmetic is binary32, toward zero, with DAZ/FTZ disabled.
"""
from verify import Unsupported
import z3
from ee_reference import REVISION

MODEL = 'pcsx2-cop1-interpreter:' + REVISION
MXCSR = 0x7f80
OPERATIONS = ('ADD.S', 'SUB.S', 'MUL.S', 'DIV.S', 'SQRT.S', 'RSQRT.S',
              'ABS.S', 'NEG.S', 'MOV.S', 'MIN.S', 'MAX.S', 'ADDA.S', 'SUBA.S',
              'MULA.S', 'MADD.S', 'MSUB.S', 'C.EQ.S', 'C.LT.S', 'C.LE.S', 'CVT.S.W', 'CVT.W.S')


def bv(value):
    return z3.BitVecVal(value, 32) if isinstance(value, int) else value


def normalize(value):
    exponent = value & 0x7f800000
    return z3.If(exponent == 0, value & 0x80000000,
                 z3.If(exponent == 0x7f800000, (value & 0x80000000) | 0x7f7fffff, value))


def execute(mnemonic, a, b, acc=0, control=0x01000001, destination=0xdeadbeef):
    a, b, acc, control, destination = map(bv, (a, b, acc, control, destination))
    if any(value.size() != 32 for value in (a, b, acc, control, destination)):
        raise ValueError('COP1 interpreter requires 32-bit register values')
    if mnemonic not in OPERATIONS:
        raise Unsupported('Unsupported pinned COP1 interpreter operation: ' + mnemonic)
    to_float = lambda bits: z3.fpBVToFP(bits, z3.Float32())
    fa, fb = map(to_float, (normalize(a), normalize(b)))
    rounding = z3.RTZ()

    def binary(op, left=fa, right=fb):
        return z3.fpToIEEEBV(op(rounding, left, right))

    def finish(raw, old, flags=True):
        overflow = raw & 0x7fffffff == 0x7f800000
        tiny = z3.And(raw & 0x7f800000 == 0, raw & 0x7fffff != 0)
        result = z3.If(overflow, (raw & 0x80000000) | 0x7f7fffff,
                       z3.If(tiny, raw & 0x80000000, raw))
        if not flags:
            return result, old
        # An overflow returns before underflow processing, preserving its cause.
        updated = z3.If(overflow, old | 0x8010,
                         z3.If(tiny, (old & ~0x8000) | 0x4008, old & ~0xc000))
        return result, updated

    output, updated_acc, updated = destination, acc, control
    if mnemonic in ('C.EQ.S', 'C.LT.S', 'C.LE.S'):
        condition = {'C.EQ.S': z3.fpEQ, 'C.LT.S': z3.fpLT, 'C.LE.S': z3.fpLEQ}[mnemonic](fa, fb)
        updated = (control & ~0x800000) | z3.If(condition, bv(0x800000), bv(0))
    elif mnemonic in ('MOV.S', 'ABS.S', 'NEG.S'):
        output = a if mnemonic == 'MOV.S' else a & 0x7fffffff if mnemonic == 'ABS.S' else a ^ 0x80000000
        if mnemonic != 'MOV.S':
            updated = control & ~0xc000
    elif mnemonic in ('MIN.S', 'MAX.S'):
        both_negative = z3.And(a < 0, b < 0)
        choose_a = z3.If(both_negative, a > b, a < b) if mnemonic == 'MIN.S' else z3.If(both_negative, a < b, a > b)
        output = z3.If(choose_a, a, b)
        updated = control & ~0xc000
    elif mnemonic == 'CVT.S.W':
        output = z3.fpToIEEEBV(z3.fpSignedToFP(rounding, a, z3.Float32()))
    elif mnemonic == 'CVT.W.S':
        in_range = z3.ULE(a & 0x7f800000, 0x4e800000)
        output = z3.If(in_range, z3.fpToSBV(rounding, to_float(a), z3.BitVecSort(32)),
                       z3.If(a < 0, bv(0x80000000), bv(0x7fffffff)))
    elif mnemonic == 'SQRT.S':
        output = z3.If(b & 0x7f800000 == 0, b & 0x80000000,
                       z3.fpToIEEEBV(z3.fpSqrt(rounding, z3.fpAbs(fb))))
        negative = z3.And(b & 0x7f800000 != 0, b < 0)
        updated = (control & ~0x30000) | z3.If(negative, bv(0x20040), bv(0))
    elif mnemonic in ('DIV.S', 'RSQRT.S'):
        if mnemonic == 'DIV.S':
            raw = binary(z3.fpDiv)
            output, _ = finish(raw, control, flags=False)
            exceptional_flags = z3.If(a & 0x7f800000 == 0, bv(0x20040), bv(0x10020))
            zero_output = ((a ^ b) & 0x80000000) | 0x7f7fffff
            initial = control
        else:
            denominator = z3.fpSqrt(rounding, z3.fpAbs(fb))
            raw = binary(z3.fpDiv, fa, denominator)
            output, _ = finish(raw, control, flags=False)
            initial = (control & ~0x30000) | z3.If(b < 0, bv(0x20040), bv(0))
            exceptional_flags = bv(0x10020)
            zero_output = (b & 0x80000000) | 0x7f7fffff
        is_zero = b & 0x7f800000 == 0
        output = z3.If(is_zero, zero_output, output)
        updated = z3.If(is_zero, (control if mnemonic == 'DIV.S' else control & ~0x30000) | exceptional_flags, initial)
    else:
        if mnemonic in ('MADD.S', 'MSUB.S'):
            product = normalize(binary(z3.fpMul))
            raw = binary(z3.fpAdd if mnemonic == 'MADD.S' else z3.fpSub,
                         to_float(normalize(acc)), to_float(product))
        else:
            operation = z3.fpAdd if mnemonic in ('ADD.S', 'ADDA.S') else z3.fpSub if mnemonic in ('SUB.S', 'SUBA.S') else z3.fpMul
            raw = binary(operation)
        result, updated = finish(raw, control)
        if mnemonic in ('ADDA.S', 'SUBA.S', 'MULA.S'):
            updated_acc = result
        else:
            output = result
    return dict(bits=output, acc=updated_acc, fcr31=updated)
