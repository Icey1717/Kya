"""Regress real compiled branches, calls, and loops; reject incomplete proofs."""
import copy
import json
from pathlib import Path
import struct
import unittest
from unittest.mock import patch
import tempfile

from verify import Unsupported
from compare_extended import compare, windows_call
import compiled_extended
import test_compiled
import check


class ExtendedCompiledTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixture = json.loads((Path(__file__).parent / 'fixtures/compiled_extended.json').read_text())

    def test_five_production_functions(self):
        for name in ['HasChanged', 'GetType', 'BaseFlags', 'AutFlags', 'IsInList']:
            with self.subTest(name=name):
                result = compare(self.fixture['positive'], name, self.fixture['originals'])
                self.assertEqual(result['result'], 'equivalent')
                self.assertGreater(result['windows_paths'], 1)

    def test_real_compiled_negative_controls(self):
        for mutation, name in [('branch', 'GetType'), ('call', 'AutFlags'), ('loop', 'IsInList')]:
            with self.subTest(mutation=mutation):
                result = compare(self.fixture['mutants'][mutation], name, self.fixture['originals'])
                self.assertEqual(result['result'], 'mismatch')
                self.assertNotEqual(result['counterexample']['windows_return'], result['counterexample']['ps2_return'])

    def test_insufficient_loop_bound_fails_closed(self):
        with self.assertRaisesRegex(Unsupported, 'loop cutoff'):
            compare(self.fixture['positive'], 'IsInList', self.fixture['originals'], loop_visits=4)

    def test_unapproved_call_target_rejected(self):
        exports = {e['name']: e for e in copy.deepcopy(self.fixture['positive']['exports'])}
        for instruction in exports['AutFlags']['instructions']:
            for op in instruction['ops']:
                if op['opcode'] == 'CALL':
                    op['inputs'][0]['offset'] = 'dead'
        with self.assertRaisesRegex(Unsupported, 'callee'):
            windows_call(exports['AutFlags'], exports['BaseFlags'])

    def test_corrupted_call_return_address_rejected(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        for export in manifest['exports']:
            if export['name'] == 'AutFlags':
                for instruction in export['instructions']:
                    if any(op['opcode'] == 'CALL' for op in instruction['ops']):
                        for op in instruction['ops']:
                            if op['opcode'] == 'STORE':
                                op['inputs'][2]['offset'] = 'dead'
        with self.assertRaisesRegex(Unsupported, 'return target'):
            compare(manifest, 'AutFlags', self.fixture['originals'])

    def test_reachable_unknown_instruction_rejected(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        manifest['exports'][0]['instructions'][0]['ops'][0]['opcode'] = 'CALLOTHER'
        with self.assertRaises(Unsupported):
            compare(manifest, 'HasChanged', self.fixture['originals'])

    def test_invalid_pointer_layout_rejected(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        manifest['layout']['pointer_size'] = 4
        with self.assertRaisesRegex(Unsupported, 'pointers'):
            compare(manifest, 'IsInList', self.fixture['originals'])

    def test_wrong_branch_field_mapping_detected(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        manifest['layout']['money'] += 4
        self.assertEqual(compare(manifest, 'GetType', self.fixture['originals'])['result'], 'mismatch')

    def test_reachable_assertion_path_fails_closed(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        base = next(e for e in manifest['exports'] if e['name'] == 'BaseFlags')
        branch = next(op for i in base['instructions'] for op in i['ops'] if op['opcode'] == 'CBRANCH')
        branch['inputs'][1] = dict(space='const', offset='0', size=1, constant=True)
        with self.assertRaises(Unsupported):
            compare(manifest, 'BaseFlags', self.fixture['originals'])


class RelocationTests(unittest.TestCase):
    def test_rel32_addend_resolved_exactly(self):
        data = test_compiled.CoffTests().object(relocation=True)
        # The helper's body is four bytes; move relocation to offset zero so
        # its four-byte field stays wholly within the selected function.
        data = bytearray(data)
        struct.pack_into('<I', data, 64, 0)
        addend = struct.unpack_from('<i', data, 60)[0]
        with patch.object(compiled_extended, 'CASES', [('file', '1', 'f', 'function')]), patch.object(compiled_extended, 'TABLES', {}):
            blob, functions, addresses = compiled_extended.bundle({'file': bytes(data)})
        self.assertEqual(struct.unpack_from('<i', blob)[0], addend - 4)
        self.assertEqual(addresses['function'], 0)
        self.assertEqual(len(functions[0]['relocations']), 1)

    def test_unsupported_relocation_rejected(self):
        data = bytearray(test_compiled.CoffTests().object(relocation=True))
        struct.pack_into('<I', data, 64, 0)  # offset 0; symbol index remains 0
        struct.pack_into('<H', data, 72, 1)  # AMD64 ADDR64 is outside this linker
        with patch.object(compiled_extended, 'CASES', [('file', '1', 'f', 'function')]):
            with self.assertRaisesRegex(ValueError, 'REL32'):
                compiled_extended.bundle({'file': bytes(data)})


class DependencyClosureTests(unittest.TestCase):
    def test_complete_callee_is_included_before_missing_global_is_reported(self):
        cases = [('file', '1', 'root', 'root')]
        with patch.object(compiled_extended, 'coff_symbols', return_value=([('callee', 0, 1, 0x20)], 1)), \
             patch.object(compiled_extended, 'bundle', side_effect=[compiled_extended.UnapprovedRelocations(['callee']),
                                                                    compiled_extended.UnapprovedRelocations(['global'])]):
            selected, prepared, frontier = compiled_extended.bundle_closure({'file': b'object'}, cases)
        self.assertIsNone(prepared)
        self.assertEqual(frontier, ['global'])
        self.assertEqual([c[3] for c in selected], ['root', 'callee'])

    def test_ambiguous_callee_does_not_choose_arbitrary_definition(self):
        with patch.object(compiled_extended, 'coff_symbols', return_value=([('callee', 0, 1, 0x20)], 1)), \
             patch.object(compiled_extended, 'bundle', side_effect=compiled_extended.UnapprovedRelocations(['callee'])):
            selected, prepared, frontier = compiled_extended.bundle_closure({'one': b'1', 'two': b'2'}, [('one', '1', 'root', 'root')])
        self.assertIsNone(prepared)
        self.assertEqual(frontier, ['callee'])
        self.assertEqual(len(selected), 1)

    def test_dependency_bound_is_failure(self):
        with patch.object(compiled_extended, 'coff_symbols', return_value=([('callee', 0, 1, 0x20)], 1)), \
             patch.object(compiled_extended, 'bundle', side_effect=compiled_extended.UnapprovedRelocations(['callee'])):
            with self.assertRaisesRegex(ValueError, 'function bound'):
                compiled_extended.bundle_closure({'file': b'object'}, [('file', '1', 'root', 'root')], max_functions=1)


class WatchInputsTests(unittest.TestCase):
    def test_registry_source_and_transition_helpers_are_watched(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            pointer = root / 'src/port/pointer_conv.cpp'
            pointer.parent.mkdir(parents=True)
            pointer.write_text('before')
            with patch.object(check, 'ROOT', root), patch.object(check.subprocess, 'check_output', return_value='') as deps:
                before = check.snapshot('ninja', root / 'build')
                pointer.write_text('after')
                after = check.snapshot('ninja', root / 'build')
            self.assertNotEqual(before, after)
            arguments = deps.call_args.args[0]
            self.assertIn('CMakeFiles/Kya.dir/src/port/pointer_conv.cpp.obj', arguments)
            self.assertIn('CMakeFiles/Kya.dir/src/b-witch/scene.cpp.obj', arguments)

    def test_local_dependency_edits_and_deletions_change_snapshot(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            header = root / 'src/changed.h'
            header.parent.mkdir()
            header.write_text('initial')
            dependencies = 'target: deps\n    ' + str(header) + '\n    C:/external/sdk.h\n'
            with patch.object(check, 'ROOT', root), patch.object(check.subprocess, 'check_output', return_value=dependencies):
                initial = check.snapshot('ninja', root / 'build')
                header.write_text('edited')
                edited = check.snapshot('ninja', root / 'build')
                header.unlink()
                deleted = check.snapshot('ninja', root / 'build')
            self.assertNotEqual(initial, edited)
            self.assertNotEqual(edited, deleted)
            self.assertFalse(any('external' in p for p in initial))


if __name__ == '__main__':
    unittest.main()
