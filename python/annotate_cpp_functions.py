import re
import os

# === Configuration ===
mapping_file = "FunctionToFilename.txt"  # Your list of functions
source_root = "../src"            # Root folder of your C++ files

# === Load function-to-file mapping ===
func_map = {}
with open(mapping_file, "r", encoding="utf-8") as f:
    for line in f:
        line = line.strip()
        if not line or "->" not in line:
            continue
        func_sig, file_path = map(str.strip, line.split("->", 1))
        func_map[func_sig] = file_path.replace("\\", "/")  # normalize slashes

# === Regex to detect function definitions ===
# Matches: return_type class::func(params) [const] { or ;
func_def_regex = re.compile(
    r'^\s*(?:[\w:<>\*&\s]+)\s+([\w:]+)\s*\(([^)]*)\)\s*(?:const)?\s*(?:\{|;)',
    re.MULTILINE
)

def normalize_params(params: str) -> str:
    """Strip whitespace and parameter names, keep only types for comparison."""
    params = params.strip()
    if not params:
        return ""
    # Remove parameter names (simple approach: take type before last word)
    normalized = []
    for p in params.split(","):
        p = p.strip()
        # Remove default values
        p = p.split("=")[0].strip()
        # Keep everything except last token (name)
        parts = p.split()
        if parts:
            normalized.append(" ".join(parts[:-1]) if len(parts) > 1 else parts[0])
    return ",".join(normalized)

# === Walk through source files ===
for root, _, files in os.walk(source_root):
    for file in files:
        if not file.endswith((".cpp", ".cxx", ".cc")):
            continue
        file_path = os.path.join(root, file)
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()

        modified = False
        lines = content.splitlines()
        new_lines = lines.copy()

        for match in func_def_regex.finditer(content):
            full_name = match.group(1)   # class::func or just func
            params = normalize_params(match.group(2))

            # Build comparable signature
            signature_to_check = f"{full_name}({params})"

            for sig, correct_file in func_map.items():
                # normalize mapping signature
                map_match = re.match(r'([\w:]+)\s*\(([^)]*)\)', sig)
                if not map_match:
                    continue
                map_full_name = map_match.group(1)
                map_params = normalize_params(map_match.group(2))
                map_sig = f"{map_full_name}({map_params})"

                if signature_to_check == map_sig:
                    if os.path.normpath(file_path) != os.path.normpath(correct_file):
                        line_no = content[:match.start()].count("\n")
                        comment = f"// Should be in: {correct_file}"
                        # Avoid duplicate comment
                        if not new_lines[line_no].strip().startswith("// Should be in:"):
                            new_lines.insert(line_no, comment)
                            modified = True
                    break

        if modified:
            with open(file_path, "w", encoding="utf-8") as f:
                f.write("\n".join(new_lines))
            print(f"Updated: {file_path}")
