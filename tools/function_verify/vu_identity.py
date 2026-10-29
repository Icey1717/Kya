"""Exact R5900 identity-matrix instruction idiom; not general VU emulation."""
import re
from verify import Unsupported
import z3


def node(space, offset, size):
    return dict(space=space, offset=hex(offset)[2:], size=size, constant=space == 'const')


def evaluate(op, text, read):
    """Check both decoded operands and raw user-op shape before interpreting it."""
    move = re.fullmatch(r'vmove\.xyzw vf(\d+),zero', text)
    rotate = re.fullmatch(r'vmr32\.xyzw vf(\d+),vf(\d+)', text)
    add = re.fullmatch(r'vadd\.w vf(\d+),vf(\d+),vf0', text)
    match = move or rotate or add
    if not match or not 4 <= int(match[1]) <= 7:
        raise Unsupported('Unapproved VU identity instruction')
    output = node('register', 0x4100 + int(match[1]) * 16, 16)
    if op.get('output') != output:
        raise Unsupported('VU output disagrees with decoded instruction')
    if move:
        expected = [node('const', 0x48, 4), node('const', 15, 1)]
        value = z3.BitVecVal(0, 128)
    elif rotate:
        source = node('register', 0x4100 + int(match[2]) * 16, 16)
        expected = [node('const', 0x35, 4), node('const', 15, 1), source]
        value = z3.RotateRight(read(source), 32)
    else:
        source = node('register', 0x4100 + int(match[2]) * 16, 16)
        zero = node('register', 0x4100, 16)
        expected = [node('const', 0x1d, 4), node('const', 1, 1), source, zero]
        # Only exact +0 + +1 is implemented; no NaN, denormal, rounding,
        # overflow, or general PS2 floating-point arithmetic is approximated.
        if not z3.is_true(z3.simplify(z3.And(read(source) == 0, read(zero) == (0x3f800000 << 96)))):
            raise Unsupported('VU add requires proven identity constants')
        value = z3.BitVecVal(0x3f800000 << 96, 128)
    if op['inputs'] != expected:
        raise Unsupported('Unapproved VU user-op operands or lane mask')
    return value
