# Original executable and Ghidra analysis differences

Checked on 24 October 2026 against the executable supplied by the user:
`D:\Downloads\15th August 2020\elf_extract_100\SLES_514.73`.

SHA-256: `d7d85d0fc342f8f8f684aac6ebbea938cc3f1d9f063d071796f76dbf18b7c0cb`.

The ELF32 little-endian load segment starts at file offset `0x80`, virtual
address `0x00100000`, and contains 3,444,224 file-backed bytes. All of those bytes
were read from the selected Ghidra program through the bridge's read-only
`inspect_data` command and compared with the executable. Capture completed
without errors. Twenty aligned four-byte words differ; all other captured bytes
match. This checks byte identity, not Ghidra's types, function boundaries,
decompiler output, or uninitialized memory.

## Modified control flow

| Instruction address | Function | Original bytes | Ghidra bytes |
|---|---|---|---|
| `00311140` | `CActorFighter::InterpretSlaveMessage` (`00311110`) | `08004000` (`jr v0`) | `65000010` (branch to `003112d8`) |
| `0039136c` | `__ptmf_scall_NOT` (`00391330`) | `08004000` (`jr v0`) | `7c000010` (direct branch) |
| `003ec6f0` | `CBehaviourNativAkasa::SetupTextStyle` (`003ec660`) | `08006000` (`jr v1`) | `79000010` (direct branch) |

These changes can make decompilation show only one switch case. Do not infer
missing or additional port behavior from those truncated decompilations. Recover
the original jump table and inspect the individual case instructions instead.
The audit did not modify the Ghidra program.

For `InterpretSlaveMessage`, the twelve-entry table at `00434370` matches the
executable. Entries 0 through 11 target, respectively:
`003113c0`, `00311148`, `003111e8`, `003112c0`, `003112d8`, `00311400`,
`00311420`, `00311440`, `00311460`, `00311480`, `003114a0`, `003113e0`.
The port's special hit handler in case 4 agrees with that table. The initial
suggestion that it belonged in case 2 was incorrect. Other absent case handlers
are assessed separately in the function report.

## Modified data

The remaining differences are data words. Their presence alone establishes no
port defect or analysis error; some may be deliberate analysis annotations or
reconstructed pointers. Claims depending on them must use the original value.

| Address | Original little-endian bytes | Ghidra bytes |
|---|---|---|
| `0041da84` | `00000000` | `40044300` |
| `0041da88` | `00000000` | `70d94100` |
| `0041da94` | `00000000` | `d8ff4200` |
| `0041da98` | `00000000` | `d0d64100` |
| `0041daa4` | `00000000` | `b0384300` |
| `0041daa8` | `00000000` | `b0514200` |
| `0041dab4` | `00000000` | `90104300` |
| `0041dab8` | `00000000` | `30e54100` |
| `0041dac4` | `00000000` | `08fd4200` |
| `0041dac8` | `00000000` | `e0d44100` |
| `00424d60` | `00000000` | `80574a00` |
| `00424df0` | `00000000` | `80574a00` |
| `00424e34` | `01000000` | `0f000000` |
| `0042684c` | `00000000` | `00774900` |
| `00426850` | `00000000` | `c0784900` |
| `00448808` | `ffffffff` | `607d4d00` |
| `00448818` | `ffffffff` | `60924d00` |

Local captures, comparison results and scripts are retained under
`out/function-audit-orchestrated/`: `original-elf.json`, `memory/`,
`ghidra-byte-differences.json`, `capture_memory.py`, and
`verify_ghidra_bytes.py`. The `out/` directory is ignored by Git; this document
records the reproducible inputs and complete difference list.
