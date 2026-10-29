# Totala Ghidra extension

Persistent localhost MCP bridge plugin. Build with:

## Ghidra 10.0 (current Kya setup)

From the repository root, run:

```powershell
python tools/ghidra-module/totala/build_ghidra10.py `
  --ghidra G:/FromGamesAndDev/Ghidra/ghidra_10.0_PUBLIC_20210621/ghidra_10.0_PUBLIC `
  --jdk G:/jdk/jdk-21.0.9
```

Requires Python 3 and JDK 21. Run Ghidra itself with Java 21 as well: the bridge
uses modern Java syntax and the builder emits Java 21 classes. Compilation uses
the installed Ghidra **10.0** API jars, not the 12.x jars.

The archive is written to `dist/ghidra_10.0_PUBLIC_YYYYMMDD_totala.zip`.
In Ghidra's project window, use **File > Install Extensions**, remove the old
Totala extension and restart if prompted, then add the new ZIP and restart.
Enable the Totala plugin in the CodeBrowser tool if necessary. Its default
loopback port is 18437; open the desired program before sending requests.

This builder bypasses Ghidra 10.0's Gradle script, which uses APIs removed from
the installed Gradle 9.4.1. It also writes explicit directory entries before
files and validates their order: Ghidra 10.0's installer requires these entries.
A successful compile alone does not validate installation or runtime behavior.

Read-only smoke checks from the repository root:

```powershell
python tools/function_verify/client.py status
python tools/function_verify/client.py export_pcode --address 00100460 --output out/verification/00100460.json
```

The new command is available through the JSON-lines bridge even if an external
MCP wrapper has not yet registered an `export_pcode` tool. The client above talks
directly to that bridge. Raw request: `{"command":"export_pcode","address":"00100460","limit":4096}`.
The optional operation limit is capped at 65536; exceeding it fails the request
without returning a partial export. `ExportPcode.java` provides a Script Manager
alternative taking an entry address and output filename.

Installation errors are logged in
`%USERPROFILE%/.ghidra/.ghidra_10.0_PUBLIC/application.log`.
An error opening `Extensions/totala/lib/totala.jar` during extraction can indicate
a ZIP missing directory entries; use the builder above rather than hand-zipping files.

## Newer Ghidra versions

```powershell
$env:GHIDRA_INSTALL_DIR = 'C:\path\to\ghidra'
gradle buildExtension
```

See `../../ghidra-mcp/README.md` for installation, configuration, protocol,
persistence guarantees, and migration instructions.
