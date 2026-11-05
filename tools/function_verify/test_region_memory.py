"""Region optimization must retain the semantics of one aliased byte array."""
import unittest
from verify import Unsupported
import z3
from region_memory import RegionMemory
from memory_relation import load, store


class RegionMemoryTests(unittest.TestCase):
    def setUp(self):
        self.initial = z3.Array('region_bytes', z3.BitVecSort(64), z3.BitVecSort(8))

    def assert_proved(self, claim, domain=True):
        solver = z3.Solver()
        solver.set(timeout=5000)
        solver.add(domain, z3.Not(claim))
        self.assertEqual(solver.check(), z3.unsat)

    def test_symbolic_store_and_disjoint_reads_match_full_array(self):
        address = z3.BitVec('region_address', 64)
        domain = z3.And(z3.UGE(address, 0x1000), z3.ULE(address, 0x100c))
        heap = RegionMemory(self.initial, [(0x1000, 16), (0x2000, 16)], domain)
        heap.store(address, z3.BitVecVal(0x12345678, 32), z3.BoolVal(True))
        expected = store(self.initial, address, z3.BitVecVal(0x12345678, 32))
        for pointer in (z3.BitVecVal(0x1000, 64), z3.BitVecVal(0x2000, 64), address):
            self.assert_proved(heap.load(pointer, 4, z3.BoolVal(True)) == load(expected, pointer, 4), domain)
        self.assert_proved(heap.memory == expected)

    def test_ambiguous_store_updates_every_view(self):
        choose = z3.Bool('choose_region')
        address = z3.If(choose, z3.BitVecVal(0x1000, 64), z3.BitVecVal(0x2000, 64))
        heap = RegionMemory(self.initial, [(0x1000, 16), (0x2000, 16)], z3.BoolVal(True))
        heap.store(address, z3.BitVecVal(7, 32), z3.BoolVal(True))
        for pointer in (0x1000, 0x2000):
            expected = load(store(self.initial, address, z3.BitVecVal(7, 32)), pointer, 4)
            self.assert_proved(heap.load(z3.BitVecVal(pointer, 64), 4, z3.BoolVal(True)) == expected)

    def test_condition_certificates_cannot_leak_to_another_branch(self):
        choose = z3.Bool('certificate_branch')
        address = z3.If(choose, z3.BitVecVal(0x1000, 64), z3.BitVecVal(0x2000, 64))
        heap = RegionMemory(self.initial, [(0x1000, 16), (0x2000, 16)], z3.BoolVal(True))
        self.assertEqual(heap.owner(address, 4, choose), 0)
        self.assertEqual(heap.owner(address, 4, z3.Not(choose)), 1)

    def test_overlapping_regions_are_one_storage_view(self):
        heap = RegionMemory(self.initial, [(0x1000, 16), (0x1008, 16)], z3.BoolVal(True))
        self.assertEqual(heap.regions, [(0x1000, 24)])
        heap.store(z3.BitVecVal(0x100c, 64), z3.BitVecVal(9, 32), z3.BoolVal(True))
        self.assert_proved(heap.load(z3.BitVecVal(0x100c, 64), 4, z3.BoolVal(True)) == 9)

    def test_initial_stores_outside_a_view_do_not_hide_symbolic_aliases(self):
        alias = z3.BitVec('initial_store_alias', 64)
        initial = store(self.initial, 0x1000, z3.BitVecVal(0x12345678, 32))
        initial = store(initial, 0x2000, z3.BitVecVal(0x87654321, 32))
        initial = store(initial, alias, z3.BitVecVal(0xff, 8))
        initial = store(initial, 0x3000, z3.BitVecVal(0xdeadbeef, 32))
        heap = RegionMemory(initial, [(0x1000, 16), (0x2000, 16)], z3.BoolVal(True))
        for base in (0x1000, 0x2000, 0x3000):
            pointer = z3.BitVecVal(base, 64)
            self.assert_proved(heap.load(pointer, 4, z3.BoolVal(True)) == load(initial, pointer, 4))
        # Later ambiguous writes must update both already-materialized views.
        heap.store(alias, z3.BitVecVal(0x42, 8), z3.BoolVal(True))
        final = store(initial, alias, z3.BitVecVal(0x42, 8))
        for base in (0x1000, 0x2000):
            pointer = z3.BitVecVal(base, 64)
            self.assert_proved(heap.load(pointer, 4, z3.BoolVal(True)) == load(final, pointer, 4))

    def test_certified_initial_store_pruning_preserves_full_array(self):
        address = z3.BitVec('initial_private_address', 64)
        domain = z3.And(z3.UGE(address, 0x2000), z3.ULE(address, 0x200c))
        initial = store(self.initial, address, z3.BitVecVal(0x12345678, 32))
        heap = RegionMemory(initial, [(0x1000, 16), (0x2000, 16)], domain, prune_initial_stores=True)
        for pointer in (z3.BitVecVal(0x1000, 64), z3.BitVecVal(0x2000, 64), address):
            self.assert_proved(heap.load(pointer, 4, z3.BoolVal(True)) == load(initial, pointer, 4), domain)
        self.assert_proved(heap.memory == initial)

    def test_pruned_initial_views_do_not_assume_a_later_branch(self):
        choose = z3.Bool('initial_alias_branch')
        address = z3.If(choose, z3.BitVecVal(0x1000, 64), z3.BitVecVal(0x2000, 64))
        initial = store(self.initial, address, z3.BitVecVal(0x42, 8))
        heap = RegionMemory(initial, [(0x1000, 16), (0x2000, 16)], z3.BoolVal(True), prune_initial_stores=True)
        for condition in (choose, z3.Not(choose)):
            for base in (0x1000, 0x2000):
                pointer = z3.BitVecVal(base, 64)
                self.assert_proved(heap.load(pointer, 4, condition) == load(initial, pointer, 4), condition)


if __name__ == '__main__':
    unittest.main()
