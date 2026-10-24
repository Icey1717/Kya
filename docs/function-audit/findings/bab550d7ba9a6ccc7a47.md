# CActorBonusFountain::Init

- PS2 entry: `003ea750` (`SLES_514.73`, image base `00100000`)
- Current source: [ActorBonusFountain.cpp](../../../src/b-witch/ActorBonusFountain.cpp:18); complete current function matches packet.
- Live Ghidra body calls CActor::Init and CAddOnGenerator::Init, then initializes four fountain records and registers their bones. The first loop is at `003ea780-003ea81c`; random values are generated at `003ea7a8` and `003ea7dc`; bone registration loop calls `003ea834` and exits at `003ea844`.

| Finding | Verdict | Evidence and rationale |
|---|---|---|
| `call_count` — `rand: port 0, PS2 2.` | real | The PS2 loop draws two random values per fountain record, setting its animation speed and phase (`003ea7a8`, `003ea7dc`; loop repeats four times). Current source initializes no fountain records or corresponding random state. |
| `call_count` — `RegisterBone: port 0, PS2 1.` | real | The second PS2 loop registers four skeleton bones via `CAnimation::RegisterBone` at `003ea834`; current source stops after add-on generator initialization. |
| `constant_count` — literal 3: port 0, PS2 1 | real | `uVar4 & 3` wraps a four-entry selector into range for the fountain-record lookup (`003ea780`); source has no such record loop or index. |
| `constant_count` — literal -4: port 0, PS2 1 | real | The PS2 signed remainder correction subtracts four when the masked index is negative (`003ea788-003ea790`); source omits that indexing logic with the record initialization. |
| `constant_count` — literal 2147484000: port 0, PS2 2 | real | The divisor converts each of the two per-record random values (`003ea7bc`, `003ea7f0`); source omits both normalized random assignments. |
| `constant_count` — literal 16: port 0, PS2 1 | real | The first random field is scaled to a 16-frame animation angle at `003ea7cc`; current source leaves the field uninitialized. |
| `constant_count` — literal 6.283185: port 0, PS2 1 | real | The second random field is scaled to a full-turn phase at `003ea800`; source has no phase initialization. |
| `constant_count` — literal 4: port 0, PS2 2 | real | Both PS2 loops process four fountain records/bones (`003ea81c`, `003ea844`); current source implements neither loop. |

Suggested fix: restore four-entry record initialization and four bone registrations using the PS2 bone IDs and random formulas. Validation: initialize the fountain with a fixed RNG seed; compare all four record values and verify exactly four bones are registered.
