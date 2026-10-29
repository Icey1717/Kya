import copy
import json
from pathlib import Path
import tempfile
import unittest
import verify
import memory_cases
import branch_cases
from paths import expand
import indexed_cases
import call_cases
import loop_cases
import alias_cases


def node(space, offset, size=8):
    return dict(space=space, offset=hex(offset)[2:], size=size, constant=space == 'const')


def fixture():
    return dict(schema='kya.raw-pcode.v1', language='r5900:LE:32:default',
                big_endian=False, complete=True, operation_count=3, instructions=[dict(ops=[
                    dict(opcode='INT_AND', output=node('register', 0x20),
                         inputs=[node('register', 0x50), node('const', 1)]),
                    dict(opcode='RETURN', inputs=[node('register', 0x1f0)])])])


class VerificationTests(unittest.TestCase):
    def setUp(self):
        self.export = fixture()
        self.export['operation_count'] = 2

    def test_equivalence_and_counterexample(self):
        self.assertEqual(verify.compare(self.export, 1)['result'], 'equivalent')
        mismatch = verify.compare(self.export, 0)
        self.assertEqual(mismatch['result'], 'mismatch')
        self.assertEqual(int(mismatch['counterexample']['kind'], 16) & 1, 1)

    def test_unsupported_effects(self):
        for opcode in ['STORE', 'LOAD', 'CALL', 'CALLOTHER', 'CBRANCH', 'BRANCH', 'FLOAT_ADD']:
            with self.subTest(opcode=opcode):
                export = copy.deepcopy(self.export)
                export['instructions'][0]['ops'][0]['opcode'] = opcode
                with self.assertRaises(verify.Unsupported):
                    verify.compare(export, 1)

    def test_incomplete_and_wrong_count(self):
        for key, value in [('complete', False), ('operation_count', 3), ('big_endian', True)]:
            export = copy.deepcopy(self.export)
            export[key] = value
            with self.assertRaises(verify.Unsupported):
                verify.compare(export, 1)

    def test_uninitialized_input(self):
        self.export['instructions'][0]['ops'][0]['inputs'][0] = node('register', 0x60)
        with self.assertRaises(verify.Unsupported):
            verify.compare(self.export, 1)

    def test_missing_return(self):
        self.export['instructions'][0]['ops'].pop()
        self.export['operation_count'] = 1
        with self.assertRaises(verify.Unsupported):
            verify.compare(self.export, 1)

    def test_wrong_return_target(self):
        self.export['instructions'][0]['ops'][-1]['inputs'][0] = node('register', 0x50)
        with self.assertRaises(verify.Unsupported):
            verify.compare(self.export, 1)

    def test_changed_source_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'source.cpp'
            path.write_text('bool CActor::IsKindOfObject(ulong kind) { return (kind & 1) != 0; }')
            self.assertEqual(verify.source_mask(path, 'CActor')[0], 1)
            path.write_text('bool CActor::IsKindOfObject(ulong kind) { kind++; return (kind & 1) != 0; }')
            with self.assertRaises(verify.Unsupported):
                verify.source_mask(path, 'CActor')


class MemoryTests(unittest.TestCase):
    def load(self, address):
        return json.loads((Path(__file__).parent / 'fixtures' / (address + '.json')).read_text())

    def test_live_snapshot_getter_and_setters(self):
        for address, operator, mask, setter in [('001003f0', '&', 0x2000, False),
                                                 ('00100400', '|', 0x2000, True),
                                                 ('00100410', '&', 0xffffdfff, True)]:
            with self.subTest(address=address):
                export = self.load(address)
                self.assertEqual(memory_cases.compare(export, operator, mask, setter)['result'], 'equivalent')
                self.assertEqual(memory_cases.compare(export, operator, mask ^ 1, setter)['result'], 'mismatch')
                self.assertEqual(memory_cases.compare(export, operator, mask, setter, 12)['result'], 'mismatch')

    def test_removed_store_detected(self):
        export = self.load('00100400')
        for instruction in export['instructions']:
            instruction['ops'] = [op for op in instruction['ops'] if op['opcode'] != 'STORE']
        export['operation_count'] -= 1
        self.assertEqual(memory_cases.compare(export, '|', 0x2000, True)['result'], 'mismatch')

    def test_wrong_memory_space_rejected(self):
        export = self.load('001003f0')
        for instruction in export['instructions']:
            for op in instruction['ops']:
                if op['opcode'] == 'LOAD':
                    op['inputs'][0]['offset'] = 'dead'
        with self.assertRaises(verify.Unsupported):
            memory_cases.compare(export, '&', 0x2000, False)

    def test_out_of_bounds_load_rejected(self):
        export = self.load('001003f0')
        export['instructions'][0]['ops'][0]['inputs'][1]['offset'] = '10'
        with self.assertRaises(verify.Unsupported):
            memory_cases.compare(export, '&', 0x2000, False)

    def test_register_subrange_read(self):
        # SetLookingAtOff performs signed 32->64 extension, then stores the
        # low 32 bits. The live snapshot checks alias extraction and sign bits.
        result = memory_cases.compare(self.load('00100410'), '&', 0xffffdfff, True)
        self.assertEqual(result['result'], 'equivalent')


