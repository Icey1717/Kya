"""Inventory serialized sector background hierarchy roots and camera-follow candidates."""
import pathlib
import struct
import sys

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'assets/CDEURO/LEVEL/LEVEL_1')
for path in sorted(root.glob('SECT*.BNK')):
    data = path.read_bytes()
    u32 = lambda p: struct.unpack_from('<I', data, p)[0]
    count, table, types = u32(0x28), u32(0x30) + 8, u32(0x34) + 8
    for i in range(count):
        pair = u32(0x14) or u32(types + 4 * i)
        if pair != (4 << 16 | 2):
            continue
        offset, size = struct.unpack_from('<II', data, table + 16 * i)
        mesh = data[offset:offset + size]
        u = lambda p: struct.unpack_from('<I', mesh, p)[0]
        pos = 16
        while pos < len(mesh) and mesh[pos:pos + 4] != b'HALL':
            pos += u(pos + 8)
        if pos >= len(mesh):
            raise ValueError(f'{path}: HALL missing')
        hashpos = pos + 16
        n = (u(hashpos + 8) - 16) // 16
        hier = [u(hashpos + 16 + 16*j + 8) + 16 for j in range(n)]
        hashes = {hier[j]: struct.unpack_from('<Q', mesh, hashpos + 16 + 16*j)[0] for j in range(n)}
        if not hier:
            continue
        parent_counts = [struct.unpack_from('<h', mesh, h + 0xac)[0] for h in hier]
        selected = max(range(n), key=lambda j: parent_counts[j])
        root_hier = hier[selected]
        descendants = []
        for h in hier:
            cur, seen = h, set()
            while cur not in seen:
                seen.add(cur)
                parent = u(cur + 0x90)
                if not parent:
                    break
                cur = parent + 16
                if cur == root_hier:
                    descendants.append(h)
                    break
        excluded = sum(hashes[h] == 0x4c464e99b2a49e87 for h in descendants)
        print(f'{path.name}: hierarchies={n} root={selected} root_children_field={parent_counts[selected]} '
              f'descendants={len(descendants)} fixed_hash_descendants={excluded}')
