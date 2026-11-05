"""Real relocation arithmetic and checked intra-function switch dispatch."""
import copy
import struct
import unittest
from compiled_extended import bundle
from coff_jump_tables import append, recover, targets
from call_graph import link
from paths import expand
from verify import Unsupported, execute
import z3
from test_dispatch_sets import body, node


def object_with_table():
    data = bytearray(228)
    struct.pack_into('<HHIIIHH', data, 0, 0x8664, 2, 0, 170, 3, 0, 0)
    struct.pack_into('<8sIIIIIIHHI', data, 20, b'.text', 0, 0, 32, 100, 160, 0, 1, 0, 0x60000020)
    struct.pack_into('<8sIIIIIIHHI', data, 60, b'.rdata', 0, 0, 8, 132, 140, 0, 2, 0, 0x40000040)
    data[100:120] = bytes.fromhex('83e8010f8700000000488b4db8488d0500000000')
    data[120], data[124] = 0xc3, 0xc3
    struct.pack_into('<ii', data, 132, 20 + 4, 24 + 8)
    struct.pack_into('<IIH', data, 140, 0, 0, 4)
    struct.pack_into('<IIH', data, 150, 4, 0, 4)
    struct.pack_into('<IIH', data, 160, 16, 1, 4)
    for i, (name, section, kind) in enumerate([(b'.text', 1, 0), (b'.rdata', 2, 0), (b'func', 1, 0x20)]):
        struct.pack_into('<8sIhHBB', data, 170 + i * 18, name, 0, section, kind, 3, 0)
    struct.pack_into('<I', data, 224, 4)
    return bytes(data)


class JumpTableTests(unittest.TestCase):
    function = dict(coff_section=1, coff_offset=0, size=32, blob_offset=0)

    def test_original_rel32_entries_become_correct_isolated_targets(self):
        data = object_with_table()
        table = recover(data, 'test', self.function, 16, 1)
        self.assertEqual(table['targets'], [20, 24])
        blob = bytearray(data[100:132])
        relocated = append(blob, table, self.function)
        self.assertEqual(relocated['address'], 32)
        self.assertEqual([32 + struct.unpack_from('<i', blob, 32 + i * 4)[0] for i in range(2)], [20, 24])

    def test_bundle_resolves_table_reference_and_retains_table_provenance(self):
        data = object_with_table()
        blob, functions, addresses = bundle({'test': data}, [('test', None, 'switch', 'func')], {}, jump_tables=True)
        table = functions[0]['jump_tables'][0]
        self.assertEqual(20 + struct.unpack_from('<i', blob, 16)[0], table['address'])
        self.assertEqual(addresses[table['symbol']], table['address'])
        self.assertEqual(table['targets'], [20, 24])

    def test_invalid_bounds_relocations_targets_and_permissions_fail(self):
        mutations = [(102, b'\x80'), (103, b'\x90'), (148, b'\x03\x00'),
                     (132, struct.pack('<i', 1000)), (136, struct.pack('<i', -200)),
                     (96, struct.pack('<I', 0xc0000040)), (16 + 100, struct.pack('<i', 4))]
        for offset, replacement in mutations:
            data = bytearray(object_with_table())
            data[offset:offset + len(replacement)] = replacement
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                recover(bytes(data), 'test', self.function, 16, 1)

    def test_data_and_selected_bodies_cannot_be_deferred_as_functions(self):
        data = object_with_table()
        for name in ('.rdata', 'func'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                bundle({'test': data}, [('test', None, 'switch', 'func')], {}, deferred_functions=[name])

    def test_lifted_targets_must_be_actual_instruction_boundaries(self):
        export = dict(instructions=[dict(address='10', ops=[dict(opcode='BRANCHIND')])],
                      coff=dict(jump_tables=[dict(targets=[0x11])]))
        with self.assertRaisesRegex(ValueError, 'instruction boundary'):
            targets(export)

    def graph(self, selected=(0x110, 0x120)):
        root = body(0x100, [(0x100, [dict(opcode='BRANCHIND', inputs=[node('register', 0x50)])]),
            (0x110, [dict(opcode='COPY', output=node('register', 0x20), inputs=[node('const', 2)])]),
            (0x114, [dict(opcode='RETURN', inputs=[node('register', 0x1f0)])]),
            (0x120, [dict(opcode='COPY', output=node('register', 0x20), inputs=[node('const', 3)])]),
            (0x124, [dict(opcode='RETURN', inputs=[node('register', 0x1f0)])])])
        return link([root], 0x100, indirect_targets={0x100: selected})

    def test_switch_membership_is_checked_and_selected_case_executes(self):
        paths = expand(self.graph())
        for address, expected in ((0x110, 2), (0x120, 3)):
            results = [execute(path, z3.BitVecVal(address, 64), path_mode=True, call_mode=True,
                               precondition=z3.BoolVal(True)) for path in paths]
            results = [value for value in results if value is not None and not (
                isinstance(value, tuple) and z3.is_false(z3.simplify(value[2])))]
            self.assertEqual(len(results), 1)
            self.assertEqual(z3.simplify(results[0][0] if isinstance(results[0], tuple) else results[0]).as_long(), expected)
        with self.assertRaises(Unsupported):
            execute(paths[0], z3.BitVecVal(0x130, 64), path_mode=True, call_mode=True,
                    precondition=z3.BoolVal(True))

    def test_unapproved_internal_address_fails_before_execution(self):
        with self.assertRaises(Unsupported):
            self.graph((0x111,))

    def test_large_switch_tree_preserves_every_case_and_bounds_depth(self):
        candidates = [0x200 + i * 0x10 for i in range(16)]
        rows = [(0x100, [dict(opcode='BRANCHIND', inputs=[node('register', 0x50)])])]
        for index, address in enumerate(candidates):
            rows.extend([(address, [dict(opcode='COPY', output=node('register', 0x20),
                                         inputs=[node('const', index)])]),
                         (address + 4, [dict(opcode='RETURN', inputs=[node('register', 0x1f0)])])])
        graph = link([body(0x100, rows)], 0x100, indirect_targets={0x100: list(reversed(candidates))})
        paths = expand(graph, max_paths=32)
        self.assertEqual(len(paths), 16)
        self.assertLessEqual(max(sum(i.get('text') == 'checked switch dispatch' for i in p['instructions'])
                                 for p in paths), 5)
        for index, address in enumerate(candidates):
            results = [execute(path, z3.BitVecVal(address, 64), path_mode=True, call_mode=True,
                               precondition=z3.BoolVal(True)) for path in paths]
            results = [value for value in results if value is not None and not (
                isinstance(value, tuple) and z3.is_false(z3.simplify(value[2])))]
            self.assertEqual(len(results), 1)
            self.assertEqual(z3.simplify(results[0][0] if isinstance(results[0], tuple) else results[0]).as_long(), index)


if __name__ == '__main__':
    unittest.main()
