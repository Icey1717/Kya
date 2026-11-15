# Collision and event primitives

Select the level preview and open **Kya > Diagnostics > Primitives**. Enable
**Show Collision Primitives** and/or **Show Event Primitives**. The controls only
refresh these overlays, without rebuilding textured level geometry.

**Collision Primitive Display** and **Event Primitive Display** independently select
**Wireframe**, **Solid**, or **Solid And Wireframe**. Solid faces use a simple directional
shade and adjustable **Primitive Solid Opacity** (default 0.25) so textures remain visible.
Keep **Show Background Meshes** / **Show Cluster Meshes** enabled to view them alongside
the level. Primitive display never switches the viewport to Unreal's collision-only mode.
For inspecting primitives alone, use the existing level geometry visibility controls.

Wires use a constant screen-pixel width (default 1.5) rather than shrinking with distance.
**Primitive Wire Depth Bias** (default 0.1) helps prevent dotted lines where collision and
visible geometry occupy the same surface; both width and bias can be adjusted. Wires and
solids remain depth-tested against the scene, so hidden surfaces stay hidden.

- Orange: authored scenery collision triangles, quads, boxes and spheres.
- Cyan: event zone groups 0 and 2 (inside results).
- Red: event zone group 1 (outside result).
- Yellow: the picked primitive. Its component Details show the source bank and
  selected entry, record offset, transform offset, zone, group, primitive and flags.

These are editor-only overlays with depth testing; they do not become Unreal physics
collision. Sector hide/show also applies to sector primitive components. The event
overlay includes all serialized zones, including zones without an active event.
The existing checkpoint trigger markers remain approximate and independent.

Event boxes, cylinders, cones and spheres use their full serialized affine matrices,
including nonuniform scale. Half-spaces and open-top boxes have finite open guides
extending upward; they do not have a finite top boundary. Group outlines show the
constituent primitives, not a Boolean surface of the final zone. Group ordering is
retained because the native test returns on its first matching group.

## Authoring architecture

`KyaPrimitives` decodes retained bank entries without changing bank bytes. Source
records are independent of the render proxy. Each record retains bank-entry index
and entry-relative byte offsets, shared collision vertex offsets, event membership
offset, zone/group/matrix identities, and serialized matrix direction. The component
holds its source bank. Viewport record indices are temporary picking indices and
must never be used as serialized identities.

An event matrix can be referenced by multiple zones/groups; a collision vertex can
be referenced by multiple faces. A future edit service must decide explicitly whether
an edit changes that shared source or duplicates/reassigns it. Event matrices map
world positions into unit-primitive coordinates, so rendering inverts them. Collision
box/sphere matrices already map local coordinates to game coordinates. Game-to-Unreal
conversion occurs once at the rendering boundary: `(x, y, z)` becomes
`100 * (x, z, y)`, followed by the preview actor transform.

Picking and highlighting use component hit proxies and a separate editor component
visualizer. Geometry editing/export is not enabled yet. Adding it should use the
existing transaction/edit notification/export validation infrastructure, with bounds
and inverse matrix updates in the same transaction. No import schema bump is needed
for this display because it derives records from retained bytes.

Current scope is static scenery `7/1` COL entries and `8/1` EVNT entries. Actor
collision `7/2` templates, procedurally generated actor colliders, collision tree
diagnostics, event messages and runtime-attached zone transforms are not displayed.
Invalid or unsupported entries produce diagnostics and contribute no partial records.

## Evidence and validation

Implementation was compared to repository sources (no decompiled game code changed):

- `LevelScheduler.cpp`: bank callbacks `7/1` and `8/1`, `BnkInstallEvents`.
- `EventManager.cpp`: `_edEventInstallChunk`, `_edEventComputePrimAgainstVertex`,
  `edEventComputeZoneAgainstVertex`; packed `ed_zone_3d` in `Types.h`.
- `edCollisions.cpp`: `edColLoadStatic`; static primitive and OBB layouts in
  `edCollisions.h` and `OBBTree.h`.
- Native reference drawing in `DebugEvent.cpp` and `DebugCollisionDrawing.h`.

`Kya.Primitives.Decode` covers shared event matrices, inverse transforms, truncated
data, invalid indices, singular matrices, nonmutating decoding, component selection,
bounds, toggle cleanup and sector hide/show. `Kya.Primitives.RealBanks` uses
`KYA_TEST_BANK` and neighboring banks. Level one decodes 319 event memberships and
51,634 collision primitives across its 13 sectors, with source bytes unchanged.

The display-edit regression also exercises the actual Details lifecycle
(`PreEditChange` followed by `PostEditChangeProperty`) for every primitive display
control and texture-animation control. Actor pre-edit unregisters components; the
fast post-edit paths restore registration while preserving existing mesh triangles.

Run `Automation RunTests Kya.Primitives` in the editor or the existing headless test
project. `Tools/capture_matched_view.py` accepts `KYA_CAPTURE_COLLISION=1` and
`KYA_CAPTURE_EVENTS=1` for viewport capture. Build and test logs are in
`out/KyaLevelEditor-primitives-build.log` and `out/KyaLevelEditor-primitives-test.log`.
Set `KYA_CAPTURE_COLLISION_MODE` / `KYA_CAPTURE_EVENT_MODE` to `wireframe`, `solid`, or
`both` to capture those display modes. Solid rendering batches faces per bank during
normal rendering; the hit-test pass retains a separate draw range per source primitive.
