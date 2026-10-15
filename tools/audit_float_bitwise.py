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
)
INTEGER_CAST = re.compile(
    r"\(\s*(?:unsigned\s+)?(?:int|uint|long|ulong|short|ushort|u?int(?:32|64)_t)\s*\)"
    r"|\bstatic_cast\s*<\s*(?:unsigned\s+)?(?:int|uint|long|ulong|short|ushort|u?int(?:32|64)_t)\s*>"
)
BITWISE = re.compile(r"(?<![|])\|(?![|])|(?<![&])&(?![&])|<<|>>|\^")
COMPARISON = re.compile(r"<=|>=|==|!=|(?<![<])<(?![<])|(?<![>\-])>(?![>])")
FLOAT_HINT = re.compile(r"\bfVar\d+\b|\b\d+\.\d*(?:[eE][+-]?\d+)?[fF]?\b")
BIT_CONSTANT = re.compile(r"\b0x(?:3[ef]|[47bB][fF])[0-9a-fA-F]{6}\b", re.IGNORECASE)


def mask_comments_and_strings(source):
    """Keep offsets and line numbers while hiding non-code text."""
    return LEXICAL_NOISE.sub(
        lambda match: "".join("\n" if c == "\n" else " " for c in match[0]), source
    )


def closing_paren(code, start):
    depth = 0
    for index in range(start, len(code)):
        if code[index] == "(":
            depth += 1
        elif code[index] == ")":
            depth -= 1
            if depth == 0:
                return index
    return None


def classify(expression):
    if not BITWISE.search(expression):
        return None
    has_integer_cast = bool(INTEGER_CAST.search(expression))
    without_casts = re.sub(r"\bstatic_cast\s*<[^>]+>", "", expression)
    if has_integer_cast and re.search(r"(?<!\|)\|(?!\|)", expression) and COMPARISON.search(without_casts) and "*" in expression:
        return "high", "float conversion of integer casts combined with comparison/multiply selection"
    if BIT_CONSTANT.search(expression) or (has_integer_cast and FLOAT_HINT.search(expression)):
        return "medium", "float values or float bit constants mixed with integer casts and bitwise operations"
    return "low", "float conversion of a bitwise expression; may be legitimate integer arithmetic"


def scan_source(source, path="<input>"):
    code = mask_comments_and_strings(source)
    newline_offsets = [-1] + [m.start() for m in re.finditer("\n", code)]
    findings = []
    seen = set()
    for match in FLOAT_CAST.finditer(code):
        start = match.end()
        if match[0].lstrip().startswith("static_cast"):
            opening = match.end() - 1
        else:
            while start < len(code) and code[start].isspace():
                start += 1
            opening = start if start < len(code) and code[start] == "(" else None
        if opening is not None:
            end = closing_paren(code, opening)
            if end is None:
                continue
            end += 1
        else:
            # Without parentheses a lexical scan can pull in unrelated function
            # arguments or later casts. Leave these to a typed C++ audit.
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
    args = parser.parse_args()
    ranks = {"high": 0, "medium": 1, "low": 2}
    findings = []
    try:
        files = source_files(args.paths or [REPO_ROOT / "src"])
        for path in files:
            try:
                label = path.relative_to(REPO_ROOT).as_posix()
            except ValueError:
                label = path.as_posix()
            findings.extend(scan_source(path.read_text(encoding="utf-8-sig", errors="replace"), label))
    except OSError as error:
        parser.error(str(error))
    findings = [f for f in findings if ranks[f["confidence"]] <= ranks[args.min_confidence]]
    findings.sort(key=lambda f: (ranks[f["confidence"]], f["path"], f["line"]))
    if args.format == "json":
        report = json.dumps({"files_scanned": len(files), "findings": findings}, indent=2)
    else:
        lines = [f"Scanned {len(files)} files; {len(findings)} candidates. Verify against disassembly before editing."]
        for finding in findings:
            lines.extend([
                f"{finding['path']}:{finding['line']}:{finding['column']} [{finding['confidence']}] {finding['reason']}",
                f"  {finding['expression']}",
            ])
        report = "\n".join(lines)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(report + "\n", encoding="utf-8")
        print(f"Wrote {len(findings)} candidates to {args.output}")
    else:
        print(report)


if __name__ == "__main__":
    main()
