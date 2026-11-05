"""Signed and unsigned <= semantics over complete symbolic 32/64-bit domains."""
import unittest
from verify import Unsupported, execute as execute_ps2
import z3
from execution_state import ExecutionState
from windows_pcode import execute as execute_windows


def node(space, offset, size):
    return dict(space=space, offset=hex(offset)[2:], size=size, constant=space == 'const')


class IntegerComparisonTests(unittest.TestCase):
    def test_fragmented_saved_register_entry_is_preserved_and_checked(self):
        value = z3.BitVec('saved_s5', 128)
        memory = z3.Array('saved_memory', z3.BitVecSort(32), z3.BitVecSort(8))
        state = ExecutionState('ps2', memory, {('register', 0x150 + i * 4, 4):
                               z3.Extract(i * 32 + 31, i * 32, value) for i in range(4)})
        export = dict(schema='kya.raw-pcode.v1', language='r5900:LE:32:default', complete=True,
                      big_endian=False, entry='100', operation_count=1, instructions=[dict(address='100', ops=[
                          dict(opcode='RETURN', inputs=[node('register', 0x1f0, 8)])])])
        options = dict(state_in=state, return_state=True, call_mode=True, graph_mode=True,
                       regions=[(0x7f00, 0x110)], precondition=z3.BoolVal(True))
        self.assertTrue(execute_ps2(export, z3.BitVecVal(0, 64), memory, **options).returned)
        export['instructions'][0]['ops'].insert(0, dict(opcode='COPY', output=node('register', 0x150, 8),
                                                       inputs=[node('const', 0, 8)]))
        export['operation_count'] += 1
        with self.assertRaisesRegex(Unsupported, 'callee-saved register preservation'):
            execute_ps2(export, z3.BitVecVal(0, 64), memory, **options)
        options['state_in'] = ExecutionState('ps2', memory)
        with self.assertRaisesRegex(Unsupported, 'Unmodeled register'):
            execute_ps2(export, z3.BitVecVal(0, 64), memory, **options)

    def test_less_equal_all_values_including_equality_and_sign_boundary(self):
        for side in ('windows', 'ps2'):
            for bits in (32, 64):
                for opcode in ('INT_LESSEQUAL', 'INT_SLESSEQUAL'):
                    with self.subTest(side=side, bits=bits, opcode=opcode):
                        x, y = z3.BitVecs('comparison_x comparison_y', bits)
                        extend = lambda value: z3.ZeroExt(64 - bits, value) if bits < 64 else value
                        ops = [dict(opcode=opcode, output=node('unique', 0x100, 1), inputs=[
                            node('register', 0x10 if side == 'windows' else 0x50, bits // 8),
                            node('register', 0x80 if side == 'windows' else 0x60, bits // 8)]),
                            dict(opcode='INT_ZEXT', output=node('register', 0 if side == 'windows' else 0x20, 8),
                                 inputs=[node('unique', 0x100, 1)])]
                        if side == 'windows':
                            ops += [dict(opcode='LOAD', output=node('register', 0x288, 8), inputs=[
                                node('const', 0x1b1, 8), node('register', 0x20, 8)]),
                                dict(opcode='INT_ADD', output=node('register', 0x20, 8), inputs=[
                                    node('register', 0x20, 8), node('const', 8, 8)])]
                        ops.append(dict(opcode='RETURN', inputs=[node('register', 0x288 if side == 'windows' else 0x1f0, 8)]))
                        export = dict(schema='kya.raw-pcode.v1', language='x86:LE:64:default' if side == 'windows' else 'r5900:LE:32:default',
                                      compiler='windows' if side == 'windows' else 'default', complete=True, big_endian=False,
                                      entry='100', instructions=[dict(address='100', ops=ops)], operation_count=len(ops))
                        if side == 'windows':
                            memory = z3.Array('comparison_memory', z3.BitVecSort(64), z3.BitVecSort(8))
                            actual = execute_windows(export, extend(x), memory, second_argument=extend(y))[0]
                        else:
                            actual = execute_ps2(export, extend(x), second_argument=extend(y))
                        expected = z3.If(z3.ULE(x, y) if opcode == 'INT_LESSEQUAL' else x <= y,
                                         z3.BitVecVal(1, 64), z3.BitVecVal(0, 64))
                        solver = z3.Solver()
                        solver.add(actual != expected)
                        self.assertEqual(solver.check(), z3.unsat)


if __name__ == '__main__':
    unittest.main()
