# Level editor coverage and checkpoint

This is a working checklist, not a claim of complete Create-stage coverage.
Update it when evidence or implementation changes.

## Working cadence

- Reconcile the working tree and unfinished work before each continuation; retain the full level-editor objective.
- Group related changes into coherent milestones before rebuilding; use intermediate checks when risk or debugging warrants them.
- Save complete build logs under `out/`, report concise results, and inspect detailed log sections only when needed. Poll long-running commands at 30–60 second intervals.
- Reuse established findings and passing checks; rerun checks after relevant changes, failures, or unresolved concerns.
- Update this coverage checklist and record completed work, decisions, gaps, and the next action at milestone boundaries.

## Create-stage data and editing

- [ ] Inventory every scene, manager, camera, and actor Create reader against the bank stream; identify all unrepresented fields and conditional layouts.
- [ ] Scene configuration, environment, camera, map, effects, objectives, waypoints, paths, and common actor placement fields have editable properties and fixed-layout export. Coverage of individual fields and optional cases is incomplete.
- [ ] Actor class-specific behaviour payloads are exposed as bytes; recover and expose their typed fields. Actor identities and path layout flags are currently read-only.
- [ ] Support adding, removing, and resizing variable-length records and changing layout-controlling fields safely; current export retains the original layout and restricts string sizes.
- [ ] Validate optional camera switching, condition, and configuration records with nonempty fixtures and edited round trips.

## Rendering and editor interaction

- [ ] Background and sector meshes, static cluster strips, and optional static actor meshes are approximated; inspect against the running game and fix missing geometry, hierarchy, transforms, visibility, animation, and material behavior.
- [ ] Indexed and direct-colour texture paths cover some GS formats; verify real direct-colour materials and support remaining formats, blending, alpha tests, texture layers, and colour-space behavior.
- [ ] Waypoint, selected path-point, and selected actor position markers write back to the bank; expand native viewport editing and verify undo, persistence, and property refresh.
- [ ] Compare a representative editor render with the game from matching viewpoints; initial captures show the preview has substantial untextured geometry.

## Export and validation

