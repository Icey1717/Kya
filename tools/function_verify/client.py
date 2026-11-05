"""Read-only client for the Totala JSON-lines bridge."""
import argparse
import json
import socket
from pathlib import Path


def request(command, port=18437, **kwargs):
    with socket.create_connection(('127.0.0.1', port), timeout=70) as connection:
        connection.sendall((json.dumps(dict(command=command, **kwargs)) + '\n').encode())
        with connection.makefile('rb') as stream:
            line = stream.readline(32 * 1024 * 1024)
        response = json.loads(line)
    if not response.get('ok'):
        raise RuntimeError(response)
    return response['result']


def export_pcode(address, port=18437):
    """Read byte provenance from a legacy live EE bridge without reinstalling it.

    New exporters already supply these fields. Legacy R5900 instructions are
    fixed-width; inspect_data supplies exact bytes including their delay slots.
    The second export rejects a changed project/image or changed instruction
    stream during collection. This never modifies the live Ghidra project.
    """
    exported = request('export_pcode', port, address=address, limit=65536)
    if exported.get('instruction_semantics_version') == 2:
        return exported
    if exported.get('language') != 'r5900:LE:32:default':
        raise ValueError('Legacy instruction provenance supports R5900 only')
    # Group contiguous ranges to avoid a socket request for every instruction.
    intervals = []
    for instruction in exported['instructions']:
        start = int(instruction['address'], 16)
        end = start + 4 * (1 + instruction.get('delay_slots', 0))
        if intervals and start <= intervals[-1][1] and end - intervals[-1][0] <= 4096:
            intervals[-1] = (intervals[-1][0], max(end, intervals[-1][1]))
        else:
            intervals.append((start, end))
    storage = {}
    for start, end in intervals:
        data = request('inspect_data', port, address=f'{start:08x}', length=end - start)
        raw = bytes.fromhex(data['hex'])
        if len(raw) != end - start:
            raise ValueError('Truncated instruction-byte provenance')
        storage.update((start + offset, value) for offset, value in enumerate(raw))
    repeated = request('export_pcode', port, address=address, limit=65536)
    if repeated != exported:
        raise ValueError('Live export changed during provenance collection')
    for instruction in exported['instructions']:
        start = int(instruction['address'], 16)
        instruction['bytes'] = bytes(storage[start + i] for i in range(4)).hex()
        instruction['length'] = 4
        instruction['mnemonic'] = instruction['text'].split(' ')[0]
        instruction['delay_slot_instructions'] = [dict(address=f'{start + 4 * (index + 1):08x}',
            bytes=bytes(storage[start + 4 * (index + 1) + byte] for byte in range(4)).hex(), length=4)
            for index in range(instruction.get('delay_slots', 0))]
    exported['instruction_semantics_version'] = 2
    exported['exporter_version'] = 'legacy-bridge-plus-read-only-bytes-1'
    return exported


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['status', 'export_pcode'])
    parser.add_argument('--address')
    parser.add_argument('--port', type=int, default=18437)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    data = request(args.command, args.port, **({'address': args.address} if args.address else {}))
    rendered = json.dumps(data, indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered + '\n', encoding='utf-8')
    else:
        print(rendered)
