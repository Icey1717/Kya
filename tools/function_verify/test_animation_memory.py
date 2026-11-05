"""Compiled Debug validation must preserve gameplay memory and update diagnostics."""
import copy
import json
from pathlib import Path
import unittest
from verify import Unsupported
from animation_memory import compare


class AnimationMemoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = json.loads((Path(__file__).parent / 'fixtures/animation-validator.json').read_text())

    def test_both_entries_and_arbitrary_wrapping_history_counter(self):
        for animation in (0, 1):
            with self.subTest(animation=animation):
                self.assertEqual(compare(self.manifest, animation=animation)['result'], 'equivalent')

    def test_lost_history_counter_update_is_a_mismatch(self):
        manifest = copy.deepcopy(self.manifest)
        target = manifest['addresses'][manifest['globals']['lookup_count']['symbol']]
        writes = [op for i in manifest['exports'][0]['instructions'] for op in i['ops']
                  if op.get('output', {}).get('space') == 'ram' and int(op['output']['offset'], 16) == target]
        self.assertEqual(len(writes), 1)
        writes[0]['inputs'] = [dict(space='const', offset='0', size=4, constant=True)]
        self.assertEqual(compare(manifest)['result'], 'mismatch')

    def test_three_playback_records_with_descriptor_payload_gaps(self):
        for animation in range(3):
            with self.subTest(animation=animation):
                result = compare(self.manifest, animation=animation, entry_count=3, bank_offsets=(12, 32, 52))
                self.assertEqual(result['result'], 'equivalent')
                self.assertEqual(result['bank_offsets'], [12, 32, 52])

    def test_invalid_bank_layouts_are_rejected(self):
        for offsets in ((8, 12), (12, 12, 16), (12, 14, 20), (12, 32, 4096)):
            with self.subTest(offsets=offsets), self.assertRaises(ValueError):
                compare(self.manifest, entry_count=3, bank_offsets=offsets)

    def test_validator_failure_is_never_treated_as_success(self):
        manifest = copy.deepcopy(self.manifest)
        root = manifest['exports'][0]
        # Every valid input must have nonnull layer storage. Invert the first
        # branch to force that input into the explicit failure-call frontier.
        branch = next(o for i in root['instructions'] for o in i['ops'] if o['opcode'] == 'CBRANCH')
        branch['inputs'][1] = dict(space='const', offset='1', size=1, constant=True)
        with self.assertRaises(Unsupported):
            compare(manifest)


if __name__ == '__main__':
    unittest.main()
