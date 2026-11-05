"""Retained morph/callback compositions and faults in their actual bodies."""
import copy
from pathlib import Path
import unittest
from animation_playback import compare, MODEL
from animation_morphing import SETTINGS, INIT_ENTRY, START_ENTRY
from animation_callbacks import ENTRIES, CALLBACK
from fighter_transitions import load_fixture, base_transition
from verify import Unsupported


class MorphingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest, cls.originals, _ = load_fixture(
            Path(__file__).parent / 'fixtures/animation-morphing-callback.json.gz')

    def test_queue_start_repeat_and_interruption_partitions(self):
        for setting in SETTINGS:
            with self.subTest(setting=setting):
                result = compare(self.manifest, self.originals, morph=setting, reference_model=MODEL)
                self.assertEqual(result['result'], 'conditional-equivalent', result)
                self.assertEqual(result['steps'][-1]['name'], 'InterruptMorph')
                self.assertEqual(len(result['steps']), 9)
                self.assertEqual(result['reference_models'], [MODEL])
                self.assertEqual(result['numeric_policies'], {})
                self.assertFalse(result['full_fighter_contract'])

    def test_real_callback_mutates_weights_before_blending(self):
        result = compare(self.manifest, self.originals, blend='two', callback=True, reference_model=MODEL)
        self.assertEqual(result['result'], 'conditional-equivalent', result)
        self.assertTrue(set(ENTRIES) <= set(result['original_entries']))
        self.assertFalse(result['full_fighter_contract'])

    def test_lost_morph_start_state_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[START_ENTRY]['instructions'] if i['address'] == '002411ac')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        operation['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        result = compare(self.manifest, originals, morph='halfway', reference_model=MODEL)
        self.assertEqual(result['result'], 'mismatch')
        self.assertEqual(result['stage'], 'StartMorph')

    def test_lost_fighter_callback_weight_store_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[ENTRIES[1]]['instructions'] if i['address'] == '0031c5f8')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        operation['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        result = compare(self.manifest, originals, blend='two', callback=True, reference_model=MODEL)
        self.assertEqual(result['result'], 'mismatch')

    def test_wrong_callback_target_is_rejected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals['00241b40']['instructions']
                           if any(op['opcode'] == 'CALLIND' for op in i['ops']))
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'CALLIND')
        operation['inputs'][0] = dict(space='const', offset='dead', size=operation['inputs'][0]['size'], constant=True)
        with self.assertRaisesRegex(Unsupported, 'target|dispatch'):
            compare(self.manifest, originals, blend='two', callback=True, reference_model=MODEL)

    def test_missing_actual_morph_and_callback_bodies_fail_closed(self):
        for entry, case in ((INIT_ENTRY, dict(morph='keep')), (START_ENTRY, dict(morph='keep')),
                            (ENTRIES[1], dict(callback=True, blend='two'))):
            originals = dict(self.originals)
            del originals[entry]
            with self.subTest(entry=entry), self.assertRaisesRegex(Unsupported, 'complete original/compiled'):
                compare(self.manifest, originals, reference_model=MODEL, **case)

    def test_missing_dispatch_and_invalid_domains_fail_closed(self):
        manifest = copy.deepcopy(self.manifest)
        manifest['dispatch']['mappings'] = []
        with self.assertRaisesRegex(Unsupported, 'dispatch evidence'):
            compare(manifest, self.originals, blend='two', callback=True, reference_model=MODEL)
        for case in (dict(morph='unknown'), dict(morph='keep', key_index=1), dict(callback=True),
                     dict(callback=True, blend='two', morph='keep')):
            with self.subTest(case=case), self.assertRaises(ValueError):
                compare(self.manifest, self.originals, reference_model=MODEL, **case)

    def test_bounded_animation_cannot_discharge_full_fighter_domain(self):
        with self.assertRaisesRegex(Unsupported, 'not established'):
            base_transition(self.manifest, self.originals, resources='populated')

    def test_actor_controller_parser_and_diagnostic_composition(self):
        result = compare(self.manifest, self.originals, actor_entry=True, reference_model=MODEL)
        self.assertEqual(result['result'], 'conditional-equivalent', result)
        self.assertEqual([s['name'] for s in result['steps']], ['ActorStartPlayback', 'ActorRepeatPlayback'])
        self.assertFalse(result['full_fighter_contract'])

    def test_lost_windows_diagnostic_counter_is_detected(self):
        manifest = copy.deepcopy(self.manifest)
        address = manifest['addresses'][manifest['globals']['lookup_count']['symbol']]
        body = next(e for e in manifest['exports'] if 'DebugValidateAnimTrackData' in e['coff']['symbol'])
        operations = [op for i in body['instructions'] for op in i['ops']
                      if op.get('output', {}).get('space') == 'ram' and int(op['output']['offset'], 16) == address]
        self.assertEqual(len(operations), 1)
        operations[0]['inputs'][0] = dict(space='const', offset='0', size=4, constant=True)
        result = compare(manifest, self.originals, actor_entry=True, reference_model=MODEL)
        self.assertEqual(result['result'], 'mismatch', result)

    def test_lost_actor_previous_animation_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals['00103db0']['instructions'] if i['address'] == '00103e1c')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        operation['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        result = compare(self.manifest, originals, actor_entry=True, reference_model=MODEL)
        self.assertEqual(result['result'], 'mismatch', result)

    def test_actor_playback_missing_validator_is_not_summarized(self):
        manifest = copy.deepcopy(self.manifest)
        manifest['exports'] = [e for e in manifest['exports'] if 'DebugValidateAnimTrackData' not in e['coff']['symbol']]
        with self.assertRaisesRegex(Unsupported, 'graph transfer|frontier'):
            compare(manifest, self.originals, actor_entry=True, reference_model=MODEL)


if __name__ == '__main__':
    unittest.main()
