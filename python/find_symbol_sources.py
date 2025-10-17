from elftools.elf.elffile import ELFFile
import re
import sys
import cw_demangler

def try_demangle_mwcc(name: str) -> str:
    try:
        # Set the mode to demangle
        cw_demangler.mode = 'demangle'
        cw_demangler.verbose = True
        return cw_demangler.demangle_try(name)
    except Exception as e:
        print(f"[WARN] Could not demangle {name}: {e}")
        return name

# Configuration
SEARCH_WINDOW = 0x400  # bytes to scan backward from symbol string
SOURCE_PATTERNS = (b'.cpp', b'.c', b'.h')

def find_symbol_file_offset(data, symbol_name: str):
    """
    Finds the file offset of a symbol name that appears in text,
    preceded by 0x20 (space) and null-terminated.
    Returns the offset of the first character of the symbol, or None.
    """

    pattern = b' ' + symbol_name.encode('ascii') + b'\x00'
    pos = data.find(pattern)
    if pos == -1:
        #print(f"[DEBUG] Symbol '{symbol_name}' not found.")
        return None
    # Return offset of the symbol itself (skip the space)
    return pos + 1

def find_nearby_filename(data, start_offset):
    """
    Scan backward from start_offset to find the nearest filename string
    ending with .cpp, .c, or .h, optionally prefixed by "MW MIPS".
    """
    
    window_start = max(0, start_offset - SEARCH_WINDOW)
    region = data[window_start:start_offset]
    
    # Find all candidate strings (ASCII only)
    ascii_matches = re.finditer(rb'[\x20-\x7E]{4,}', region)
    candidates = []
    for match in ascii_matches:
        s = match.group(0)
        if any(s.endswith(ext) for ext in SOURCE_PATTERNS):
            candidates.append((window_start + match.start(), s))
    
    if not candidates:
        return None

    # print all the candidates for debugging
    #for offset, s in candidates:
        #print(f"[DEBUG] Candidate filename at 0x{offset:X}: {s.decode('ascii', errors='ignore')}")

    # Pick the last one before the symbol (closest)
    candidates.sort(key=lambda x: x[0])
    filename = candidates[-1][1].decode('ascii', errors='ignore')
    return filename

def main(path):
    with open(path, 'rb') as f:
        elf = ELFFile(f)
        f.seek(0)
        data = f.read()

        symtab = None
        for section in elf.iter_sections():
            if section.name == '.symtab':
                symtab = section
                break

        if not symtab:
            print("No symbol table found.")
            return

        print(f"[INFO] Loaded ELF: {path}")
        for sym in symtab.iter_symbols():
            name = sym.name
            if not name or sym['st_value'] == 0:
                continue

            #print(f"[INFO] Processing symbol: {name}")

            # Find symbol in raw file bytes
            file_offset = find_symbol_file_offset(data, name)
            if file_offset is None:
                continue

            # Debug log
            #print(f"[DEBUG] Found symbol '{name}' at text offset 0x{file_offset:X}")

            # Scan backward for filename
            src = find_nearby_filename(data, file_offset)
            if src:
                print(f"{try_demangle_mwcc(name)} -> {src}")
            #else:
                #print(f"  ↳ [no nearby source string found]")
        
        print("[INFO] Done.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python find_source_names.py <file.elf>")
    else:
        main(sys.argv[1])
