"""Batch survey controls using retained real compiled machine-code exports."""
import copy
import json
import hashlib
from pathlib import Path
import struct
import unittest
import tempfile
from unittest.mock import patch

from small_functions import arguments, compare, signature, verify_original_bytes, exit_status, outcomes, emitted_candidates, compare_prepared, main
from ps2_identity import DEFAULT_MANIFEST, source_identity
from compare_extended import store, word, prove
from verify import Unsupported
import z3


class SmallFunctionTests(unittest.TestCase):
    def test_overload_without_active_compiler_definition_cannot_fall_back_to_name_prefix(self):
        fn = dict(name='F', source='int F() { return 1; }', body='{ return 1; }', parseError=False)
        for status in ('no-active-definition', 'identity-review', 'not-examined', 'ambiguous'):
            with self.subTest(status=status), self.assertRaisesRegex(Unsupported, 'Compiler identity unavailable'):
                signature(dict(fn, discovery_result=status))
            with self.subTest(status=status), self.assertRaisesRegex(Unsupported, 'Compiler identity unavailable'):
                emitted_candidates(dict(fn, discovery_result=status), '?F@@', {})

    def test_prepared_replay_revalidates_reviewed_identity_even_when_relocation_blocked(self):
        from verify import ROOT
        with tempfile.TemporaryDirectory(dir=ROOT / 'out') as folder:
            output = Path(folder)
            source = output / 'F.cpp'
            source.write_text('int F() { return 1; }')
            fn = dict(id='replay-test', name='F', path=source.relative_to(ROOT).as_posix(),
                      source=source.read_text(), line=1, lines=1)
            live = dict(entry='00123450', name='RenamedF', namespace='Global', signature='int RenamedF()',
                        body='[[00123450, 00123457] ]')
            evidence = output / 'evidence.json'
            evidence.write_text('{}')
            manifest = output / 'reviews.json'
            review = dict(
                id=fn['id'], source=source_identity(fn), disposition='mapped', ps2=live, obligation='Identity only',
                evidence=[dict(kind='live-review', path=evidence.relative_to(ROOT).as_posix(),
                               sha256=hashlib.sha256(evidence.read_bytes()).hexdigest(), summary='Reviewed fixture')])
            manifest.write_text(json.dumps(dict(schema=1, original_elf_sha256='elf', records=[review])))
            row = dict(id=fn['id'], name=fn['name'], path=fn['path'], line=1, lines=1,
                       source_sha256=source_identity(fn)['source_sha256'], ps2_entry=live['entry'],
                       ps2_identity=dict(status='reviewed', review_id=fn['id'], selected=live),
                       result='unsupported', reason='Unapproved relocation')
            build = dict(build_directory=str(output))
            document = dict(functions=[row], counts={'unsupported': 1}, build=build, line_limit=20,
                            ps2_executable_sha256='elf', identity_manifest_sha256=hashlib.sha256(manifest.read_bytes()).hexdigest())
            (output / 'inventory.json').write_text(json.dumps(dict(functions=[fn])))
            (output / 'layouts.json').write_text('{}')
            (output / 'report.json').write_text(json.dumps(document))
            with patch('small_functions.identity', return_value=build), \
                    patch('small_functions.request', return_value=live):
                self.assertEqual(compare_prepared(output, identity_manifest=manifest), 2)
            replayed = json.loads((output / 'report.json').read_text())
            self.assertEqual(replayed['functions'][0]['result'], 'unsupported')
            with patch('small_functions.identity', return_value=build), \
                    patch('small_functions.request', return_value=dict(live, body='[[00123450, 0012345f] ]')):
                with self.assertRaisesRegex(Unsupported, 'Reviewed live PS2 identity changed'):
                    compare_prepared(output, identity_manifest=manifest)

    def test_identity_only_cli_uses_complete_hash_export_and_never_claims_equivalence(self):
        from verify import ROOT
        with tempfile.TemporaryDirectory(dir=ROOT / 'out') as folder:
            output = Path(folder)
            source = output / 'F.cpp'
            source.write_text('int F() { return 1; }')
            fn = dict(id='cli-test', path=source.relative_to(ROOT).as_posix(), name='F',
                      source=source.read_text(), body='{ return 1; }', line=1, lines=1, parseError=False)
            (output / 'inventory.json').write_text(json.dumps(dict(functions=[fn])))
            candidate = dict(entry='00123450', name='F', namespace='Global', signature='int F()',
                             body='[[00123450, 00123457] ]')
            def request(command, port, **kwargs):
                if command == 'status':
                    return {}
                if command == 'export_pcode':
                    self.assertEqual(kwargs['limit'], 65536)
                    return dict(executable_sha256='hash')
                self.assertEqual(command, 'list_functions')
                return dict(functions=[candidate], limit=1000)
            with patch('sys.argv', ['small_functions.py', '--output', str(output), '--reuse-inventory',
                                    '--no-clang-fallback', '--identities-only']), \
                    patch('small_functions.request', side_effect=request), \
                    patch('small_functions.identity', return_value={}):
                self.assertEqual(main(), 2)
            report = json.loads((output / 'report.json').read_text())
            self.assertEqual(report['survey_status'], 'identities-only')
            self.assertEqual(report['counts'], {'unsupported': 1})
            self.assertEqual(report['functions'][0]['ps2_identity']['status'], 'exact')

    def test_prepared_replay_rejects_changed_identity_manifest_before_comparison(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder)
            (output / 'report.json').write_text(json.dumps(dict(identity_manifest_sha256='changed')))
            with self.assertRaisesRegex(Unsupported, 'Identity manifest changed'):
                compare_prepared(output, identity_manifest=DEFAULT_MANIFEST)

    def test_recovered_overload_requires_exact_compiler_symbol(self):
        target = 'CMakeFiles/Kya.dir/src/test.cpp.obj'
        fn = dict(name='F', path='src/test.cpp', compiler_definition=dict(mangled='?F@@YAHH@Z'))
        symbols = {'?F@@YAHH@Z': [target], '?F@@YAHXZ': [target]}
        self.assertEqual(emitted_candidates(fn, '?F@@', symbols), [('?F@@YAHH@Z', target)])
        del symbols['?F@@YAHH@Z']
        self.assertEqual(emitted_candidates(fn, '?F@@', symbols), [])
        fn['compiler_definition']['mangled'] = ''
        with self.assertRaisesRegex(Unsupported, 'no emitted-symbol'):
            emitted_candidates(fn, '?F@@', symbols)

    @classmethod
    def setUpClass(cls):
        cls.fixture = json.loads((Path(__file__).parent / 'fixtures/compiled.json').read_text())

    def test_production_predicate_and_actual_compiler_mutant(self):
        sig = dict(arguments=['ulong'], fields=[], bits=8)
        original = self.fixture['ps2']['00100460']
        self.assertEqual(compare(self.fixture['exports'][0], original, sig)['result'], 'equivalent')
        report = compare(self.fixture['mutant'], original, sig)
        self.assertEqual(report['result'], 'mismatch')
        self.assertNotEqual(report['counterexample']['windows_return'], report['counterexample']['ps2_return'])

    def test_inferred_field_mapping_is_always_conditional(self):
        sig = dict(arguments=[], fields=['flags'], bits=32)
        report = compare(self.fixture['exports'][1], self.fixture['ps2']['001003f0'], sig,
                         dict(offset=self.fixture['layout'], size=4, integral=True))
        self.assertEqual(report['result'], 'conditional-equivalent')
        self.assertEqual(report['mapping']['ps2_offset'], 8)

    def test_no_field_mapping_cannot_pass_a_getter(self):
        with self.assertRaisesRegex(Unsupported, 'requires a reviewed mapping'):
            compare(self.fixture['exports'][1], self.fixture['ps2']['001003f0'],
                    dict(arguments=[], fields=['flags'], bits=32))

    def test_wrong_probe_offset_is_rejected_before_comparison(self):
        with self.assertRaisesRegex(Unsupported, 'compiler-probed'):
            compare(self.fixture['exports'][1], self.fixture['ps2']['001003f0'],
                    dict(arguments=[], fields=['flags'], bits=32),
                    dict(offset=self.fixture['layout'] + 4, size=4, integral=True))

    def test_unknown_instruction_is_not_a_success(self):
        pc = copy.deepcopy(self.fixture['exports'][0])
        pc['instructions'][0]['ops'][0]['opcode'] = 'UNSUPPORTED_TEST'
        with self.assertRaises(Unsupported):
            compare(pc, self.fixture['ps2']['00100460'], dict(arguments=['ulong'], fields=[], bits=8))

    def test_discovery_excludes_methods_from_field_names(self):
        fn = dict(parseError=False, name='Foo::Test',
                  source='bool Foo::Test() { return this->Helper(); }', body='{ return this->Helper(); }')
        self.assertEqual(signature(fn)['fields'], [])

    def test_boolean_argument_domain_is_canonical(self):
        args, inputs = arguments(['bool'])
        self.assertEqual(inputs[0][1].size(), 1)
        solver = z3.Solver()
        solver.add(z3.And(args[0] != 0, args[0] != 1))
        self.assertEqual(solver.check(), z3.unsat)

    def test_missing_object_write_needs_no_field_correspondence(self):
        report = compare(self.fixture['exports'][0], self.fixture['ps2']['00100400'],
                         dict(arguments=['ulong'], fields=[], bits=0))
        self.assertEqual(report['result'], 'mismatch')
        self.assertIsNone(report['mapping'])
        self.assertNotEqual(report['counterexample']['ps2_before'], report['counterexample']['ps2_after'])

    def test_extra_object_write_needs_no_field_correspondence(self):
        report = compare(self.fixture['exports'][2], self.fixture['ps2']['00100460'],
                         dict(arguments=['ulong'], fields=[], bits=0))
        self.assertEqual(report['result'], 'mismatch')
        self.assertIsNone(report['mapping'])

    def test_global_second_argument_uses_rdx_and_a1(self):
        report = compare(self.fixture['exports'][0], self.fixture['ps2']['00100460'],
                         dict(owner=None, arguments=['uint', 'ulong'], fields=[], bits=8))
        self.assertEqual(report['result'], 'equivalent')

    def test_global_discovery_and_pointer_domain(self):
        fn = dict(parseError=False, name='Test', source='int Test(void* data) { return 0; }', body='{ return 0; }')
        sig = signature(fn)
        self.assertIsNone(sig['owner'])
        self.assertEqual(sig['arguments'], ['pointer31'])
        args, _ = arguments(sig['arguments'])
        solver = z3.Solver()
        solver.add(z3.UGE(args[0], 0x80000000))
        self.assertEqual(solver.check(), z3.unsat)

    def test_original_elf_check_includes_delay_slot(self):
        elf = bytearray(136)
        elf[:6] = b'\x7fELF\x01\x01'
        struct.pack_into('<I', elf, 28, 52)
        struct.pack_into('<HH', elf, 42, 32, 1)
        struct.pack_into('<IIIIIIII', elf, 52, 1, 128, 0x100000, 0x100000, 8, 8, 5, 4)
        elf[128:] = bytes.fromhex('0800e0030400828c')  # jr ra; lw v0,4(a0)
        original = dict(executable_sha256=hashlib.sha256(elf).hexdigest(), instructions=[
            dict(address='00100000', bytes='0800e003', delay_slot_instructions=[
                dict(address='00100004', bytes='0400828c')])])
        verify_original_bytes(original, elf)
        original['instructions'][0]['delay_slot_instructions'][0]['bytes'] = '0800828c'
        with self.assertRaisesRegex(Unsupported, '00100004'):
            verify_original_bytes(original, elf)

    def test_incomplete_survey_never_exits_as_proof_success(self):
        self.assertEqual(exit_status([dict(result='equivalent')]), 0)
        self.assertEqual(exit_status([dict(result='unsupported')]), 2)
        self.assertEqual(exit_status([dict(result='conditional-equivalent')]), 2)
        self.assertEqual(exit_status([dict(result='mismatch')]), 1)
        self.assertEqual(exit_status([]), 2)

    def test_retained_survey_discrepancies_are_reproduced(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/small-functions-survey.json').read_text())
        self.assertEqual(len(fixture['cases']), 4)
        for case in fixture['cases']:
            with self.subTest(name=case['name']):
                self.assertEqual(compare(case['pc'], case['ps2'], case['signature'], case['layout'])['result'], 'mismatch')

    def test_int_returning_interfaces_match_all_field_bits(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/interface-is-active.json').read_text())
        self.assertEqual(len(fixture['cases']), 4)
        for case in fixture['cases']:
            with self.subTest(name=case['name']):
                self.assertEqual(case['signature']['bits'], 32)
                result = compare(case['pc'], case['ps2'], case['signature'], case['layout'],
                                 reviewed_mapping=case['reviewed_mapping'])
                self.assertEqual(result['result'], 'equivalent')

    def test_reviewed_mapping_cannot_approve_a_different_field_or_function(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/interface-is-active.json').read_text())
        case = next(c for c in fixture['cases'] if c['reviewed_mapping'])
        for key, value in [('ps2_entry', '00100000'), ('ps2_offset', 8), ('field', 'differentField')]:
            review = dict(case['reviewed_mapping'], **{key: value})
            with self.subTest(key=key), self.assertRaisesRegex(Unsupported, 'Reviewed field identity'):
                compare(case['pc'], case['ps2'], case['signature'], case['layout'], reviewed_mapping=review)

    def test_reviewed_boolean_invariant_and_zero_user_count(self):
        fixture = json.loads((Path(__file__).parent / 'fixtures/small-functions-survey.json').read_text())
        for case in fixture['cases']:
            for initial in (0, 1):
                with self.subTest(name=case['name'], initial=initial):
                    po = 0x1000 + case['mapping']['windows_offset']
                    so = 0x1000 + case['mapping']['ps2_offset']
                    pm = store(z3.Array('review_pc', z3.BitVecSort(64), z3.BitVecSort(8)), po, z3.BitVecVal(initial, 32))
                    sm = store(z3.Array('review_ps2', z3.BitVecSort(32), z3.BitVecSort(8)), so, z3.BitVecVal(initial, 32))
                    pc_out, ps2_out = outcomes(case['pc'], case['ps2'], pm, sm, [None, None],
                                              field_regions=([(po, 4)], [(so, 4)]))
                    for pv, pf, _, pg in pc_out:
                        for sv, sf, sg in ps2_out:
                            if case['signature']['bits']:
                                equal = z3.Extract(7, 0, pv) == z3.Extract(7, 0, sv)
                            else:
                                equal = word(pf, po, 4) == word(sf, so, 4)
                            # Remove differs at zero; the boolean getters agree
                            # for both canonical states. This does not prove
                            # their runtime initialization/lifetime invariant.
                            expected = z3.Not(equal) if not case['signature']['bits'] and initial == 0 else equal
                            prove(expected, z3.And(pg, sg), 'Reviewed domain observation changed')


if __name__ == '__main__':
    unittest.main()
