"""Retained real parser/playback composition and independent fault controls."""
import copy
from pathlib import Path
import unittest
from animation_playback import compare, LAYER, INITIALIZE_ENTRY, MODEL, TIMINGS
from fighter_transitions import load_fixture
from processor_fp import ProcessorFP, ee_register
from transitions import prove
from verify import Unsupported
import z3


class AnimationPlaybackTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest, cls.originals, _ = load_fixture(
            Path(__file__).parent / 'fixtures/animation-playback.json.gz')

    def test_start_and_persistent_same_animation_replay(self):
        for timing in TIMINGS:
            with self.subTest(timing=timing):
                result = compare(self.manifest, self.originals, timing=timing, reference_model=MODEL)
                self.assertEqual(result['result'], 'conditional-equivalent')
                self.assertEqual(result['reference_models'], [MODEL])
                self.assertEqual(result['numeric_policies'], {})
                self.assertFalse(result['full_fighter_contract'])
                self.assertEqual([s['name'] for s in result['steps']],
                                 ['BuildAnimDesc', 'StartStoppedLayer', 'ReplaySameAnimation'])

    def test_reference_policy_must_be_explicit(self):
        for policy in (None, 'ieee', 'hardware'):
            with self.subTest(policy=policy), self.assertRaisesRegex(Unsupported, 'explicit pinned'):
                compare(self.manifest, self.originals, reference_model=policy)

    def test_missing_layout_fails_closed(self):
        manifest = copy.deepcopy(self.manifest)
        del manifest['layouts']['Animation']['playback_currentAnimDesc_state_pKeyDataArray']
        with self.assertRaisesRegex(Unsupported, 'compiler-backed'):
            compare(manifest, self.originals, reference_model=MODEL)

    def test_missing_real_initializer_fails_closed(self):
        originals = copy.deepcopy(self.originals)
        del originals[INITIALIZE_ENTRY]
        with self.assertRaisesRegex(Unsupported, 'complete original/compiled'):
            compare(self.manifest, originals, reference_model=MODEL)

    def test_lost_compiled_playback_store_is_detected(self):
        manifest = copy.deepcopy(self.manifest)
        body = next(e for e in manifest['exports'] if e['coff']['symbol'] == LAYER)
        instruction, operation = [(i, op) for i in body['instructions'] for op in i['ops']
                                  if op['opcode'] == 'STORE' and op['inputs'][2]['size'] == 4][-1]
        instruction['ops'].remove(operation)
        body['operation_count'] -= 1
        self.assertEqual(compare(manifest, self.originals, reference_model=MODEL)['result'], 'mismatch')

    def test_lost_callback_identity_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals['00240530']['instructions'] if i['address'] == '002405c8')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        operation['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        self.assertEqual(compare(self.manifest, originals, reference_model=MODEL)['result'], 'mismatch')

    def test_corrupt_key_duration_is_detected(self):
        originals = copy.deepcopy(self.originals)
        instruction = next(i for i in originals[INITIALIZE_ENTRY]['instructions'] if i['address'] == '00241e54')
        operation = next(op for op in instruction['ops'] if op['opcode'] == 'STORE')
        operation['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        self.assertEqual(compare(self.manifest, originals, reference_model=MODEL)['result'], 'mismatch')

    def test_f20_storage_is_checked_against_real_instruction(self):
        instruction = next(i for i in self.originals[INITIALIZE_ENTRY]['instructions'] if i['address'] == '00241d94')
        self.assertEqual(instruction['bytes'], '03a00146')
        self.assertEqual(int(instruction['ops'][0]['inputs'][0]['offset'], 16), ee_register(20))
        operation = copy.deepcopy(instruction['ops'][0])
        operation['inputs'][0]['offset'] = '1050'
        fp = ProcessorFP('ps2', {'ee_arithmetic_model': MODEL},
                         lambda claim, reason: prove(claim, z3.BoolVal(True), reason))
        with self.assertRaisesRegex(Unsupported, 'source disagrees'):
            fp.evaluate(instruction, operation, [z3.BitVecVal(0, 32), z3.BitVecVal(0x3f800000, 32)], 32)


if __name__ == '__main__':
    unittest.main()
