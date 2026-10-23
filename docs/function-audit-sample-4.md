# Fourth random function-audit sample

Reviewed on 23 October 2026 against live Ghidra program `SLES_514.73`,
image base `00100000`. Five candidates were drawn without replacement using
PowerShell `Get-Random -Count 5` from the 2,274 candidates in
`out/function-audit-resume/report.json`, with no exclusions or redraws.
This exploratory sample does not estimate the remaining defect rate.

The exact draw is in `out/function-audit-resume/random-sample-4.json`.
`random-sample-4-evidence.json` in that directory retains live decompilation and
disassembly. All five current source definitions match their packet source,
after line-ending normalization; all five live PS2 decompilations match the
snapshot. The generated artifacts are local to this checkout.

| Function | PS2 address | Assessment |
|---|---|---|
| `CActorBoomy::UpdateFromOwner` | `0012c710` | Expected enum representation |
| `edAnmAnim::LoadFromMem` | `00243eb0` | Expected packed-field offsets for ordinary non-null input |
| `CActorFighter::FUN_00311f80` | `00311f80` | Equivalent named lookup wrapper |
| `CScene::~CScene` | `001b9f30` | Confirmed missing manager destruction behavior |
| `FUN_002a9220` | `002a9220` | Expected packet macros and fixed-size initialization/copy |

## Scene destruction: confirmed omission

The reverse iteration matches: `ManagerContainer::IterateBackwards()` returns
`aManagers + 0x17`, and both versions visit 24 slots in descending order.
The difference is destruction of each non-null manager.

Original instructions `001b9f74`–`001b9f80` load the manager's vtable, load slot
`+8`, and call it with deletion argument `1`: a virtual deleting-destructor call.
The `CActorManager` vtable at `0043a470` contains destructor `00107f50` in slot
`0043a478`. That destructor frees the allocation at `this + 4`
(`00107f78`–`00107f7c`), destroys 0x57 class-info entries
(`00107f80`–`00107f94`), calls the embedded cluster destructor
(`00107f98`–`00107fa0`), then conditionally frees the manager itself.

The port uses `delete *ppManager` at `src/b-witch/scene.cpp:1032`, through a
`CObjectManager*`. `CObjectManager` in `Scene.h` is polymorphic but has no virtual
destructor. Deleting derived managers this way has undefined C++ behavior and
does not provide the original virtual destruction contract. `CActorManager`
also lacks a recovered destructor; adding only a default virtual base destructor
would not recover its explicit cleanup.

There is a concrete allocation to account for: `ActorManager.cpp:21` allocates
`aLinkedActorData` using `new _linked_actor[0x80]`, corresponding to the original
destructor's `this + 4` allocation. Current game sources contain no deletion of
that allocation; two tests explicitly free it themselves. Its release is
missing from the recovered scene/manager destruction path.

The factory dispatch at `001ba004`–`001ba010` is also an original virtual deleting
destructor. The final conditional `Free(this)` at `001ba018`–`001ba02c` belongs
to the original compiler's deleting-destructor convention; absence of an explicit
self-free in a native C++ destructor is not itself a missing operation.

No runtime symptom was reproduced, and reachability of this destructor during
ordinary Windows shutdown was not established. This is a destruction-path
omission, not a demonstrated level-transition leak. A fix needs the manager
destruction contract, relevant derived cleanup, allocator-pairing checks and
regression coverage for deletion through the base.

## Boomy owner update: enum constant

The only signal is original literal `7`. `Actor.h:124` declares
`MESSAGE_GET_VISUAL_DETECTION_POINT = 0x7` as an enum; current normalization
resolves numeric macros rather than enums. Original `0012c76c`–`0012c770` calls
`DoMessage` with `a2 = 7`, matching the source.

Both versions start from the hero location, conditionally add the returned
visual-detection offset, copy the incoming direction into actor and dynamic
rotation, update position, then copy location and rotation to vision. Quaternion
stores and the position call at `0012c788`–`0012c7d4`, followed by vision stores
at `0012c7dc`–`0012c818`, support that comparison. No omission found.

## Animation loader: packed fields

`AnmManager.h:13` defines packed `AnimHeaderPacked` with four-byte hash and offset
fields followed by float payload at `+8`. The source expresses raw offsets via
these members. Original `00243ed0` selects the length at `+4`; `00243ef4` reads
the version at `+8`; `00243f1c` returns data at `+8`; and
`00243f20`–`00243f28` advances by chunk length plus eight bytes.
`00243ef0`–`00243efc` constructs float32 `0x3f8ccccd`, matching `1.1f`.

Hash tests, version rejection and the end-bound comparison match for ordinary
non-null buffers. `pFileEnd = &pFileData->hash` retains the initial base address
because the hash offset is zero. This does not establish malformed-buffer safety.
The source also forms that member address before its null check: null-pointer
expression semantics remain a separate C++ portability concern, not a reproduced
runtime failure in this review.

## Fighter reaction: renamed wrapper

The signal compares `FindGrabReactByHash` with Ghidra's
`_FindByHashcode_s_fighter_grab_react_`. Original call `003120b0` targets
`00302520`; instructions `00302520`–`00302530` load reaction count at `+0x858`
and table at `+0x850`, then tail-call lookup `00303830`. The source wrapper at
`ActorFighter.cpp:6157` passes `aGrabReacts`, `nbGrabReacts` and the requested
hash to `_FindByHashcode`.

The template preserves the null/empty checks, unsigned comparisons, search
bounds, exact-match test and null result. Original stride is 16 bytes
(`00303848`, `00303880`), consistent with Ghidra's 16-byte structure with fields
at 0, 4 and 8. The port builds, sorts and searches the table using the same native
type and its `sizeof`, rather than indexing a serialized array with a guessed
stride. The caller's clamp/sign, duration division, normalized velocity,
reaction fields, time warper and state `0x70` branches match. Instructions
`00311fc8`–`00311fe0` and `00312028`–`0031202c` construct actual `-1.0f` values,
explaining `DAT_bf800000`. No omission found.

## Matrix packet: macro expansion

The byte-clearing loop at `002a9248`–`002a926c` initializes eight bytes, matching
`local_8 = {}` for two floats. Near/far signs, frustum arguments, transpose and
matrix multiplication match. Instructions `002a92f8`–`002a9330` establish:

- First 64-bit word `0x0000000e10000004`. `ED_VIF1_SET_TAG_CNT(4)` expands to
  `4 | (0x10ULL << 24) | (0xeULL << 32)`, exactly that value.
- Next two 32-bit words `0` and `0x6c040072`. `SCE_VIF1_SET_NOP(0)` is zero;
  `SCE_VIF1_SET_UNPACK(0x72, 4, UNPACK_V4_32, 0)` expands to
  `0x72 | (4 << 16) | ((0x60 | 0x6c) << 24)`.
- A 64-byte matrix copy to the next 16-byte packet and a return advance of
  `0x50` bytes (`002a9334`), matching `pPkt + 5`.

Definitions in `ed3D.h`, `port/include/port.h` and `Types.h` confirm the macro
values and overlapping `edpkt_data` views. No packet-word mismatch found; this
does not validate the downstream Windows renderer's interpretation.

## Follow-up

Enum resolution and evidence-backed wrapper recognition could reduce noise.
Raw field offsets and packet encodings still require type or macro evidence.
The scene candidate demonstrates why unresolved indirect calls must stay visible:
a native `delete` expression alone does not establish recovered virtual cleanup.

This investigation changed no game code, filters or reviewed exceptions. No
runtime test or game build was performed; validation comprised source/snapshot
consistency checks and live PS2 decompilation/disassembly review.
