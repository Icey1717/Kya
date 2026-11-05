"""Relocate bounded Clang AMD64 relative jump tables from actual COFF bytes.

Only a RIP-relative LEA preceded by the audited subtract/JA bounds idiom is
accepted. Every table relocation must target an instruction inside its owning
complete function. Instruction boundaries are checked again after lifting.
"""
import struct
from coff_data import symbol_name


def recover(data, source, function, relocation_offset, symbol_index):
    symoff, nsyms = struct.unpack_from('<II', data, 8)
    if symbol_index >= nsyms:
        raise ValueError('Jump table symbol index outside COFF table')
    symbol_value, section, kind = struct.unpack_from('<IhH', data, symoff + symbol_index * 18 + 8)
    if section <= 0 or kind != 0 or symbol_name(data, symbol_index) != '.rdata':
        raise ValueError('Unsupported local jump-table symbol')
    header = 20 + (section - 1) * 40
    size, raw, reloc = struct.unpack_from('<III', data, header + 16)
    flags = struct.unpack_from('<I', data, header + 36)[0]
    count = struct.unpack_from('<H', data, header + 32)[0]
    if flags & 0xa0000020 or not flags & 0x40 or not raw or raw + size > len(data) or count == 0xffff:
        raise ValueError('Jump table must be bounded non-executable read-only data')
    code_header = 20 + (function['coff_section'] - 1) * 40
    code_raw = struct.unpack_from('<I', data, code_header + 20)[0]
    site = code_raw + relocation_offset
    if relocation_offset < function['coff_offset'] + 16:
        raise ValueError('Jump table bound lies before its owning function')
    if data[site - 3:site] != b'\x48\x8d\x05':
        raise ValueError('Unsupported jump table address instruction')
    # Clang -O0: SUB EAX, imm8; JA default; MOV RCX, [RBP+disp8]; LEA RAX, table.
    prefix = data[site - 16:site - 3]
    if (len(prefix) != 13 or prefix[:2] != b'\x83\xe8' or prefix[3:5] != b'\x0f\x87'
            or prefix[9:12] != b'\x48\x8b\x4d' or prefix[2] >= 0x80):
        raise ValueError('Unsupported jump table bound instruction sequence')
    entries = prefix[2] + 1
    start = symbol_value + struct.unpack_from('<i', data, site)[0]
    if start < 0 or start % 4 or start + entries * 4 > size:
        raise ValueError('Jump table extent outside COFF storage')
    relocations = {}
    for i in range(count):
        offset, target, typ = struct.unpack_from('<IIH', data, reloc + i * 10)
        if start <= offset < start + entries * 4:
            if offset in relocations or offset % 4 or typ != 4 or target >= nsyms:
                raise ValueError('Invalid jump table relocation')
            value, target_section = struct.unpack_from('<Ih', data, symoff + target * 18 + 8)
            if target_section != function['coff_section'] or symbol_name(data, target) != '.text':
                raise ValueError('Jump table leaves its owning code section')
            addend = struct.unpack_from('<i', data, raw + offset)[0]
            target_address = value + addend - (offset - start) - 4
            if not function['coff_offset'] <= target_address < function['coff_offset'] + function['size']:
                raise ValueError('Jump table target leaves its complete function')
            relocations[offset] = target_address - function['coff_offset']
    if set(relocations) != set(range(start, start + entries * 4, 4)):
        raise ValueError('Jump table has missing entry relocations')
    return dict(symbol=f'$jump:{source}:{section}:{start}', source=source, section=section,
                start=start, entries=entries, source_site=relocation_offset - function['coff_offset'],
                targets=[relocations[offset] for offset in sorted(relocations)])


def append(blob, table, function):
    while len(blob) % 16:
        blob.append(0xcc)
    base = len(blob)
    targets = [function['blob_offset'] + offset for offset in table['targets']]
    for target in targets:
        displacement = target - base
        if not -(1 << 31) <= displacement < 1 << 31:
            raise ValueError('Jump table displacement outside signed 32-bit range')
        blob.extend(struct.pack('<i', displacement))
    return dict(table, address=base, targets=targets)


def targets(export):
    tables = export.get('coff', {}).get('jump_tables', [])
    if not tables:
        return {}
    branches = [i for i in export['instructions'] if any(op['opcode'] == 'BRANCHIND' for op in i['ops'])]
    if len(tables) != 1 or len(branches) != 1:
        raise ValueError('Jump table requires exactly one lifted indirect branch')
    addresses = {int(i['address'], 16) for i in export['instructions']}
    destinations = sorted(set(tables[0]['targets']))
    if not set(destinations) <= addresses:
        raise ValueError('Jump table target is not a lifted instruction boundary')
    return {int(branches[0]['address'], 16): destinations}
