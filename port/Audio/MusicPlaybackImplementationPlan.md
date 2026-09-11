# Windows Music Backend Implementation Plan

## Objective and status

Make level music audible on Windows using the game's original song and bank
assets. Preserve music selection, channel mixing, priorities, transitions,
looping, pause/resume, and level teardown. Keep the existing menu, sound-effect,
and cutscene audio working.

The Windows backend is implemented. The user confirmed working in-game playback
on 2026-09-11. The original implementation stages below are retained as design
history; detailed PS2 fidelity comparison remains follow-up work.

## Implemented backend

- `edMusicData.*` parses Sony PS2 sequence and instrument-bank chunks with bounds
  checking, resolves program/key/velocity regions, and decodes ADPCM samples.
- `edMusicSynth.*` renders stereo PCM at 48 kHz using sample-clock sequencing,
  envelopes, instrument loops, pitch bend, volume/pan, and triangle modulation.
- `edMusicService.*` owns installed resources and stream lifetimes. It uses the
  existing shared XAudio2 device and a render worker with up to three 10 ms PCM
  blocks queued per stream. Eden state and completion callbacks stay on the game
  thread. Removal invalidates active playback and queued commands referencing
  the removed resource, including commands waiting to replace another resource.
- Music transfer callbacks now retain bytes behind opaque handles. Install and
  unload paths register and release songs, bank headers, and sample bodies.
- Windows Eden adapters connect restart/resume/pause, mute, automatic looping,
  volume/tempo/channel controls, fades, and end-of-song handling to the backend.
  The game scheduler continues to own music selection and transitions.
- Every change made by this implementation under `src/` is guarded with
  `#ifdef PLATFORM_WIN`; the non-Windows decompiled code is preserved.

Validation includes the Windows build, CTest, deterministic parser/rendering and
service lifecycle tests, Eden installation/fade/removal tests, and an opt-in muted
XAudio2 completion/shutdown test. Asset validation parsed 90 songs and 13 banks
and rendered the first second of each song. Across the complete event streams,
164,645 notes match defined instrument regions; 123 refer to unassigned
keys/programs and are left silent rather than given substitute instruments.
Those entries remain a point to check against original-game audio.

The renderer uses linear interpolation and simplified LFO/envelope behavior;
it does not emulate PS2 reverb, all key/velocity-follow and crossfade parameters,
or exact shared SPU voice allocation. Polyphony is bounded per stream, with
oldest-note replacement at the configured limit. These are fidelity limitations,
not claims of a bit-exact SPU2 implementation. User testing confirmed playback;
the full manual transition/combat/cutscene checklist below has not been recorded
as complete.

## Tools and focused validation

Inventory the local assets without extracting copyrighted payloads:

```powershell
python python/inspect_music.py assets/CDEURO/LEVEL
```

Render a song to a local WAV using the same parser and synthesizer as the backend:

```powershell
out/build/x64-debug/port/Audio/KyaMusicRender.exe assets/CDEURO/LEVEL/LEVEL_1/LEVELIOP.BNK 0 20 out/level1-song0.wav
```

The arguments are archive, zero-based song index, duration in seconds (1..600),
and output path. The tool accepts uncompressed, single-music-bank level archives.
A 20-second example was generated at `out/music-reference/level1-song0.wav`.

Run music tests, including local-asset and muted real-device checks:

```powershell
bin/WIN/KyaPortTest.exe --gtest_filter=AudioMusic.*:MusicServiceTest.* --gtest_also_run_disabled_tests
```

## Recovered format and command reference

Archives use `KNAB` at byte 8. Table offsets are relative to that header;
payload offsets are absolute. Category 3 subtypes 5, 6, and 7 identify songs,
sample bodies, and bank headers respectively. The inspector rejects compressed
archives rather than interpreting compressed bytes as music.

Song chunks are `IECSsreV`, `IECSuqeS`, and `IECSidiM` on disk. The observed
MIDI block has a six-byte header and a PPQN value. Events use running status,
variable-length delta times, a single data byte for note-off, and a high bit in
the final data byte to omit the next delta. They are not standard MIDI files.
The observed controller numbers are 1 (modulation), 7 (volume), and 10 (pan).

Bank chunks are `IECSdaeH`, `IECSgorP`, `IECStesS`, `IECSlpmS`, and `IECSigaV`.
Their tables include sparse entries represented by `0xffffffff`. Program splits
reference sample sets, which reference sample parameters and ADPCM waveforms.
The parsers validate these references before creating playback objects.

