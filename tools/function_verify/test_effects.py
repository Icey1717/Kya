"""Actual compiled LOD callees, complete child dispatch, and negative controls."""
import copy
import json
from pathlib import Path
import unittest
from unittest.mock import patch
from effects import compare_case
from call_graph import link
from verify import Unsupported
from check import validate_report


class EffectTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixture = json.loads((Path(__file__).parent / 'fixtures/effect-callees.json').read_text())

    def test_actual_compiled_callees(self):
        self.assertEqual(compare_case(self.fixture['manifest'], self.fixture['originals'], 'LodIsLooped')['result'], 'equivalent')
        for handle in ('null', 'zero-id', 'stale', 'valid'):
            with self.subTest(handle=handle):
                self.assertEqual(compare_case(self.fixture['manifest'], self.fixture['originals'], 'LodStop', handle)['result'], 'equivalent')

    def test_incorrect_looped_return_detected(self):
        manifest = copy.deepcopy(self.fixture['manifest'])
        target = next(e for e in manifest['exports'] if e['name'] == 'LodIsLooped')
        operation = next(op for i in target['instructions'] for op in i['ops']
                         if op['opcode'] == 'COPY' and op['output']['space'] == 'register'
                         and op['output']['offset'] == '0' and op['output']['size'] == 1)
        operation['inputs'] = [dict(space='const', offset='0', size=1, constant=True)]
        self.assertEqual(compare_case(manifest, self.fixture['originals'], 'LodIsLooped')['result'], 'mismatch')

    def test_wrong_virtual_child_target_rejected(self):
        def wrong_target(*args, **kwargs):
            graph = link(*args, **kwargs)
            for instruction in graph['instructions']:
                for op in instruction['ops']:
                    if op['opcode'] == 'CHECK_DISPATCH':
                        op['expected'] += 1
            return graph
        with patch('effects.link', side_effect=wrong_target), self.assertRaisesRegex(Unsupported, 'dispatch target'):
            compare_case(self.fixture['manifest'], self.fixture['originals'], 'LodStop', 'valid')

    def test_changed_image_rejected(self):
        manifest = copy.deepcopy(self.fixture['manifest'])
        manifest['bundle_sha256'] = 'stale'
        with self.assertRaisesRegex(Unsupported, 'compiled image'):
            compare_case(manifest, self.fixture['originals'], 'LodIsLooped')

    def test_reports_require_all_handle_partitions(self):
        row = dict(result='equivalent', ps2_entry='0036a1f0', partitions=[dict(handle='null', result='equivalent')])
        with self.assertRaisesRegex(ValueError, 'handle partitions'):
            validate_report(dict(functions=[row]), ['0036a1f0'])

