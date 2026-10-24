# CActorBonePhysics::FUN_003c2910

- **PS2 address:** `003c2910`
- **Current source:** [src/b-witch/ActorBonePhysics.cpp](../../../src/b-witch/ActorBonePhysics.cpp) line 344
- **Packet/source match:** confirmed after line-ending normalization
- **Function verdict:** **not_real**
- **Live Ghidra:** SLES_514.73, image base 00100000; decompilation and disassembly reviewed

## Signal-by-signal review

| Signal | Finding | Verdict | PS2 instruction addresses | Rationale |
|---|---|---|---|---|
| call_count | SetLink: port 1, PS2 0. | **not_real** | 003c292c, 003c2938, 003c2944, 003c2950, 003c295c, 003c2968, 003c2970 | SetLink is inlined by PS2. Current SetLink also writes back existing pointA/pointB values and configures the same three min/max distances; no endpoint changes. |
| constant_count | Literal 3: port 1, PS2 0. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |
| constant_count | Literal 0.1: port 1, PS2 0. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |
| constant_count | Literal 0.2: port 1, PS2 0. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |
| constant_count | Literal 8: port 0, PS2 5. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |
| constant_count | Literal 1036831949: port 0, PS2 3. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |
| constant_count | Literal 12: port 0, PS2 3. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |
| constant_count | Literal 1045220557: port 0, PS2 3. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |
| constant_count | Literal 4: port 0, PS2 2. | **not_real** | 003c2914, 003c292c, 003c2938, 003c295c, 003c2968, 003c2970 | The PS2 literal is a link field offset or raw float encoding in its three unrolled writes. Source uses SetLink's named fields and decimal float parameters, which produce the same values. |

## Fix and validation

No gameplay fix is supported. Keep the SetLink abstraction; if reproducing instruction shape is ever required, the three identical calls can be expanded, but this is not needed for behavior. Validate the three links receive the same min/max distances and `field_0x74` remains 90 degrees.

