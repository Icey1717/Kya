"""Byte-backed signed conversions against independent processor oracles."""
import copy
import json
from pathlib import Path
import unittest
from verify import Unsupported
import z3
from fighter_transitions import load_fixture
from processor_fp import ProcessorFP, validate_instruction
from ee_interpreter import MODEL
from transitions import prove


class ConversionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        folder = Path(__file__).parent / 'fixtures'
        cls.manifest, cls.originals, _ = load_fixture(folder / 'animation-morphing-callback.json.gz')
        cls.windows = next(i for e in cls.manifest['exports'] if 'MorphingStartDT' in e['coff']['symbol']
                           for i in e['instructions'] if i['mnemonic'] == 'CVTSI2SS')
        cls.ps2 = next(i for i in cls.originals['002410e0']['instructions'] if i['address'] == '00241108')
        cls.folder = folder

    def evaluate(self, architecture, instruction, value, state):
        validate_instruction(instruction, architecture)
        op = next(op for op in instruction['ops'] if op['opcode'] == 'INT2FLOAT')
        result = ProcessorFP(architecture, state, lambda c, r: prove(c, z3.BoolVal(True), r)).evaluate(
            instruction, op, [z3.BitVecVal(value, 32)], 32)
        return z3.simplify(result).as_long()

    def test_native_sse_bits_rounding_and_sticky_precision(self):
        vectors = json.loads((self.folder / 'sse-conversions.json').read_text())
        for row in vectors['vectors']:
            with self.subTest(row=row):
                state = dict(mxcsr=row['control'])
                self.assertEqual(self.evaluate('windows', self.windows, row['a'], state), row['bits'])
                self.assertEqual(z3.simplify(state['mxcsr']).as_long(), row['mxcsr'])

    def test_pinned_ee_bits_and_unchanged_status(self):
        vectors = json.loads((self.folder / 'pcsx2-cop1-interpreter.json').read_text())
        for row in vectors['vectors']:
            if row['mnemonic'] == 'CVT.S.W':
                state = dict(ee_arithmetic_model=MODEL, fcr31=row['initial_fcr31'])
                self.assertEqual(self.evaluate('ps2', self.ps2, row['a'], state), row['bits'])
                self.assertEqual(z3.simplify(state['fcr31']).as_long(), row['fcr31'])
                self.assertEqual(state['reference_models'], (MODEL,))

    def test_removed_or_changed_conversion_rejected(self):
        for architecture, source in (('windows', self.windows), ('ps2', self.ps2)):
            for kind in ('removed', 'source', 'destination', 'bytes'):
                instruction = copy.deepcopy(source)
                operation = next(op for op in instruction['ops'] if op['opcode'] == 'INT2FLOAT')
                if kind == 'removed':
                    instruction['ops'].remove(operation)
                elif kind == 'source':
                    operation['inputs'][0]['offset'] = 'dead'
                elif kind == 'destination':
                    operation['output']['offset'] = 'dead'
                else:
                    instruction['bytes'] = '00000000'
                with self.subTest(architecture=architecture, kind=kind), self.assertRaises(Unsupported):
                    validate_instruction(instruction, architecture)

    def test_memory_source_displacement_base_and_load_checked(self):
        for kind in ('displacement', 'base', 'load', 'width'):
            instruction = copy.deepcopy(self.windows)
            if kind == 'displacement':
                instruction['ops'][0]['inputs'][1]['offset'] = '0'
            elif kind == 'base':
                instruction['ops'][0]['inputs'][0]['offset'] = '0'
            elif kind == 'load':
                instruction['ops'][1]['inputs'][1]['offset'] = 'dead'
            else:
                instruction['bytes'] = 'f3480f2ac0'  # 64-bit input remains unsupported.
            with self.subTest(kind=kind), self.assertRaises(Unsupported):
                validate_instruction(instruction, 'windows')

    def test_unmasked_sse_precision_and_missing_reference_fail_closed(self):
        with self.assertRaisesRegex(Unsupported, 'unmasked'):
            self.evaluate('windows', self.windows, 16777217, dict(mxcsr=0xf80))
        with self.assertRaisesRegex(Unsupported, 'opt-in'):
            self.evaluate('ps2', self.ps2, 500, {})


if __name__ == '__main__':
    unittest.main()
