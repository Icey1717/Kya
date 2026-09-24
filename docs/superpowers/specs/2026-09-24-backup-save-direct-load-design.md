# Direct Backup Save Loading

## Goal

Add a `Load` action to the debug backup browser that starts the selected backup save without restoring it to a live save slot. The action must not write, rename, rotate, or otherwise modify any save files.

## Approach

Add a `CSaveManagement` method that accepts a complete, already-validated save-file buffer. It will:

1. Accept `(const void* data, size_t size)` and verify non-null source and destination.
2. Require `initialBlockSize == sizeof(SaveDataDesc)` and `mainBlockSize > 0` before converting either signed field for a size comparison.
3. Verify the file header and CRCs, then verify `size >= prefixSize` before comparing `mainBlockSize` with `size - prefixSize`; never form a potentially overflowing total offset.
4. Reject payloads larger than the fixed 64 KiB allocation or `gameSaveMaxBufferSize`.
5. Structurally validate the payload before staging: each chunk header and declared extent must stay within the supplied payload, lengths must advance without overflow, `BSHD` must exist and be large enough to read its level ID, and that level ID must be loadable.
6. Copy only the payload into the existing save-management buffer and set `saveSize_0x44` after all validation succeeds.
7. Return `false` without changing memory on invalid input.

The staging method must not change `aSaveDataDescriptions`, `slotID_0x28`, `saveDataHeader`, or file-related flags. The descriptor is validated but never becomes live slot metadata.

The debug backup browser will queue the operation through the existing level-management task queue. Once the buffer is staged successfully, it will execute the same post-load state cleanup as `MemCardLoad0`: call `gSaveManagement.load_level()`, leave the pause menu when appropriate, clear `UINT_00448eac`, and clear the global pause. The `Restore` and `Restore & Load` actions retain their current behavior.

## User Interface

Each valid backup entry gains a `Load` button alongside `Restore` and `Restore & Load`. It uses the same readiness checks and disables during another queued restore/load operation. Status text distinguishes a queued load, validation/staging failure, and a successfully requested level transition.

## Error Handling

The new loader validates all externally supplied bytes before writing to `pBigAlloc_0x34`. Invalid files, absent buffers, or incompatible payload sizes fail explicitly and preserve the active in-memory save state. The debug operation is wrapped in the existing exception reporting path for unexpected failures.

## Testing

Extract the validation/staging behavior behind the new `CSaveManagement` method so it can be covered by GoogleTest. Tests will verify that valid data stages only the payload and updates its size; malformed headers, invalid CRCs, truncated input, negative or oversized declared sizes, declared payloads beyond supplied bytes, absent allocations, malformed root or nested chunks, missing or short `BSHD`, invalid saved level IDs, and payloads beyond either size limit fail without mutating previously staged data or slot metadata. Direct loads will be exercised from the title screen and paused gameplay to verify normal post-load cleanup. The existing Windows save test target will run afterward.
