# Animation event-track corruption diagnostics

Windows Debug builds (including `x64-debug-asan`) snapshot the animation macro
bank when `BnkInstallAnimMacro` installs it. No actor or animation structure
layouts change. Release and PS2 builds omit the diagnostics.

`GetAnimEventTrackID`, `SetAnim`, and `SetAnimOnLayer` check bank pointers,
layer indices, animation lookup bounds, offsets, and track IDs before using
them. `CTrackManager::GetTrack` also checks its actual track count.

On failure, the game flushes its logs and asserts. Look in `logs/Animation.txt`
or `logs/async_log.txt`, relative to the application's working directory.
The report includes actor name/state, layer mask, current/next animation types,
bank and field addresses, original and live values, surrounding bank words,
and the last 32 lookups (oldest first). History contains both track reads and
animation selections, across all actors; it does not log every successful read.

- **Track ID changed since bank load:** the reported track slot was overwritten.
- **Offset changed or outside bank:** inspect the offset slot and its original value.
- **Track ID invalid in unchanged bank data:** inspect the layer's animation type;
  it may have selected data that is not a macro entry. The snapshot alone does
  not establish that the selected animation type is correct.
- **Bank pointer does not match:** inspect the animator's bank pointer and bank lifecycle.

`0x3f000000` is the IEEE-754 bit pattern for `0.5f`. That is a clue, not proof
of a float write: a wrong lookup can also land on existing float data. ASan
does not generally catch writes that stay inside an allocation but overwrite
a different logical object or field.

To catch the writer in Visual Studio:

1. Reproduce once and note the actor, animation type, and failing field.
2. Restart and break in `DebugValidateAnimTrackData`, conditionally on that
   `animType` (and actor if needed), before corruption occurs.
3. Add a **4-byte native data breakpoint** on `pTrackData` after it is computed,
   on `&pAnimator->pAnimKeyEntryData[animType]` for offset corruption, or on
   `&pAnimator->aAnimData[layerIndex].currentAnimDesc.animType` for a bad type.
4. Continue to stop at the instruction that writes the field. Addresses can
   change between runs; obtain them again after restarting. If the bank is
   already corrupted on the first lookup, break at `DebugSnapshotAnimMacroBank`
   after the snapshot and calculate the slot from the bank base plus the offset
   reported in the previous run.

These checks stop at the first observed bad lookup, not necessarily at the write.
An invalid index does not get silently replaced with `-1`.
