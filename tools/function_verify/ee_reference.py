"""Explicitly conditional EE add/sub model based on PCSX2's corrected path.

This is not a hardware oracle. Callers must opt into the pinned reference and
reports must retain that assumption. Multiplication/division are not supplied:
the same upstream source explicitly documents inaccuracies for those ops.
"""
from verify import z3

REVISION = '1f67c53b6723fcd6a90041a5b296f0e34a6a3222'
MODEL = 'pcsx2-corrected-ee-add-sub:' + REVISION
SOURCE = 'https://github.com/PCSX2/pcsx2/blob/' + REVISION + '/pcsx2/x86/iFPUd.cpp'


def add_sub(a, b, subtract=False):
    """Mask alignment bits, compute at EE precision with an extended exponent.

    The smaller operand retains a single alignment guard bit. Differences of
    25 or more discard its magnitude. Exponent-zero inputs are signed zero.
    Underflow retains normalized fraction bits; overflow saturates at EE Fmax.
    Returns raw result plus overflow/underflow cause predicates.
    """
    if a.size() != 32 or b.size() != 32:
        raise ValueError('EE reference requires 32-bit operands')
    def exponent(v):
        return z3.Extract(30, 23, v)
    a, b = [z3.If(exponent(v) == 0, v & 0x80000000, v) for v in (a, b)]
    difference = z3.ZeroExt(24, exponent(a)) - z3.ZeroExt(24, exponent(b))
    def truncate(value, distance):
        return z3.If(distance >= 25, value & 0x80000000,
                     z3.If(distance > 0, value & (z3.BitVecVal(0xffffffff, 32) << (distance - 1)), value))
    adjusted_a, adjusted_b = truncate(a, -difference), truncate(b, difference)
    sort = z3.FPSort(9, 24)
    def decode(value):
        raw = z3.Concat(z3.Extract(31, 31, value), z3.ZeroExt(1, exponent(value)) + 128,
                        z3.Extract(22, 0, value))
        raw = z3.If(exponent(value) == 0, z3.Concat(z3.Extract(31, 31, value), z3.BitVecVal(0, 32)), raw)
        return z3.fpBVToFP(raw, sort)
    result = (z3.fpSub if subtract else z3.fpAdd)(z3.RTZ(), decode(adjusted_a), decode(adjusted_b))
    raw = z3.fpToIEEEBV(result)
    exp = z3.Extract(31, 23, raw)
    sign, fraction = z3.Extract(32, 32, raw), z3.Extract(22, 0, raw)
    overflow = z3.UGT(exp, 383)
    underflow = z3.And(z3.Not(z3.fpIsZero(result)), z3.ULT(exp, 129))
    normal = z3.Concat(sign, z3.Extract(7, 0, exp - 128), fraction)
    encoded = z3.If(overflow, z3.Concat(sign, z3.BitVecVal(0x7fffffff, 31)),
                  z3.If(underflow, z3.Concat(sign, z3.BitVecVal(0, 8), fraction),
                        z3.If(z3.fpIsZero(result), z3.Concat(sign, z3.BitVecVal(0, 31)), normal)))
    return encoded, overflow, underflow
