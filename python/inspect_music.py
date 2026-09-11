"""Read-only inventory of music in unpacked Eden LEVELIOP.BNK archives."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct


def Read(data, offset, fmt):
    if offset < 0 or offset + struct.calcsize(fmt) > len(data):
        raise ValueError(f"truncated field at {offset:#x}")
    return struct.unpack_from(fmt, data, offset)


def Entries(data):
    if data[8:12] != b"KNAB":
        raise ValueError("not an Eden bank")
    flags, = Read(data, 12, '<I')
    if flags & 1:
        raise ValueError("compressed archive is not supported by this inspector")
    count, = Read(data, 40, '<I')
    table, types = Read(data, 48, '<II')
    if count > len(data) // 16:
        raise ValueError("invalid entry count")
    for index in range(count):
        subtype, category = Read(data, 8 + types + index * 4, '<HH')
        offset, size = Read(data, 8 + table + index * 16, '<II')
        if offset + size > len(data):
            raise ValueError(f"entry {index} is out of bounds")
        if category == 3 and subtype in (5, 6, 7):
            yield index, subtype, offset, data[offset:offset + size]


def Song(data):
    if data[:8] != b'IECSsreV' or data[16:24] != b'IECSuqeS':
        raise ValueError('unsupported song header')
    midi, = Read(data, 36, '<I')
    if data[midi:midi + 8] != b'IECSidiM':
        raise ValueError('missing MIDI chunk')
    size, maximum, relative = Read(data, midi + 8, '<III')
    if maximum != 0 or midi + size > len(data):
        raise ValueError('unsupported MIDI block count or size')
    block = midi + relative
    offset, ppqn = Read(data, block, '<IH')
    cursor, end = block + offset, midi + size
    running, skip, tick = 0, False, 0
    events, controls, programs = Counter(), Counter(), set()

    def Byte():
        nonlocal cursor
        if cursor >= end:
            raise ValueError(f'truncated event at {cursor:#x}')
        value = data[cursor]
        cursor += 1
        return value

    def Variable():
        value = 0
        for _ in range(4):
            byte = Byte()
            value = (value << 7) | (byte & 127)
            if byte < 128:
                return value
        raise ValueError('overlong delta')

    while cursor < end:
        eventOffset = cursor
        if not skip:
            tick += Variable()
        skip = False
        status = Byte()
        if status < 128:
            cursor -= 1
            status = running
        elif status != 255:
            running = status
        kind = status >> 4
        events[f'{status & 240:02x}'] += 1
        if kind in (8, 9, 11, 12, 14):
            first = Byte()
            last = Byte() if kind in (9, 11, 14) else first
            skip = bool(last & 128)
            last &= 127
            if kind == 11:
                controls[f'{first}:{last}'] += 1
            if kind == 12:
                programs.add(last)
        elif status == 255:
            meta = Byte()
            if meta == 47:
                return dict(ppqn=ppqn, ticks=tick, events=dict(events),
                            controls=dict(controls), programs=sorted(programs))
            if meta != 81 or Byte() != 3:
                raise ValueError(f'unsupported meta {meta:#x} at {eventOffset:#x}')
            Byte(); Byte(); Byte()
        else:
            raise ValueError(f'unsupported event {status:#x} at {eventOffset:#x}')
    raise ValueError('missing end of song')


def Main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('path', type=Path, help='archive or directory containing LEVELIOP.BNK files')
    args = parser.parse_args()
    paths = sorted(args.path.rglob('LEVELIOP.BNK')) if args.path.is_dir() else [args.path]
    report = []
    for path in paths:
        for index, subtype, offset, payload in Entries(path.read_bytes()):
            entry = dict(archive=str(path), index=index, subtype=subtype,
                         offset=offset, size=len(payload), sha256=hashlib.sha256(payload).hexdigest())
            if subtype == 5:
                try:
                    entry.update(Song(payload))
                except ValueError as error:
                    entry['error'] = str(error)
            report.append(entry)
    print(json.dumps(report, indent=2))
    return int(any('error' in entry for entry in report))


if __name__ == '__main__':
    raise SystemExit(Main())
