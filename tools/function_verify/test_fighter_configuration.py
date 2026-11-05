"""Configuration constructor execution and real PS2 table negative controls."""
import copy
import json
from pathlib import Path
import unittest
from verify import Unsupported
from fighter_configuration import compare


class FighterConfigurationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        fixture = json.loads((Path(__file__).parent / 'fixtures/fighter-configuration.json').read_text())
        cls.manifest, cls.original = fixture['manifest'], fixture['original']
        cls.blob = bytes.fromhex(fixture['blob_hex'])

    def test_complete_initializer_and_108_constructors(self):
        report = compare(self.manifest, self.blob, self.original)
        self.assertEqual(report['result'], 'equivalent')
        self.assertEqual(report['constructor_invocations'], 108)
        self.assertEqual(len(report['entries']), 108)
        self.assertEqual(report['entries'][0], dict(state=6, animation=12, flags='0x1f00900'))
        self.assertEqual(report['entries'][3], dict(state=9, animation=21, flags='0x1f00900'))

    def test_missing_constructor_store_detected(self):
        manifest = copy.deepcopy(self.manifest)
        ctor = next(e for e in manifest['exports'] if e['coff']['symbol'] == '??0StateConfig@@QEAA@HI@Z')
        # The final STORE writes flags into the supplied configuration object.
        instruction, store = [(i, op) for i in ctor['instructions'] for op in i['ops'] if op['opcode'] == 'STORE'][-1]
        instruction['ops'].remove(store)
        ctor['operation_count'] -= 1
        self.assertEqual(compare(manifest, self.blob, self.original)['result'], 'mismatch')

    def test_changed_original_animation_detected(self):
        original = copy.deepcopy(self.original)
        raw = bytearray.fromhex(original['hex'])
        raw[0] ^= 1
        original['hex'] = raw.hex()
        self.assertEqual(compare(self.manifest, self.blob, original)['result'], 'mismatch')

    def test_static_zero_storage_is_not_initialized_configuration(self):
        original = copy.deepcopy(self.original)
        original['hex'] = bytes(864).hex()
        self.assertEqual(compare(self.manifest, self.blob, original)['result'], 'mismatch')

    def test_wrong_table_identity_and_stale_image_fail(self):
        original = copy.deepcopy(self.original)
        original['address'] = '00434468'
        with self.assertRaisesRegex(Unsupported, 'identity'):
            compare(self.manifest, self.blob, original)
        with self.assertRaisesRegex(Unsupported, 'compiled image'):
            compare(self.manifest, self.blob + b'\0', self.original)

    def test_conditionals_are_never_silently_followed(self):
        manifest = copy.deepcopy(self.manifest)
        root = next(e for e in manifest['exports'] if e['coff']['symbol'].startswith('??__E'))
        first = root['instructions'][0]
        first['ops'].append(dict(opcode='CBRANCH', inputs=[
            dict(space='ram', offset=root['instructions'][1]['address'], size=8, constant=False),
            dict(space='const', offset='1', size=1, constant=True)]))
        root['operation_count'] += 1
        with self.assertRaisesRegex(Unsupported, 'unconditional'):
            compare(manifest, self.blob, self.original)


if __name__ == '__main__':
    unittest.main()
