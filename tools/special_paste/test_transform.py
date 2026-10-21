from pathlib import Path
import subprocess
import sys
import unittest

from transform import transform, GetRepositoryIndex


EMPTY_INDEX = {"symbols": {}, "methods": {}}


class TransformTests(unittest.TestCase):
    def convert(self, source):
        return transform(source, EMPTY_INDEX)

    def test_projection_aim_matches_target(self):
        source = """bool CActorAutonomous::IsProjectionAim(CActorAutonomous *this)

{
  bool bVar1;
  CLifeInterface *pCVar2;
  bool bVar3;
  float fVar4;

  bVar3 = false;
  bVar1 = CActor::IsProjectionAim((CActor *)this);
  if (bVar1 != false) {
    pCVar2 = (*((this->base).base.pVTable)->GetLifeInterface)((CActor *)this);
    fVar4 = (*pCVar2->pVtable->GetValue)((CInterface *)pCVar2);
    if (0.0 < fVar4) {
      bVar3 = true;
    }
  }
  return bVar3;
}"""
        expected = """bool CActorAutonomous::IsProjectionAim()
{
	bool bVar1;
	CLifeInterface* pCVar2;
	bool bVar3;
	float fVar4;

	bVar3 = false;

	bVar1 = CActor::IsProjectionAim();
	if (bVar1 != false) {
		pCVar2 = GetLifeInterface();
		fVar4 = pCVar2->GetValue();
		if (0.0f < fVar4) {
			bVar3 = true;
		}
	}

	return bVar3;
}"""
        self.assertEqual(self.convert(source), expected)
        self.assertEqual(self.convert(expected), expected)
        self.assertEqual(self.convert(source.replace("\n", "\r\n")),
                         expected.replace("\n", "\r\n"))

    def test_base_dispatch_is_preserved(self):
        self.assertEqual(self.convert("CActor::IsProjectionAim((CActor *)this);"),
                         "CActor::IsProjectionAim();")

    def test_virtual_receiver_matches_table_owner(self):
        cases = [
            ("(*other->pVTable->Visit)(other, this);", "other->Visit(this);"),
            ("(*this->pVTable->Visit)(other, this);", "Visit(other);"),
            ("(*other->pVTable->Visit)(2.0, (Actor *)other);", "other->Visit(2.0f);"),
            ("(*other->pVTable->Visit)(other, other);", "(*other->pVTable->Visit)(other, other);"),
            ("(*other->pVTable->Visit)(this);", "(*other->pVTable->Visit)(this);"),
            ("(*callbacks->OnEvent)(context, event);", "(*callbacks->OnEvent)(context, event);"),
        ]
        for source, expected in cases:
            with self.subTest(source=source):
                self.assertEqual(self.convert(source), expected)

    def test_uncertain_casts_stay_unchanged(self):
        for source in ["value = (int)pActor->flags;", "pActor = (CActor *)pData;",
                       "value = (int)Read()->flags;", "value = (float)(a + b);",
                       "Util::Run((int)count);"]:
            with self.subTest(source=source):
                self.assertEqual(self.convert(source), source)

    def test_preserves_component_writes_and_evaluation_counts(self):
        for components in ["xyz", "xyzw"]:
            for rhs in ["src", "Next()->value"]:
                source = "\n".join(f"dst.{c} = {rhs}.{c};" for c in components)
                self.assertEqual(self.convert(source), source)

    def test_preserves_timer_assignments_and_values(self):
        for source in [
            "Timer* pTimer;\npTimer = AcquireTimer();\nUse(pTimer);",
            "Timer* pTimer;\npTimer = GetTimer();\nUse(pTimer);",
            "Timer value;\nvalue = *AcquireTimer();\nUse(value);",
        ]:
            self.assertEqual(self.convert(source), source)

    def test_inlines_timer_lookup_for_bounce_speed(self):
        for lookup in ['GetTimer()', 'Timer::GetTimer()']:
            source = ('  Timer *pTVar1;\n  float fVar2;\n  float fVar4;\n\n'
                      '  CActor::SV_UpdateOrientation2D(*(float *)&(this->base).field_0x1520, (CActor *)this, &local_20, 0);\n'
                      '  fVar4 = edF32Vector4SafeNormalize0Hard(&this->dynamic, &local_10);\n'
                      f'  pTVar1 = {lookup};\n'
                      '  fVar2 = fVar4 / pTVar1->cutsceneDeltaTime;\n'
                      '  fVar3 = 2.0;\n'
                      '  if (fVar2 < 2.0) {\n    fVar3 = fVar2;\n  }')
            for newline in ['\n', '\r\n']:
                result = self.convert(source.replace('\n', newline))
                self.assertNotIn('pTVar1', result)
                self.assertIn(f'fVar2 = fVar4 / {lookup}->cutsceneDeltaTime;', result)
                self.assertEqual(self.convert(result), result)
                if newline == '\r\n':
                    self.assertNotIn('\n', result.replace('\r\n', ''))

    def test_timer_lookup_keeps_ambiguous_or_mutated_locals(self):
        for body in [
            'Use(pTimer);',
            'if (ready) {\nUse(pTimer->elapsed);\n}',
            'pTimer = AcquireTimer();\nUse(pTimer->elapsed);',
            'pTimer->elapsed = 2;',
            'pTimer->elapsed++;',
            '++pTimer->elapsed;',
            'Use(&pTimer->elapsed);',
        ]:
            source = 'Timer* pTimer;\npTimer = GetTimer();\n' + body
            with self.subTest(body=body):
                self.assertEqual(self.convert(source), source)

    def test_timer_lookup_preserves_protected_text(self):
        source = ('Timer* pTimer;\npTimer = GetTimer();\n'
                  'Use(pTimer->elapsed);\n// pTimer\nPrint("pTimer");')
        self.assertEqual(self.convert(source),
                         'Use(GetTimer()->elapsed);\n// pTimer\nPrint("pTimer");')

    def test_protects_non_code(self):
        cases = [
            'Print("version 1.0, ABS(x), CActor::Run((CActor *)this)");',
            'Print("escaped \\" quote, 1.0");',
            'Print(R"tag(raw " quote, 1.0, ABS(x)\nsecond line)tag");',
            "char ch = '(';",
            "// 1.0, ABS(x), CActor::Run((CActor *)this)\nx = 2.0f;",
            "/* 1.0\n  ABS(x) */\nx = 2.0f;",
            '#define FLOAT_TEXT "1.0" \\\n  ABS(x)',
        ]
        for source in cases:
            with self.subTest(source=source):
                self.assertEqual(self.convert(source), source)

    def test_literals_do_not_confuse_argument_splitting(self):
        self.assertEqual(self.convert('(*obj->pVTable->Print)(obj, "a,b) 1.0");'),
                         'obj->Print("a,b) 1.0");')

    def test_float_suffixes(self):
        self.assertEqual(self.convert("x = 0.0; y = 1.25f; z = 2.50L;"),
                         "x = 0.0f; y = 1.25f; z = 2.50L;")

    def test_member_path_parentheses_and_call_spacing(self):
        cases = [
            ('p = &(this->squad).chessboard;', 'p = &this->squad.chessboard;'),
            ('x = ((obj->member).nested).x;', 'x = obj->member.nested.x;'),
            ('x = Read(obj->member).x;', 'x = Read(obj->member).x;'),
            ('x = (left + right).x;', 'x = (left + right).x;'),
            ('Call(&a,&b,  c);', 'Call(&a, &b, c);'),
            ('Call("a,b",&b);', 'Call("a,b", &b);'),
        ]
        for source, expected in cases:
            with self.subTest(source=source):
                self.assertEqual(self.convert(source), expected)

    def test_code_cast_virtual_calls(self):
        cases = [
            ('(*(code *)this->pVTable->_UpdatePattern)(this);', '_UpdatePattern();'),
            ('(*(code *)obj->pVTable->Visit)(obj, 2);', 'obj->Visit(2);'),
            ('(*(code *)obj->pVTable->Visit)(other);', '(*(code *)obj->pVTable->Visit)(other);'),
            ('(*(code *)callbacks->Visit)(obj);', '(*(code *)callbacks->Visit)(obj);'),
        ]
        for source, expected in cases:
            with self.subTest(source=source):
                self.assertEqual(self.convert(source), expected)

    def test_commander_example_with_repository_types(self):
        source = Path(__file__).with_name('fixtures').joinpath('commander_beat_up.cpp').read_text()
        index = GetRepositoryIndex()
        result = transform(source, index)
        for expected in [
            'void CActorCommander::StateCommanderBeatUp()\n{',
            'iVar1 = this->squad.NbElt();',
            'pChessboard = &this->squad.chessboard;',
            'this->squad.chessboard.field_0x230 = pFighter->currentLocation;',
            'this->squad.chessboard.field_0x240 = gF32Vector4UnitZ;',
            'local_10 = pFighter->rotationQuat;',
            'pChessboard->InvalidateSection(1.570796f, &local_10);',
            'pChessboard->InvalidateSection(3.141593f, &local_10);',
            'pPathFinderClient = this->field_0x9f4->GetPathfinderClient();',
            'pChessboard->InvalidateByZone(this->guardAreaZoneId);',
            'pChessboard->InvalidateByPath(pPathFinderClient);',
            '_UpdatePattern();',
        ]:
            self.assertIn(expected, result)
        for obsolete in ['fVar2', 'fVar3', 'fVar4', '(this->squad)', 'pVTable']:
            self.assertNotIn(obsolete, result)
        self.assertEqual(transform(result, index), result)
        self.assertEqual(transform(source.replace('\n', '\r\n'), index),
                         result.replace('\n', '\r\n'))

    def test_cli_utf8_bom_and_line_endings(self):
        script = Path(__file__).with_name("transform.py")
        for newline in ["\n", "\r\n"]:
            source = f'// caf\u00e9{newline}x = 0.0;{newline}'
            result = subprocess.run([sys.executable, "-B", str(script)],
                                    input=b"\xef\xbb\xbf" + source.encode("utf-8"),
                                    capture_output=True, check=True)
            self.assertEqual(result.stdout, source.replace("0.0", "0.0f").encode("utf-8"))
            self.assertEqual(result.stderr, b"")


if __name__ == "__main__":
    unittest.main()
