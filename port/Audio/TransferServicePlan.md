# PC Transfer Service Plan

## Goal

Implement the PC equivalent of the EE-to-IOP service used by
`_edSysTransferData`. Preserve its observable contract—owned transferred data,
monotonic indices, ordered completion, return-data delivery, and main-thread
callbacks—without emulating SIF RPC, DMA, IOP memory, or IOP threads.

The first consumer is sound loading. The four-byte value returned to an
`ed_sound_sample` will be an opaque PC handle, never a truncated native pointer.

Runtime stream playback state is separate from this transfer service. The PC
equivalent of the IOP stream-status snapshot is implemented by
`edSoundStreamService`, which owns stream lifecycle and playback cursor state;
the transfer service must not be used as a playback clock.

## Proposed Files

- `port/Audio/CMakeLists.txt`: define an `Audio` library.
- `port/Audio/edSysTransferService.h`: public PC transfer-service API.
- `port/Audio/edSysTransferService.cpp`: loaded-data registry, pending queue,
  and callback dispatch.
- `port/Audio/edSoundStreamService.h/.cpp`: Windows stream lifecycle and host
  playback cursor state exposed through the recovered `SOUND_*` interface.
- `port/Test/src/tests.cpp`: transfer-service unit tests.

Add `Audio` from `port/CMakeLists.txt`, link it into `Port`, and expose only the
small API required by the decompiled Eden wrapper.

## Phase 1: Core Transfer Model

Define a pending transfer containing:

- transfer index;
- an owned byte buffer copied from `pSource` during submission;
- requested alignment and `EdSysTransferFlags`;
- setup, transfer, and end callback identifiers;
- callback parameter address, return-byte count, user data, and EE callback;
- a bounded return-data buffer.

Use a FIFO queue and a monotonically increasing nonzero index. A zero-size
submission must retain the PS2 behavior: return index zero and enqueue nothing.

Do not introduce a worker thread initially. Copying at submission protects the
source lifetime, while deferred completion preserves the no-wait API's
reentrancy behavior.

## Phase 2: Loaded-Data Registry

Store completed sound data in a registry keyed by a nonzero `uint32_t` handle.
Never place a 64-bit PC pointer in `ed_sound_sample::soundRamAddress`.

Each entry should retain its owned bytes, alignment, setup flags, and optional
backend state added later. Provide lookup, release, and reset operations. Handle
wraparound must skip zero and every live handle. Later connect release to sound
bank or level teardown so repeated level loads do not leak data.

## Phase 3: IOP Callback-ID Dispatch

Implement a small dispatch table for the recovered callback identifiers:

- `0`: none;
- `1`: sound setup;
- `2`: sound transfer;
- `3`: sound end;
- `4`: music setup;
- `5`: music transfer;
- `6`: music end.

For the first sound implementation:

1. Setup allocates a registry handle.
2. Transfer associates the owned ADPCM bytes with that handle.
3. End writes the four-byte handle into the transfer's return-data buffer.

Both regular and `KEEP_IN_IOP`/`ONE_BLOCK_ONLY` submissions may use the same
owned-buffer path initially. Preserve the flags so a later backend can
implement distinct streaming or direct-transfer behavior.

Unknown nonzero callback identifiers should log an error and fail the transfer
deterministically rather than silently reporting success.

## Phase 4: Ordered Main-Thread Completion

Add a pump operation that completes queued transfers in submission order. Match
the EE command handler's ordering for each completion:

1. advance `_edSysCompletedTransferIndex`;
2. copy `returnNbBytes` to `pCallbackParams`;
3. invoke `pCallback(pCallbackParams)`.

Keep callbacks on the main thread because they mutate decompiled game objects.
The single completed-index counter represents a contiguous FIFO frontier, so
future background work must not publish out-of-order completion.

Call the pump from:

- `_edSysWaitUntilTransferFinished`, completing through the requested index;
- `_edSysWaitUntilAllTransfersFinished`;
- the sound/music completion polling path;
- eventually one stable once-per-frame update point.

Replace the PC busy-wait with queue pumping. If a worker is introduced later,
use a condition variable while still executing final callbacks on the main
thread.

## Phase 5: Eden Wrapper Integration

Keep the recovered PS2 implementation and layout in
`src/EdenLib/edSys/sources/ps2/_edSysDataTransfer.cpp`.

Under `PLATFORM_WIN`:

- initialize and shut down the port service from the existing edSys lifecycle;
- translate `_edSysTransferData` arguments into a port-service submission;
- mirror returned and completed indices in the recovered globals;
- pump through requested indices in the wait functions.

Avoid exposing STL types in decompiled structures or replacing recovered fields
with PC-native pointers. Modern ownership and synchronization remain inside
`port/Audio`.

## Phase 6: Tests and Validation

Add tests covering:

- zero-size submission returns zero and does not call back;
- source bytes are owned after submission;
- indices are monotonic and complete FIFO;
- completion writes the result before invoking the callback;
- callback receives the original `pCallbackParams`;
- waiting for index N completes every transfer through N;
- a four-byte sound handle lands in `ed_sound_sample::soundRamAddress`;
- handles resolve to the original ADPCM bytes;
- every `EdSysTransferFlags` bit survives submission unchanged;
- reset/shutdown releases queued and loaded data.

Run:

```text
cmake --preset x64-debug
cmake --build out/build/x64-debug
ctest --test-dir out/build/x64-debug
```

The first milestone is complete when sample loading no longer stalls level
loading, callbacks run in PS2-observable order, and a returned sound handle can
resolve the copied ADPCM data.