Original executable inspection confirmed `edMusicStreamPlay` at `00267f30`:
mode 0 resets position and plays, mode 1 resumes, and mode 2 pauses. Automatic
looping is stored in bit 2 of `fadeFlags` (`00268320` / `00268370`). The existing
`CAudioManager::Level_Init` translates song/bank indices after loading; Windows
teardown now uses the installed bank index too.

Format references consulted alongside the local assets and original executable:

- [VGMTrans Sony PS2 sequence parser](https://github.com/vgmtrans/vgmtrans/blob/master/src/main/formats/SonyPS2/SonyPS2Seq.cpp)
- [VGMTrans Sony PS2 instrument structures](https://github.com/vgmtrans/vgmtrans/blob/master/src/main/formats/SonyPS2/SonyPS2InstrSet.h)
- [SPU envelope register and rate documentation](https://psx-spx.consoledev.net/soundprocessingunitspu/)

No original game payloads or downloaded reference source files are added to the
repository. Local inventory, reference downloads, WAV output, and validation logs
are under the ignored `out/music-reference` directory.

## Original findings and integration points

| Area | Finding |
| --- | --- |
| `src/b-witch/Audio.cpp` | `CMusicManager` selects streams and manages transitions, but its local `edMusicStream*` adapters only update state or do nothing. |
| `src/b-witch/LevelScheduler.cpp` | `BnkInstallSong`, `BnkInstallBank`, and `BnkInstallBankHeader` load song bytes, sample bytes, and bank metadata; installed indices are recorded in `aSongIndexes` and `aBankIndexes`. |
| `src/EdenLib/edMusic/sources/ps2/_edMusicPlay.cpp` | Installation uses the transfer service; original playback registration commands are PS2-only. Windows song/bank metadata release needs auditing because the non-sound-RAM free path is PS2-only. |
| `src/EdenLib/edMusic/sources/edMusicPlay.cpp` | Windows command submission, channel gain, tempo, pause, and mute are unimplemented. Fade calculations already exist. |
| `src/b-witch/Audio.cpp` | `CMusicManager_EndOfSongCallback` remains behind `IMPLEMENTATION_GUARD` and depends on an unimplemented looping query. |
| `port/Audio/edSysTransferService.*` | Provides owned transferred bytes, opaque handles, ordered completion, lookup, and release. |
| `port/Audio/edSoundSampleService.*` | Provides sample decoding and a testable voice abstraction; review reusable decoding separately from sound-instance lifecycle. |
| `port/Audio/edSoundStreamService.*`, `edSoundDevice.h` | Provide working audio output and access to the shared XAudio2 device. |

## Proposed architecture

Keep game policy in `CMusicManager` and Eden's wrappers. Put Windows parsing,
sequencing, synthesis, resource ownership, and output under `port/Audio`.
Use Windows side tables rather than expanding layout-sensitive Eden structures
or storing native pointers in serialized PS2 fields.

Proposed flow:

```text
Level bank loader -> transfer registry -> parsed songs/banks -> decoded samples
CMusicManager -> Eden music commands -> music service -> sequencer/synthesizer
                                                      -> PCM -> shared XAudio2 device
Eden end-of-song callback <- game-thread completion polling <- music service
```

The preferred design, if the assets prove to be sequences plus instruments, is
a deterministic software sequencer/synthesizer producing bounded PCM blocks.
This allows events and envelopes to advance on an audio sample clock. The
existing sample service can supply decoder patterns, but routing every note
through actor sound instances should not be required.

Choose the final synthesis implementation after format recovery. Reuse an
existing compatible implementation only after verifying format support,
integration cost, and licensing. XAudio2 is the output layer; interpreting the
game's songs and instruments is a separate responsibility. If asset inspection
instead reveals recorded audio, revise the playback design accordingly.

## 1. Recover formats and command semantics

- Capture representative song, bank-header, and sample payloads at the existing
  installation boundaries. Record level, local index, installed index, size,
  and a stable content identifier. Keep proprietary payloads out of commits.
- Inspect available original IOP code/binaries alongside the assets to identify
  headers, offsets, event encoding, timing units, instrument selection, sample
  ranges, tuning, loops, envelopes, and any custom controllers.
- Inventory events across the available levels, including ambient and combat
  music. Distinguish observed fields from hypotheses.
- Recover the exact meanings of play modes `0`, `1`, and `2`, stop versus pause,
  restart, song replacement, automatic looping, and completion notification.
  The current adapter's state assignment is not sufficient evidence.
- Verify whether game song/bank indices must be translated through
  `aSongIndexes`/`aBankIndexes`; test non-identity mappings.
- Document master, stream, and channel gain and tempo units, output mode, voice
  limit, priority/preemption behavior, and fade ownership. Preserve existing
  fade behavior until discrepancies are demonstrated.

Deliverables: a format/command reference and a read-only inspection tool that
can describe at least one level song and its instruments. This is the decision
gate for the decoder and synthesis approach; avoid estimating the full backend
as a small adapter change before completing it.

## 2. Implement asset parsing and ownership

- Add bounded parsers for the recovered formats. Validate offsets, counts,
  lengths, instrument/sample references, and event progress before allocation
  or playback. Report unsupported events with asset and byte-offset context.
- Resolve transfer handles through `LookupLoadedData`. Never interpret a
  four-byte transfer result as a native pointer. Audit initialization of
  pointer-shaped fields receiving four-byte results on Windows.
- Retain or copy source data explicitly while parsers and active voices need
  it. Cache decoded instrument samples by bank identity and sample identity.
- Establish installation readiness only after all required song/bank data is
  available. Ensure a failed installation cannot leave a perpetual loading
  state; preserve the caller's existing loading contract.
- Add removal/reset handling for song data, bank headers, sample data, decoded
  caches, pending commands, and active playback. Prevent double release where
  the transfer registry and music service interact.
- Use generations for reused song, bank, and stream slots so delayed work cannot
  affect a replacement resource after a level transition.

Deliverable: install, inspect, unload, and reinstall assets without audio output,
leaks, stale references, or changing Eden layouts.

## 3. Build deterministic offline playback

- Implement the observed event set, including note lifetime, instrument/channel
  state, timing changes, looping, and the controllers required by the first song.
- Decode instrument samples using verified metadata. Share ADPCM decoding where
  its format matches; do not assume stream or sound-effect headers match banks.
- Implement tuning, interpolation, gain/pan, envelopes, and sample loops from
  recovered semantics. Define bounded polyphony and voice stealing against the
  configured music voice budget.
- Advance musical time from rendered sample frames, independently of game FPS.
  Split rendering at event boundaries and bound malformed zero-time event loops.
- Render a representative level song to a local WAV for inspection. Compare
  timing, instrumentation, and loop boundaries with original-game playback when
  a reference is available. Mark approximations explicitly.

Deliverable: recognizable, correctly timed level music offline, with synthetic
fixtures suitable for automated tests. Do not bake full songs into the runtime:
the game needs live channel controls and transitions.

## 4. Add Windows output and controls

- Add a music service and output abstraction using the existing XAudio2 device.
  Preserve independent music gain so music volume does not change effects or
  cutscene volume.
- Submit bounded PCM buffers with explicit ownership through completion. Keep
  enough audio queued for normal frame jitter while measuring control latency.
- Service rendering independently of long game frames/loading stalls, preferably
  with a dedicated render worker once live output is added. Pass copied commands
  and owned resources; never access Eden objects from the worker or device
  callbacks. Keep queue capacity and shutdown ordering explicit.
- Serialize start, pause/resume, stop, replacement, looping, channel gain, stream
  gain, master gain, and tempo commands. Apply coherent control snapshots before
  starting playback to avoid an initial full-volume burst.
- Define how queued PCM affects pause and gain latency. Flush obsolete audio on
  restart/replacement and preserve the correct sequence position on resume.
- Poll completion on the game thread, rejecting stale generations. Distinguish
  natural end, loop boundary, explicit stop, and playback failure according to
  the recovered callback contract.

Deliverable: a test harness can play and control one installed level song through
the real device while sound effects and cutscene streams continue to work.

## 5. Wire Eden and the game scheduler

- Replace local placeholder adapters with declared Eden music APIs backed by
  typed Windows commands. Keep platform-specific implementation out of
  `CMusicManager` and respect `NoAudio`.
- Connect installation/removal, initialization/reset, master settings, fades,
  channel settings, mute, and `edMusicFlush` to the service.
- Restore the looping query and `CMusicManager_EndOfSongCallback`; dispatch
  notifications on the game thread in a defined flush order and allow callback
  reentrancy without invalidating iteration.
- Audit fade completion: the existing volume/tempo final branch submits current
  values rather than explicitly snapping to targets. Verify intended behavior
  and add a regression test before changing it. Avoid applying a fade twice in
  Eden and the renderer.
- Verify priority changes, queued songs, combat interruption, ambient channel
  fades, stop-all, pause/resume, and initial level playback through the existing
  scheduler. Bound stream allocation by configured capacity.
- Stop and join music rendering, destroy music voices, then release their data;
  shut down the shared device only after all audio services have released voices.

Deliverable: menu-to-level and level-to-level transitions play the intended music
without stale notes, muted replacement tracks, or dangling assets.

## 6. Validation and acceptance

Add meaningful tests under `port/Test/src/audio_music_tests.cpp` and register
them using the existing test build patterns:

- Parser fixtures: valid minimal assets, truncated input, invalid references,
  unsupported events, and bounded handling of malformed event loops.
- Offline rendering: known note/event frame positions, tempo changes, sample
  loop boundaries, gain/channel controls, and frame-chunk-size independence.
- Service lifecycle: pending installs, non-identity indices, stop before start,
  pause/resume, replacement, slot reuse, completion exactly as specified,
  unload during playback, and shutdown with queued buffers.
- Scheduler integration: first level track, crossfade/preemption, completion
  cleanup, and music-volume changes without affecting other audio services.
- Fake output tests for deterministic ownership and ordering; an opt-in real
  XAudio2 test for device playback and cleanup.

Build and run the existing suite:

```powershell
cmake --preset x64-debug
cmake --build out/build/x64-debug
ctest --test-dir out/build/x64-debug --output-on-failure
```

Manual acceptance: enter a level from the menu, hear its intended music, trigger
combat and return to ambience, pause/resume, adjust music volume, hear a loop
boundary, change levels repeatedly, return to the menu, and play a cutscene.
Check for clicks, timing drift, stuck notes, underruns, unbounded memory growth,
and regressions in existing audio. Report audible validation separately from
headless test results.

## Volume investigation (2026-09-11)

The default music and SFX sliders are both 1.0. The Windows synth incorrectly
combined stream and master volumes as ordinary normalized linear gains.
The bundled original IOP drivers establish different gain stages:

- In `assets/IOP/MODULESD.BIN`, the ELF at offset `0x60980` contains
  `_edMusicStreamSetChannelVolume` (`0x2220`) and `_edMusicSetMasterVolume`
  (`0x2878`). Both convert API volumes using `floor(volume16 * 255 / 65535)`.
- The MIDI ELF at offset `0x4c7c0`, function `0x1218`, computes
  `min(127, (CC7 * channelByte * streamByte) >> 14)`. The controller-event
  path at `0x1520` and external-volume update path at `0x1fec` use this function.
  Initialization at `0x400` sets external channel gains to 128. Setting a song
  reinitializes those gains; the EE's initial channel values are not commands.
- The HSyn ELF at offset `0x51f00`, function `0x4664`, applies master gain
  with a right shift of eight: `masterByte / 256`, separately from MIDI gain.

Windows now preserves these stages, byte quantization, MIDI saturation, and
explicit channel-volume updates. With CC7=64, streamByte=255, and the default
channelByte=128, the original produces CC7=127; the old backend retained 64.
That is approximately a 6 dB difference before the separate master stage.
The actual difference varies with each song's controllers and authored gains.
No arbitrary global boost or slider-default change was added.

Regression tests cover integer boundaries, rendered gain, master attenuation,
mute, persistent channel updates, and resetting channel gains for a new song.
All 12 music tests pass, including the opt-in device and installed-asset tests.
The user validated playback before this correction; listening to the revised
balance in game remains manual validation. This does not establish exact PS2
synthesis fidelity for other stages such as envelopes, instrument gain or reverb.

## Suggested implementation slices

1. Format reference, inspection tool, and evidence for command semantics.
2. Parsers, owned resource registry, and install/unload tests.
3. Offline sequencer/synthesis and one verified level song.
4. Shared-device output, command queue, and lifecycle tests.
5. Eden integration, scheduler transitions, and in-game validation.
6. Remaining observed format features and fidelity corrections across levels.

The first playable milestone is one verified level song with correct controls,
looping, and teardown. Full acceptance requires the observed event set across
tested levels. Reverb and exact PS2 synthesis fidelity may follow that milestone
if documented differences do not prevent correct music behavior; unsupported
events that alter a song's structure remain blockers for that song.
