"""Domain separation, cache invalidation and fail-closed contract accounting."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
from fighter_cases import FighterCase
from fighter_domains import ProofDomain, full_contract, job_document, read_cached


class FighterDomainTests(unittest.TestCase):
    def setUp(self):
        self.case = FighterCase('SetState', 6, 9, 'configured', 'zero', 'a' * 64)

    def test_job_identity_covers_domain_policy_inputs_and_limits(self):
        baseline = job_document(self.case, ProofDomain(), 'a' * 64)
        alternatives = [job_document(self.case, ProofDomain('controller-only'), 'a' * 64),
                        job_document(self.case, ProofDomain(behaviour='fighter-slot-0'), 'a' * 64),
                        job_document(self.case, ProofDomain(), 'b' * 64),
                        job_document(self.case, ProofDomain(), 'a' * 64, policy='pcsx2-reference'),
                        job_document(self.case, ProofDomain(), 'a' * 64, timeout=1200)]
        self.assertEqual(len({j['job_id'] for j in [baseline] + alternatives}), 6)

    def test_legacy_flag_or_restrictions_cannot_discharge_contract(self):
        self.assertFalse(full_contract(dict(result='equivalent', full_fighter_contract=True)))
        self.assertFalse(full_contract(dict(result='equivalent', full_fighter_contract=True,
                                           restrictions=[], domain_profile=ProofDomain('populated').document())))

    def test_resume_rejects_changed_inputs_and_failed_results(self):
        job = job_document(self.case, ProofDomain(), 'a' * 64)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'job.json'
            row = dict(job, result='equivalent', restrictions=ProofDomain().restrictions)
            path.write_text(json.dumps(row))
            self.assertEqual(read_cached(path, job), row)
            changed = dict(job, inputs_sha256='b' * 64)
            self.assertIsNone(read_cached(path, changed))
            for result in ('unsupported', 'inconclusive', 'mismatch'):
                path.write_text(json.dumps(dict(row, result=result)))
                self.assertIsNone(read_cached(path, job))

    def test_reference_approval_does_not_invent_actual_dependency(self):
        job = job_document(self.case, ProofDomain(), 'a' * 64, policy='pcsx2-reference',
                           reference_models=['approved'])
        self.assertNotIn('reference_models', job)

    def test_cli_separates_domains_resumes_and_inventory_preserves_proofs(self):
        from fighter_transitions import main
        fixture = json.loads((Path(__file__).parent / 'fixtures/fighter-configuration.json').read_text())
        raw = bytes.fromhex(fixture['original']['hex'])
        with tempfile.TemporaryDirectory() as directory:
            args = ['fighter_transitions.py', '--artifacts', '.', '--output-root', directory,
                    '--state-pair', '9:9', '--function', 'SetStandAnim', '--idle-loops', 'zero',
                    '--dormant-actor', '--resources', 'null', '--resources', 'controller-only']
            with patch('fighter_transitions.load_artifacts', return_value=({}, {}, raw)), patch('builtins.print'), \
                 patch('fighter_transitions.run_case_worker', return_value=dict(result='equivalent', reference_models=[])) as worker:
                with patch('sys.argv', args):
                    self.assertEqual(main(), 1)
                self.assertEqual(worker.call_count, 4)
                folder = Path(directory) / 'fighter-transitions'
                report = json.loads((folder / 'report.json').read_text())
                self.assertEqual(report['jobs'], 4)
                self.assertEqual(set(report['domains']), {'null', 'controller-only'})
                files = {p.name: p.read_text() for p in (folder / 'jobs').glob('*.json')}
                self.assertEqual(len(files), 4)
                worker.reset_mock()
                with patch('sys.argv', args + ['--resume']):
                    self.assertEqual(main(), 1)
                worker.assert_not_called()
                inventory_args = [arg for arg in args if arg != '--dormant-actor'] + ['--inventory-only', '--resume']
                with patch('sys.argv', inventory_args):
                    self.assertEqual(main(), 0)
                worker.assert_not_called()
                with patch('sys.argv', inventory_args + ['--behaviour-profile', 'fighter-general-two']):
                    self.assertEqual(main(), 0)
                worker.assert_not_called()
                self.assertEqual({p.name: p.read_text() for p in (folder / 'jobs').glob('*.json')}, files)

    def test_cache_rejects_non_object_json(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'job.json'
            for value in ([], None, 'text'):
                path.write_text(json.dumps(value))
                self.assertIsNone(read_cached(path, job_document(self.case, ProofDomain(), 'a' * 64)))


if __name__ == '__main__':
    unittest.main()
