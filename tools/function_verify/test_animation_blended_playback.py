"""Nonzero pointer indexing and real multi-key initialization fault controls."""
import copy
from pathlib import Path
import unittest
from animation_playback import compare, BLENDS, MODEL, UPDATE_ENTRY, INITIALIZE_ENTRY
from fighter_transitions import load_fixture
from processor_fp import ProcessorFP, ee_register
from transitions import prove
from verify import Unsupported
import z3


class BlendedPlaybackTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest, cls.originals, _ = load_fixture(
            Path(__file__).parent / 'fixtures/animation-blended-playback.json.gz')

    def check_equivalent(self, **case):
        result = compare(self.manifest, self.originals, reference_model=MODEL, **case)
        self.assertEqual(result['result'], 'conditional-equivalent')
        self.assertEqual(result['reference_models'], [MODEL])
        self.assertEqual(result['numeric_policies'], {})
        self.assertFalse(result['full_fighter_contract'])
        self.assertEqual(result['steps'][-1]['name'], 'ReplaySameAnimation')
        return result

    def test_nonzero_indices_select_distinct_records(self):
        for key_index in (1, 2):
            with self.subTest(key_index=key_index):
                self.check_equivalent(key_index=key_index)

    def test_blend_terms_aliases_and_weight_sum_branches(self):
        for blend in BLENDS:
            with self.subTest(blend=blend):
                result = self.check_equivalent(blend=blend)
                self.assertIn(UPDATE_ENTRY, result['original_entries'])

    def test_blended_negative_start_and_duration_are_clamped(self):
        for timing in ((-1, 1), (2, 1)):
            with self.subTest(timing=timing):
                self.check_equivalent(blend='alias', timing=timing)

    def test_lost_nonzero_index_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[INITIALIZE_ENTRY]['instructions'] if i['address'] == '00241e28')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'INT_LEFT')
        operation['inputs'][0] = dict(space='const', offset='0', size=operation['inputs'][0]['size'], constant=True)
        self.assertEqual(compare(self.manifest, originals, key_index=2, reference_model=MODEL)['result'], 'mismatch')

    def test_lost_blended_duration_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[UPDATE_ENTRY]['instructions'] if i['address'] == '00241c48')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        operation['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        self.assertEqual(compare(self.manifest, originals, blend='two', reference_model=MODEL)['result'], 'mismatch')

    def test_lost_normalization_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[UPDATE_ENTRY]['instructions'] if i['address'] == '00241c70')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        instruction['ops'].remove(operation)
        originals[UPDATE_ENTRY]['operation_count'] -= 1
        self.assertEqual(compare(self.manifest, originals, blend='two', reference_model=MODEL)['result'], 'mismatch')

    def test_missing_update_body_fails_closed(self):
        originals = copy.deepcopy(self.originals)
        del originals[UPDATE_ENTRY]
        with self.assertRaisesRegex(Unsupported, 'complete original/compiled'):
            compare(self.manifest, originals, blend='two', reference_model=MODEL)

    def test_reachable_callback_is_a_frontier(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[UPDATE_ENTRY]['instructions'] if i['address'] == '00241b54')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'CBRANCH')
        operation['inputs'][1] = dict(space='const', offset='0', size=1, constant=True)
        with self.assertRaisesRegex(Unsupported, 'Unresolved|frontier|graph transfer'):
            compare(self.manifest, originals, blend='two', reference_model=MODEL)

    def test_f3_f4_operands_are_byte_checked(self):
        for address, raw, register, wrong in [('00241c1c', '02000346', 3, '100c'),
                                                ('00241c38', '00210346', 4, '1010')]:
            with self.subTest(address=address):
                instruction = next(i for i in self.originals[UPDATE_ENTRY]['instructions'] if i['address'] == address)
                self.assertEqual(instruction['bytes'], raw)
                operation = copy.deepcopy(instruction['ops'][0])
                operand = operation['inputs'][1 if register == 3 else 0]
                self.assertEqual(int(operand['offset'], 16), ee_register(register))
                operand['offset'] = wrong
                fp = ProcessorFP('ps2', {'ee_arithmetic_model': MODEL},
                                 lambda claim, reason: prove(claim, z3.BoolVal(True), reason))
                with self.assertRaisesRegex(Unsupported, 'source disagrees'):
                    fp.evaluate(instruction, operation, [z3.BitVecVal(0, 32)] * 2, 32)

    def test_invalid_domains_fail_closed(self):
        for case in [dict(key_index=-1), dict(key_index=3), dict(key_index=True),
                     dict(blend='unbounded'), dict(blend='two', key_index=1)]:
            with self.subTest(case=case), self.assertRaises(ValueError):
                compare(self.manifest, self.originals, reference_model=MODEL, **case)


if __name__ == '__main__':
    unittest.main()
