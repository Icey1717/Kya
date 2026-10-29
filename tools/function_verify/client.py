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
