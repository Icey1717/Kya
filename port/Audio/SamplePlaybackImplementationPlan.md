# Windows Sound Effect Playback Implementation Plan

## Objective

Make actor sound effects play through the Windows audio compatibility layer, including overlapping instances, looping, spatialization, controls, completion callbacks, and cleanup. Preserve the existing working cutscene streaming behavior and the layout-sensitive Eden sound structures.

## Implementation status

Implemented in `edSoundSampleService.*`, with integration in Eden's sound flush,
instance lifecycle, sample unload, and actor completion paths. Sample and stream
voices share the existing XAudio2 device. Device setup now initializes COM when
needed and balances that initialization at shutdown.

The game thread owns the command list, sample cache, and voices. It polls
XAudio2 completion at the start of the next flush. There is no audio callback
that accesses Eden data, and no additional worker thread. PCM is shared between
overlapping instances and retained until their source voices are destroyed.

Windows command mapping:

| Eden pending mask | Windows operation |
| --- | --- |
| `0x800` | Create a prepared sample voice |
| `0x2` | Create/recreate, apply controls, then start |
| `0x4` | Stop and destroy the sample voice |
| `0x8` / `0x10` | Pause / resume |
| `0x20`, `0x40`, `0x80` | Apply computed left/right gains and frequency ratio |
| Instance free | Immediately destroy by full instance ID and cancel queued commands |

Control snapshots accompany every live sample command update, so master volume
and fades are applied even without an explicit pitch command. Start plus pause
in the same flush prepares a paused voice without briefly starting it. Existing
stream start calls retain their separate path; stream instances are never
interpreted as sample descriptions.

Associated fixes cover the invalid acoustic gain array indexing, a Windows
voice-index shift, zero-distance spatialization, the stubbed game completion
callback, the sample `IsLooping()` override, and pooled bone sound-data lookup.

Validation includes deterministic decoder/service/Eden integration tests in
`port/Test/src/audio_sample_tests.cpp`, the normal Windows build and CTest suite,
and a passing muted test against the real XAudio2 device. Run the optional device
test explicitly with:

```powershell
./bin/WIN/KyaPortTest.exe --gtest_filter=AudioSamples.DISABLED_XAudio2PlaysCompletesLoopsAndStops --gtest_also_run_disabled_tests
```

Audible in-game verification of effects and cutscene coexistence remains a
manual check. The backend currently outputs Eden's stereo gains (folding a
second PS2 sample voice into that pair); it does not add PS2 reverb emulation or
native multichannel spatialization. Pitch ratios are limited to the backend's
configured range of 1/1024 through 4.

API references: [XAudio2 initialization](https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2)
and [buffer ownership and loop regions](https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/ns-xaudio2-xaudio2_buffer).

## Original findings

- `CActorSound::SoundStart` in `src/b-witch/Actor.cpp` calls the sound object's virtual `Play` method.
- `CSoundSample::Play` in `src/b-witch/Audio.cpp` selects a loaded sample and calls `edSoundSamplePlay`.
- `edSoundSamplePlay` allocates an Eden instance and records pending commands in `edSoundInstanceCom`.
- The Windows branches of `edSoundFlush` in `src/EdenLib/edSound/sources/edSoundPlay.cpp` contain placeholder comments for command creation, destruction, and translation.
- `_edSoundEndFlush` in `src/EdenLib/edSound/sources/ps2/_edSoundPlay.cpp` has no Windows command submission implementation.
- `EdSoundPrepareFlush` has a placeholder for consuming host voice completions.
- `port/Audio/edSysTransferService.*` stores loaded sample data behind handles and exposes `LookupLoadedData` and `ReleaseLoadedData`.
- `port/Audio/edSoundStreamService.*` provides ADPCM decoding and working XAudio2 streaming playback.

These findings come from source tracing; runtime behavior still needs validation.

## 1. Establish the command and ownership contracts

Inspect the Eden command producers, instance allocation/free paths, acoustic calculations, and finished-instance callbacks before implementing their Windows equivalents.

- Document each supported command mask, its payload, and ordering when multiple bits are set in one flush.
- Verify create/start, stop/destroy, pause/resume, gain, and pitch semantics against the existing code.
- Identify which paths also operate on streaming instances and route those without creating duplicate voices or duplicate completion notifications.
- Define typed Windows commands containing full instance IDs and copied parameters. Do not pass Eden instance pointers to audio callbacks.
- Use full instance IDs, including their generation bits, for voice lookup and completion validation.
- Choose a single owner for backend voice state. Prefer submitting from the game-thread flush and returning completion IDs through a synchronized queue unless a dedicated command worker is justified.
- Define ownership of decoded PCM and transfer data explicitly. Audio buffers must remain valid until XAudio2 has finished using them.

