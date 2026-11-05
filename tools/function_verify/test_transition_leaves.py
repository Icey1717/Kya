"""Real compiled audio/animation leaves and adversarial dispatch evidence."""
import copy
import json
from pathlib import Path
import struct
import unittest
from combat_cases import compare
from fighter_dispatch import slots
from verify import Unsupported
from call_graph import link
from paths import expand


class TransitionLeavesTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixture = json.loads((Path(__file__).parent / 'fixtures/transition-leaves.json').read_text())

    def test_compiled_audio_and_bit_exact_animation_store(self):
        for name in ('AudioEnter', 'AudioLeave', 'LayerTimeWarper', 'StandAnimNoDispatch'):
            with self.subTest(name=name):
                self.assertEqual(compare(self.fixture['positive'], name, self.fixture['originals'])['result'], 'equivalent')

    def test_missing_audio_counter_write_detected(self):
        manifest = copy.deepcopy(self.fixture['positive'])
        target = next(e for e in manifest['exports'] if e['name'] == 'AudioEnter')
        # Mutate the final non-stack STORE, retaining the actual address calculation.
        stores = [(i, op) for i in target['instructions'] for op in i['ops'] if op['opcode'] == 'STORE']
        instruction, op = stores[-1]
        instruction['ops'].remove(op)
        target['operation_count'] -= 1
        self.assertEqual(compare(manifest, 'AudioEnter', self.fixture['originals'])['result'], 'mismatch')

    def test_zeroing_float_store_detected(self):
        originals = copy.deepcopy(self.fixture['originals'])
        instruction = next(i for i in originals['002425a0']['instructions'] if i['address'] == '00242604')
        store = next(o for o in instruction['ops'] if o['opcode'] == 'STORE')
        store['inputs'][2] = dict(space='const', offset='0', size=4, constant=True)
        result = compare(self.fixture['positive'], 'LayerTimeWarper', originals)
        self.assertEqual(result['result'], 'mismatch')
        self.assertNotEqual(int(result['counterexample']['play_rate_bits'], 16), 0)

    def test_unknown_float_operation_rejected(self):
        originals = copy.deepcopy(self.fixture['originals'])
        instruction = next(i for i in originals['002425a0']['instructions'] if i['address'] == '00242604')
        instruction['ops'][-1]['opcode'] = 'FLOAT_UNSUPPORTED'
        with self.assertRaises(Unsupported):
            compare(self.fixture['positive'], 'LayerTimeWarper', originals)

    def test_unresolved_dispatch_frontier_is_retained(self):
        for export in (next(e for e in self.fixture['positive']['exports'] if e['name'] == 'StandAnimNoDispatch'),
                       self.fixture['originals']['0031ba50']):
            with self.subTest(language=export['language']):
                graph = link([export], int(export['entry'], 16), defer_unresolved=True)
                paths = expand(graph)
                self.assertEqual(sum(p['instructions'][-1]['ops'][-1]['opcode'] == 'UNRESOLVED_TRANSFER' for p in paths), 1)
                with self.assertRaisesRegex(Unsupported, 'Synthetic opcode'):
                    link([graph], int(graph['entry'], 16))

    def test_reachable_unresolved_transfer_fails_on_both_executors(self):
        import z3
        from verify import execute as ps2_execute
        from windows_pcode import execute as pc_execute
        for language, execute, bits in [('r5900:LE:32:default', ps2_execute, 32), ('x86:LE:64:default', pc_execute, 64)]:
            export = dict(schema='kya.raw-pcode.v1', language=language, compiler='windows' if bits == 64 else 'default',
                          big_endian=False, complete=True, entry='100', operation_count=1,
                          instructions=[dict(address='100', text='unresolved', ops=[dict(opcode='UNRESOLVED_TRANSFER', inputs=[], reason='missing body')])])
            memory = z3.Array('frontier_memory', z3.BitVecSort(bits), z3.BitVecSort(8))
            with self.subTest(language=language), self.assertRaisesRegex(Unsupported, 'missing body'):
                execute(export, z3.BitVecVal(0, 64), memory, path_mode=True)


class VtableTests(unittest.TestCase):
    def object(self, kind=1, addend=0, missing=False):
        # One 8-byte RTTI prefix and one function pointer in a COFF section.
        strings = b'table\0target\0'
        data = bytearray(20 + 40 + 16 + 10 + 36 + 4 + len(strings))
        struct.pack_into('<HHIIIHH', data, 0, 0x8664, 1, 0, 86, 2, 0, 0)
        struct.pack_into('<III', data, 36, 16, 60, 76)
        struct.pack_into('<H', data, 52, 0 if missing else 1)
        struct.pack_into('<Q', data, 68, addend)
        struct.pack_into('<IIH', data, 76, 8, 1, kind)
        struct.pack_into('<8sIhHBB', data, 86, b'table', 8, 1, 0, 2, 0)
        struct.pack_into('<8sIhHBB', data, 104, b'target', 0, 0, 0x20, 2, 0)
        struct.pack_into('<I', data, 122, len(strings) + 4)
        data[126:] = strings
        return bytes(data)

    def test_slot_derived_from_addr64_relocation(self):
        self.assertEqual(slots(self.object(), 'table'), {0: 'target'})

    def test_unmodeled_relocations_and_adjustments_rejected(self):
        for args in (dict(kind=4), dict(addend=8), dict(missing=True)):
            with self.subTest(args=args), self.assertRaises(ValueError):
                slots(self.object(**args), 'table')


if __name__ == '__main__':
    unittest.main()
