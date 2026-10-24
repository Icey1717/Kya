# CActorBasicBox::LoadContext

- **PS2 address:** `0038d810`
- **Current source:** [src/b-witch/ActorBasicBox.cpp](../../../src/b-witch/ActorBasicBox.cpp) line 156
- **Packet/source match:** confirmed after normalizing line endings
- **Function verdict:** **not_real**
- **Live Ghidra:** SLES_514.73, image base 00100000; function decompilation and disassembly reviewed

## Signal-by-signal review

| Signal | Finding | Verdict | PS2 instruction addresses | Rationale |
|---|---|---|---|---|
| call_count | SetValue: port 1, PS2 0. | **not_real** | 0038d83c | The source setter receives exactly 0.0f, which maps to the same zero field value that PS2 writes directly. This is representation only. |

## Fix and validation

No gameplay fix is supported. Keep the setter if its zero input preserves the stored value; validation can compare saved-state restore with mode 1 and field_0x0==0, confirming lifeBase becomes zero and the following flags/behaviour state match.

