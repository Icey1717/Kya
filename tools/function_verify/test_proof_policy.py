"""Reference policy cannot approve hardware claims, hidden assumptions or other backends."""
import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from ee_interpreter import MODEL
from proof_policy import BUDGET, approve_reference, load_reference, validate_result


class ProofPolicyTests(unittest.TestCase):
    def oracle(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/pcsx2-cop1-interpreter.json').read_text())
        return dict(fixture['oracle'], executable_sha256='1' * 64, vectors_sha256='2' * 64,
                    source_manifest_sha256='3' * 64)

    def test_conditional_verdict_needs_explicit_policy_and_approved_model(self):
        row = dict(result='conditional-equivalent', reference_models=[MODEL])
        for options in ({}, {'policy': 'pcsx2-reference'}):
            with self.assertRaises(ValueError):
                validate_result(row, **options)
        self.assertIs(validate_result(row, policy='pcsx2-reference', approved_models=[MODEL]), row)

    def test_relabelled_or_nested_reference_assumptions_are_rejected(self):
        for row in (dict(result='equivalent', reference_models=[MODEL]),
                    dict(result='conditional-equivalent'),
                    dict(result='equivalent', partitions=[dict(result='conditional-equivalent', reference_models=[MODEL])])):
            with self.assertRaises(ValueError):
                validate_result(row, policy='pcsx2-reference', approved_models=[MODEL])

    def test_tolerance_requires_named_allow_list_and_exact_budget(self):
        row = dict(result='conditional-equivalent-with-tolerance', reference_models=[MODEL],
                   numeric_policies={'animationTime': BUDGET})
        options = dict(policy='pcsx2-reference', approved_models=[MODEL])
        with self.assertRaises(ValueError):
            validate_result(row, **options)
        validate_result(row, allowed_numeric_fields=['animationTime'], **options)
        row = copy.deepcopy(row)
        row['numeric_policies']['animationTime']['absolute'] = '1'
        with self.assertRaises(ValueError):
            validate_result(row, allowed_numeric_fields=['animationTime'], **options)

    def test_instruction_oracle_cannot_be_promoted_to_another_backend(self):
        manifest = self.oracle()
        self.assertEqual(approve_reference(manifest), (MODEL,))
        for mutation in ({'kind': 'pcsx2-executable-oracle'}, {'hardware_validated': True},
                         {'backend': 'microVU'}, {'models': ['vu:' + manifest['revision']]},
                         {'settings': dict(manifest['settings'], ftz=True)},
                         {'source_manifest': dict(manifest['source_manifest'], **{'pcsx2/R5900.h': '0' * 64})}):
            with self.assertRaises(ValueError):
                approve_reference(dict(manifest, **mutation))

    def test_stale_oracle_artifact_is_rejected_before_execution(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / 'oracle.exe').write_bytes(b'not an executable')
            manifest = dict(self.oracle(), executable='oracle.exe', vectors='vectors.json')
            (path / 'manifest.json').write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError, 'artifact changed'):
                load_reference(path / 'manifest.json')


if __name__ == '__main__':
    unittest.main()
