"""Real collision/combat compiler outputs and fail-closed graph/VU controls."""
import copy
import json
from pathlib import Path
import unittest
from combat_cases import compare
from call_graph import link
from verify import Unsupported


class CombatTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixture = json.loads((Path(__file__).parent / 'fixtures/combat.json').read_text())

    def test_five_real_compiled_targets(self):
        for name in ['EndFlip', 'Stagger', 'CollisionClear', 'CollisionReset', 'MatrixIdentity']:
            with self.subTest(name=name):
                result = compare(self.fixture['positive'], name, self.fixture['originals'])
                self.assertEqual(result['result'], 'equivalent')

    def test_original_signed_stagger_bug_detected(self):
        self.assertEqual(compare(self.fixture['before'], 'Stagger', self.fixture['originals'])['result'], 'mismatch')

    def test_real_compiled_wrong_collision_flag_detected(self):
        self.assertEqual(compare(self.fixture['mutant'], 'EndFlip', self.fixture['originals'])['result'], 'mismatch')

    def test_unrestricted_contact_flags_are_not_claimed_equivalent(self):
        result = compare(self.fixture['positive'], 'CollisionReset', self.fixture['originals'], full_contact_flags=True)
        self.assertEqual(result['result'], 'mismatch')
        self.assertGreater(int(result['counterexample']['contactFlags'], 16), 255)

    def test_unknown_vu_operation_is_rejected(self):
        originals = copy.deepcopy(self.fixture['originals'])
        originals['00267850']['instructions'][0]['ops'][0]['inputs'][0]['offset'] = 'dead'
        with self.assertRaisesRegex(Unsupported, 'VU'):
            compare(self.fixture['positive'], 'MatrixIdentity', originals)

    def test_partial_vu_lane_mask_is_rejected(self):
        originals = copy.deepcopy(self.fixture['originals'])
        originals['00267850']['instructions'][0]['ops'][0]['inputs'][1]['offset'] = '7'
        with self.assertRaisesRegex(Unsupported, 'VU'):
            compare(self.fixture['positive'], 'CollisionClear', originals)

    def test_vu_operand_disagreement_is_rejected(self):
        originals = copy.deepcopy(self.fixture['originals'])
        originals['00267850']['instructions'][2]['text'] = 'vmr32.xyzw vf5,vf6'
        with self.assertRaises(Unsupported):
            compare(self.fixture['positive'], 'MatrixIdentity', originals)

    def test_missing_callee_is_rejected(self):
        originals = copy.deepcopy(self.fixture['originals'])
        with self.assertRaisesRegex(Unsupported, 'Unapproved graph transfer'):
            link([originals['0019dba0']], 0x19dba0)

    def test_nested_call_depth_exhaustion_is_rejected(self):
        originals = self.fixture['originals']
        with self.assertRaisesRegex(Unsupported, 'depth limit'):
            link([originals[a] for a in ['0019dba0', '0019e310', '00267850']], 0x19dba0, max_depth=2)

    def test_corrupted_nested_return_is_rejected(self):
        originals = copy.deepcopy(self.fixture['originals'])
        for i in originals['0019dba0']['instructions']:
            for op in i['ops']:
                if op['opcode'] == 'COPY' and op['output']['offset'] == '1f0':
                    op['inputs'][0]['offset'] = 'dead'
        with self.assertRaisesRegex(Unsupported, 'return target'):
            compare(self.fixture['positive'], 'CollisionReset', originals)

    def test_unresolved_virtual_call_still_rejected(self):
        export = copy.deepcopy(self.fixture['originals']['00328300'])
        export['instructions'][0]['ops'] = [dict(opcode='CALLIND', inputs=[dict(space='register', offset='190', size=8, constant=False)])]
        export['operation_count'] = sum(len(i['ops']) for i in export['instructions'])
        with self.assertRaisesRegex(Unsupported, 'indirect'):
            link([export], int(export['entry'], 16))


if __name__ == '__main__':
    unittest.main()
