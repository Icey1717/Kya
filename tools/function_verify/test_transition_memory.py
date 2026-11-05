"""Heap relations and actual COFF data/duplicate-definition validation."""
import struct
import unittest
from verify import Unsupported
import z3
from coff_data import extract, identical_comdat
from compiled_extended import bundle, UnapprovedRelocations
from memory_relation import MemoryRelation, load, store


def coff(*, bss=False, pointer=False, body=b'\xc3\xcc\xcc\xcc', comdat=True):
    """Two-section AMD64 object with a function and an eight-byte global."""
    reloc = struct.pack('<IIH', 0, 0, 1) if pointer else b''
    raw = body + (b'' if bss else bytes(8))
    reloc_offset = 100 + len(raw)
    symoff = reloc_offset + len(reloc)
    header = struct.pack('<HHIIIHH', 0x8664, 2, 0, symoff, 2, 0, 0)
    text = struct.pack('<8sIIIIIIHHI', b'.text', 0, 0, len(body), 100, 0, 0, 0, 0, 0x60000020 | (0x1000 if comdat else 0))
    data = struct.pack('<8sIIIIIIHHI', b'.bss' if bss else b'.data', 0, 0, 8, 0 if bss else 100 + len(body),
                       reloc_offset if pointer else 0, 0, int(pointer), 0, 0xc0000080 if bss else 0xc0000040)
    symbols = struct.pack('<8sIhHBB', b'func', 0, 1, 0x20, 2, 0) + struct.pack('<8sIhHBB', b'global', 0, 2, 0, 2, 0)
    return header + text + data + raw + reloc + symbols + struct.pack('<I', 4)


class CoffDataTests(unittest.TestCase):
    def test_bss_bytes_and_permissions_are_recorded(self):
        item = extract(coff(bss=True), 'global', 8)
        self.assertEqual(item['bytes'], bytes(8))
        self.assertTrue(item['uninitialized'])
        self.assertTrue(item['writable'])

    def test_real_pointer_relocation_is_bound_to_function(self):
        data = coff(pointer=True)
        blob, _, addresses = bundle({'file': data}, [('file', None, 'f', 'func')], {}, globals={'global': ('file', 8)})
        self.assertEqual(struct.unpack_from('<Q', blob, addresses['global'])[0], addresses['func'])

    def test_missing_pointer_target_remains_frontier(self):
        with self.assertRaises(UnapprovedRelocations) as raised:
            bundle({'file': coff(pointer=True)}, [], {}, globals={'global': ('file', 8)})
        self.assertEqual(raised.exception.targets, ['func'])

    def test_pointer_relocation_addend_is_preserved(self):
        data = bytearray(coff(pointer=True))
        struct.pack_into('<Q', data, 104, 8)
        blob, _, addresses = bundle({'file': bytes(data)}, [('file', None, 'f', 'func')], {}, globals={'global': ('file', 8)})
        self.assertEqual(struct.unpack_from('<Q', blob, addresses['global'])[0], addresses['func'] + 8)

    def test_bad_extent_and_data_relocation_kind_rejected(self):
        with self.assertRaisesRegex(ValueError, 'extent'):
            extract(coff(), 'global', 9)
        data = bytearray(coff(pointer=True))
        struct.pack_into('<H', data, 120, 4)
        with self.assertRaisesRegex(ValueError, 'ADDR64'):
            extract(bytes(data), 'global', 8)

    def test_identical_comdat_selection_requires_all_bytes(self):
        self.assertEqual(identical_comdat({'b': coff(), 'a': coff()}, ['b', 'a'], 'func'), 'a')
        self.assertIsNone(identical_comdat({'a': coff(), 'b': coff(body=b'\x90\xc3\xcc\xcc')}, ['a', 'b'], 'func'))
        self.assertIsNone(identical_comdat({'a': coff(), 'b': coff(comdat=False)}, ['a', 'b'], 'func'))


class MemoryRelationTests(unittest.TestCase):
    def setUp(self):
        self.relation = MemoryRelation()
        self.relation.region('windows', 'actor', 0x100000000, 32)
        self.relation.region('ps2', 'actor', 0x1000, 16)
        self.w = z3.Array('w', z3.BitVecSort(64), z3.BitVecSort(8))
        self.s = z3.Array('s', z3.BitVecSort(32), z3.BitVecSort(8))

    def assert_proved(self, claim):
        solver = z3.Solver()
        solver.add(z3.Not(claim))
        self.assertEqual(solver.check(), z3.unsat)

    def test_native_pointers_are_related_by_identity_not_truncation(self):
        self.relation.identity(0x200000000, 0x4000)
        self.relation.field('actor', 8, 'actor', 4, 4, pointer=True)
        w = store(self.w, 0x100000008, z3.BitVecVal(0x200000000, 64))
        s = store(self.s, 0x1004, z3.BitVecVal(0x4000, 32))
        self.assert_proved(self.relation.condition(w, s))
        wrong = store(self.w, 0x100000008, z3.BitVecVal(0x4000, 64))
        self.assert_proved(z3.Not(self.relation.condition(wrong, s)))

    def test_unmapped_write_is_detected(self):
        self.relation.field('actor', 0, 'actor', 0, 4)
        changed = store(self.w, 0x100000008, load(self.w, 0x100000008, 1) ^ 1)
        self.assert_proved(z3.Not(self.relation.unchanged('windows', self.w, changed)))
        permitted = store(self.w, 0x100000000, z3.BitVecVal(123, 32))
        self.assert_proved(self.relation.unchanged('windows', self.w, permitted))

    def test_integer_width_mapping_preserves_signed_byte_values(self):
        self.relation.field('actor', 0, 'actor', 0, 4, ps2_size=1, signed=True)
        byte = z3.BitVec('idle_loop_byte', 8)
        w = store(self.w, 0x100000000, z3.SignExt(24, byte))
        s = store(self.s, 0x1000, byte)
        self.assert_proved(self.relation.condition(w, s))
        unsigned = store(self.w, 0x100000000, z3.ZeroExt(24, byte))
        self.assert_proved(z3.Implies(z3.Extract(7, 7, byte) == 1,
                                    z3.Not(self.relation.condition(unsigned, s))))

    def test_integer_width_mapping_rejects_incompatible_policies(self):
        for options in ({'ps2_size': 8}, {'ps2_size': 1, 'pointer': True},
                        {'signed': True}, {'ps2_size': 1, 'numeric_policy': object(), 'name': 'bad'}):
            with self.subTest(options=options), self.assertRaises(ValueError):
                self.relation.field('actor', 0, 'actor', 0, 4, **options)

    def test_empty_domain_aliases_and_overlapping_fields_rejected(self):
        with self.assertRaises(Unsupported):
            self.relation.require_satisfiable(z3.BoolVal(False))
        with self.assertRaisesRegex(ValueError, 'overlapping'):
            self.relation.region('windows', 'bad', 0x100000004, 16)
        self.relation.field('actor', 0, 'actor', 0, 4)
        with self.assertRaisesRegex(ValueError, 'Overlapping'):
            self.relation.field('actor', 2, 'actor', 4, 4)


if __name__ == '__main__':
    unittest.main()
