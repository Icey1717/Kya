"""Opt-in live MCP check. Uses two short Codex turns on the signed-in account."""

import asyncio
import json
from pathlib import Path
import sys


async def main():
    proc = await asyncio.create_subprocess_exec(
        sys.executable, str(Path(__file__).with_name("server.py")),
        stdin=asyncio.subprocess.PIPE, stdout=asyncio.subprocess.PIPE,
    )
    serial = 0

    async def rpc(method, params):
        nonlocal serial
        serial += 1
        proc.stdin.write((json.dumps({"jsonrpc": "2.0", "id": serial, "method": method, "params": params}) + "\n").encode())
        await proc.stdin.drain()
        response = json.loads(await asyncio.wait_for(proc.stdout.readline(), 35))
        if "error" in response:
            raise RuntimeError(response["error"])
        return response["result"]

    async def tool(name, arguments):
        result = await rpc("tools/call", {"name": name, "arguments": arguments})
        value = json.loads(result["content"][0]["text"])
        if result.get("isError"):
            raise RuntimeError(value)
        return value

    async def ask(message, thread_id=None):
        args = {"message": message}
        if thread_id:
            args["thread_id"] = thread_id
        job = await tool("ask_codex", args)
        print("Started job " + job["job_id"], flush=True)
        while job["status"] == "running":
            job = await tool("codex_result", {"job_id": job["job_id"], "wait_seconds": 20})
            print(json.dumps(job), flush=True)
        if job["status"] != "completed":
            raise RuntimeError(job)
        return job

    try:
        await rpc("initialize", {"protocolVersion": "2024-11-05", "capabilities": {}, "clientInfo": {"name": "bridge-smoke-test", "version": "1"}})
        tools = await rpc("tools/list", {})
        assert len(tools["tools"]) == 3
        async with asyncio.timeout(240):
            first = await ask("Bridge smoke test. Do not use any tools or inspect files. Remember the token cobalt-otter-739. Reply with exactly that token.")
            assert "cobalt-otter-739" in first["reply"], first
            second = await ask("Do not use tools. What exact token did I ask you to remember? Reply only with the token.", first["thread_id"])
            assert "cobalt-otter-739" in second["reply"], second
            assert second["thread_id"] == first["thread_id"]
            print("PASS: MCP handshake, real Codex response, and persistent thread follow-up", flush=True)
    finally:
        proc.stdin.close()
        await asyncio.wait_for(proc.wait(), 10)


if __name__ == "__main__":
    asyncio.run(main())
