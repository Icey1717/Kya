# PS2 register shifts and Windows long

Compared against `SLES_514.73` (R5900, image base `0x00100000`) with
Ghidra MCP. Decompiled `long` expressions sometimes model 64-bit registers;
Windows C++ `long` is 32 bits. Reconstruct flag operations with masks and pack
64-bit fields with `ulong` (the project's unsigned 64-bit type).

## Comparisons used

| Source function | PS2 entry | Relevant instructions / recovered operation |
| --- | --- | --- |
| `CActorAutonomous::SV_AUT_MoveTo_DynFence` | `0x00201030` | `0x00201078–0x00201084`: test bits 0 and 1 separately; reset if either is clear. |
| `CBehaviourFighterWolfen::UpdateFightContext` | `0x001f6f00` | `0x001f72bc–0x001f72c4`: extract bit 4 before filtering the context. |
| `CBehaviourFighterWolfen::TreatContext` | `0x001f5880` | `0x001f58b8–0x001f58c8`, `0x001f58dc–0x001f58e4`, `0x001f5dd4–0x001f5ddc`, `0x001f60a4–0x001f60ac`: test bits 1, 2, 3, and 4. |
| `S_STREAM_EVENT_CAMERA::Init` | `0x00118db0` | `0x00118dd8–0x00118ddc`: shift left/right by 35 logically, retaining the low 29 bits. |
| `CBasicPathFinder::ComputeSubTarget` | `0x001bdbd0` | `0x001bddb4–0x001bddb8`: shift left 47, logically right 48, extracting bits 1–16. |
| `CIniFile::ReadHexValueFromFile` | `0x001a76a0` | `0x001a7908–0x001a7914`: sign-extend the current byte using left 56 / arithmetic right 56. |
| `CLightTorch::Manage` | `0x001abef0` | `0x001ac080`, `0x001ac094–0x001ac09c`: signed byte load, left 56 / arithmetic right 60; signed upper nibble. |
| `CLightSun::Manage` | `0x001acd30` | `0x001acd58`, `0x001acd6c–0x001acd70`: same signed upper-nibble extraction. The port already used `long long`; replaced it with masks too. |
| `edPacketFunc_0026a440` | `0x0026a440` | `0x0026a61c–0x0026a638`: left/right 33 logically, retaining the low 31 address bits. This additional case shifts a `long` parameter without an explicit cast. |
| `SplashFunc_002ba880` | `0x002ba880` | `0x002ba968–0x002baa34`: pack format, width, and transfer coordinates into 64-bit GS fields using shifts of 32, 48, and 56. |
| `ed3DHierachyCheckForGlobalAlphaSetPKT_` | `0x002a9560` | `0x002a971c–0x002a9734`: pack the masked alpha value into the upper word. |
| `ed3DFlushStripMultiTexture` | `0x002a6760` | `0x002a69fc–0x002a6a20`, `0x002a6a50–0x002a6c44`, `0x002a70f4–0x002a7354`: mask DMA addresses to 28 bits on PS2; pack addresses and VIF commands into the upper word. |
| `ed3DFlushStrip` | `0x002a5630` | `0x002a5a94`, `0x002a5b44–0x002a5c40`: scratchpad address masking and upper-word packing of DMA addresses/VIF commands. |
| `ed3DFlushStripShadowRender` | `0x002aace0` | `0x002aae24`: mask the scratchpad DMA destination to 28 bits. |
| `ed3DPrepareMaterial` | `0x0029ffd0` | `0x002a0398–0x002a03a8`: pack mipmap K into the upper word; sign-extend the other integer before shifting it by 2. |

Both decompilation and disassembly were inspected for every changed game
function. Packet pointers use the existing `PKT_TO_SCRATCHPAD` and
`STORE_POINTER` helpers: PS2 keeps its physical address mask, while Windows
preserves its pointer handles instead of truncating native pointers.

## Scope and remaining behavior

The six originally identified sign tests and the other confirmed width/shift
errors in the audited `src/b-witch/` code were corrected. Narrow, valid shifts
and unrelated pointer casts were not changed. Existing signed 64-bit light
manager expressions have no Windows `long` width mismatch.

The Wolfen reaction bodies remain guarded/unimplemented. Correcting their
conditions makes those guards reachable; it does not implement blocking,
evasion, or the other missing reactions. In particular, the PS2
`TreatContext_Attacked` at `0x001f6570` is still absent from the port.

Regression coverage exercises the real INI hex parser and camera event
initialization, including preservation of configuration flags.

## Validation

- `cmake --build out/build/x64-debug -j 4`: passed (game and tests).
- PS2 instruction equivalence checks: all 256 context bytes, all 256 signed
  upper-nibble inputs, 262144 packed path values, and camera/address boundary
  values passed.
- `KyaPortTest.exe --gtest_filter=IniFile.*:CameraEvent.*:ShadowLighting.*`:
  all five tests passed.
- `ctest --test-dir out/build/x64-debug --output-on-failure`: 306 Google Tests
  ran; 304 passed, one skipped, one failed. The failure was
  `BinocularHud.DisplayRoutesHeroInterfaceAndGameCleanupToTheWidget`, which
  throws access violation `0xc0000005` and also fails when run alone. Its
  baseline status was not established; no binocular HUD code was changed.
