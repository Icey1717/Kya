"""Request game status or a full-resolution screenshot from Kya's debug socket."""

import argparse
import json
from pathlib import Path
import socket
import sys


def request(command, port=18438, timeout=15):
    with socket.create_connection(("127.0.0.1", port), timeout=timeout) as connection:
        connection.sendall(json.dumps({"command": command, "id": 1}).encode() + b"\n")
        with connection.makefile("rb") as stream:
            line = stream.readline(65537)
            if not line.endswith(b"\n") or len(line) > 65536:
                raise RuntimeError("Invalid or missing response header")
            header = json.loads(line)
            if not header.get("ok"):
                raise RuntimeError(header.get("error", "Request failed"))
            size = header.get("bytes", 0)
            if not isinstance(size, int) or not 0 <= size <= 256 * 1024 * 1024:
                raise RuntimeError("Invalid response payload size")
            payload = stream.read(size)
            if len(payload) != size:
                raise RuntimeError("Truncated response payload")
            return header, payload


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("status", "screenshot"))
    parser.add_argument("--port", type=int, default=18438)
    parser.add_argument("--timeout", type=float, default=15)
    parser.add_argument("--output", type=Path,
                        default=Path(__file__).resolve().parents[2] / "out/screenshots/latest.png")
    args = parser.parse_args()
    try:
        header, payload = request(args.command, args.port, args.timeout)
        if args.command == "screenshot":
            if header.get("encoding") != "png" or not payload.startswith(b"\x89PNG\r\n\x1a\n"):
                raise RuntimeError("Server did not return a PNG")
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(payload)
            header["path"] = str(args.output.resolve())
        print(json.dumps(header, indent=2))
    except (OSError, ValueError, RuntimeError) as error:
        print(f"Debug socket: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
