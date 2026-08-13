# Sound-instance generation handles

`ed_sound_instance` objects are stored in the contiguous `pedSoundInstances` array. Public sound-instance IDs are not plain array indices: they are 32-bit generation handles.

```text
31                         16 15                          0
+---------------------------+-----------------------------+
| generation                | slot index                  |
+---------------------------+-----------------------------+
```

The first word of `ed_sound_instance` is represented by this union:

```cpp
union
{
	uint fullSoundInstanceId;
	struct
	{
		ushort soundInstanceIndex;
		ushort generation;
	};
	ushort soundInstanceId;
};
```

`soundInstanceIndex` and `generation` expose the two halves directly. `soundInstanceId` is retained as a legacy alias for the low 16-bit slot portion, including existing free-marker writes. Use `fullSoundInstanceId` whenever the value is a public ID, command payload, callback value, or is checked for liveness.

## Lifetime

When an instance is allocated, its low half is set to the selected array slot and its high half is set to `edSoundInstanceNewGlobalIndex`. The generation increments for each allocation, skipping zero. The handle returned to callers is the resulting `fullSoundInstanceId`.

When an instance is freed, the low half is set to `0xffff`. This is an intentional low-half write used as the free-slot marker; it is not a handle update.

The generation prevents a stale ID from operating on a later sound instance that reused the same slot:

```text
slot 7 allocated with generation 3: handle = 0x00030007
slot 7 freed, then allocated again:    handle = 0x00040007

0x00030007 & 0xffff == 7
```

Slot-only validation would incorrectly accept the old handle. Full-handle validation rejects it.

## Required lookup pattern

```cpp
bool edSoundInstanceIsAlive(uint soundID)
{
	return soundID != 0 &&
		pedSoundInstances[soundID & 0xffff].fullSoundInstanceId == soundID;
}
```

Use the low half only to select a table entry:

```cpp
uint slot = soundID & 0xffff;
ed_sound_instance* pInstance = &pedSoundInstances[slot];
```

Then compare `pInstance->fullSoundInstanceId` with `soundID` before reading or modifying that instance. The same rule applies to `edSoundInstanceCom[slot]`: `slot` selects the command slot, while `edSoundInstanceCom[slot].soundInstanceId` must retain the full handle.

## Recovered decompiler artifacts

### Interior-pointer arithmetic mistaken for `voiceIndices`

Ghidra originally rendered the liveness check as:

```cpp
pedSoundInstances->voiceIndices[
	(soundID & 0xffff) * 0x28 + (soundID & 0xffff) - 0x1e]
```

`voiceIndices` starts at offset `0x78` in an `ed_sound_instance`, while each instance is `0xa4` bytes. In 32-bit words, the expression resolves to:

```text
0x78 + (41 * slot - 30) * 4 = 0xa4 * slot
```

That is the start of `pedSoundInstances[slot]`, not an indexed voice array. The recovered source is therefore:

```cpp
pedSoundInstances[soundID & 0xffff].fullSoundInstanceId == soundID
```

`voiceIndices[2]` remains a real field: it stores allocated PS2 hardware voice indices and is used when releasing those voices.

### Truncated handles in queued commands

Several decompiled paths copied `instance->soundInstanceId` into command and delete queues. That silently discarded the generation. These now copy `instance->fullSoundInstanceId`, while still using `handle & 0xffff` to index the relevant command entry. This covers fade completion, active-instance command preparation, termination, acoustic updates, and the PS2 pending-delete queue.

### Generation-checked public operations

`edSoundInstanceFade`, `edSoundInstanceFadeTypeSet`, and `edSoundInstanceIsAlive` all first resolve and validate the handle. For example, fade updates its current/target values only after confirming that the slot still contains the exact caller-provided handle.

## Review checklist

- Is this value crossing an API, callback, command buffer, or delete queue? Use `fullSoundInstanceId`.
- Is it only selecting an instance or command-array slot? Use `handle & 0xffff`.
- Is a low-half `0xffff` write marking a slot free? Keep it as `soundInstanceId`.
- Does code access an instance from an externally held ID? Validate `fullSoundInstanceId == handle` first.
- Does a strange array expression originate from an interior struct member? Reduce the byte arithmetic before changing the member type.
