# Reconnect the persisted bridge if OpenCode has not activated its MCP config.
param(
    [string]$OpenCode = "$env:LOCALAPPDATA/Programs/ai.opencode.desktop/resources/opencode-cli.exe"
)

$ErrorActionPreference = 'Stop'
$workspace = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
$configPath = Join-Path $env:USERPROFILE '.config/opencode/opencode.json'
$config = Get-Content -LiteralPath $configPath -Raw | ConvertFrom-Json
$bridgeConfig = $config.mcp.servers.codex_bridge
if (-not $bridgeConfig) { throw "codex_bridge is missing from $configPath" }
$body = @{ config = $bridgeConfig } | ConvertTo-Json -Depth 10 -Compress
$locationQuery = 'location%5Bdirectory%5D=' + [Uri]::EscapeDataString($workspace)
& $OpenCode api PUT "/api/experimental/mcp/codex_bridge?$locationQuery" --data $body
if ($LASTEXITCODE -ne 0) { throw 'OpenCode could not connect codex_bridge' }
& $OpenCode api GET "/api/mcp?$locationQuery"
if ($LASTEXITCODE -ne 0) { throw 'OpenCode could not read MCP status' }
