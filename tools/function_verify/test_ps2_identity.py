"""Identity selection controls, independent of the equivalence solver."""
import copy
from collections import Counter
import json
from pathlib import Path
import tempfile
import unittest

from ps2_identity import (Manifest, digest, source_identity, resolve, validate_export,
                          listing, reconcile, load_manifest, DEFAULT_COHORT)


class IdentityTests(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        (self.root / 'evidence.json').write_text('{}')
        self.fn = dict(id='source-id', path='src/test.cpp', name='A::F', source='int A::F() { return 0; }')
        self.original = dict(entry='00123450', name='OldF', namespace='Global', signature='int OldF(A * this)',
                             body='[[00123450, 00123457] ]')
        self.document = dict(schema=1, original_elf_sha256='elf-hash', records=[dict(
            id=self.fn['id'], source=source_identity(self.fn), disposition='mapped',
            ps2=self.original, obligation='Identity reviewed; machine-code proof still required', evidence=[dict(
                kind='live-review', path='evidence.json', sha256=digest(b'{}'), summary='Reviewed live body')])])

    def manifest(self):
        return Manifest(self.document, self.root)

    def resolve(self, fn=None, candidates=(), elf='elf-hash', live=None):
        return resolve(fn or self.fn, candidates, self.manifest(), elf, lambda _: live or self.original)

    def export(self):
        return dict(entry=self.original['entry'], name='OldF', complete=True, executable_sha256='elf-hash')

    def test_reviewed_rename_and_position_change(self):
        fn = dict(self.fn, id='moved-id')
        selected, diagnostic = self.resolve(fn)
        self.assertEqual(selected, self.original)
        self.assertEqual(diagnostic['status'], 'reviewed')
        validate_export(fn, self.export(), diagnostic, self.manifest(), 'elf-hash', lambda _: self.original)

    def test_stale_source_declaration_elf_and_metadata_fail_closed(self):
        for fn, elf, live in [(dict(self.fn, source='int A::F() { return 1; }'), 'elf-hash', None),
                              (dict(self.fn, name='A::G'), 'elf-hash', None),
                              (self.fn, None, None), (self.fn, 'wrong-elf', None),
                              (self.fn, 'elf-hash', dict(self.original, entry='00123451')),
                              (self.fn, 'elf-hash', dict(self.original, signature='void OldF()'))]:
            with self.subTest(fn=fn, elf=elf, live=live):
                selected, diagnostic = self.resolve(fn, elf=elf, live=live)
                self.assertIsNone(selected)
                self.assertEqual(diagnostic['status'], 'stale')

    def test_duplicate_and_incomplete_reviews_rejected(self):
        self.document['records'].append(copy.deepcopy(self.document['records'][0]))
        with self.assertRaisesRegex(ValueError, 'Duplicate'):
            self.manifest()
        self.document['records'].pop()
        del self.document['records'][0]['ps2']['signature']
        with self.assertRaisesRegex(ValueError, 'Incomplete mapped'):
            self.manifest()

    def test_changed_evidence_rejected(self):
        (self.root / 'evidence.json').write_text('changed')
        with self.assertRaisesRegex(ValueError, 'Changed identity evidence'):
            self.manifest()

    def test_missing_ambiguous_exact_and_no_short_name_guessing(self):
        self.document['records'] = []
        selected, diagnostic = self.resolve(candidates=[self.original])
        self.assertIsNone(selected)
        self.assertEqual(diagnostic['status'], 'missing')
        exact = dict(self.original, name='F', namespace='A')
        self.assertEqual(self.resolve(candidates=[exact])[0], exact)
        selected, diagnostic = self.resolve(candidates=[exact, dict(exact, entry='00123460')])
        self.assertIsNone(selected)
        self.assertEqual(diagnostic['status'], 'ambiguous')
        self.assertEqual(len(diagnostic['candidates']), 2)

    def test_non_standalone_stays_unsupported_even_with_name_candidate(self):
        row = self.document['records'][0]
        row['disposition'] = 'non-standalone'
        row['evidence'][0]['kind'] = 'source-review'
        selected, diagnostic = self.resolve(candidates=[dict(self.original, name='F', namespace='A')])
        self.assertIsNone(selected)
        self.assertEqual(diagnostic['status'], 'non-standalone')

    def test_truncated_listing_rejected(self):
        with self.assertRaisesRegex(ValueError, 'Truncated'):
            listing(dict(functions=[self.original], limit=1))
        self.assertEqual(listing(dict(functions=[self.original], limit=2)), [self.original])
        with self.assertRaisesRegex(ValueError, 'Truncated'):
            listing(dict(functions=[self.original] * 1000))

    def test_export_rejects_wrong_entry_name_elf_incomplete_and_changed_review(self):
        _, diagnostic = self.resolve()
        for changes in [dict(entry='00123460'), dict(name='A::F'), dict(executable_sha256='wrong'), dict(complete=False)]:
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                validate_export(self.fn, dict(self.export(), **changes), diagnostic,
                                self.manifest(), 'elf-hash', lambda _: self.original)
        with self.assertRaisesRegex(ValueError, 'Missing reviewed export diagnostic'):
            validate_export(self.fn, self.export(), {}, self.manifest(), 'elf-hash', lambda _: self.original)
        self.document['records'][0]['disposition'] = 'unresolved'
        with self.assertRaisesRegex(ValueError, 'Missing reviewed'):
            validate_export(self.fn, self.export(), diagnostic, self.manifest(), 'elf-hash', lambda _: self.original)

    def test_unreviewed_address_alias_and_thunk_are_not_selected(self):
        self.document['records'] = []
        fn = dict(self.fn, name='A::FUN_00123450')
        self.assertIsNone(self.resolve(fn, [dict(self.original, name='FUN_00123450')])[0])
        exact = dict(self.original, name='F', namespace='A')
        self.assertIsNone(self.resolve(candidates=[exact, dict(exact, entry='00123460')])[0])

    def test_fixed_cohort_not_selected_and_next_blocker(self):
        cohort = dict(functions=[dict(id='old', path='src/test.cpp', name='A::F', source_sha256='hash'),
                                 dict(id='absent', path='absent.cpp', name='Absent', source_sha256='hash')])
        rows = [dict(id='moved', path='src/test.cpp', name='A::F', source_sha256='hash',
                     ps2_identity=dict(status='reviewed'), result='unsupported', reason='Unapproved relocation')]
        report = reconcile(cohort, rows)
        self.assertEqual(report['total'], 2)
        self.assertEqual(report['counts'], {'reviewed': 1, 'not-selected-or-changed': 1})
        self.assertEqual(report['functions'][0]['reason'], 'Unapproved relocation')

    def test_checked_in_cohort_is_complete_and_manifest_is_valid(self):
        cohort = json.loads(DEFAULT_COHORT.read_text())
        manifest = load_manifest()
        self.assertEqual(len(cohort['functions']), 426)
        self.assertEqual(len({f['id'] for f in cohort['functions']}), 426)
        self.assertEqual(set(manifest.records), {f['id'] for f in cohort['functions']})
        self.assertEqual(dict(Counter(row['initial_status'] for row in cohort['functions'])),
                         {'missing': 358, 'ambiguous': 68})


if __name__ == '__main__':
    unittest.main()