- [ ] Export edited banks and verify them in the game; current export is experimental and only handles uncompressed fixed-layout banks.
- [ ] Add coverage for malformed/truncated data, optional layouts, and each changed property family; current LevelOneRoundTrip test is broad but not exhaustive.
- [x] UE 5.8 Win64 plugin build succeeded on 2026-11-09; full output: `out/KyaLevelEditor-build.log`.
- [x] `Kya.Bank.LevelOneRoundTrip` passed against LEVEL_1 on 2026-11-09 with NullRHI; full logs: `out/KyaLevelEditor-test.log` and `out/UnrealBankTest/Saved/Logs/BankTest.log`. This does not verify rendered pixels or in-game export.
- [x] Cluster material milestone: UE 5.8 plugin build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-cluster-build.log` and `out/KyaLevelEditor-cluster-test.log`. The test resolves material hashes from real sector strips.
- [x] Cluster tree parser build and LevelOneRoundTrip passed. A synthetic fixture verifies nested nodes, distinct per-node material banks, and rejection of a child that exceeds its parent; logs `out/KyaLevelEditor-nested-fixture-build.log` and `out/KyaLevelEditor-nested-fixture-test.log`. The real-sector test verifies root traversal.
- [x] Four-bit indexed texture milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-psmt4-build-retry.log` and `out/KyaLevelEditor-psmt4-test-retry.log`. Synthetic tests cover nibble addressing and CSM1 palette indexing, but actual rendered colour fidelity remains unverified.
- [x] Indexed upload milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-indexed-upload-build-retry.log` and `out/KyaLevelEditor-indexed-upload-test-retry.log`. A synthetic PSMT4 upload test passes.
- [x] Alpha-mask milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-alpha-mask-build.log` and `out/KyaLevelEditor-alpha-mask-test.log`. The test verifies a real sector material selects masked rendering, not its rendered pixels.
- [x] Zero-width GS texture milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-zero-tbw-build.log` and `out/KyaLevelEditor-zero-tbw-test.log`. Tests cover zero TEX0 buffer width for both indexed formats.
- [x] Scene configuration field milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-scene-create-build-retry.log` and `out/KyaLevelEditor-scene-create-test-retry.log`. The test edits and reimports each newly exposed field group and checks unrelated bytes.
- [x] Actor common Create milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-actor-create-build.log` and `out/KyaLevelEditor-actor-create-test.log`. The test edits a real actor scenario condition and class-data byte, reimports them, and checks unrelated bytes.
- [x] Checkpoint manager milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-checkpoints-build-retry.log` and `out/KyaLevelEditor-checkpoints-test.log`. A real checkpoint sector edit round-trips and unrelated bytes remain unchanged.
- [x] Clusteriser zone milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-cluster-zones-final-build.log` and `out/KyaLevelEditor-cluster-zones-final-test.log`. A real zone reference edit round-trips and unrelated bytes remain unchanged.
- [x] Moving-platform prefix milestone: UE 5.8 build and LevelOneRoundTrip passed; logs `out/KyaLevelEditor-platform-fixture-build.log` and `out/KyaLevelEditor-platform-fixture-test.log`. Real platform weight, carried-actor and zone edits round-trip with unrelated-byte checks. A synthetic fixture checks no-friction parsing and truncation; no-friction export edits still need a fixture.

## Checkpoint — 2026-11-09

Completed: reconciled pending untextured preview material, which displays decoded vertex colours; built plugin and passed existing bank automation. Existing plugin retains original bank bytes and patches supported fixed-size fields on export. Runtime and editor modules remain separate for future use.

Remaining: the Create-stage inventory, typed actor data, resizable export, faithful rendering, interactive editing, and in-game verification are incomplete. Unrelated dirty `port/KyaMesh` and `port/KyaTexture` submodules were left untouched.

Visual evidence: the test project contains `LEVEL`, `SECT1`–`SECT3`, and `Empty` assets. An isolated game working directory with startup level override 1 produced `out/screenshots/level-one-reference.png`. The unsaved Unreal preview capture is `out/screenshots/unreal-level-preview.png`, generated by `out/level-capture/unreal_preview.py` with the imported scene camera and three sectors. Views differ, but the Unreal image shows extensive pale untextured geometry while the sky is textured. Both launched processes were closed; neither capture proves rendering fidelity.

Cluster checkpoint: root cluster strips now resolve their MBNK material hashes using sector-local texture data and the common scene texture bank. `out/screenshots/unreal-level-cluster.png` shows the central tree textured compared with the prior capture; much of the scene remains untextured. The captures do not use matching game and editor cameras.

Cluster-tree checkpoint: added bounded quadtree and octree traversal with per-node material banks. The first test exposed a difference between on-disk `HASH` chunks and runtime-prepared memory; corrected traversal passes on the real sector root and a synthetic nested fixture. `out/level-capture/count_cluster_nodes.py` found one root node and zero nested nodes in each of the 13 LEVEL_1 sector banks, so nested traversal does not explain the pale level 1 geometry. Other real nested formats have not been visually verified.

Material audit checkpoint: sector 1 resolves and decodes 17/17 cluster strips; sector 2 resolves all 368, with decoded strips increasing from 268 to 304 after PSMT4 support; sector 3 resolves all 76, increasing from 62 to 63. The remaining 64 and 13 decode failures need diagnosis by reason. These counts measure strip material decodability, not rendered pixel correctness.

Indexed upload checkpoint: failure diagnostics identified unsupported indexed uploads and narrow GS buffer widths. Supporting PSMT8/PSMT4 uploads and the renderer's page stride yields 17/17, 368/368, and 76/76 decodable cluster-strip materials in sectors 1, 2, and 3 respectively. `out/screenshots/unreal-level-indexed-upload.png` now shows textures on previously pale geometry, but its orange cast and remaining gaps need investigation; these counts do not prove correct pixel colours, transparency, geometry, or the other sectors.

Alpha checkpoint: all 13 level 1 sectors were audited (`out/KyaLevelEditor-alpha-audit-test.log`). Most cluster strips use GS alpha test GREATER 1 with failure KEEP; the preview now uses an Unreal masked material for that exact state. `out/screenshots/unreal-level-alpha-mask.png` shows foliage cutouts, although colour, mask fidelity and gaps remain unverified against the game. Other alpha modes and blending are not implemented. Sectors 4, 5, and 12 still contain 6, 13, and 9 texture decode failures respectively; one sector 5 strip has no resolvable material hash.

Texture audit checkpoint: the remaining 28 decode failures were 32×32 indexed textures with TEX0 buffer width zero. The port's GS address calculation accepts zero, and the plugin now does too. All resolved cluster-strip materials across the 13 level 1 sectors decode in the audit. The sole sector 5 strip without a hash has material index -1, which the game treats as untextured. Decode success does not verify pixel correctness, transparency, material layers, or rendering outside cluster strips.

Create-stage checkpoint: scene configuration now exposes the two header words, common mesh values, texture word, four post-animation/scene/sector words, and particle word consumed or skipped by the game's readers. The exporter patches their original offsets and enforces fixed counts. Existing imported assets must be reimported to populate these arrays before export. This does not establish coverage of all Create readers.

Actor checkpoint: `CActor::Create` scenario conditions are editable as typed records; remaining per-class Create data is now available as fixed-size serialized bytes. This preserves and exposes the data but does not recover class-specific semantics. Previously imported assets require reimport before export to populate the added arrays.

Checkpoint manager checkpoint: class 0x1e now exposes checkpoint zone, waypoint, sector, flags, and actor/waypoint pairs as typed fixed-size properties. The original class payload supplies any remaining bytes; record and pair counts cannot yet be resized. Reimport older assets to populate typed checkpoints.

Clusteriser checkpoint: class 0x18 behaviour 2 now exposes zone flags and references, actor/camera/light lists, and 64-bit hashes as typed fixed-size properties. Raw behaviour editing is disabled when that payload is decoded to avoid conflicting edits. Reimport older assets to populate the typed zones; list resizing remains unsupported.

Moving-platform checkpoint: the level 1 stream contains 166 class 0x0d actors. Their movable weight, carried actor/carry method list, zone list, and no-friction zone actor lists now have typed fixed-size properties. Remaining platform data is retained in a separate editable raw suffix, avoiding overlap with the typed prefix. Reimport older assets before export. The platform's camera, sound and other properties still need typed decoding.

Platform validation checkpoint: level 1 has populated carried-actor and zone lists, but no no-friction zone lists. Tests now exercise the former edits against the real bank and the latter parser with synthetic data; export of nonempty no-friction lists remains unverified. The remaining suffix consists of camera records, a 76-byte property block, and two sound lists.

Sector diagnosis checkpoint: game camera metadata showed active sectors 1 and 7, not the earlier preview's sectors 1–3. Opt-in Unreal diagnostics revealed that sector 1's cluster references 589 renderable hierarchy strips in addition to 17 static strips; sector 7 references 27 hierarchy strips and has no static strips. The preview now submits those referenced hierarchy strips. The sector 1–7 capture at `out/screenshots/matched-unreal-hierarchies.png` shows the forest path that was missing; `out/screenshots/matched-game.png` is the corresponding game capture. Geometry placement now visibly aligns, while colour, FOV/aspect, dynamic actors, sprites and material behavior still differ. Diagnostic logs are in `out/level-capture/sector-hierarchies-before.log` and `out/level-capture/sector-hierarchies-after.log`.

The incremental UE 5.8 build and LevelOneRoundTrip test passed with explicit checks for sector 1's 589 hierarchy strips and sector 7's 27 hierarchy-only strips; logs `out/KyaLevelEditor-sector7-regression-build.log` and `out/KyaLevelEditor-sector7-regression-test.log`. The Win64 plugin package succeeded; log `out/KyaLevelEditor-sector-hierarchy-package-retry.log`.

Linked hierarchy checkpoint: cluster rendering now follows each serialized hierarchy link, including through hidden or non-rendering records, and deduplicates shared chains. Invalid links and cycles are rejected. Synthetic tests cover shared tails, cycles and bad targets; real SECT1 and SECT7 chains parse successfully. The incremental editor build, LevelOneRoundTrip, and Win64 package passed: `out/KyaLevelEditor-linked-hierarchy-real-build.log`, `out/KyaLevelEditor-linked-hierarchy-real-test.log`, and `out/KyaLevelEditor-linked-hierarchy-package.log`. This does not verify linked geometry visually, dynamic LOD selection, or sprites.

Matched-camera checkpoint: the reusable capture now pilots an unsaved camera constrained to the game's recorded aspect and horizontal FOV. `out/screenshots/matched-unreal-constrained.png` (2345×1759) and `out/screenshots/matched-game.png` (2005×1504) show corresponding path landmarks in the same framing; dynamic actors were disabled in Unreal. Colour, lighting, distant background, sprites, LOD and animation still differ, and resolution and dynamic scene state differ. Logs are in `out/level-capture/constrained-camera.log`. The real capture also found 11 linked SECT1 hierarchy records and two in SECT7, with unchanged renderable strip counts for this view. The Win64 package passed; log `out/KyaLevelEditor-constrained-camera-package.log`.

Background visibility checkpoint: `Sector::Init` hides the common background while a loaded sector supplies its own; the preview now follows that rule and restores the common background when no sector background is loaded. A real SECT1 regression compares vertex counts with and without its background entry. The incremental build, LevelOneRoundTrip and Win64 package passed: `out/KyaLevelEditor-background-visibility-build.log`, `out/KyaLevelEditor-background-visibility-test.log`, and `out/KyaLevelEditor-background-visibility-package.log`. The matched capture `out/screenshots/matched-unreal-sector-background.png` confirms suppression but looks broadly similar, so it does not explain the main colour mismatch. `C3DFileManager::ManageBackground` also moves background hierarchies with the camera under scene flags; the preview does not yet reproduce that behavior.

Background hierarchy checkpoint: inventoried sector background HALL roots and descendants with `Tools/inventory_background_hierarchies.py`. SECT1 has two descendants, one excluded by the game's fixed-background hash; SECT2 has 98, one excluded. Added bounded classification and camera-relative world-matrix evaluation, with real SECT1 and synthetic fixed-hash/cycle tests. The incremental build, LevelOneRoundTrip and Win64 package passed: `out/KyaLevelEditor-background-fixture-build.log`, `out/KyaLevelEditor-background-fixture-test.log`, `out/KyaLevelEditor-background-fixture-package.log`. These helpers are not yet connected to viewport rendering; they do not establish visual correctness.

Camera-follow checkpoint: the preview now computes camera-relative background vertex ranges from the hierarchy classification and updates them from the active level-editing viewport camera when `BackgroundFlags & 1` is set. Fixed-hash geometry and other scene geometry remain fixed. A real SECT1 regression toggles the flag and checks camera movement and fixed geometry; incremental build and LevelOneRoundTrip passed: `out/KyaLevelEditor-camera-background-regression-build.log` and `out/KyaLevelEditor-camera-background-regression-test.log`. The Win64 package passed: `out/KyaLevelEditor-camera-background-package.log`. The matched LEVEL_1 capture reports `BackgroundFlags=0` and zero moving ranges, so this behavior does not explain that capture's colour or background mismatch. One active viewport supplies the camera for the whole preview; independent simultaneous viewport transforms, runtime camera selection, and a flag-enabled game-to-editor visual comparison remain unverified.

Next: investigate lighting and distant background mismatch using the matched view, and support independent viewport camera-relative rendering; continue sprites, LOD and the remaining platform payload and Create-stage audit.
