"""Differential checks against actual, pinned upstream interpreter handlers."""
import json
import copy
from pathlib import Path
import unittest
from verify import Unsupported
import z3
from ee_interpreter import MODEL, OPERATIONS, execute
from processor_fp import ProcessorFP, validate_instruction, numeric_source


class InterpreterReferenceTests(unittest.TestCase):
    def test_all_captured_interpreter_results_and_guest_flags(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/pcsx2-cop1-interpreter.json').read_text())
        self.assertEqual(fixture['model'], MODEL)
        self.assertEqual(fixture['oracle']['kind'], 'pcsx2-instruction-oracle')
        self.assertFalse(fixture['oracle']['hardware_validated'])
        self.assertEqual({r['mnemonic'] for r in fixture['vectors']}, set(OPERATIONS))
        failures = []
        for row in fixture['vectors']:
            actual = execute(row['mnemonic'], row['a'], row['b'], row['acc_before'], row['initial_fcr31'])
            for name, value in actual.items():
                raw = z3.simplify(value).as_long()
                if raw != row[name]:
                    failures.append((row['mnemonic'], hex(row['a']), hex(row['b']), name, hex(raw), hex(row[name])))
        self.assertEqual(failures[:20], [], f'{len(failures)} mismatched outputs')

    def test_unimplemented_accumulator_variant_fails(self):
        with self.assertRaises(Unsupported):
            execute('MADDA.S', 0, 0)

    def test_actual_fighter_instruction_shapes_against_captured_oracle(self):
        folder = Path(__file__).parent / 'fixtures'
        fighter = json.loads((folder / 'fighter-cop1-instructions.json').read_text())
        oracle = json.loads((folder / 'pcsx2-cop1-interpreter.json').read_text())
        self.assertEqual(fighter['entry'], '00313d80')
        self.assertEqual(fighter['name'], 'CBehaviourFighter::InitState')
        self.assertEqual(len(fighter['instructions']), 9)
        for instruction in fighter['instructions']:
            validate_instruction(instruction, 'ps2')
            op = next(op for op in instruction['ops'] if op['opcode'].startswith('FLOAT_'))
            mnemonic = numeric_source(instruction, 'ps2')['mnemonic'].upper()
            for vector in oracle['vectors']:
                if vector['mnemonic'] != mnemonic:
                    continue
                state = dict(ee_arithmetic_model=MODEL, fcr31=vector['initial_fcr31'])
                operands = [z3.BitVecVal(vector['a'], 32)]
                if len(op['inputs']) == 2:
                    operands.append(z3.BitVecVal(vector['b'], 32))
                comparison = mnemonic == 'C.EQ.S'
                actual = ProcessorFP('ps2', state, lambda claim, reason: None).evaluate(
                    instruction, op, operands, 8 if comparison else 32)
                expected = int(bool(vector['fcr31'] & 0x800000)) if comparison else vector['bits']
                self.assertEqual(z3.simplify(actual).as_long(), expected, instruction['address'])
                self.assertEqual(z3.simplify(state['fcr31']).as_long(), vector['fcr31'], instruction['address'])
                self.assertEqual(state['reference_models'], (MODEL,))

    def test_call_delay_slot_requires_its_own_byte_provenance(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/fighter-cop1-instructions.json').read_text())
        original = next(i for i in fixture['instructions'] if i['mnemonic'] == 'jal')
        for mutation in ('owner', 'slot', 'address', 'count'):
            instruction = copy.deepcopy(original)
            if mutation == 'owner':
                instruction['bytes'] = '00000000'
            elif mutation == 'slot':
                instruction['delay_slot_instructions'][0]['bytes'] = '00000000'
            elif mutation == 'address':
                instruction['delay_slot_instructions'][0]['address'] = '00314828'
            else:
                instruction['delay_slots'] = 2
            with self.subTest(mutation=mutation), self.assertRaises(Unsupported):
                validate_instruction(instruction, 'ps2')

    def instruction(self, mnemonic, function, opcode, *, unary=False):
        node = lambda offset: dict(space='register', offset=hex(offset)[2:], size=4, constant=False)
        word = (0x11 << 26) | (16 << 21) | (2 << 16) | (0 if unary else 1 << 11) | function
        op = dict(opcode=opcode, output=node(0x1004),
                  inputs=[node(0x100c)] if unary else [node(0x1000), node(0x100c)])
        return dict(address='100', text=mnemonic, bytes=word.to_bytes(4, 'little').hex(), ops=[op]), op

    def test_instruction_handlers_validate_bits_and_record_the_model(self):
        def prove(condition, reason):
            solver = z3.Solver()
            solver.add(z3.Not(condition))
            if solver.check() != z3.unsat:
                raise Unsupported(reason)
        for mnemonic, function, opcode, unary in [('ADD.S', 0, 'FLOAT_ADD', False),
                ('SUB.S', 1, 'FLOAT_SUB', False), ('MUL.S', 2, 'FLOAT_MULT', False),
                ('DIV.S', 3, 'FLOAT_DIV', False), ('SQRT.S', 4, 'FLOAT_SQRT', True)]:
            instruction, op = self.instruction(mnemonic, function, opcode, unary=unary)
            validate_instruction(instruction, 'ps2')
            values = [z3.BitVecVal(0x40800000, 32)] if unary else [z3.BitVecVal(0x3f800000, 32)] * 2
            state = dict(ee_arithmetic_model=MODEL)
            actual = ProcessorFP('ps2', state, prove).evaluate(instruction, op, values, 32)
            self.assertEqual(z3.simplify(actual).as_long(), 0x40000000 if unary or mnemonic == 'ADD.S' else
                             0 if mnemonic == 'SUB.S' else 0x3f800000)
            self.assertEqual(state['reference_models'], (MODEL,))
            wrong = dict(instruction, bytes='00000000')
            with self.assertRaises(Unsupported):
                ProcessorFP('ps2', dict(ee_arithmetic_model=MODEL), prove).evaluate(wrong, op, values, 32)
            if mnemonic in ('MUL.S', 'DIV.S', 'SQRT.S'):
                with self.assertRaises(Unsupported):
                    ProcessorFP('ps2', {}, prove).evaluate(instruction, op, values, 32)

    def test_guest_flags_and_model_survive_invocation_boundaries(self):
        from execution_state import ExecutionState
        from transitions import invocation
        from verify import execute as execute_ps2
        memory = z3.Array('interpreter_sequence_memory', z3.BitVecSort(32), z3.BitVecSort(8))
        initial = ExecutionState('ps2', memory, {('register', 0x1000, 4): z3.BitVecVal(0x00800000, 32),
                                               ('register', 0x100c, 4): z3.BitVecVal(0x3f000000, 32)},
                                 processor=dict(ee_arithmetic_model=MODEL, fcr31=0x01008001))
        instruction, _ = self.instruction('MUL.S', 2, 'FLOAT_MULT')
        result = dict(schema='kya.raw-pcode.v1', language='r5900:LE:32:default', big_endian=False, complete=True,
                      entry='100', instructions=[instruction, dict(address='104', ops=[dict(opcode='RETURN', inputs=[
                          dict(space='register', offset='1f0', size=8, constant=False)])])], operation_count=2)
        outcome = execute_ps2(result, z3.BitVecVal(0, 64), memory, state_in=initial, return_state=True,
                              regions=[(0x1000, 4)], fp_mode=True)
        self.assertEqual(z3.simplify(outcome.state.processor['fcr31']).as_long(), 0x01004009)
        self.assertEqual(z3.simplify(outcome.state.registers[('register', 0x1210, 4)]).as_long(), 0x01004009)
        following = invocation(outcome.state)
        self.assertEqual(following.processor['reference_models'], (MODEL,))

    def test_explicit_incoming_callee_saved_fpr_cannot_be_clobbered(self):
        from execution_state import ExecutionState
        from test_dispatch_sets import body, node
        from verify import execute as execute_ps2
        memory = z3.Array('saved_fp_memory', z3.BitVecSort(32), z3.BitVecSort(8))
        incoming = z3.BitVec('incoming_f20', 32)
        initial = ExecutionState('ps2', memory, {('register', 0x1054, 4): incoming})
        graph = body(0x100, [(0x100, [dict(opcode='COPY', output=node('register', 0x1054, 4),
                                         inputs=[node('const', 0, 4)])]),
                             (0x104, [dict(opcode='RETURN', inputs=[node('register', 0x1f0)])])])
        with self.assertRaisesRegex(Unsupported, 'callee-saved register preservation'):
            execute_ps2(graph, z3.BitVecVal(0, 64), memory, state_in=initial, return_state=True,
                        regions=[(0x1000, 4)], graph_mode=True, call_mode=True, fp_mode=True,
                        precondition=z3.BoolVal(True))
        graph['instructions'][0]['ops'][0]['inputs'] = [node('register', 0x1054, 4)]
        self.assertTrue(execute_ps2(graph, z3.BitVecVal(0, 64), memory, state_in=initial, return_state=True,
                                   regions=[(0x1000, 4)], graph_mode=True, call_mode=True, fp_mode=True,
                                   precondition=z3.BoolVal(True)).returned)

    def test_unimplemented_nonfloating_pcode_status_effects_fail_closed(self):
        with self.assertRaisesRegex(Unsupported, 'status handler'):
            validate_instruction(dict(text='MIN.S f0,f1,f2', ops=[]), 'ps2')

    def test_unary_sign_handlers_preserve_payloads_and_clear_cause_flags(self):
        for mnemonic, function in [('ABS.S', 5), ('NEG.S', 7)]:
            instruction, op = self.instruction(mnemonic, function, 'FLOAT_ABS' if function == 5 else 'FLOAT_NEG')
            word = (0x11 << 26) | (16 << 21) | (1 << 11) | function
            instruction['bytes'] = word.to_bytes(4, 'little').hex()
            op['inputs'] = op['inputs'][:1]
            validate_instruction(instruction, 'ps2')
            for value in (0, 0x80000000, 0xffc12345, 0x807fffff):
                state = dict(ee_arithmetic_model=MODEL, fcr31=0x01ffffff)
                actual = ProcessorFP('ps2', state, lambda condition, reason: None).evaluate(
                    instruction, op, [z3.BitVecVal(value, 32)], 32)
                expected = value & 0x7fffffff if function == 5 else value ^ 0x80000000
                self.assertEqual(z3.simplify(actual).as_long(), expected)
                self.assertEqual(z3.simplify(state['fcr31']).as_long(), 0x01ff3fff)
                self.assertEqual(state['reference_models'], (MODEL,))
            with self.assertRaises(Unsupported):
                ProcessorFP('ps2', {}, lambda condition, reason: None).evaluate(
                    instruction, op, [z3.BitVecVal(0, 32)], 32)


if __name__ == '__main__':
    unittest.main()
