#!/usr/bin/env python3
"""Find possible numeric-cast mistranslations of Ghidra float bit selections.

This is a lexical scanner, not a C++ type checker. Findings require review
against PS2 disassembly; it never rewrites source files.
"""

import argparse
import bisect
import json
import re
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SOURCE_SUFFIXES = {".cpp", ".c", ".h", ".hpp", ".cc", ".cxx", ".inl"}
LEXICAL_NOISE = re.compile(
    r'R"(?P<delimiter>[^\s()\\]{0,16})\(.*?\)(?P=delimiter)"'
    r'|//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
    re.DOTALL,
)
FLOAT_CAST = re.compile(
    r"\bstatic_cast\s*<\s*(?:float|double)\s*>\s*\("
    r"|\(\s*(?:float|double)\s*\)"
    r"|\b(?:float|double)\s*\("
)
INTEGER_TYPE = r"(?:unsigned\s+|signed\s+)?(?:int|uint|long(?:\s+long)?|ulong|short|ushort|u?int(?:8|16|32|64)_t|[us](?:8|16|32|64))"
INTEGER_CAST = re.compile(
    rf"\(\s*{INTEGER_TYPE}\s*\)"
    rf"|\bstatic_cast\s*<\s*{INTEGER_TYPE}\s*>"
)
BITWISE = re.compile(r"(?<![|])\|(?![|])|(?<![&])&(?![&])|<<|>>|\^")
COMPARISON = re.compile(r"<=|>=|==|!=|(?<![<])<(?![<])|(?<![>\-])>(?![>])")
FLOAT_HINT = re.compile(r"\bfVar\d+\b|\b\d+\.\d*(?:[eE][+-]?\d+)?[fF]?\b")
BIT_CONSTANT = re.compile(
    r"\b0x(?:[0-9a-f]{16}|[0-9a-f]{8}|7fffff)[uUlL]*\b", re.IGNORECASE
)
IEEE_MASKS = {0x7fffff, 0x7f800000, 0x80000000, 0x7fffffff,
              0xfffffffffffff, 0x7ff0000000000000, 0x8000000000000000,
              0x7fffffffffffffff}
