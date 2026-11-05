"""Conditional moves retain intra-instruction temporaries and skipped effects."""
import unittest
from verify import Unsupported
import z3
from paths import expand
from windows_pcode import execute


def node(space, offset, size):
    return dict(space=space, offset=hex(offset)[2:], size=size, constant=space == 'const')


class ConditionalMoveTests(unittest.TestCase):
    def test_taken_branch_skips_suffix_and_untaken_keeps_unique_temporary(self):
        export = dict(language='x86:LE:64:default', compiler='windows', complete=True, entry='100',
                      instructions=[dict(address='100', ops=[
                          dict(opcode='COPY', output=node('register', 0, 8), inputs=[node('const', 7, 8)]),
                          dict(opcode='COPY', output=node('unique', 0x100, 8), inputs=[node('const', 9, 8)]),
                          dict(opcode='CBRANCH', inputs=[node('ram', 0x101, 8), node('register', 0x10, 8)]),
                          dict(opcode='COPY', output=node('register', 0, 8), inputs=[node('unique', 0x100, 8)])]),
                          dict(address='101', ops=[
                              dict(opcode='LOAD', output=node('register', 0x288, 8), inputs=[node('const', 0x1b1, 4), node('register', 0x20, 8)]),
                              dict(opcode='INT_ADD', output=node('register', 0x20, 8), inputs=[node('register', 0x20, 8), node('const', 8, 8)]),
                              dict(opcode='RETURN', inputs=[node('register', 0x288, 8)])])], operation_count=7)
        memory = z3.Array('cmov_memory', z3.BitVecSort(64), z3.BitVecSort(8))
        paths = expand(export)
        for argument, expected in [(1, 7), (0, 9)]:
            results = [execute(p, z3.BitVecVal(argument, 64), memory, path_mode=True) for p in paths]
            results = [r for r in results if r is not None]
            self.assertEqual(len(results), 1)
            self.assertEqual(z3.simplify(results[0][0]).as_long(), expected)


if __name__ == '__main__':
    unittest.main()
