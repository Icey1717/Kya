from pathlib import Path
import unittest

from symbol_index import BuildIndex, LoadIndex
from transform import transform
from vector_assignments import CollapseVectorAssignments


class VectorAssignmentTests(unittest.TestCase):
    def setUp(self):
        self.index = BuildIndex(['''
            struct Sound { edF32VECTOR3 position; edF32VECTOR3 rotation; };
            PACK(struct Properties { edF32VECTOR3 offset; });
            class Base { Sound sound; };
            class Platform : public Base { Properties* properties; };
        '''])

    def componentCopy(self, lhs='dst', rhs='src', components='xyz', scalar=''):
        return '\n'.join(f'{lhs}.{c} = {rhs}.{c}{scalar};' for c in components)

    def test_all_vector_type_combinations(self):
        for lhsType in (3, 4):
            for rhsType in (3, 4):
                with self.subTest(lhsType=lhsType, rhsType=rhsType):
                    declarations = f'edF32VECTOR{lhsType} dst;\nedF32VECTOR{rhsType} src;\n'
                    lhs = 'dst.xyz' if lhsType == 4 else 'dst'
                    rhs = 'src.xyz' if rhsType == 4 else 'src'
                    source = declarations + self.componentCopy()
                    expected = declarations + f'{lhs} = {rhs};'
                    self.assertEqual(CollapseVectorAssignments(source, self.index), expected)

    def test_four_component_copy_and_different_w(self):
        declarations = 'edF32VECTOR4 dst;\nedF32VECTOR4 src;\n'
        source = declarations + self.componentCopy(components='xyzw')
        self.assertEqual(CollapseVectorAssignments(source, self.index), declarations + 'dst = src;')
        source = declarations + self.componentCopy() + '\ndst.w = 1.0f;'
        self.assertEqual(CollapseVectorAssignments(source, self.index),
                         declarations + 'dst.xyz = src.xyz;\ndst.w = 1.0f;')

    def test_scalar_multiply(self):
        for dimension in (3, 4):
            declarations = f'edF32VECTOR{dimension} dst;\nfloat scale;\n'
            source = declarations + self.componentCopy(rhs='dst', scalar=' * scale')
            view = 'dst.xyz' if dimension == 4 else 'dst'
            self.assertEqual(CollapseVectorAssignments(source, self.index),
                             declarations + f'{view} = {view} * scale;')

    def test_inherited_members_and_packed_structs(self):
        declarations = 'edF32VECTOR4 local;\nProperties* props;\n'
        copies = self.componentCopy('local', '(props->offset)') + '\n'
        copies += self.componentCopy('this->sound.position', 'local')
        source = 'void Platform::Run()\n{\n' + declarations + copies + '\n}'
        expected = 'void Platform::Run()\n{\n' + declarations
        expected += 'local.xyz = props->offset;\nthis->sound.position = local.xyz;\n}'
        self.assertEqual(CollapseVectorAssignments(source, self.index), expected)

    def test_parameter_types_and_function_isolation(self):
        for dimension in (3, 4):
            source = f'void Copy(edF32VECTOR{dimension} dst, edF32VECTOR{dimension} src)\n{{\n'
            source += self.componentCopy() + '\n}\n'
            view = '.xyz' if dimension == 4 else ''
            expected = source[:source.index('dst.x =')] + f'dst{view} = src{view};\n}}\n'
            if dimension == 3:
                first, firstExpected = source, expected
            else:
                self.assertEqual(CollapseVectorAssignments(first + source, self.index), firstExpected + expected)

    def test_unknown_types_calls_and_mismatches_stay_explicit(self):
        cases = [
            self.componentCopy(),
            'edF32VECTOR4 dst;\n' + self.componentCopy(rhs='Next()->value'),
            'edF32VECTOR4 dst;\nedF32VECTOR4 src;\n' + self.componentCopy().replace('src.y', 'other.y'),
            'edF32VECTOR4 dst;\nedF32VECTOR4 src;\n' + self.componentCopy().replace('\ndst.y', '\n// comment\ndst.y'),
            'edF32VECTOR4 dst;\nedF32VECTOR4 src;\n' + self.componentCopy().replace('\ndst.y', '\nTouch();\ndst.y'),
            'edF32VECTOR4 dst;\nint scale;\n' + self.componentCopy(rhs='dst', scalar=' * scale'),
            'volatile edF32VECTOR4 dst;\nedF32VECTOR4 src;\n' + self.componentCopy(),
        ]
        for source in cases:
            with self.subTest(source=source):
                self.assertEqual(CollapseVectorAssignments(source, self.index), source)

    def test_conflicting_member_types_prevent_collapse(self):
        index = BuildIndex(['struct Object { edF32VECTOR3 value; };',
                            'struct Object { edF32VECTOR4 value; };'])
        source = 'Object* obj;\nedF32VECTOR4 dst;\n' + self.componentCopy(rhs='obj->value')
        self.assertEqual(CollapseVectorAssignments(source, index), source)

    def test_copies_through_dead_scalar_temporaries(self):
        declarations = 'edF32VECTOR4 dst;\nedF32VECTOR4 src;\nfloat a;\nfloat b;\nfloat c;\n'
        copies = ('b = src.z;\na = src.y;\nc = src.w;\n'
                  'dst.x = src.x;\ndst.y = a;\ndst.z = b;\ndst.w = c;')
        expected = 'edF32VECTOR4 dst;\nedF32VECTOR4 src;\ndst = src;'
        self.assertEqual(CollapseVectorAssignments(declarations + copies, self.index), expected)
        self.assertEqual(CollapseVectorAssignments(declarations + copies + '\n' + copies, self.index),
                         expected + '\ndst = src;')

    def test_live_escaped_or_mismatched_temporaries_stay_explicit(self):
        declarations = 'edF32VECTOR4 dst;\nedF32VECTOR4 src;\nfloat a;\nfloat b;\nfloat c;\n'
        copies = ('a = src.y;\nb = src.z;\nc = src.w;\n'
                  'dst.x = src.x;\ndst.y = a;\ndst.z = b;\ndst.w = c;')
        cases = [
            declarations + copies + '\nUse(a);',
            declarations + 'Escape(&a);\n' + copies,
            declarations + 'BindReference(a);\n' + copies,
            declarations + copies.replace('dst.y = a;', 'dst.y = b;'),
            declarations + copies.replace('b = src.z;', 'b = other.z;'),
            declarations + copies.replace('dst.x = src.x;', 'Touch();\ndst.x = src.x;'),
            declarations + copies.replace('dst.w = c;', 'dst.w = 1.0f;'),
            declarations.replace('float a;', 'volatile float a;') + copies,
        ]
        for source in cases:
            with self.subTest(source=source):
                self.assertEqual(CollapseVectorAssignments(source, self.index), source)

        index = BuildIndex(['float a; float b; float c;'])
        source = 'edF32VECTOR4 dst;\nedF32VECTOR4 src;\n' + copies
        self.assertEqual(CollapseVectorAssignments(source, index), source)

    def test_moving_platform_with_repository_types(self):
        source = Path(__file__).with_name('fixtures').joinpath('moving_platform.cpp').read_text()
        result = transform(source, LoadIndex())
        for expected in [
            'local_10.xyz = pCVar1->field_0x10;',
            'local_20.xyz = this->vector_0x120.position;',
            'local_20.xyz = local_20.xyz * fVar3;',
            'this->vector_0x120.position = local_10.xyz;',
            'this->vector_0x120.rotation = local_20.xyz;',
            'local_10.w = 1.0f;',
            'local_20.w = 1.0f;',
            'pTVar2 = Timer::GetTimer();',
        ]:
            self.assertIn(expected, result)
        self.assertNotIn('local_10.x =', result)
        self.assertEqual(transform(result, LoadIndex()), result)
        self.assertEqual(transform(source.replace('\n', '\r\n'), LoadIndex()), result.replace('\n', '\r\n'))


if __name__ == '__main__':
    unittest.main()
