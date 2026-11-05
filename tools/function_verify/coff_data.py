"""Extract explicitly sized COFF data definitions without inventing initializers.

The bytes describe link-time storage, not post-constructor runtime state. Callers
must supply and prove their runtime memory relation separately.
"""
import hashlib
import struct
from compiled import coff_symbols


def symbol_name(data, index):
    symoff, count = struct.unpack_from('<II', data, 8)
    if not 0 <= index < count:
        raise ValueError('Invalid COFF relocation symbol index')
    # Reject auxiliary records as relocation targets.
    cursor = 0
    while cursor < index:
        cursor += 1 + data[symoff + cursor * 18 + 17]
    if cursor != index:
        raise ValueError('COFF relocation targets auxiliary record')
    raw = data[symoff + index * 18:symoff + index * 18 + 8]
    if raw[:4] == bytes(4):
        start = symoff + count * 18 + struct.unpack_from('<I', raw, 4)[0]
        return data[start:data.index(b'\0', start)].decode()
    return raw.rstrip(b'\0').decode()


def extract(data, symbol, size):
    """Return exact bytes and ADDR64 pointer fixups for a declared data object."""
    symbols, sections = coff_symbols(data)
    matches = [s for s in symbols if s[0] == symbol and s[2] > 0]
    if len(matches) != 1 or not isinstance(size, int) or isinstance(size, bool) or size <= 0:
        raise ValueError('Missing/ambiguous data definition or invalid extent: ' + symbol)
    _, start, section, kind = matches[0]
    if kind != 0 or not 1 <= section <= sections:
        raise ValueError('Expected a COFF data object: ' + symbol)
    header = 20 + (section - 1) * 40
    length, raw, reloc = struct.unpack_from('<III', data, header + 16)
    count = struct.unpack_from('<H', data, header + 32)[0]
    flags = struct.unpack_from('<I', data, header + 36)[0]
    if flags & 0x20 or not flags & (0x40 | 0x80) or flags & 0x20000000:
        raise ValueError('Executable/non-data COFF section')
    if not 0 <= start < start + size <= length:
        raise ValueError('COFF data extent outside section')
    # Section marker symbols are not separate storage definitions. Other data
    # names inside the requested span need an explicit alias model instead.
    for name, offset, other_section, other_kind in symbols:
        if (other_section == section and other_kind == 0 and name != symbol
                and not name.startswith('.') and start <= offset < start + size):
            raise ValueError('Overlapping COFF data definition: ' + name)
    uninitialized = bool(flags & 0x80)
    if uninitialized:
        if raw != 0 or count:
            raise ValueError('Unexpected initialized bytes/relocations in BSS')
        body = bytes(size)
    else:
        if not raw or raw + start + size > len(data):
            raise ValueError('Truncated COFF data bytes')
        body = data[raw + start:raw + start + size]
    if count == 0xffff or (count and (not reloc or reloc + count * 10 > len(data))):
        raise ValueError('Invalid/overflow COFF data relocations')
    fixups = []
    occupied = set()
    for index in range(count):
        offset, target, typ = struct.unpack_from('<IIH', data, reloc + index * 10)
        width = 8 if typ == 1 else 4
        if offset + width <= start or offset >= start + size:
            continue
        if typ != 1 or not start <= offset <= start + size - 8:
            raise ValueError('Only bounded AMD64 ADDR64 data relocations are supported')
        relative = offset - start
        span = set(range(relative, relative + 8))
        if occupied & span:
            raise ValueError('Overlapping COFF data relocations')
        occupied |= span
        fixups.append(dict(offset=relative, symbol=symbol_name(data, target),
                           addend=struct.unpack_from('<Q', body, relative)[0]))
    return dict(symbol=symbol, size=size, section=section, section_offset=start,
                writable=bool(flags & 0x80000000), uninitialized=uninitialized,
                bytes=body, relocations=fixups, object_sha256=hashlib.sha256(data).hexdigest())


def identical_comdat(objects, sources, symbol):
    """Select duplicates only when full bytes and named relocations agree."""
    identities = []
    for source in sorted(sources):
        data = objects[source]
        if len(data) < 20:
            return None
        symbols, _ = coff_symbols(data)
        matches = [s for s in symbols if s[0] == symbol and s[2] > 0 and s[3] == 0x20]
        if len(matches) != 1:
            return None
        _, start, section, _ = matches[0]
        header = 20 + (section - 1) * 40
        length, raw, reloc = struct.unpack_from('<III', data, header + 16)
        count = struct.unpack_from('<H', data, header + 32)[0]
        flags = struct.unpack_from('<I', data, header + 36)[0]
        end = min([s[1] for s in symbols if s[2] == section and s[3] == 0x20 and s[1] > start] + [length])
        if not flags & 0x1000 or count == 0xffff or not 0 <= start < end <= length or raw + end > len(data):
            return None
        fixups = []
        for index in range(count):
            offset, target, typ = struct.unpack_from('<IIH', data, reloc + index * 10)
            if start <= offset < end:
                if typ != 4 or offset + 4 > end:
                    return None
                target_name = symbol_name(data, target)
                symoff = struct.unpack_from('<I', data, 8)[0]
                target_section = struct.unpack_from('<h', data, symoff + target * 18 + 12)[0]
                # A name alone cannot equate two object-local storage bindings.
                # Keep such duplicates ambiguous until their binding is modeled.
                if target_section > 0 and not struct.unpack_from('<I', data, 20 + (target_section - 1) * 40 + 36)[0] & 0x1000:
                    return None
                fixups.append((offset - start, typ, target_name))
        identities.append((data[raw + start:raw + end], sorted(fixups)))
    return sorted(sources)[0] if identities and all(i == identities[0] for i in identities) else None
