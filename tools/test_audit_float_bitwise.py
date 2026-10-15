import unittest

from audit_float_bitwise import scan_source


class FloatBitwiseAuditTest(unittest.TestCase):
    def test_multiline_selection_preserves_location(self):
        findings = scan_source("\nreturn static_cast<float>(\n (int)a * (uint)(a < b) |\n (int)b * (uint)(a >= b));")
        self.assertEqual(len(findings), 1)
        self.assertEqual(findings[0]["confidence"], "high")
        self.assertEqual(findings[0]["line"], 2)
        self.assertEqual(findings[0]["column"], 8)

    def test_c_style_float_cast(self):
        findings = scan_source("x = (float)((int)x * (uint)(x < y) | (int)y * (uint)(x >= y));")
        self.assertEqual(findings[0]["confidence"], "high")

    def test_bit_constant(self):
        findings = scan_source("x = (float)((int)x | 0x3e800000);")
        self.assertEqual(findings[0]["confidence"], "medium")

    def test_bit_constant_without_integer_cast(self):
        self.assertEqual(scan_source("x = (float)(seed & 0x7fffff | 0x3f800000);")[0]["confidence"], "medium")

    def test_pointer_member_access_is_not_a_comparison(self):
        findings = scan_source("x = (float)(*(int*)(p->data) + (uint) * (ushort*)&p->value * 0x10);")
        self.assertEqual(findings[0]["confidence"], "low")

    def test_unparenthesized_cast_does_not_capture_other_arguments(self):
        self.assertEqual(scan_source("call((float)width * 0.5f, (float)(int)y, &out);"), [])

    def test_integer_reconstruction_is_low_confidence(self):
        findings = scan_source("x = (float)(uVar1 >> 1 | uVar1 & 1);")
        self.assertEqual(findings[0]["confidence"], "low")

    def test_comments_and_strings_are_ignored(self):
        source = '''// (float)((int)x | 0x3e800000);
/* static_cast<float>((int)x | 0x3e800000); */
const char* s = "(float)((int)x | 0x3e800000)";
const char* r = R"tag((float)((int)x | 0x3e800000))tag";
flags = flags | 4;
if ((float)x > 0 || y > 0) {}
'''
        self.assertEqual(scan_source(source), [])

    def test_logical_operators_and_normal_clamp_are_ignored(self):
        self.assertEqual(scan_source("x = (float)(a || b); y = a < b ? a : b;"), [])


if __name__ == "__main__":
    unittest.main()
