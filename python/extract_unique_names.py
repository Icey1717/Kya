from pathlib import Path
import re


SCRIPT_DIR = Path(__file__).resolve().parent
SOURCE_ROOT = SCRIPT_DIR.parent / "src"
MAPPING_FILE = SCRIPT_DIR / "FunctionToFilename.txt"
PROJECT_PREFIX = "D:" + chr(92) + "Projects" + chr(92)


def include_guard(filename: str) -> str:
    """Convert a filename to an uppercase include guard.

    Existing separators are retained as underscores, and underscores are
    inserted at camelCase and PascalCase word boundaries.
    """
    guard = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", filename)
    guard = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", guard)
    guard = re.sub(r"[^A-Za-z0-9]+", "_", guard)
    return guard.strip("_").upper()


def create_missing_files() -> list[Path]:
    missing_files: set[Path] = set()

    with MAPPING_FILE.open("r", encoding="utf-16") as mapping:
        for line in mapping:
            if " -> " not in line:
                continue

            source_path = line.rsplit(" -> ", 1)[1].strip()
            if not source_path.startswith(PROJECT_PREFIX):
                continue

            relative_path = source_path.removeprefix(PROJECT_PREFIX).replace(chr(92), "/")
            destination = SOURCE_ROOT / relative_path
            if not destination.is_file():
                missing_files.add(destination)

    for destination in sorted(missing_files):
        destination.parent.mkdir(parents=True, exist_ok=True)
        guard = include_guard(destination.name)
        destination.write_text(
            f"#ifndef {guard}\n"
            f"#define {guard}\n"
            "\n"
            f"#endif // {guard}\n",
            encoding="utf-8",
        )

    return sorted(missing_files)


if __name__ == "__main__":
    created_files = create_missing_files()
    print(f"Created {len(created_files)} missing files:")
    for created_file in created_files:
        print(created_file.relative_to(SOURCE_ROOT.parent).as_posix())
