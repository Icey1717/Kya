"""Reference-mode arithmetic stays distinct from unconditional EE proofs."""
import json
from pathlib import Path
import unittest
from verify import Unsupported
import z3
from ee_reference import MODEL, add_sub
from processor_fp import ProcessorFP
from test_processor_fp import assert_claim
from execution_state import ExecutionState
from memory_relation import MemoryRelation, store
from transitions import TransitionStep, compare


class EEReferenceTests(unittest.TestCase):
    def test_concrete_reference_vectors(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/ee-add-sub-reference.json').read_text())
        self.assertEqual(fixture['model'], MODEL)
        for row in fixture['vectors']:
            with self.subTest(row=row):
                value, overflow, underflow = add_sub(z3.BitVecVal(row['a'], 32), z3.BitVecVal(row['b'], 32), row['mnemonic'] == 'SUB.S')
                flags = (row['initial_fcr31'] & ~0xc000) | (0x8010 if z3.is_true(z3.simplify(overflow)) else 0) | (0x4008 if z3.is_true(z3.simplify(underflow)) else 0)
                self.assertEqual(z3.simplify(value).as_long(), row['bits'])
                self.assertEqual(flags, row['fcr31'])

    def test_profile_is_explicit_and_recorded(self):
        node = lambda offset: dict(space='register', offset=f'{offset:x}', size=4, constant=False)
        instruction = dict(text='add.S f0,f1,f2', bytes='00080246')
        op = dict(opcode='FLOAT_ADD', inputs=[node(0x1000), node(0x100c)], output=node(0x1004))
        values = [z3.BitVecVal(0x3f800000, 32)] * 2
        with self.assertRaisesRegex(Unsupported, 'opt-in'):
            ProcessorFP('ps2', {}, assert_claim).evaluate(instruction, op, values, 32)
        state = dict(ee_arithmetic_model=MODEL)
        result = ProcessorFP('ps2', state, assert_claim).evaluate(instruction, op, values, 32)
        self.assertEqual(z3.simplify(result).as_long(), 0x40000000)
        self.assertEqual(state['reference_models'], (MODEL,))

    def test_execution_and_composition_keep_the_reference_assumption(self):
        from test_dispatch_sets import body, node
        from check import validate_report
        fp = dict(opcode='FLOAT_ADD', inputs=[node('register', 0x1000, 4), node('register', 0x100c, 4)], output=node('register', 0x1004, 4))
        ps2 = body(0x100, [(0x100, [fp]), (0x104, [dict(opcode='STORE', inputs=[node('const', 0x1a1, 4),
            node('const', 0x1000, 4), node('register', 0x1004, 4)])]),
            (0x108, [dict(opcode='RETURN', inputs=[node('register', 0x1f0)])])])
        ps2['instructions'][0].update(text='add.S f0,f1,f2', bytes='00080246')
        windows = body(0x100, [(0x100, [dict(opcode='STORE', inputs=[node('const', 0x1b1),
            node('const', 0x1000), node('const', 0x40000000, 4)])]),
            (0x104, [dict(opcode='COPY', output=node('register', 0x20), inputs=[node('const', 0x8008)]),
                     dict(opcode='COPY', output=node('register', 0x288), inputs=[node('const', 0xdead0000)]),
                     dict(opcode='RETURN', inputs=[node('register', 0x288)])])])
        windows.update(language='x86:LE:64:default', compiler='windows')
        relation = MemoryRelation()
        for side in ('windows', 'ps2'):
            relation.region(side, 'value', 0x1000, 4)
        relation.field('value', 0, 'value', 0, 4)
        states = [ExecutionState(side, store(z3.Array(side + '_ee_ref', z3.BitVecSort(bits), z3.BitVecSort(8)),
                                            0x1000, z3.BitVecVal(0, 32))) for side, bits in [('windows', 64), ('ps2', 32)]]
        states[1].processor['ee_arithmetic_model'] = MODEL
        states[1].registers = {('register', offset, 4): z3.BitVecVal(0x3f800000, 32) for offset in (0x1000, 0x100c)}
        step = TransitionStep('reference add', windows, ps2, z3.BitVecVal(0, 64),
                              windows_options=dict(regions=[(0x1000, 4)]), ps2_options=dict(regions=[(0x1000, 4)], fp_mode=True))
        result = compare([step], *states, relation, z3.BoolVal(True))
        self.assertEqual(result['result'], 'conditional-equivalent')
        self.assertEqual(result['reference_models'], [MODEL])
        result['ps2_entry'] = '100'
        with self.assertRaisesRegex(ValueError, 'reference semantics'):
            validate_report(dict(functions=[result]), ['100'])

