"""Dependency-free stdio MCP bridge to the installed Codex App Server."""

import argparse
import asyncio
import json
import os
from pathlib import Path
import shutil
import sys
import uuid


TOOLS = [
    {
        "name": "ask_codex",
        "description": "Message a dedicated Codex agent. Returns a job ID immediately; call codex_result to get its reply. Reuse thread_id for follow-ups, including after restarting this bridge. Defaults to read-only repository access. Do not send secrets. Only request workspace-write when the user has authorized edits.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "message": {"type": "string", "minLength": 1},
                "thread_id": {"type": "string", "minLength": 1},
                "sandbox": {"type": "string", "enum": ["read-only", "workspace-write"], "default": "read-only"},
            },
            "required": ["message"],
            "additionalProperties": False,
        },
    },
    {
        "name": "codex_result",
        "description": "Read a Codex job's status, thread_id, reply, and any error. Wait up to 25 seconds per call; repeat while running. Job IDs last for this MCP process; thread IDs persist in Codex.",
        "inputSchema": {
            "type": "object",
            "properties": {"job_id": {"type": "string"}, "wait_seconds": {"type": "integer", "minimum": 0, "maximum": 25, "default": 20}},
            "required": ["job_id"],
            "additionalProperties": False,
        },
    },
    {
        "name": "cancel_codex",
        "description": "Cancel a running Codex job. Its dedicated App Server process is stopped; completed file edits are not undone.",
        "inputSchema": {"type": "object", "properties": {"job_id": {"type": "string"}}, "required": ["job_id"], "additionalProperties": False},
    },
]


class Bridge:
    def __init__(self, codex, cwd):
        self.codex = codex
        self.cwd = str(Path(cwd).resolve(strict=True))
        self.jobs = {}
        self.tasks = {}

    async def run_job(self, job, message, sandbox):
        proc = None
        stderr_task = None
        diagnostics = []
        replies = {}
        serial = 0

        async def read_stderr():
            while line := await proc.stderr.readline():
                diagnostics.append(line.decode("utf-8", "replace").rstrip())
                del diagnostics[:-12]

        async def receive():
            line = await proc.stdout.readline()
            if not line:
                raise RuntimeError("Codex App Server disconnected. " + "\n".join(diagnostics))
            msg = json.loads(line)
            # Never silently approve tool execution or user-input requests.
            if "method" in msg and "id" in msg:
                send({"id": msg["id"], "error": {"code": -32601, "message": "Interactive requests are unsupported by this bridge. Report the blocked action to the caller."}})
            if msg.get("method") == "item/completed":
                item = msg.get("params", {}).get("item", {})
                if item.get("type") == "agentMessage":
                    replies[item["id"]] = item.get("text", "")
                    job["reply"] = "\n\n".join(replies.values())
            return msg

        def send(msg):
            proc.stdin.write((json.dumps(msg) + "\n").encode())

        async def request(method, params):
            nonlocal serial
            serial += 1
            request_id = serial
            send({"id": request_id, "method": method, "params": params})
            await proc.stdin.drain()
            while True:
                msg = await receive()
                if msg.get("id") == request_id and "method" not in msg:
                    if "error" in msg:
                        raise RuntimeError(json.dumps(msg["error"]))
                    return msg["result"]

        try:
            env = os.environ.copy()
            env.setdefault("CODEX_HOME", str(Path.home() / ".codex"))
            proc = await asyncio.create_subprocess_exec(
                self.codex, "app-server", "--listen", "stdio://",
                cwd=self.cwd, env=env,
                stdin=asyncio.subprocess.PIPE, stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE, limit=16 * 1024 * 1024,
                creationflags=0x08000000 if os.name == "nt" else 0,
            )
            stderr_task = asyncio.create_task(read_stderr())
            async with asyncio.timeout(1800):
                await request("initialize", {"clientInfo": {"name": "opencode_codex_bridge", "title": "OpenCode Codex Bridge", "version": "1.0.0"}})
                send({"method": "initialized", "params": {}})
                params = {"cwd": self.cwd, "approvalPolicy": "never", "sandbox": sandbox}
                if job.get("thread_id"):
                    params.update(threadId=job["thread_id"], excludeTurns=True)
                    result = await request("thread/resume", params)
                else:
                    result = await request("thread/start", params)
                job["thread_id"] = result["thread"]["id"]
                result = await request("turn/start", {"threadId": job["thread_id"], "input": [{"type": "text", "text": message}]})
                turn_id = result["turn"]["id"]
                while True:
                    msg = await receive()
                    params = msg.get("params", {})
                    if msg.get("method") == "turn/completed" and params.get("turn", {}).get("id") == turn_id:
                        turn = params["turn"]
                        job["status"] = turn["status"]
                        if turn.get("error"):
                            job["error"] = turn["error"]
                        break
        except asyncio.CancelledError:
            job["status"] = "cancelled"
        except Exception as exc:
            job.update(status="failed", error=str(exc) or type(exc).__name__)
        finally:
            if proc is not None:
                if proc.returncode is None:
                    proc.terminate()
                await proc.wait()
            if stderr_task:
                await stderr_task

    async def call(self, name, args):
        if name == "ask_codex":
            if set(args) - {"message", "thread_id", "sandbox"}:
                raise ValueError("Unknown argument")
            message = args.get("message")
            sandbox = args.get("sandbox", "read-only")
            thread_id = args.get("thread_id")
            if not isinstance(message, str) or not message.strip():
                raise ValueError("message must be a nonempty string")
            if thread_id is not None and (not isinstance(thread_id, str) or not thread_id.strip()):
                raise ValueError("thread_id must be a nonempty string")
            if sandbox not in ("read-only", "workspace-write"):
                raise ValueError("Unsupported sandbox")
            active = [j for j in self.jobs.values() if j["status"] == "running"]
            if len(active) >= 3:
                raise ValueError("Three Codex jobs are already running")
            if thread_id and any(j.get("thread_id") == thread_id for j in active):
                raise ValueError("A turn is already running on this thread")
            if len(self.jobs) >= 100:
                oldest = next(k for k, v in self.jobs.items() if v["status"] != "running")
                del self.jobs[oldest]
                self.tasks.pop(oldest, None)
            job_id = uuid.uuid4().hex
            job = {"job_id": job_id, "status": "running", "thread_id": thread_id, "reply": "", "sandbox": sandbox, "cwd": self.cwd}
            self.jobs[job_id] = job
            self.tasks[job_id] = asyncio.create_task(self.run_job(job, message, sandbox))
            return dict(job)
        if name not in ("codex_result", "cancel_codex"):
            raise ValueError("Unknown tool: " + name)
        job_id = args.get("job_id")
        if job_id not in self.jobs:
            raise ValueError("Unknown job ID; jobs are held by the current bridge process")
        task = self.tasks[job_id]
        if name == "cancel_codex" and not task.done():
            task.cancel()
            await asyncio.gather(task, return_exceptions=True)
            self.jobs[job_id]["status"] = "cancelled"
        if name == "codex_result":
            wait = args.get("wait_seconds", 20)
            if type(wait) is not int or not 0 <= wait <= 25:
                raise ValueError("wait_seconds must be an integer from 0 to 25")
            await asyncio.wait({task}, timeout=wait)
        return dict(self.jobs[job_id])


