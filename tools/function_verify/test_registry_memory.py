"""Actual compiled pointer resolution, bounded symbolic heaps and mutations."""
import copy
import json
from pathlib import Path
import unittest
from verify import Unsupported
import z3
from registry_memory import compare, _compare_case, seed


class RegistryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        fixture = json.loads((Path(__file__).parent / 'fixtures/registry-resolver.json').read_text())
        cls.manifest, cls.blob = fixture['manifest'], bytes.fromhex(fixture['blob_hex'])

    def test_complete_compiled_resolver_both_registries_and_null(self):
        result = compare(self.manifest, self.blob)
        self.assertEqual(result['result'], 'equivalent')
        self.assertEqual(result['exhaustive_handle_partitions'], 5)
        self.assertGreater(result['feasible_paths'], 4)

    def test_wrong_slot_layout_does_not_pass(self):
        manifest = copy.deepcopy(self.manifest)
        manifest['layout']['slot_pointer'] = 16
        result = _compare_case(manifest, self.blob, capacity=1, handle=z3.BitVecVal(0x100000, 32))
        self.assertEqual(result['result'], 'mismatch')

    def test_injected_registry_write_is_rejected(self):
        manifest = copy.deepcopy(self.manifest)
        root = next(e for e in manifest['exports'] if e['name'] == 'ResolvePointerChecked')
        address = next(a for s, a in manifest['addresses'].items() if s.startswith('?gPersistentRegistry@'))
        root['instructions'][0]['ops'].insert(0, dict(opcode='STORE', inputs=[
            dict(space='const', offset='1b1', size=4, constant=True),
            dict(space='const', offset=hex(address)[2:], size=8, constant=True),
            dict(space='const', offset='0', size=4, constant=True)]))
        root['operation_count'] += 1
        with self.assertRaisesRegex(Unsupported, 'memory access'):
            _compare_case(manifest, self.blob, capacity=1, handle=z3.BitVecVal(0, 32))

    def test_stale_and_released_slots_are_excluded_explicitly(self):
        memory = z3.Array('registry_test', z3.BitVecSort(64), z3.BitVecSort(8))
        _, _, _, domain, _ = seed(memory, self.manifest['addresses'], self.manifest['layout'], capacity=1,
                                   handle=z3.BitVecVal(0x100000, 32))
        solver = z3.Solver()
        solver.add(domain, z3.BitVec('persistent_slot_0_refs', 32) == 0)
        self.assertEqual(solver.check(), z3.unsat)
        solver = z3.Solver()
        solver.add(domain, z3.BitVec('persistent_slot_0_generation', 16) != 1)
        self.assertEqual(solver.check(), z3.unsat)

    def test_mixed_image_exports_do_not_pass(self):
        manifest = copy.deepcopy(self.manifest)
        manifest['exports'][0]['executable_sha256'] = '0' * 64
        with self.assertRaisesRegex(Unsupported, 'compiled image'):
            compare(manifest, self.blob)


if __name__ == '__main__':
    unittest.main()
