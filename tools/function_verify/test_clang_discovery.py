"""Exercise compiler discovery on actual C++ syntax rejected by Tree-sitter."""
import os
from pathlib import Path
import tempfile
import unittest

from build_config import ROOT
from clang_discovery import Clang, parse_arguments, reconcile, recover_inventory
from small_functions import signature
from verify import Unsupported


DLL = Path('C:/Program Files/Microsoft Visual Studio/18/Insiders/VC/Tools/Llvm/x64/bin/libclang.dll')


@unittest.skipUnless(os.name == 'nt' and DLL.exists(), 'Requires the Windows libclang installation')
class ClangDiscoveryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.clang = Clang(DLL)

    def setUp(self):
        (ROOT / 'out').mkdir(exist_ok=True)
        self.folder = tempfile.TemporaryDirectory(prefix='clang-discovery-', dir=ROOT / 'out')
        self.addCleanup(self.folder.cleanup)
        self.source = Path(self.folder.name) / 'source with spaces.cpp'

    def test_real_compiler_handles_comma_new_pointer_array_and_macros(self):
        self.source.write_text('''// Unicode prefix: \u03b1\n
#define PACK(x) x
PACK(struct Packed { int x; });
namespace N {
struct Foo {
    int value;
    void Assign(int x) { if (x && (value = x, value != 0)) {} }
    void Allocate(int n) { int** p = new int*[n](); delete[] p; }
    int Overload(int x) { return x; }
    int Overload() { return value; }
};
extern "C" { int Native(int x) { return x; } }
template<typename T> struct Box { bool Test(T value); };
template<typename T> bool Box<T>::Test(T value) { return value != 0; }
int UseTemplate() { Box<int> box; return box.Test(1); }
#ifdef PLATFORM_WIN
int Platform() { return 1; }
#else
int Inactive() { return 2; }
#endif
}
''', encoding='utf-8')
        rows, errors, examined = self.clang.functions(self.source, ['-x', 'c++', '-std=c++17', '-DPLATFORM_WIN'], {self.source})
        self.assertEqual(errors, [])
        self.assertIn(str(self.source), examined)
        names = [row['name'] for row in rows]
        self.assertIn('N::Foo::Assign', names)
        self.assertIn('N::Foo::Allocate', names)
        self.assertIn('N::Native', names)
        self.assertIn('N::Platform', names)
        self.assertIn('N::Box<T>::Test', names)
        self.assertNotIn('N::Inactive', names)
        self.assertNotIn('PACK', names)
        overloads = [row for row in rows if row['name'] == 'N::Foo::Overload']
        self.assertEqual(len(overloads), 2)
        self.assertEqual(len({row['mangled'] for row in overloads}), 2)
        raw = self.source.read_bytes()
        for row in rows:
            self.assertEqual(row['source'].encode('utf-8'), raw[row['start_byte']:row['end_byte']])
            self.assertTrue(row['source'].rstrip().endswith('}'))

    def test_compiler_errors_never_supply_partial_definitions(self):
        self.source.write_text('int Good() { return 1; }\nint Bad() { return unknown; }')
        rows, errors, examined = self.clang.functions(self.source, ['-x', 'c++'], {self.source})
        self.assertEqual(rows, [])
        self.assertTrue(errors)
        self.assertEqual(examined, [])

    def test_legacy_comment_bytes_keep_exact_offsets_and_provenance(self):
        self.source.write_bytes(b'int Legacy() { /* 2\xb2 */ return 1; }')
        rows, errors, examined = self.clang.functions(self.source, ['-x', 'c++'], {self.source})
        self.assertEqual(errors, [])
        self.assertEqual(len(rows), 1)
        self.assertFalse(rows[0]['source_utf8_valid'])
        self.assertEqual(bytes.fromhex(rows[0]['source_bytes_hex']), self.source.read_bytes())

    def test_windows_command_preserves_flags_and_removes_case_variant_source(self):
        command = ('"C:/Program Files/LLVM/clang.exe" -DPLATFORM_WIN -I"C:/includes with spaces" '
                   '-MD -MT target.obj -MF target.d -o target.obj -c "' + self.source.as_posix().upper() + '"')
        compiler, args = parse_arguments(command, self.source)
        self.assertEqual(compiler, 'C:/Program Files/LLVM/clang.exe')
        self.assertEqual(args, ['-DPLATFORM_WIN', '-IC:/includes with spaces'])

    def inventory(self, row, name=None):
        return dict(id='test', path=row['path'], name=name or row['name'], line=row['line'],
                    lines=row['lines'], source=row['source'], body=row['source'].split('{', 1)[1], parseError=True)

    def recover(self, fn, rows, types=None, errors=None):
        units = [dict(errors=errors or [], examined_files=[str(self.source)])]
        for row in rows:
            row['translation_unit'] = 0
        review = reconcile(fn, rows, types or [], units)
        report = dict(functions=[review], translation_units=units, build={}, clang_version='test')
        return recover_inventory(dict(functions=[fn]), report), review

    def test_recovery_reaches_signature_gate_and_rejects_stale_byte_range(self):
        self.source.write_text('int F(int x) { if (x && (x = 2, x != 0)) {} return x; }')
        rows, errors, _ = self.clang.functions(self.source, ['-x', 'c++'], {self.source})
        self.assertEqual(errors, [])
        fn = self.inventory(rows[0])
        recovered, review = self.recover(fn, rows)
        self.assertEqual(review['result'], 'recovered')
        accepted, = recovered['functions']
        self.assertTrue(accepted['parseError'])
        self.assertEqual(signature(accepted)['arguments'], ['int'])
        self.source.write_text('// Shift the source range\n' + fn['source'])
        with self.assertRaisesRegex(Unsupported, 'range changed'):
            signature(accepted)

    def test_normal_overloads_gain_distinct_exact_compiler_symbols(self):
        self.source.write_text('int F() { return 1; }\nint F(int x) { return x; }')
        rows, errors, _ = self.clang.functions(self.source, ['-x', 'c++'], {self.source})
        self.assertEqual(errors, [])
        recovered = []
        for row in rows:
            fn = dict(self.inventory(row), parseError=False)
            accepted, review = self.recover(fn, rows)
            self.assertEqual(review['result'], 'recovered')
            recovered.append(accepted['functions'][0])
        self.assertNotEqual(recovered[0]['compiler_definition']['mangled'],
                            recovered[1]['compiler_definition']['mangled'])
        self.assertFalse(recovered[0]['parseError'])
        self.assertEqual(signature(recovered[1])['arguments'], ['int'])

    def test_template_owner_reconciles_but_remains_outside_signature_subset(self):
        self.source.write_text('template<typename T> struct Box { int F() { return 1; } };\n'
                               'int Use() { Box<int> box; return box.F(); }')
        rows, errors, _ = self.clang.functions(self.source, ['-x', 'c++'], {self.source})
        self.assertEqual(errors, [])
        recovered, review = self.recover(self.inventory(rows[0], 'Box::F'), rows)
        self.assertEqual(review['result'], 'recovered')
        accepted, = recovered['functions']
        self.assertEqual(accepted['name'], 'Box<T>::F')
        with self.assertRaisesRegex(Unsupported, 'non-template'):
            signature(accepted)

    def test_declaration_exclusion_requires_positive_compiler_type_evidence(self):
        self.source.write_text('#define PACK(x) x\nPACK(\nstruct Packed { int x; });')
        types = []
        rows, errors, _ = self.clang.functions(self.source, ['-x', 'c++'], {self.source}, types)
        self.assertEqual(errors, [])
        fn = dict(id='pack', path=self.source.relative_to(ROOT).as_posix(), name='PACK', line=2,
                  lines=2, source='PACK(\nstruct Packed { int x; })', parseError=True)
        recovered, review = self.recover(fn, rows, types)
        self.assertEqual(review['result'], 'not-function')
        self.assertEqual(recovered['functions'], [])
        _, review = self.recover(fn, rows)
        self.assertNotEqual(review['result'], 'not-function')

    def test_template_declaration_range_is_preserved_when_tree_sitter_omits_it(self):
        self.source.write_text('template<typename T> struct Box { int F(); };\n'
                               'template<typename T>\nint Box<T>::F() { return 1; }\n'
                               'int Use() { Box<int> box; return box.F(); }')
        rows, errors, _ = self.clang.functions(self.source, ['-x', 'c++'], {self.source})
        self.assertEqual(errors, [])
        row = next(row for row in rows if row['name'] == 'Box<T>::F')
        fn = self.inventory(row)
        fn.update(source=fn['source'].split('\n', 1)[1], line=fn['line'] + 1, lines=fn['lines'] - 1)
        recovered, review = self.recover(fn, [row])
        self.assertEqual(review['result'], 'recovered')
        self.assertEqual(recovered['functions'][0]['line'], row['line'])
        self.assertEqual(recovered['functions'][0]['source'], row['source'])

    def test_source_or_range_mismatch_and_ambiguous_overloads_never_recover(self):
        self.source.write_text('int F() { return 1; }')
        rows, _, _ = self.clang.functions(self.source, ['-x', 'c++'], {self.source})
        fn = self.inventory(rows[0])
        _, review = self.recover(fn, rows + rows)
        self.assertEqual(review['result'], 'ambiguous')
        for key, value in [('line', 2), ('source', 'int F() { return 2; }')]:
            _, review = self.recover(dict(fn, **{key: value}), rows)
            self.assertNotEqual(review['result'], 'recovered')

    def test_header_definition_requires_a_successful_consuming_translation_unit(self):
        header = self.source.with_suffix('.h')
        header.write_text('inline int F(int x) { return (x = 2, x); }')
        self.source.write_text('#include "' + header.as_posix() + '"\n')
        rows, errors, _ = self.clang.functions(self.source, ['-x', 'c++'], {header})
        self.assertEqual(errors, [])
        recovered, _ = self.recover(self.inventory(rows[0]), rows)
        self.assertEqual(signature(recovered['functions'][0])['return_type'], 'int')
        with self.assertRaisesRegex(ValueError, 'Compiler errors'):
            self.recover(self.inventory(rows[0]), rows, errors=['Invalid consuming translation unit'])
        self.source.write_text(self.source.read_text() + 'int Bad() { return unknown; }')
        rows, errors, _ = self.clang.functions(self.source, ['-x', 'c++'], {header})
        self.assertEqual(rows, [])
        self.assertTrue(errors)
