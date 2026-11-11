# OpenCode to Codex bridge

A local stdio MCP server using Python's standard library and `codex app-server`.
No API key or Python packages are needed; sign into the installed Codex CLI first.
It inherits your Codex model configuration and loads the workspace's AGENTS.md.

The OpenCode MCP server is named `codex_bridge`. Ask your OpenCode agent:

> Use codex_bridge to ask Codex to review the renderer changes, then show me its reply.

Tools:

- `ask_codex({"message": "Review ..."})` starts a conversation and returns a job ID.
- `codex_result({"job_id": "...", "wait_seconds": 20})` returns its status and reply.
  Repeat while `status` is `running`.
- `ask_codex({"message": "Follow-up ...", "thread_id": "..."})` continues that conversation.
- `cancel_codex({"job_id": "..."})` stops a job without reverting any changes.

Save the returned `thread_id` to continue after restarting OpenCode. Job IDs and
results are held in bridge memory (the latest 100 jobs); Codex stores conversation
history in its normal local storage. A bridge shutdown cancels active jobs.
There may be three active jobs, each with a separate App Server, and only one
active turn per thread in this bridge. Avoid using the same thread concurrently
from multiple OpenCode instances.

Access defaults to `read-only`. For explicitly authorized coding work, pass
`"sandbox": "workspace-write"`; writes are restricted by Codex to the configured
workspace. The bridge never approves escalation requests or interactive prompts.
Each turn has a 30-minute limit. Replies report failures, including login and
model availability errors. Calls use your normal Codex account usage.

Installed registration (absolute paths are intentional for desktop launches):

```powershell
opencode mcp add codex_bridge --global -- C:/Python314/python.exe G:/repos/Kya/tools/codex_bridge/server.py --codex C:/Users/icey1/AppData/Local/Programs/OpenAI/Codex/bin/codex.exe --cwd G:/repos/Kya
```

This registration is global, but this instance's workspace is fixed to Kya. Register
a separate server name with a different `--cwd` to work on another repository.
Restart OpenCode if it does not discover the new server immediately.

Tests:

```powershell
python -m unittest discover -s tools/codex_bridge -v
```

For an opt-in end-to-end check using two short turns on your Codex account:

```powershell
python tools/codex_bridge/smoke_test.py
```

Installation verification: the global entry was written by OpenCode v2.2.6's
`mcp add`, and `debug config --standalone` confirmed it was loaded. The live MCP
smoke test passed, including a follow-up on the same Codex thread. OpenCode's
authenticated API confirmed `codex_bridge` is `connected` in `G:\repos\Kya`.

This OpenCode version initially listed no MCP servers despite loading the config,
even after `opencode reload`. Explicit runtime activation fixed the connection.
If this recurs after restarting OpenCode, run:

```powershell
./tools/codex_bridge/activate.ps1
```

The helper uses OpenCode's own authenticated CLI, reads only the bridge config,
and reconnects it in the Kya workspace. It does not read service credentials.

Protocol references: [Codex App Server](https://learn.chatgpt.com/docs/app-server)
and [OpenCode MCP configuration](https://opencode.ai/docs/mcp-servers/).
