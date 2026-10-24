# CActorAmortos::InterpretMessage

- **PS2 address:** `0032ed10`
- **Current source:** [src/b-witch/ActorAmortos.cpp](../../../src/b-witch/ActorAmortos.cpp:226)
- **Packet/source match:** confirmed
- **Function verdict:** **not_real**

## Signal-by-signal review

| Signal | Finding | Verdict | PS2 instruction addresses | Rationale |
|---|---|---|---|---|
| call_count | IsLockable: port 1, PS2 0. | **not_real** | 0032ed68, 0032ed70 | Current IsLockable() invokes the same virtual actor method; PS2 reaches its vtable slot at 0032ed70. |
| indirect_call_count | Unresolved indirect calls: port 0, PS2 1. Inspect targets before deciding equivalence. | **not_real** | 0032ed68, 0032ed70 | The single PS2 indirect call at 0032ed70 is resolved as IsLockable; it is represented by the named virtual method call in current source. |

## Fix and validation

No fix is supported by these signals. Keep source-level helpers and constructors unless separate evidence shows differing semantics. Validate the corresponding state/message branch or construction path against the cited PS2 blocks.


