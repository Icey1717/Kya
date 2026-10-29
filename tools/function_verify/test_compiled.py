import copy
import json
from pathlib import Path
import struct
import unittest
from unittest.mock import patch
import verify
import compiled
import compare_binaries


class BinaryComparisonTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixture = json.loads((Path(__file__).parent / 'fixtures/compiled.json').read_text())

    def test_four_production_functions(self):
        for pc in self.fixture['exports']:
            with self.subTest(name=pc['name']):
                ps2 = self.fixture['ps2'][pc['coff']['ps2_entry']]
                self.assertEqual(compare_binaries.compare(pc, ps2, self.fixture['layout'])['result'], 'equivalent')

    def test_actual_compiler_mutation_detected(self):
        report = compare_binaries.compare(self.fixture['mutant'], self.fixture['ps2']['00100460'], self.fixture['layout'])
        self.assertEqual(report['result'], 'mismatch')
        self.assertNotEqual(report['counterexample']['pc_return'], report['counterexample']['ps2_return'])

    def test_wrong_layout_detected(self):
        pc = self.fixture['exports'][1]
        self.assertEqual(compare_binaries.compare(pc, self.fixture['ps2']['001003f0'], self.fixture['layout'] + 4)['result'], 'mismatch')

    def test_unknown_instruction_fails_closed(self):
        pc = copy.deepcopy(self.fixture['exports'][0])
        pc['instructions'][0]['ops'][0]['opcode'] = 'CALLOTHER'
        with self.assertRaises(verify.Unsupported):
            compare_binaries.compare(pc, self.fixture['ps2']['00100460'], self.fixture['layout'])

    def test_unbalanced_windows_stack_rejected(self):
        pc = copy.deepcopy(self.fixture['exports'][0])
        # Corrupt RET's stack increment, after its return-address LOAD.
        pc['instructions'][-1]['ops'][1]['inputs'][1]['offset'] = '10'
        with self.assertRaisesRegex(verify.Unsupported, 'preservation failed'):
            compare_binaries.compare(pc, self.fixture['ps2']['00100460'], self.fixture['layout'])

    def test_missing_windows_return_rejected(self):
        pc = copy.deepcopy(self.fixture['exports'][0])
        pc['instructions'][-1]['ops'].pop()
        pc['operation_count'] -= 1
        with self.assertRaises(verify.Unsupported):
            compare_binaries.compare(pc, self.fixture['ps2']['00100460'], self.fixture['layout'])


class CoffTests(unittest.TestCase):
    def object(self, relocation=False):
        name = b'function\0'
        raw = b'\x90\x90\xc3\xcc'
        reloc = struct.pack('<IIH', 1, 0, 4) if relocation else b''
        symoff = 64 + len(reloc)
        header = struct.pack('<HHIIIHH', 0x8664, 1, 0, symoff, 1, 0, 0)
        section = struct.pack('<8sIIIIIIHHI', b'.text\0\0\0', 0, 0, 4, 60, 64, 0, int(relocation), 0, 0x60000020)
        symbol = struct.pack('<8sIhHBB', b'\0\0\0\0\x04\0\0\0', 0, 1, 0x20, 2, 0)
        return header + section + raw + reloc + symbol + struct.pack('<I', 4 + len(name)) + name

    def test_extract_exact_symbol_bytes(self):
        with patch.object(compiled, 'SYMBOLS', [('1', 'f', 'function')]):
            blob, items = compiled.extract(self.object())
        self.assertEqual(blob, b'\x90\x90\xc3\xcc')
        self.assertEqual(items[0]['coff_section'], 1)

    def test_relocation_rejected(self):
        with patch.object(compiled, 'SYMBOLS', [('1', 'f', 'function')]):
            with self.assertRaisesRegex(ValueError, 'Relocation'):
                compiled.extract(self.object(True))

    def test_missing_symbol_rejected(self):
        with self.assertRaisesRegex(ValueError, 'Missing'):
            compiled.extract(self.object())


if __name__ == '__main__':
    unittest.main()
