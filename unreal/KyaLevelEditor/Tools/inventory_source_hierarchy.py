"""List source descendants of CActor and CBehaviour, including nested classes.

Run from any directory: python Tools/inventory_source_hierarchy.py
Outputs Markdown to stdout; does not overwrite the maintained audit checklist.
This is a lexical inventory, not a C++ compiler: conditional definitions are
included and unresolved/ambiguous bases are reported for manual review.
"""
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
SOURCE = ROOT / "src"
DECL = re.compile(r"\b(?:class|struct)\s+(\w+)\s*(?:final\s*)?(?::\s*([^;{}]+))?\s*\{")
NOISE = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)


def inventory():
    records = []
    for path in sorted(SOURCE.rglob("*")):
        if path.suffix not in (".h", ".hpp", ".cpp"):
            continue
        original = path.read_text(encoding="utf-8", errors="replace")
        text = NOISE.sub(lambda m: re.sub(r"[^\n]", " ", m.group()), original)
        ends, stack = {}, []
        for pos, char in enumerate(text):
            if char == "{":
                stack.append(pos)
            elif char == "}" and stack:
                ends[stack.pop()] = pos
        scopes = []
        for match in DECL.finditer(text):
            start = match.end() - 1
            while scopes and scopes[-1][0] < start:
                scopes.pop()
            name = "::".join([scope[1] for scope in scopes] + [match[1]])
            bases = []
            for base in (match[2] or "").split(","):
                base = re.sub(r"\b(public|protected|private|virtual)\b", "", base).strip()
                if base:
                    bases.append(base)
            records.append(dict(name=name, bases=bases, path=path,
                                line=original.count("\n", 0, match.start()) + 1))
            scopes.append((ends.get(start, len(text)), match[1]))

    by_name = {}
    for record in records:
        by_name.setdefault(record["name"], []).append(record)
    for record in records:
        resolved = []
        scope = record["name"].split("::")[:-1]
        for base in record["bases"]:
            candidates = ["::".join(scope[:depth] + [base])
                          for depth in range(len(scope), -1, -1)]
            resolved.append(next((name for name in candidates if name in by_name), base))
        record["resolved"] = resolved

    families = {"CActor": "Actor", "CBehaviour": "Behaviour"}
    changed = True
    while changed:
        changed = False
        for record in records:
            if record["name"] in families:
                continue
            inherited = {families[base] for base in record["resolved"] if base in families}
            if inherited:
                families[record["name"]] = "/".join(sorted(inherited))
                changed = True
    return records, families, by_name


def main():
    records, families, by_name = inventory()
    included = [record for record in records if record["name"] in families]
    actors = sum(families[record["name"]] == "Actor" for record in included)
    behaviours = sum(families[record["name"]] == "Behaviour" for record in included)
    print(f"Source inventory: {actors} actor definitions and {behaviours} behaviour definitions "
          "(including roots and shared/abstract bases).\n")
    for kind in ("Actor", "Behaviour"):
        print(f"## {kind} checklist\n")
        print("| Audited | Source type | Immediate base(s) | Declaration |")
        print("| --- | --- | --- | --- |")
        for record in sorted(included, key=lambda item: (item["path"], item["line"])):
            if families[record["name"]] != kind:
                continue
            path = record["path"].relative_to(ROOT).as_posix()
            # Relative to the checklist in unreal/KyaLevelEditor.
            link = "../../" + path
            print(f"| [ ] | `{record['name']}` | "
                  f"{', '.join('`' + base + '`' for base in record['resolved']) or '—'} | "
                  f"[{path}:{record['line']}]({link}#L{record['line']}) |")
        print()
    print("## Inventory review\n")
    print("Lexical discovery includes all source configurations. Review macro-generated types, "
          "factory registrations, and unresolved inheritance before claiming exhaustiveness.\n")
    for name, definitions in sorted(by_name.items()):
        if name in families and len(definitions) > 1:
            print(f"- Duplicate definition requiring review: `{name}` ({len(definitions)} declarations).")
    unresolved = sorted({base for record in records for base in record["resolved"]
                         if base not in by_name and (base.startswith("CActor") or base.startswith("CBehaviour"))})
    for base in unresolved:
        print(f"- Unresolved actor/behaviour-like base: `{base}`.")


if __name__ == "__main__":
    main()
