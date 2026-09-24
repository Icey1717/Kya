# Direct Backup Save Loading Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Load a validated debug backup save without restoring it to an on-disk live slot.

**Architecture:** Add a small save-manager API that validates and stages a backup save payload in existing transient memory, then invoke the same post-load transition cleanup as `MemCardLoad0`. The debug backup browser gains a `Load` action that queues this path; the existing restore actions remain unchanged.

**Tech Stack:** C++20 Windows port, decompiled C++ save manager, Dear ImGui, GoogleTest.

---

## Chunk 1: Buffer staging API

### Task 1: Validate backup bytes before staging

**Files:**
- Modify: `src/b-witch/SaveManagement.h`
- Modify: `src/b-witch/SaveManagement.cpp`
- Test: `port/Test/src/windows_save_tests.cpp`

- [ ] **Step 1: Write the failing test**

Add GoogleTests for the public staging method using complete save-file byte buffers. Verify valid bytes update `saveSize_0x44` and payload memory only, while malformed header, CRC, size, and truncated inputs leave all state unchanged.

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest -R KyaPortTest --test-dir out/build/x64-debug`

Expected: the new staging test fails because the API does not exist.

- [ ] **Step 3: Write minimal implementation**

Declare `bool stage_backup_save(const void* data, size_t size)` on `CSaveManagement`. Verify the save envelope and CRCs with existing `edFileComputeCRC32`, require the descriptor size and a positive bounded main block, then copy only the main block to `pBigAlloc_0x34` and update `saveSize_0x44` after validation. Do not mutate live slot descriptor/file metadata.

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest -R KyaPortTest --test-dir out/build/x64-debug`

Expected: the new staging tests pass.

- [ ] **Step 5: Commit**

```powershell
git add src/b-witch/SaveManagement.h src/b-witch/SaveManagement.cpp port/Test/src/windows_save_tests.cpp
git commit -m "Stage backup save data in memory"
```

## Chunk 2: Debug menu direct-load action

### Task 2: Queue direct loading without restore

**Files:**
- Modify: `port/DebugMenu/src/DebugSaveLoad.cpp`
- Test: `port/Test/src/windows_save_tests.cpp`

- [ ] **Step 1: Write the failing test**

Add coverage for selecting a valid backup buffer and retaining live save-file and slot metadata. If menu state cannot be constructed in the test harness, test the staging boundary and retain manual UI verification as the thin integration step.

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest -R KyaPortTest --test-dir out/build/x64-debug`

Expected: the test fails because no direct-load operation exists.

- [ ] **Step 3: Write minimal implementation**

Add `QueueLoad` beside `QueueRestore`. It captures the selected displayed bytes, queues through `EnqueueLevelManageTask`, reuses existing readiness checks, stages the bytes, calls `gSaveManagement.load_level()`, then performs the successful-load post-processing from `MemCardLoad0`. Add an enabled `Load` button beside `Restore` and `Restore & Load`, with explicit queued/success/failure status.

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest -R KyaPortTest --test-dir out/build/x64-debug`

Expected: direct-load staging tests pass and existing save tests remain green.

- [ ] **Step 5: Commit**

```powershell
git add port/DebugMenu/src/DebugSaveLoad.cpp port/Test/src/windows_save_tests.cpp
git commit -m "Load backup saves without restoring"
```

## Chunk 3: Final verification

### Task 3: Build and verify the Windows target

**Files:**
- Modify: none

- [ ] **Step 1: Configure**

Run: `cmake --preset x64-debug`

- [ ] **Step 2: Build**

Run: `cmake --build out/build/x64-debug`

- [ ] **Step 3: Run save tests**

Run: `ctest -R KyaPortTest --test-dir out/build/x64-debug --output-on-failure`

Expected: all tests pass.
