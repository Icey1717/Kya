"""Conservative declaration index for special paste; not a full C++ parser."""

import hashlib
import json
from pathlib import Path
import re
import tempfile

VERSION = 1


def BuildIndex(sources):
    symbols = {}
    methods = {}

    def Add(mapping, name, value):
        values = mapping.setdefault(name, [])
        if value not in values:
            values.append(value)

    def Scan(text, scope="", inClass=False):
        start = 0
        pos = 0
        while pos < len(text):
            if text[pos] not in ";{":
                pos += 1
                continue
            statement = text[start:pos].strip()
            statement = re.sub(r"\b(?:public|private|protected)\s*:", "", statement).strip()
            if text[pos] == "{":
                depth = 1
                end = pos + 1
                while end < len(text) and depth:
                    depth += (text[end] == "{") - (text[end] == "}")
                    end += 1
                container = re.fullmatch(r"(?:class|struct)\s+(\w+)(?:\s*:[^{]+)?", statement)
                namespace = re.fullmatch(r"namespace\s+(\w+)", statement)
                if container or namespace:
                    name = (container or namespace).group(1)
                    Scan(text[pos + 1:end - 1], scope + name + "::", bool(container))
                else:
                    Record(statement, scope, inClass)
                pos = end
                start = pos
            else:
                Record(statement, scope, inClass)
                pos += 1
                start = pos

    def Record(statement, scope, inClass):
        if not statement or re.match(r"(?:typedef|using|return|friend)\b", statement):
            return
        method = re.fullmatch(
            r"[\w\s:*&<>]+?\s+(\w+)\s*\([^;{}]*\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?(?:=\s*0)?",
            statement,
        )
        if inClass and method:
            Add(methods, scope + method.group(1), "static" if re.search(r"\bstatic\b", statement) else "member")
            return
        if inClass and not re.search(r"\bstatic\b", statement):
            return
        declaration = re.fullmatch(
            r"(?:(?:extern|static|inline|constexpr|const|volatile|struct|class)\s+)*"
            r"(?P<type>\w+(?:::\w+)*)\s*(?P<pointer>\*)?\s*(?:const\s+)?"
            r"(?P<name>\w+(?:::\w+)*)(?:\s*=\s*[^;{}]+)?",
            statement,
        )
        if declaration:
            Add(symbols, scope + declaration['name'], [declaration['type'], bool(declaration['pointer'])])

    for source in sources:
        # Hide comments, literals and preprocessing directives before scanning.
        source = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', " ", source)
        source = re.sub(r"^[ \t]*#(?:[^\n]*\\\n)*[^\n]*", "", source, flags=re.MULTILINE)
        Scan(source)
    return {"symbols": symbols, "methods": methods}


def LoadIndex():
    root = Path(__file__).resolve().parents[2]
    paths = sorted(path for path in (root / "src").rglob("*") if path.suffix in {".h", ".hpp", ".cpp"})
    manifest = [(str(path.relative_to(root)), path.stat().st_mtime_ns, path.stat().st_size) for path in paths]
    key = hashlib.sha256(str(root).encode()).hexdigest()[:16]
    cache = Path(tempfile.gettempdir()) / f"kya_special_paste_symbols_{key}.json"
    signature = hashlib.sha256(json.dumps([VERSION, manifest]).encode()).hexdigest()
    try:
        saved = json.loads(cache.read_text(encoding="utf-8"))
        if saved.get("signature") == signature:
            return saved["index"]
    except (OSError, ValueError, KeyError):
        pass
    index = BuildIndex(path.read_text(encoding="utf-8", errors="replace") for path in paths)
    try:
        cache.write_text(json.dumps({"signature": signature, "index": index}), encoding="utf-8")
    except OSError:
        pass  # A read-only cache directory must not prevent pasting.
    return index
