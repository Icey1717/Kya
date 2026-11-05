"""Real collision/combat compiler outputs and fail-closed graph/VU controls."""
import copy
import json
from pathlib import Path
import unittest
from unittest.mock import patch
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
                result = compare(self.fixture['positive'], name, self.fixture['originals'], full_contact_flags=False)
                self.assertEqual(result['result'], 'equivalent')

    def test_original_signed_stagger_bug_detected(self):
        self.assertEqual(compare(self.fixture['before'], 'Stagger', self.fixture['originals'])['result'], 'mismatch')

    def test_real_compiled_wrong_collision_flag_detected(self):
        self.assertEqual(compare(self.fixture['mutant'], 'EndFlip', self.fixture['originals'])['result'], 'mismatch')

    def test_unrestricted_contact_flags_are_not_claimed_equivalent(self):
        result = compare(self.fixture['positive'], 'CollisionReset', self.fixture['originals'], full_contact_flags=True)
        self.assertEqual(result['result'], 'mismatch')
        self.assertGreater(int(result['counterexample']['contactFlags'], 16), 255)

    def test_fixed_reset_preserves_arbitrary_upper_bytes(self):
        fixed = json.loads((Path(__file__).parent / 'fixtures/collision-reset-fixed.json').read_text())
        for name in ('CollisionReset', 'CollisionClear'):
            self.assertEqual(compare(fixed['positive'], name, fixed['originals'])['result'], 'equivalent')

    def check_dispatch(self, target):
        originals = copy.deepcopy(self.fixture['originals'])
        reset = originals['0019dba0']
        call = next(i for i in reset['instructions'] if any(o['opcode'] == 'CALL' for o in i['ops']))
        op = next(o for o in call['ops'] if o['opcode'] == 'CALL')
        op.update(opcode='CALLIND', inputs=[dict(space='const', offset=hex(target)[2:], size=4, constant=True)])
        def checked_link(exports, root):
            return link(exports, root, indirect_targets={int(call['address'], 16): 0x19e310}
                        if exports[0]['language'].startswith('r5900') else None)
        with patch('call_graph.link', side_effect=checked_link):
            return compare(self.fixture['positive'], 'CollisionReset', originals, full_contact_flags=False)

    def test_checked_indirect_call_executes_complete_callee(self):
        self.assertEqual(self.check_dispatch(0x19e310)['result'], 'equivalent')

    def test_wrong_dispatch_is_not_assumed_correct(self):
        with self.assertRaisesRegex(Unsupported, 'dispatch target not proven'):
            self.check_dispatch(0x19e314)

    def test_unused_dispatch_mapping_rejected(self):
        export = self.fixture['originals']['00328300']
        with self.assertRaisesRegex(Unsupported, 'Unused'):
            link([export], int(export['entry'], 16), indirect_targets={0x1234: int(export['entry'], 16)})

    def test_windows_dispatch_target_checked(self):
        for wrong in (False, True):
            manifest = copy.deepcopy(self.fixture['positive'])
            reset = next(e for e in manifest['exports'] if e['name'] == 'CollisionReset')
            call = next(i for i in reset['instructions'] if any(o['opcode'] == 'CALL' for o in i['ops']))
            op = next(o for o in call['ops'] if o['opcode'] == 'CALL')
            target = int(op['inputs'][0]['offset'], 16)
            op.update(opcode='CALLIND', inputs=[dict(space='const', offset=hex(target + int(wrong))[2:], size=8, constant=True)])
            def checked_link(exports, root):
                return link(exports, root, indirect_targets={int(call['address'], 16): target}
                            if exports[0]['language'].startswith('x86') else None)
            with self.subTest(wrong=wrong), patch('call_graph.link', side_effect=checked_link):
                if wrong:
                    with self.assertRaisesRegex(Unsupported, 'dispatch target not proven'):
                        compare(manifest, 'CollisionReset', self.fixture['originals'], full_contact_flags=False)
                else:
                    self.assertEqual(compare(manifest, 'CollisionReset', self.fixture['originals'], full_contact_flags=False)['result'], 'equivalent')

    def test_raw_dispatch_check_injection_rejected(self):
        export = copy.deepcopy(self.fixture['originals']['00328300'])
        export['instructions'][0]['ops'][0]['opcode'] = 'CHECK_DISPATCH'
        with self.assertRaisesRegex(Unsupported, 'Synthetic'):
            link([export], int(export['entry'], 16))

    def test_transition_targets_are_required_and_blocked_reports_fail(self):
        from check import SUITES, validate_report
        suite = next(s for s in SUITES if s[0] == 'combat-transitions')
        for result in ('unsupported', 'inconclusive', 'mismatch'):
            with self.subTest(result=result), self.assertRaisesRegex(ValueError, 'incomplete'):
                validate_report({'functions': [dict(ps2_entry=a, result=result) for a in suite[4]]}, suite[4])

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
