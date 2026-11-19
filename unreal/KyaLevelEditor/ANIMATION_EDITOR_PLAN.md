# Animation editor integration plan

Deferred plan for using Unreal's animation tools to inspect and edit Kya animations, then export the results back to the game.

## Current baseline

- The editor's static actor preview uses stored bind poses; animation playback is outside its current scope.
- Sector-bank export currently preserves animation entries verbatim. Editing Unreal animation assets will require a native animation writer before those edits can reach Kya.
- Existing bank identity, edit notifications, undo/redo and validated export patterns should also govern animation edits.

## Stage 1: Investigate native animation data

- Trace skeleton hierarchy, bind transforms, skin weights, animation-bank references, clip timing and native track encoding.
- Establish coordinate, unit, quaternion, bone-index and root-motion conventions.
- Distinguish skeletal clips from hierarchy animation and texture animation; identify which formats the first implementation supports.
- Compare relevant game decoding and playback functions with the original PS2 code through Ghidra. Record function addresses and layout evidence before changing decompiled behavior.
- Document compression, quantization, interpolation, events and any runtime assumptions that constrain export.

Deliverable: a native-format specification and a small set of representative clips with known skeletons and expected poses.

## Stage 2: Playback and inspection

- Build Unreal skeletal mesh and skeleton representations from Kya's hierarchy, bind pose and skin data.
- Decode supported clips into Unreal animation sequences while retaining native bank, entry, clip and bone identities.
- Add clip selection, playback, scrubbing, speed controls and looping to actor preview.
- Expose bone names or stable indices, hierarchy, bind pose and animated transforms for inspection.
- Match native interpolation and timing, including the final frame and loop boundary.
- Keep imported native data authoritative until editing and export are supported.

Acceptance: representative poses and playback match the game, including root movement and hierarchy transforms; unsupported data produces explicit diagnostics.

## Stage 3: Editing through Unreal tools

- Open imported clips in Unreal's Animation Sequence Editor.
- Support Control Rig and Sequencer workflows for pose changes, IK-assisted editing and motion adjustments.
- Bake edited motion onto the original Kya skeleton before applying it to a bank.
- Initially keep bone count, bone ordering, hierarchy and bind pose fixed. Skeleton and skin-weight authoring are separate future work.
- Define handling for clip duration, sampling rate, root motion and native animation events before enabling changes to those properties.
- Provide an explicit Apply to Kya Bank operation with validation, undo/redo and bank change notifications.
- Track derived Unreal assets against their source bank generation so reimport cannot silently apply stale edits.

Acceptance: an edited clip can be baked and applied as one undoable bank edit; incompatible or stale edits are rejected with actionable diagnostics.

## Stage 4: Native encoding and export

- Convert baked local bone transforms back to native coordinates and track conventions.
- Encode supported tracks using the game's required representation, quantization and timing.
- Preserve unrelated clips, unknown fields and animation metadata that the editor does not understand.
- Rebuild entry sizes, offsets, references and alignment where required by the bank format.
- Integrate with staged, validated bank export; reject unsupported edits before publishing output.
- Measure pose error after native encode/decode and report values outside an established tolerance.

Acceptance: export/reimport retains the intended motion, untouched data remains preserved, and edited clips play correctly in the PC port.

## Verification

- Cover bind pose, parent-child transforms, axis conversion, quaternion continuity, single-frame clips, loop boundaries and root movement.
- Exercise representative native encodings and multiple character skeletons rather than relying only on synthetic clips.
- Check unedited round trips for byte preservation where possible and decoded equivalence where rebuilding is necessary.
- Verify edited round trips, undo/redo, source reimport invalidation and interrupted or rejected export.
- Compare selected poses and playback against the running game before declaring a format supported.

## Suggested first slice

Import one character skeleton and one native clip, render its skin correctly, and provide a scrubber with bone inspection. Establish playback correctness before adding Control Rig or native writeback.

## Unreal references

- [Animation Sequence Editor](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-sequence-editor-in-unreal-engine)
- [Animating with Control Rig](https://dev.epicgames.com/documentation/en-us/unreal-engine/animating-with-control-rig-in-unreal-engine)
- [Animation Editor Mode](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-editor-mode-in-unreal-engine)

Confirm API and plugin dependencies against the Unreal version used by this project when implementation begins.
