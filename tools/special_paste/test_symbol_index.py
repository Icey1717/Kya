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
            "Service *local;\nService::StaticRun(local);",
            "Service::Mixed(globalPointer,2);",
            "Service::Unknown(globalPointer);",
            "Service::Run(&globalPointer,2);",
            "Service::Run(globalObject,2);",
            "Other local;\nService::Run(&local,2);",
        ]:
            with self.subTest(source=source):
                self.assertEqual(transform(source, self.index), source)

    def test_skips_function_bodies_and_comments(self):
        for name in ["hiddenLocal", "hiddenGlobal", "commentedOut"]:
            self.assertNotIn(name, self.index["symbols"])
        self.assertEqual(self.index["methods"]["Service::Inline"], ["member"])

    def test_conflicting_declarations(self):
        index = BuildIndex(["class Service { void Run(); }; Service* object; Other* object;"])
        source = "Service::Run(object);"
        self.assertEqual(transform(source, index), source)


if __name__ == "__main__":
    unittest.main()
