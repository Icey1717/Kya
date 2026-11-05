"""Real descriptor bodies with arbitrary IDs, relocated headers and corruptions."""
import copy
from pathlib import Path
import unittest
from animation_descriptor import compare, SYMBOL, ENTRY
from fighter_transitions import load_fixture
from verify import Unsupported


class AnimationDescriptorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest, cls.originals, _ = load_fixture(
            Path(__file__).parent / 'fixtures/animation-descriptor.json.gz')

    def test_all_bounded_table_sizes_and_symbolic_header_aliases(self):
        for count in range(1, 5):
            with self.subTest(table_words=count):
                result = compare(self.manifest, self.originals, table_words=count)
                self.assertEqual(result['result'], 'equivalent')
                self.assertEqual(result['reference_models'], [])
                self.assertFalse(result['full_fighter_contract'])

    def test_lost_descriptor_store_is_detected(self):
        manifest = copy.deepcopy(self.manifest)
        body = next(e for e in manifest['exports'] if e['coff']['symbol'] == SYMBOL)
        instruction, operation = [(i, op) for i in body['instructions'] for op in i['ops']
                                  if op['opcode'] == 'STORE' and op['inputs'][2]['size'] == 4][-1]
        instruction['ops'].remove(operation)
        body['operation_count'] -= 1
        self.assertEqual(compare(manifest, self.originals)['result'], 'mismatch')

    def test_wrong_original_header_pointer_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[ENTRY]['instructions'] if i['address'] == '002414e4')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        operation['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        self.assertEqual(compare(self.manifest, originals, table_words=3)['result'], 'mismatch')

    def test_wrong_return_identity_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[ENTRY]['instructions'] if i['address'] == '00241494')
        operation = next(op for op in instruction['ops'] if op.get('output', {}).get('space') == 'register')
        operation['inputs'] = [dict(space='const', offset='0', size=operation['output']['size'], constant=True)]
        self.assertEqual(compare(self.manifest, originals)['result'], 'mismatch')

    def test_missing_layout_evidence_fails_closed(self):
        manifest = copy.deepcopy(self.manifest)
        del manifest['layouts']['Animation']['state_desc_header_a']
        with self.assertRaisesRegex(Unsupported, 'compiler-backed'):
            compare(manifest, self.originals)


if __name__ == '__main__':
    unittest.main()
