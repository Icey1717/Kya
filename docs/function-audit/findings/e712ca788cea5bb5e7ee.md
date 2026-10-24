# CActorBrazul::BuildBehaviour

- PS2 address: `003e94d0` (SLES_514.73, image base 00100000)
- Source: `src/b-witch/ActorBrazul.cpp:417`
- Source matches current packet: **true** (full function text, normalized line endings)
- Overall verdict: **not_real**
- Live evidence: `out/function-audit-orchestrated/evidence/e712ca788cea5bb5e7ee.json`

All three constructors select the same behaviour ids and object sizes as the PS2 branches; allocation count and literals are representation-only.

| Exact packet signal | Verdict | PS2 instruction evidence | Assessment |
|---|---|---|---|
| call_count: operator_new: port 0, PS2 3. | not_real | `003e9578` | `new CBehaviourBrazulB` lowers to operator_new(0x14), followed by its vtable initialization; both select behaviour 0x1b. PS2 branch at 003e9578 allocates 0x14 and installs BrazulB vtable; source branch constructs CBehaviourBrazulB. |
| constant_count: Literal 20: port 0, PS2 1. | not_real | `003e957c` | The 0x14 allocation size is the concrete object size supplied by typed new. PS2 passes 0x14 at 003e957c; source uses the class allocation. |
| constant_count: Literal 40: port 0, PS2 1. | not_real | `003e954c` | The 0x28 allocation size is the concrete object size supplied by typed new. PS2 passes 0x28 at 003e954c; source constructs CBehaviourBrazulA. |
| constant_count: Literal 192: port 0, PS2 1. | not_real | `003e9504` | The 0xc0 allocation size is the concrete object size supplied by typed new. PS2 passes 0xc0 at 003e9504; source constructs CBehaviourFighterBrazul. |

## Suggested fix and validation

Fix: none. Validation: confirm behaviour types 0x1b, 0x1a, and 3 map to class sizes 0x14, 0x28, and 0xc0.

