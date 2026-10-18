import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

from audit_vtables import Auditor, Bridge, BridgeError, SourceClass, Method, scan_source, virtual_methods


class SourceTests(unittest.TestCase):
    def scan(self, source):
        with tempfile.TemporaryDirectory() as directory:
            Path(directory, 'actors.h').write_text(source)
            return scan_source(Path(directory))

    def test_implicit_overrides_defaults_and_nested_bodies(self):
        classes, _ = self.scan('''
            class Base { public: virtual void F(int n = 0); virtual int G() const { return 1; } };
            class Child : public Base { public: void F(int n) { if (n) { F(0); } }
                struct Nested { virtual void Unrelated(); }; int G() const override;
                int (*callback)(int); static void Static(); };
            // class Fake { virtual void Bad(); };
            struct Plain { int value; };
        ''')
        self.assertEqual(virtual_methods('Child', classes), {'F', 'G'})
        self.assertEqual({m.name for m in classes['Child'].methods}, {'F', 'G'})
        self.assertEqual(classes['Child'].bases, ['Base'])
        self.assertNotIn('Fake', classes)
        self.assertFalse(virtual_methods('Plain', classes))

    def test_duplicate_definitions_are_explicit(self):
        classes, duplicates = self.scan('class A { virtual void F(); }; class A { virtual void G(); };')
        self.assertEqual(len(duplicates), 1)
        self.assertIn('duplicate', classes['A'].issues[0])

    def test_multiple_and_unknown_bases_are_retained(self):
        classes, _ = self.scan('class A {}; class B {}; class C : public A, protected B { virtual void F(); };')
        self.assertEqual(classes['C'].bases, ['A', 'B'])

    def test_parameter_types_distinguish_overloads_and_strip_defaults(self):
        classes, _ = self.scan('''class A { virtual void F(const Thing* value = nullptr); };
            class B : public A { void F(const Thing* renamed); void F(float value); };''')
        self.assertEqual(classes['A'].methods[0].param_types, ('constThing*',))
        self.assertEqual(classes['B'].methods[0].param_types, ('constThing*',))
        self.assertEqual(classes['B'].methods[1].param_types, ('float',))


def method(name, arity=0, virtual=True):
    return Method(name, arity, virtual, False, name + '()')


def target(address, name=None):
    result = {'address': address, 'canonical': address, 'kind': 'function', 'bytes': ''}
    if name:
        result['function'] = {'name': name}
    return result


class CompareTests(unittest.TestCase):
    def auditor(self, own=()):
        classes = {'Base': SourceClass('Base', [], '', 1, [method('GetWindState')]),
                   'Child': SourceClass('Child', ['Base'], '', 2, list(own))}
        audit = Auditor(None, classes)
        audit.tables = {
            'Base': {'address': '00400000', 'entries': [{'offset': 0x100, 'target': target('00100000', 'GetWindState')}]},
            'Child': {'address': '00400100', 'entries': [{'offset': 0x100, 'target': target('00100010')}]},
        }
        return audit

    def test_missing_unnamed_getter_uses_parent_slot_name(self):
        finding = self.auditor().compare('Child')[0]
        self.assertEqual(finding['kind'], 'missing_override')
        self.assertEqual(finding['method'], 'GetWindState')
        self.assertEqual(finding['offset'], 0x100)

    def test_non_virtual_keyword_override_is_recognized(self):
        self.assertEqual(self.auditor([method('GetWindState', virtual=False)]).compare('Child'), [])

    def test_wrong_arity_is_reviewed_not_silently_accepted(self):
        finding = self.auditor([method('GetWindState', arity=1)]).compare('Child')[0]
        self.assertEqual(finding['kind'], 'review')

    def test_jump_thunk_to_parent_is_not_an_override(self):
        audit = self.auditor()
        audit.tables['Child']['entries'][0]['target']['canonical'] = '00100000'
        self.assertEqual(audit.compare('Child'), [])

    def test_unknown_slot_is_reviewed(self):
        audit = self.auditor()
        audit.tables['Base']['entries'][0]['target'].pop('function')
        self.assertEqual(audit.compare('Child')[0]['kind'], 'review')

    def test_actor_slot_hints_do_not_conflict_with_unrelated_hierarchy(self):
        audit = self.auditor([method('GetWindState')])
        audit.classes['Base'].methods.append(method('Create'))
        audit.slot_hints = {0x100: 'Create'}
        self.assertEqual(audit.compare('Child'), [])

    def test_actor_hierarchy_still_uses_slot_hints_for_unnamed_targets(self):
        audit = self.auditor()
        audit.classes['CActor'] = SourceClass('CActor', [], '', 1)
        audit.classes['Base'].bases = ['CActor']
        audit.tables['Base']['entries'][0]['target'].pop('function')
        audit.slot_hints = {0x100: 'GetWindState'}
        self.assertEqual(audit.compare('Child')[0]['kind'], 'missing_override')

    def test_new_child_slot_is_not_a_missing_parent_override(self):
        audit = self.auditor()
        audit.tables['Child']['entries'][0]['offset'] = 0x104
        self.assertEqual(audit.compare('Child'), [])

    def test_same_name_and_arity_with_different_types_requires_review(self):
        own = method('GetWindState')
        own.param_types = ('float',)
        audit = self.auditor([own])
        audit.classes['Base'].methods[0].param_types = ('int',)
        self.assertEqual(audit.compare('Child')[0]['kind'], 'review')


