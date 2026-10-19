import unittest

from symbol_index import BuildIndex
from transform import transform


class SymbolIndexTests(unittest.TestCase):
    def setUp(self):
        self.index = BuildIndex(["""
            class Service {
            public:
                void Run(int count);
                static void StaticRun(Service* object);
                void Mixed(int count);
                static void Mixed(Service* object, int count);
                static Service* arbitraryName;
                static Service valueInstance;
                void Inline() { Service* hiddenLocal; }
            };
            extern Service globalObject;
            Service* globalPointer = nullptr;
            void FreeFunction() { Service* hiddenGlobal; }
            // Service* commentedOut;
        """])

    def test_arbitrary_names(self):
        cases = [
            ("Service::Run(&globalObject,2);", "globalObject.Run(2);"),
            ("Service::Run(globalPointer,2);", "globalPointer->Run(2);"),
            ("Service::Run(Service::arbitraryName,2);", "Service::arbitraryName->Run(2);"),
            ("Service::Run(&Service::valueInstance,2);", "Service::valueInstance.Run(2);"),
            ("Service local;\nService::Run(&local,2);", "Service local;\nlocal.Run(2);"),
        ]
        for source, expected in cases:
            with self.subTest(source=source):
                self.assertEqual(transform(source, self.index), expected)

    def test_static_and_ambiguous_methods(self):
        for source in [
            "Service::StaticRun(globalPointer);",
            "Service* local;\nService::StaticRun(local);",
            "Service::Mixed(globalPointer, 2);",
            "Service::Unknown(globalPointer);",
            "Service::Run(&globalPointer, 2);",
            "Service::Run(globalObject, 2);",
            "Other local;\nService::Run(&local, 2);",
        ]:
            with self.subTest(source=source):
                self.assertEqual(transform(source, self.index), source)

    def test_typed_receiver_in_any_argument_position(self):
        index = BuildIndex(['''
            class Board { void Invalidate(float angle); static void Static(Board* board); };
            struct Squad { Board board; };
            class Commander { Squad squad; };
        '''])
        cases = [
            ('Board* board;\nBoard::Invalidate(1.0, board);',
             'Board* board;\nboard->Invalidate(1.0f);'),
            ('void Commander::Run()\n{\n  Board::Invalidate(1.0, &(this->squad).board);\n}',
             'void Commander::Run()\n{\n\tthis->squad.board.Invalidate(1.0f);\n}'),
            ('void Run(Board* board)\n{\n  Board::Invalidate(1.0, board);\n}',
             'void Run(Board* board)\n{\n\tboard->Invalidate(1.0f);\n}'),
        ]
        for source, expected in cases:
            with self.subTest(source=source):
                self.assertEqual(transform(source, index), expected)
        for source in [
            'Board* board;\nBoard* other;\nBoard::Invalidate(board, other);',
            'Board* board;\nBoard* other;\nBoard::Invalidate((Board *)board, other);',
            'Board* board;\nBoard::Static(board);',
            'void Other::Run()\n{\n\tBoard::Invalidate(1.0f, &this->squad.board);\n}',
        ]:
            self.assertEqual(transform(source, index), source)
        # The same parameter name has a different type in the next function.
        first, converted = cases[-1]
        second = '\nvoid Run(Other* board)\n{\n\tBoard::Invalidate(1.0f, board);\n}'
        self.assertEqual(transform(first + second, index), converted + second)

    def test_skips_function_bodies_and_comments(self):
        for name in ["hiddenLocal", "hiddenGlobal", "commentedOut"]:
            self.assertNotIn(name, self.index["symbols"])
        self.assertEqual(self.index["methods"]["Service::Inline"], ["member"])

    def test_conflicting_declarations(self):
        index = BuildIndex(["class Service { void Run(); }; Service* object; Other* object;"])
        source = "Service::Run(object);"
        self.assertEqual(transform(source, index), source)

    def test_instance_members_bases_and_packed_structs(self):
        index = BuildIndex(['''
            PACK(struct Properties { edF32VECTOR3 offset; });
            class Base { edF32VECTOR4 location; };
            class Derived : public Base {
                Properties* properties;
                volatile edF32VECTOR4 observed;
            };
        '''])
        self.assertEqual(index['members']['Properties::offset'], [['edF32VECTOR3', False]])
        self.assertEqual(index['members']['Base::location'], [['edF32VECTOR4', False]])
        self.assertEqual(index['members']['Derived::properties'], [['Properties', True]])
        self.assertEqual(index['bases']['Derived'], ['Base'])
        self.assertEqual(index['members']['Derived::observed'], [['__volatile__', False]])
        self.assertNotIn('Base::location', index['symbols'])


if __name__ == "__main__":
    unittest.main()
