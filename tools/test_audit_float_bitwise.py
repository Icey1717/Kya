import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from audit_float_bitwise import load_reviewed, reviewed_key, scan_source


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

    def test_chained_unparenthesized_cast_reaches_bitwise_operand(self):
        finding = scan_source("x = (float)(uint32_t)(fVar1 & 0x7fffff);")[0]
        self.assertEqual(finding["confidence"], "medium")
        self.assertEqual(finding["expression"], "(float)(uint32_t)(fVar1 & 0x7fffff)")

    def test_chained_named_cast_and_function_operand(self):
        finding = scan_source("x = (float)static_cast<int>(get(fVar1 | bits));")[0]
        self.assertEqual(finding["confidence"], "medium")
        self.assertEqual(finding["expression"], "(float)static_cast<int>(get(fVar1 | bits))")

    def test_function_style_double_cast(self):
        self.assertEqual(scan_source("x = double(bits | 0x3ff0000000000000ULL);")[0]["confidence"], "medium")

    def test_binary_operation_outside_cast_is_not_its_operand(self):
        self.assertEqual(scan_source("x = (float)array[index].value | 0x3f800000;"), [])

    def test_cast_does_not_swallow_following_statement(self):
        self.assertEqual(scan_source("x = (float)get(a); flags = flags | 0x3f800000;"), [])

    def test_mask_selection_without_multiplication(self):
        finding = scan_source("x = (float)(((int)a & -(int)(a < b)) | ((int)b & -(int)(a >= b)));")[0]
        self.assertEqual(finding["confidence"], "high")

    def test_comparison_and_shift_selection(self):
        finding = scan_source("x = (float)((int)fVar1 >> (uint)(a < b));")[0]
        self.assertEqual(finding["confidence"], "medium")

    def test_sign_and_exponent_masks_with_suffixes(self):
        for expression in ["bits & 0x80000000U", "bits ^ 0xbf800000u", "bits | 0x7f800000UL"]:
            with self.subTest(expression=expression):
                self.assertEqual(scan_source(f"x = (float)({expression});")[0]["confidence"], "medium")

    def test_ordinary_hex_flag_is_low_confidence(self):
        self.assertEqual(scan_source("x = (float)(flags | 0x00000004);")[0]["confidence"], "low")

    def test_static_cast_template_is_not_a_comparison(self):
        self.assertEqual(scan_source("x = float(static_cast<int>(count) | mask);")[0]["confidence"], "low")

    def test_unbalanced_expression_is_skipped(self):
        self.assertEqual(scan_source("x = (float)((int)fVar1 | bits;"), [])

    def test_reviewed_key_ignores_line_and_whitespace_changes(self):
        self.assertEqual(reviewed_key("src\\test.cpp", "(float)(a | b)"),
                         reviewed_key("src/test.cpp", "(float) ( a\n|b )"))
        self.assertNotEqual(reviewed_key("src/test.cpp", "(float)(a | b)"),
                            reviewed_key("src/test.cpp", "(float)(a | c)"))

    def test_reviewed_cli_hides_only_matching_expression(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "test.cpp"
            source.write_text("x = (float)(bits | 0x3f800000);", encoding="utf-8")
            reviewed = root / "reviewed.json"
            reviewed.write_text(json.dumps({"reviewed": [{
                "path": source.as_posix(), "expression": "(float)(bits | 0x3f800000)",
                "reason": "Reviewed fixture", "ps2_functions": ["0x00100000"],
            }]}), encoding="utf-8")
            command = [sys.executable, str(Path(__file__).with_name("audit_float_bitwise.py")),
                       str(source), "--reviewed", str(reviewed), "--format", "json"]
            hidden = json.loads(subprocess.check_output(command, text=True))
            self.assertEqual(hidden["findings"], [])
            self.assertEqual(hidden["reviewed_count"], 1)
            shown = json.loads(subprocess.check_output(command + ["--include-reviewed"], text=True))
            self.assertEqual(shown["findings"][0]["reviewed"], "Reviewed fixture")
            source.write_text("x = (float)(bits | 0x3f000000);", encoding="utf-8")
            changed = json.loads(subprocess.check_output(command, text=True))
            self.assertEqual(len(changed["findings"]), 1)
            self.assertEqual(changed["reviewed_count"], 0)

    def test_reviewed_entries_require_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "reviewed.json"
            path.write_text('{"reviewed": [{"path": "x", "expression": "x"}]}', encoding="utf-8")
            with self.assertRaises(ValueError):
                load_reviewed(path)


if __name__ == "__main__":
    unittest.main()
