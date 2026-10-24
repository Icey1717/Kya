# CActorWolfen::StateEscapeJumpRecept

PS2 address: 00175870 (SLES_514.73, base 00100000). Current source: src/b-witch/ActorWolfen.cpp:6445. Full packet source was confirmed in the current source file.

Function verdict: **real**.

## Signal by signal

### unfinished: Contains an implementation guard; inspect compiled behavior.

- Verdict: **real**.
- Reason: Guard-only source omits animation-completion state selection and dynamic update.
- Supporting PS2 instructions: 00175890, 001758d4, 001758fc

### call_count: SetState: port 0, PS2 1.

- Verdict: **real**.
- Reason: Animation completion transitions to state 0x72.
- Supporting PS2 instructions: 001758cc, 001758d4

### call_count: ManageDyn: port 0, PS2 1.

- Verdict: **real**.
- Reason: Always updates movement at speed 4.0 with flags 0x1002023b.
- Supporting PS2 instructions: 001758e4, 001758fc

### constant_count: Literal 2: port 0, PS2 1.

- Verdict: **real**.
- Reason: Bit 2 of animation flags is tested for completion.
- Supporting PS2 instructions: 001758b0

### constant_count: Literal 114: port 0, PS2 1.

- Verdict: **real**.
- Reason: 114 (0x72) is the state transition argument.
- Supporting PS2 instructions: 001758cc

### constant_count: Literal 4: port 0, PS2 1.

- Verdict: **real**.
- Reason: 4.0 is the dynamic update speed.
- Supporting PS2 instructions: 001758e4

### constant_count: Literal 268567099: port 0, PS2 1.

- Verdict: **real**.
- Reason: 0x1002023b is the dynamic update flag argument.
- Supporting PS2 instructions: 001758f4, 001758fc

## Suggested fix

Restore animation completion test, SetState(0x72,-1), and ManageDyn; validate both completion states.

## Validation

Use the scenarios in the suggested fix in the Windows x64 Debug build and smoke test after a future fix. For platform render differences, compare shadow output to a PS2 reference. No code was changed in this audit.