class FakeBridge:
    def __init__(self, functions=None, references=None, labels=None, memory=None):
        self.functions = functions or []
        self.references = references or []
        self.labels = labels or {}
        self.memory = memory or {}

    def call(self, command, **args):
        if command == 'list_functions':
            return {'functions': self.functions}
        if command == 'references_from':
            return {'references': self.references}
        if command == 'get_labels':
            return {'labels': self.labels.get(args['address'], [])}
        if command == 'inspect_data':
            return {'hex': self.memory[args['address']].hex()}
        if command == 'get_function':
            raise BridgeError('No function defined')
        raise AssertionError(command)


class CaptureTests(unittest.TestCase):
    def test_derived_constructor_discovers_inline_base_table(self):
        fake = FakeBridge(
            functions=[{'name': 'Child', 'namespace': 'Child', 'entry': '00100000'}],
            references=[{'to': '00400000', 'type': 'DATA'}, {'to': '00400100', 'type': 'DATA'}],
            labels={'00400000': [{'name': '_vt', 'namespace': 'Base'}],
                    '00400100': [{'name': '_vt', 'namespace': 'Child'}]})
        audit = Auditor(fake, {'Base': None, 'Child': None})
        self.assertEqual(audit.discover('Child'), '00400100')
        self.assertEqual(audit.discovered['Base'], {'00400000'})

    def test_unnamed_getter_and_next_table_header(self):
        fake = FakeBridge(memory={
            '00400000': struct.pack('<6I', 0, 0, 0x100000, 0x100010, 0, 0),
            '00100000': struct.pack('<2I', 0x03e00008, 0),
            '00100010': struct.pack('<2I', 0x03e00008, 0x24820380),
        })
        table = Auditor(fake, {}).capture('Child', '00400000')
        self.assertEqual([e['offset'] for e in table['entries']], [8, 12])
        self.assertEqual(table['entries'][1]['target']['kind'], 'unnamed_leaf')
        self.assertTrue(table['bounded'])

    def test_only_safe_direct_mips_jump_thunks_are_followed(self):
        fake = FakeBridge(memory={
            '00100000': struct.pack('<2I', 0x08040004, 0),
            '00100010': struct.pack('<2I', 0x03e00008, 0),
        })
        result = Auditor(fake, {}).target('00100000')
        self.assertEqual(result['kind'], 'thunk')
        self.assertEqual(result['canonical'], '00100010')

    def test_virtual_dispatch_wrapper_keeps_destination_for_review(self):
        fake = FakeBridge(memory={
            '00100000': struct.pack('<4I', 0x8c990000, 0x8f39016c, 0x03200008, 0),
        })
        result = Auditor(fake, {}).target('00100000')
        self.assertEqual(result['kind'], 'virtual_dispatch')
        self.assertEqual(result['dispatch_offset'], 0x16c)

    def test_slot_limit_is_reported_as_incomplete(self):
        fake = FakeBridge(memory={'00400000': struct.pack('<3I', 0, 0, 0x100000),
                                 '00100000': struct.pack('<2I', 0x03e00008, 0)})
        table = Auditor(fake, {}, max_slots=1).capture('Child', '00400000')
        self.assertFalse(table['bounded'])

    def test_offline_missing_response_and_mutation_are_refused(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory, 'snapshot.json')
            path.write_text(json.dumps({'schema': 1, 'program': {'language': 'r5900:LE:32:default'}, 'responses': {}}))
            bridge = Bridge('127.0.0.1', 1, path, offline=True)
            with self.assertRaisesRegex(BridgeError, 'no response'):
                bridge.call('get_labels', address='00400000')
            with self.assertRaisesRegex(BridgeError, 'Non-read-only'):
                bridge.call('rename_function')

    def test_offline_cached_response_never_opens_socket(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory, 'snapshot.json')
            key = json.dumps({'command': 'get_labels', 'address': '00400000'}, sort_keys=True)
            path.write_text(json.dumps({'schema': 1, 'program': {'language': 'r5900:LE:32:default'}, 'responses': {key: {'ok': True, 'result': {'labels': []}}}}))
            with patch('socket.create_connection', side_effect=AssertionError('network used')):
                bridge = Bridge('127.0.0.1', 1, path, offline=True)
                self.assertEqual(bridge.call('get_labels', address='00400000'), {'labels': []})

    def test_offline_non_ps2_snapshot_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory, 'snapshot.json')
            path.write_text(json.dumps({'schema': 1, 'program': {'language': 'x86:LE:64:default'}, 'responses': {}}))
            with self.assertRaisesRegex(BridgeError, 'PS2 program identity'):
                Bridge('127.0.0.1', 1, path, offline=True)


if __name__ == '__main__':
    unittest.main()
