"""Real one-layer reset closure and missing-write negative controls."""
import copy
import gzip
import json
from pathlib import Path
import unittest
from fighter_transitions import base_transition, load_fixture, validate_artifacts
from verify import Unsupported


class FighterAnimationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest, cls.originals, _ = load_fixture(
            Path(__file__).parent / 'fixtures/fighter-animation-reset.json.gz')

    def test_configured_idle_to_run_resets_real_layer(self):
        result = base_transition(self.manifest, self.originals, resources='populated-reset')
        self.assertEqual(result['result'], 'equivalent')
        self.assertEqual(result['reference_models'], [])
        self.assertEqual(result['domain_profile']['resources'], 'populated-reset')
        self.assertFalse(result['full_fighter_contract'])

    def test_missing_layer_reset_writes_are_detected(self):
        manifest = copy.deepcopy(self.manifest)
        body = next(e for e in manifest['exports'] if e['coff']['symbol'] == '?Reset@edAnmLayer@@QEAAXXZ')
        removed = 0
        for instruction in body['instructions']:
            # Keep ABI spills/restores: mutate only the 32-bit layer fields.
            stores = [op for op in instruction['ops'] if op['opcode'] == 'STORE' and op['inputs'][2]['size'] == 4]
            for op in stores:
                instruction['ops'].remove(op)
                removed += 1
        self.assertGreater(removed, 0)
        body['operation_count'] -= removed
        result = base_transition(manifest, self.originals, resources='populated-reset')
        self.assertEqual(result['result'], 'mismatch')

    def test_dormant_cycle_carries_actual_animation_state(self):
        result = base_transition(self.manifest, self.originals, sequence_states=(6, 9, 6))
        self.assertEqual(result['result'], 'equivalent')
        self.assertEqual(len(result['steps']), 2)
        self.assertFalse(result['full_fighter_contract'])

    def test_stand_animation_dispatch_and_nonidle_path(self):
        for state in (6, 9):
            result = base_transition(self.manifest, self.originals, old_states=(state,), new_states=(state,),
                                     function='SetStandAnim', animation='stand-changed')
            self.assertEqual(result['result'], 'equivalent')
            self.assertFalse(result['full_fighter_contract'])

    def test_wrong_literal_address_is_rejected_against_actual_image(self):
        document = json.loads(gzip.decompress((Path(__file__).parent / 'fixtures/fighter-animation-reset.json.gz').read_bytes()))
        manifest = copy.deepcopy(self.manifest)
        manifest['addresses']['__real@3f800000'] += 1
        with self.assertRaisesRegex(Unsupported, 'literal'):
            validate_artifacts(manifest, self.originals, bytes.fromhex(document['blob_hex']))


if __name__ == '__main__':
    unittest.main()
