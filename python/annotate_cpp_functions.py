import re
import os

# === Configuration ===
mapping_file = "FunctionToFilename.txt"  # Your list of functions
source_root = "../src"            # Root folder of your C++ files

# === Load function-to-file mapping ===

# Paths are in the format _edFileDbgDispOpFiles() -> D:\Projects\EdenLib\edFile\sources\edFileDebug.cpp
# Symbol -> filename
# When comparing, strip the 'D:\\Projects\\' prefix from the mapping file path and compare to the source_root-relative path.

func_map = {}
# Open as UTF-16-LE
with open(mapping_file, "r", encoding="utf-16-le") as f:
    for line in f:
        line = line.strip()
        if not line or "->" not in line:
            continue
        func_sig, file_path = map(str.strip, line.split("->", 1))
        func_map[func_sig] = file_path.replace("\\", "/")
    print(f"Loaded {len(func_map)} function mappings.")

"""
Regex to detect function definitions with a return type.
Matches: return_type class::func(params) [const] { or ;
Excludes control statements and functions without a return type (e.g., constructors/destructors).
"""
func_def_regex = re.compile(
    r'^\s*([\w:<>&\*\s]+)\s+([\w:~]+)\s*\(([^)]*)\)\s*(?:const)?\s*(?:\{|;)',
    re.MULTILINE
)
control_keywords = {"if", "while", "switch", "for", "catch"}

def normalize_params(params: str) -> str:
    """Strip whitespace and parameter names, keep only types for comparison."""
    params = params.strip()
    if not params:
        return ""
    # Special case: (void) should be treated as empty parameters ()
    if params.lower() == "void":
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

        print(f"Processing: {file_path}")

        # Try different encodings common on Windows
        content = None
        encoding_used = None
        for encoding in ['utf-8', 'utf-8-sig', 'cp1252', 'latin-1']:
            try:
                with open(file_path, "r", encoding=encoding) as f:
                    content = f.read()
                encoding_used = encoding
                break
            except (UnicodeDecodeError, LookupError):
                continue
        
        if content is None:
            print(f"  Error: Could not decode file with any known encoding")
            continue

        modified = False
        lines = content.splitlines()

        # Compute the path relative to source_root
        rel_path = os.path.relpath(file_path, source_root).replace("\\", "/")

        print(f"Relative path: {rel_path}")

        # Collect all annotations first
        annotations = []
        for match in func_def_regex.finditer(content):
            return_type = match.group(1).strip()
            full_name = match.group(2)   # class::func or just func
            # Skip control statements and matches without a return type
            if full_name in control_keywords or not return_type or full_name == return_type:
                continue
            params = normalize_params(match.group(3))

            # Build comparable signature
            signature_to_check = f"{full_name}({params})"

            # Only compare if this signature is in the mapping file
            if signature_to_check not in func_map:
                continue

            correct_file = func_map[signature_to_check]
            # Remove 'D:/Projects/' prefix from correct_file if present
            correct_file_norm = correct_file
            prefix = "D:/Projects/"
            if correct_file_norm.startswith(prefix):
                correct_file_norm = correct_file_norm[len(prefix):]
            # Compare rel_path to correct_file_norm
            if rel_path != correct_file_norm:
                # Calculate the line number where the match starts
                sig_line_idx = content[:match.start()].count("\n")
                
                # Find the actual function line (skip empty/whitespace lines)
                actual_func_line = sig_line_idx
                while actual_func_line < len(lines) and lines[actual_func_line].strip() == "":
                    actual_func_line += 1
                
                comment = f"// Should be in: {correct_file}"
                # Check if comment already exists on the line before the actual function
                if actual_func_line > 0 and actual_func_line - 1 < len(lines) and "// Should be in:" in lines[actual_func_line - 1]:
                    continue
                # Check if comment already exists on the current line
                if actual_func_line < len(lines) and "// Should be in:" in lines[actual_func_line]:
                    continue
                annotations.append((actual_func_line, comment, signature_to_check))
                print(f"Will annotate line {actual_func_line} (0-indexed) in {file_path} for function {signature_to_check}")

        # Insert annotations in reverse order to maintain correct line indices
        if annotations:
            annotations.sort(reverse=True)
            new_lines = lines.copy()
            for sig_line_idx, comment, signature_to_check in annotations:
                print(f"Inserting '{comment}' at line {sig_line_idx}")
                if sig_line_idx < len(new_lines):
                    print(f"  Line {sig_line_idx} currently contains: '{new_lines[sig_line_idx]}'")
                    if sig_line_idx + 1 < len(new_lines):
                        print(f"  Line {sig_line_idx + 1} contains: '{new_lines[sig_line_idx + 1]}'")
                new_lines.insert(sig_line_idx, comment)
                modified = True

        if modified:
            try:
                with open(file_path, "w", encoding=encoding_used) as f:
                    f.write("\n".join(new_lines))
                print(f"Updated: {file_path} (encoding: {encoding_used})")
            except Exception as e:
                print(f"  Error writing file: {e}")
