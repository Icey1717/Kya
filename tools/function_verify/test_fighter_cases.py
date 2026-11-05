"""Exhaustive inventory accounting and fail-closed case/report identities."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import subprocess
from fighter_cases import cases, configuration, parse_pair, parse_shard
from fighter_transitions import guard_inventory, summarize, run_case_worker


class FighterCaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        fixture = json.loads((Path(__file__).parent / 'fixtures/fighter-configuration.json').read_text())
        cls.raw = bytes.fromhex(fixture['original']['hex'])

    def test_all_pairs_and_partitions_are_present_once(self):
        selected = list(cases(self.raw))
        pairs = {(case.old_state, case.new_state) for case in selected if case.function == 'SetState'}
        self.assertEqual(len(pairs), 11664)
        self.assertEqual(sum(old != new for old, new in pairs), 11556)
        self.assertEqual(len(selected), 70416)
        self.assertEqual(len({case.key for case in selected}), len(selected))

    def test_shards_are_disjoint_and_exhaustive(self):
        whole = {case.key for case in cases(self.raw, pairs=[(6, 9), (9, 9)])}
        shards = [{case.key for case in cases(self.raw, pairs=[(6, 9), (9, 9)], shard=(i, 3))} for i in range(3)]
        self.assertEqual(set.union(*shards), whole)
        self.assertFalse(any(shards[i] & shards[j] for i in range(3) for j in range(i)))

    def test_configuration_changes_invalidate_every_case_identity(self):
        raw = bytearray(self.raw)
        raw[0] ^= 1
        before = {case.key for case in cases(self.raw, pairs=[(6, 9)])}
        after = {case.key for case in cases(bytes(raw), pairs=[(6, 9)])}
        self.assertFalse(before & after)

    def test_state_family_uses_actual_configuration_flags(self):
        table = configuration(self.raw)
        selected = list(cases(self.raw, pairs=[(6, 9)], family=table[3]['flags'] & 0xff800))
        self.assertEqual(len(selected), 6)
        with self.assertRaises(ValueError):
            list(cases(self.raw, family=0xdeadbeef))

    def test_incomplete_and_partial_domains_never_count_as_complete(self):
        selected = list(cases(self.raw, pairs=[(6, 9)]))
        rows = [dict(case.document(), result='equivalent', full_fighter_contract=False) for case in selected]
        report = summarize(selected, rows)
        self.assertEqual(report['result'], 'incomplete')
        self.assertEqual(report['verdict_counts'], {'incomplete': 6})

    def test_missing_duplicate_and_stale_cases_fail(self):
        selected = list(cases(self.raw, pairs=[(6, 9)]))
        rows = [dict(case.document(), result='equivalent', full_fighter_contract=True) for case in selected]
        # A legacy flag is not evidence that runtime/animation obligations hold.
        self.assertEqual(summarize(selected, rows)['result'], 'incomplete')
        for broken in (rows[:-1], rows + [rows[0]]):
            with self.assertRaises(ValueError):
                summarize(selected, broken)
        stale = copy.deepcopy(rows)
        stale[0]['old_state'] = 7
        with self.assertRaises(ValueError):
            summarize(selected, stale)

    def test_invalid_requests_do_not_silently_produce_empty_success(self):
        for value in ('-1:6', '6:114', '6', '6:9:10'):
            with self.assertRaises(ValueError):
                parse_pair(value)
        for value in ('1/1', '-1/2', '0/0', 'abc'):
            with self.assertRaises(ValueError):
                parse_shard(value)
        self.assertEqual(parse_pair('0x6:9'), (6, 9))
        with self.assertRaises(ValueError):
            configuration(self.raw[:-1])

    def test_guard_inventory_excludes_comments_and_strings(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'test.cpp'
            source.write_text('void CBehaviourFighter::TermState(int oldState)\n{\n'
                              '// IMPLEMENTATION_GUARD();\nconst char* s = "IMPLEMENTATION_GUARD()";\n'
                              'switch(oldState) { case FIGHTER_TEST: IMPLEMENTATION_GUARD(); }\n}\n')
            inventory = guard_inventory(source)
            self.assertEqual(len(inventory['guards']), 1)
            self.assertEqual(inventory['guards'][0]['function'], 'CBehaviourFighter::TermState')
            self.assertEqual(inventory['guards'][0]['case'], 'FIGHTER_TEST')

    def test_worker_timeout_and_invalid_output_do_not_pass(self):
        case = next(cases(self.raw, pairs=[(6, 9)]))
        with patch('fighter_transitions.subprocess.run', side_effect=subprocess.TimeoutExpired('proof', 1)):
            result = run_case_worker('.', case, 'progress.jsonl', timeout=1)
        self.assertEqual(result['result'], 'inconclusive')
        self.assertFalse(result['full_fighter_contract'])
        for value in ('not json', '{"result":"passed"}', '[]'):
            with self.subTest(value=value), patch('fighter_transitions.subprocess.run',
                    return_value=subprocess.CompletedProcess('proof', 0, value, '')):
                self.assertEqual(run_case_worker('.', case, 'progress.jsonl')['result'], 'unsupported')

    def test_worker_preserves_conditional_status_for_parent_policy(self):
        case = next(cases(self.raw, pairs=[(6, 9)]))
        from ee_interpreter import MODEL
        result = dict(result='conditional-equivalent', reference_models=[MODEL], full_fighter_contract=False)
        with patch('fighter_transitions.subprocess.run',
                   return_value=subprocess.CompletedProcess('proof', 0, json.dumps(result), '')):
            self.assertEqual(run_case_worker('.', case, 'progress.jsonl'), result)


if __name__ == '__main__':
    unittest.main()
