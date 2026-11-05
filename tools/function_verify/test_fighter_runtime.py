"""Production lookup, registry composition and unchanged-state regression checks."""
import copy
import json
from pathlib import Path
import unittest
from verify import Unsupported
import z3
from fighter_runtime import compare, compare_case, inputs


class FighterRuntimeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        fixture = json.loads((Path(__file__).parent / 'fixtures/fighter-runtime.json').read_text())
        cls.manifest, cls.originals = fixture['manifest'], fixture['originals']
        cls.blob = bytes.fromhex(fixture['blob_hex'])

    def test_complete_bodies_and_all_bounded_partitions(self):
        report = compare(self.manifest, self.blob, self.originals)
        self.assertEqual(report['result'], 'equivalent')
        self.assertEqual(len(report['functions']), 2)
        for row in report['functions']:
            self.assertEqual(len(row['partitions']), 9)
            self.assertTrue(all(p['windows_paths'] and p['ps2_paths'] for p in row['partitions']))

    def test_general_domain_includes_mixed_null_and_registry_bindings(self):
        data = inputs(self.manifest, count=2, transient=0, general_bindings=True)
        handles = [z3.BitVec('behaviour_handle_' + str(i), 32) for i in range(2)]
        for first, second in [(0, 0x80000001), (1, 0x80000001), (0, 0)]:
            solver = z3.Solver()
            solver.add(data['domain'], handles[0] == first, handles[1] == second)
            self.assertEqual(solver.check(), z3.sat)

    def test_unsorted_runtime_list_is_not_equivalent(self):
        result = compare_case(self.manifest, self.originals, 'GetBehaviour', sorted_ids=False)
        self.assertEqual(result['result'], 'mismatch')
        self.assertEqual(result['architecture'], 'ps2')

    def test_wrong_registry_slot_mapping_does_not_pass(self):
        manifest = copy.deepcopy(self.manifest)
        manifest['layouts']['PointerConv']['slot_pointer'] = 16
        result = compare_case(manifest, self.originals, 'GetBehaviour', count=1)
        self.assertEqual(result['result'], 'mismatch')

    def test_unsigned_ps2_lookup_is_a_mismatch(self):
        originals = copy.deepcopy(self.originals)
        changed = 0
        for instruction in originals['00103f60']['instructions']:
            for op in instruction['ops']:
                if instruction['address'] == '00103f8c' and op['opcode'] == 'INT_SLESS':
                    op['opcode'] = 'INT_LESS'
                    changed += 1
        self.assertEqual(changed, 1)
        self.assertEqual(compare_case(self.manifest, originals, 'GetBehaviour')['result'], 'mismatch')

    def test_live_registry_is_an_explicit_input_obligation(self):
        data = inputs(self.manifest, count=2, transient=0)
        for stale in [z3.BitVec('persistent_slot_0_refs', 32) == 0,
                      z3.BitVec('persistent_slot_0_generation', 16) !=
                      z3.ZeroExt(5, z3.BitVec('behaviour_generation_0', 11))]:
            solver = z3.Solver()
            solver.add(data['domain'], stale)
            self.assertEqual(solver.check(), z3.unsat)

    def test_mixed_ps2_images_and_stale_windows_image_fail(self):
        originals = copy.deepcopy(self.originals)
        originals['00103f60']['executable_sha256'] = '0' * 64
        with self.assertRaisesRegex(Unsupported, 'Mixed PS2'):
            compare_case(self.manifest, originals, 'GetBehaviour', count=0)
        with self.assertRaisesRegex(Unsupported, 'compiled image'):
            compare(self.manifest, self.blob + b'\0', self.originals)

    def test_same_state_body_cannot_write_actor_memory(self):
        manifest = copy.deepcopy(self.manifest)
        root = next(e for e in manifest['exports'] if e['name'] == 'SetState')
        root['instructions'][0]['ops'].insert(0, dict(opcode='STORE', inputs=[
            dict(space='const', offset='1b1', size=4, constant=True),
            dict(space='const', offset='100000', size=8, constant=True),
            dict(space='const', offset='0', size=4, constant=True)]))
        root['operation_count'] += 1
        with self.assertRaisesRegex(Unsupported, 'memory access'):
            compare_case(manifest, self.originals, 'SetState', count=0)

    def test_upper_ps2_saved_register_corruption_detected(self):
        originals = copy.deepcopy(self.originals)
        root = originals['00102a00']
        instruction = next(i for i in root['instructions'] if any(op['opcode'] == 'RETURN' for op in i['ops']))
        instruction['ops'].insert(-1, dict(opcode='COPY',
            output=dict(space='register', offset='154', size=4, constant=False),
            inputs=[dict(space='const', offset='0', size=4, constant=True)]))
        root['operation_count'] += 1
        with self.assertRaisesRegex(Unsupported, 'callee-saved register preservation'):
            compare_case(self.manifest, originals, 'SetState', count=0)


if __name__ == '__main__':
    unittest.main()
