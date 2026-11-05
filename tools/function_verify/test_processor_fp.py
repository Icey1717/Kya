"""Processor distinctions and native-validated scalar SSE results/status."""
import json
from pathlib import Path
import unittest
from verify import Unsupported
import z3
from processor_fp import ProcessorFP, ee_compare, validate_instruction


def assert_claim(claim, reason):
    solver = z3.Solver()
    solver.set(timeout=5000)
    solver.add(z3.Not(claim))
    if solver.check() != z3.unsat:
        raise Unsupported(reason)


def evaluate(mnemonic, a, b, mxcsr):
    if mnemonic in ('COMISS', 'UCOMISS'):
        processor = dict(mxcsr=mxcsr)
        fp = ProcessorFP('windows', processor, assert_claim)
        instruction = dict(text=mnemonic + ' XMM0,XMM1', bytes='0f2fc1' if mnemonic == 'COMISS' else '0f2ec1')
        a, b = z3.BitVecVal(a, 32), z3.BitVecVal(b, 32)
        isnan = [fp.evaluate(instruction, dict(opcode='FLOAT_NAN'), [v], 8) for v in (a, b)]
        equal = fp.evaluate(instruction, dict(opcode='FLOAT_EQUAL'), [a, b], 8)
        less = fp.evaluate(instruction, dict(opcode='FLOAT_LESS'), [a, b], 8)
        unordered = isnan[0] | isnan[1]
        flags = (unordered | less) | (unordered << 2) | ((unordered | equal) << 6)
        return z3.simplify(flags).as_long(), z3.simplify(processor['mxcsr']).as_long()
    operation = {'ADDSS': 'FLOAT_ADD', 'SUBSS': 'FLOAT_SUB', 'MULSS': 'FLOAT_MULT', 'DIVSS': 'FLOAT_DIV'}[mnemonic]
    encoding = {'ADDSS': 'f30f58c1', 'SUBSS': 'f30f5cc1', 'MULSS': 'f30f59c1', 'DIVSS': 'f30f5ec1'}[mnemonic]
    processor = dict(mxcsr=mxcsr)
    value = ProcessorFP('windows', processor, assert_claim).evaluate(
        dict(text=mnemonic + ' XMM0,XMM1', bytes=encoding), dict(opcode=operation),
        [z3.BitVecVal(a, 32), z3.BitVecVal(b, 32)], 32)
    return z3.simplify(value).as_long(), z3.simplify(processor['mxcsr']).as_long()


class ProcessorTests(unittest.TestCase):
    def test_ee_extended_exponent_is_finite(self):
        # IEEE sees NaN; EE sees a finite positive number larger than 1.
        result = ee_compare('FLOAT_LESS', [z3.BitVecVal(0x3f800000, 32), z3.BitVecVal(0x7fffffff, 32)], 8)
        self.assertEqual(z3.simplify(result).as_long(), 1)

    def test_ee_denormal_encodings_are_signed_zero(self):
        for a, b in [(1, 0), (0x80000001, 0), (0x80000000, 0)]:
            self.assertEqual(z3.simplify(ee_compare('FLOAT_EQUAL',
                [z3.BitVecVal(a, 32), z3.BitVecVal(b, 32)], 8)).as_long(), 1)

    def test_ee_rounding_is_not_approximated(self):
        with self.assertRaisesRegex(Unsupported, 'non-IEEE'):
            ProcessorFP('ps2', {}, assert_claim).evaluate(dict(text='add.S f0,f1,f2', bytes='00000000'),
                dict(opcode='FLOAT_ADD'), [z3.BitVecVal(0, 32)] * 2, 32)

    def test_native_sse_vectors(self):
        path = Path(__file__).parent / 'fixtures/sse-scalar.json'
        vectors = json.loads(path.read_text())
        for row in vectors['vectors']:
            with self.subTest(row=row):
                result, status = evaluate(row['mnemonic'], row['a'], row['b'], row['control'])
                self.assertEqual(result, row['flags'] if row['mnemonic'] in ('COMISS', 'UCOMISS') else row['bits'])
                self.assertEqual(status, row['mxcsr'])

    def test_unmasked_exception_is_frontier(self):
        with self.assertRaisesRegex(Unsupported, 'unmasked'):
            evaluate('DIVSS', 0x3f800000, 0, 0x1d80)

    def test_nan_payload_arithmetic_is_frontier(self):
        with self.assertRaisesRegex(Unsupported, 'payload'):
            evaluate('ADDSS', 0x7fc00001, 0x3f800000, 0x1f80)

    def test_instruction_bytes_required(self):
        with self.assertRaisesRegex(Unsupported, 'provenance'):
            ProcessorFP('windows', dict(mxcsr=0x1f80), assert_claim).evaluate(
                dict(text='ADDSS XMM0,XMM1'), dict(opcode='FLOAT_ADD'), [z3.BitVecVal(0, 32)] * 2, 32)

    def test_control_writes_are_not_accepted_as_ordinary_copies(self):
        for architecture, mnemonic in [('windows', 'LDMXCSR'), ('ps2', 'CTC1')]:
            with self.subTest(mnemonic=mnemonic), self.assertRaisesRegex(Unsupported, 'control instruction'):
                validate_instruction(dict(text=mnemonic, ops=[]), architecture)

    def test_removed_comparison_operation_is_rejected(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/effect-callees.json').read_text())
        instruction = next(i for e in fixture['manifest']['exports'] for i in e['instructions']
                           if i.get('mnemonic') == 'UCOMISS')
        instruction['ops'] = [op for op in instruction['ops'] if op['opcode'] != 'FLOAT_EQUAL']
        with self.assertRaisesRegex(Unsupported, 'shape changed'):
            validate_instruction(instruction, 'windows')

