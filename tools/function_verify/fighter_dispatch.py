"""Recover checked concrete fighter vtable slots from real COFF and PS2 bytes.

This is dispatch evidence, not a transition equivalence result. The executors
must still prove that the object uses these tables at every indirect call.
"""
import argparse
import hashlib
import json
import struct
from compiled import coff_symbols
from build_config import arguments, identity
from client import request


TABLES = [
    ('CActorFighter', 'ActorFactory', '00441a90', [
        (0xa0, '0031baa0', '?AnimEvaluate@CActorFighter@@UEAAXIPEAUedAnmMacroAnimator@@I@Z'),
        (0x7c, '0011e7c0', '?SetState@CActorMovable@@UEAAXHH@Z'),
        (0x54, '0031dd50', '?GetStateCfg@CActorFighter@@UEAAPEAUStateConfig@@H@Z')]),
    ('CBehaviourFighter', 'ActorFighter', '00441a30', [
        (0x34, '00313d80', '?InitState@CBehaviourFighter@@UEAAXH@Z'),
        (0x38, '00313290', '?TermState@CBehaviourFighter@@UEAAXHH@Z')]),
]


def slots(data, table):
    symbols, sections = coff_symbols(data)
    found = [s for s in symbols if s[0] == table and s[2] > 0]
    if len(found) != 1:
        raise ValueError('Missing/ambiguous vtable definition: ' + table)
    _, start, section, _ = found[0]
    if not 1 <= section <= sections:
        raise ValueError('Invalid vtable section')
    header = 20 + (section - 1) * 40
    size, raw, relocation = struct.unpack_from('<III', data, header + 16)
    count = struct.unpack_from('<H', data, header + 32)[0]
    if count == 0xffff or not 0 <= start < size or raw + size > len(data):
        raise ValueError('Invalid vtable bounds or relocation overflow')
    # MSVC emits one vftable per COMDAT section, preceded by its RTTI pointer.
    if any(s[2] == section and s[1] > start for s in symbols):
        raise ValueError('Ambiguous vtable extent')
    symoff, nsyms = struct.unpack_from('<II', data, 8)
    result = {}
    for index in range(count):
        offset, target, kind = struct.unpack_from('<IIH', data, relocation + index * 10)
        if offset < start:
            continue
        if kind != 1 or target >= nsyms or offset + 8 > size or (offset - start) % 8:
            raise ValueError('Invalid AMD64 vtable pointer relocation')
        if data[raw + offset:raw + offset + 8] != bytes(8):
            raise ValueError('Vtable target adjustment requires explicit analysis')
        name = data[symoff + target * 18:symoff + target * 18 + 8]
        if name[:4] == bytes(4):
            position = symoff + nsyms * 18 + struct.unpack_from('<I', name, 4)[0]
            name = data[position:data.index(b'\0', position)]
        symbol = name.rstrip(b'\0').decode()
        slot = offset - start
        if slot in result:
            raise ValueError('Duplicate vtable slot relocation')
        result[slot] = symbol
    if set(result) != set(range(0, size - start, 8)):
        raise ValueError('Vtable has unaccounted bytes or slots')
    return result


def recover(build, port):
    rows, exports = [], {}
    for cls, source, address, expected in TABLES:
        path = build / ('CMakeFiles/Kya.dir/src/b-witch/' + source + '.cpp.obj')
        data = path.read_bytes()
        windows = slots(data, '??_7' + cls + '@@6B@')
        original = request('inspect_data', port, address=address, length=max(s[0] for s in expected) + 4)
        raw = bytes.fromhex(original['hex'])
        if not any(l['namespace'] == cls and l['name'] == '_vt' for l in original['labels']):
            raise ValueError('PS2 vtable identity differs: ' + cls)
        for offset, target, symbol in expected:
            if struct.unpack_from('<I', raw, offset)[0] != int(target, 16):
                raise ValueError('PS2 vtable target differs: ' + cls)
            found = [slot for slot, value in windows.items() if value == symbol]
            if len(found) != 1:
                raise ValueError('Windows target missing/ambiguous: ' + symbol)
            exports[target] = request('export_pcode', port, address=target)
            if not exports[target]['complete']:
                raise ValueError('Incomplete target export: ' + target)
            rows.append(dict(concrete_type=cls, ps2_vtable=address, ps2_offset=offset,
                             ps2_target=target, windows_offset=found[0], windows_symbol=symbol,
                             object=str(path), object_sha256=hashlib.sha256(data).hexdigest(),
                             ps2_table_bytes=raw.hex()))
    hashes = {e['executable_sha256'] for e in exports.values()}
    if len(hashes) != 1:
        raise ValueError('Mixed PS2 executables')
    return dict(scope='Concrete dispatch evidence only; not equivalence', build=identity(build),
                ps2_executable_sha256=hashes.pop(), mappings=rows, exports=exports)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    arguments(parser)
    parser.add_argument('--port', type=int, default=18437)
    args = parser.parse_args()
    output = args.output_root / 'fighter-dispatch.json'
    output.parent.mkdir(parents=True, exist_ok=True)
    output.unlink(missing_ok=True)
    document = recover(args.build_dir.resolve(), args.port)
    output.write_text(json.dumps(document, indent=2) + '\n')
    print(output)


if __name__ == '__main__':
    main()
