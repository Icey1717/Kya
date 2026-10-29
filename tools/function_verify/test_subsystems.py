"""Production parser/save/HUD comparisons and integration failure controls."""
import copy
import json
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import check
from subsystems import compare, MAPPINGS
from verify import Unsupported


class SubsystemTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixture = json.loads((Path(__file__).parent / 'fixtures/compiled_subsystems.json').read_text())

    def test_eight_production_functions(self):
        for name in MAPPINGS:
            with self.subTest(name=name):
                self.assertEqual(compare(self.fixture['positive'], name, self.fixture['originals'])['result'], 'equivalent')

    def test_actual_compiled_wrong_cursor_advance(self):
        result = compare(self.fixture['mutant'], 'ReadU32', self.fixture['originals'])
        self.assertEqual(result['result'], 'mismatch')
        example = result['counterexample']
        self.assertEqual(example['windows_return'], example['ps2_return'])
        self.assertNotEqual(example['windows_field'], example['ps2_field'])

    def test_omitted_setter_store_detected(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        pc = next(e for e in manifest['exports'] if e['name'] == 'SetMagic')
        stores = [(i, op) for i in pc['instructions'] for op in i['ops'] if op['opcode'] == 'STORE']
        instruction, op = stores[-1]
        instruction['ops'].remove(op)
        pc['operation_count'] -= 1
        self.assertEqual(compare(manifest, 'SetMagic', self.fixture['originals'])['result'], 'mismatch')

    def test_wrong_hud_field_mapping_detected(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        manifest['layout']['priority'] += 4
        self.assertEqual(compare(manifest, 'GetPriority', self.fixture['originals'])['result'], 'mismatch')

    def test_unmapped_cursor_read_fails_closed(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        manifest['layout']['cursor'] += 8
        with self.assertRaises(Unsupported):
            compare(manifest, 'ReadU32', self.fixture['originals'])


class IntegrationTests(unittest.TestCase):
    def test_incomplete_coverage_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'expected function set'):
            check.validate_report({'functions': [{'ps2_entry': '1', 'result': 'equivalent'}]}, ['1', '2'])

    def test_inconclusive_is_not_a_pass(self):
        with self.assertRaisesRegex(ValueError, 'incomplete proof'):
            check.validate_report({'functions': [{'ps2_entry': '1', 'result': 'inconclusive'}]}, ['1'])

    def test_concurrent_runs_cannot_mix_artifacts(self):
        with tempfile.TemporaryDirectory() as directory:
            with check.run_lock(Path(directory)):
                with self.assertRaises(check.RunBusy):
                    with check.run_lock(Path(directory)):
                        self.fail('Concurrent lock acquired')
            with check.run_lock(Path(directory)):
                pass

    def test_offline_checks_can_run_during_live_verification(self):
        with tempfile.TemporaryDirectory() as directory:
            with check.run_lock(Path(directory)):
                with check.run_lock(Path(directory), 'offline.lock'):
                    pass

    def test_zero_exit_without_fresh_reports_cannot_reuse_old_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            for _, _, _, folder, expected in check.SUITES:
                path = output / folder / 'report.json'
                path.parent.mkdir()
                path.write_text(json.dumps({'functions': [{'ps2_entry': a, 'result': 'equivalent'} for a in expected]}))
            args = SimpleNamespace(offline=False, ninja='ninja', port=18437, ghidra=None, java=None)
            with patch.object(check.subprocess, 'run', return_value=SimpleNamespace(returncode=0)), patch('builtins.print'):
                self.assertEqual(check.run_checks(args, output), 1)
            status = json.loads((output / 'check-status.json').read_text())
            self.assertEqual(status['result'], 'failed')
            self.assertEqual(status['equivalent_functions'], 0)


if __name__ == '__main__':
    unittest.main()