Deliverable: documented command mapping and a small sample-service interface under `port/Audio`.

## 2. Implement a minimal sample backend

Add `edSoundSampleService.h/.cpp` and register them in `port/Audio/CMakeLists.txt`.

- Resolve each sample's `soundRamAddress` handle through `Audio::LookupLoadedData`.
- Decode the raw ADPCM payload using sample metadata for its rate and length. Do not assume the transferred bytes include a VAG header.
- Extract reusable decoder/device helpers from the streaming service only where needed; retain its public behavior.
- Cache decoded PCM by a stable loaded-data identity, with invalidation tied to unload. Allow multiple independent voices to share immutable PCM.
- Create an XAudio2 source voice, submit the PCM buffer, and start it.
- Report invalid handles, malformed data, and backend failures with enough context to identify the sample and instance. Failed starts must not leave permanently alive Eden instances.

Deliverable: a loaded sample can play through the new service with bounded buffer ownership.

## 3. Wire the Eden flush and completion paths

- Implement the Windows command translation in `edSoundFlush` and submission in `_edSoundEndFlush`.
- Consume pending delete entries and clear command masks after their data has been captured successfully.
- Initialize flush counters and other values used by the Windows path; remove reliance on PS2-only initialization.
- Preserve command ordering so initial gain/pitch settings apply before audible playback.
- Drain backend completion IDs in `EdSoundPrepareFlush` on the game thread.
- Route natural completion through Eden's existing finish/free/callback lifecycle after verifying its requirements.
- Handle stop, replacement, priority eviction, and destruction without duplicate completion or double-free behavior.
- Ignore stale completion IDs when an instance slot has been reused.

First integration milestone: `CActorSound::SoundStart` plays a one-shot effect, `IsAlive` becomes false after completion, and repeated playback reclaims voices correctly.

## 4. Implement controls, loops, and spatialization

- Support stop, pause/resume, gain, pitch, and changes while playing.
- Translate Eden frequency values into the correct XAudio2 frequency ratio, checking their units and valid range.
- Apply Eden's computed fades and acoustic gains each flush instead of duplicating its attenuation calculations.
- Verify existing listener, distance, and channel-gain calculations; translate their output into XAudio2 channel gains or an output matrix.
- Apply master volume consistently and check interactions with the streaming path to avoid applying it twice.
- Convert ADPCM byte loop offsets to PCM frame positions, verifying inclusive/exclusive end conventions and terminal-block behavior.
- Honor looping flags independently of incidental end markers introduced during sample preparation.

Deliverable: moving and looping actor effects respond correctly to gameplay controls and listener position.

## 5. Complete unload and shutdown handling

- Connect Windows sample freeing to `ReleaseLoadedData` and decoded-cache invalidation; audit `_edSoundSampleFree` and `_edSoundMemFree`.
- Define unload behavior for active voices and retain their PCM until the backend releases it.
- Destroy voices and drain or invalidate queued completions during level resets and shutdown.
- Ensure late callbacks cannot access destroyed service state.
- Verify initialization/shutdown ordering if sample and stream playback share XAudio2 infrastructure.
- Keep Windows-specific ownership in `port/Audio`; preserve serialized PS2 fields and pointer-conversion conventions.

Deliverable: repeated level loads and service resets leave no live voices, stale handles, or retained sample buffers.

## Validation

Use the repository's existing test infrastructure after locating the relevant audio tests. Add focused coverage for behavior that can regress:

- Raw ADPCM decoding, malformed/truncated input, and loop frame conversion.
- Multiple instances sharing one decoded sample.
- Create/start/control/stop command ordering and command consumption.
- Natural completion, explicit stop, failed start, and stale completion after slot reuse.
- Unloading samples while voices retain PCM and resetting with pending completions.

Keep decoder and lifecycle tests runnable without an audio device where practical. Use a narrow fake backend for command/lifecycle tests if required.

Build and smoke-test:

```powershell
cmake --preset x64-debug
cmake --build out/build/x64-debug
ctest --test-dir out/build/x64-debug
```

Perform an audible in-game check of one-shot effects, overlapping effects, loops, moving/distant actors, pause/resume, fades, master volume, and repeated level transitions. Recheck cutscene streaming both alone and alongside effects. Automated headless tests do not establish audible correctness.

## Completion criteria

- Actor sound requests reach XAudio2 and produce the expected sound.
- One-shots complete and release their Eden/backend resources.
- Loops, controls, and spatialization behave correctly.
- Stops, eviction, unload, and reset do not leak resources or corrupt reused instances.
- Existing cutscene streaming continues to work.
- Relevant tests and the Windows build pass, with remaining fidelity gaps recorded explicitly.
