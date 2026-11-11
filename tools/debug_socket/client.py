"""Control scene captures through Kya's debug socket."""

import argparse
import json
from pathlib import Path
import socket
import sys
import time


def request(command, port=18438, timeout=15, target=None, **fields):
    with socket.create_connection(("127.0.0.1", port), timeout=timeout) as connection:
        message = {"command": command, "id": 1, **fields}
        if target is not None:
            message["target"] = target
        connection.sendall(json.dumps(message).encode() + b"\n")
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


def screenshot_without_actors(port, timeout):
    status, _ = request("status", port, timeout)
    was_hidden = status["actors_hidden"]
    try:
        hidden, _ = request("actor_visibility", port, timeout, visible=False)
        # Allow queued rendering and temporal buffers to advance before readback.
        deadline = time.monotonic() + timeout
        while True:
            status, _ = request("status", port, max(0.01, deadline - time.monotonic()))
            if status["frame"] >= hidden["frame"] + 3:
                break
            if time.monotonic() >= deadline:
                raise RuntimeError("Timed out waiting for actor-free rendered frames")
            time.sleep(0.02)
        return request("screenshot", port, timeout, target="game")
    finally:
        request("actor_visibility", port, timeout, visible=not was_hidden)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("status", "screenshot", "pause", "resume", "step",
                                            "capture_draws", "actors", "actor_override", "focus_actor", "actor_lighting",
                                            "actor_info", "camera_reset", "camera", "checkpoint", "actor_visibility",
                                            "skip_cutscene"))
    parser.add_argument("--port", type=int, default=18438)
    parser.add_argument("--target", choices=("game", "mesh"), default="game")
    parser.add_argument("--timeout", type=float, default=15)
    parser.add_argument("--output", type=Path,
                        default=Path(__file__).resolve().parents[2] / "out/screenshots/latest.png")
    parser.add_argument("--metadata", type=Path, help="Write the response header, including camera data, as JSON")
    parser.add_argument("--mode", choices=("none", "bind_pose", "texture_only", "isolate_bind_pose", "hide", "show"))
    parser.add_argument("--hide-actors", action="store_true", help="Hide actor draws for a game screenshot, then restore visibility")
    parser.add_argument("--actor", type=int, help="Actor index returned by the actors command")
    parser.add_argument("--manager", help="Checkpoint manager name for the checkpoint command")
    parser.add_argument("--checkpoint-index", type=int, help="Checkpoint index for the checkpoint command")
    parser.add_argument("--name", help="Actor name for the actor_info command")
    parser.add_argument("--camera-position", nargs=3, type=float, metavar=("X", "Y", "Z"),
                        help="Game-space camera position for the camera command")
    parser.add_argument("--camera-yaw", type=float, help="Camera yaw in radians for the camera command")
    parser.add_argument("--camera-pitch", type=float, help="Camera pitch in radians for the camera command")
    args = parser.parse_args()
    fields = {}
    if args.hide_actors and (args.command != "screenshot" or args.target != "game"):
        parser.error("--hide-actors requires screenshot --target game")
    if args.command == "actor_override":
        if args.mode not in ("none", "bind_pose", "texture_only", "isolate_bind_pose") or (args.mode != "none" and args.actor is None):
            parser.error("actor_override requires --mode and an --actor for any active override")
        fields["mode"] = args.mode
        if args.actor is not None:
            fields["actor"] = args.actor
    elif args.command == "actor_visibility":
        if args.mode not in ("hide", "show"):
            parser.error("actor_visibility requires --mode hide or show")
        fields["visible"] = args.mode == "show"
    elif args.command in ("focus_actor", "actor_lighting"):
        if args.actor is None:
            parser.error(f"{args.command} requires --actor")
        fields["actor"] = args.actor
    elif args.command == "checkpoint":
        if args.manager is None or args.checkpoint_index is None:
            parser.error("checkpoint requires --manager and --checkpoint-index")
        fields["manager"] = args.manager
        fields["index"] = args.checkpoint_index
    elif args.command == "actor_info":
        if args.name is None:
            parser.error("actor_info requires --name")
        fields["name"] = args.name
    elif args.command == "camera":
        if args.camera_position is None or args.camera_yaw is None or args.camera_pitch is None:
            parser.error("camera requires --camera-position, --camera-yaw and --camera-pitch")
        fields["position"] = list(args.camera_position)
        fields["yaw"] = args.camera_yaw
        fields["pitch"] = args.camera_pitch
    try:
        if args.hide_actors:
            header, payload = screenshot_without_actors(args.port, args.timeout)
        else:
            header, payload = request(args.command, args.port, args.timeout, args.target, **fields)
        if args.command == "screenshot":
            if header.get("encoding") != "png" or not payload.startswith(b"\x89PNG\r\n\x1a\n"):
                raise RuntimeError("Server did not return a PNG")
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(payload)
            header["path"] = str(args.output.resolve())
        if args.metadata:
            args.metadata.parent.mkdir(parents=True, exist_ok=True)
            args.metadata.write_text(json.dumps(header, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(header, indent=2))
    except (OSError, ValueError, RuntimeError) as error:
        print(f"Debug socket: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