async def serve(bridge):
    requests = {}

    def emit(msg):
        print(json.dumps({"jsonrpc": "2.0", **msg}, ensure_ascii=False), flush=True)

    async def handle(msg):
        request_id = msg.get("id")
        method = msg.get("method")
        if request_id is None:
            if method == "notifications/cancelled":
                task = requests.get(msg.get("params", {}).get("requestId"))
                if task:
                    task.cancel()
            return
        try:
            if method == "initialize":
                result = {"protocolVersion": "2024-11-05", "capabilities": {"tools": {}}, "serverInfo": {"name": "codex-bridge", "version": "1.0.0"}, "instructions": "Use ask_codex then poll codex_result while running. Save thread_id and pass it to ask_codex for follow-ups. Default access is read-only."}
            elif method == "ping":
                result = {}
            elif method == "tools/list":
                result = {"tools": TOOLS}
            elif method == "tools/call":
                params = msg.get("params", {})
                try:
                    value = await bridge.call(params["name"], params.get("arguments", {}))
                    result = {"content": [{"type": "text", "text": json.dumps(value)}], "isError": value.get("status") == "failed"}
                except (ValueError, KeyError, TypeError) as exc:
                    result = {"content": [{"type": "text", "text": str(exc)}], "isError": True}
            else:
                emit({"id": request_id, "error": {"code": -32601, "message": "Method not found"}})
                return
            emit({"id": request_id, "result": result})
        finally:
            requests.pop(request_id, None)

    try:
        while line := await asyncio.to_thread(sys.stdin.readline):
            try:
                msg = json.loads(line)
                if not isinstance(msg, dict):
                    raise ValueError("Expected an object")
            except (ValueError, json.JSONDecodeError):
                emit({"id": None, "error": {"code": -32700, "message": "Invalid JSON-RPC message"}})
                continue
            task = asyncio.create_task(handle(msg))
            if "id" in msg:
                requests[msg["id"]] = task
    finally:
        tasks = list(requests.values()) + list(bridge.tasks.values())
        for task in tasks:
            task.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--codex", default=shutil.which("codex"))
    parser.add_argument("--cwd", default=str(Path(__file__).resolve().parents[2]))
    options = parser.parse_args()
    if not options.codex:
        parser.error("Codex executable not found; supply --codex")
    sys.stdin.reconfigure(encoding="utf-8")
    sys.stdout.reconfigure(encoding="utf-8")
    asyncio.run(serve(Bridge(options.codex, options.cwd)))
