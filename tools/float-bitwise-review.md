# Float bitwise candidate review

Compared all 17 candidates from `out/reports/float-bitwise-audit.json` against
Ghidra's `SLES_514.73` decompilation and PS2 disassembly. Source line numbers below
refer to the initial scanner report. No structure layouts were changed.

| Source / initial line | PS2 function / instruction evidence | Result |
| --- | --- | --- |
| ActorNativ.cpp:4297 | ArenaUpdateDisplayBorderSize `0x003ed150`; `max.s` at `0x003ed380` | Fixed float maximum, inside existing implementation guard. |
| ActorPatternService.cpp:337 | FUN_003a6320 `0x003a6320`; `min.s` at `0x003a64d4` | Fixed float minimum; preserves fractional trail render factors. |
| ChessBoard.cpp:1179 | _PermutPawns `0x003529d0`; `min.s` at `0x00352c54` | Fixed float minimum. |
| edParticles.cpp:962 | edPartGenNewPosAndSpeed `0x002787a0`; mask/OR/store at `0x00279608–0x00279614`, float load at `0x0027961c` | Fixed bit reinterpretation without advancing the RNG again. |
| edParticles.cpp:1163 | Same function; OR/store/load at `0x0027a370–0x0027a378` | Fixed bit reinterpretation, inside existing implementation guard. |
| edParticles.cpp:1577 | edPartGeneratorNewParticle `0x0027b220`; OR/store/load at `0x0027b738–0x0027b740` | Fixed bit reinterpretation, inside existing implementation guard. |
| edParticles.cpp:1588 | Same function; OR/store/load at `0x0027b5bc–0x0027b5c8` | Fixed bit reinterpretation, inside existing implementation guard. |
| edParticles.cpp:1589 | Same function; OR/store/load at `0x0027b620–0x0027b628` | Fixed bit reinterpretation, inside existing implementation guard. |
| ActorHero_Wind.cpp:615,639,658 | StateHeroGlide `0x00149a10`; unsigned-to-float conversion sequences at `0x00149bcc`, `0x00149c68`, `0x00149ce0`, including `cvt.s.w` then doubling | Retained integer conversion for click counts. Port input stores these counts as numeric floats. |
| ActorHero_Wind.cpp:1267,1283 | StateHeroCheatFly `0x0014b6d0`; unsigned-to-float sequences at `0x0014b760`, `0x0014b7cc` | Retained integer conversion for click counts. |
| ActorProjectile.cpp:101 | Draw `0x002099a0`; float-to-integer, mask `0xff`, then integer-to-float at `0x00209a38–0x00209a58` | Retained intentional integer quantization. |
| ActorWolfen.cpp:10845 | CBehaviourFighterWolfen::Init `0x001fb840`; float-to-integer conversions, integer random range, addition/store at `0x001fb900–0x001fb980` | Retained integer-valued random count in the port's numeric float field. PS2 stores the integer directly; this is not float-bit selection. |
| InputManager.cpp:580,594 | CPlayerInput::UpdateOne `0x001b7350`; float-to-unsigned conversion with high-bit reconstruction at `0x001b74a8–0x001b74e4`, `0x001b7528–0x001b7558` | Retained integer click conversion. Port represents stored clicks as numeric floats. |

Particle conversions share `ParticleRandom01FromBits`, which copies the masked
mantissa/exponent bits into a float and subtracts 1. Existing RNG helpers use it
too, avoiding pointer aliasing while preserving the RNG update sequence.

Guarded branches remain guarded; their broader implementations were not enabled
or completed by this audit. Regression coverage exercises fractional trail
rendering and the active perimeter particle path, including generator state.

## Expanded scanner: Types.h / DecodeFloat

The functional cast `float((value & 0x7FFFFFFF))` is part of an unsigned numeric
conversion, but its surrounding implementation differed from the PS2 sequence.
Compared the helper's caller `edSoundInstanceGetPlaybackTime` (`0x002889f0`):

- `0x00288adc–0x00288ae0` constructs float bits `0x4f000000`, exactly 2147483648.
  The port's literal 2147484000 rounded to 2147483904, shifting EncodeFloat's
  high-bit conversion by 256. Corrected the shared constant to exact 2^31.
- `0x00288b3c–0x00288b50` shifts the unsigned integer right, preserves the low bit
  with OR, converts to float, then doubles. Replaced DecodeFloat's mask/add
  approximation with that sequence. Merely correcting the constant still gives
  double-rounding errors for inputs such as `0xc0000081`.

Exact-equality boundary tests reproduced five failures before the fixes. Inputs
include 2^31, the next float above it, a rounding-sensitive larger integer, and
the highest representable float below 2^32. These helpers perform numeric
conversion, not float-bit serialization.