UNARY_CAST = re.compile(rf"\(\s*(?:{INTEGER_TYPE}|float|double)\s*\)")
IDENTIFIER = re.compile(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*")
NAMED_CAST = re.compile(r"(?:static|reinterpret|const|dynamic)_cast\s*<[^<>]+>\s*\(")
DEFAULT_REVIEWED = REPO_ROOT / "tools" / "float-bitwise-reviewed.json"


def mask_comments_and_strings(source):
    """Keep offsets and line numbers while hiding non-code text."""
    return LEXICAL_NOISE.sub(
        lambda match: "".join("\n" if c == "\n" else " " for c in match[0]), source
    )


def closing_paren(code, start):
    return closing_delimiter(code, start)


def closing_delimiter(code, start):
    pairs = {"(": ")", "[": "]", "{": "}"}
    stack = []
    for index in range(start, len(code)):
        char = code[index]
        if char in pairs:
            stack.append(pairs[char])
        elif char in ")]}":
            if not stack or stack.pop() != char:
                return None
            if not stack:
                return index
    return None


def skip_space(code, start):
    while start < len(code) and code[start].isspace():
        start += 1
    return start


def unary_operand_end(code, start):
    """Bound a C cast operand without swallowing later arguments/statements.

    Supports cast chains, unary operators, calls, indexing, and member access.
    Deliberately stops before binary operators: (float)a | b does not cast a|b.
    """
    start = skip_space(code, start)
    if start >= len(code):
        return None
    cast = UNARY_CAST.match(code, start)
    if cast:
        return unary_operand_end(code, cast.end())
    if code[start] in "~!+-*&":
        return unary_operand_end(code, start + 1)
    named = NAMED_CAST.match(code, start)
    if named:
        end = closing_delimiter(code, named.end() - 1)
        if end is None:
            return None
        end += 1
    elif code[start] == "(":
        end = closing_delimiter(code, start)
        if end is None:
            return None
        end += 1
    else:
        identifier = IDENTIFIER.match(code, start)
        atom = identifier or re.match(r"(?:0[xX][\da-fA-F]+|\d+(?:\.\d*)?)[\w]*", code[start:])
        if not atom:
            return None
        end = atom.end() if identifier else start + atom.end()
    while True:
        next_start = skip_space(code, end)
        if next_start < len(code) and code[next_start] in "([":
            close = closing_delimiter(code, next_start)
            if close is None:
                return None
            end = close + 1
        elif code.startswith("->", next_start) or code.startswith(".", next_start):
            member_start = next_start + (2 if code.startswith("->", next_start) else 1)
            member = IDENTIFIER.match(code, skip_space(code, member_start))
            if not member:
                return None
            end = member.end()
        else:
            return end


def has_float_bit_constant(expression):
    for match in BIT_CONSTANT.finditer(expression):
        literal = match[0].rstrip("uUlL")
        value = int(literal, 16)
        if value in IEEE_MASKS:
            return True
        # Common IEEE exponent construction, including negative values and doubles.
        if len(literal) == 10 and (value & 0x7f800000) in {0x3e800000, 0x3f000000, 0x3f800000, 0x40000000}:
            return True
        if len(literal) == 18 and (value & 0x7ff0000000000000) in {0x3fe0000000000000, 0x3ff0000000000000, 0x4000000000000000}:
            return True
    return False


def reviewed_key(path, expression):
    return path.replace("\\", "/"), re.sub(r"\s+", "", expression)


def load_reviewed(path):
    if path is None:
        return {}
    data = json.loads(path.read_text(encoding="utf-8"))
    reviewed = {}
    for entry in data["reviewed"]:
        if not entry.get("reason") or not entry.get("ps2_functions"):
            raise ValueError("Reviewed entries need a reason and PS2 function addresses")
        reviewed[reviewed_key(entry["path"], entry["expression"])] = entry
    return reviewed


def classify(expression):
    # Cast angle brackets are not comparisons or shifts.
    without_casts = re.sub(r"\b(?:static|reinterpret|const|dynamic)_cast\s*<[^<>]+>", "", expression)
    if not BITWISE.search(without_casts):
        return None
    has_integer_cast = bool(INTEGER_CAST.search(expression))
    if has_integer_cast and re.search(r"(?<!\|)\|(?!\|)|(?<!&)\&(?!&)|\^", without_casts) and COMPARISON.search(without_casts):
        return "high", "float conversion of integer casts combined with comparison and bitwise selection"
    if has_float_bit_constant(expression) or (has_integer_cast and FLOAT_HINT.search(expression)):
        return "medium", "float values or IEEE float bits mixed with integer casts and bitwise operations"
    return "low", "float conversion of a bitwise expression; may be legitimate integer arithmetic"


def scan_source(source, path="<input>"):
    code = mask_comments_and_strings(source)
    newline_offsets = [-1] + [m.start() for m in re.finditer("\n", code)]
    findings = []
    seen = set()
    for match in FLOAT_CAST.finditer(code):
        start = match.end()
        if match[0].rstrip().endswith("("):
            opening = match.end() - 1
        else:
            opening = None
        if opening is not None:
            end = closing_paren(code, opening)
            if end is None:
                continue
            end += 1
        else:
            end = unary_operand_end(code, start)
            if end is None:
                continue
        expression = code[match.start():end]
        result = classify(expression)
        if result is None:
            continue
        line = bisect.bisect_left(newline_offsets, match.start())
        key = (line, end)
        if key in seen:
            continue
        seen.add(key)
        confidence, reason = result
        findings.append({
            "path": str(path),
            "line": line,
            "column": match.start() - newline_offsets[line - 1],
            "confidence": confidence,
            "reason": reason,
            "expression": re.sub(r"\s+", " ", source[match.start():end]).strip(),
        })
    return findings


def source_files(paths):
    files = set()
    for path in paths:
        if not path.exists():
            raise FileNotFoundError(path)
        candidates = [path] if path.is_file() else path.rglob("*")
        for candidate in candidates:
            if candidate.is_file() and candidate.suffix.lower() in SOURCE_SUFFIXES:
                files.add(candidate.resolve())
    return sorted(files)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", type=Path, help="Files/directories; default: repository src/")
    parser.add_argument("--min-confidence", choices=["low", "medium", "high"], default="medium")
    parser.add_argument("--format", choices=["text", "json"], default="text")
    parser.add_argument("--output", type=Path, help="Write the report to this file")
    parser.add_argument("--reviewed", type=Path, default=DEFAULT_REVIEWED, help="Reviewed exceptions JSON")
    parser.add_argument("--include-reviewed", action="store_true", help="Show previously reviewed expressions too")
    args = parser.parse_args()
    ranks = {"high": 0, "medium": 1, "low": 2}
    findings = []
    reviewed_count = 0
    try:
        reviewed = load_reviewed(args.reviewed)
        files = source_files(args.paths or [REPO_ROOT / "src"])
        for path in files:
            try:
                label = path.relative_to(REPO_ROOT).as_posix()
            except ValueError:
                label = path.as_posix()
            findings.extend(scan_source(path.read_text(encoding="utf-8-sig", errors="replace"), label))
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))
    findings = [f for f in findings if ranks[f["confidence"]] <= ranks[args.min_confidence]]
    for finding in findings:
        entry = reviewed.get(reviewed_key(finding["path"], finding["expression"]))
        if entry:
            reviewed_count += 1
            finding["reviewed"] = entry["reason"]
    if not args.include_reviewed:
        findings = [f for f in findings if "reviewed" not in f]
    findings.sort(key=lambda f: (ranks[f["confidence"]], f["path"], f["line"]))
    if args.format == "json":
        report = json.dumps({"files_scanned": len(files), "reviewed_count": reviewed_count, "findings": findings}, indent=2)
    else:
        lines = [f"Scanned {len(files)} files; {len(findings)} candidates; {reviewed_count} reviewed matches. Verify against disassembly before editing."]
        for finding in findings:
            lines.extend([
                f"{finding['path']}:{finding['line']}:{finding['column']} [{finding['confidence']}] {finding['reason']}",
                f"  {finding['expression']}",
            ])
            if "reviewed" in finding:
                lines.append(f"  Reviewed: {finding['reviewed']}")
        report = "\n".join(lines)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(report + "\n", encoding="utf-8")
        print(f"Wrote {len(findings)} candidates to {args.output}")
    else:
        print(report)


if __name__ == "__main__":
    main()