class BranchTests(unittest.TestCase):
    def load(self, index=0):
        case = branch_cases.CASES[index]
        return json.loads((Path(__file__).parent / 'fixtures' / (case['address'] + '.json')).read_text())

    def test_live_paths_and_negative_controls(self):
        for index, case in enumerate(branch_cases.CASES):
            export = self.load(index)
            report = branch_cases.compare(export, case)
            self.assertEqual(report['result'], 'equivalent')
            self.assertEqual(report['paths'], index + 2)
            self.assertEqual(branch_cases.compare(export, case, mutation=True)['result'], 'mismatch')

    def test_path_limit_is_inconclusive(self):
        with self.assertRaises(verify.Unsupported):
            branch_cases.compare(self.load(), branch_cases.CASES[0], max_paths=1)

    def test_cycle_is_inconclusive(self):
        export = self.load()
        for instruction in export['instructions']:
            for op in instruction['ops']:
                if op['opcode'] == 'CBRANCH':
                    op['inputs'][0]['offset'] = export['entry']
        with self.assertRaises(verify.Unsupported):
            expand(export)

    def test_external_target_is_inconclusive(self):
        export = self.load()
        for instruction in export['instructions']:
            for op in instruction['ops']:
                if op['opcode'] == 'CBRANCH':
                    op['inputs'][0]['offset'] = 'deadbeef'
        with self.assertRaises(verify.Unsupported):
            expand(export)

    def test_instruction_bound_is_inconclusive(self):
        with self.assertRaises(verify.Unsupported):
            expand(self.load(), max_instructions=1)


class IndexedTests(unittest.TestCase):
    def load(self):
        return json.loads((Path(__file__).parent / 'fixtures/001022b0.json').read_text())

    def test_symbolic_table_equivalence(self):
        self.assertEqual(indexed_cases.compare(self.load())['result'], 'equivalent')

    def test_wrong_sentinel_detected(self):
        report = indexed_cases.compare(self.load(), mutation=True)
        self.assertEqual(report['result'], 'mismatch')
        self.assertEqual(report['counterexample']['state'], -1)

    def test_unrestricted_index_rejected(self):
        with self.assertRaisesRegex(verify.Unsupported, 'bounds not proven'):
            indexed_cases.compare(self.load(), unrestricted=True)

    def test_wrong_stride_detected(self):
        export = self.load()
        for instruction in export['instructions']:
            if instruction['text'].startswith('sll v1'):
                instruction['ops'][0]['inputs'][1]['offset'] = '1'
        self.assertEqual(indexed_cases.compare(export)['result'], 'mismatch')

    def test_out_of_bounds_stride_rejected(self):
        export = self.load()
        for instruction in export['instructions']:
            if instruction['text'].startswith('sll v1'):
                instruction['ops'][0]['inputs'][1]['offset'] = '3'
        with self.assertRaisesRegex(verify.Unsupported, 'bounds not proven'):
            indexed_cases.compare(export)


