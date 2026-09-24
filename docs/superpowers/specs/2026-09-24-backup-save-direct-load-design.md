# Direct Backup Save Loading

## Goal

Add a `Load` action to the debug backup browser that starts the selected backup save without restoring it to a live save slot. The action must not write, rename, rotate, or otherwise modify any save files.

## Approach

Add a `CSaveManagement` method that accepts a complete, already-validated save-file buffer. It will:

1. Verify the file header, descriptor size, payload size, and CRCs.
2. Copy the save descriptor and payload into the existing save-management buffers.
3. Set `saveSize_0x44` to the staged payload size.
4. Return `false` without changing memory on invalid input.

The debug backup browser will queue the operation through the existing level-management task queue. Once the buffer is staged successfully, it will call `gSaveManagement.load_level()`, which is the existing path used after an ordinary memory-card load. The `Restore` and `Restore & Load` actions retain their current behavior.

## User Interface

Each valid backup entry gains a `Load` button alongside `Restore` and `Restore & Load`. It uses the same readiness checks and disables during another queued restore/load operation. Status text distinguishes a queued load, validation/staging failure, and a successfully requested level transition.

## Error Handling

The new loader validates all externally supplied bytes before writing to `pBigAlloc_0x34`. Invalid files, absent buffers, or incompatible payload sizes fail explicitly and preserve the active in-memory save state. The debug operation is wrapped in the existing exception reporting path for unexpected failures.

## Testing

Extract the validation/staging behavior behind the new `CSaveManagement` method so it can be covered by GoogleTest. Tests will verify that a valid file stages the descriptor and payload, while malformed headers, invalid CRCs, and oversized payloads fail without mutating previously staged data. The existing Windows save test target will run afterward.
