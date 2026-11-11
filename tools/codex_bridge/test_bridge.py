"""Protocol and lifecycle checks; the fake Codex never makes network requests."""

import asyncio
import json
from pathlib import Path
import subprocess
import sys
import unittest

from server import Bridge


class FakeBridge(Bridge):
    async def run_job(self, job, message, sandbox):
        try:
            await asyncio.sleep(0.02)
            job.update(status="completed", thread_id=job.get("thread_id") or "fake-thread", reply=message)
        except asyncio.CancelledError:
            job["status"] = "cancelled"


class BridgeTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.bridge = FakeBridge("unused", Path(__file__).parent)

    async def asyncTearDown(self):
        for task in self.bridge.tasks.values():
            task.cancel()
        await asyncio.gather(*self.bridge.tasks.values(), return_exceptions=True)

    async def test_reply_and_followup(self):
        job = await self.bridge.call("ask_codex", {"message": "remember 42"})
        reply = await self.bridge.call("codex_result", {"job_id": job["job_id"], "wait_seconds": 1})
        self.assertEqual(reply["reply"], "remember 42")
        followup = await self.bridge.call("ask_codex", {"message": "recall", "thread_id": reply["thread_id"]})
        self.assertEqual(followup["thread_id"], "fake-thread")
        with self.assertRaisesRegex(ValueError, "already running"):
            await self.bridge.call("ask_codex", {"message": "overlap", "thread_id": "fake-thread"})

    async def test_cancel_before_job_starts(self):
        job = await self.bridge.call("ask_codex", {"message": "cancel"})
        reply = await self.bridge.call("cancel_codex", {"job_id": job["job_id"]})
        self.assertEqual(reply["status"], "cancelled")

    async def test_invalid_and_busy(self):
        for args in ({"message": " "}, {"message": "hi", "sandbox": "danger-full-access"}):
            with self.assertRaises(ValueError):
                await self.bridge.call("ask_codex", args)
        for _ in range(3):
            await self.bridge.call("ask_codex", {"message": "hello"})
        with self.assertRaisesRegex(ValueError, "Three"):
            await self.bridge.call("ask_codex", {"message": "fourth"})

    async def test_missing_executable_reports_failure(self):
        bridge = Bridge("nonexistent-codex-bridge-test-executable", Path(__file__).parent)
        job = await bridge.call("ask_codex", {"message": "hello"})
        result = await bridge.call("codex_result", {"job_id": job["job_id"], "wait_seconds": 2})
        self.assertEqual(result["status"], "failed")
        self.assertTrue(result["error"])


class ProtocolTests(unittest.TestCase):
    def test_stdio_handshake_and_errors(self):
        proc = subprocess.Popen([sys.executable, str(Path(__file__).with_name("server.py")), "--codex", "unused"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, encoding="utf-8")
        try:
            for request in (
                {"id": 1, "method": "initialize", "params": {"protocolVersion": "2024-11-05", "capabilities": {}, "clientInfo": {"name": "test", "version": "1"}}},
                {"id": 2, "method": "tools/list"},
                {"id": 3, "method": "tools/call", "params": {"name": "codex_result", "arguments": {"job_id": "missing"}}},
                {"id": 4, "method": "not-a-method"},
            ):
                proc.stdin.write(json.dumps({"jsonrpc": "2.0", **request}) + "\n")
                proc.stdin.flush()
                response = json.loads(proc.stdout.readline())
                self.assertEqual(response["id"], request["id"])
                if request["id"] == 2:
                    self.assertEqual(len(response["result"]["tools"]), 3)
                if request["id"] == 3:
                    self.assertTrue(response["result"]["isError"])
                if request["id"] == 4:
                    self.assertEqual(response["error"]["code"], -32601)
            proc.stdin.close()
            self.assertEqual(proc.wait(timeout=5), 0)
        finally:
            if proc.poll() is None:
                proc.kill()
            proc.stdout.close()
            proc.stderr.close()


if __name__ == "__main__":
    unittest.main()