class CallTests(unittest.TestCase):
    def load(self):
        return [json.loads((Path(__file__).parent / 'fixtures' / (entry + '.json')).read_text())
                for entry in call_cases.ENTRIES]

    def test_call_graph_and_wrong_table(self):
        exports = self.load()
        result = call_cases.compare(exports)
        self.assertEqual(result['result'], 'equivalent')
        self.assertEqual(result['paths'], 3)
        self.assertEqual(call_cases.compare(exports, mutation=True)['result'], 'mismatch')

    def test_unknown_call_rejected(self):
        exports = self.load()
        for instruction in exports[0]['instructions']:
            for op in instruction['ops']:
                if op['opcode'] == 'CALL':
                    op['inputs'][0]['offset'] = 'dead'
        with self.assertRaises(verify.Unsupported):
            call_cases.compare(exports)

    def test_broken_stack_restore_rejected(self):
        exports = self.load()
        # Return delay slot restores sp by 16; changing it to 8 must fail.
        exports[0]['instructions'][-1]['ops'][0]['inputs'][1]['offset'] = '8'
        with self.assertRaisesRegex(verify.Unsupported, 'Stack or return address'):
            call_cases.compare(exports)

    def test_broken_return_link_rejected(self):
        exports = self.load()
        for instruction in exports[0]['instructions']:
            for op in instruction['ops']:
                if op.get('output', {}).get('offset') == '1f0' and op['opcode'] == 'COPY':
                    op['inputs'][0]['offset'] = '123c84'
        with self.assertRaisesRegex(verify.Unsupported, 'return target'):
            call_cases.compare(exports)

    def test_mixed_executables_rejected(self):
        exports = self.load()
        exports[1]['executable_sha256'] = 'different'
        with self.assertRaises(verify.Unsupported):
            call_cases.compare(exports)


class LoopTests(unittest.TestCase):
    def load(self):
        return json.loads((Path(__file__).parent / 'fixtures/00131f10.json').read_text())

    def test_loop_matches_with_proven_cutoff(self):
        result = loop_cases.compare(self.load())
        self.assertEqual(result['result'], 'equivalent')
        self.assertEqual(result['cutoff_paths'], 1)
        self.assertEqual(result['cutoff_proof'], 'unreachable for all valid inputs')

    def test_last_entry_error_detected(self):
        result = loop_cases.compare(self.load(), mutation=True)
        self.assertEqual(result['result'], 'mismatch')
        self.assertEqual(result['counterexample']['count'], 4)

    def test_insufficient_bound_rejected(self):
        with self.assertRaisesRegex(verify.Unsupported, 'Loop bound insufficient'):
            loop_cases.compare(self.load(), visits=3)

    def test_loops_still_rejected_by_default(self):
        with self.assertRaisesRegex(verify.Unsupported, 'Cycle detected'):
            expand(self.load())

    def test_loop_path_limit_rejected(self):
        with self.assertRaisesRegex(verify.Unsupported, 'Path count limit'):
            expand(self.load(), loop_visits=4, max_paths=2)

    def test_invalid_loop_bound_rejected(self):
        with self.assertRaises(verify.Unsupported):
            expand(self.load(), loop_visits=0)


class AliasingTests(unittest.TestCase):
    def load(self, address):
        return json.loads((Path(__file__).parent / 'fixtures' / (address + '.json')).read_text())

    def test_equal_and_distinct_indices(self):
        for address, _ in alias_cases.CASES:
            for equal in (True, False):
                with self.subTest(address=address, equal=equal):
                    self.assertEqual(alias_cases.compare(self.load(address), equal=equal)['result'], 'equivalent')

    def test_lost_temporary_detected(self):
        for address, _ in alias_cases.CASES:
            result = alias_cases.compare(self.load(address), mutation=True)
            self.assertEqual(result['result'], 'mismatch')
            self.assertNotEqual(result['counterexample']['a'], result['counterexample']['b'])

    def test_equal_index_mutation_is_noop(self):
        result = alias_cases.compare(self.load('002000f0'), mutation=True, equal=True)
        self.assertEqual(result['result'], 'equivalent')

    def test_omitted_store_detected(self):
        export = self.load('002000f0')
        for instruction in export['instructions']:
            stores = [op for op in instruction['ops'] if op['opcode'] == 'STORE']
            if stores:
                instruction['ops'].remove(stores[0])
                export['operation_count'] -= 1
                break
        self.assertEqual(alias_cases.compare(export)['result'], 'mismatch')


if __name__ == '__main__':
    unittest.main()
